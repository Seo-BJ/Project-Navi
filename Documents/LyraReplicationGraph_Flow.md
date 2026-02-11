# Lyra Replication Graph Deep Dive: System Flow & Engine Integration

이 문서는 Unreal Engine의 `ReplicationGraph` 프레임워크와 `Lyra`의 구현체가 엔진 레벨에서 어떻게 상호작용하는지, 그리고 시스템의 시작부터 종료(Cleanup)까지의 전 과정을 상세히 분석합니다.

---

## 1. 핵심 클래스 정의 (Core Classes)

### A. ULyraReplicationGraph
게임 전체의 레플리케이션 전략을 제어하는 중앙 허브입니다.

#### 주요 변수 (Key Variables)
*   **`GridNode`**: `UReplicationGraphNode_GridSpatialization2D*`. 거리 기반 복제를 처리하는 메인 2D 그리드 노드입니다.
*   **`AlwaysRelevantGlobalNode`**: `UReplicationGraphNode_ActorList*`. 모든 연결에 항상 복제되어야 하는 액터들을 관리합니다.
*   **`AlwaysRelevantNode`**: `UReplicationGraphNode_AlwaysRelevant_WithPending*`. 팀 기반 액터 및 지연된 액터 등록을 처리합니다.
*   **`TeamConnectionListMap`**: `FTeamConnectionListMap`. 팀 ID별로 연결(Connection) 목록을 관리하는 커스텀 맵입니다.

#### 주요 메소드 (Key Methods)
*   **`InitGlobalActorClassSettings()`**: `DefaultGame.ini` 설정을 로드하여 클래스별 노드 매핑 정책을 초기화합니다.
*   **`RouteAddNetworkActorToNodes()`**: 액터의 클래스 정책에 따라 적절한 노드(Grid, Team 등)에 액터를 배치합니다.
*   **`HandlePendingActorsAndTeamRequests()`**: `PrepareForReplication` 단계에서 호출되며, 팀 변경이나 지연된 액터 배치를 일괄 처리합니다.
*   **`SetTeamForPlayerController()`**: 플레이어의 팀 정보를 갱신하고 관련 노드들을 재배치하도록 요청합니다.

### B. ULyraConnectionManager
개별 클라이언트의 상태를 관리하고 맞춤형 복제 목록을 생성합니다.

#### 주요 변수 (Key Variables)
*   **`TeamConnectionNode`**: `ULyraReplicationGraphNode_AlwaysRelevant_ForTeam*`. 해당 클라이언트의 팀 기반 복제 로직을 수행하는 전용 노드입니다.
*   **`Team`**: `int32`. 클라이언트가 현재 소속된 팀 ID입니다.
*   **`Pawn`**: `TWeakObjectPtr<APawn>`. 해당 연결이 소유하고 있는 캐릭터 객체입니다.

#### 주요 메소드 (Key Methods)
*   **`GetConnectionArrayForTeam(int32 Team)`**: 특정 팀에 속한 모든 연결 리스트를 반환합니다.
*   **`GetVisibleConnectionArrayForNonTeam(const APawn* Pawn, int32 Team)`**: (FTeamConnectionListMap) 물리 레이캐스트를 통해 가시성이 확보된 적군 연결 리스트를 반환합니다. (현재 병목 지점)

---

## 2. 시스템 부팅 및 초기화 (Initialization)

Replication Graph는 `UNetDriver`에 의해 생명주기가 관리됩니다.

### A. 엔진 초기화 호출 스택 (Callstack)
1.  **`UEngine::Browse` / `UWorld::SetGameMode`**: 새로운 맵 로드 및 게임 세션 시작.
2.  **`UNetDriver::InitBase`**: 네트워킹 레이어 초기화.
3.  **`UReplicationDriver::CreateReplicationDriver`**: 
    - `DefaultEngine.ini`의 `ReplicationDriverClassName` 설정 확인.
    - `ULyraReplicationGraph` 인스턴스 생성.
4.  **`UReplicationGraph::PostInit`**: 엔진 레벨 초기화 루틴 시작.
5.  **`ULyraReplicationGraph::InitGlobalActorClassSettings()`**:
    - `FClassReplicationInfo` 테이블 구축. 
    - `ClassRepNodePolicies` 맵에 액터별 `EClassRepNodeMapping` 저장.
6.  **`ULyraReplicationGraph::InitGlobalGraphNodes()`**:
    - `GridNode`, `AlwaysRelevantNode`, `PlayerStateNode` 생성 및 전역 등록.

---

## 2. 노드 분석 및 분류 (Node Analysis & Classification)

Replication Graph의 노드는 크게 **전역 노드(Global Nodes)**와 **연결 노드(Connection Nodes)**로 나뉩니다.

### A. 전역 노드 vs 연결 노드 비교

| 비교 항목 | 전역 노드 (Global Nodes) | 연결 노드 (Connection Nodes) |
| :--- | :--- | :--- |
| **소유 및 수량** | `ReplicationGraph`가 소유 (시스템당 1개) | `UNetConnection`마다 생성 (클라이언트당 1개) |
| **데이터 공유** | 모든 클라이언트가 동일한 노드/결과를 공유 | 특정 클라이언트에게만 해당되는 데이터 처리 |
| **메모리** | 서버 메모리에 하나만 상주하여 효율적 | 클라이언트 수에 비례하여 메모리 사용량 증가 |
| **주요 목적** | 거리 기반 컬링, 전역 상태 액터 복제 | 본인 캐릭터, 소유한 액터, 팀 정보 등 개별 데이터 |
| **대표 예시** | `GridNode`, `AlwaysRelevantGlobalNode` | `AlwaysRelevantForConnectionNode`, `TeamNode` |

