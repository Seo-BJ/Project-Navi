# AnimGraph 입력 스냅샷 기반 Lag Compensation — 사전 조사 보고서

**대상 엔진:** UE 5.5
**현재 구현 경로:** `Source/LyraGame/LagCompensation/LyraLagCompensationComponent_SkeletalMesh.{h,cpp}`
**참조 사례:** Valorant — "Update만 매 tick 실행 + 노드 입력값 스냅샷, hit 검증 시에만 과거 시점으로 Evaluate"
([Riot Tech Blog: Peeking Into Valorant's Netcode](https://technology.riotgames.com/news/peeking-valorants-netcode))

---

## 0. 목표 정리

### 0.1 현재 방식 (본 트랜스폼 스냅샷)

[LyraLagCompensationComponent_SkeletalMesh.cpp:79-106](Source/LyraGame/LagCompensation/LyraLagCompensationComponent_SkeletalMesh.cpp) `CacheCurrentFrame()`:

- 매 `TG_PostPhysics` tick마다 `Mesh->GetComponentSpaceTransforms()`로 **모든 본의 component-space transform을 통째로 복사**
- `FrameHistory`에 `MaxRecordTime=2.0s` 분량 누적
- Rewind 시: 보간 → physics asset capsule/sphere/box 충돌 검사

비용:
- **매 캐릭터 × 매 tick × 본 수** 만큼 `FTransform` 복사
- 본 위치를 얻기 위한 AnimGraph Evaluate는 이미 엔진이 매 tick 모두 수행 중 (그래야 위 transforms가 채워짐)

### 0.2 목표 방식 (Valorant 스타일)

- **Tick마다는 AnimGraph Update만** 실행 → 시간/state 누적은 하되 본 pose 계산은 생략 가능
- **노드별 internal state를 스냅샷**으로 저장 (위치 배열 대비 훨씬 작음)
- **Hit 검증이 발생한 캐릭터에 대해서만** 과거 시점으로 state 복원 → 그 시점에 1회 Evaluate → 본 transform 획득

이득의 출처:
- 매 tick 전체 캐릭터 N명에 대해 `Evaluate + bone copy`를 돌리던 비용을 → "총 발사 횟수 × 후보 캐릭터 수"로 환산. 일반적으로 후자가 훨씬 작음.
- 스냅샷 메모리도 (본 수 × FTransform) → (노드 수 × 작은 state) 로 줄어듦

---

## 1. UE5 AnimGraph Update / Evaluate 분리 구조

### 1.1 호출 진입점

**Update 경로** — `FAnimInstanceProxy::UpdateAnimation()`
([UE_5.5/Engine/Source/Runtime/Engine/Private/Animation/AnimInstanceProxy.cpp:1220](file:///C:/Program%20Files/Epic%20Games/UE_5.5/Engine/Source/Runtime/Engine/Private/Animation/AnimInstanceProxy.cpp))

```cpp
void FAnimInstanceProxy::UpdateAnimation()
{
    FMemMark Mark(FMemStack::Get());
    FAnimationUpdateSharedContext SharedContext;
    FAnimationUpdateContext Context(this, CurrentDeltaSeconds, &SharedContext);
    UpdateAnimation_WithRoot(Context, RootNode, NAME_AnimGraph);
    Sync.TickAssetPlayerInstances(*this, CurrentDeltaSeconds);
}
```

- `RootNode`(= `FAnimNode_Base*`)부터 DFS로 각 노드의 `Update_AnyThread()` 호출
- 이 단계에서 **pose 데이터(`FCompactPose`)는 만들지 않음**. 시간 누적, state 전이, blend weight 갱신만.

**Evaluate 경로** — `FAnimInstanceProxy::EvaluateAnimation()` / `EvaluateAnimationNode_WithRoot()`
([같은 파일:1395 / 1485](file:///C:/Program%20Files/Epic%20Games/UE_5.5/Engine/Source/Runtime/Engine/Private/Animation/AnimInstanceProxy.cpp))

```cpp
void FAnimInstanceProxy::EvaluateAnimationNode_WithRoot(FPoseContext& Output, FAnimNode_Base* InRootNode)
{
    if (InRootNode != nullptr) {
        EvaluationCounter.Increment();
        InRootNode->Evaluate_AnyThread(Output);  // FCompactPose에 본 transform 작성
    } else {
        Output.ResetToRefPose();
    }
}
```

- 여기서만 `FPoseContext::Pose` (= `FCompactPose`, local-space bone transform) 채움
- 이후 `USkeletalMeshComponent`가 component-space로 변환

### 1.2 SkeletalMeshComponent 호출 흐름

- `USkeletalMeshComponent::TickComponent` → `TickPose()` → `TickAnimation()` → `UpdateAnimation()`
- `USkeletalMeshComponent::RefreshBoneTransforms()` → `EvaluateAnimation()`

**핵심:** UE는 이미 Update와 Evaluate를 별도 함수로 분리. Tick 단계에서 둘 다 호출되지만, **둘 중 하나만 호출하는 것도 구조적으로 합법**.

### 1.3 기존에 존재하는 "Update만" 메커니즘

[SkinnedMeshComponent.h:84-104](file:///C:/Program%20Files/Epic%20Games/UE_5.5/Engine/Source/Runtime/Engine/Public/Components/SkinnedMeshComponent.h) `EVisibilityBasedAnimTickOption`:

```cpp
enum class EVisibilityBasedAnimTickOption : uint8
{
    AlwaysTickPoseAndRefreshBones,
    AlwaysTickPose,                                       // ★ Update만, Evaluate 스킵
    OnlyTickMontagesAndRefreshBonesWhenPlayingMontages,
    OnlyTickMontagesWhenNotRendered,
    OnlyTickPoseWhenRendered,
};
```

- `AlwaysTickPose`가 정확히 "Update는 하지만 화면 밖이면 RefreshBoneTransforms는 스킵" 의미
- 즉 **엔진이 이미 'Update만 돌리는 모드'를 지원**. 우리는 이 메커니즘을 hijack하거나 비슷한 방식으로 서버 측에 적용 가능.

### 1.4 Parallel Animation Evaluation 주의점

- `bUseMultiThreadedAnimationUpdate` (AnimInstance.h:346) 활성 시 워커 스레드에서 Update/Evaluate 실행
- `FAnimationEvaluationContext` ([SkeletalMeshComponent.h:76]) 이중 버퍼 스왑
- **외부에서 강제 Evaluate 호출 시 race 위험** → `IsRunningParallelEvaluation()` 체크 후 game thread 동기화 지점에서 수행해야 함

---

## 2. FAnimNode_Base Lifecycle

[AnimNodeBase.h:828-861](file:///C:/Program%20Files/Epic%20Games/UE_5.5/Engine/Source/Runtime/Engine/Classes/Animation/AnimNodeBase.h):

| 가상 함수 | 호출 시점 | 우리 시스템에서의 역할 |
|---|---|---|
| `Initialize_AnyThread` | 그래프 첫 활성화, state 진입 시 | — |
| `CacheBones_AnyThread` | LOD 변경 등 본 인덱스 재계산 필요 시 | — |
| `Update_AnyThread` | **매 tick** (시간 누적, state 갱신) | **매 tick 호출 + 직후 스냅샷 캡처** |
| `Evaluate_AnyThread` | RefreshBoneTransforms 시 (pose 계산) | **rewind 시점에만 호출** |
| `EvaluateComponentSpace_AnyThread` | component-space 평가가 필요한 노드용 | 위와 동일 |
| `GatherDebugData` | 디버그 표시 (GameThread) | — |

### Context 타입

- `FAnimationUpdateContext` — Update 인자. `DeltaTime`, `CurrentWeight`, `SharedContext` 포함
- `FPoseContext` — Evaluate 출력 버퍼. `FCompactPose Pose`, `FBlendedCurve Curve`. **MemStack 임시 할당**이라 함수 밖으로 보존 불가
- `FComponentSpacePoseContext` — IK 등 component-space 노드용

---

## 3. 주요 Stateful 노드의 Internal State

이 섹션이 **이번 작업의 난이도를 결정**하는 핵심. 각 노드 유형마다 "Update에서 누적되어 Evaluate 결과에 영향을 주는 변수"를 식별해야 함.

### 3.1 FAnimNode_StateMachine

[AnimNode_StateMachine.h:122 부근](file:///C:/Program%20Files/Epic%20Games/UE_5.5/Engine/Source/Runtime/Engine/Classes/Animation/AnimNode_StateMachine.h):

| 필드 | 타입 | 스냅샷 필요 |
|---|---|---|
| `CurrentState` | `int32` | ✅ 필수 |
| `ElapsedTime` | `float` | ✅ 필수 |
| `ActiveTransitionArray` | `TArray<FAnimationActiveTransitionEntry>` | ✅ 각 전이의 ElapsedTime, Alpha |
| `QueuedTransitionEvents` | `TArray<FTransitionEvent>` | ⚠️ 통상 1프레임 내 소비됨, 보존 여부 검토 |
| `bFirstUpdate` | `bool` | ⚠️ 첫 진입 마커 |

**난점:** 각 활성 state 내부의 sub-graph도 재귀적으로 스냅샷 필요. State 전이 중이면 두 sub-graph 모두 상태 보존해야 함.

### 3.2 FAnimNode_SequencePlayerBase / SequencePlayer

[AnimNode_SequencePlayer.h](file:///C:/Program%20Files/Epic%20Games/UE_5.5/Engine/Source/Runtime/Engine/Classes/Animation/AnimNode_SequencePlayer.h):

| 필드 | 의미 | 스냅샷 |
|---|---|---|
| `InternalTimeAccumulator` (AssetPlayer 인터페이스) | 현재 재생 위치 | ✅ 필수 |
| `PlayRateScaleBiasClampState` | PlayRate 보간 상태 | ✅ |
| `MarkerTickRecord` | 동기화 마커 | ✅ 동기화 그룹 사용 시 |

이 노드는 비교적 **단순** — 핵심은 `CurrentTime` 하나.

### 3.3 FAnimNode_BlendSpacePlayerBase

- 현재 X/Y/Z 입력 (이미 외부 입력이지만 필터링된 값을 노드가 유지)
- 샘플 가중치 캐시
- 마커 동기화 상태

(이 노드는 직접 확인 못 했음. 구현 시 별도 조사 필요.)

### 3.4 FAnimNode_Inertialization ★ 가장 까다로움

[AnimNode_Inertialization.h:200](file:///C:/Program%20Files/Epic%20Games/UE_5.5/Engine/Source/Runtime/Engine/Classes/Animation/AnimNode_Inertialization.h):

| 필드 | 타입 | 비고 |
|---|---|---|
| `CurrentState` | `EInertializationState` (Inactive/Pending/Active) | ✅ |
| `PreviousPose` | `FInertializationSparsePose` | **본 전체 transform 포함** — 스냅샷이 사실상 본 위치 캐시와 동급 |
| `Curves` | `FInertializationCurve` | ✅ |

**중요:** Inertialization은 정의상 "이전 pose"를 들고 있어야 동작함. 즉 이 노드 하나만으로도 **본 전체 분량의 데이터**가 스냅샷에 포함됨. Valorant 시스템의 메모리 절감 효과가 이 노드 때문에 크게 줄어들 수 있음.

### 3.5 기타 위험 노드

- **`FAnimNode_RigidBody`** (Physics IK): 물리 시뮬레이션 상태. **deterministic하지 않음** → 과거 평가 시 결과 보장 불가
- **`FAnimNode_AnimDynamics`**: 위와 동일
- **Montage** (AnimInstance에서 관리, 노드 아님): `FAnimMontageInstance` 배열을 AnimInstance가 직접 보유 → 별도 스냅샷 경로 필요

---

## 4. Root 노드 접근 및 외부 평가 트리거

[AnimInstanceProxy.h:1052](file:///C:/Program%20Files/Epic%20Games/UE_5.5/Engine/Source/Runtime/Engine/Public/Animation/AnimInstanceProxy.h):

```cpp
protected:
    FAnimNode_Base* RootNode;
```

접근 API (이미 제공됨):

```cpp
FAnimNode_Base* GetRootNode();
bool HasRootNode() const;

// 핵심: 임의 root로 Update/Evaluate 호출 가능
void UpdateAnimation_WithRoot(const FAnimationUpdateContext&, FAnimNode_Base*, FName LayerName);
void EvaluateAnimation_WithRoot(FPoseContext&, FAnimNode_Base*);
void EvaluateAnimationNode_WithRoot(FPoseContext&, FAnimNode_Base*);
```

또한 노드 인덱스 기반 접근:

```cpp
FAnimNode_Base* GetMutableNodeFromIndexUntyped(int32 NodeIdx, UScriptStruct* RequiredStructType);
template<class NodeType> NodeType* GetCheckedMutableNodeFromIndex(int32 NodeIdx);
```

이게 있어서 **개별 노드를 인덱스로 찾아 state를 직접 쓰는 게 가능**. 다만 각 노드의 private state에 접근하려면:

1. Engine 측 `FAnimNode_*` 클래스에 friend 추가 (엔진 fork)
2. 또는 public setter/getter 추가 (엔진 fork)
3. 또는 reflection/offsetof로 우회 (위험, 버전 호환성 깨짐)

**현실적으로 1번이 표준 경로**. 엔진 수정이 필수입니다.

---

## 5. 본 transform 결과 획득

Evaluate가 채우는 `FPoseContext::Pose`는 **local-space, MemStack 임시**. 보존하려면:

1. 함수 종료 전에 component-space로 변환 → 일반 메모리로 복사
2. 또는 `USkeletalMeshComponent::FillComponentSpaceTransforms()` 경로를 거치게 → `GetComponentSpaceTransforms()`로 획득

현재 `LyraLagCompensationComponent_SkeletalMesh::CacheCurrentFrame()`이 호출하는 [GetComponentSpaceTransforms()](Source/LyraGame/LagCompensation/LyraLagCompensationComponent_SkeletalMesh.cpp:97) 와 같은 출력을 얻으면 **기존 ConfirmHit 로직을 그대로 재활용 가능**.

---

## 6. 제안 아키텍처

### 6.1 데이터 구조

```cpp
struct FAnimNodeInputSnapshot
{
    int32 NodeIndex = INDEX_NONE;
    UScriptStruct* NodeTypeHint = nullptr;  // 디버깅/검증

    // Union-like (노드 타입에 따라 일부만 채워짐)
    struct FStateMachineState
    {
        int32 CurrentState = INDEX_NONE;
        float ElapsedTime = 0.f;
        TArray<FAnimationActiveTransitionEntry> ActiveTransitions;
    };
    struct FSequencePlayerState
    {
        float CurrentTime = 0.f;
        FMarkerTickRecord MarkerRecord;
    };
    struct FInertializationState
    {
        uint8 State = 0;
        FInertializationSparsePose PreviousPose;
        FInertializationCurve Curves;
    };
    // ... 노드 타입별 sub-state

    TVariant<FStateMachineState, FSequencePlayerState, FInertializationState /*, ...*/> Payload;
};

struct FAnimGraphFrameSnapshot
{
    float Time = 0.f;
    FTransform ComponentTransform;                      // mesh의 world transform
    TArray<FAnimNodeInputSnapshot> NodeSnapshots;       // 인덱스 정렬

    // Montage 등 AnimInstance 레벨 상태
    TArray<FAnimMontageInstanceSnapshot> MontageStates;
};
```

### 6.2 캡처 (매 tick, Update 직후)

```cpp
void Capture(FAnimInstanceProxy& Proxy, FAnimGraphFrameSnapshot& Out)
{
    // RootNode부터 DFS 순회. 각 노드 타입을 캐스팅해서 필요한 필드 복사.
    // 노드 인덱스를 안다면 GetCheckedMutableNodeFromIndex로 직접 접근.
    // PoseLink 순회는 별도 visitor 필요 (각 노드의 자식 PoseLink 식별).
}
```

**구현 난제:** PoseLink는 노드 멤버로 `UPROPERTY()`되어 있어서 UStruct reflection으로 순회 가능하지만, 각 노드 타입마다 패턴이 다름.

### 6.3 복원 + Evaluate (rewind 시점)

```cpp
FPoseContext EvaluateAtPastFrame(USkeletalMeshComponent* Mesh, const FAnimGraphFrameSnapshot& Snap)
{
    UAnimInstance* AI = Mesh->GetAnimInstance();
    FAnimInstanceProxy& Proxy = AI->GetProxyOnGameThread<FAnimInstanceProxy>();

    // 1. 현재 노드 상태 백업 (rewind 후 복원용)
    FAnimGraphFrameSnapshot Backup;
    Capture(Proxy, Backup);

    // 2. Snap의 상태를 노드에 강제 주입 (engine fork 필요)
    Restore(Proxy, Snap);

    // 3. Evaluate만 수행
    FMemMark Mark(FMemStack::Get());
    FPoseContext Out(&Proxy);
    Proxy.EvaluateAnimation(Out);

    // 4. Component-space로 변환 + 결과 보존
    //    (Mesh->FillComponentSpaceTransforms 경로 사용)

    // 5. 현재 상태 복원
    Restore(Proxy, Backup);

    return Out;  // 또는 component-space 배열 반환
}
```

### 6.4 엔진 수정 범위 (최소)

다음 위치에 friend 선언 또는 public 접근자 추가:

1. `FAnimNode_StateMachine` — `CurrentState`, `ElapsedTime`, `ActiveTransitionArray` setter
2. `FAnimNode_SequencePlayerBase` — `InternalTimeAccumulator` setter
3. `FAnimNode_BlendSpacePlayerBase` — 동기화 상태 setter
4. `FAnimNode_Inertialization` — `CurrentState`, `PreviousPose`, `Curves` setter
5. `FAnimInstance` — Montage instance 배열 접근자 (이미 일부 존재)

게임 모듈에 별도 helper를 만들고, 위 setter들을 호출하는 식. **엔진 자체를 fork하지 않고는 어려움**.

---

## 7. 위험 요소 / Open Questions

| # | 이슈 | 영향 | 대응 후보 |
|---|---|---|---|
| 1 | **Inertialization은 본 전체 pose를 들고 있음** | 메모리 절감 효과 반감 | Inertialization이 active일 때만 별도 저장. 또는 sparse 압축. |
| 2 | **Physics 기반 노드 (RigidBody, AnimDynamics)는 deterministic 아님** | 과거 evaluate 결과 부정확 | 이 노드들은 rewind 대상에서 제외하고, 본 위치만 별도 캐시 (하이브리드) |
| 3 | **Parallel anim evaluation 충돌** | crash 또는 corrupted pose | rewind는 game thread에서만, 또는 worker가 idle한 시점에 동기화 |
| 4 | **Montage는 AnimInstance 멤버, 노드 아님** | 노드 visitor만으로는 캡처 못 함 | 별도 montage snapshot 경로 추가 |
| 5 | **AnimBP 변수 (모든 노드 입력의 원천)** | 외부에서 매 tick 바뀌는 값들. 노드 입력의 일부 | AnimInstance의 변수 메모리 자체를 스냅샷? blueprint property는 reflection으로 가능 |
| 6 | **노드 그래프 토폴로지 변경 (LOD, Layer overrides)** | 스냅샷 시점과 evaluate 시점의 노드 인덱스 일치 보장 | rewind window 동안은 LOD 고정, 또는 토폴로지 변경 시 snapshot invalidate |
| 7 | **엔진 fork 유지 비용** | 엔진 버전업마다 패치 재적용 | Setter를 plugin module 안에서 reinterpret_cast로 우회 (비표준이지만 fork 불필요) |
| 8 | **Pose가 dependent on world state** (e.g. AimOffset이 target actor 향함) | 과거 evaluate 시 target도 과거 위치여야 정확 | 노드 입력에 포함된 actor 위치도 함께 스냅샷 |

### 7번 추가 설명

엔진 fork를 피하고 싶다면, **friend declaration 대신 sizeof + offsetof를 활용한 unsafe accessor**를 게임 모듈에서 만들 수 있음. 예:

```cpp
// 엔진 버전 고정 (5.5.x) 기준. UE 업그레이드 시 깨질 위험 있음.
struct FStateMachineAccessor
{
    static void SetCurrentState(FAnimNode_StateMachine& Node, int32 NewState)
    {
        // 헤더에서 CurrentState 위치 분석 후 offsetof
        constexpr SIZE_T Offset = /* offsetof 계산 */;
        *reinterpret_cast<int32*>(reinterpret_cast<uint8*>(&Node) + Offset) = NewState;
    }
};
```

**권장하지 않음** — 엔진 fork가 훨씬 안전. 단 개인 포트폴리오라면 트레이드오프 검토 가치 있음.

---

## 8. 구현 단계 제안

이 작업의 전체 규모를 고려해 **점진적으로** 진행하는 것을 권장:

### Phase 1 — 검증 (1~2주)
- 단순 AnimBP (SequencePlayer + StateMachine 한두 개) 위에서 PoC
- StateMachine + SequencePlayer만 스냅샷/복원
- Update만 매 tick, Evaluate는 강제로 1회 따로 호출 → 결과 본 위치가 "원본 매 tick Evaluate 결과"와 일치하는지 비교 테스트
- **이 단계에서 안 되면 작업 전체를 재검토**

### Phase 2 — 노드 커버리지 확장 (2~4주)
- BlendSpace, Inertialization, LayeredBoneBlend 등 추가
- Montage 스냅샷 경로 추가
- 실제 게임 캐릭터 AnimBP에서 동작 검증

### Phase 3 — 통합 (1~2주)
- `ULyraLagCompensationComponent_SkeletalMesh`의 `CacheCurrentFrame` / `GetHitTimeFrame` / `ConfirmHit` 경로를 교체
- Update-only 모드로 서버 측 SkeletalMeshComponent 설정 (또는 자체 Tick에서 RefreshBoneTransforms만 스킵)
- 기존 capsule 충돌 검사 (`PerformPhysicsAssetCollision`)는 그대로 재사용 가능 — `FrameToCheck.BoneTransforms`를 채울 출처만 바뀜

### Phase 4 — 프로파일링 (1주)
- 현재 deque 본 트랜스폼 방식 vs 새 방식의 CPU/메모리 비교
- 시나리오: 10~20명 캐릭터, 발사 빈도 5~10Hz
- 이 결과가 포트폴리오 핵심 데이터

---

## 9. 결론 (의견)

**이 작업은 가능하지만 매우 큽니다.** Riot이 "significant engine modifications"라고 한 표현이 정확합니다.

- 엔진 fork가 사실상 필수 (각 노드 클래스에 friend/setter 추가)
- 노드 타입별 state 식별이 끝없는 작업 (커스텀 노드까지 포함하면 더 큼)
- Inertialization, Physics 노드 등 deterministic하지 않은 부분의 처리가 트리키
- Parallel evaluation과의 상호작용이 안정성 측면에서 가장 큰 리스크

**포트폴리오 가치 측면:** 매우 높음. Riot의 시스템을 UE에서 재현했다는 자체가 강한 차별점이고, "엔진 fork + 노드 visitor + parallel sync"는 시니어급 작업으로 인정받음.

**대안 또는 병행:** Phase 1 PoC를 먼저 끝내고, 그 결과(SequencePlayer + StateMachine 한정으로도 동작한다는 증거)를 가지고 포트폴리오 1차 마감 → 이후 노드 커버리지 확장 단계를 별도 챕터로 잡는 것을 권장.

---

## 10. 다음 액션 후보

1. **Phase 1 PoC 시작** — 가장 단순한 AnimBP 만들어서 StateMachine + SequencePlayer 스냅샷 PoC
2. **엔진 fork 결정** — Engine/Source를 직접 수정할지, plugin 모듈에서 unsafe accessor로 우회할지
3. **`FAnimNode_StateMachine`의 friend list 분석** — 이미 어떤 friend가 있는지 확인하고 패치 최소화
4. **현재 LagCompensation 프로파일링 먼저** — 새 시스템 도입 전에 베이스라인 측정 (사용자가 이미 진행 중인 `project_lag_comp_profiler` 작업과 연계 가능)
