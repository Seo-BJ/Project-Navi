# AnimGraph Node Input/Output Engine Source Summary

대상 엔진 소스: `C:\UE_Source\5_5_4`

이 문서는 `AnimGraph_Investigation.md`에서 언급된 AnimGraph 실행 경로와 주요 노드에 대해, 에디터에서 보이는 입력/출력 핀이 어떤 엔진 코드에서 만들어지고 런타임에서 어떤 `FAnimNode_*` 필드/컨텍스트로 연결되는지 정리한 것이다.

## 1. 공통 핀 생성 규칙

### 로컬 포즈 노드

- 출력 핀 생성:
  - `C:\UE_Source\5_5_4\Engine\Source\Editor\AnimGraph\Private\AnimGraphNode_Base.cpp`
  - `UAnimGraphNode_Base::CreateOutputPins()`
  - sink node가 아니면 `EGPD_Output`, `FPoseLink`, 이름 `Pose` 출력 핀을 만든다.
- 입력 포즈 핀 생성:
  - 같은 파일의 `UAnimGraphNode_Base::CreatePinsForPoseLink()`
  - 런타임 구조체에 `UPROPERTY`로 선언된 `FPoseLink` 또는 `TArray<FPoseLink>`가 있으면 입력 핀으로 노출된다.
- 런타임 출력:
  - `FAnimNode_Base::Evaluate_AnyThread(FPoseContext& Output)`
  - 실제 출력은 멤버 변수가 아니라 `FPoseContext::Pose`, `Curve`, `CustomAttributes`에 기록된다.

### 컴포넌트 스페이스 / Skeletal Control 노드

- 출력 핀 생성:
  - `C:\UE_Source\5_5_4\Engine\Source\Editor\AnimGraph\Private\AnimGraphNode_SkeletalControlBase.cpp`
  - `UAnimGraphNode_SkeletalControlBase::CreateOutputPins()`
  - `FComponentSpacePoseLink` 타입의 `Pose` 출력 핀을 만든다.
- 공통 입력 포즈:
  - `C:\UE_Source\5_5_4\Engine\Source\Runtime\AnimGraphRuntime\Public\BoneControllers\AnimNode_SkeletalControlBase.h`
  - `FAnimNode_SkeletalControlBase::ComponentPose`
- 런타임 출력:
  - `FAnimNode_SkeletalControlBase::EvaluateComponentSpace_AnyThread(FComponentSpacePoseContext& Output)`
  - 실제 출력은 `FComponentSpacePoseContext::Pose`에 component-space bone transform으로 기록된다.

### 일반 값 핀 노출

- `C:\UE_Source\5_5_4\Engine\Source\Editor\AnimGraph\Private\AnimBlueprintNodeOptionalPinManager.cpp`
- `FAnimBlueprintNodeOptionalPinManager::GetRecordDefaults()`
- `FPoseLinkBase` 계열은 항상 입력 핀으로 취급된다.
- 값 프로퍼티는 `UPROPERTY(meta=(PinShownByDefault))`, `PinHiddenByDefault`, `AlwaysAsPin`, `NeverAsPin` 메타데이터에 따라 핀으로 노출된다.
- 핀 타입 판정은 `C:\UE_Source\5_5_4\Engine\Source\Editor\AnimGraph\Private\AnimationGraphSchema.cpp`의 `IsLocalSpacePosePin()`, `IsComponentSpacePosePin()`에서 `FPoseLink` / `FComponentSpacePoseLink`로 구분한다.

## 2. 그래프 루트 / Sink 노드

| 그래프/노드 | 에디터 노드 | 런타임 노드 | 입력 | 출력 | 핵심 소스 |
|---|---|---|---|---|---|
| Main AnimGraph Output Pose | `UAnimGraphNode_Root` | `FAnimNode_Root` | `FPoseLink Result` | sink node라 출력 핀 없음. `Evaluate_AnyThread()`에서 `Result.Evaluate(Output)` | `Engine\Source\Runtime\Engine\Classes\Animation\AnimNode_Root.h`, `Private\Animation\AnimNode_Root.cpp`, `Editor\AnimGraph\Private\AnimGraphNode_Root.cpp` |
| State Graph Output Animation Pose | `UAnimGraphNode_StateResult` | `FAnimNode_StateResult : FAnimNode_Root` | 상속된 `FPoseLink Result` | sink node라 출력 핀 없음 | `Engine\Source\Runtime\Engine\Public\Animation\AnimNode_StateResult.h`, `Editor\AnimGraph\Private\AnimGraphNode_StateResult.cpp` |
| Transition Rule Result | `UAnimGraphNode_TransitionResult` | `FAnimNode_TransitionResult` | `bool bCanEnterTransition`, `PinShownByDefault` | sink node라 포즈 출력 없음. StateMachine이 bool 결과만 읽음 | `Engine\Source\Runtime\Engine\Classes\Animation\AnimNode_TransitionResult.h`, `Private\Animation\AnimNode_StateMachine.cpp`, `Editor\AnimGraph\Private\AnimGraphNode_TransitionResult.cpp` |

## 3. 주요 로컬 포즈 노드

