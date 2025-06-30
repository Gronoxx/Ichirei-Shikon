#pragma once
#include "Actor.h"
#include "../GameMath.h"


class Slash : public Actor
{
public:
    explicit Slash(class Game* game, const Vector2& position, float duration, float rotation, const Vector2& velocity = Vector2::Zero);


    void OnUpdate(float deltaTime) override;
    void OnHorizontalCollision(float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(float minOverlap, AABBColliderComponent* other) override;

private:
    float mLifeTime; // Tempo restante de vida
    Vector2 mVelocity;

    class DrawAnimatedComponent* mDrawComponent;
    class AABBColliderComponent* mColliderComponent;
};
