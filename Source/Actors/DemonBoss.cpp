#include "DemonBoss.h"
#include "../Game.h"
#include "Mario.h"
#include "../Random.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Components/ColliderComponents/AABBColliderComponent.h"
#include "../Components/DrawComponents/DrawPolygonComponent.h"

const float JUMP_FORCE = 50000.0f;
const float JUMP_COOLDOWN = 2.0f;
const int INITIAL_HEALTH = 5;

DemonBoss::DemonBoss(Game* game, float attackCooldown, float moveSpeed)
    : Actor(game)
    , mCurrentState(State::Moving)
    , mAttackCooldown(attackCooldown)
    , mAttackTimer(attackCooldown)
    , mMoveSpeed(moveSpeed)
    , mJumpForce(JUMP_FORCE)
    , mJumpCooldown(JUMP_COOLDOWN)
    , mJumpTimer(0.0f)
    , mHealth(INITIAL_HEALTH)
    , mIsGrounded(false)
    , mIsFacingLeft(true)
{
    // Set size to 4x4 tiles
    float size = Game::TILE_SIZE * 3.0f;

    // Set up physics
    mRigidBodyComponent = new RigidBodyComponent(this, 2.0f, 10.0f, true);
    mColliderComponent = new AABBColliderComponent(this, 0, 0, size, size - 10, ColliderLayer::Boss);

    // Set up animations
    std::vector<Vector2> vertices;

    mDrawComponent = new DrawSpriteComponent(this, "Assets/Sprites/DemonBoss/DemonBoss.png", size, size - 10);
    SetScale(1.0f);

    // Start with moving animation
    // mDrawComponent->SetAnimation("idle");

    Random::Init();
}

void DemonBoss::OnUpdate(float deltaTime)
{
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
            mAttackTimer -= deltaTime;
            if (mAttackTimer <= 0.0f) {
                StartAttack();
            }
            break;

        case State::Attacking:
            UpdateAttacking(deltaTime);
            break;

        case State::Vulnerable:
            // Just wait until attack cooldown is over
            mAttackTimer += deltaTime;
            if (mAttackTimer >= mAttackCooldown) {
                mCurrentState = State::Moving;
                mAttackTimer = 0.0f;
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

void DemonBoss::UpdateMoving(float deltaTime)
{
    Mario* mario = mGame->GetMario();
    if (!mario)
        return;

    // Face away from Mario
    FaceAwayFromSamurai();

    // Move away from Mario
    float direction = mIsFacingLeft ? -1.0f : 1.0f;
    Vector2 moveForce = Vector2(direction * mMoveSpeed, 0);
    mRigidBodyComponent->ApplyForce(moveForce);

    // Random chance to jump
    if (CanJump() && Random::GetFloat() < 0.01f) {
        JumpAwayFromSamurai();
    }
}

void DemonBoss::UpdateAttacking(float deltaTime)
{
    // Spawn minions after a short delay
    static float spawnTimer = 0.5f;
    static bool hasSpawned = false;

    spawnTimer -= deltaTime;
    if (spawnTimer <= 0.0f && !hasSpawned) {
        SpawnMinions();
        hasSpawned = true;

        // Go to vulnerable state after attack
        mCurrentState = State::Vulnerable;
        mAttackTimer = 0.0f; // Reset attack timer
    }
}

void DemonBoss::StartAttack()
{
    if (mCurrentState != State::Moving)
        return;

    mCurrentState = State::Attacking;
    mRigidBodyComponent->SetVelocity(Vector2::Zero);

    // Reset attack state
    static_cast<void>(0); // This seems to be a placeholder
}

void DemonBoss::SpawnMinions()
{
    // Spawn two flying demons on either side
    float spawnOffset = Game::TILE_SIZE * 5.0f;

    // Left minion
    auto* leftMinion = new FlyingDemon(mGame,
        mGame->GetCameraPos() + Vector2(-spawnOffset, 6 * Game::TILE_SIZE),
        6.0f,
        400.0f);
    leftMinion->SetPosition(mGame->GetCameraPos() + Vector2(-spawnOffset, 0));

    // Right minion
    auto* rightMinion = new FlyingDemon(mGame,
        mGame->GetCameraPos() + Vector2(spawnOffset, 6 * Game::TILE_SIZE),
        6.0f,
        400.0f);
    rightMinion->SetPosition(mGame->GetCameraPos() + Vector2(-spawnOffset, 0));
}

void DemonBoss::ManageAnimations()
{
}

bool DemonBoss::IsSamuraiOnLeft() const
{
    Mario* mario = mGame->GetMario();
    if (!mario) return false;

    return mario->GetPosition().x < mPosition.x;
}

void DemonBoss::FaceAwayFromSamurai()
{
    bool samuraiOnLeft = IsSamuraiOnLeft();
    mIsFacingLeft = !samuraiOnLeft;
    mRotation = samuraiOnLeft ? Math::Pi : 0.0f;
}

void DemonBoss::JumpAwayFromSamurai()
{
    if (!CanJump())
        return;

    Vector2 jumpForce(0.0f, -mJumpForce);

    // Add slight horizontal force away from Mario
    if (IsSamuraiOnLeft()) {
        jumpForce.x = -mJumpForce * 0.5f;
    } else {
        jumpForce.x = mJumpForce * 0.5f;
    }

    mRigidBodyComponent->ApplyForce(jumpForce);
    mJumpTimer = JUMP_COOLDOWN;
    mIsGrounded = false;
}

bool DemonBoss::CanJump() const
{
    return mIsGrounded && mJumpTimer <= 0.0f;
}

void DemonBoss::Kill()
{
    // Play death animation/sound
    mGame->GetAudio()->PlaySound("BossDefeated.wav");

    // Spawn power-up or trigger level completion
    // ...

    // Destroy the boss
    SetState(ActorState::Destroy);
}

void DemonBoss::LoadAnimationsFromFile(const std::string& filePath)
{
    // // Load animations from JSON file
    // mDrawComponent->LoadAnimations(filePath);
    // mDrawComponent->SetAnimation("idle");
}
