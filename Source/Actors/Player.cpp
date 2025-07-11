#include "Player.h"
#include "Block.h"
#include "../Game.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Components/DrawComponents/DrawPolygonComponent.h"
#include "../Json.h"
#include <../UIElements/UIHud.h>
#include <string>
#include "Slash.h"

Player::Player(Game *game, const float forwardSpeed, const float jumpSpeed)
    : Actor(game)
      , mIsRunning(false)
      , mIsOnPole(false)
      , mIsDying(false)
      , mIsRolling(false)
      , mIsJumping(false)
      , mIsFalling(false)
      , mIsAttacking(false)
      , mHasStartedIdleToRun(false)
      , mForwardSpeed(forwardSpeed)
      , mJumpSpeed(jumpSpeed)
      , mPoleSlideTimer(0.0f)
      , mIsHurt(false)
      , mHurtTimer(0.0f)
      {
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f, 5.0f);
    mColliderComponent = new AABBColliderComponent(this, 0, 0, Game::TILE_SIZE, Game::TILE_SIZE,
                                                   ColliderLayer::Player);

    mDrawComponent = new DrawAnimatedComponent(this, 150);

    mDrawComponent->LoadCharacterAnimations("Assets/Sprites/Samurai/Samurai.json");

    // Define a animação inicial
    mDrawComponent->SetAnimation("idle");
    SetScale(1.25);
}

void Player::HandleInput(const uint8_t* state, const SDL_Event* event) {
    if (mGame->GetGamePlayState() != Game::GamePlayState::Playing) return;
    if (mIsRolling || mIsHurt) return;

    // Eventos de tecla (pressionar/release)
    if (event) {
        if ((event->type == SDL_KEYDOWN) &&
            (event->key.keysym.sym == SDLK_LSHIFT || event->key.keysym.sym == SDLK_RSHIFT) &&
            !mIsRolling && mIsOnGround)
        {
            mIsRolling = true;
            float direction = (mRotation == Math::Pi) ? -1.0f : 1.0f;
            float rollDuration = mDrawComponent->GetCurrentAnimationDuration();
            const float MIN_ROLL_DURATION = 0.3f;
            if (rollDuration < MIN_ROLL_DURATION) rollDuration = MIN_ROLL_DURATION;
            float rollVelocity = ROLL_TOTAL_DISTANCE / rollDuration;
            const float MAX_ROLL_VELOCITY = 400.0f;
            if (rollVelocity > MAX_ROLL_VELOCITY) rollVelocity = MAX_ROLL_VELOCITY;
            mRigidBodyComponent->SetAcceleration(Vector2::Zero);
            mRigidBodyComponent->SetApplyFriction(false);
            mRigidBodyComponent->SetVelocity(Vector2(0.0f, mRigidBodyComponent->GetVelocity().y));
            mRigidBodyComponent->SetVelocity(Vector2(direction * rollVelocity, mRigidBodyComponent->GetVelocity().y));
            mIsRunning = false;
            return;
        }
        // Outros eventos (pulo, ataque, etc) podem ser tratados aqui, se necessário
        if ((event->type == SDL_KEYDOWN) &&
            (event->key.keysym.sym == SDLK_SPACE || event->key.keysym.sym == SDLK_w || event->key.keysym.sym == SDLK_UP) &&
            mIsOnGround)
        {
            mRigidBodyComponent->SetVelocity(Vector2(mRigidBodyComponent->GetVelocity().x, mJumpSpeed));
            mIsJumping = true;
            mGame->GetAudio()->PlaySound("Jump.wav");
        }
        if ((event->type == SDL_KEYDOWN) &&
            (event->key.keysym.sym == SDLK_e) && !mIsAttacking)
        {
            mIsAttacking = true;
            float rotation = GetRotation();
            Vector2 direction(Math::Cos(rotation), -Math::Sin(rotation));
            Vector2 positionSlash;
            if (rotation == Math::Pi) {
                positionSlash = GetPosition() - Vector2(SPLASH_WIDTH - Game::TILE_SIZE,Game::TILE_SIZE/2);
            } else {
                positionSlash = GetPosition() - Vector2(0,Game::TILE_SIZE/2);
            }
            Vector2 playerVelocity = mRigidBodyComponent->GetVelocity();
            Vector2 slashVelocity(playerVelocity.x, 0.0f);
            mSlash = new Slash(mGame, positionSlash, 0.25f, rotation, slashVelocity);
            mAttackTimer = ATTACK_TIME;
            mGame->GetAudio()->PlaySound("swing.wav");
        }
    }

    float maxSpeed = mForwardSpeed;

    // Estado contínuo do teclado (movimentação)
    if (state[SDL_SCANCODE_D] || state[SDL_SCANCODE_RIGHT])
    {
        Vector2 vel = mRigidBodyComponent->GetVelocity();
        mRigidBodyComponent->SetVelocity(Vector2(mForwardSpeed, vel.y));
        mRotation = 0.0f;
        if (vel.x > maxSpeed) {
            vel.x = maxSpeed;
            mRigidBodyComponent->SetVelocity(vel);
        }
        if (!mIsStartingToRun && !mHasStartedIdleToRun) {
            mIsStartingToRun = true;
            mHasStartedIdleToRun = true;
            mIdleToRunTimer = mDrawComponent->GetAnimationDuration("idle_to_run");
            mDrawComponent->SetLoop(false);
            mDrawComponent->SetAnimation("idle_to_run");
        }
        mIsRunning = true;
    }
    else if (state[SDL_SCANCODE_A] || state[SDL_SCANCODE_LEFT])
    {
        Vector2 vel = mRigidBodyComponent->GetVelocity();
        mRigidBodyComponent->SetVelocity(Vector2(-mForwardSpeed, vel.y));
        mRotation = Math::Pi;
        if (vel.x < -maxSpeed) {
            vel.x = -maxSpeed;
            mRigidBodyComponent->SetVelocity(vel);
        }
        if (!mIsStartingToRun && !mHasStartedIdleToRun) {
            mIsStartingToRun = true;
            mHasStartedIdleToRun = true;
            mIdleToRunTimer = mDrawComponent->GetAnimationDuration("idle_to_run");
            mDrawComponent->SetLoop(false);
            mDrawComponent->SetAnimation("idle_to_run");
        }
        mIsRunning = true;
    } else {
        mHasStartedIdleToRun = false;
    }
    if (!(state[SDL_SCANCODE_A] || state[SDL_SCANCODE_LEFT]) && !(state[SDL_SCANCODE_D] || state[SDL_SCANCODE_RIGHT])){
        mIsRunning = false;
    }
}

