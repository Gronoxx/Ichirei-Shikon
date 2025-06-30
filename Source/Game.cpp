// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
//
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#include <algorithm>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include "CSV.h"
#include "Random.h"
#include "Game.h"
#include "HUD.h"
#include "SpatialHashing.h"
#include "Actors/Actor.h"
#include "Actors/Player.h"
#include "Actors/Block.h"
#include "Actors/DemonBoss.h"
#include "Actors/FlyingDemon.h"
#include "Actors/Trigger.h"
#include "Actors/Spawner.h"
#include "UIElements/UIScreen.h"
#include "Components/DrawComponents/DrawComponent.h"
#include "Components/DrawComponents/DrawSpriteComponent.h"
#include "Components/DrawComponents/DrawPolygonComponent.h"
#include "Components/ColliderComponents/AABBColliderComponent.h"


Game::Game(int windowWidth, int windowHeight)
        :mWindow(nullptr)
        ,mRenderer(nullptr)
        ,mTicksCount(0)
        ,mIsRunning(true)
        ,mWindowWidth(windowWidth)
        ,mWindowHeight(windowHeight)
        ,mMario(nullptr)
        ,mHUD(nullptr)
        ,mBackgroundColor(0, 0, 0)
        ,mModColor(255, 255, 255)
        ,mCameraPos(Vector2::Zero)
        ,mAudio(nullptr)
        ,mGameTimer(0.0f)
        ,mGameTimeLimit(0)
        ,mSceneManagerTimer(0.0f)
        ,mSceneManagerState(SceneManagerState::None)
        ,mGameScene(GameScene::Intro)
        ,mNextScene(GameScene::Intro)
        ,mBackgroundTexture(nullptr)
        ,mBackgroundSize(Vector2::Zero)
        ,mBackgroundPosition(Vector2::Zero)
        ,mNumberOfCoinsCollected(0)
        ,mIsCameraLocked(false)
        ,mScore(0)
{

}

bool Game::Initialize()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    mWindow = SDL_CreateWindow("Ichirei Shikon", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, mWindowWidth, mWindowHeight, 0);
    if (!mWindow)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!mRenderer)
    {
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        return false;
    }

    if (IMG_Init(IMG_INIT_PNG) == 0)
    {
        SDL_Log("Unable to initialize SDL_image: %s", SDL_GetError());
        return false;
    }

    // Initialize SDL_ttf
    if (TTF_Init() != 0)
    {
        SDL_Log("Failed to initialize SDL_ttf");
        return false;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
    {
        SDL_Log("Failed to initialize SDL_mixer");
        return false;
    }

    // Start random number generator
    Random::Init();

    mAudio = new AudioSystem();


    mSpatialHashing = new SpatialHashing(TILE_SIZE * 4.0f,
                                         LEVEL_WIDTH * TILE_SIZE,
                                         LEVEL_HEIGHT * TILE_SIZE);
    mTicksCount = SDL_GetTicks();

    // Init all game actors
    mNextScene = GameScene::Intro;       // Define qual cena carregar
    ChangeScene();                       // Carrega os recursos da Intro
    mSceneManagerTimer = TRANSITION_TIME;// Define a DURAÇÃO da animação de fade-in
    mSceneManagerState = SceneManagerState::FadeIn; // Inicia no estado de FADE-IN

    return true;
}

void Game::SetGameScene(Game::GameScene scene, float transitionTime)
{
    // Se uma transição já estiver em andamento, não faça nada.
    if (mSceneManagerState != SceneManagerState::None)
    {
        SDL_Log("SceneManager is busy. Transition to a new scene was ignored.");
        return;
    }

    // Se o código chegou até aqui, 'scene' é um valor válido do enum GameScene.
    mSceneManagerState = SceneManagerState::Entering;
    mNextScene = scene;
    mSceneManagerTimer = transitionTime;

}

void Game::ResetGameScene(float transitionTime)
{
    SetGameScene(mGameScene, transitionTime);
}

