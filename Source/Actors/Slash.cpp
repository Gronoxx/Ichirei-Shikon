#include "Slash.h"

#include "Player.h"
#include "../Game.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Components/ColliderComponents/AABBColliderComponent.h"

Slash::Slash(Game* game, const Vector2& position, const float totalLifeTime, float rotation, const Vector2& velocity)
        : Actor(game)
        , mLifeTime(totalLifeTime)
        , mVelocity(velocity)
{
    SetPosition(position);
    SetScale(1.0f);
    SetRotation(rotation);

    // Componente visual
    mDrawComponent = new DrawAnimatedComponent(this, 200);
    mDrawComponent->LoadCharacterAnimations("Assets/Sprites/Slash/Slash.json");
    mDrawComponent->SetAnimation("slash01");
    mDrawComponent->SetLoop(false);

    // Colisor
    mColliderComponent = new AABBColliderComponent(
        this,
        0, 0,                // Offset
        106, 32,             // Tamanho
        ColliderLayer::Slash
    );

    SetPosition(position);
}

void Slash::OnUpdate(float deltaTime)
{
    // Movimenta o slash junto à direção do jogador
    SetPosition(GetPosition() + mVelocity * deltaTime);

    mLifeTime -= deltaTime;
    if (mLifeTime <= 0.0f || mDrawComponent->IsAnimationFinished()) {
        SetState(ActorState::Destroy);
    }
}

void Slash::OnHorizontalCollision(float minOverlap, AABBColliderComponent* other)
{
    other->GetOwner()->Hurt();
}

void Slash::OnVerticalCollision(float minOverlap, AABBColliderComponent* other)
{
    other->GetOwner()->Hurt();
    Particle *p = other->GetOwner()->GetComponent<Particle>();
    if (p != nullptr)
    {
        p->Parry(mPosition);
        mGame->GetAudio()->PlaySound("Parry.wav");
        SDL_Log("Parrying.");
    }
}
