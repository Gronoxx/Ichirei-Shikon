//
// Created by Lucas N. Ferreira on 28/09/23.
//

#pragma once

#include "Actor.h"
#include "Coin.h"
#include "Mushroom.h"

class Block : public Actor
{
public:
    explicit Block(Game* game, const std::string &texturePath, int numberOfCoins = 0, const bool isStatic = true, bool hasMushroom = false);

    void SetPosition(const Vector2& position)
    {
        Actor::SetPosition(position);
        mOriginalPosition.Set(position.x, position.y);
    }

    void ChangeBlockTexture(const std::string &texturePath);
    void OnUpdate(float deltaTime) override;
    void OnBump();
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;

private:
    const int BUMP_FORCE = 200;
    const int COIN_WIDTH = static_cast<int>(8 * 1.6);

    Vector2 mOriginalPosition;

    class AABBColliderComponent* mColliderComponent;
    class RigidBodyComponent* mRigidBodyComponent;
    class DrawSpriteComponent* mDrawSpriteComponent;
    int mNumberOfCoins;
    bool mHasMushroom;

};
