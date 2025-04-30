// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviTeamUserBoard.h"
#include "NaviUserInfoRow.h"


#include "Components/VerticalBox.h"
#include "Blueprint/UserWidget.h"        
#include "UObject/ConstructorHelpers.h"  
#include "Components/Widget.h"           
#include "Player/LyraPlayerController.h"
#include "Player/LyraPlayerState.h"

void UNaviTeamUserBoard::CreateAndAddUserInfoRow(const FString& UserName)
{
    if (UserName.IsEmpty() || !UserInfoRowClass || !IsValid(UserInformations_VerticalBox)) return;
    
    UNaviUserInfoRow* NewUserInfoRow = CreateWidget<UNaviUserInfoRow>(this, UserInfoRowClass);
    if (!NewUserInfoRow) return;
    NewUserInfoRow->SetUserNameText(*UserName); 
    UserInformations_VerticalBox->AddChildToVerticalBox(NewUserInfoRow);
    /*
    int32 NewTeamIndex = UserInformations_VerticalBox->GetAllChildren().Num();

    ALyraPlayerController* LyraPlayerController = CastChecked<ALyraPlayerController>(GetOwningPlayer());
    ALyraPlayerState* LyraPlayerState = LyraPlayerController->GetPlayerState<ALyraPlayerState>();
    // Todo : Local에만 Server RPC를 호출하기 위핸 확인. Team 변경을 위해 Server RPC를 호출할 때 로직 추가 필요
    // 이동할 수 있는지, 그때 Team Index를 설정하는 등...
    if (UserName == LyraPlayerState->GetPlayerUserName())
    {
        LyraPlayerController->ServerRequestSetIndexInTeam(NewTeamIndex);
    }
    */

}

UNaviUserInfoRow* UNaviTeamUserBoard::FindUserInfoRow(const FString& UserName)
{
    if (UserName.IsEmpty()) return nullptr;
    check(UserInformations_VerticalBox);
    
    TArray<UWidget*> Children = UserInformations_VerticalBox->GetAllChildren();
    for (int32 i = 0; i < Children.Num(); i++)
    {
        UNaviUserInfoRow* UserInfoRow = Cast<UNaviUserInfoRow>(Children[i]);
        if (IsValid(UserInfoRow))
        {
            FString ChildUserName = UserInfoRow->GetInUserName();
            if (UserName.Equals(ChildUserName))
            {
            return UserInfoRow;
            }
        }
    }
    return nullptr;
}

void UNaviTeamUserBoard::RemoveUserInfoRow(const FString& UserName)
{
    if (UserName.IsEmpty()) return;
    check(UserInformations_VerticalBox);
    
    UNaviUserInfoRow* OutUserInfoRow = FindUserInfoRow(UserName);
    if (IsValid(OutUserInfoRow))
    {
        UserInformations_VerticalBox->RemoveChild(OutUserInfoRow);
    }
}
