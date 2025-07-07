//
// Created by Lucas N. Ferreira on 08/09/23.
//

#pragma once
#include "Actor.h"

class FlyingDemon : public Actor
{
public:
    explicit FlyingDemon(Game* game, const Vector2& targetPosition, float lifetime, float forwardSpeed = 700.0f);

    void Kill() override;

    void LoadAnimationsFromFile(const std::string& filePath);
    bool isPlayerAttacking() {return  mIsAttacking;};

    void OnUpdate(float deltaTime) override;

private:
    static const int POLE_SLIDE_TIME = 1; // Time in seconds to slide down the pole
    const float ATTACK_TIME = 3.5f;

    void ManageAnimations();
    void UpdateWorkingMode(float deltaTime);
    void MoveToTargetPosition(float deltaTime);
    void StartFlyingAway();

    float mForwardSpeed;
    float mPoleSlideTimer;
    float mAttackTimer;
    bool mIsRunning;
    bool mIsOnPole;
    bool mIsDying;
    bool mIsAttacking;
    bool mAttackStart;
    bool mInWorkingMode;
    Vector2 mTargetPosition;
    float mArrivalThreshold;  // How close we need to be to consider target reached
    float mTimeToLive;        // How long the demon stays in working mode before flying away
    float mWorkingTime;       // Tracks how long the demon has been in working mode
    float mEntranceTimer;     // Timer for the sinusoidal entrance movement
    bool mIsFlyingAway;       // True when the demon is in the process of flying away

    class RigidBodyComponent* mRigidBodyComponent;
    class DrawAnimatedComponent* mDrawComponent;
    class AABBColliderComponent* mColliderComponent;
};