//
// Created by Gustavo  Dias Apolinário  on 09/06/25.
//

#ifndef POINTS_H
#define POINTS_H
#include "Actor.h"
#include <random>
#include <chrono>


class Points : public Actor {
public:
    const int TIME_GOING_UP = 1.4;
    const int UP_FORCE = 20;
    Points(Game * game, Vector2 position);
    void OnUpdate(float deltaTime) override;
private:
    class DrawSpriteComponent* mDrawComponent;
    void FlyingEffect();
    bool mIsGoingUp;
    float mFlipTime;
    class RigidBodyComponent* mRigidBodyComponent;
};



#endif //POINTS_H
