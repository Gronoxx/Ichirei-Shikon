//
// Created by Gustavo  Dias Apolinário  on 09/06/25.
//

#include "Mushroom.h"

#include "../Game.h"
#include "../Components/DrawComponents/DrawSpriteComponent.h"

Mushroom::Mushroom(Game* game,Vector2 position, int marioXVelocity):
    Actor(game)
{
    SetPosition(Vector2(position.x,position.y));
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f);
    mRigidBodyComponent->SetEnabled(false);
    mColliderComponent = new AABBColliderComponent(this, 0, 0, Game::TILE_SIZE - 4.0f,Game::TILE_SIZE,
                                                   ColliderLayer::Mushroom);
    mColliderComponent->SetEnabled(false);
    mDrawComponent = new DrawSpriteComponent(this, "Assets/Sprites/Collectables/Mushroom.png", 32, 32);
    mGame->GetAudio()->PlaySound("Mushroom.wav");
    marioXVelocity >= 0 ? mForwardSpeed = 100.0f : mForwardSpeed = -100.0f;
    mMushroomState = MushroomState::Emerging;
    mTimeEmerging = TIME_EMERGING;
    mMushroomHeight = 0;
}



void Mushroom::OnUpdate(float deltatime) {
    if (GetPosition().y > GetGame()->GetWindowHeight())
    {
        this->SetState(ActorState::Destroy);
    }

    if (mMushroomState == MushroomState::Emerging) {
    mTimeEmerging -= deltatime;
    if (mTimeEmerging <=0) {
        mMushroomState = MushroomState::Moving;
        SetPosition(mPosition - Vector2(0,Game::TILE_SIZE));
        mColliderComponent->SetEnabled(true);
        mRigidBodyComponent->SetEnabled(true);
        mDrawComponent->ResetCustomDstRect();
        mDrawComponent->ResetCustomSourceRect();
        mRigidBodyComponent->SetVelocity(Vector2(mForwardSpeed, 0.0f));
    }
    else {
        float timePassedEmerging = TIME_EMERGING - mTimeEmerging;
        mMushroomHeight = (Game::TILE_SIZE * (timePassedEmerging));

        mDrawComponent->SetSourceRect(0,0,Game::TILE_SIZE,Game::TILE_SIZE);
        mDrawComponent->SetDstRect(mPosition.x,
        mPosition.y - mMushroomHeight
        ,Game::TILE_SIZE,
        mMushroomHeight);
    }

    }
}

void Mushroom::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{
    SDL_Log("Colliding");
    if ((other->GetLayer() == ColliderLayer::Blocks || other->GetLayer() == ColliderLayer::Enemy) || other->GetLayer() == ColliderLayer::Mushroom)
    {
        if (minOverlap > 0) {
            mRigidBodyComponent->SetVelocity(Vector2(-mForwardSpeed, 0.0f));
        }
        else {
            mRigidBodyComponent->SetVelocity(Vector2(mForwardSpeed, 0.0f));
        }
    }

    if (other->GetLayer() == ColliderLayer::Player) {
        this->SetState(ActorState::Destroy);
    }
}
void Mushroom::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (other->GetLayer() == ColliderLayer::Player) {
        this->SetState(ActorState::Destroy);
    }
}

