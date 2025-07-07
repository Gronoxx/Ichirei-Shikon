//
// Created by Mateus on 06/07/2025.
//

#include "PolygonColliderComponent.h"
#include "../../Actors/Actor.h"
#include "../../Game.h"
#include <algorithm>

PolygonColliderComponent::PolygonColliderComponent(class Actor* owner,
                                                   const std::vector<Vector2>& points,
                                                   ColliderLayer layer,
                                                   bool isStatic,
                                                   int updateOrder)
    : Component(owner, updateOrder)
    , mLocalPoints(points)
    , mLayer(layer)
    , mIsStatic(isStatic)
{
}

PolygonColliderComponent::~PolygonColliderComponent() {}

void PolygonColliderComponent::SetPoints(const std::vector<Vector2>& points)
{
    mLocalPoints = points;
}

std::vector<Vector2> PolygonColliderComponent::GetWorldPoints() const
{
    Vector2 origin = mOwner->GetPosition();
    std::vector<Vector2> worldPoints;
    for (const auto& p : mLocalPoints)
    {
        worldPoints.push_back(origin + p);
    }
    return worldPoints;
}

Vector2 PolygonColliderComponent::GetCenter() const
{
    Vector2 sum(0.0f, 0.0f);
    for (const auto& p : mLocalPoints)
    {
        sum += p;
    }
    return mOwner->GetPosition() + sum / static_cast<float>(mLocalPoints.size());
}

// Função de Interseção Polígono-Polígono com Separating Axis Theorem (SAT)
bool PolygonColliderComponent::Intersect(const PolygonColliderComponent& other) const
{
    auto axesFrom = [](const std::vector<Vector2>& pts) {
        std::vector<Vector2> axes;
        for (size_t i = 0; i < pts.size(); ++i)
        {
            Vector2 edge = pts[(i + 1) % pts.size()] - pts[i];
            axes.push_back(Vector2(-edge.y, edge.x).Normalize());
        }
        return axes;
    };

    auto project = [](const std::vector<Vector2>& pts, const Vector2& axis) {
        float min = Vector2::Dot(pts[0], axis);
        float max = min;
        for (const auto& p : pts)
        {
            float proj = Vector2::Dot(p, axis);
            min = std::min(min, proj);
            max = std::max(max, proj);
        }
        return std::make_pair(min, max);
    };

    auto a = GetWorldPoints();
    auto b = other.GetWorldPoints();
    auto axes = axesFrom(a);
    auto axesB = axesFrom(b);
    axes.insert(axes.end(), axesB.begin(), axesB.end());

    for (const auto& axis : axes)
    {
        auto [minA, maxA] = project(a, axis);
        auto [minB, maxB] = project(b, axis);
        if (maxA < minB || maxB < minA)
            return false;
    }

    return true;
}

float PolygonColliderComponent::DetectHorizontalCollision(RigidBodyComponent* rigidBody)
{
    if (mIsStatic || !mIsEnabled) return 0.0f;

    auto colliders = mOwner->GetGame()->GetNearbyPolygonColliders(mOwner->GetPosition());

    for (auto* other : colliders)
    {
        if (other == this || !other->IsEnabled()) continue;
        if (ColliderIgnoreMap.at(mLayer).count(other->GetLayer())) continue;

        if (Intersect(*other))
        {
            float overlap = rigidBody->GetVelocity().x; // simplificação
            ResolveHorizontalCollisions(rigidBody, overlap);
            mOwner->OnHorizontalCollision(overlap, other);
            return overlap;
        }
    }

    return 0.0f;
}

float PolygonColliderComponent::DetectVerticalCollision(RigidBodyComponent* rigidBody)
{
    if (mIsStatic || !mIsEnabled) return 0.0f;

    auto colliders = mOwner->GetGame()->GetNearbyPolygonColliders(mOwner->GetPosition());

    for (auto* other : colliders)
    {
        if (other == this || !other->IsEnabled()) continue;
        if (ColliderIgnoreMap.at(mLayer).count(other->GetLayer())) continue;

        if (Intersect(*other))
        {
            float overlap = rigidBody->GetVelocity().y; // simplificação
            ResolveVerticalCollisions(rigidBody, overlap);
            mOwner->OnVerticalCollision(overlap, other);
            return overlap;
        }
    }

    return 0.0f;
}

void PolygonColliderComponent::ResolveHorizontalCollisions(RigidBodyComponent* rigidBody, float overlap)
{
    mOwner->SetPosition(mOwner->GetPosition() - Vector2(overlap, 0.0f));
    rigidBody->SetVelocity(Vector2(0.f, rigidBody->GetVelocity().y));
}

void PolygonColliderComponent::ResolveVerticalCollisions(RigidBodyComponent* rigidBody, float overlap)
{
    mOwner->SetPosition(mOwner->GetPosition() - Vector2(0.0f, overlap));
    rigidBody->SetVelocity(Vector2(rigidBody->GetVelocity().x, 0.f));

    if (overlap > 0.f)
        mOwner->SetOnGround();
}

void PolygonColliderComponent::AddAnimationAction(const std::string& animName, AnimationAction action)
{
    mAnimationActions[animName] = action;
}

void PolygonColliderComponent::OnAnimationChange(const std::string& newAnimName)
{
    auto it = mAnimationActions.find(newAnimName);
    if (it != mAnimationActions.end())
    {
        it->second(this);
    }
}