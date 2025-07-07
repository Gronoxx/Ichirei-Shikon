// Goomba.cpp

#include "Ghost.h"
#include "../Game.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/ColliderComponents/AABBColliderComponent.h"
#include "../GameMath.h" // Para Math::Pi
#include "../Actors/Player.h"

Ghost::Ghost(Game *game, float forwardSpeed, float deathTime)
    : Actor(game)
      , mDyingTimer(deathTime)
      , mIsDying(false)
      , mForwardSpeed(forwardSpeed)
      , mSeekingRadius(250.0f)
      , mPatrolRadius(150.0f) // Pode patrulhar até 150 pixels de distância do seu ponto inicial
      , mPatrolDirectionChangeInterval(3.0f) // Muda de direção a cada 3 segundos
      , mPatrolTimer(0.0f) // Começa com 0 para escolher uma direção imediatamente
      , mCurrentPatrolDirection(Vector2::Zero)
      , mRandomGenerator(std::random_device()()) // Semeia o gerador
      , mDistribution(0.0f, Math::TwoPi)
      , mCurrentState(GhostState::Patrolling)
      , mRetreatTimer(0.0f)
      , mSpawnGracePeriod(0.2f)
      , mPerlin(std::random_device()())
      , mNoiseInput(0.0f)
      , mNoiseScale(0.7f) // Um bom valor para começar
      , mNoiseStrength(0.8f)
      , mTurnSpeed(4.0f)                // Um valor entre 3 e 8 costuma ser bom
      , mOscillationFrequency(3.0f)
      , mOscillationAmplitude(15.0f)
{
    // A ordem de criação dos componentes está correta
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f);
    mRigidBodyComponent->SetVelocity(Vector2(-mForwardSpeed, 0.0f));

    mColliderComponent = new AABBColliderComponent(this, 0, 0,
                                                   Game::TILE_SIZE, Game::TILE_SIZE,
                                                   ColliderLayer::Enemy);
    mRigidBodyComponent->SetApplyGravity(false);

    mDrawComponent = new DrawAnimatedComponent(this, 150);

    mDrawComponent->LoadCharacterAnimations("Assets/Sprites/Ghost/Ghost.json");

    // Define a animação inicial
    mDrawComponent->SetAnimation("idle");
    SetScale(1.25);

    mPatrolAnchorPoint = GetPosition();
}

void Ghost::Kill() {
    mIsDying = true;

    // Agora é simples: basta dizer o nome da animação a ser tocada.
    if (mDrawComponent) {
        mDrawComponent->SetAnimation("dying");
    }

    mRigidBodyComponent->SetEnabled(false);
    mColliderComponent->SetEnabled(false);
}


