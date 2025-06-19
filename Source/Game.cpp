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
#include "Actors/Mario.h"
#include "Actors/Block.h"
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
    mNextScene = GameScene::Intro;
    ChangeScene();

    // Garanta que a máquina de estados comece desligada.
    //mSceneManagerState = SceneManagerState::None;

    return true;
}

void Game::SetGameScene(Game::GameScene scene, float transitionTime)
{
    if (mSceneManagerState == SceneManagerState::None) {
        if (scene == GameScene::MainMenu || scene == GameScene::Level1 || scene == GameScene::Level2) {
            mSceneManagerState = SceneManagerState::Entering;
            mNextScene = scene;
            mSceneManagerTimer = transitionTime;
        }
        else {
            SDL_Log("Invalid scene state");
            return;
        }
    }
    else {
        SDL_Log("Invalid SceneManager Busy");
        return;
    }
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

    // Scene Manager FSM: using if/else instead of switch
    if (mNextScene == GameScene::Intro) {
        // Cor de fundo preta
        mBackgroundColor.Set(0.0f, 0.0f, 0.0f);

        // Crie uma tela de UI para a intro
        auto introScreen = new UIScreen(this, "Assets/Fonts/SMB.ttf");

        introScreen->AddImage("Assets/Sprites/Intro.png", Vector2(50.0f, 0.0f), Vector2(512.0f, 420.0f));

        // Timer para transição automática (ex: 2 segundos)
        mSceneManagerTimer = TRANSITION_TIME;
    }
    else if (mNextScene == GameScene::MainMenu)
    {
        // Set background color
        mBackgroundColor.Set(107.0f, 140.0f, 255.0f);

        // Initialize main menu actors
        LoadMainMenu();
    }
    else if (mNextScene == GameScene::Level1)
    {
        mHUD = new HUD(this, "Assets/Fonts/SMB.ttf");

        mGameTimeLimit = 400;
        mHUD->SetTime(mGameTimeLimit);
        mHUD->SetLevelName("1-1");

        mMusicHandle = mAudio->PlaySound("MusicMain.ogg",true);

        // Set background color
        mBackgroundColor.Set(107.0f, 140.0f, 255.0f);

        // Set background color
        SetBackgroundImage("Assets/Sprites/Background.png", Vector2(TILE_SIZE,0), Vector2(6784,448));

        // Draw Flag
        auto flag = new Actor(this);
        flag->SetPosition(Vector2(LEVEL_WIDTH * TILE_SIZE - (16 * TILE_SIZE) - 16, 3 * TILE_SIZE));

        // Add a flag sprite
        new DrawSpriteComponent(flag, "Assets/Sprites/Background_Flag.png", 32.0f, 32.0f, 1);

        // Add a flag pole taking the entire height
        new AABBColliderComponent(flag, 30, 0, 4, TILE_SIZE * LEVEL_HEIGHT, ColliderLayer::Pole, true);

        // Initialize actors
        LoadLevel("Assets/Levels/level1-1.csv", LEVEL_WIDTH, LEVEL_HEIGHT);
    }
    else if (mNextScene == GameScene::Level2)
    {
        mHUD = new HUD(this, "Assets/Fonts/SMB.ttf");

        mGameTimeLimit = 400;
        mHUD->SetTime(mGameTimeLimit);
        mHUD->SetLevelName("1-2");

        mMusicHandle = mAudio->PlaySound("MusicUnderground.ogg",true);

        // Set background color
        mBackgroundColor.Set(0.0f, 0.0f, 0.0f);

        // Set mod color
        mModColor.Set(0.0f, 255.0f, 200.0f);

        // Initialize actors
        LoadLevel("Assets/Levels/level1-2.csv", LEVEL_WIDTH, LEVEL_HEIGHT);
    }

    // Set new scene
    mGameScene = mNextScene;
}


