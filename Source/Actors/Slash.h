#pragma once
#include "Actor.h"

class Slash : public Actor
{
public:
    explicit Slash(class Game* game, const Vector2& position,const float TotalLifeTime, float rotation, const Vector2& velocity);

    void OnUpdate(float deltaTime) override;
    void OnHorizontalCollision(float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(float minOverlap, AABBColliderComponent* other) override;
    void HandleCollision(AABBColliderComponent* other);

private:
    float mLifeTime; // Tempo restante de vida
    Vector2 mVelocity;

    std::set<Actor*> mHitActors;

    class DrawAnimatedComponent* mDrawComponent;
    class AABBColliderComponent* mColliderComponent;
};