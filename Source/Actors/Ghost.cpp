#include "Ghost.h"
#include "../Game.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/ColliderComponents/AABBColliderComponent.h"
#include "../GameMath.h" // For Math constants
#include "../Actors/Player.h"

Ghost::Ghost(Game* game, float patrolSpeed, float chasingSpeed, float deathTime)
    : Actor(game),
      mDyingTimer(deathTime),
      mIsDying(false),
      mPatrolSpeed(patrolSpeed),
      mChasingSpeed(chasingSpeed),
      mCurrentState(GhostState::Patrolling),
      mSeePlayerDistance(200.0f),
      mLosePlayerDistance(350.0f),
      mSpawnGracePeriod(0.2f),
      mPatrolRadius(10.0f),
      mPerlin(std::random_device()()),
      mNoiseInput(0.0f),
      mNoiseScale(0.7f),
      mNoiseStrength(0.8f),
      mTurnSpeed(4.0f),
      mOscillationFrequency(3.0f),
      mOscillationAmplitude(15.0f)
{
    // The order of component creation is important
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f);
    mRigidBodyComponent->SetApplyGravity(false); // Ghosts don't fall

    mColliderComponent = new AABBColliderComponent(this, 0, 0,
                                                   Game::TILE_SIZE, Game::TILE_SIZE,
                                                   ColliderLayer::Enemy);

    mDrawComponent = new DrawAnimatedComponent(this, 150);
    mDrawComponent->LoadCharacterAnimations("Assets/Sprites/Ghost/Ghost.json");
    mDrawComponent->SetAnimation("idle");
    SetScale(1.25);

    // Set initial patrol state
    mRigidBodyComponent->SetVelocity(Vector2(-mPatrolSpeed, 0.0f));
}

void Ghost::Kill() {
    mIsDying = true;

    if (mDrawComponent) {
        mDrawComponent->SetAnimation("dying");
    }

    mRigidBodyComponent->SetEnabled(false);
    mColliderComponent->SetEnabled(false);
}

void Ghost::OnUpdate(float deltaTime) {
    // Control timers
    if (mSpawnGracePeriod > 0.0f) mSpawnGracePeriod -= deltaTime;
    mNoiseInput += deltaTime;

    // Death logic
    if (mIsDying) {
        mRigidBodyComponent->SetVelocity(Vector2::Zero);
        mDyingTimer -= deltaTime;
        if (mDyingTimer <= 0.0f) mState = ActorState::Destroy;
        return;
    }

    // Fall out of the world
    if (GetPosition().y > GetGame()->GetWindowHeight() + 100) {
        mState = ActorState::Destroy;
    }

    // Flip sprite based on velocity direction
    if (mRigidBodyComponent->GetVelocity().x < -1.0f) {
        mRotation = Math::Pi; // Left
    } else if (mRigidBodyComponent->GetVelocity().x > 1.0f) {
        mRotation = 0; // Right
    }

    // --- STATE MACHINE LOGIC ---

    Player* player = GetGame()->GetPlayer();
    Vector2 ghostPos = GetPosition();
    float distanceToPlayer = mSeePlayerDistance + 1.0f; // Default distance if no player

    if (player && player->GetState() == ActorState::Active) {
        distanceToPlayer = (player->GetPosition() - ghostPos).Length();
    }

    // 1. STATE TRANSITIONS
    if (mCurrentState == GhostState::Patrolling && distanceToPlayer < mSeePlayerDistance) {
        mCurrentState = GhostState::Chasing;
    } else if (mCurrentState == GhostState::Chasing && distanceToPlayer > mLosePlayerDistance) {
        mCurrentState = GhostState::Patrolling;
    }

    // 2. CALCULATE DESIRED VELOCITY BASED ON CURRENT STATE
    Vector2 desiredVelocity = Vector2::Zero;

    switch (mCurrentState) {
        case GhostState::Chasing: {
            Vector2 playerPos = player->GetPosition();
            Vector2 mainDirection = playerPos - ghostPos;
            mainDirection.Normalize();

            // Add Perlin noise for a more unpredictable movement
            double noise = mPerlin.noise(mNoiseInput * mNoiseScale, 0.0, 0.0);
            float noiseAngle = static_cast<float>(noise) * Math::TwoPi;
            Vector2 noiseDirection = Vector2(Math::Cos(noiseAngle), Math::Sin(noiseAngle));
            Vector2 desiredDirection = mainDirection + noiseDirection * mNoiseStrength;
            desiredDirection.Normalize();

            desiredVelocity = desiredDirection * mChasingSpeed; // Use chasing speed
            break;
        }
        case GhostState::Patrolling: {
            float distFromStart = GetPosition().x - mPatrolStartPoint.x;
            Vector2 currentVelocity = mRigidBodyComponent->GetVelocity();

            // If at the edge of patrol radius, turn around
            if ((distFromStart > mPatrolRadius && currentVelocity.x > 0.0f) ||
                (distFromStart < -mPatrolRadius && currentVelocity.x < 0.0f)) {
                currentVelocity.x *= -1.0f; // Reverse horizontal direction
            }
            // If velocity is very low (e.g., after stopping), give it a push
            else if (Math::Abs(currentVelocity.x) < 1.0f) {
                currentVelocity.x = -mPatrolSpeed;
            }

            desiredVelocity = Vector2(currentVelocity.x, 0.0f);
            if (desiredVelocity.LengthSq() > 0.0f) {
                desiredVelocity.Normalize();
            }
            desiredVelocity *= mPatrolSpeed; // Use patrol speed
            break;
        }
    }

    // Apply vertical oscillation to both states for a "wavy" movement
    float verticalOscillation = Math::Sin(mNoiseInput * mOscillationFrequency) * mOscillationAmplitude;
    desiredVelocity.y += verticalOscillation;

    // 3. SMOOTHLY APPLY THE VELOCITY (LERP)
    Vector2 currentVelocity = mRigidBodyComponent->GetVelocity();
    Vector2 smoothedVelocity = Vector2::Lerp(currentVelocity, desiredVelocity, deltaTime * mTurnSpeed);
    mRigidBodyComponent->SetVelocity(smoothedVelocity);
}

void Ghost::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) {
    if (other->GetLayer() == ColliderLayer::Blocks) {
        Vector2 currentVel = mRigidBodyComponent->GetVelocity();
        // Reverse horizontal direction upon hitting a wall
        currentVel.x *= -1.0f;
        mRigidBodyComponent->SetVelocity(currentVel);
    }
}

void Ghost::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) {
    if (other->GetLayer() == ColliderLayer::Blocks) {
        Vector2 currentVel = mRigidBodyComponent->GetVelocity();
        // Negate the vertical velocity from oscillation to prevent "sticking" to ceilings/floors
        currentVel.y *= -0.5f;
        mRigidBodyComponent->SetVelocity(currentVel);
    }
}
void Ghost::StartPatrol()
{
    // Mova as linhas que você removeu do construtor para cá.
    // Agora, GetPosition() retornará a posição correta definida pelo CSV.
    mPatrolStartPoint = GetPosition();
    mRigidBodyComponent->SetVelocity(Vector2(-mPatrolSpeed, 0.0f));
}