void Game::LoadMainMenu()
{
    auto mainMenu = new UIScreen(this, "Assets/Fonts/SMB.ttf");
    const Vector2 titleSize = Vector2(178.0f, 88.0f) * 2.0f;
    const Vector2 titlePos = Vector2(mWindowWidth/2.0f - titleSize.x/2.0f, 64.0f);

    SetBackgroundImage("Assets/Sprites/MarioBG.png", Vector2(0,5), Vector2(1280/2.0,892/2.0));
    mainMenu->AddImage("Assets/Sprites/Logo.png", titlePos, titleSize);

    mainMenu->AddButton("1 PLAYER GAME", Vector2(mWindowWidth/2.0f - 115.0f, 264.0f), Vector2(200.0f, 40.0f),
    [this]() { SetGameScene(GameScene::Level1);});
    mainMenu->AddButton("2 PLAYER GAME", Vector2(mWindowWidth/2.0f - 115.0f, 314.0f), Vector2(200.0f, 40.0f),
                                           nullptr);


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
            {0, "Assets/Sprites/Blocks/BlockA.png"},
            {1, "Assets/Sprites/Blocks/BlockC.png"},
            {2, "Assets/Sprites/Blocks/BlockF.png"},
            {4, "Assets/Sprites/Blocks/BlockB.png"},
            {6, "Assets/Sprites/Blocks/BlockI.png"},
            {8, "Assets/Sprites/Blocks/BlockD.png"},
            {9, "Assets/Sprites/Blocks/BlockH.png"},
            {12, "Assets/Sprites/Blocks/BlockG.png"}
    };

    for (int y = 0; y < LEVEL_HEIGHT; ++y)
    {
        for (int x = 0; x < LEVEL_WIDTH; ++x)
        {
            int tile = levelData[y][x];

            if(tile == 16) // Mario
            {
                mMario = new Mario(this);
                mMario->SetPosition(Vector2((x) * TILE_SIZE, (y) * TILE_SIZE));
            }
            else if(tile == 10) // Spawner
            {
                Spawner* spawner = new Spawner(this, SPAWN_DISTANCE);
                spawner->SetPosition(Vector2(x * TILE_SIZE, y * TILE_SIZE));
            }
            else if (tile == 1) { // Special Block With One Coin
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
            }
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
    if (mGameScene == GameScene::Intro) {
        mSceneManagerTimer -= deltaTime;
        if (mSceneManagerTimer <= 0) {
            mNextScene = GameScene::MainMenu;
            ChangeScene();
            mSceneManagerState = SceneManagerState::FadeOut;
        }
        return;
    }

    if (SceneManagerState::Entering == mSceneManagerState) {
        mSceneManagerTimer-=deltaTime;
        if (mSceneManagerTimer <= 0) {
            mSceneManagerTimer = TRANSITION_TIME;
            mSceneManagerState = SceneManagerState::FadeOut;
        }
    }

    if (mNextScene == GameScene::MainMenu && mGameScene != GameScene::MainMenu) {
        ChangeScene();
        mSceneManagerState = SceneManagerState::None;
    }

    if (SceneManagerState::FadeOut == mSceneManagerState) {
        mSceneManagerTimer-=deltaTime;
        if (mSceneManagerTimer <= 0) {
            mSceneManagerTimer = TRANSITION_TIME;
            ChangeScene();
            mSceneManagerState = SceneManagerState::FadeIn;
        }
    }
    else if (SceneManagerState::FadeIn == mSceneManagerState) {
        mSceneManagerTimer-=deltaTime;
        if (mSceneManagerTimer <= 0) {
            mSceneManagerState = SceneManagerState::None;
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

    if (horizontalCameraPos > mCameraPos.x)
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
        float alphaOut =  (1 - mSceneManagerTimer);
        SDL_SetRenderDrawBlendMode(mRenderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255 * alphaOut);
        SDL_RenderFillRect(mRenderer, nullptr);
    }
    else if (SceneManagerState::FadeIn == mSceneManagerState) {
        float alphaIn = 1 - (1 - mSceneManagerTimer);
        SDL_SetRenderDrawBlendMode(mRenderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255 * alphaIn);
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