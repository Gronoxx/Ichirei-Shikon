//
// Created by Lucas N. Ferreira on 03/08/23.
//

#include "FlyingDemon.h"
#include "Block.h"
#include "../Game.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Components/DrawComponents/DrawPolygonComponent.h"
#include "../Json.h"
#include <fstream>
#include <string>

#include "Player.h"
#include "Particle.h"

FlyingDemon::FlyingDemon(Game *game, const Vector2 &targetPosition, float lifetime, const float forwardSpeed)
    : Actor(game)
      , mIsRunning(false)
      , mIsOnPole(false)
      , mIsDying(false)
      , mForwardSpeed(forwardSpeed)
      , mPoleSlideTimer(0.0f)
      , mInWorkingMode(false)
      , mTargetPosition(targetPosition)
      , mArrivalThreshold(10.0f)
      , mTimeToLive(lifetime)
      , mWorkingTime(0.0f)
      , mEntranceTimer(0.5f)
      , mIsFlyingAway(false) {
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f, 5.0f, false);

    // Create and configure DrawAnimatedComponent first
    mDrawComponent = new DrawAnimatedComponent(this, 150);
    mDrawComponent->LoadCharacterAnimations("Assets/Sprites/FlyingDemon/FlyingDemon.json");
    mDrawComponent->SetAnimation("idle");

    // Get the animation size to set the collider
    Vector2 animSize = mDrawComponent->GetAnimationSize("idle");

    mColliderComponent = new AABBColliderComponent(this, 0, 0, animSize.x, animSize.y,
                                                   ColliderLayer::Enemy);

    SetScale(1);
}

void FlyingDemon::OnUpdate(float deltaTime) {
    if (mGame->GetGamePlayState() != Game::GamePlayState::Playing) return;

    if (mIsFlyingAway) {
        if (mPosition.y < -100.0f) {
            SetState(ActorState::Destroy);
        }
    } else if (!mInWorkingMode) {
        MoveToTargetPosition(deltaTime);
    } else {
        // Update working time
        mWorkingTime += deltaTime;

        // Check if it's time to fly away
        if (mWorkingTime >= mTimeToLive) {
            StartFlyingAway();
        } else {
            UpdateWorkingMode(deltaTime);
        }
    }

    // Limit FlyingDemon's position to the camera view
    mPosition.x = Math::Max(mPosition.x, mGame->GetCameraPos().x);

    // Kill FlyingDemon if he falls below the screen
    if (mGame->GetGamePlayState() == Game::GamePlayState::Playing && mPosition.y > mGame->GetWindowHeight()) {
        Kill();
    }

    ManageAnimations();
}

void FlyingDemon::MoveToTargetPosition(float deltaTime) {
    // mEntranceTimer += deltaTime;
    //
    // // Calculate direction and distance to the target
    // Vector2 direction = mTargetPosition - mPosition;
    // float distance = direction.Length();
    //
    // // Check if we've arrived at the target position
    // if (distance <= mArrivalThreshold) {
    //     mInWorkingMode = true;
    //     mIsRunning = false;
    //     mRigidBodyComponent->SetVelocity(Vector2::Zero); // Stop completely on arrival
    //     return;
    // }
    //
    // // --- SINUSOIDAL ENTRANCE ---
    // // Normalize the direction vector
    // if (distance > 0) {
    //     direction.Normalize();
    // }
    //
    // // Use a speed boost that decreases as the demon gets closer to the target
    // const float maxRushSpeed = mForwardSpeed * 0.75f;
    // const float minRushSpeed = mForwardSpeed * 0.3f;
    // const float decelerationRadius = 200.0f;
    // float speed;
    // if (distance > decelerationRadius) {
    //     speed = maxRushSpeed;
    // } else {
    //     speed = Math::Lerp(minRushSpeed, maxRushSpeed, distance / decelerationRadius);
    // }
    //
    // // Calculate the sinusoidal velocity component
    // const float sineAmplitude = 40.0f; // How wide the wave is
    // const float sineFrequency = 2.0f; // How fast the wave oscillates
    // Vector2 perpendicularDir(-direction.y, direction.x);
    // float sinusoidalSpeed = sineAmplitude * sineFrequency * cos(mEntranceTimer * sineFrequency);
    //
    // // Combine forward velocity with the sinusoidal velocity
    // Vector2 finalVelocity = (direction * speed) + (perpendicularDir * sinusoidalSpeed);
    // mRigidBodyComponent->SetVelocity(finalVelocity);
    //
    // // Update facing direction based on movement
    // if (std::abs(direction.x) > 0.1f) {
    //     mRotation = (direction.x < 0) ? 0.0f : Math::Pi;
    // }
    //
    // mIsRunning = true;
}

