//
// Created by Lucas N. Ferreira on 03/08/23.
//

#include "Mario.h"
#include "Block.h"
#include "../Game.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Components/DrawComponents/DrawPolygonComponent.h"
#include "../Json.h"
#include <fstream>
#include <string>

Mario::Mario(Game* game, const float forwardSpeed, const float jumpSpeed)
        : Actor(game)
        , mIsRunning(false)
        , mIsOnPole(false)
        ,mIsDying(false)
        ,mIsRolling(false)
        ,mIsJumping(false)
        ,mIsFalling(false)
        ,mIsAttacking(false)
        , mForwardSpeed(forwardSpeed)
        , mJumpSpeed(jumpSpeed)
        , mPoleSlideTimer(0.0f)
{
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f, 5.0f);
    mColliderComponent = new AABBColliderComponent(this, 0, 0, Game::TILE_SIZE - 4.0f,Game::TILE_SIZE,
                                                   ColliderLayer::Player);

    mDrawComponent = new DrawAnimatedComponent(this, 150);

    mDrawComponent->LoadCharacterAnimations("Assets/Sprites/Samurai/Samurai.json");

    // Define a animação inicial
    mDrawComponent->SetAnimation("katanaidle");
    SetScale(1.25);

    //Defini colisor para animação
    mColliderComponent->AddAnimationAction("attack", [](AABBColliderComponent* collider) {
        // Durante o ataque com a espada, expandimos o colisor para a direita.
        // O offset muda para alinhar o colisor com o corpo + espada.
        collider->SetSize(64, 32); // Dobra a largura
        collider->SetOffset(Vector2(0, 0));
        collider->SetEnabled(true);
    });

}

void Mario::OnProcessInput(const uint8_t* state)
{
    if(mGame->GetGamePlayState() != Game::GamePlayState::Playing) return;

    if (state[SDL_SCANCODE_D] || state[SDL_SCANCODE_RIGHT])
    {
        mRigidBodyComponent->ApplyForce(Vector2::UnitX * mForwardSpeed);
        mRotation = 0.0f;
        mIsRunning = true;
    }

    if (state[SDL_SCANCODE_A] || state[SDL_SCANCODE_LEFT])
    {
        mRigidBodyComponent->ApplyForce(Vector2::UnitX * -mForwardSpeed);
        mRotation = Math::Pi;
        mIsRunning = true;
    }

    if (!( state[SDL_SCANCODE_A] || state[SDL_SCANCODE_LEFT] ) && !( state[SDL_SCANCODE_D] || state[SDL_SCANCODE_RIGHT] )){
        mIsRunning = false;
    }

}

void Mario::OnHandleKeyPress(const int key, const bool isPressed)
{
    if(mGame->GetGamePlayState() != Game::GamePlayState::Playing) return;

    // Jump
    if ((key == SDLK_SPACE || key == SDLK_w || key == SDLK_UP) && isPressed && mIsOnGround)
    {
        mRigidBodyComponent->SetVelocity(Vector2(mRigidBodyComponent->GetVelocity().x, mJumpSpeed));
        //mIsJumping = true;

        // --------------
        // TODO - PARTE 4
        // --------------

        // TODO 1.: Toque o som "Jump.wav" quando Mario pular.
        mGame->GetAudio()->PlaySound("Jump.wav");
    }
    else if (key == SDLK_e && isPressed && !mIsAttacking) {
        mIsAttacking = true;
        mAttackTimer = ATTACK_TIME;
    }
    if (( key == SDLK_t)){
        mIsRolling = true;
    }
}

void Mario::OnUpdate(float deltaTime)
{

    // Camera
    mPosition.x = Math::Max(mPosition.x, mGame->GetCameraPos().x);

    // Jumping
    if (mRigidBodyComponent && mRigidBodyComponent->GetVelocity().y != 0)
    {
        mIsOnGround = false;
    }

    // Death
    if (mGame->GetGamePlayState() == Game::GamePlayState::Playing && mPosition.y > mGame->GetWindowHeight())
    {
        Kill();
    }

    //Stop
    if (!mIsRunning && mIsOnGround) {
        mRigidBodyComponent->SetVelocity(Vector2::Zero);
    }

    //Attack
    if (mIsAttacking) {
        mAttackTimer -= deltaTime;
        if (mAttackTimer <= 0.0f) {
            mIsAttacking = false;
        }
    }

    //Roll
    if (mIsRolling) {
        if (mDrawComponent->IsAnimationFinished()) {
            mIsRolling = false;
        }
    }

    // if (mIsJumping) {
    //     if (mDrawComponent->IsAnimationFinished()) {
    //         SDL_Log("Finish Jumping");
    //         mIsJumping = false;
    //         mIsFalling = true;
    //     }
    // }
    //
    // if (mIsFalling) {
    //     if (mDrawComponent->IsAnimationFinished()) {
    //         mIsFalling = false;
    //     }
    // }

    //Pole
    // if (mIsOnPole)
    // {
    //     // If Mario is on the pole, update the pole slide timer
    //     mPoleSlideTimer -= deltaTime;
    //     if (mPoleSlideTimer <= 0.0f)
    //     {
    //         mRigidBodyComponent->SetApplyGravity(true);
    //         mRigidBodyComponent->SetApplyFriction(false);
    //         mRigidBodyComponent->SetVelocity(Vector2::UnitX * 100.0f);
    //         mGame->SetGamePlayState(Game::GamePlayState::Leaving);
    //
    //         // --------------
    //         // TODO - PARTE 4
    //         // --------------
    //
    //         // TODO 1.: Toque o som "StageClear.wav"
    //         mGame->GetAudio()->PlaySound("StageClear.wav");
    //
    //
    //         mIsOnPole = false;
    //         mIsRunning = true;
    //     }
    // }

    //Rest
    // If Mario is leaving the level, kill him if he enters the castle
    // const float castleDoorPos = Game::LEVEL_WIDTH * Game::TILE_SIZE - 10 * Game::TILE_SIZE;
    //
    // if (mGame->GetGamePlayState() == Game::GamePlayState::Leaving &&
    //     mPosition.x >= castleDoorPos)
    // {
    //     // Stop Mario and set the game scene to Level 2
    //     mState = ActorState::Destroy;
    //     mGame->SetGameScene(Game::GameScene::Level2, 3.5f);
    //
    //     return;
    // }

    ManageAnimations();
}