void Game::ChangeScene()
{
    // Unload current Scene
    UnloadScene();

    // Reset camera position
    mCameraPos.Set(0.0f, 0.0f);

    // Reset game timer
    mGameTimer = 0.0f;

    // Reset gameplay state
    mGamePlayState = GamePlayState::Playing;

    // Reset scene manager state
    mSpatialHashing = new SpatialHashing(TILE_SIZE * 4.0f, LEVEL_WIDTH * TILE_SIZE, LEVEL_HEIGHT * TILE_SIZE);

    mSceneManagerState = SceneManagerState::FadeIn;


    if (mNextScene == GameScene::Intro) {
        mBackgroundColor.Set(0.0f, 0.0f, 0.0f);

        auto introScreen = new UIScreen(this, "Assets/Fonts/SMB.ttf");

        const float originalWidth = 512.0f;
        const float originalHeight = 420.0f;
        const float aspectRatio = originalWidth / originalHeight;
        Vector2 newImageSize;
        newImageSize.y = mWindowHeight * (2.0f / 3.0f);
        newImageSize.x = newImageSize.y * aspectRatio;
        Vector2 newImagePos;
        newImagePos.x = (mWindowWidth - newImageSize.x) / 2.0f;
        newImagePos.y = (mWindowHeight - newImageSize.y) / 2.0f;
        introScreen->AddImage("Assets/Sprites/Intro.png", newImagePos, newImageSize, Vector3(255, 255, 255));

        const int mainTextPointSize = 48;
        const float mainTextRenderedHeight = mainTextPointSize / 3.0f; // 16px
        const float mainTextCharWidth = mainTextRenderedHeight * 0.8f;
        const float mainTextBoxHeight = mainTextRenderedHeight * 1.2f;

        std::string mainTextStr = "Nao Grita Game Studios";
        float mainTextWidth = mainTextStr.length() * mainTextCharWidth;

        Vector2 mainTextDims(mainTextWidth, mainTextBoxHeight);
        Vector2 mainTextPos;
        mainTextPos.x = (mWindowWidth - mainTextWidth) / 2.0f;
        mainTextPos.y = newImagePos.y - mainTextBoxHeight - 24.0f; // 24px acima da imagem

        introScreen->AddText(mainTextStr, mainTextPos, mainTextDims, mainTextPointSize, 0);

        const int subTextPointSize = mainTextPointSize;
        const float subTextRenderedHeight = subTextPointSize / 3.0f;
        const float subTextCharWidth = subTextRenderedHeight * 0.8f;
        const float subTextBoxHeight = subTextRenderedHeight * 1.2f;

        std::string subTextStr = "presents";
        float subTextWidth = subTextStr.length() * subTextCharWidth;

        Vector2 subTextDims(subTextWidth, subTextBoxHeight);
        Vector2 subTextPos;
        subTextPos.x = (mWindowWidth - subTextWidth) / 2.0f;
        subTextPos.y = newImagePos.y + newImageSize.y + 24.0f; // 24px abaixo da imagem

        introScreen->AddText(subTextStr, subTextPos, subTextDims, subTextPointSize, 0);

        mAudio->PlaySound("Intro.mp3", 0);
    }
    else if (mNextScene == GameScene::MainMenu)
    {
        // Set background color
        mBackgroundColor.Set(107.0f, 140.0f, 255.0f);

        // Initialize main menu actors
        LoadMainMenu();

        mAudio->PlaySound("MainMenu_Goetia.mp3", true);
    }
    else if (mNextScene == GameScene::Level1)
    {
        mHUD = new HUD(this, "Assets/Fonts/SMB.ttf");

        mGameTimeLimit = 400;
        mHUD->SetTime(mGameTimeLimit);
        mHUD->SetLevelName("1-1");

        mAudio->StopAllSounds();
        mAudio->PlayMusic("Level1_NhacNhatBanHay.mp3",true,13);

        // Set background color
        mBackgroundColor.Set(245.0f, 230.0f, 190.0f);

        // Set background color
        SetBackgroundImage("Assets/Sprites/level1-background.png", Vector2(TILE_SIZE,0), Vector2(6784,448));

        // Initialize actors
        LoadLevel("Assets/Levels/Level1.csv", LEVEL_WIDTH, LEVEL_HEIGHT);
    }
    else if (mNextScene == GameScene::Level2)
    {
        mHUD = new HUD(this, "Assets/Fonts/SMB.ttf");

        mGameTimeLimit = 400;
        mHUD->SetTime(mGameTimeLimit);
        mHUD->SetLevelName("1-2");

        mAudio->StopAllSounds();
        mAudio->PlayMusic("FinalFight_Inferia.mp3",true,13);

        // Set background color
        mBackgroundColor.Set(245.0f, 230.0f, 190.0f);

        // Set background color
        SetBackgroundImage("Assets/Sprites/level1-background.png", Vector2(TILE_SIZE,0), Vector2(6784,448));

        LoadLevel("Assets/Levels/Level2.csv", LEVEL_WIDTH, LEVEL_HEIGHT);
    }

    // Set new scene
    mGameScene = mNextScene;
}


