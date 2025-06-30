//
// Created by olive on 30/06/2025.
//

#pragma once

#include "Actor.h"

class Trigger : public Actor {
public:
    explicit Trigger(Game* game);

    void SetPosition(const Vector2& position) {
        Actor::SetPosition(position);
        mOriginalPosition.Set(position.x, position.y);
    }


    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;

private:
    Vector2 mOriginalPosition;

    class AABBColliderComponent* mColliderComponent;
};
