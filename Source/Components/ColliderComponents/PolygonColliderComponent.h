//
// Created by Mateus on 06/07/2025.
//

#pragma once
#include "../Component.h"
#include "../../GameMath.h"
#include "../RigidBodyComponent.h"
#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <functional>
#include <string>

class PolygonColliderComponent : public Component
{
public:
    PolygonColliderComponent(class Actor* owner,
                              const std::vector<Vector2>& points,
                              ColliderLayer layer,
                              bool isStatic = false,
                              int updateOrder = 10);
    ~PolygonColliderComponent() override;

    void SetPoints(const std::vector<Vector2>& points);
    const std::vector<Vector2>& GetPoints() const { return mLocalPoints; }

    std::vector<Vector2> GetWorldPoints() const;
    Vector2 GetCenter() const;

    bool Intersect(const PolygonColliderComponent& other) const;

    float DetectHorizontalCollision(RigidBodyComponent* rigidBody);
    float DetectVerticalCollision(RigidBodyComponent* rigidBody);

    void SetStatic(bool isStatic) { mIsStatic = isStatic; }
    ColliderLayer GetLayer() const { return mLayer; }
    void SetLayer(ColliderLayer layer) { mLayer = layer; }

    using AnimationAction = std::function<void(PolygonColliderComponent*)>;
    void AddAnimationAction(const std::string& animName, AnimationAction action);
    void OnAnimationChange(const std::string& newAnimName);

private:
    void ResolveHorizontalCollisions(RigidBodyComponent* rigidBody, float overlap);
    void ResolveVerticalCollisions(RigidBodyComponent* rigidBody, float overlap);

    std::vector<Vector2> mLocalPoints;
    bool mIsStatic;
    ColliderLayer mLayer;

    std::unordered_map<std::string, AnimationAction> mAnimationActions;

    const std::map<ColliderLayer, const std::set<ColliderLayer>> ColliderIgnoreMap = {
        {ColliderLayer::Player, {ColliderLayer::Slash}},
        {ColliderLayer::Boss, {}},
        {ColliderLayer::Enemy, {}},
        {ColliderLayer::Blocks, {ColliderLayer::Blocks}},
        {ColliderLayer::Slash, {ColliderLayer::Player, ColliderLayer::Blocks}},
        {ColliderLayer::EnemyProjectile, {ColliderLayer::Enemy, ColliderLayer::Blocks}},
        {ColliderLayer::EndLevel, {}}
    };
};