void Game::LoadMainMenu()
{
    mAudio->StopAllSounds();

    auto mainMenu = new UIScreen(this, "Assets/Fonts/SMB.ttf");
    const Vector2 titleSize = Vector2(178.0f, 88.0f) * 2.0f;
    const Vector2 titlePos = Vector2(mWindowWidth/2.0f - titleSize.x/2.0f, 64.0f);

    SetBackgroundImage("Assets/Sprites/MainMenu.png", Vector2(0,0), Vector2(mWindowWidth, mWindowHeight));
    //mainMenu->AddImage("Assets/Sprites/Logo.png", titlePos, titleSize);

    mainMenu->AddButton("START GAME", Vector2(mWindowWidth/2.0f - 110.0f, 350.0f), Vector2(200.0f, 40.0f),
    [this]() { SetGameScene(GameScene::Level1);});
    //mainMenu->AddButton("2 PLAYER GAME", Vector2(mWindowWidth/2.0f - 115.0f, 314.0f), Vector2(200.0f, 40.0f),
    //                                       nullptr);
    mainMenu->AddButton("QUIT", Vector2(mWindowWidth/2.0f - 50.0f, 400.0f), Vector2(200.0f, 40.0f),
        [this]() {mIsRunning = false;});
}

void Game::LoadLevel(const std::string& levelName, const int levelWidth, const int levelHeight)
{
    // Load level data
    int **mLevelData = ReadLevelData(levelName, levelWidth, levelHeight);

    if (!mLevelData) {
        SDL_Log("Failed to load level data");
        return;
    }

    // Instantiate level actors
    BuildLevel(mLevelData, levelWidth, levelHeight);
}

void Game::BuildLevel(int** levelData, int width, int height)
{

    // Const map to convert tile ID to block type
    const std::map<int, const std::string> tileMap = {
            {0, "Assets/Sprites/Blocks/japan/bamboo.png"},
            {1, "Assets/Sprites/Blocks/japan/dirt.png"},
            {2, "Assets/Sprites/Blocks/japan/grass.png"},
            {3, "Assets/Sprites/Blocks/japan/stone.png"},
            {4, "Assets/Sprites/Blocks/japan/stone2.png"},
            {5, "Assets/Sprites/Blocks/japan/wood.png"},
            {6, "Assets/Sprites/Blocks/japan/wood2.png"},
            {7, "Assets/Sprites/Blocks/japan/woodengate.png"},
            {8, "Assets/Sprites/Blocks/japan/woodengate2.png"}
    };

    for (int y = 0; y < LEVEL_HEIGHT; ++y)
    {
        for (int x = 0; x < LEVEL_WIDTH; ++x)
        {
            int tile = levelData[y][x];

            if(tile == 9) // Samurai
            {
                mMario = new Player(this);
                mMario->SetPosition(Vector2((x) * TILE_SIZE, (y) * TILE_SIZE));
            }
            if(tile == 11) // Flying Demon
            {

                //auto demon = new FlyingDemon(this, Vector2((x) * TILE_SIZE, (y) * TILE_SIZE), 6.0f);
                //demon->SetPosition(Vector2((x) * TILE_SIZE, (y) * TILE_SIZE));
            }
            else if (tile == 12) // Demon Boss
            {
                //auto demonBoss = new DemonBoss(this);
                //demonBoss->SetPosition(Vector2((x) * TILE_SIZE, (y) * TILE_SIZE));
            }
            else if(tile == 10) // Spawner
            {
                Spawner* spawner = new Spawner(this, SPAWN_DISTANCE);
                spawner->SetPosition(Vector2(x * TILE_SIZE, y * TILE_SIZE));
            }
            else if (tile == 13) // End level trigger
            {
                Trigger* trigger = new Trigger(this);
                trigger->SetPosition(Vector2(x * TILE_SIZE, y * TILE_SIZE));
            }
            /*else if (tile == 1) { // Special Block With One Coin
                Block* block = new Block(this, "Assets/Sprites/Blocks/BlockC.png", 1);
                block->SetPosition(Vector2(x * TILE_SIZE, y * TILE_SIZE));
            }
            else if (tile == 14) { //Special Block With One Mushroom
                Block* block = new Block(this, "Assets/Sprites/Blocks/BlockC.png", 0,true,true);
                block->SetPosition(Vector2(x * TILE_SIZE, y * TILE_SIZE));
            }
            else if (tile == 30) {
                Block* block = new Block(this, "Assets/Sprites/Blocks/BlockB.png", 10);
                block->SetPosition(Vector2(x * TILE_SIZE, y * TILE_SIZE));
            }*/
            else // Blocks
            {
                auto it = tileMap.find(tile);
                if (it != tileMap.end())
                {
                    // Create a block actor
                    Block* block = new Block(this, it->second);
                    block->SetPosition(Vector2(x * TILE_SIZE, y * TILE_SIZE));
                }
            }
        }
    }
}

