#include "Block.h"
#include "Player.h"
#include "../Game.h"
#include "../Actors/Goomba.h"
#include "../Components/DrawComponents/DrawSpriteComponent.h"
#include "../Components/DrawComponents/DrawPolygonComponent.h"
#include "../Components/ColliderComponents/AABBColliderComponent.h"

Block::Block(Game* game, const std::string &texturePath, const bool isStatic)
        :Actor(game)
{
    mDrawSpriteComponent = new DrawSpriteComponent(this, texturePath, Game::TILE_SIZE, Game::TILE_SIZE, 10);
    mColliderComponent = new AABBColliderComponent(this, 0, 0, Game::TILE_SIZE, Game::TILE_SIZE, ColliderLayer::Blocks, isStatic);
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f, 0.0f, false);
}

void Block::OnUpdate(float deltaTime)
{
    // If the block is moving, check if it has reached its original position
    if (mRigidBodyComponent->GetVelocity().y > 0.0f && mPosition.y >= mOriginalPosition.y)
    {
        mPosition.Set(mOriginalPosition.x, mOriginalPosition.y);
        mRigidBodyComponent->SetVelocity(Vector2::Zero);
        mRigidBodyComponent->SetApplyGravity(false);
        mColliderComponent->SetStatic(true);
    }
}

void Block::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other)
{
    // If collide against enemy, apply bump force
    if (other->GetLayer() == ColliderLayer::Enemy)
    {
        Goomba* goomba = static_cast<Goomba*>(other->GetOwner());
        goomba->BumpKill();

        mRigidBodyComponent->SetVelocity(Vector2::NegUnitY * BUMP_FORCE);
        mRigidBodyComponent->SetApplyGravity(true);
        mOriginalPosition.Set(mPosition.x, mPosition.y);
    }

}

void Block::ChangeBlockTexture(const std::string &texturePath) {
    if (!mDrawSpriteComponent)
    {
        return;
    }

    SDL_Texture* usedBlockTexture = mGame->LoadTexture(texturePath);

    if (usedBlockTexture)
    {
        mDrawSpriteComponent->SetTexture(usedBlockTexture,texturePath);
    }
    else
    {
        SDL_Log("Falha ao carregar a textura do bloco %s.", texturePath.c_str());
    }
}
