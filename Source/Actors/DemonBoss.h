#pragma once
#include "Actor.h"
#include "FlyingDemon.h"

class DrawSpriteComponent;

class DemonBoss : public Actor {
public:
    explicit DemonBoss(Game *game, float attackCooldown = 9.0f, float vulnerableCooldown = 4.0f,
                       float moveSpeed = 400.0f);

    void OnUpdate(float deltaTime) override;

    void LoadAnimationsFromFile(const std::string &filePath);

    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;

    void Kill() override;
    void Hurt() override;

private:
    enum class State {
        Moving,
        Vulnerable,
        Waiting
    };

    void UpdateMoving(float deltaTime);

    void UpdateWaiting(float deltaTime);

    void StartAttack();

    void SpawnMinions();

    void ManageAnimations();

    bool IsSamuraiOnLeft() const;

    void FaceAwayFromSamurai();

    void Jump();

    bool CanJump() const;

    State mCurrentState;
    float mAttackCooldown;
    float mVulnerableCooldown;
    float mAttackTimer;
    float mMoveSpeed;
    float mJumpForce;
    float mJumpCooldown;
    float mJumpTimer;
    int mHealth;
    bool mIsGrounded;
    bool mIsFacingLeft;

    RigidBodyComponent *mRigidBodyComponent;
    DrawSpriteComponent *mDrawComponent;
    AABBColliderComponent *mColliderComponent;
};