int **Game::ReadLevelData(const std::string& fileName, int width, int height)
{
    std::ifstream file(fileName);
    if (!file.is_open())
    {
        SDL_Log("Failed to load paths: %s", fileName.c_str());
        return nullptr;
    }

    // Create a 2D array of size width and height to store the level data
    int** levelData = new int*[height];
    for (int i = 0; i < height; ++i)
    {
        levelData[i] = new int[width];
    }

    // Read the file line by line
    int row = 0;

    std::string line;
    while (!file.eof())
    {
        std::getline(file, line);
        if(!line.empty())
        {
            auto tiles = CSVHelper::Split(line);

            if (tiles.size() != width) {
                SDL_Log("Invalid level data");
                return nullptr;
            }

            for (int i = 0; i < width; ++i) {
                levelData[row][i] = tiles[i];
            }
        }

        ++row;
    }

    // Close the file
    file.close();

    return levelData;
}

void Game::RunLoop()
{
    while (mIsRunning)
    {
        ProcessInput();
        UpdateGame();
        GenerateOutput();
    }
}

void Game::ProcessInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                Quit();
                break;
            case SDL_KEYDOWN:
                // Handle key press for UI screens
                if (!mUIStack.empty()) {
                    mUIStack.back()->HandleKeyPress(event.key.keysym.sym);
                }

                HandleKeyPressActors(event.key.keysym.sym, event.key.repeat == 0);

                // Check if the Return key has been pressed to pause/unpause the game
                if (event.key.keysym.sym == SDLK_RETURN)
                {
                    TogglePause();
                }
                break;
        }
    }

    ProcessInputActors();
}

void Game::ProcessInputActors()
{
    if(mGamePlayState == GamePlayState::Playing)
    {
        // Get actors on camera
        std::vector<Actor*> actorsOnCamera =
                mSpatialHashing->QueryOnCamera(mCameraPos,mWindowWidth,mWindowHeight);

        const Uint8* state = SDL_GetKeyboardState(nullptr);

        bool isMarioOnCamera = false;
        for (auto actor: actorsOnCamera)
        {
            actor->ProcessInput(state);

            if (actor == mMario) {
                isMarioOnCamera = true;
            }
        }

        // If Mario is not on camera, process input for him
        if (!isMarioOnCamera && mMario) {
            mMario->ProcessInput(state);
        }
    }
}

