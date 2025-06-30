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
    mCircleColliderComponent = new CircleColliderComponent(this, mLength);
}

void Particle::OnUpdate(float deltaTime) {
    mDeathTimer -= deltaTime;

    if (mDeathTimer <= 0) {
        SetState(ActorState::Destroy);
        return;
    }

    Player *player = GetGame()->GetMario();
    auto *playerCollider = player->GetComponent<AABBColliderComponent>();
    if (mCircleColliderComponent->Intersect(*playerCollider)) {
        player->Hurt();
        SetState(ActorState::Destroy);
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