void Mario::ManageAnimations()
{
    if(mIsDying)
    {
        mDrawComponent->SetAnimation("Dead");
    }
    else if (mIsRolling) {
        mDrawComponent->SetLoop(false);
        mDrawComponent->SetAnimation("katanaroll");
    }
    else if (mIsFalling) {
        mDrawComponent->SetLoop(false);
        mDrawComponent->SetAnimation("katanafall");
    }
    else if (mIsJumping) {
        mDrawComponent->SetLoop(false);
        mDrawComponent->SetAnimation("katanajump");
    }
    else if (mIsAttacking)
    {
        mDrawComponent->SetAnimation("attack");
        mColliderComponent->OnAnimationChange("attack");
    }
    else if(mIsOnPole)
    {
        mDrawComponent->SetAnimation("win");
    }
    else if (mIsOnGround && mIsRunning)
    {
        mDrawComponent->SetAnimation("katanarun");
    }
    else if (mIsOnGround && !mIsRunning)
    {
        mDrawComponent->SetAnimation("katanaidle");
    }
}

void Mario::Kill()
{
    mIsDying = true;
    mGame->SetGamePlayState(Game::GamePlayState::GameOver);
    mDrawComponent->SetAnimation("Dead");

    // Disable collider and rigid body
    mRigidBodyComponent->SetEnabled(false);
    mColliderComponent->SetEnabled(false);

    // --------------
    // TODO - PARTE 4
    // --------------

    // TODO 1.: Pare todos os sons com StopAllSounds() e toque o som "Dead.wav".
    mGame->GetAudio()->StopAllSounds();
    mGame->GetAudio()->PlaySound("Dead.wav");

    mGame->ResetGameScene(3.5f); // Reset the game scene after 3 seconds
}

void Mario::Win(AABBColliderComponent *poleCollider)
{
    mDrawComponent->SetAnimation("win");
    mGame->SetGamePlayState(Game::GamePlayState::LevelComplete);

    // Set mario velocity to go down
    mRigidBodyComponent->SetVelocity(Vector2::UnitY * 100.0f); // 100 pixels per second
    mRigidBodyComponent->SetApplyGravity(false);

    // Disable collider
    poleCollider->SetEnabled(false);

    // Adjust mario x position to grab the pole
    mPosition.Set(poleCollider->GetOwner()->GetPosition().x + Game::TILE_SIZE / 4.0f, mPosition.y);

    // --------------
    // TODO - PARTE 4
    // --------------

    // TODO 1.: Pare todos os sons com StopAllSounds()
    mGame->GetAudio()->StopAllSounds();

    mPoleSlideTimer = POLE_SLIDE_TIME; // Start the pole slide timer
}

void Mario::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (other->GetLayer() == ColliderLayer::Enemy)
    {
        Kill();
    }
    else if (other->GetLayer() == ColliderLayer::Pole)
    {
        mIsOnPole = true;
        Win(other);
    }
    else if (other->GetLayer() == ColliderLayer::Mushroom)
    {
        Mushroom* mushroom = static_cast<Mushroom*>(other->GetOwner());
        mushroom->SetState(ActorState::Destroy);
    }
}

void Mario::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (other->GetLayer() == ColliderLayer::Enemy)
    {
        other->GetOwner()->Kill();
        mRigidBodyComponent->SetVelocity(Vector2(mRigidBodyComponent->GetVelocity().x, mJumpSpeed / 2.5f));

        // --------------
        // TODO - PARTE 4
        // --------------

        // TODO 1.: Toque o som "Stomp.wav"
        mGame->GetAudio()->PlaySound("Stomp.wav");

    }
    else if (other->GetLayer() == ColliderLayer::Blocks)
    {
        if (!mIsOnGround)
        {
            // --------------
            // TODO - PARTE 4
            // --------------

            // TODO 1.: Toque o som "Bump.wav"
            mGame->GetAudio()->PlaySound("Bump.wav");

            // Cast actor to Block to call OnBump
            auto* block = dynamic_cast<Block*>(other->GetOwner());
            block->OnBump();
        }
        else if (other->GetLayer() == ColliderLayer::Mushroom)
        {
            Mushroom* mushroom = static_cast<Mushroom*>(other->GetOwner());
            mushroom->SetState(ActorState::Destroy);
        }
    }
}