void Game::HandleKeyPressActors(const int key, const bool isPressed)
{
    if(mGamePlayState == GamePlayState::Playing)
    {
        // Get actors on camera
        std::vector<Actor*> actorsOnCamera =
                mSpatialHashing->QueryOnCamera(mCameraPos,mWindowWidth,mWindowHeight);

        // Handle key press for actors
        bool isMarioOnCamera = false;
        for (auto actor: actorsOnCamera) {
            actor->HandleKeyPress(key, isPressed);

            if (actor == mMario) {
                isMarioOnCamera = true;
            }
        }

        // If Mario is not on camera, handle key press for him
        if (!isMarioOnCamera && mMario)
        {
            mMario->HandleKeyPress(key, isPressed);
        }
    }

}

void Game::TogglePause()
{

    if (mGameScene != GameScene::MainMenu)
    {
        if (mGamePlayState == GamePlayState::Playing)
        {
            mGamePlayState = GamePlayState::Paused;

            mAudio->PauseSound(mMusicHandle);
            mAudio->PlaySound("Coin.wav");
        }
        else if (mGamePlayState == GamePlayState::Paused)
        {
            mGamePlayState = GamePlayState::Playing;

            mAudio->ResumeSound(mMusicHandle);
            mAudio->PlaySound("Coin.wav");
        }
    }
}

void Game::UpdateGame()
{
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16));

    float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
    if (deltaTime > 0.05f)
    {
        deltaTime = 0.05f;
    }

    mTicksCount = SDL_GetTicks();

    if(mGamePlayState != GamePlayState::Paused && mGamePlayState != GamePlayState::GameOver)
    {
        // Reinsert all actors and pending actors
        UpdateActors(deltaTime);
    }

    if (mGamePlayState == GamePlayState::LevelComplete) {
            // If the level is complete, change to the next scene
        if (mGameScene == GameScene::Level1) {
            SetGameScene(GameScene::Level2);
        } else if (mGameScene == GameScene::Level2) {
            SetGameScene(GameScene::MainMenu);
        }
    }

    // Reinsert audio system
    mAudio->Update(deltaTime);

    // Reinsert UI screens
    for (auto ui : mUIStack) {
        if (ui->GetState() == UIScreen::UIState::Active) {
            ui->Update(deltaTime);
        }
    }

    // Delete any UIElements that are closed
    auto iter = mUIStack.begin();
    while (iter != mUIStack.end()) {
        if ((*iter)->GetState() == UIScreen::UIState::Closing) {
            delete *iter;
            iter = mUIStack.erase(iter);
        } else {
            ++iter;
        }
    }

    // ---------------------
    // Game Specific Updates
    // ---------------------
    UpdateCamera();

    UpdateSceneManager(deltaTime);

    if (mGameScene != GameScene::Intro && mGameScene != GameScene::MainMenu && mGamePlayState == GamePlayState::Playing)
        UpdateLevelTime(deltaTime);
}

void Game::UpdateSceneManager(float deltaTime) {
    // FSM de Transição
    if (SceneManagerState::Entering == mSceneManagerState) {
        // O estado 'Entering' serve como um pequeno buffer antes do FadeOut.
        mSceneManagerTimer -= deltaTime;
        if (mSceneManagerTimer <= 0) {
            mSceneManagerTimer = TRANSITION_TIME;
            mSceneManagerState = SceneManagerState::FadeOut;
        }
    }
    else if (SceneManagerState::FadeOut == mSceneManagerState) {
        mSceneManagerTimer -= deltaTime;
        if (mSceneManagerTimer <= 0) {
            ChangeScene(); // Troca de cena no ponto mais escuro
            mSceneManagerTimer = TRANSITION_TIME; // Prepara timer para o FadeIn
            mSceneManagerState = SceneManagerState::FadeIn;
        }
    }
    else if (SceneManagerState::FadeIn == mSceneManagerState) {
        mSceneManagerTimer -= deltaTime;
        if (mSceneManagerTimer <= 0) {
            mSceneManagerState = SceneManagerState::None; // Transição terminou
            if (mGameScene == GameScene::Intro) {
                mSceneManagerTimer = INTRO_TIME;
            } else {
                mSceneManagerTimer = 0.0f; // Zera para outras cenas
            }
        }
    }

    // Lógica Específica da Cena (só executa quando não há transição)
    if (mSceneManagerState == SceneManagerState::None) {
        if (mGameScene == GameScene::Intro) {
            mSceneManagerTimer -= deltaTime;
            if (mSceneManagerTimer <= 0) {
                SetGameScene(GameScene::MainMenu);
            }
        }
    }
}


