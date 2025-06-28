//
// Created by Lucas N. Ferreira on 28/09/23.
//

#include <algorithm>
#include "AABBColliderComponent.h"
#include "../../Actors/Actor.h"
#include "../../Game.h"

AABBColliderComponent::AABBColliderComponent(class Actor* owner, int dx, int dy, int w, int h,
        ColliderLayer layer, bool isStatic, int updateOrder)
        :Component(owner, updateOrder)
        ,mOffset(Vector2((float)dx, (float)dy))
        ,mIsStatic(isStatic)
        ,mWidth(w)
        ,mHeight(h)
        ,mLayer(layer)
{
//    mOwner->GetGame()->AddCollider(this);
}

AABBColliderComponent::~AABBColliderComponent()
{
//    mOwner->GetGame()->RemoveCollider(this);
}

Vector2 AABBColliderComponent::GetMin() const
{
    return mOwner->GetPosition() + mOffset;
}

Vector2 AABBColliderComponent::GetMax() const
{
    return GetMin() + Vector2((float)mWidth, (float)mHeight);
}

Vector2 AABBColliderComponent::GetCenter() const
{
    return GetMin() + Vector2((float)mWidth / 2.0f, (float)mHeight / 2.0f);
}

bool AABBColliderComponent::Intersect(const AABBColliderComponent& b) const
{
    return (GetMin().x < b.GetMax().x && GetMax().x > b.GetMin().x &&
            GetMin().y < b.GetMax().y && GetMax().y > b.GetMin().y);
}

float AABBColliderComponent::GetMinVerticalOverlap(AABBColliderComponent* b) const
{
    float top = GetMin().y - b->GetMax().y; // Top
    float down = GetMax().y - b->GetMin().y; // Down

    return (Math::Abs(top) < Math::Abs(down)) ? top : down;
}

float AABBColliderComponent::GetMinHorizontalOverlap(AABBColliderComponent* b) const
{
    float right = GetMax().x - b->GetMin().x; // Right
    float left = GetMin().x - b->GetMax().x; // Left

    return (Math::Abs(left) < Math::Abs(right)) ? left : right;
}

float AABBColliderComponent::DetectHorizontalCollision(RigidBodyComponent *rigidBody)
{
    if (mIsStatic || !mIsEnabled) return false;

    // Use spatial hashing to get nearby colliders
    auto colliders = mOwner->GetGame()->GetNearbyColliders(mOwner->GetPosition());

    std::sort(colliders.begin(), colliders.end(), [this](AABBColliderComponent* a, AABBColliderComponent* b) {
        return Math::Abs((a->GetCenter() - GetCenter()).LengthSq() < (b->GetCenter() - GetCenter()).LengthSq());
    });

    for (auto& collider : colliders)
    {
        if (collider == this || !collider->IsEnabled()) continue;

        // Check if the collider is in the same layer or if it should be ignored
        if (ColliderIgnoreMap.at(mLayer).find(collider->GetLayer()) != ColliderIgnoreMap.at(mLayer).end())
        {
            continue; // Ignore this collider
        }

        if (Intersect(*collider))
        {
            float minHorizontalOverlap = GetMinHorizontalOverlap(collider);
            ResolveHorizontalCollisions(rigidBody, minHorizontalOverlap);

            mOwner->OnHorizontalCollision(minHorizontalOverlap, collider);
            return minHorizontalOverlap;
        }
    }

    return 0.0f;
}

float AABBColliderComponent::DetectVertialCollision(RigidBodyComponent *rigidBody)
{
    if (mIsStatic || !mIsEnabled) return false;

    // Use spatial hashing to get nearby colliders
    auto colliders = mOwner->GetGame()->GetNearbyColliders(mOwner->GetPosition());

    std::sort(colliders.begin(), colliders.end(), [this](AABBColliderComponent* a, AABBColliderComponent* b) {
        return Math::Abs((a->GetCenter() - GetCenter()).LengthSq() < (b->GetCenter() - GetCenter()).LengthSq());
    });

    for (auto& collider : colliders)
    {
        if (collider == this || !collider->IsEnabled()) continue;

        // Check if the collider is in the same layer or if it should be ignored
        if (ColliderIgnoreMap.at(mLayer).find(collider->GetLayer()) != ColliderIgnoreMap.at(mLayer).end())
        {
            continue; // Ignore this collider
        }

        if (Intersect(*collider))
        {
            float minVerticalOverlap = GetMinVerticalOverlap(collider);
            ResolveVerticalCollisions(rigidBody, minVerticalOverlap);

            // Callback only for closest (first) collision
            mOwner->OnVerticalCollision(minVerticalOverlap, collider);
            return minVerticalOverlap;
        }
    }

    return 0.0f;
}

void AABBColliderComponent::ResolveHorizontalCollisions(RigidBodyComponent *rigidBody, const float minXOverlap)
{
    mOwner->SetPosition(mOwner->GetPosition() - Vector2(minXOverlap, 0.0f));
    rigidBody->SetVelocity(Vector2(0.f, rigidBody->GetVelocity().y));
}

void AABBColliderComponent::ResolveVerticalCollisions(RigidBodyComponent *rigidBody, const float minYOverlap)
{
    mOwner->SetPosition(mOwner->GetPosition() - Vector2(0.0f, minYOverlap));
    rigidBody->SetVelocity(Vector2(rigidBody->GetVelocity().x, 0.f));

    if (minYOverlap > .0f) {
        mOwner->SetOnGround();
    }
}


void AABBColliderComponent::AddAnimationAction(const std::string& animName, AnimationAction action)
{
    if (mAnimationActions.count(animName)) {
        SDL_Log("Aviso: Ação para a animação '%s' já existe. Substituindo.", animName.c_str());
    }
    mAnimationActions[animName] = action;
}

void AABBColliderComponent::OnAnimationChange(const std::string& newAnimName)
{
    // Procura por uma ação associada à nova animação
    auto it = mAnimationActions.find(newAnimName);
    if (it != mAnimationActions.end())
    {
        // SDL_Log("Executando ação do colisor para a animação: %s", newAnimName.c_str());
        it->second(this);
    }
    else
    {
        SDL_Log("Nenhuma ação de colisor encontrada para a animação: %s", newAnimName.c_str());
    }
}

