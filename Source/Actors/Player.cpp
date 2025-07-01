//
// Created by Lucas N. Ferreira on 03/08/23.
//

#include "Player.h"
#include "Block.h"
#include "../Game.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Components/DrawComponents/DrawPolygonComponent.h"
#include "../Json.h"
#include <fstream>
#include <HUD.h>
#include <string>

#include "Slash.h"

Player::Player(Game* game, const float forwardSpeed, const float jumpSpeed)
        : Actor(game)
        , mIsRunning(false)
        , mIsOnPole(false)
        ,mIsDying(false)
        ,mIsRolling(false)
        ,mIsJumping(false)
        ,mIsFalling(false)
        ,mIsAttacking(false)
        ,mHasStartedIdleToRun(false)
        , mForwardSpeed(forwardSpeed)
        , mJumpSpeed(jumpSpeed)
        , mPoleSlideTimer(0.0f)
        , mHealth(5)
{
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f, 5.0f);
    mColliderComponent = new AABBColliderComponent(this, 0, 0, Game::TILE_SIZE,Game::TILE_SIZE,
                                                   ColliderLayer::Player);

    mDrawComponent = new DrawAnimatedComponent(this, 150);

    mDrawComponent->LoadCharacterAnimations("Assets/Sprites/Samurai/Samurai.json");

    // Define a animação inicial
    mDrawComponent->SetAnimation("idle");
    SetScale(1.25);

}

void Player::OnProcessInput(const uint8_t* state)
{
    if(mGame->GetGamePlayState() != Game::GamePlayState::Playing) return;
    if (mIsRolling) return;

    if (state[SDL_SCANCODE_D] || state[SDL_SCANCODE_RIGHT])
    {
        mRigidBodyComponent->ApplyForce(Vector2::UnitX * mForwardSpeed);
        mRotation = 0.0f;

        if (!mIsStartingToRun && !mHasStartedIdleToRun) {
            mIsStartingToRun = true;
            mHasStartedIdleToRun = true;
            mIdleToRunTimer = mDrawComponent->GetAnimationDuration("idle_to_run"); // 👈 nova função
            mDrawComponent->SetLoop(false);
            mDrawComponent->SetAnimation("idle_to_run");
        }

        mIsRunning = true;
    }

    else if (state[SDL_SCANCODE_A] || state[SDL_SCANCODE_LEFT])
    {
        mRigidBodyComponent->ApplyForce(Vector2::UnitX * -mForwardSpeed);
        mRotation = Math::Pi;

        if (!mIsStartingToRun && !mHasStartedIdleToRun) {
            mIsStartingToRun = true;
            mHasStartedIdleToRun = true;
            mIdleToRunTimer = mDrawComponent->GetAnimationDuration("idle_to_run");
            mDrawComponent->SetLoop(false);
            mDrawComponent->SetAnimation("idle_to_run");
        }

        mIsRunning = true;
    }
    else {
        mHasStartedIdleToRun = false;
    }

    if (!( state[SDL_SCANCODE_A] || state[SDL_SCANCODE_LEFT] ) && !( state[SDL_SCANCODE_D] || state[SDL_SCANCODE_RIGHT] )){
        mIsRunning = false;
    }

}

void Player::OnHandleKeyPress(const int key, const bool isPressed)
{
    if(mGame->GetGamePlayState() != Game::GamePlayState::Playing) return;

    // Jump
    if ((key == SDLK_SPACE || key == SDLK_w || key == SDLK_UP) && isPressed && mIsOnGround)
    {
        mRigidBodyComponent->SetVelocity(Vector2(mRigidBodyComponent->GetVelocity().x, mJumpSpeed));
        mIsJumping = true;

        // --------------
        // TODO - PARTE 4
        // --------------

        // TODO 1.: Toque o som "Jump.wav" quando Mario pular.
        mGame->GetAudio()->PlaySound("Jump.wav");
    }
    else if (key == SDLK_e && isPressed && !mIsAttacking) {
        mIsAttacking = true;
        float rotation = GetRotation(); // 0 (direita) ou Pi (esquerda)
        Vector2 direction(Math::Cos(rotation), -Math::Sin(rotation));
        Vector2 positionSlash;
        if (rotation == Math::Pi) {
            positionSlash = GetPosition() - Vector2(SPLASH_WIDTH - Game::TILE_SIZE,Game::TILE_SIZE/2);
        }else {
            positionSlash = GetPosition() - Vector2(0,Game::TILE_SIZE/2);
        }

        Vector2 playerVelocity = mRigidBodyComponent->GetVelocity();
        Vector2 slashVelocity(playerVelocity.x, 0.0f);
        mSlash = new Slash(mGame, positionSlash, 0.25f, rotation);
        mAttackTimer = ATTACK_TIME;
        mGame->GetAudio()->PlaySound("swing.wav");
    }
    if ((key == SDLK_t) && isPressed && !mIsRolling && mIsOnGround) {
        mIsRolling = true;

        float direction = (mRotation == Math::Pi) ? -1.0f : 1.0f;

        float rollDuration = mDrawComponent->GetCurrentAnimationDuration(); // em segundos
        float rollVelocity = ROLL_TOTAL_DISTANCE / rollDuration;

        mRigidBodyComponent->SetAcceleration(Vector2::Zero);
        mRigidBodyComponent->SetApplyFriction(false);
        mRigidBodyComponent->SetVelocity(Vector2(direction * rollVelocity, mRigidBodyComponent->GetVelocity().y));
    }
}

