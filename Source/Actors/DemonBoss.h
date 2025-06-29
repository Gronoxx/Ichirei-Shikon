#pragma once
#include "Actor.h"
#include "FlyingDemon.h"

class DemonBoss : public Actor
{
public:
    explicit DemonBoss(Game* game, float attackCooldown = 10.0f, float moveSpeed = 300.0f);

    void OnUpdate(float deltaTime) override;
    void LoadAnimationsFromFile(const std::string& filePath);
    void Kill() override;

private:
    enum class State {
        Moving,
        Attacking,
        Vulnerable
    };

    void UpdateMoving(float deltaTime);
    void UpdateAttacking(float deltaTime);
    void StartAttack();
    void SpawnMinions();
    void ManageAnimations();
    bool IsSamuraiOnLeft() const;
    void FaceAwayFromSamurai();
    void JumpAwayFromSamurai();
    bool CanJump() const;

    State mCurrentState;
    float mAttackCooldown;
    float mAttackTimer;
    float mMoveSpeed;
    float mJumpForce;
    float mJumpCooldown;
    float mJumpTimer;
    int mHealth;
    bool mIsGrounded;
    bool mIsFacingLeft;
    
    class RigidBodyComponent* mRigidBodyComponent;
    class DrawSpriteComponent* mDrawComponent;
    class AABBColliderComponent* mColliderComponent;
};
