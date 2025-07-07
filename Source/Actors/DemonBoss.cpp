#include "DemonBoss.h"
#include "../Game.h"
#include "Player.h"
#include "../Random.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Components/ColliderComponents/AABBColliderComponent.h"
#include "../Components/DrawComponents/DrawPolygonComponent.h"

const float JUMP_FORCE = 43000.0f;
const float JUMP_COOLDOWN = 2.5f;
const int INITIAL_HEALTH = 20;

DemonBoss::DemonBoss(Game *game, float attackCooldown, float unvulnerableCooldown, float moveSpeed)
    : Actor(game)
      , mCurrentState(State::Waiting)
      , mAttackCooldown(attackCooldown)
      , mVulnerableCooldown(unvulnerableCooldown)
      , mAttackTimer(attackCooldown / 1.5f)
      , mMoveSpeed(moveSpeed)
      , mJumpForce(JUMP_FORCE)
      , mJumpCooldown(JUMP_COOLDOWN)
      , mJumpTimer(JUMP_COOLDOWN)
      , mHealth(INITIAL_HEALTH)
      , mIsGrounded(true)
      , mIsFacingLeft(true) {
    // Set up physics
    mRigidBodyComponent = new RigidBodyComponent(this, 2.8f, 10.0f, true);

    // Set up animations
    std::vector<Vector2> vertices;

    mDrawComponent = new DrawAnimatedComponent(this, 150);
    mDrawComponent->LoadCharacterAnimations("Assets/Sprites/DemonBoss/DemonBoss.json");
    mDrawComponent->SetAnimation("idle");

    // Get the animation size to set the collider
    Vector2 animSize = mDrawComponent->GetAnimationSize("idle");

    mColliderComponent = new AABBColliderComponent(this, 0, 0, animSize.x, animSize.y,
                                                   ColliderLayer::Boss);

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
        case State::Waiting:
            UpdateWaiting(deltaTime);
            break;

        case State::Moving:
            mAttackTimer += deltaTime;
            UpdateMoving(deltaTime);
            break;

        case State::Vulnerable:
            // Just wait until attack cooldown is over
            mAttackTimer -= deltaTime;
            if (mAttackTimer <= 0.0f) {
                mCurrentState = State::Moving;
            }
            break;
    }

    // Update animations
    ManageAnimations();

    // Keep boss on screen and reverse direction if it hits the edge
    Vector2 cameraPos = mGame->GetCameraPos();
    float minX = cameraPos.x + Game::TILE_SIZE * 0.1f;
    float maxX = cameraPos.x + mGame->GetWindowWidth() - Game::TILE_SIZE * 3.0f;

    auto velocity = mRigidBodyComponent->GetVelocity();
    if ((mPosition.x <= minX && velocity.x < 0.0f) || (mPosition.x >= maxX && velocity.x > 0.0f)) {
        velocity.x *= -1.0f;
        mRigidBodyComponent->SetVelocity(velocity);
    }

    // mPosition.x = Math::Clamp(mPosition.x, minX, maxX);
}

void DemonBoss::UpdateMoving(float deltaTime) {
    if (!mIsGrounded) {
        return;
    }

    Player *mario = mGame->GetPlayer();
    if (!mario) {
        return;
    }

    FaceTowardsSamurai();

    if (CanJump()) {
        Jump(true);
    } else if (mAttackTimer >= mAttackCooldown) {
        StartAttack();
    }
}

void DemonBoss::UpdateWaiting(float deltaTime) {
    auto playerPos = mGame->GetPlayer()->GetPosition();

    if (playerPos.x - mPosition.x < mGame->GetWindowWidth() - Game::TILE_SIZE * 10) {
        mCurrentState = State::Moving;
        //mGame->LockCamera();
    }
}

void DemonBoss::StartAttack() {
    if (mCurrentState != State::Moving)
        return;

    mRigidBodyComponent->SetVelocity(Vector2::Zero);

    SpawnMinions();

    mCurrentState = State::Vulnerable;
    mAttackTimer = mVulnerableCooldown; // Reset attack timer

    ManageAnimations();
}

