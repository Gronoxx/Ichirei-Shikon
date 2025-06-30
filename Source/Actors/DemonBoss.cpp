#include "DemonBoss.h"
#include "../Game.h"
#include "Player.h"
#include "../Random.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Components/ColliderComponents/AABBColliderComponent.h"
#include "../Components/DrawComponents/DrawPolygonComponent.h"

const float JUMP_FORCE = 50000.0f;
const float JUMP_COOLDOWN = 5.0f;
const int INITIAL_HEALTH = 20;

DemonBoss::DemonBoss(Game *game, float attackCooldown, float vulnerableCooldown, float moveSpeed)
    : Actor(game)
      , mCurrentState(State::Moving)
      , mAttackCooldown(attackCooldown)
      , mVulnerableCooldown(vulnerableCooldown)
      , mAttackTimer(attackCooldown/3.0f)
      , mMoveSpeed(moveSpeed)
      , mJumpForce(JUMP_FORCE)
      , mJumpCooldown(JUMP_COOLDOWN)
      , mJumpTimer(JUMP_COOLDOWN)
      , mHealth(INITIAL_HEALTH)
      , mIsGrounded(true)
      , mIsFacingLeft(true) {
    // Set size to 4x4 tiles
    float size = Game::TILE_SIZE * 3.0f;

    // Set up physics
    mRigidBodyComponent = new RigidBodyComponent(this, 2.0f, 10.0f, true);
    mColliderComponent = new AABBColliderComponent(this, 0, 0, size, size - 10, ColliderLayer::Boss);

    // Set up animations
    std::vector<Vector2> vertices;

    mDrawComponent = new DrawSpriteComponent(this, "Assets/Sprites/DemonBoss/DemonBoss.png", size, size - 10);
    SetScale(1.0f);

    mGame->LockCamera();

    // Start with moving animation
    // mDrawComponent->SetAnimation("idle");

    Random::Init();
}

void DemonBoss::OnUpdate(float deltaTime) {
    if (mGame->GetGamePlayState() != Game::GamePlayState::Playing)
        return;

    // Update timers
    if (mJumpTimer > 0.0f) {
        mJumpTimer -= deltaTime;
    }


    // Update state machine
    switch (mCurrentState) {
        case State::Moving:
            UpdateMoving(deltaTime);
        // Check if it's time to attack

            mAttackTimer += deltaTime;
            if (mAttackTimer >= mAttackCooldown) {
                SDL_Log("Change state to vulnerable");
                StartAttack();
            }
            break;

        case State::Vulnerable:
            // Just wait until attack cooldown is over
            mAttackTimer += deltaTime;
            if (mAttackTimer >= mVulnerableCooldown) {
                SDL_Log("Change state to moving");
                mCurrentState = State::Moving;
            }
            break;
    }

    // Update animations
    ManageAnimations();

    // Keep boss on screen
    Vector2 cameraPos = mGame->GetCameraPos();
    float minX = cameraPos.x;
    float maxX = cameraPos.x + mGame->GetWindowWidth() - (Game::TILE_SIZE * 4);
    mPosition.x = Math::Clamp(mPosition.x, minX, maxX);
}

void DemonBoss::UpdateMoving(float deltaTime) {
    Player *mario = mGame->GetMario();
    if (!mario)
        return;

    // Face away from Mario
    FaceAwayFromSamurai();

    // Move away from Mario
    float direction = mIsFacingLeft ? -1.0f : 1.0f;
    Vector2 moveForce = Vector2(direction * mMoveSpeed, 0);
    mRigidBodyComponent->ApplyForce(moveForce);


    if (CanJump() && Random:: GetFloat() < 0.01f) {
        Jump();
    }
}

void DemonBoss::StartAttack() {
    if (mCurrentState != State::Moving)
        return;

    mRigidBodyComponent->SetVelocity(Vector2::Zero);

    SpawnMinions();

    mCurrentState = State::Vulnerable;
    mAttackTimer = 0.0f; // Reset attack timer
}

void DemonBoss::SpawnMinions() {
    SDL_Log("Spawning minions");

    // Get screen bounds
    float screenLeft = mGame->GetCameraPos().x;
    float screenRight = screenLeft + mGame->GetWindowWidth();
    float screenMiddle = screenLeft + mGame->GetWindowWidth() / 2;
    float screenTop = mGame->GetCameraPos().y;

    // Calculate spawn positions just outside the screen
    float spawnY = screenTop;
    float leftSpawnX = screenLeft - Game::TILE_SIZE * 2.0f; // Left of screen
    float rightSpawnX = screenRight + Game::TILE_SIZE * 2.0f; // Right of screen

    // Target positions near the boss
    float targetOffset = Game::TILE_SIZE * 3.0f;

    Vector2 leftTarget =  Vector2(screenMiddle - targetOffset, Game::TILE_SIZE * 3);
    Vector2 rightTarget = Vector2(screenMiddle + targetOffset, Game::TILE_SIZE * 3);

    // Left minion (spawns from left, moves to right of boss)
    auto *leftMinion = new FlyingDemon(mGame, leftTarget, 6.0f, 400.0f);
    leftMinion->SetPosition(Vector2(leftSpawnX, spawnY));

    // Right minion (spawns from right, moves to left of boss)
    auto *rightMinion = new FlyingDemon(mGame, rightTarget, 6.0f, 400.0f);
    rightMinion->SetPosition(Vector2(rightSpawnX, spawnY));
}

void DemonBoss::ManageAnimations() {
}

bool DemonBoss::IsSamuraiOnLeft() const {
    Player *mario = mGame->GetMario();
    if (!mario) return false;

    return mario->GetPosition().x < mPosition.x;
}

void DemonBoss::FaceAwayFromSamurai() {
    bool samuraiOnLeft = IsSamuraiOnLeft();
    mIsFacingLeft = !samuraiOnLeft;
    mRotation = samuraiOnLeft ? Math::Pi : 0.0f;
}

void DemonBoss::Jump() {
    if (!CanJump())
        return;

    Vector2 jumpForce(0.0f, -mJumpForce);

    if (IsSamuraiOnLeft()) {
        jumpForce.x = mJumpForce;
    } else {
        jumpForce.x = -mJumpForce;
    }

    mRigidBodyComponent->ApplyForce(jumpForce);
    mJumpTimer = JUMP_COOLDOWN;
}

bool DemonBoss::CanJump() const {
    return mIsGrounded && mJumpTimer <= 0.0f;
}

void DemonBoss::Kill() {
    // Play death animation/sound
    // mGame->GetAudio()->PlaySound("BossDefeated.wav");

    mGame->UnlockCamera();

    // Destroy the boss
    SetState(ActorState::Destroy);
}

void DemonBoss::Hurt() {
    SDL_Log("Boss hurt life: %d", mHealth);

    mHealth--;
    if (mHealth <= 0) {
        Kill();
    }
}

void DemonBoss::LoadAnimationsFromFile(const std::string &filePath) {
    // // Load animations from JSON file
    // mDrawComponent->LoadAnimations(filePath);
    // mDrawComponent->SetAnimation("idle");
}