// O resto do Goomba.cpp (OnUpdate, colisões, etc.) permanece o mesmo.
void Ghost::OnUpdate(float deltaTime) {
    // Timers de controle
    if (mSpawnGracePeriod > 0.0f) mSpawnGracePeriod -= deltaTime;
    mNoiseInput += deltaTime;

    // Lógica de Morte
    if (mIsDying) {
        mRigidBodyComponent->SetVelocity(Vector2::Zero);
        mDyingTimer -= deltaTime;
        if (mDyingTimer <= 0.0f) mState = ActorState::Destroy;
        return;
    }

    // Se cair do mundo
    if (GetPosition().y > GetGame()->GetWindowHeight() + 100) {
        mState = ActorState::Destroy;
    }

    // Vira o sprite com base na direção da velocidade
    if (mRigidBodyComponent->GetVelocity().x < -1.0f) {
        mRotation = Math::Pi; // Esquerda
    } else if (mRigidBodyComponent->GetVelocity().x > 1.0f) {
        mRotation = 0; // Direita
    }

    // --- LÓGICA DA MÁQUINA DE ESTADOS ---

    Player* player = GetGame()->GetPlayer();
    Vector2 ghostPos = GetPosition();
    Vector2 playerPos = player ? player->GetPosition() : ghostPos;
    float distance = player && player->GetState() == ActorState::Active ? (playerPos - ghostPos).Length() : mSeekingRadius + 1.0f;

    // 1. VERIFICA TRANSIÇÕES DE ESTADO
    if (mCurrentState == GhostState::Pursuing && distance >= mSeekingRadius) {
        mCurrentState = GhostState::Retreating;
        mRetreatTimer = 1.5f; // Duração do recuo
    }
    else if (mCurrentState == GhostState::Retreating) {
        mRetreatTimer -= deltaTime;
        if (mRetreatTimer <= 0.0f) {
            mCurrentState = GhostState::Patrolling;
            mPatrolAnchorPoint = GetPosition();
        }
    }
    else if (distance < mSeekingRadius && mCurrentState != GhostState::Retreating) {
        mCurrentState = GhostState::Pursuing;
    }

    // 2. CALCULA A VELOCIDADE DESEJADA COM BASE NO ESTADO ATUAL
    Vector2 desiredVelocity = Vector2::Zero;

    switch (mCurrentState) {
        case GhostState::Pursuing: {
            Vector2 mainDirection = playerPos - ghostPos;
            mainDirection.Normalize();
            double noise = mPerlin.noise(mNoiseInput * mNoiseScale, 0.0, 0.0);
            float noiseAngle = static_cast<float>(noise) * Math::TwoPi;
            Vector2 noiseDirection = Vector2(Math::Cos(noiseAngle), Math::Sin(noiseAngle));
            Vector2 desiredDirection = mainDirection + noiseDirection * mNoiseStrength;
            desiredDirection.Normalize();

            desiredVelocity = desiredDirection * mForwardSpeed;

            float verticalOscillation = Math::Sin(mNoiseInput * mOscillationFrequency) * mOscillationAmplitude;
            desiredVelocity.y += verticalOscillation;
            break;
        }
        case GhostState::Retreating: {
            Vector2 retreatDirection = ghostPos - playerPos;
            if (retreatDirection.LengthSq() < 1.0f) retreatDirection = Vector2(1.0f, 0.0f);
            retreatDirection.Normalize();
            desiredVelocity = retreatDirection * mForwardSpeed;
            break;
        }
        case GhostState::Patrolling: {
            mPatrolTimer -= deltaTime;
            if (mPatrolTimer <= 0.0f) {
                mPatrolTimer = mPatrolDirectionChangeInterval;
                float distFromAnchor = (ghostPos - mPatrolAnchorPoint).Length();
                if (distFromAnchor > mPatrolRadius) {
                    mCurrentPatrolDirection = mPatrolAnchorPoint - ghostPos;
                } else {
                    float randomAngle = mDistribution(mRandomGenerator);
                    mCurrentPatrolDirection = Vector2(Math::Cos(randomAngle), Math::Sin(randomAngle));
                }
                mCurrentPatrolDirection.Normalize();
            }
            const float patrolSpeed = mForwardSpeed * 0.5f;
            desiredVelocity = mCurrentPatrolDirection * patrolSpeed;
            break;
        }
    }

    // 3. APLICA A VELOCIDADE DE FORMA SUAVIZADA (LERP)
    Vector2 currentVelocity = mRigidBodyComponent->GetVelocity();
    Vector2 smoothedVelocity = Vector2::Lerp(currentVelocity, desiredVelocity, deltaTime * mTurnSpeed);
    mRigidBodyComponent->SetVelocity(smoothedVelocity);
}

void Ghost::OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other) {
    if (other->GetLayer() == ColliderLayer::Blocks ||
        other->GetLayer() == ColliderLayer::Enemy ||
        other->GetLayer() == ColliderLayer::Player) {
        if (minOverlap > 0) {
            mRigidBodyComponent->SetVelocity(Vector2(-mForwardSpeed, 0.0f));
        } else {
            mRigidBodyComponent->SetVelocity(Vector2(mForwardSpeed, 0.0f));
        }
    }

    auto owner = other->GetOwner();
    AABBColliderComponent *collider = owner->GetComponent<AABBColliderComponent>();
    if (owner && collider->GetLayer() == ColliderLayer::Slash) {
        Kill();
    }

    if (other->GetLayer() == ColliderLayer::Blocks) {
        // Inverte a direção horizontal da patrulha
        mCurrentPatrolDirection.x *= -1.0f;

        // Reinicia o timer da patrulha para que a nova direção dure um pouco
        mPatrolTimer = mPatrolDirectionChangeInterval;

        // Aplica imediatamente a nova velocidade para que ele se afaste da parede
        const float patrolSpeed = mForwardSpeed * 0.5f;
        mRigidBodyComponent->SetVelocity(mCurrentPatrolDirection * patrolSpeed);
    }

}

void Ghost::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other) {
    auto owner = other->GetOwner();
    AABBColliderComponent *collider = owner->GetComponent<AABBColliderComponent>();
    if (owner && collider->GetLayer() == ColliderLayer::Player) {
        Kill();
    }

    if (other->GetLayer() == ColliderLayer::Blocks) {
        // Inverte a direção vertical da patrulha
        mCurrentPatrolDirection.y *= -1.0f;

        // Reinicia o timer da patrulha
        mPatrolTimer = mPatrolDirectionChangeInterval;

        // Aplica imediatamente a nova velocidade
        const float patrolSpeed = mForwardSpeed * 0.5f;
        mRigidBodyComponent->SetVelocity(mCurrentPatrolDirection * patrolSpeed);
    }
}