void Game::UpdateLevelTime(float deltaTime)
{
    mGameTimer += deltaTime;
    if (mGameTimer >= 1.0) {
        mGameTimer = 0.0f;
        mGameTimeLimit -= 1.0f;
        mHUD->SetTime(mGameTimeLimit);
    }

    if (mGameTimeLimit <= 0.0f) {
        mMario->Kill();
    }
}

void Game::UpdateCamera()
{
    if (!mMario) return;

    float horizontalCameraPos = mMario->GetPosition().x - (mWindowWidth / 2.0f);

    if (horizontalCameraPos > mCameraPos.x && !mIsCameraLocked)
    {
        // Limit camera to the right side of the level
        float maxCameraPos = (LEVEL_WIDTH * TILE_SIZE) - mWindowWidth;
        horizontalCameraPos = Math::Clamp(horizontalCameraPos, 0.0f, maxCameraPos);

        mCameraPos.x = horizontalCameraPos;
    }
}

void Game::UpdateActors(float deltaTime)
{
    std::vector<Actor*> actorsOnCamera =
        mSpatialHashing->QueryOnCamera(mCameraPos, mWindowWidth, mWindowHeight);

    bool isMarioOnCamera = false;
    for (auto actor : actorsOnCamera)
    {
        actor->Update(deltaTime);
        if (actor == mMario)
        {
            isMarioOnCamera = true;
        }
    }

    if (!isMarioOnCamera && mMario)
    {
        mMario->Update(deltaTime);
    }

    std::vector<Actor*> actorsToDestroy;

    for (auto actor : actorsOnCamera)
    {
        if (actor->GetState() == ActorState::Destroy)
        {
            actorsToDestroy.push_back(actor);
        }
    }

    for (auto actor : actorsToDestroy)
    {
        if (actor == mMario) {
            mMario = nullptr;
        }
        delete actor;
    }
}

void Game::AddActor(Actor* actor)
{
    mSpatialHashing->Insert(actor);
}

void Game::RemoveActor(Actor* actor)
{
    mSpatialHashing->Remove(actor);
}
void Game::Reinsert(Actor* actor)
{
    mSpatialHashing->Reinsert(actor);
}

std::vector<Actor *> Game::GetNearbyActors(const Vector2& position, const int range)
{
    return mSpatialHashing->Query(position, range);
}

std::vector<AABBColliderComponent *> Game::GetNearbyColliders(const Vector2& position, const int range)
{
    return mSpatialHashing->QueryColliders(position, range);
}

void Game::GenerateOutput()
{
    // Clear frame with background color
    SDL_SetRenderDrawColor(mRenderer, mBackgroundColor.x, mBackgroundColor.y, mBackgroundColor.z, 255);

    // Clear back buffer
    SDL_RenderClear(mRenderer);

    // Draw background texture considering camera position
    if (mBackgroundTexture)
    {
        SDL_Rect dstRect = { static_cast<int>(mBackgroundPosition.x - mCameraPos.x),
                             static_cast<int>(mBackgroundPosition.y - mCameraPos.y),
                             static_cast<int>(mBackgroundSize.x),
                             static_cast<int>(mBackgroundSize.y) };

        SDL_RenderCopy(mRenderer, mBackgroundTexture, nullptr, &dstRect);
    }

    // Get actors on camera
    std::vector<Actor*> actorsOnCamera =
            mSpatialHashing->QueryOnCamera(mCameraPos,mWindowWidth,mWindowHeight);

    // Get list of drawables in draw order
    std::vector<DrawComponent*> drawables;

    for (auto actor : actorsOnCamera)
    {
        auto drawable = actor->GetComponent<DrawComponent>();
        if (drawable && drawable->IsVisible())
        {
            drawables.emplace_back(drawable);
        }
    }

    // Sort drawables by draw order
    std::sort(drawables.begin(), drawables.end(),
              [](const DrawComponent* a, const DrawComponent* b) {
                  return a->GetDrawOrder() < b->GetDrawOrder();
              });

    // Draw all drawables
    for (auto drawable : drawables)
    {
        drawable->Draw(mRenderer, mModColor);
    }

    // Draw all UI screens
    for (auto ui :mUIStack)
    {
        ui->Draw(mRenderer);
    }

    if (SceneManagerState::FadeOut == mSceneManagerState){
        float alpha = (TRANSITION_TIME - mSceneManagerTimer) / TRANSITION_TIME;
        alpha = std::clamp(alpha, 0.0f, 1.0f); // Usar std::clamp é mais moderno
        SDL_SetRenderDrawBlendMode(mRenderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, static_cast<Uint8>(255 * alpha));
        SDL_RenderFillRect(mRenderer, nullptr);
    }
    else if (SceneManagerState::FadeIn == mSceneManagerState) {
        float alpha = mSceneManagerTimer / TRANSITION_TIME;
        alpha = std::clamp(alpha, 0.0f, 1.0f);
        SDL_SetRenderDrawBlendMode(mRenderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, static_cast<Uint8>(255 * alpha));
        SDL_RenderFillRect(mRenderer, nullptr);
    }



    // Swap front buffer and back buffer
    SDL_RenderPresent(mRenderer);
}