void DemonBoss::SpawnMinions() {
    // --- LÓGICA CORRIGIDA ---
    // Obtenha os limites reais do nível, em vez dos limites da câmera.
    const Vector2 levelOffset = mGame->GetLevelOffset();
    const float levelWidth = mGame->GetLevelPixelWidth();
    const float levelHeight = mGame->GetLevelPixelHeight();

    const float levelLeft = levelOffset.x;
    const float levelRight = levelOffset.x + levelWidth;
    const float levelTop = levelOffset.y;

    for (int i = 0; i < 3; ++i) {
        // Posição de spawn aleatória ao longo da largura do nível, um pouco acima do topo.
        float spawnX = Random::GetFloatRange(levelLeft, levelRight);
        float spawnY = levelTop - Game::TILE_SIZE; // Nasce fora da tela, acima

        // Posição alvo aleatória DENTRO dos limites da arena.
        // Adicionamos um pouco de margem (padding) para não irem direto para as bordas.
        const float padding = Game::TILE_SIZE * 2.0f;
        float targetX = Random::GetFloatRange(levelLeft + padding, levelRight - padding);
        float targetY = Random::GetFloatRange(levelTop + padding, levelTop + levelHeight / 2.0f);
        // Mira na metade superior da arena

        Vector2 targetPos(targetX, targetY);

        // Cria e posiciona o lacaio
        auto *minion = new FlyingDemon(mGame, targetPos, 6.0f, 400.0f);
        minion->SetPosition(targetPos);
    }
}

void DemonBoss::ManageAnimations() {
    if (mCurrentState == State::Vulnerable) {
        mDrawComponent->SetAnimation("invoking");
    }

    else if (!mIsGrounded) {
        mDrawComponent->SetAnimation("jump");
    }
    //  else if (mAttackTimer <= mVulnerableCooldown) {
    //     mDrawComponent->SetAnimation("invoking");
    // }
     else {
        mDrawComponent->SetAnimation("idle");
    }
}

bool DemonBoss::IsSamuraiOnLeft() const {
    Player *mario = mGame->GetPlayer();
    if (!mario) return false;

    return mario->GetPosition().x < mPosition.x;
}

void DemonBoss::FaceAwayFromSamurai() {
    bool samuraiOnLeft = IsSamuraiOnLeft();
    mIsFacingLeft = !samuraiOnLeft;
    mRotation = samuraiOnLeft ? Math::Pi : 0.0f;
}

void DemonBoss::FaceTowardsSamurai() {
    bool samuraiOnLeft = IsSamuraiOnLeft();
    mIsFacingLeft = samuraiOnLeft;
    mRotation = samuraiOnLeft ? Math::Pi : 0.0f;
}

void DemonBoss::Jump(bool towardsPlayer) {
    if (!CanJump())
        return;

    // A more natural jump has a stronger vertical component.
    const float horizontalForceMultiplier = 0.5f;
    float horizontalForce = mJumpForce * horizontalForceMultiplier;

    Vector2 jumpForce(0.0f, -mJumpForce);

    bool samuraiOnLeft = IsSamuraiOnLeft();
    float direction;

    if (towardsPlayer) {
        // Jump towards the player
        direction = samuraiOnLeft ? -1.0f : 1.0f;
    } else {
        // Jump away from the player
        direction = samuraiOnLeft ? 1.0f : -1.0f;
    }

    jumpForce.x = horizontalForce * direction;

    mRigidBodyComponent->SetApplyFriction(false);
    mRigidBodyComponent->ApplyForce(jumpForce);
    mJumpTimer = JUMP_COOLDOWN;
    mIsGrounded = false;
}

bool DemonBoss::CanJump() const {
    Vector2 cameraPos = mGame->GetCameraPos();
    float minX = cameraPos.x + Game::TILE_SIZE * 0.1f;
    float maxX = cameraPos.x + mGame->GetWindowWidth() - Game::TILE_SIZE * 3.0f;

    return mIsGrounded && mJumpTimer <= 0.0f || mIsGrounded && mPosition.x < minX || mIsGrounded && mPosition.x > maxX;
}

void DemonBoss::Kill() {
    // Play death animation/sound
    // mGame->GetAudio()->PlaySound("BossDefeated.wav");

    mGame->UnlockCamera();

    // Destroy the boss
    SetState(ActorState::Destroy);
}

void DemonBoss::Hurt() {
    SDL_Log("Boss hurt!");
    if (mCurrentState == State::Vulnerable) {
        return;
    }

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

void DemonBoss::OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other) {
}

void DemonBoss::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other) {
    auto owner = other->GetOwner();
    AABBColliderComponent *collider = owner->GetComponent<AABBColliderComponent>();

    if (owner && collider->GetLayer() == ColliderLayer::Blocks) {
        if (minOverlap > 0.0f) {
            mIsGrounded = true;
            mRigidBodyComponent->SetApplyFriction(true);
        }
    }
}
