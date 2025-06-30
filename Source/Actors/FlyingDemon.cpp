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

FlyingDemon::FlyingDemon(Game *game, const Vector2& targetPosition, float lifetime, const float forwardSpeed)
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
      , mIsFlyingAway(false) {
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f, 5.0f, false);
    mColliderComponent = new AABBColliderComponent(this, 0, 0, Game::TILE_SIZE - 4.0f, Game::TILE_SIZE,
                                                   ColliderLayer::Enemy);

    mDrawComponent = new DrawAnimatedComponent(this, 150);

    mDrawComponent->LoadCharacterAnimations("Assets/Sprites/FlyingDemon/FlyingDemon.json");

    // Define a animação inicial
    mDrawComponent->SetAnimation("idle");
    SetScale(1);
}

void FlyingDemon::OnProcessInput(const uint8_t *state) {
    return;
}

void FlyingDemon::OnHandleKeyPress(const int key, const bool isPressed) {
    return;
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
    // Calculate direction to target
    Vector2 direction = mTargetPosition - mPosition;
    float distance = direction.Length();
    
    // Check if we've reached the target position
    if (distance <= mArrivalThreshold) {
        mInWorkingMode = true;
        mIsRunning = false;

        mRigidBodyComponent->SetVelocity(Vector2::Zero);

        return;
    }
    
    // Normalize direction and move towards target
    if (distance > 0) {
        direction.Normalize();
    }
    
    // Move towards target
    Vector2 moveForce = direction * mForwardSpeed; // Move slightly slower than max speed
    mRigidBodyComponent->ApplyForce(moveForce);
    
    // Update facing direction based on movement
    if (std::abs(direction.x) > 0.1f) {  // Only update if we're moving horizontally
        mRotation = (direction.x < 0) ? 0.0f : Math::Pi;
    }
    
    mIsRunning = true;
}

void FlyingDemon::StartFlyingAway() {
    mIsFlyingAway = true;
    mIsAttacking = false;
    mIsRunning = false;
    mRigidBodyComponent->SetVelocity(Vector2::Zero);
    // Make the demon fly upward
    mRigidBodyComponent->SetApplyGravity(false);
    mRigidBodyComponent->ApplyForce(Vector2{0, -10000.0f});
}

void FlyingDemon::UpdateWorkingMode(float deltaTime) {
    // Get Mario's position
    Player* mario = mGame->GetMario();
    if (!mario) return;
    
    Vector2 marioPos = mario->GetPosition();
    
    // Only consider horizontal distance for following
    float horizontalDistance = marioPos.x - mPosition.x;
    float directionX = (horizontalDistance > 0) ? 1.0f : -1.0f;
    
    // Update facing direction based on Mario's horizontal position
    mRotation = (marioPos.x < mPosition.x) ? 0.0f : Math::Pi;
    
    // Move horizontally towards Mario if not too close
    float minFollowDistance = 150.0f; // Minimum distance to keep from Mario

    if (std::abs(horizontalDistance) > minFollowDistance) {
        Vector2 moveForce = Vector2(directionX * mForwardSpeed, 0);
        mRigidBodyComponent->ApplyForce(moveForce);
        mIsRunning = true;
    } else {
        mIsRunning = false;

        // Attack if in range and attack cooldown is over
        if (!mIsAttacking) {
            mIsAttacking = true;
            mAttackStart = true;
            mAttackTimer = ATTACK_TIME;
            
            // Calculate spawn position based on facing direction
            float facingDirection = (mRotation == 0.0f) ? -1.0f : 1.0f;
            float offsetX = 20.0f * facingDirection;
            Vector2 spawnPos = mPosition + Vector2(offsetX, 25.0f);
            
            // Create projectile
            auto *projectile = new Particle(mGame, 34.0f, "Assets/Sprites/Particles/projectile.png", 
                                          "Mushroow.wav", Vector2{facingDirection * 20000, 0}, 1.8f);
            projectile->SetPosition(spawnPos);
            projectile->SetRotation(0.0f);
        }
    }
    
    // Update attack timer
    if (mIsAttacking) {
        mAttackTimer -= deltaTime;
        if (mAttackTimer <= 0.0f) {
            mIsAttacking = false;
        }
    }

    if (mAttackStart) mAttackStart = false;

    if (mIsAttacking) {
        mAttackTimer -= deltaTime;
        if (mAttackTimer <= 0.0f) {
            mIsAttacking = false;
        }
    }

    if (!mIsRunning && mIsOnGround) {
        mRigidBodyComponent->SetVelocity(Vector2::Zero);
    }
    ManageAnimations();
}

void FlyingDemon::ManageAnimations() {
    if (mIsDying) {
        mDrawComponent->SetAnimation("Dead");
        mDrawComponent->SetLoop(false);
    } else if (mAttackStart) {
        // TODO 2: This is not working, fix it
        mDrawComponent->SetAnimation("attack");
        mDrawComponent->SetLoop(false);

        if (mDrawComponent->IsAnimationFinished()) {
            mAttackStart = false;
        }
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

    // Disable collider and rigid body
    mRigidBodyComponent->SetEnabled(false);
    mColliderComponent->SetEnabled(false);

    // --------------
    // TODO - PARTE 4
    // --------------

    // TODO 1.: Pare todos os sons com StopAllSounds() e toque o som "Dead.wav".
    mGame->GetAudio()->StopAllSounds();
    mGame->GetAudio()->PlaySound("Dead.wav");
}