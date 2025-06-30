//
// Created by olive on 30/06/2025.
//

#include "Trigger.h"
#include "../Game.h"


Trigger::Trigger(Game *game) : Actor(game)
{
    mColliderComponent = new AABBColliderComponent(this, 0, 0, Game::TILE_SIZE, Game::TILE_SIZE, ColliderLayer::EndLevel, true);
}

void Trigger::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (other->GetLayer() == ColliderLayer::Player)
    {
        mGame->SetGamePlayState(Game::GamePlayState::LevelComplete);
    }
}

void Trigger::OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (other->GetLayer() == ColliderLayer::Player)
    {
        mGame->SetGamePlayState(Game::GamePlayState::LevelComplete);
    }
}