void Game::SetBackgroundImage(const std::string& texturePath, const Vector2 &position, const Vector2 &size)
{
    // Apenas pegue a textura do nosso gerenciador.
    // A chamada a LoadTexture agora é inteligente: se a textura já existir no cache,
    // ela retorna o ponteiro existente; senão, ela carrega, armazena no cache e retorna.
    mBackgroundTexture = LoadTexture(texturePath);
    if (!mBackgroundTexture) {
        SDL_Log("Failed to load background texture: %s", texturePath.c_str());
    }

    // Set background position
    mBackgroundPosition.Set(position.x, position.y);

    // Set background size
    mBackgroundSize.Set(size.x, size.y);
}

// Em Game.cpp
SDL_Texture* Game::LoadTexture(const std::string& texturePath)
{
    auto iter = mTextures.find(texturePath);
    if (iter != mTextures.end())
    {
        return iter->second;
    }
    else
    {
        SDL_Surface* surface = IMG_Load(texturePath.c_str());
        if (!surface) {
            SDL_Log("Failed to load image: %s", IMG_GetError());
            return nullptr;
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(mRenderer, surface);
        SDL_FreeSurface(surface);

        if (!texture) {
            SDL_Log("Failed to create texture: %s", SDL_GetError());
            return nullptr;
        }

        mTextures[texturePath] = texture;

        return texture;
    }
}


UIFont* Game::LoadFont(const std::string& fileName)
{
    auto it = mFonts.find(fileName);
    if (it != mFonts.end())
    {
        return it->second;
    }
    else
    {
        UIFont* font = new UIFont(mRenderer);
        if (font->Load(fileName))
        {
            mFonts[fileName] = font;
            return font;
        }
        else
        {
            font->Unload();
            delete font;
        }
    }
    return nullptr;
}


void Game::UnloadScene()
{
    // Delete actors
    delete mSpatialHashing;

    // Delete UI screens
    for (auto ui : mUIStack) {
        delete ui;
    }
    mUIStack.clear();

    // Delete background texture
    for (auto const& [path, texture] : mTextures)
    {
        SDL_DestroyTexture(texture);
    }
    mTextures.clear(); // Limpa o mapa para a próxima cena.

    mBackgroundTexture = nullptr;
}

void Game::Shutdown()
{
    UnloadScene();

    for (auto font : mFonts) {
        font.second->Unload();
        delete font.second;
    }
    mFonts.clear();

    for (auto const& [path, texture] : mTextures)
    {
        SDL_DestroyTexture(texture);
    }
    mTextures.clear();

    delete mAudio;
    mAudio = nullptr;

    Mix_CloseAudio();

    Mix_Quit();
    TTF_Quit();
    IMG_Quit();

    SDL_DestroyRenderer(mRenderer);
    SDL_DestroyWindow(mWindow);
    SDL_Quit();
}