| 노드 | 입력 핀 / 입력 상태 | 출력 | Update/Evaluate 소스 | 리와인드 스냅샷 관점 |
|---|---|---|---|---|
| `FAnimNode_StateMachine` | 직접 노출되는 외부 pose 입력은 없음. 컴파일 시 생성되는 `TArray<FPoseLink> StatePoseLinks`가 각 state graph 결과에 연결됨. 설정값: `MaxTransitionsPerFrame`, `bSkipFirstUpdateTransition`, `bReinitializeOnBecomingRelevant` 등 | 기본 `UAnimGraphNode_Base`의 local `Pose` 출력. `Evaluate_AnyThread()`가 현재 state 또는 transition blend 결과를 `FPoseContext`에 기록 | `Engine\Source\Runtime\Engine\Classes\Animation\AnimNode_StateMachine.h`, `Private\Animation\AnimNode_StateMachine.cpp` | `CurrentState`, `ElapsedTime`, `ActiveTransitionArray`, `QueuedTransitionEvents`, `bFirstUpdate`, `StatePoseLinks` 연결 상태가 핵심 |
| `FAnimNode_SequencePlayer` | 값 핀: `Sequence`, `PlayRate`, `PlayRateBasis`, `StartPosition`, `bLoopAnimation`, sync 관련 설정. 대부분 `PinHiddenByDefault`, standalone은 동일 필드가 런타임 멤버 | local `Pose` 출력. `Evaluate_AnyThread()`가 `UAnimSequenceBase::GetAnimationPose()` 결과를 `Output`에 기록 | `Engine\Source\Runtime\Engine\Classes\Animation\AnimNode_SequencePlayer.h`, `Private\Animation\AnimNode_SequencePlayer.cpp`; editor: `Editor\AnimGraph\Private\AnimGraphNode_SequencePlayer.cpp` | `InternalTimeAccumulator`, `MarkerTickRecord`, `PlayRateScaleBiasClampState`, sync group 상태가 핵심 |
| `FAnimNode_BlendSpacePlayer` | 값 핀: `X`, `Y`는 `PinShownByDefault`; `BlendSpace`, `PlayRate`, `bLoop`, `StartPosition`, `bResetPlayTimeWhenBlendSpaceChanges`는 대체로 숨김 기본 | local `Pose` 출력. `UpdateAssetPlayer()`가 sample cache/time을 갱신하고 `Evaluate_AnyThread()`가 `BlendSpace->GetAnimationPose()` 호출 | `Engine\Source\Runtime\AnimGraphRuntime\Public\AnimNodes\AnimNode_BlendSpacePlayer.h`, `Private\AnimNodes\AnimNode_BlendSpacePlayer.cpp`; editor: `Editor\AnimGraph\Private\AnimGraphNode_BlendSpaceBase.cpp`, `AnimGraphNode_BlendSpacePlayer.cpp` | `InternalTimeAccumulator`, `BlendSampleDataCache`, `MarkerTickRecord`, `PreviousBlendSpace`, `X/Y` 입력값 |
| `FAnimNode_Inertialization` | pose 입력: `FPoseLink Source`. 값 입력: `DefaultBlendProfile`, `FilteredCurves`, `FilteredBones`, `bResetOnBecomingRelevant`, `bForwardRequestsThroughSkippedCachedPoseNodes` | local `Pose` 출력. `Source.Evaluate(Output)` 후 inertialization 상태가 active면 `ApplyTo()`로 `Output` 수정 | `Engine\Source\Runtime\Engine\Classes\Animation\AnimNode_Inertialization.h`, `Private\Animation\AnimNode_Inertialization.cpp`; editor: `Editor\AnimGraph\Public\AnimGraphNode_Inertialization.h` | `PrevPoseSnapshot`, `CurrPoseSnapshot`, `RequestQueue`, `InertializationState`, `InertializationElapsedTime/Duration`, bone/curve diff 배열. 이 노드는 pose급 상태를 들고 있어 메모리 절감 효과를 크게 깎을 수 있음 |
| `FAnimNode_LayeredBoneBlend` | pose 입력: `FPoseLink BasePose`, `TArray<FPoseLink> BlendPoses`; 값 핀: `TArray<float> BlendWeights`는 `PinShownByDefault`; 설정: `LayerSetup`, `BlendMasks`, mesh-space rotation/scale flags | local `Pose` 출력. `Evaluate_AnyThread()`가 base와 각 blend pose를 평가 후 `FAnimationRuntime::BlendPosesPerBoneFilter()` 호출 | `Engine\Source\Runtime\AnimGraphRuntime\Public\AnimNodes\AnimNode_LayeredBoneBlend.h`, `Private\AnimNodes\AnimNode_LayeredBoneBlend.cpp`; editor: `Editor\AnimGraph\Private\AnimGraphNode_LayeredBoneBlend.cpp` | `BlendWeights`, `CurrentBoneBlendWeights`, `DesiredBoneBlendWeights`, `PerBoneBlendWeights` |
| `FAnimNode_ApplyAdditive` / `FAnimNode_ApplyMeshSpaceAdditive` | pose 입력: `FPoseLink Base`, `FPoseLink Additive`; 값 핀: `Alpha`, `bAlphaBoolEnabled`, `AlphaCurveName` 등이 `AlphaInputType`에 따라 노출/숨김 | local `Pose` 출력. base pose에 additive pose를 누적 | `Engine\Source\Runtime\AnimGraphRuntime\Public\AnimNodes\AnimNode_ApplyAdditive.h`, `Private\AnimNodes\AnimNode_ApplyAdditive.cpp`; editor: `Editor\AnimGraph\Private\AnimGraphNode_ApplyAdditive.cpp`, `AnimGraphNode_ApplyMeshSpaceAdditive.cpp` | `ActualAlpha`, `AlphaBoolBlend`, `AlphaScaleBiasClamp` |

