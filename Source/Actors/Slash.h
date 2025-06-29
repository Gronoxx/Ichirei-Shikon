#pragma once
#include "Actor.h"

class Slash : public Actor
{
public:
    explicit Slash(class Game* game, const Vector2& position, const float TotalLifeTime, float rotation);

    void OnUpdate(float deltaTime) override;
    void OnHorizontalCollision(float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(float minOverlap, AABBColliderComponent* other) override;

private:
    float mLifeTime; // Tempo restante de vida

    class DrawAnimatedComponent* mDrawComponent;
    class AABBColliderComponent* mColliderComponent;
};