### B. 세부 노드 분석
#### 1. 전역 노드 (Global Nodes)
*   **`UReplicationGraphNode_GridSpatialization2D`**: 2D 그리드로 맵을 나누어 액터 위치에 따라 복제 결정.
*   **`UReplicationGraphNode_ActorList`**: 모든 플레이어에게 항상 복제되어야 하는 전역 액터 관리.
*   **`ULyraReplicationGraphNode_PlayerStateFrequencyLimiter`**: 수십 명의 PlayerState를 라운드 로빈 방식으로 분할 복제.
*   **`UReplicationGraphNode_AlwaysRelevant_WithPending`**: `Prepare` 단계에서 팀 변경 등 지연된 요청을 처리하는 트리거.

#### 2. 연결 노드 (Connection Nodes)
*   **`ULyraReplicationGraphNode_AlwaysRelevant_ForConnection`**: 본인의 Pawn, PlayerState 등 전용 액터 관리.
*   **`ULyraReplicationGraphNode_AlwaysRelevant_ForTeam`**: 팀원 정보 상시 복제 + 적 팀 레이캐스트 가시성 복제. (현재 병목 지점)

---

## 3. 레플리케이션 프레임 루프 (Detailed Execution Flow)

매 프레임 `UNetDriver::ServerReplicateActors`에 의해 수행되는 엔진-그래프 간 상세 흐름입니다.

### [Step 1] 전역 준비 (PrepareForReplication)
**Callstack:** `UNetDriver::ServerReplicateActors` -> `UReplicationGraph::ReplicateActors` -> `PrepareForReplication`
*   **엔진**: 모든 전역 노드와 연결 노드의 `PrepareForReplication`을 루프 돌며 호출.
*   **Lyra**: `AlwaysRelevantNode`가 트리거되어 `HandlePendingActorsAndTeamRequests()` 실행. 
    - 여기서 팀 변경(`PendingTeamRequests`)과 신규 액터(`PendingConnectionActors`)를 각 노드에 재배치.

### [Step 2] 리스트 수집 (GatherActorLists)
**Callstack:** `UReplicationGraph::ReplicateActors` -> 연결별 루프 -> `GatherActorListsForConnection`
*   **엔진**: `ConnectionManager->ConnectionGraphNodes` 루프 실행.
*   **Lyra (Team Node)**: `ULyraReplicationGraphNode_AlwaysRelevant_ForTeam::GatherActorListsForConnection` 호출.
    - **핵심 로직**: `GetVisibleConnectionArrayForNonTeam`을 통해 적군 가시성 체크.
    - **병목 원인**: 이 루프 내부에서 `World->LineTraceSingleByChannel` 동기식 물리 쿼리 발생.

### [Step 3] 우선순위 결정 및 전송 (Prioritize & Send)
**Callstack:** `UReplicationGraph::ReplicateActors` -> `ProcessPrioritizedActors`
*   **엔진**: 수집된 `FActorRepListRefView` 리스트들을 병합.
*   **엔진**: 각 액터의 `NetUpdateFrequency`와 거리/중요도를 계산하여 이번 프레임에 보낼 최종 액터 선별 후 패킷 전송.

---

## 4. 종료 및 정리 로직 (Shutdown & Cleanup)

클라이언트 종료 또는 게임 종료 시 시스템이 자원을 해제하는 과정입니다.

### A. 클라이언트 접속 종료 (Connection Closed)
**Callstack:** `UNetConnection::Close` -> `UNetDriver::RemoveClientConnection` -> `ULyraReplicationGraph::RemoveClientConnection`
1.  **팀 데이터 제거**: `TeamConnectionListMap.RemoveConnectionFromTeam` 호출하여 해당 클라이언트를 팀 맵에서 삭제.
2.  **노드 파괴**: 해당 연결에 종속되었던 `AlwaysRelevantConnectionNode`와 `TeamConnectionNode` 제거.
3.  **부모 호출**: `Super::RemoveClientConnection`을 통해 엔진 레벨의 연결 데이터 정리.

### B. 게임 종료 / 레벨 변경 (World Cleanup)
**Callstack:** `UWorld::CleanupWorld` -> `UNetDriver::FinishDestroy` -> `ULyraReplicationGraph::ResetGameWorldState`
1.  **전역 맵 클리어**: `AlwaysRelevantStreamingLevelActors`, `TeamConnectionListMap`, `PendingTeamRequests` 등 모든 컨테이너 `Empty()`.
2.  **노드 상태 초기화**: 
    - 각 연결의 `AlwaysRelevantConnectionNode->ResetGameWorldState()` 호출.
    - 각 연결의 `TeamNode->NotifyResetAllNetworkActors()` 호출.
3.  **가비지 컬렉션**: `ReplicationGraph` 인스턴스 자체가 파괴되면서 내부의 `TObjectPtr`로 관리되던 노드들이 GC 대상이 됨.

---

## 5. 결론 및 개선 포인트 (Conclusion)

1.  **호출 주기 문제**: `Gather`는 "복제 시도"마다 발생하지만 가시성은 그렇게 자주 바뀔 필요가 없음.
2.  **물리 스톨(Stall)**: `Step 2`의 동기식 레이캐스트가 메인 스레드를 멈추게 함.
3.  **해결책**: 가시성 계산을 `Step 1 (Prepare)`로 옮기고, 업데이트 빈도를 낮추어 `Gather`에서는 캐싱된 결과만 읽도록 수정해야 함.