void Player::OnUpdate(float deltaTime)
{

    // Camera
    mPosition.x = Math::Max(mPosition.x, mGame->GetCameraPos().x);
    mPosition.x = Math::Min(mPosition.x, mGame->GetCameraPos().x + mGame->GetWindowWidth() - Game::TILE_SIZE);


    // Jumping && Falling
    if (mRigidBodyComponent && mRigidBodyComponent->GetVelocity().y != 0)
    {
        mIsOnGround = false;
    }

    if (mIsJumping && !mIsOnGround && mRigidBodyComponent->GetVelocity().y >= 0) {
        mIsJumping = false;
        mIsFalling = true;
    }

    if (!mIsOnGround && !mIsJumping) {
        mIsFalling = true;
    }

    // Death
    if (mGame->GetGamePlayState() == Game::GamePlayState::Playing && mPosition.y > mGame->GetWindowHeight())
    {
        Kill();
    }

    //Stop
    if (!mIsRunning && mIsOnGround && !mIsRolling) {
        mRigidBodyComponent->SetVelocity(Vector2::Zero);
    }

    //Attack
    if (mIsAttacking) {
        if (mDrawComponent->IsAnimationFinished()) {
            mIsAttacking = false;
        }
    }

    //Roll
    if (mIsRolling) {
        Vector2 vel = mRigidBodyComponent->GetVelocity();
        if (mDrawComponent->IsAnimationFinished()) {
            mIsRolling = false;
            mRigidBodyComponent->SetVelocity(Vector2::Zero);
            mRigidBodyComponent->SetApplyFriction(true);
        }
    }

    //Run
    if (mIsStartingToRun) {
        mIdleToRunTimer -= deltaTime;
        if (mIdleToRunTimer <= 0.0f) {
            mIsStartingToRun = false;
            mDrawComponent->SetLoop(true);
            // SDL_Log("Finish Transition");
            mDrawComponent->SetAnimation("run");
        }
    }

    ManageAnimations();
}

void Player::ManageAnimations()
{
    if(mIsDying)
    {
        mDrawComponent->SetAnimation("Dead");
    }
    else if (mIsRolling) {
        mDrawComponent->SetLoop(false);
        mDrawComponent->SetAnimation("roll");
    }
    else if (mIsFalling) {
        mDrawComponent->SetLoop(false);
        mDrawComponent->SetAnimation("fall");
    }
    else if (mIsJumping) {
        mDrawComponent->SetLoop(false);
        mDrawComponent->SetAnimation("jump");
    }
    else if (mIsAttacking)
    {
        mDrawComponent->SetLoop(false);
        mDrawComponent->SetAnimation("attack");
        mColliderComponent->OnAnimationChange("attack");
    }
    else if(mIsOnPole)
    {
        mDrawComponent->SetLoop(false);
        mDrawComponent->SetAnimation("win");
    }
    else if (!mIsStartingToRun && mIsOnGround && mIsRunning)
    {
        mDrawComponent->SetLoop(true);
        mDrawComponent->SetAnimation("run");
    }
    else if (mIsOnGround && !mIsRunning && mDrawComponent->GetCurrentAnimationName() != "idle")
    {
        // SDL_Log("idle");
        mDrawComponent->SetLoop(true);
        mDrawComponent->SetAnimation("idle");
    }


}

void Player::Kill()
{
    mIsDying = true;
    mGame->SetGamePlayState(Game::GamePlayState::GameOver);
    mDrawComponent->SetAnimation("Dead");

    // Disable collider and rigid body
    mRigidBodyComponent->SetEnabled(false);
    mColliderComponent->SetEnabled(false);

    mGame->GetAudio()->StopAllSounds();
    mGame->GetAudio()->PlaySound("Dead.wav");

    mGame->ResetGameScene(3.5f); // Reset the game scene after 3 seconds
    mGame->UnlockCamera();
}

void Player::Win(AABBColliderComponent *poleCollider)
{
    mGame->SetGamePlayState(Game::GamePlayState::LevelComplete);
}

void Player::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (other->GetLayer() == ColliderLayer::Enemy)
    {
        // Tenta aplicar dano
        HUD* hud = mGame->GetHUD(); // você deve garantir que Game tenha esse getter
        hud->TakeDamage();

        if (hud->GetCurrentBattery() <= 0)
        {
            Kill();
        }
        else
        {
            //mGame->GetAudio()->PlaySound("Hit.wav");
        }
    }

    else if (other->GetLayer() == ColliderLayer::EndLevel)
    {
        Win(other);
    }
    else if (other->GetLayer() == ColliderLayer::Mushroom)
    {
        Mushroom* mushroom = static_cast<Mushroom*>(other->GetOwner());
        mushroom->SetState(ActorState::Destroy);
    }
}

void Player::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (other->GetLayer() == ColliderLayer::Enemy)
    {
        other->GetOwner()->Kill();
        mRigidBodyComponent->SetVelocity(Vector2(mRigidBodyComponent->GetVelocity().x, mJumpSpeed / 2.5f));

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
        else {
            mIsFalling = false;
            mIsJumping = false;
        }

        if (other->GetLayer() == ColliderLayer::Mushroom)
        {

        }
    }
}

void Player::Hurt() {
    SDL_Log("Player hurt life: %d", mHealth);

    mHealth--;
    if (mHealth <= 0) {
        Kill();
    }

    auto xComponent = mRotation == Math::Pi ? 1.0f : -1.0f;
    mRigidBodyComponent->ApplyForce(Vector2{xComponent, -1} * 10000.0f);
}
