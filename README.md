# Navi Project README

## Project Overview (프로젝트 개요)

This project is a team-based shooter game developed using Unreal Engine 5 and based on the Lyra Starter Game framework. It features custom gameplay mechanics, characters, and maps, and utilizes AWS GameLift for dedicated server management.

이 프로젝트는 언리얼 엔진 5와 Lyra Starter Game 프레임워크를 기반으로 개발된 팀 기반 슈팅 게임입니다. 커스텀 게임플레이 메카닉, 캐릭터, 맵을 특징으로 하며, AWS GameLift를 사용하여 데디케이티드 서버를 관리합니다.

## Source Code Structure (소스 코드 구조)

The source code is organized into several modules within the `Source` directory.

소스 코드는 `Source` 디렉토리 내의 여러 모듈로 구성되어 있습니다.

*   `Source/LyraGame`: The main game module that contains the core gameplay logic. It extends the Lyra framework with custom gameplay features.
    *   `Source/LyraGame`: 핵심 게임플레이 로직을 포함하는 메인 게임 모듈입니다. Lyra 프레임워크를 확장하여 커스텀 게임플레이 기능을 구현합니다.
*   `Source/DedicatedServers`: Code specific to running dedicated servers for the game.
    *   `Source/DedicatedServers`: 게임의 데디케이티드 서버 실행과 관련된 코드입니다.
*   `Source/LyraEditor`: Editor-specific code, including custom asset types, editor utilities, and validation tools.
    *   `Source/LyraEditor`: 커스텀 에셋 타입, 에디터 유틸리티, 검증 툴 등 에디터 관련 코드를 포함합니다.

## Plugin Structure (플러그인 구조)

The project uses a modular approach with plugins, primarily through Unreal Engine's Game Features system.

이 프로젝트는 플러그인을 사용한 모듈식 접근 방식을 채택하고 있으며, 주로 언리얼 엔진의 Game Features 시스템을 활용합니다.

*   **Custom Game Features (커스텀 게임 기능)**:
    *   `Plugins/GameFeatures/NaviShooterCore`: The core custom gameplay plugin. It introduces new systems like the `CredsSystem` and uses a data-driven approach for weapon stats via `DataTables`.
        *   `Plugins/GameFeatures/NaviShooterCore`: 핵심 커스텀 게임플레이 플러그인입니다. `CredsSystem`과 같은 새로운 시스템을 도입하고, `DataTable`을 통해 무기 스탯을 데이터 기반으로 관리합니다.
    *   `Plugins/GameFeatures/NaviAgentSelect`: Manages the agent/character selection screen and logic.
        *   `Plugins/GameFeatures/NaviAgentSelect`: 에이전트/캐릭터 선택 화면 및 로직을 관리합니다.
    *   `Plugins/GameFeatures/NaviMaps`: Contains the custom maps for the game.
        *   `Plugins/GameFeatures/NaviMaps`: 게임의 커스텀 맵을 포함합니다.

*   **Backend (백엔드)**:
    *   `Plugins/GameLiftPlugin`: Integrates AWS GameLift for hosting and managing dedicated game servers.
        *   `Plugins/GameLiftPlugin`: 데디케이티드 게임 서버의 호스팅 및 관리를 위해 AWS GameLift를 통합합니다.

*   **Standard Lyra Plugins (기본 Lyra 플러그인)**:
    *   The project also includes standard Lyra plugins like `ShooterCore`, `CommonGame`, `CommonUser`, and `GameSettings`, which provide the foundational framework for the game.
        *   이 프로젝트는 또한 `ShooterCore`, `CommonGame`, `CommonUser`, `GameSettings`와 같은 표준 Lyra 플러그인을 포함하며, 이는 게임의 기본 프레임워크를 제공합니다.

## Class Structure (클래스 구조)

The project extends several key classes from the Lyra framework to implement its custom logic.

이 프로젝트는 커스텀 로직을 구현하기 위해 Lyra 프레임워크의 여러 핵심 클래스를 확장합니다.

*   **Gameplay Ability System (GAS)**: The project heavily utilizes GAS for abilities, effects, and attributes. Custom abilities are implemented by inheriting from `LyraGameplayAbility`.
    *   **Gameplay Ability System (GAS)**: 어빌리티, 이펙트, 어트리뷰트에 GAS를 광범위하게 활용합니다. 커스텀 어빌리티는 `LyraGameplayAbility`를 상속하여 구현됩니다.
*   **Character and Pawn**: `LyraCharacter` and `LyraPawn` are extended to create custom characters with specific abilities and attributes.
    *   **Character and Pawn**: `LyraCharacter`와 `LyraPawn`을 확장하여 특정 능력과 속성을 가진 커스텀 캐릭터를 생성합니다.
*   **Data-Driven Design**: The project uses a data-driven approach, for instance, by using `UNaviInventoryFragment_SetStatsFromDataTable` to define weapon statistics in `DataTables`, allowing for easy iteration by designers.
    *   **Data-Driven Design**: `DataTables`에서 무기 통계를 정의하기 위해 `UNaviInventoryFragment_SetStatsFromDataTable`을 사용하는 등 데이터 기반 접근 방식을 사용하여 디자이너가 쉽게 반복 작업을 수행할 수 있도록 합니다.
