//
// Created by Lucas N. Ferreira on 08/09/23.
//

#pragma once
#include "Actor.h"

class FlyingDemon : public Actor
{
public:
    explicit FlyingDemon(Game* game, float forwardSpeed = 700.0f);

    void OnProcessInput(const Uint8* keyState) override;
    void OnUpdate(float deltaTime) override;
    void OnHandleKeyPress(const int key, const bool isPressed) override;

    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;

    void Kill() override;

    void LoadAnimationsFromFile(const std::string& filePath);
    bool isPlayerAttacking() {return  mIsAttacking;};

private:
    static const int POLE_SLIDE_TIME = 1; // Time in seconds to slide down the pole
    const float ATTACK_TIME = 3.5f;

    void ManageAnimations();

    float mForwardSpeed;
    float mPoleSlideTimer;
    float mAttackTimer;
    bool mIsRunning;
    bool mIsOnPole;
    bool mIsDying;
    bool mIsAttacking;
    bool mAttackStart;

    class RigidBodyComponent* mRigidBodyComponent;
    class DrawAnimatedComponent* mDrawComponent;
    class AABBColliderComponent* mColliderComponent;
};