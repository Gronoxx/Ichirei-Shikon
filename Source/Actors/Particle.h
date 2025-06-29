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

private:
    float mLength;
    float mDeathTimer;

    DrawSpriteComponent *mDrawComponent;
    RigidBodyComponent *mRigidBodyComponent;
    CircleColliderComponent *mCircleColliderComponent;
};
