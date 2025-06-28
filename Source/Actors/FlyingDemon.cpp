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

#include "Mario.h"
#include "Particle.h"

FlyingDemon::FlyingDemon(Game *game, const float forwardSpeed)
    : Actor(game)
      , mIsRunning(false)
      , mIsOnPole(false)
      , mIsDying(false)
      , mForwardSpeed(forwardSpeed)
      , mPoleSlideTimer(0.0f) {
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f, 5.0f, false);
    mColliderComponent = new AABBColliderComponent(this, 0, 0, Game::TILE_SIZE - 4.0f, Game::TILE_SIZE,
                                                   ColliderLayer::Player);

    mDrawComponent = new DrawAnimatedComponent(this, 150);

    mDrawComponent->LoadCharacterAnimations("Assets/Sprites/FlyingDemon/FlyingDemon.json");

    // Define a animação inicial
    mDrawComponent->SetAnimation("idle");
    SetScale(1);
}

void FlyingDemon::OnProcessInput(const uint8_t *state) {
    // No longer processing player input
    return;
}

void FlyingDemon::OnHandleKeyPress(const int key, const bool isPressed) {
    // No longer processing player input
    return;
}

void FlyingDemon::OnUpdate(float deltaTime) {
    if (mGame->GetGamePlayState() != Game::GamePlayState::Playing) return;
    
    // Get Mario's position
    Mario* mario = mGame->GetMario();
    if (!mario) return;
    
    Vector2 marioPos = mario->GetPosition();
    
    // Only consider horizontal distance for following
    float horizontalDistance = marioPos.x - mPosition.x;
    float directionX = (horizontalDistance > 0) ? 1.0f : -1.0f;
    
    // Update facing direction based on Mario's horizontal position
    mRotation = (marioPos.x < mPosition.x) ? 0.0f : Math::Pi;
    
    // Move horizontally towards Mario if not too close
    float minFollowDistance = 150.0f; // Increased minimum distance to keep from Mario

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

    // Limit FlyingDemon's position to the camera view
    mPosition.x = Math::Max(mPosition.x, mGame->GetCameraPos().x);

    // Kill FlyingDemon if he falls below the screen
    if (mGame->GetGamePlayState() == Game::GamePlayState::Playing && mPosition.y > mGame->GetWindowHeight()) {
        Kill();
    }

    if (!mIsRunning && mIsOnGround) {
        mRigidBodyComponent->SetVelocity(Vector2::Zero);
    }

    if (mIsAttacking) {
        mAttackTimer -= deltaTime;
        if (mAttackTimer <= 0.0f) {
            mIsAttacking = false;
        }
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
    mGame->SetGamePlayState(Game::GamePlayState::GameOver);
    mDrawComponent->SetAnimation("Dead");

    // Disable collider and rigid body
    mRigidBodyComponent->SetEnabled(false);
    mColliderComponent->SetEnabled(false);

    // --------------
    // TODO - PARTE 4
    // --------------

    // TODO 1.: Pare todos os sons com StopAllSounds() e toque o som "Dead.wav".
    mGame->GetAudio()->StopAllSounds();
    mGame->GetAudio()->PlaySound("Dead.wav");

    mGame->ResetGameScene(3.5f); // Reset the game scene after 3 seconds
}

void FlyingDemon::OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other) {
    if (other->GetLayer() == ColliderLayer::Enemy) {
        Kill();
    }
}

void FlyingDemon::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other) {
 if (other->GetLayer() == ColliderLayer::Blocks) {
        if (!mIsOnGround) {
            // --------------
            // TODO - PARTE 4
            // --------------

            // TODO 1.: Toque o som "Bump.wav"
            mGame->GetAudio()->PlaySound("Bump.wav");

            // Cast actor to Block to call OnBump
            auto *block = dynamic_cast<Block *>(other->GetOwner());
            block->OnBump();
        }
    }
}
