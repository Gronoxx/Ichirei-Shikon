// Goomba.cpp

#include "Goomba.h"
#include "../Game.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/ColliderComponents/AABBColliderComponent.h"
#include "../GameMath.h" // Para Math::Pi
#include "../Actors/Player.h"

Goomba::Goomba(Game* game, float forwardSpeed, float deathTime)
    : Actor(game)
    , mDyingTimer(deathTime)
    , mIsDying(false)
    , mForwardSpeed(forwardSpeed)
{
    // A ordem de criação dos componentes está correta
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f);
    mRigidBodyComponent->SetVelocity(Vector2(-mForwardSpeed, 0.0f));

    mColliderComponent = new AABBColliderComponent(this, 0, 0,
                                                   Game::TILE_SIZE, Game::TILE_SIZE,
                                                   ColliderLayer::Enemy);

    // --- CONEXÃO COM O NOVO MOTOR DE ANIMAÇÃO ---

    // 1. Crie o componente usando o construtor vazio.
    mDrawComponent = new DrawAnimatedComponent(this, 110); // Ordem de desenho 110

    // 2. Carregue todas as animações a partir do manifesto do personagem.
    mDrawComponent->LoadCharacterAnimations("Assets/Sprites/Goomba/Goomba_Anims.json");

    // 3. Defina a animação inicial.
    mDrawComponent->SetAnimation("walk");

    // --- FIM DA CONEXÃO ---
}

void Goomba::Kill()
{
    mIsDying = true;

    // Agora é simples: basta dizer o nome da animação a ser tocada.
    if (mDrawComponent) {
        mDrawComponent->SetAnimation("Dead");
    }

    mRigidBodyComponent->SetEnabled(false);
    mColliderComponent->SetEnabled(false);
}

void Goomba::BumpKill(const float bumpForce)
{
    // Define a animação para 'Idle' (o sprite de pé, mas estático)
    if (mDrawComponent) {
        mDrawComponent->SetAnimation("Idle");
    }

    mRigidBodyComponent->SetVelocity(Vector2(bumpForce / 2.0f, -bumpForce));
    mColliderComponent->SetEnabled(false);

    // Vira o ator de cabeça para baixo
    // Assumindo que a rotação em radianos é usada. Math::Pi = 180 graus.
    SetRotation(Math::Pi);
}

// O resto do Goomba.cpp (OnUpdate, colisões, etc.) permanece o mesmo.
void Goomba::OnUpdate(float deltaTime)
{
    if (mIsDying)
    {
        mDyingTimer -= deltaTime;
        if (mDyingTimer <= 0.0f) {
            mState = ActorState::Destroy;
        }
    }

    // Se o Goomba cair para fora do mundo (após um BumpKill, por exemplo)
    if (GetPosition().y > GetGame()->GetWindowHeight() + 100)
    {
        mState = ActorState::Destroy;
    }
}

void Goomba::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if ((other->GetLayer() == ColliderLayer::Blocks || other->GetLayer() == ColliderLayer::Enemy || other->GetLayer() == ColliderLayer::Mushroom))
    {
        if (minOverlap > 0) {
            mRigidBodyComponent->SetVelocity(Vector2(-mForwardSpeed, 0.0f));
        }
        else {
            mRigidBodyComponent->SetVelocity(Vector2(mForwardSpeed, 0.0f));
        }
    }

    auto owner = other->GetOwner();
    AABBColliderComponent* collider = owner->GetComponent<AABBColliderComponent>();
    if (owner && collider->GetLayer() == ColliderLayer::Player) {
        Player* mario = dynamic_cast<Player*>(owner);
        if (mario->isPlayerAttacking()) {
            Kill();
        }
        // else{
        //     mario->Kill();
        // }
    }

    if (owner && collider->GetLayer() == ColliderLayer::Slash) {
        Kill();
    }
}

void Goomba::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other)
{
    auto owner = other->GetOwner();
    AABBColliderComponent* collider = owner->GetComponent<AABBColliderComponent>();
    if (owner && collider->GetLayer() == ColliderLayer::Player) {
        Kill();
    }
}