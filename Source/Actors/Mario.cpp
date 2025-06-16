//
// Created by Lucas N. Ferreira on 03/08/23.
//

#include "Mario.h"
#include "Block.h"
#include "../Game.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Components/DrawComponents/DrawPolygonComponent.h"

Mario::Mario(Game* game, const float forwardSpeed, const float jumpSpeed)
        : Actor(game)
        , mIsRunning(false)
        , mIsOnPole(false)
        , mIsDying(false)
        , mForwardSpeed(forwardSpeed)
        , mJumpSpeed(jumpSpeed)
        , mPoleSlideTimer(0.0f)
{
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f, 5.0f);
    mColliderComponent = new AABBColliderComponent(this, 0, 0, Game::TILE_SIZE - 4.0f,Game::TILE_SIZE,
                                                   ColliderLayer::Player);

    mDrawComponent = new DrawAnimatedComponent(this,
                                              "../Assets/Sprites/Samurai/Idle/texture.png",
                                              "../Assets/Sprites/Samurai/Idle/texture.json",
                                              "idle",
                                              {0,2,3,4,5,6,7,8,9,1}
                                              );

        std::vector<Vector2> idleOffsets = {
        Vector2(8, 34), //IDLE1
        Vector2(10, 34), //IDLE10
        Vector2(8, 33), //IDLE2
        Vector2(8, 33), //IDLE3
        Vector2(8, 33), //IDLE4
        Vector2(9, 33), //IDLE5
        Vector2(10, 33), //IDLE6
        Vector2(10, 33),//IDLE7
        Vector2(10, 33), //IDLE8
        Vector2(10, 33) //IDLE9
    };

    mDrawComponent->AddAnimationOffsets("idle",idleOffsets);
    mDrawComponent->SetAnimFPS(14.0f);

    mDrawComponent -> LoadSpriteSheetForAnimation("run",
    "../Assets/Sprites/Samurai/Run/texture.png",
    "../Assets/Sprites/Samurai/Run/texture.json");

    std::vector<Vector2> runOffsets = {
        Vector2(25, 4), // Run3.png
        Vector2(20, 5), // Run14.png
        Vector2(24, 4), // Run4.png
        Vector2(19, 6), // Run15.png
        Vector2(23, 5), // Run5.png
        Vector2(23, 5), // Run12.png
        Vector2(18, 5), // Run16.png
        Vector2(25, 4), // Run2.png
        Vector2(22, 5), // Run6.png
        Vector2(21, 4), // Run13.png
        Vector2(21, 4), // Run1.png
        Vector2(22, 5), // Run10.png
        Vector2(22, 5), // Run7.png
        Vector2(23, 6), // Run8.png
        Vector2(22, 4), // Run9.png
        Vector2(22, 5), // Run11.png
    };

    mDrawComponent->AddAnimation("run", {
    2, // Run3.png
    13, // Run14.png
    3, // Run4.png
    14, // Run15.png
    4, // Run5.png
    11, // Run12.png
    15, // Run16.png
    1, // Run2.png
    5, // Run6.png
    12, // Run13.png
    0, // Run1.png
    9, // Run10.png
    6, // Run7.png
    7, // Run8.png
    8, // Run9.png
    10, // Run11.png
});

    mDrawComponent->AddAnimationOffsets("run",runOffsets);
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
        mIsOnGround = false;

        // --------------
        // TODO - PARTE 4
        // --------------

        // TODO 1.: Toque o som "Jump.wav" quando Mario pular.
        mGame->GetAudio()->PlaySound("Jump.wav");
    }
}

void Mario::OnUpdate(float deltaTime)
{
    // Check if Mario is off the ground
    if (mRigidBodyComponent && mRigidBodyComponent->GetVelocity().y != 0)
    {
        mIsOnGround = false;
    }

    // Limit Mario's position to the camera view
    mPosition.x = Math::Max(mPosition.x, mGame->GetCameraPos().x);

    // Kill mario if he falls below the screen
    if (mGame->GetGamePlayState() == Game::GamePlayState::Playing && mPosition.y > mGame->GetWindowHeight())
    {
        Kill();
    }

    if (mIsOnPole)
    {
        // If Mario is on the pole, update the pole slide timer
        mPoleSlideTimer -= deltaTime;
        if (mPoleSlideTimer <= 0.0f)
        {
            mRigidBodyComponent->SetApplyGravity(true);
            mRigidBodyComponent->SetApplyFriction(false);
            mRigidBodyComponent->SetVelocity(Vector2::UnitX * 100.0f);
            mGame->SetGamePlayState(Game::GamePlayState::Leaving);

            // --------------
            // TODO - PARTE 4
            // --------------

            // TODO 1.: Toque o som "StageClear.wav"
            mGame->GetAudio()->PlaySound("StageClear.wav");


            mIsOnPole = false;
            mIsRunning = true;
        }
    }

    // If Mario is leaving the level, kill him if he enters the castle
    const float castleDoorPos = Game::LEVEL_WIDTH * Game::TILE_SIZE - 10 * Game::TILE_SIZE;

    if (mGame->GetGamePlayState() == Game::GamePlayState::Leaving &&
        mPosition.x >= castleDoorPos)
    {
        // Stop Mario and set the game scene to Level 2
        mState = ActorState::Destroy;
        mGame->SetGameScene(Game::GameScene::Level2, 3.5f);

        return;
    }

    ManageAnimations();
}

void Mario::ManageAnimations()
{
    if(mIsDying)
    {
        mDrawComponent->SetAnimation("Dead");
    }
    else if(mIsOnPole)
    {
        mDrawComponent->SetAnimation("win");
    }
    else if (mIsOnGround && mIsRunning)
    {
        mDrawComponent->SetAnimation("run");
    }
    else if (mIsOnGround && !mIsRunning)
    {
        mDrawComponent->SetAnimation("idle");
    }
    else if (!mIsOnGround)
    {
        mDrawComponent->SetAnimation("jump");
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