#pragma once

#include "Actor.h"

class Block : public Actor
{
public:
    explicit Block(Game* game, const std::string &texturePath, bool isStatic = true);

    void SetPosition(const Vector2& position) override {
        Actor::SetPosition(position);
        mOriginalPosition.Set(position.x, position.y);
    }

    void ChangeBlockTexture(const std::string &texturePath);
    void OnUpdate(float deltaTime) override;
    void OnVerticalCollision(float minOverlap, AABBColliderComponent* other) override;

private:
    const int BUMP_FORCE = 200;

    Vector2 mOriginalPosition;

    class AABBColliderComponent* mColliderComponent;
    class RigidBodyComponent* mRigidBodyComponent;
    class DrawSpriteComponent* mDrawSpriteComponent;
};
