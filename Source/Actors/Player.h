//
// Created by Lucas N. Ferreira on 08/09/23.
//

#pragma once
#include <SDL_events.h>

#include "Actor.h"
#include "Slash.h"

class Player : public Actor
{
public:
    explicit Player(Game* game, float forwardSpeed = 300.0f, float jumpSpeed = -750.0f);

    //void OnProcessInput(const Uint8* keyState) override;
    void OnUpdate(float deltaTime) override;
    //void OnHandleKeyPress(const int key, const bool isPressed) override;
    void HandleInput(const uint8_t* state, const SDL_Event* event);

    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;

    void Kill() override;
    void Hurt() override;

    void Win() const;

    void LoadAnimationsFromFile(const std::string& filePath);
    bool isPlayerAttacking() {return  mIsAttacking;};

private:
    static const int POLE_SLIDE_TIME = 1; // Time in seconds to slide down the pole
    const float ATTACK_TIME = 0.25f;
    const float ROLL_TOTAL_DISTANCE = 300.0f;
    const float ROLL_DURATION = 0.3f; // segundos, depende da duração da animação "roll"
    const float HURT_DURATION = 0.5f;
    float mRollTimer = 0.0f;
    const int SPLASH_WIDTH = 106;

    void ManageAnimations();
    void KnockBack() const;

    float mForwardSpeed;
    float mJumpSpeed;
    float mPoleSlideTimer;
    float mAttackTimer;
    float mIdleToRunTimer;
    float mHurtTimer;
    bool mIsRunning;
    bool mIsOnPole;
    bool mIsDying;
    bool mIsAttacking;
    bool mIsRolling;
    bool mIsFalling;
    bool mIsJumping;
    bool mIsHurt;
    bool mIsStartingToRun;
    bool mHasStartedIdleToRun;
    Slash* mSlash{};

    class RigidBodyComponent* mRigidBodyComponent;
    class DrawAnimatedComponent* mDrawComponent;
    class AABBColliderComponent* mColliderComponent;
};
