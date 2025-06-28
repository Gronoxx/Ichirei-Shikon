//
// Created by Lucas N. Ferreira on 10/09/23.
//

#include "CircleColliderComponent.h"
#include "../../Actors/Actor.h"

CircleColliderComponent::CircleColliderComponent(class Actor* owner, const float radius, const int updateOrder)
        :Component(owner, updateOrder)
        ,mRadius(radius) {

}

const Vector2& CircleColliderComponent::GetCenter() const
{
    return mOwner->GetPosition();
}

float CircleColliderComponent::GetRadius() const
{
    return mOwner->GetScale() * mRadius;
}

bool CircleColliderComponent::Intersect(const CircleColliderComponent& c) const
{
    // Calculate amount squared
    Vector2 diff = GetCenter() - c.GetCenter();
    float distSq = diff.LengthSq();

    // Calculate sum of radii squared
    float radiiSq = GetRadius() + c.GetRadius();
    radiiSq *= radiiSq;

    return distSq <= radiiSq;
}

bool CircleColliderComponent::Intersect(const AABBColliderComponent& c) const
{
    // Get the circle center
    Vector2 circleCenter = GetCenter();

    // Get the AABB min and max points
    Vector2 aabbMin = c.GetMin();
    Vector2 aabbMax = c.GetMax();

    // Clamp circle center to AABB boundaries
    float closestX = std::max(aabbMin.x, std::min(circleCenter.x, aabbMax.x));
    float closestY = std::max(aabbMin.y, std::min(circleCenter.y, aabbMax.y));

    // Calculate the distance between circle center and closest point
    Vector2 closestPoint(closestX, closestY);
    Vector2 difference = circleCenter - closestPoint;
    float distanceSquared = difference.LengthSq();

    // Check if distance is less than or equal to the radius squared
    return distanceSquared <= (mRadius * mRadius);
}
