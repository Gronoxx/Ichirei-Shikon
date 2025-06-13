//
// Created by Gustavo  Dias Apolinário  on 09/06/25.
//

#include "Points.h"

#include "../Game.h"
#include "../HUD.h"
#include "../Components/DrawComponents/DrawSpriteComponent.h"

Points::Points(Game* game, Vector2 position)
        : Actor(game),
        mFlipTime(TIME_GOING_UP)
{
    SetPosition(position);

    mDrawComponent = new DrawSpriteComponent(this,
    "../Assets/Sprites/Collectables/200Points.png",
    16,8);
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f, 0.0f, false);
    SetScale(1.6f);
    mGame->SetScore((mGame->GetScore() + 200) % 1000000);
    mGame->GetHUD()->SetScore();

    FlyingEffect();
}

void Points::FlyingEffect() {
    mRigidBodyComponent->SetVelocity(Vector2::NegUnitY * UP_FORCE);
}

void Points::OnUpdate(float deltaTime) {
    mFlipTime -= deltaTime;
    if (mFlipTime <= 0)
    {
        this->SetState(ActorState::Destroy);
    }
}