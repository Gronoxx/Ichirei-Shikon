//
// Created by Lucas N. Ferreira on 12/09/23.
//

#include "Particle.h"

#include "Player.h"
#include "../Game.h"

Particle::Particle(class Game *game, float length,
                   const std::string &spritePath,
                   const std::string &soundPath,
                   const Vector2 &initialForce,
                   const float mass,
                   float deathTimer)
    : Actor(game)
	  , mParried(false)
      , mLength(length)
      , mDeathTimer(deathTimer) {
    std::vector<Vector2> vertices = {
        Vector2(-length / 2, 0),
        Vector2(length / 2, 0)
    };

    mDrawComponent = new DrawSpriteComponent(this, spritePath, mLength, mLength);
    // mGame->GetAudio()->PlaySound(soundPath);

    mRigidBodyComponent = new RigidBodyComponent(this, mass);
    mRigidBodyComponent->ApplyForce(initialForce);
    mAABBColliderComponent = new AABBColliderComponent(this, 0, 0, Game::TILE_SIZE/2, Game::TILE_SIZE/2, ColliderLayer::EnemyProjectile);
}

void Particle::OnUpdate(float deltaTime) {
    mDeathTimer -= deltaTime;

    if (mDeathTimer <= 0) {
        SetState(ActorState::Destroy);
        return;
    }

    // TODO: This is not setting the rotation correctly
    // Update rotation based on velocity direction
    if (mRigidBodyComponent) {
        Vector2 velocity = mRigidBodyComponent->GetVelocity();
        if (velocity.LengthSq() > 0.1f) {  // Only update if moving
            mRotation = atan2f(velocity.y, velocity.x);
        }
    }
}

void Particle::OnHorizontalCollision(float minOverlap, AABBColliderComponent* other)
{
    HandleCollision(other);
}

void Particle::OnVerticalCollision(float minOverlap, AABBColliderComponent* other)
{
    HandleCollision(other);
}

void Particle::HandleCollision(AABBColliderComponent* other)
{
    Actor *a = other->GetOwner();
    Player *p = dynamic_cast<Player*>(a);
    if (p != nullptr && !mParried)
    {
        p->Hurt();
        SetState(ActorState::Destroy);
        return;
    }

    Particle *p2 = dynamic_cast<Particle*>(a);
    {
        Parry(a->GetPosition());
    }
}

void Particle::Parry(Vector2 parryOrigin)
{
    if (mParried) return;

    SDL_Log("Parrying.");
    mGame->GetAudio()->PlaySound("Parry.wav");
    float parryStrength = 50000;
    Vector2 parryDirection = mPosition - parryOrigin;
    mParried = true;

    parryDirection.Normalize();
    mRigidBodyComponent->SetVelocity(parryDirection * parryStrength);
}