void FlyingDemon::StartFlyingAway() {
    mIsFlyingAway = true;
    mIsAttacking = false;
    mIsRunning = false;
    mRigidBodyComponent->SetVelocity(Vector2::Zero);
    // Make the demon fly upward
    mRigidBodyComponent->SetApplyGravity(false);
    mRigidBodyComponent->ApplyForce(Vector2{0, -10000.0f});
    mColliderComponent->SetEnabled(false);
}

void FlyingDemon::UpdateWorkingMode(float deltaTime) {
    if (mIsDying) {
        if (mDrawComponent->IsAnimationFinished()) {
            SetState(ActorState::Destroy);
        }
        return;
    }

    Player *player = mGame->GetPlayer();
    if (!player) return;

    Vector2 playerPos = player->GetPosition();
    Vector2 targetPos;

    // Define the demon's desired position relative to the player
    const float hoverHeight = Game::TILE_SIZE * 5.0f;
    const float maxHoverHeight = Game::TILE_SIZE * 6.5f;

    // Set target X to align with the player
    targetPos.x = playerPos.x;
    // Set target Y to be above the player, clamped by a max distance
    targetPos.y = playerPos.y - hoverHeight;
    targetPos.y = Math::Max(targetPos.y, playerPos.y - maxHoverHeight);

    // Calculate force to move towards the dynamic target position
    Vector2 directionToTarget = targetPos - mPosition;
    float distanceToTarget = directionToTarget.Length();
    Vector2 moveForce = Vector2::Zero;

    // Apply force only if not already at the target, to prevent jittering
    if (distanceToTarget > 15.0f) {
        directionToTarget.Normalize();
        moveForce = directionToTarget * mForwardSpeed;
    }
    mRigidBodyComponent->ApplyForce(moveForce);
    mIsRunning = moveForce.LengthSq() > 0.1f;

    // Update facing direction based on player's horizontal position
    mRotation = (playerPos.x < mPosition.x) ? 0.0f : Math::Pi;

    // Attack if cooldown is over
    if (!mIsAttacking) {
        mIsAttacking = true;
        mAttackStart = true;
        mAttackTimer = ATTACK_TIME;

        // Calculate spawn position for the projectile
        float facingDirection = (mRotation == 0.0f) ? -1.0f : 1.0f;
        float offsetX = 30.0f * facingDirection;
        Vector2 spawnPos = mPosition + Vector2(offsetX, 25.0f);

        // Calculate direction to player for aiming
        Vector2 directionToPlayer = player->GetPosition() - spawnPos;
        directionToPlayer.Normalize();

        // Create and launch projectile
        const float projectileSpeed = 25000.0f;
        Vector2 projectileForce = directionToPlayer * projectileSpeed;
        auto *projectile = new Particle(mGame, 34.0f, "Assets/Sprites/Particles/projectile.png",
                                        "Mushroow.wav", projectileForce, 2.5f);
        projectile->SetPosition(spawnPos);
        projectile->SetRotation(Math::Atan2(-directionToPlayer.y, directionToPlayer.x));
    }

    // Update attack timer
    if (mIsAttacking) {
        mAttackTimer -= deltaTime;
        if (mAttackTimer <= 0.0f) {
            mIsAttacking = false;
        }
    }

    if (mAttackStart) mAttackStart = false;

    // Reset velocity if not moving and on ground (to prevent sliding)
    if (!mIsRunning && mIsOnGround) {
        mRigidBodyComponent->SetVelocity(Vector2::Zero);
    }
    ManageAnimations();
}

void FlyingDemon::ManageAnimations() {
    if (mIsDying) {
        return;
    }

    if (!mInWorkingMode) {
        mDrawComponent->SetAnimation("spawn");
        mDrawComponent->SetLoop(false);

        if (mDrawComponent->IsAnimationFinished()) {
            mDrawComponent->SetAnimation("idle");
            mDrawComponent->SetLoop(true);

            mIsAttacking = true;
            mAttackStart = true;
            mAttackTimer = 1.0f;
            mInWorkingMode = true;
        }
    } else if (mAttackStart) {
        mDrawComponent->SetAnimation("attack");
        mDrawComponent->SetLoop(true);
    } else if (mIsRunning) {
        mDrawComponent->SetAnimation("flying");
        mDrawComponent->SetLoop(true);
    } else {
        mDrawComponent->SetAnimation("idle");
        mDrawComponent->SetLoop(true);
    }
}

void FlyingDemon::Kill() {
    mIsDying = true;

    if (mDrawComponent) {
        mDrawComponent->SetAnimation("death");
        mDrawComponent->SetLoop(false);
    }

    mRigidBodyComponent->SetEnabled(false);
    mColliderComponent->SetEnabled(false);
}
