//
// Created by Gustavo  Dias Apolinário  on 09/06/25.
//

#ifndef MUSHROOM_H
#define MUSHROOM_H
#include "Actor.h"


class Mushroom : public Actor {
public:
    const int TIME_EMERGING = 1.0;
    Mushroom(Game * game, Vector2 position, int marioXVelocity);
    void OnUpdate(float deltaTime) override;
    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;
private:
    enum class MushroomState {
        Emerging,
        Moving
    };
    class DrawSpriteComponent* mDrawComponent;
    class RigidBodyComponent* mRigidBodyComponent;
    class AABBColliderComponent* mColliderComponent;
    MushroomState mMushroomState;
    float mTimeEmerging;
    float mMushroomHeight;
    float mForwardSpeed;
};



#endif //MUSHROOM_H