void Player::OnUpdate(float deltaTime) {
    // Camera
    mPosition.x = Math::Min(mPosition.x, mGame->GetCameraPos().x + mGame->GetWindowWidth() - Game::TILE_SIZE);

    if (mIsHurt) {
        mHurtTimer -= deltaTime;
        if (mHurtTimer <= 0.0f) {
            mIsHurt = false;
        }
    }

    // Jumping && Falling
    if (mRigidBodyComponent && mRigidBodyComponent->GetVelocity().y != 0) {
        mIsOnGround = false;
    }

    if (mIsJumping && !mIsOnGround && mRigidBodyComponent->GetVelocity().y >= 0) {
        mIsJumping = false;
        mIsFalling = true;
    }

    if (!mIsOnGround && !mIsJumping) {
        mIsFalling = true;
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
        // Se sair do chão durante o roll, encerra o roll imediatamente
        if (!mIsOnGround) {
            mIsRolling = false;
            mRigidBodyComponent->SetApplyFriction(true);
            // Opcional: zera ou limita a velocidade horizontal ao cair
            Vector2 vel = mRigidBodyComponent->GetVelocity();
            mRigidBodyComponent->SetVelocity(Vector2(vel.x * 0.5f, vel.y)); // Reduz pela metade, ou use Zero se preferir
        }
        else if (mDrawComponent->IsAnimationFinished()) {
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

    std::vector<std::string> states;

    if (mIsHurt) {
        states.push_back("machucado");
    }
    if (mIsRolling) {
        states.push_back("rolando");
    }
    if (mIsAttacking) {
        states.push_back("atacando");
    }
    if (mIsJumping) {
        states.push_back("pulando");
    }
    if (mIsFalling) {
        states.push_back("caindo");
    }
    if (mIsRunning) {
        states.push_back("correndo");
    }
    if (mIsStartingToRun) {
        states.push_back("iniciando corrida");
    }
    if (!mIsRunning && mIsOnGround && !mIsRolling && !mIsJumping && !mIsFalling && !mIsAttacking && !mIsHurt) {
        states.push_back("parado");
    }

    // Monta a string com os estados
    std::string stateDesc;
    for (size_t i = 0; i < states.size(); ++i) {
        stateDesc += states[i];
        if (i < states.size() - 1) {
            stateDesc += ", ";
        }
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Player está: %s | Posição: (%.2f, %.2f) | Velocidade: (%.2f, %.2f)",
        stateDesc.c_str(),
        mPosition.x, mPosition.y,
        mRigidBodyComponent ? mRigidBodyComponent->GetVelocity().x : 0.0f,
        mRigidBodyComponent ? mRigidBodyComponent->GetVelocity().y : 0.0f);

    ManageAnimations();
}

void Player::ManageAnimations() {
    if (mIsDying) {
        mDrawComponent->SetAnimation("Dead");
    } else if (mIsRolling) {
        mDrawComponent->SetLoop(false);
        mDrawComponent->SetAnimation("roll");
    } else if (mIsFalling) {
        mDrawComponent->SetLoop(false);
        mDrawComponent->SetAnimation("fall");
    } else if (mIsJumping) {
        mDrawComponent->SetLoop(false);
        mDrawComponent->SetAnimation("jump");
    } else if (mIsAttacking) {
        mDrawComponent->SetLoop(false);
        mDrawComponent->SetAnimation("attack");
        mColliderComponent->OnAnimationChange("attack");
    } else if (mIsOnPole) {
        mDrawComponent->SetLoop(false);
        mDrawComponent->SetAnimation("win");
    } else if (!mIsStartingToRun && mIsOnGround && mIsRunning) {
        mDrawComponent->SetLoop(true);
        mDrawComponent->SetAnimation("run");
    } else if (mIsOnGround && !mIsRunning && mDrawComponent->GetCurrentAnimationName() != "idle") {
        // SDL_Log("idle");
        mDrawComponent->SetLoop(true);
        mDrawComponent->SetAnimation("idle");
    }
}

void Player::Kill() {
    mIsDying = true;
    mGame->SetGamePlayState(Game::GamePlayState::GameOver);
    mDrawComponent->SetAnimation("Dead");

    // Disable collider and rigid body
    mRigidBodyComponent->SetEnabled(false);
    mColliderComponent->SetEnabled(false);

    mGame->GetAudio()->StopAllSounds();
    mGame->GetAudio()->PlaySound("Dead.wav");

    auto screen = new UIScreen(mGame, "Assets/Fonts/SMB.ttf");
    screen->AddImage("Assets/Sprites/Death.png", Vector2(0,150), Vector2(640,108));

    mGame->ResetGameScene(3.5f); // Reset the game scene after 3 seconds
    mGame->UnlockCamera();
}

void Player::Win() const {
    mGame->SetGamePlayState(Game::GamePlayState::LevelComplete);
}

void Player::OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other) {
    if (other->GetLayer() == ColliderLayer::Enemy || other->GetLayer() == ColliderLayer::Boss) {
        Hurt();
        KnockBack();
    } else if (other->GetLayer() == ColliderLayer::EndLevel) {
        Win();
    }
}

void Player::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other) {
    if (other->GetLayer() == ColliderLayer::Enemy || other->GetLayer() == ColliderLayer::Boss) {
        Hurt();
        KnockBack();
    } else if (other->GetLayer() == ColliderLayer::Blocks) {
        if (mIsOnGround) {
            mIsFalling = false;
            mIsJumping = false;
        }
    }
}

void Player::Hurt() {
    UIHud *hud = mGame->GetHUD(); // você deve garantir que Game tenha esse getter
    hud->TakeDamage();

    if (hud->GetCurrentBattery() <= 0) {
        Kill();
        return;
    }

    if (!mIsHurt) {
        mIsHurt = true;
        mHurtTimer = HURT_DURATION;
    }

    // TODO: Adicionar som de hit
    // mGame->GetAudio()->PlaySound("Hit.wav");
}

void Player::KnockBack() const {
    const auto xComponent = mRotation == Math::Pi ? 1.0f : -1.0f;
    mRigidBodyComponent->SetVelocity(Vector2(xComponent * 300.0f, -300.0f));
}
