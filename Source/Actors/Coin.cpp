//
// Created by Gustavo  Dias Apolinário  on 09/06/25.
//

#include "Coin.h"

#include "Points.h"
#include "../Game.h"
#include "../HUD.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"

Coin::Coin(Game* game, Vector2 position)
        : Actor(game),
        mFlipTime(TIME_FLIPPING)
{
    SetPosition(position);
    mOriginalPosition = GetPosition();

    mDrawComponent = new DrawAnimatedComponent(this,
    "../Assets/Sprites/Collectables/Coin.png",
    "../Assets/Sprites/Collectables/Coin.json");
    mDrawComponent->AddAnimation("Flip", {0,1,2,3});
    mDrawComponent->SetAnimFPS(10.0f);
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f, 0.0f, true);
    SetScale(1.6f);
    mGame->GetAudio()->PlaySound("Coin.wav");
    FlipAnimation();

}

void Coin::FlipAnimation() {
    mDrawComponent->SetAnimation("Flip");
    mIsFliping = true;
    mRigidBodyComponent->SetVelocity(Vector2::NegUnitY * FLIP_FORCE);
    mRigidBodyComponent->SetApplyGravity(true);
}

void Coin::OnUpdate(float deltaTime) {
    if (mRigidBodyComponent->GetVelocity().y > 0.0f && mPosition.y >= mOriginalPosition.y)
    {
        new Points(mGame, mPosition + Vector2(COIN_WIDTH/2.0 - POINTS_WIDTH/2.0,5));
        mGame->SetNumberOfCoinsCollected((mGame->GetNumberOfCoinsCollected() + 1) % 100);
        mGame->GetHUD()->SetCoins();
        this->SetState(ActorState::Destroy);
    }
}





