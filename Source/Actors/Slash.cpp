#include "Slash.h"

#include "Mario.h"
#include "../Game.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Components/ColliderComponents/AABBColliderComponent.h"

Slash::Slash(Game* game, const Vector2& position,const float TotalLifeTime, float rotation)
        : Actor(game)
        , mLifeTime(TotalLifeTime)
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
        106, 32,              // Tamanho
        ColliderLayer::Slash // Ou outro layer se quiser que não colida com o jogador
    );


    SetPosition(position);
}

void Slash::OnUpdate(float deltaTime)
{
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
}
