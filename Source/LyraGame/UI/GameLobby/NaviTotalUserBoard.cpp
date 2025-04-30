// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviTotalUserBoard.h"
#include "NaviTeamUserBoard.h"
#include "LyraLogChannels.h"
#include "Components/VerticalBox.h"
#include "GameModes/GameLobby/NaviLobbyGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Lobby/LobbyState.h"

#include "NaviUserInfoRow.h"

#include "Player/LyraPlayerState.h"


void UNaviTotalUserBoard::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    //@Todo: OnConstruct로 위치 변경?

    ANaviLobbyGameState* NaviLobbyGameState = GetWorld()->GetGameState<ANaviLobbyGameState>();
    if (!IsValid(NaviLobbyGameState)) return;
    // AddExistingPlayers();
    if (IsValid(NaviLobbyGameState->LobbyState))
    {
        OnLobbyStateInitialized( NaviLobbyGameState->LobbyState);
    }
    else
    {
        NaviLobbyGameState->OnLobbyStateInitialized.AddDynamic(this, &UNaviTotalUserBoard::OnLobbyStateInitialized);
    }
}

void UNaviTotalUserBoard::NativeConstruct()
{
    Super::NativeConstruct();
    ANaviLobbyGameState* NaviLobbyGameState = GetWorld()->GetGameState<ANaviLobbyGameState>();
    if (!IsValid(NaviLobbyGameState)) return;
    AddExistingPlayers();
}

void UNaviTotalUserBoard::OnLobbyStateInitialized(ALobbyState* LobbyState)
{
    if (!IsValid(LobbyState)) return;
    
    LobbyState->OnPlayerInfoRemoved.AddDynamic(this, &UNaviTotalUserBoard::OnPlayerRemoved);
}

void UNaviTotalUserBoard::AddExistingPlayers()
{
    AGameStateBase* GameState = UGameplayStatics::GetGameState(this);
    if (!IsValid(GameState)) return;
    TArray<APlayerState*> PlayerStates = GameState->PlayerArray;
    for (APlayerState* PlayerState : PlayerStates)
    {
        ALyraPlayerState* LayraPlayerState = CastChecked<ALyraPlayerState>(PlayerState);
        FString UserName = LayraPlayerState->GetPlayerUserName();
        int32 TeamId = LayraPlayerState->GetTeamId();
        AddUserInfoRow(TeamId, UserName);
    }
}
void UNaviTotalUserBoard::OnPlayerRemoved(const FLobbyPlayerInfo& PlayerInfo)
{
    RemoveUserInfoRow(PlayerInfo.Username);
}
void UNaviTotalUserBoard::AddUserInfoRow(int32 TeamID, const FString& UserName)
{
    if (UserName.IsEmpty()) return; 
    
    UNaviTeamUserBoard* TargetBoard = nullptr;
    if (TeamID == 1)
    {
        TargetBoard = First_TeamUserBoard;
    }
    else if (TeamID == 2)
    {
        TargetBoard = Second_TeamUserBoard;
    }
    else
    {
        UE_LOG(LogLyraDedicatedServer, Warning, TEXT("UNaviTwoTeamsUserBoard::AddUserInfoRow: Invalid TeamID (%d) provided for User %s."), TeamID, *UserName);
        return; 
    }
    bool bAlreadyExist = false;
    int TeamId = -1;
    if (!IsValid(TargetBoard)) return;
    FindUserInfoRow(UserName, bAlreadyExist, TeamId);
    if (!bAlreadyExist)
    {
        TargetBoard->CreateAndAddUserInfoRow(UserName);
    }
}

UNaviUserInfoRow* UNaviTotalUserBoard::FindUserInfoRow(const FString& UserName, bool& OutbSuccess, int32& OutTeamID)
{
    UNaviUserInfoRow* UserInfoRow = First_TeamUserBoard->FindUserInfoRow(UserName);
    if (IsValid(UserInfoRow))
    {
        OutbSuccess = true;
        OutTeamID = 1;
        return UserInfoRow;
    }
    UserInfoRow = Second_TeamUserBoard->FindUserInfoRow(UserName);
    if (IsValid(UserInfoRow))
    {
        OutbSuccess = true;
        OutTeamID = 2;
        return UserInfoRow;
    }
    
    OutbSuccess = false;
    OutTeamID = -1;
    return nullptr;
}

void UNaviTotalUserBoard::RemoveUserInfoRow(const FString& UserName)
{
    int32 TeamID = -1;
    bool bSuccess = false;
    UNaviUserInfoRow* UserInfoRow = FindUserInfoRow(UserName, bSuccess, TeamID);
    if (bSuccess)
    {
        if (TeamID == 1)
        {
            First_TeamUserBoard->RemoveUserInfoRow(UserName);   
        }
        else if (TeamID == 2)
        {
            Second_TeamUserBoard->RemoveUserInfoRow(UserName);   
        }
    }
}

// 플레이어를 다른 팀으로 이동시키는 함수
void UNaviTotalUserBoard::MoveUserInfoRow(int32 NewTeamID, const FString& UserName)
{
    if (UserName.IsEmpty()) return;
    
    UNaviTeamUserBoard* OldTeamBoard = nullptr;
    UNaviTeamUserBoard* NewTeamBoard = nullptr;
    
    if (NewTeamID == 1)
    {
        OldTeamBoard = Second_TeamUserBoard; 
        NewTeamBoard = First_TeamUserBoard;  
    }
    else if (NewTeamID == 2)
    {
        OldTeamBoard = First_TeamUserBoard; 
        NewTeamBoard = Second_TeamUserBoard; 
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UNaviTwoTeamsUserBoard::UserMoveToOtherTeam: Invalid NewTeamID (%d) provided for User %s."), NewTeamID, *UserName);
        return; 
    }
    if (OldTeamBoard && NewTeamBoard)
    {
        OldTeamBoard->RemoveUserInfoRow(UserName);
        NewTeamBoard->CreateAndAddUserInfoRow(UserName);
        UE_LOG(LogTemp, Log, TEXT("UNaviTwoTeamsUserBoard: Moved user %s to Team %d."), *UserName, NewTeamID);
    }
    else
    {
        // 어느 보드가 null인지 구체적으로 로그 남기기
        if (!OldTeamBoard)
        {
            UE_LOG(LogTemp, Error, TEXT("UNaviTwoTeamsUserBoard::UserMoveToOtherTeam: OldTeamBoard (opposite of TeamID %d) is null!"), NewTeamID);
        }
        if (!NewTeamBoard)
        {
             UE_LOG(LogTemp, Error, TEXT("UNaviTwoTeamsUserBoard::UserMoveToOtherTeam: NewTeamBoard (TeamID %d) is null!"), NewTeamID);
        }
    }
}

void UNaviTotalUserBoard::ClearAllTeamUserBoard() const
{
    check(First_TeamUserBoard);
    check(Second_TeamUserBoard);
    First_TeamUserBoard->UserInformations_VerticalBox->ClearChildren();
    Second_TeamUserBoard->UserInformations_VerticalBox->ClearChildren();
}

