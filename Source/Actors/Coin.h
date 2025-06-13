//
// Created by Gustavo  Dias Apolinário  on 09/06/25.
//

#ifndef COIN_H
#define COIN_H

#pragma once
#include "Actor.h"
#include "Points.h"

class Coin : public Actor {
public:
    const int TIME_FLIPPING = 1.0;
    const int FLIP_FORCE = 580;
    const int COIN_WIDTH = static_cast<int>(8 * 1.6);
    const int POINTS_WIDTH = static_cast<int>(12 * 1.6);
    Coin(Game * game, Vector2 position);
    void OnUpdate(float deltaTime) override;
private:
    class DrawAnimatedComponent* mDrawComponent;
    void FlipAnimation();
    bool mIsFliping;
    float mFlipTime;
    class RigidBodyComponent* mRigidBodyComponent;
    Vector2 mOriginalPosition;
};



#endif //COIN_H
