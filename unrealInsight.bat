@echo off
setlocal

:: ============================================================
:: [경로 설정]
:: 1. 언리얼 에디터 (인사이트와 같은 폴더에 있다고 가정)
set UE_EDITOR="C:\UE_Source\5_5_4\Engine\Binaries\Win64\UnrealEditor.exe"

:: 2. 언리얼 인사이트 뷰어
set UE_INSIGHTS="C:\UE_Source\5_5_4\Engine\Binaries\Win64\UnrealInsights.exe"

:: 3. 프로젝트 파일 (.uproject)
:: 폴더명이 Navi이므로 파일명도 Navi.uproject라고 가정했습니다.
set UPROJECT="C:\Users\binij\Documents\UnrealProjects\Navi\Navi.uproject"
:: ============================================================

echo.
echo ============================================================
echo   [Navi Project] 프로파일링 모드 선택
echo ============================================================
echo.
echo   1. 로딩 속도 분석 (Load Time)
echo      - 에셋 로딩 지연, 파일 I/O 병목 확인
echo.
echo   2. 메모리 최적화 (Memory)
echo      - 메모리 누수, LLM(Low Level Memory) 추적
echo.
echo   3. 프레임 드랍/CPU 최적화 (CPU / Frame)
echo      - 병목 함수 찾기, 스레드 지연 확인 (-statnamedevents 포함)
echo.
echo   4. GPU 최적화 (GPU)
echo      - 렌더링 패스별 소요 시간, GPU 병목 확인
echo.
echo   5. [전체] 모든 데이터 수집 (All-in-One)
echo      - 1~4번 모두 포함 (데이터가 커질 수 있음)
echo.
echo ============================================================
set /p mode="실행할 모드 번호를 입력하세요 (1-5): "

:: 먼저 인사이트 뷰어를 실행합니다 (이미 켜져 있어도 괜찮습니다)
start "" %UE_INSIGHTS%

:: 선택에 따른 트레이스 채널 설정
if "%mode%"=="1" goto LOADTIME
if "%mode%"=="2" goto MEMORY
if "%mode%"=="3" goto CPU
if "%mode%"=="4" goto GPU
if "%mode%"=="5" goto ALL
goto END

:LOADTIME
echo [로딩 속도 분석] 모드로 시작합니다...
set TRACE_CMD=-trace=loadtime,file,cpu
goto RUN

:MEMORY
echo [메모리 최적화] 모드로 시작합니다...
set TRACE_CMD=-trace=memory
goto RUN

:CPU
echo [CPU/프레임 분석] 모드로 시작합니다...
:: -statnamedevents: 구체적인 함수 이름을 남겨주어 CPU 병목 찾기에 필수
set TRACE_CMD=-trace=default,cpu,bookmarks -statnamedevents
goto RUN

:GPU
echo [GPU 최적화] 모드로 시작합니다...
set TRACE_CMD=-trace=gpu,frame,cpu
goto RUN

:ALL
echo [전체 통합 분석] 모드로 시작합니다...
set TRACE_CMD=-trace=default,gpu,memory,loadtime,file -statnamedevents
goto RUN

:RUN
:: 게임을 Standalone 모드로 실행 (-game)
start "" %UE_EDITOR% %UPROJECT% -game %TRACE_CMD%
exit

:END
echo 잘못된 번호입니다.
pause