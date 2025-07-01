//
// Created by Lucas N. Ferreira on 28/09/23.
//

#pragma once
#include "../Component.h"
#include "../../GameMath.h"
#include "../RigidBodyComponent.h"
#include <vector>
#include <map>
#include <set>
#include <functional>
#include <string>
#include <unordered_map>

enum class ColliderLayer
{
    Player,
    Enemy,
    Boss,
    Blocks,
    Slash,
    EndLevel,
    EnemyProjectile
};

class AABBColliderComponent : public Component
{
public:
    // Collider ignore map
    const std::map<ColliderLayer, const std::set<ColliderLayer>> ColliderIgnoreMap = {
        {ColliderLayer::Player, {ColliderLayer::Slash}},
        {ColliderLayer::Boss, {}},
        {ColliderLayer::Enemy,  {}},
        {ColliderLayer::Blocks, {ColliderLayer::Blocks}},
        {ColliderLayer::Slash, {
            ColliderLayer::Player,
            ColliderLayer::Blocks,
            }},
        { ColliderLayer::EnemyProjectile, {
            ColliderLayer::Enemy,
            ColliderLayer::Blocks,
            }},
        {ColliderLayer::EndLevel,{}}
    };

    AABBColliderComponent(class Actor* owner, int dx, int dy, int w, int h,
                                ColliderLayer layer, bool isStatic = false, int updateOrder = 10);
    ~AABBColliderComponent() override;

    bool Intersect(const AABBColliderComponent& b) const;

    float DetectHorizontalCollision(RigidBodyComponent *rigidBody);
    float DetectVertialCollision(RigidBodyComponent *rigidBody);

    void SetStatic(bool isStatic) { mIsStatic = isStatic; }

    Vector2 GetMin() const;
    Vector2 GetMax() const;
    Vector2 GetCenter() const;
    ColliderLayer GetLayer() const { return mLayer; }

    //Animations
    using AnimationAction = std::function<void(AABBColliderComponent*)>;
    void AddAnimationAction(const std::string& animName, AnimationAction action);
    void OnAnimationChange(const std::string& newAnimName);

    //Tornar propriedades dinâmicas
    void SetOffset(const Vector2& offset) { mOffset = offset; }
    void SetSize(int width, int height) { mWidth = width; mHeight = height; }
    void SetLayer(ColliderLayer layer) { mLayer = layer; }

private:
    float GetMinVerticalOverlap(AABBColliderComponent* b) const;
    float GetMinHorizontalOverlap(AABBColliderComponent* b) const;

    void ResolveHorizontalCollisions(RigidBodyComponent *rigidBody, const float minOverlap);
    void ResolveVerticalCollisions(RigidBodyComponent *rigidBody, const float minOverlap);
    std::unordered_map<std::string, AnimationAction> mAnimationActions;


    Vector2 mOffset;
    int mWidth;
    int mHeight;
    bool mIsStatic;

    ColliderLayer mLayer;
};