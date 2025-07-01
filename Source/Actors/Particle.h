#pragma once
#include "Actor.h"

#include "../Components/ColliderComponents/CircleColliderComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/DrawComponents/DrawSpriteComponent.h"

class Particle : public Actor {
public:
    Particle(Game *game, float length, const std::string &spritePath, const std::string &soundPath,
             const Vector2 &initialForce, const float mass, float deathTimer = 1.0f);

    void OnUpdate(float deltaTime) override;
    void OnHorizontalCollision(float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(float minOverlap, AABBColliderComponent* other) override;
    void HandleCollision(AABBColliderComponent* other);
    void Parry(Vector2 parryOrigin);

private:
    float mLength;
    float mDeathTimer;
    bool mParried;

    DrawSpriteComponent *mDrawComponent;
    RigidBodyComponent *mRigidBodyComponent;
    AABBColliderComponent *mAABBColliderComponent;
};
