# Lyra Lag Compensation System (Server-Side Rewind)

이 문서는 프로젝트 Navi에 구현된 렉 보상(Lag Compensation) 시스템의 아키텍처와 동작 원리를 상세히 설명합니다.

---

## 1. 개요 (Overview)

슈팅 게임에서 클라이언트는 자신의 화면에서 적을 조준하고 발사하지만, 서버는 지연 시간(Latency) 때문에 적이 이미 다른 위치에 있다고 판단할 수 있습니다. 이를 해결하기 위해 **서버 사이드 리와인드(Server-Side Rewind)** 기법을 사용하여, 사격 시점의 과거 시간을 되감아 서버에서 판정합니다.

### 핵심 목표
*   **"What you see is what you hit"**: 클라이언트 화면에서 맞았다면 서버에서도 맞은 것으로 판정.
*   **보안성**: 판정 로직은 서버에서 수행하여 변조 방지.
*   **정밀도**: 본(Bone) 단위의 정밀한 판정 지원.

---

## 2. 시스템 구성 요소 (Components)

### A. ULyraTimeSyncComponent (시간 동기화)
*   **역할**: 클라이언트가 서버의 현재 시간을 정확히 추정할 수 있도록 동기화.
*   **동작**:
    1. 클라이언트가 `ServerRequestServerTime` 요청.
    2. 서버가 응답하여 `ClientServerDelta`(서버-클라이언트 시간 차) 계산.
    3. `SingleTripTime`(지연 시간)을 산출하여 정밀한 서버 시간 계산에 활용.

### B. ULyraLagCompensationComponent (기반 클래스)
*   모든 렉 보상 컴포넌트의 추상 기반 클래스.
*   `ServerSideRewind` 인터페이스 제공.

### C. 노드 유형별 구현체
| 구현체 | 방식 | 특징 |
| :--- | :--- | :--- |
| **SkeletalMesh Component** | **Bone Pose Rewind** | 실제 애니메이션 포즈와 Physics Asset을 활용한 초정밀 판정. |
| **Box Component** | **Hitbox Rewind** | 미리 정의된 `UBoxComponent`들의 위치/회전을 저장하여 빠르게 판정. |

---

## 3. 동작 프로세스 (Execution Flow)

사격 발생 시점부터 판정 완료까지의 단계별 흐름입니다.

### [Step 1] 클라이언트 사격 (Client)
*   `ULyraGameplayAbility_RangedWeapon` 실행.
*   `ULyraTimeSyncComponent->GetServerTime()`을 호출하여 **현재 서버 시간**을 획득.
*   서버에 `ServerFire(HitTime)` RPC 요청을 보냄.

### [Step 2] 히스토리 저장 (Server - Every Tick)
*   서버는 매 틱마다 복제되는 모든 캐릭터의 위치/포즈 정보를 `FrameHistory`(`TDoubleLinkedList`)에 저장.
*   보통 최근 0.5초 ~ 1초 사이의 데이터를 보관.

### [Step 3] 서버 사이드 리와인드 (Server - On Request)
1.  **시간 검색**: 전달받은 `HitTime`이 히스토리의 어느 지점인지 탐색.
2.  **보간(Interpolation)**: 정확히 일치하는 데이터가 없을 경우, 앞뒤 프레임을 선형 보간하여 가상의 `FFramePackage` 생성.
3.  **포즈 되감기**:
    *   **Mesh 방식**: 해당 시점의 모든 본 트랜스폼을 적용.
    *   **Box 방식**: 해당 시점의 히트박스 위치/회전을 복구.
4.  **판정(ConfirmHit)**: 되감아진 상태에서 `LineTrace`를 수행하여 피격 여부 최종 확인.
5.  **복구**: 판정 직후 캐릭터를 다시 현재 시점의 포즈로 복구.

---

## 4. 상세 로직 분석

### A. 보간 (Interpolation)
두 프레임 $F_{older}$와 $F_{younger}$ 사이의 시간 $T_{hit}$에 대한 보간 공식:
$$Alpha = (T_{hit} - T_{older}) / (T_{younger} - T_{older})$$
모든 Transform(Location, Rotation, Scale)에 대해 `Lerp` 및 `Slerp`를 수행합니다.

### B. 피격 채널
*   **Trace Channel**: `LagCompensation_TraceChannel_HitBox` (ECC_GameTraceChannel6)
*   일반적인 가시성 채널과 분리되어 렉 보상 판정 전용으로 사용됩니다.

---

## 5. 디버깅 및 시각화 (Debugging)

시스템의 정확도를 검증하기 위해 다음 콘솔 변수를 제공합니다.

*   `lyra.Weapon.DrawMeshLagCompensation 1`: 서버에서 판정한 되감기 위치를 시각화.
*   `lyra.Weapon.DrawMeshLagCompensationDuration 3.0`: 디버그 라인 유지 시간.

### 테스트 액터
*   `ALyraLagCompensationTestCharacter`: 패트롤 경로를 따라 움직이며 렉 보상 테스트를 돕는 특수 캐릭터.
*   다양한 테스트 모드(Normal, Jump, Crouch) 지원.

---

## 6. 결론 및 주의사항

1.  **성능 비용**: 매 틱 본 트랜스폼을 저장하는 과정은 CPU 비용이 발생하므로 히스토리 길이를 적절히 조절해야 함.
2.  **데이터 무결성**: 클라이언트가 보낸 `HitTime`이 너무 과거거나 미래일 경우 서버에서 검증하여 거절(Reject)하는 로직이 필수적임.
3.  **외삽(Extrapolation)**: 네트워크 지연이 극심하여 히스토리 범위를 벗어날 경우 `ExtrapolateByTwoFrames`를 통해 위치를 추측하기도 함.