## 4. Component-Space / Physics 계열 노드

| 노드 | 입력 핀 / 입력 상태 | 출력 | Update/Evaluate 소스 | 리와인드 스냅샷 관점 |
|---|---|---|---|---|
| `FAnimNode_SkeletalControlBase` 공통 | `FComponentSpacePoseLink ComponentPose`; 값 핀: `Alpha`, `bAlphaBoolEnabled`, `AlphaCurveName`, `LODThreshold` 등 | component-space `Pose` 출력. 하위 클래스가 `EvaluateSkeletalControl_AnyThread()`에서 `FBoneTransform`을 생성하고 base가 적용 | `Engine\Source\Runtime\AnimGraphRuntime\Public\BoneControllers\AnimNode_SkeletalControlBase.h`, `Private\BoneControllers\AnimNode_SkeletalControlBase.cpp` | 하위 물리/IK 노드의 공통 입력. `ActualAlpha`와 alpha blend state 포함 |
| `FAnimNode_RigidBody` | 공통 `ComponentPose`; 값 입력: `OverridePhysicsAsset`, `ExternalForce`는 `PinShownByDefault`, `OverrideWorldGravity`, `ComponentLinearAccScale`, `ComponentLinearVelScale`, `SimulationSpace`, `bEnableWorldGeometry` 등 | component-space `Pose` 출력. physics simulation 결과를 `OutBoneTransforms`로 반환 | `Engine\Source\Runtime\AnimGraphRuntime\Public\BoneControllers\AnimNode_RigidBody.h`, `Private\BoneControllers\AnimNode_RigidBody.cpp`; editor: `Editor\AnimGraph\Private\AnimGraphNode_RigidBody.cpp` | `PhysicsSimulation`, simulation space transforms, previous component/world transform, pending forces, world collision state까지 필요. deterministic rewind 대상으로는 위험 |
| `FAnimNode_AnimDynamics` | 공통 `ComponentPose`; 값 입력: `ExternalForce`는 `PinShownByDefault`, `SimulationSpace`, spring/damping/constraint/body definition 관련 다수 설정 | component-space `Pose` 출력. `FAnimPhys::PhysicsUpdate()` 후 bone transform 적용 | `Engine\Source\Runtime\AnimGraphRuntime\Public\BoneControllers\AnimNode_AnimDynamics.h`, `Private\BoneControllers\AnimNode_AnimDynamics.cpp`; editor: `Editor\AnimGraph\Private\AnimGraphNode_AnimDynamics.cpp` | `SimBodies`, `LinearLimits`, `AngularLimits`, `Springs`, `PreviousComponentLinearVelocity`, `LastSimSpace` 등 물리 상태가 커서 snapshot 비용이 큼 |

## 5. Montage는 AnimGraph 노드가 아님

`AnimGraph_Investigation.md`의 결론처럼 montage는 일반 `FAnimNode_*` pose node가 아니라 `UAnimInstance` / `FAnimInstanceProxy` 쪽 asset player 및 montage instance 경로에서 관리된다.

- 대표 소스:
  - `C:\UE_Source\5_5_4\Engine\Source\Runtime\Engine\Classes\Animation\AnimInstance.h`
  - `C:\UE_Source\5_5_4\Engine\Source\Runtime\Engine\Private\Animation\AnimInstance.cpp`
  - `C:\UE_Source\5_5_4\Engine\Source\Runtime\Engine\Private\Animation\AnimInstanceProxy.cpp`
- 따라서 AnimGraph node visitor만으로는 montage 재생 위치, section, blend state를 충분히 복원할 수 없다.

## 6. 리와인드 구현 시 우선순위

1. 최소 PoC:
   - `FAnimNode_StateMachine`
   - `FAnimNode_SequencePlayer`
   - `FAnimNode_Root` / `FAnimNode_StateResult`
2. 다음 확장:
   - `FAnimNode_BlendSpacePlayer`
   - `FAnimNode_LayeredBoneBlend`
   - alpha 계열 blend/additive 노드
3. 별도 정책 필요:
   - `FAnimNode_Inertialization`: pose snapshot급 상태 보유
   - `FAnimNode_RigidBody`, `FAnimNode_AnimDynamics`: physics simulation state 때문에 과거 evaluate 재현성이 낮음
   - Montage: AnimGraph node snapshot과 별도 경로 필요

