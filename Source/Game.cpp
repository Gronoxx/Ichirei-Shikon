#include <algorithm>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <memory>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include "CSV.h"
#include "Random.h"
#include "Game.h"
#include "UIElements/UIHud.h"
#include "SpatialHashing.h"
#include "UIPauseMenuScreen.h"
#include "Actors/Actor.h"
#include "Actors/Player.h"
#include "Actors/Block.h"
#include "Actors/DemonBoss.h"
#include "Actors/FlyingDemon.h"
#include "Actors/Trigger.h"
#include "Actors/Spawner.h"
#include "Actors/BlockSpriteSheet.h"
#include "UIElements/UIScreen.h"
#include "Components/DrawComponents/DrawComponent.h"
#include "Components/ColliderComponents/AABBColliderComponent.h"
#include "Components/DrawComponents/DrawAnimatedComponent.h"
#include "Actors/InvisibleBlock.h"
#include "Components/ColliderComponents/PolygonColliderComponent.h"

const float BOSS_LEVEL_WIDTH = 938.0f;
const float BOSS_LEVEL_HEIGHT = 366.0f;

Game::Game(const int windowWidth, const int windowHeight)
        :mSceneManagerState(SceneManagerState::None)
        ,mSceneManagerTimer(0.0f)
        ,mIsSceneManagerActive(false)
        ,mSpatialHashing(nullptr)
        ,mWindow(nullptr)
        ,mRenderer(nullptr)
        ,mAudio(nullptr)
        ,mWindowWidth(windowWidth)
        ,mWindowHeight(windowHeight)
        ,mTicksCount(0)
        ,mIsRunning(true)
        ,mGamePlayState(GamePlayState::NotStarted)
        ,mGameScene(GameScene::Intro)
        ,mNextScene(GameScene::Intro)
        ,mBackgroundColor(0, 0, 0)
        ,mModColor(255, 255, 255)
        ,mCameraPos(Vector2::Zero)
        ,mIsCameraLocked(false)
        ,mPlayer(nullptr)
        ,mHUD(nullptr)
        ,mGameTimer(0.0f)
        ,mGameTimeLimit(0)
        ,mBackgroundTexture(nullptr)
        ,mBackgroundSize(Vector2::Zero)
        ,mBackgroundPosition(Vector2::Zero)
        , mIsPauseMenuActive(false)
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

    // Start a random number generator
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

void Game::SetGameScene(const GameScene scene, const float transitionTime)
{
    // Se uma transição já estiver em andamento, não faça nada.
    if (mSceneManagerState != SceneManagerState::None)
    {
        SDL_Log("SceneManager is busy. Transition to a new scene was ignored.");
        return;
    }

    mSceneManagerState = SceneManagerState::Entering;
    mNextScene = scene;
    mSceneManagerTimer = transitionTime;
}

void Game::ResetGameScene(const float transitionTime)
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

        constexpr float originalWidth = 512.0f;
        constexpr float originalHeight = 420.0f;
        constexpr float aspectRatio = originalWidth / originalHeight;
        Vector2 newImageSize;
        newImageSize.y = static_cast<float>(mWindowHeight) * (2.0f / 3.0f);
        newImageSize.x = newImageSize.y * aspectRatio;
        Vector2 newImagePos;
        newImagePos.x = (static_cast<float>(mWindowWidth) - newImageSize.x) / 2.0f;
        newImagePos.y = (static_cast<float>(mWindowHeight) - newImageSize.y) / 2.0f;
        introScreen->AddImage("Assets/Sprites/Intro.png", newImagePos, newImageSize, Vector3(255, 255, 255));

        constexpr int mainTextPointSize = 48;
        constexpr float mainTextRenderedHeight = mainTextPointSize / 3.0f; // 16px
        constexpr float mainTextCharWidth = mainTextRenderedHeight * 0.8f;
        constexpr float mainTextBoxHeight = mainTextRenderedHeight * 1.2f;

        const std::string mainTextStr = "Nao Grita Game Studios";
        const float mainTextWidth = static_cast<float>(mainTextStr.length()) * mainTextCharWidth;
        const Vector2 mainTextDims(mainTextWidth, mainTextBoxHeight);
        Vector2 mainTextPos;
        mainTextPos.x = (static_cast<float>(mWindowWidth) - mainTextWidth) / 2.0f;
        mainTextPos.y = newImagePos.y - mainTextBoxHeight - 24.0f; // 24px acima da imagem        mainTextPos.y = newImagePos.y - mainTextBoxHeight - 24.0f; // 24px acima da imagem

        introScreen->AddText(mainTextStr, mainTextPos, mainTextDims, mainTextPointSize, 0);

        constexpr int subTextPointSize = mainTextPointSize;
        constexpr float subTextRenderedHeight = subTextPointSize / 3.0f;
        constexpr float subTextCharWidth = subTextRenderedHeight * 0.8f;
        constexpr float subTextBoxHeight = subTextRenderedHeight * 1.2f;

        const std::string subTextStr = "presents";
        const float subTextWidth = static_cast<float>(subTextStr.length()) * subTextCharWidth;
        const Vector2 subTextDims(subTextWidth, subTextBoxHeight);
        Vector2 subTextPos;
        subTextPos.x = (static_cast<float>(mWindowWidth) - subTextWidth) / 2.0f;
        subTextPos.y = newImagePos.y + newImageSize.y + 24.0f; // 24px abaixo da imagem

        introScreen->AddText(subTextStr, subTextPos, subTextDims, subTextPointSize, 0);

        mAudio->PlaySound("Intro.mp3", false);
    }
    else if (mNextScene == GameScene::MainMenu)
    {
        LoadMainMenu();
    }
    else if (mNextScene == GameScene::Level2)
    {
        mGameTimeLimit = 400;

        // Crie o HUD apenas uma vez, se necessário
        if (!mHUD)
        {
            mHUD = new UIHud(this, "Assets/Fonts/SMB.ttf", mRenderer);
            SDL_Log("mHUD: %p", mHUD);
        }

        // Defina todos os dados para o Nível 1
        LevelData level1_data;
        level1_data.nameForHUD = "1-1";
        level1_data.timeLimit = 400;
        level1_data.backgroundTilemapFile = "Assets/Levels/bg_layer_tiledata.csv";
        level1_data.actorFile = "Assets/Levels/level1.csv";
        level1_data.backgroundSpritesheet = "Assets/Sprites/spr_chinatown_background_0.png";
        level1_data.foregroundTilemapFile = "Assets/Levels/fg_layer_tiledata.csv";
        level1_data.foregroundSpritesheet = "Assets/Sprites/spr_chinatown_foreground_0.png";
        level1_data.musicFile = "Level1_NhacNhatBanHay.mp3";
        level1_data.width = LEVEL_WIDTH;
        level1_data.height = LEVEL_HEIGHT;

        // Chame a função genérica para carregar o nível
        LoadLevel(level1_data);
    }
    else if (mNextScene == GameScene::Level1)
    {
        LoadBossLevel();
    }

    // Set a new scene
    mGameScene = mNextScene;
    mIsSceneManagerActive = false;
}

void Game::LoadMainMenu()
{
    // Set background color
    mBackgroundColor.Set(0.0f, 0.0f, 0.0f);

    mAudio->StopAllSounds();
    mAudio->PlayMusic("MainMenu_Goetia.mp3", true, 50);

    const auto mainMenu = new UIScreen(this, "Assets/Fonts/SMB.ttf");

    SetBackgroundImage("Assets/Sprites/MainMenu.png", Vector2(0,0), Vector2(static_cast<float>(mWindowWidth), static_cast<float>(mWindowHeight)));

    mainMenu->AddButton("START GAME", Vector2(static_cast<float>(mWindowWidth)/2.0f - 110.0f, 300.0f), Vector2(200.0f, 40.0f),
        [this]() { SetGameScene(GameScene::Level1); });

    mainMenu->AddButton("CONTROLES", Vector2(static_cast<float>(mWindowWidth)/2.0f - 100.0f, 350.0f), Vector2(200.0f, 40.0f),
        [this]() { ShowTutorialScreen(); });

    mainMenu->AddButton("QUIT", Vector2(static_cast<float>(mWindowWidth)/2.0f - 50.0f, 400.0f), Vector2(200.0f, 40.0f),
        [this]() { mIsRunning = false; });
}

void Game::ShowTutorialScreen()
{
    auto tutorial = new UIScreen(this, "Assets/Fonts/SMB.ttf");
    Vector2 pos(0.0f, 0.0f);
    Vector2 size(static_cast<float>(mWindowWidth), static_cast<float>(mWindowHeight));

    // Fundo preto semitransparente cobrindo toda a tela
    tutorial->AddRect(pos, size, Vector4(0, 0, 0, 230));

    // Centraliza os textos verticalmente e horizontalmente
    float centerX = static_cast<float>(mWindowWidth) / 2.0f;
    float startY = static_cast<float>(mWindowHeight) / 2.0f - 120.0f;

    tutorial->AddText("CONTROLES", Vector2(centerX - 200.0f, startY), Vector2(400, 40), 32, 0);
    tutorial->AddText("WASD: Movimentação", Vector2(centerX - 200.0f, startY + 60), Vector2(400, 30), 24, 0);
    tutorial->AddText("Espaço: Pular", Vector2(centerX - 200.0f, startY + 100), Vector2(400, 30), 24, 0);
    tutorial->AddText("E: Ataque Fraco", Vector2(centerX - 200.0f, startY + 140), Vector2(400, 30), 24, 0);
    tutorial->AddText("SHIFT: Rolamento", Vector2(centerX - 200.0f, startY + 180), Vector2(400, 30), 24, 0);

    // Botão centralizado na parte inferior
    tutorial->AddButton("VOLTAR", Vector2(centerX - 75, startY + 240), Vector2(200, 40),
        [this, tutorial]() { tutorial->SetState(UIScreen::UIState::Closing); });
}

void Game::LoadLevel(const LevelData& data)
{
    // --- Etapa 1: Configurar HUD e Áudio ---
    mCurrentLevelPixelWidth = data.width * TILE_SIZE;
    mCurrentLevelPixelHeight = data.height * TILE_SIZE;
    mCurrentLevelOffset = Vector2::Zero;
    mAudio->StopAllSounds();
    mAudio->PlayMusic(data.musicFile, true, 13);

    // --- Etapa 2: Carregar e construir os ATORES ESPECIAIS ---
    // Carrega os dados para um std::vector, que é mais seguro.
    std::vector<std::vector<int>> actorData = LoadMapFromCSV(data.actorFile);

    // A verificação agora é para checar se o vetor não está vazio.
    if (!actorData.empty())
    {
        // Passa o vetor diretamente para a BuildLevel (que também será refatorada).
        BuildLevel(actorData);
    }
    else
    {
        SDL_Log("Could not load actor data for level: %s", data.actorFile.c_str());
    }

    // O bloco "delete[] actorData" foi REMOVIDO. O std::vector cuida da memória automaticamente!

    // --- Etapa 3: Carregar as CAMADAS DE TILES do cenário ---
    int tileSize = 32;
    int spritesheetColumns = 16;

    CreateLevelBoundaries(data.width, data.height);

    // Carrega a camada de fundo (background)
    LoadTilemapLayer(data.backgroundTilemapFile, data.backgroundSpritesheet, tileSize, spritesheetColumns, 5);

    // Carrega a camada da frente (foreground)
    LoadTilemapLayer(data.foregroundTilemapFile, data.foregroundSpritesheet, tileSize, spritesheetColumns, 15);
}

void Game::BuildLevel(const std::vector<std::vector<int>>& levelData)
{
    // Usamos o .size() do vetor para controlar os laços, tornando a função mais segura.
    for (size_t y = 0; y < levelData.size(); ++y)
    {
        for (size_t x = 0; x < levelData[y].size(); ++x)
        {
            int tile = levelData[y][x];

            if (tile == 9) // Samurai
            {
                mPlayer = new Player(this);
                mPlayer->SetPosition(Vector2(static_cast<float>(x) * TILE_SIZE, static_cast<float>(y) * TILE_SIZE));
                if (auto* drawComp = mPlayer->GetComponent<DrawAnimatedComponent>())
                {
                    drawComp->SetDrawOrder(20);
                }
            }
            else if (tile == 11) // Flying Demon
            {
                auto demon = new FlyingDemon(this, Vector2(0,0), 6.0f); // Posição alvo e lifetime de exemplo
                demon->SetPosition(Vector2(static_cast<float>(x) * TILE_SIZE, static_cast<float>(y) * TILE_SIZE));
                if (auto* drawComp = demon->GetComponent<DrawAnimatedComponent>())
                {
                    drawComp->SetDrawOrder(10);
                }
            }
            else if (tile == 12) // Demon Boss
            {
                auto demonBoss = new DemonBoss(this);
                demonBoss->SetPosition(Vector2(static_cast<float>(x) * TILE_SIZE, static_cast<float>(y) * TILE_SIZE));
                if (auto* drawComp = demonBoss->GetComponent<DrawAnimatedComponent>())
                {
                    drawComp->SetDrawOrder(10);
                }
            }
            else if (tile == 10) // Spawner
            {
                auto* spawner = new Spawner(this, SPAWN_DISTANCE);
                spawner->SetPosition(Vector2(static_cast<float>(x) * TILE_SIZE, static_cast<float>(y) * TILE_SIZE));
            }
            else if (tile == 13) // End level trigger
            {
                auto* trigger = new Trigger(this);
                trigger->SetPosition(Vector2(static_cast<float>(x) * TILE_SIZE, static_cast<float>(y) * TILE_SIZE));
            }
        }
    }
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
    const Uint8* state = SDL_GetKeyboardState(nullptr);

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

                if (mPlayer) {
                    mPlayer->HandleInput(state, &event);
                }

                // Check if the Return key has been pressed to pause/unpause the game
                if (event.key.keysym.sym == SDLK_RETURN && !mIsPauseMenuActive)
                {
                    TogglePause();
                }
                break;
            default:
                break;
        }
    }

    if (mPlayer) {
        mPlayer->HandleInput(state, nullptr);
    }
}

void Game::TogglePause()
{
    if (mGameScene != GameScene::MainMenu)
    {
        if (mGamePlayState == GamePlayState::Playing)
        {
            mGamePlayState = GamePlayState::Paused;
            if (Mix_PlayingMusic()) Mix_PauseMusic();
            // Create and push the pause menu screen onto the UI stack
            mUIStack.push_back(new UIPauseMenuScreen(this));
            mIsPauseMenuActive = true;
        }
        else if (mGamePlayState == GamePlayState::Paused)
        {
            mGamePlayState = GamePlayState::Playing;
            if (Mix_PausedMusic()) Mix_ResumeMusic();
            // Remove the pause menu from the stack
            mUIStack.pop_back();
        }
    }
}

void Game::UpdateGame()
{
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16)){}

    float deltaTime = static_cast<float>(SDL_GetTicks() - mTicksCount) / 1000.0f;
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

    // Update the game scenes
    UpdateScenes();

    // Update audio system
    mAudio->Update(deltaTime);

    // Update all UI elements
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

    // Update the camera position
    UpdateCamera();

    // Update Scene Manager
    UpdateSceneManager(deltaTime);


    if (mGameScene != GameScene::Intro && mGameScene != GameScene::MainMenu && mGamePlayState == GamePlayState::Playing)
        UpdateLevelTime(deltaTime);
}

void Game::UpdateScenes() {
    if (mGamePlayState == GamePlayState::LevelComplete && !mIsSceneManagerActive) {
        mIsSceneManagerActive = true;
        if (mGameScene == GameScene::Level1) {
            SetGameScene(GameScene::Level2);
        } else if (mGameScene == GameScene::Level2) {
            SetGameScene(GameScene::MainMenu);
        }
    }
}

void Game::UpdateSceneManager(float deltaTime) {
    // FSM de Transição
    if (SceneManagerState::Entering == mSceneManagerState) {
        // O estado 'Entering' serve como um pequeno ‘buffer’ antes do FadeOut.
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
                mSceneManagerTimer = 0.0f;
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

void Game::UpdateLevelTime(const float deltaTime)
{
    mGameTimer += deltaTime;
    if (mGameTimer >= 1.0) {
        mGameTimer = 0.0f;
        mGameTimeLimit -= 1.0f;
    }

    if (static_cast<float>(mGameTimeLimit) <= 0.0f) {
        mPlayer->Kill();
    }
}

void Game::UpdateCamera()
{
    // Etapa 0: Verificações iniciais
    if (!mPlayer || mIsCameraLocked)
    {
        return;
    }

    // ETAPA 1 e 2: A lógica da "Dead Zone" para calcular cameraDelta permanece a mesma.
    // ... (código da dead zone que você já tem) ...
    const Vector2 playerWorldCenter(mPlayer->GetPosition().x + (Game::TILE_SIZE / 2.0f), mPlayer->GetPosition().y + (Game::TILE_SIZE / 2.0f));
    const Vector2 playerScreenCenter(playerWorldCenter.x - mCameraPos.x, playerWorldCenter.y - mCameraPos.y);
    const float DEAD_ZONE_PERCENT_X = 0.4f;
    const float DEAD_ZONE_PERCENT_Y = 0.3f;
    const float deadZoneWidth = static_cast<float>(mWindowWidth) * DEAD_ZONE_PERCENT_X;
    const float deadZoneHeight = static_cast<float>(mWindowHeight) * DEAD_ZONE_PERCENT_Y;
    const float deadZoneLeft = (static_cast<float>(mWindowWidth) - deadZoneWidth) / 2.0f;
    const float deadZoneRight = deadZoneLeft + deadZoneWidth;
    const float deadZoneTop = (static_cast<float>(mWindowHeight) - deadZoneHeight) / 2.0f;
    const float deadZoneBottom = deadZoneTop + deadZoneHeight;
    Vector2 cameraDelta = Vector2::Zero;
    if (playerScreenCenter.x < deadZoneLeft) { cameraDelta.x = playerScreenCenter.x - deadZoneLeft; }
    else if (playerScreenCenter.x > deadZoneRight) { cameraDelta.x = playerScreenCenter.x - deadZoneRight; }
    if (playerScreenCenter.y < deadZoneTop) { cameraDelta.y = playerScreenCenter.y - deadZoneTop; }
    else if (playerScreenCenter.y > deadZoneBottom) { cameraDelta.y = playerScreenCenter.y - deadZoneBottom; }


    // --- ETAPA 3: APLICAR O MOVIMENTO E LIMITES (VERSÃO CORRIGIDA) ---

    // Aplica o movimento desejado à câmera
    mCameraPos.x += cameraDelta.x;
    mCameraPos.y += cameraDelta.y;

    // Calcula os limites da câmera usando as variáveis do nível ATUAL
    const float minCameraX = mCurrentLevelOffset.x;
    const float minCameraY = mCurrentLevelOffset.y;
    const float maxCameraX = mCurrentLevelOffset.x + mCurrentLevelPixelWidth - static_cast<float>(mWindowWidth);
    const float maxCameraY = mCurrentLevelOffset.y + mCurrentLevelPixelHeight - static_cast<float>(mWindowHeight);

    // Garante que a câmera não saia dos limites do nível.
    // Esta lógica funciona para níveis grandes e pequenos (centralizados).
    mCameraPos.x = Math::Clamp(mCameraPos.x, minCameraX, (maxCameraX < minCameraX ? minCameraX : maxCameraX));
    mCameraPos.y = Math::Clamp(mCameraPos.y, minCameraY, (maxCameraY < minCameraY ? minCameraY : maxCameraY));

    // --- ETAPA 4: LOG DE DEPURAÇÃO ---
    SDL_Log("--- Frame Câmera ---");
    SDL_Log("Offset Nível: (%.2f, %.2f)", mCurrentLevelOffset.x, mCurrentLevelOffset.y);
    SDL_Log("Câmera Limites Y: Min=%.2f, Max=%.2f", minCameraY, maxCameraY);
    SDL_Log("Pos Final Câmera: (%.2f, %.2f)", mCameraPos.x, mCameraPos.y);
    SDL_Log("----------------------\n");
}

void Game::UpdateActors(float deltaTime)
{
    std::vector<Actor*> actorsOnCamera;
    // if (mGameScene == GameScene::Level1) {
    //     actorsOnCamera = mSpatialHashing->QueryOnCamera(mCameraPos, static_cast<float>(mWindowWidth), static_cast<float>(mWindowHeight));
    // }

    if (mGameScene == GameScene::Level1) {
        SDL_Log(" Querying for Level2");

        actorsOnCamera = mSpatialHashing->QueryOnCamera(Vector2{0, 0},
        BOSS_LEVEL_WIDTH,
        BOSS_LEVEL_HEIGHT);
    }

    for (auto actor : actorsOnCamera)
        actor->Update(deltaTime);


    std::vector<Actor*> actorsToDestroy;
    for (auto actor : actorsOnCamera)
        if (actor->GetState() == ActorState::Destroy)
            actorsToDestroy.push_back(actor);

    for (auto actor : actorsToDestroy) {
        if (actor == mPlayer) {
            delete mPlayer;
            mPlayer = nullptr;
            continue;
        }
        delete actor;
    }
}

void Game::AddActor(Actor* actor) const {
    mSpatialHashing->Insert(actor);
}

void Game::RemoveActor(Actor* actor) const {
    mSpatialHashing->Remove(actor);
}

void Game::Reinsert(Actor* actor) const {
    mSpatialHashing->Reinsert(actor);
}

std::vector<Actor *> Game::GetNearbyActors(const Vector2& position, const int range) const {
    return mSpatialHashing->Query(position, range);
}

std::vector<AABBColliderComponent *> Game::GetNearbyColliders(const Vector2& position, const int range) const {
    return mSpatialHashing->QueryColliders(position, range);
}

void Game::GenerateOutput() const {
    // Clear frame with background color
    SDL_SetRenderDrawColor(mRenderer, static_cast<Uint8>(mBackgroundColor.x), static_cast<Uint8>(mBackgroundColor.y), static_cast<Uint8>(mBackgroundColor.z), 255);
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
            mSpatialHashing->QueryOnCamera(mCameraPos,static_cast<float>(mWindowWidth),static_cast<float>(mWindowHeight));

    // Get a list of drawables in draw order
    std::vector<DrawComponent*> drawables;

    // Collect all drawables that are visible
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

    // Draw fade effect if needed
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
    // Load the background texture
    mBackgroundTexture = LoadTexture(texturePath);
    if (!mBackgroundTexture) {
        SDL_Log("Failed to load background texture: %s", texturePath.c_str());
    }

    // Set the background position
    mBackgroundPosition.Set(position.x, position.y);

    // Set background size
    mBackgroundSize.Set(size.x, size.y);
}

SDL_Texture* Game::LoadTexture(const std::string& texturePath)
{
    if (const auto iter = mTextures.find(texturePath); iter != mTextures.end())
    {
        return iter->second;
    }
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

UIFont* Game::LoadFont(const std::string& fileName)
{
    if (const auto it = mFonts.find(fileName); it != mFonts.end())
    {
        return it->second.get();
    }

    auto font = std::make_unique<UIFont>(mRenderer);
    if (font->Load(fileName))
    {
        mFonts[fileName] = std::move(font);
        return mFonts[fileName].get();
    }

    return nullptr;
}

void Game::UnloadScene()
{
    // Delete actors
    auto actorsOnCamera = mSpatialHashing->QueryOnCamera(mCameraPos, static_cast<float>(mWindowWidth), static_cast<float>(mWindowHeight));
    for (auto actor : actorsOnCamera)
    {
        if (actor == mPlayer) {
            delete mPlayer;
            mPlayer = nullptr;
        } else {
            delete actor;
        }
    }

    delete mSpatialHashing;
    mSpatialHashing = nullptr;

    // Delete UI screens
    for (const auto ui : mUIStack) {
        if (ui == mHUD) {
            mHUD = nullptr;
        }
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
    mBackgroundColor = Vector3::Zero;
    mBackgroundPosition = Vector2::Zero;
    mBackgroundSize = Vector2::Zero;
}

void Game::Shutdown()
{
    UnloadScene();

    for (auto& pair : mFonts) {
        if (pair.second) {
            pair.second->Unload();
        }
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

bool IsCollisionExempt(int tileID)
{
    // Verifica se o ID está na primeira faixa (240 a 271)
    if (tileID >= 240 && tileID <= 271) {
        return true;
    }
    // Verifica se o ID está na segunda faixa (288 a 303)
    if (tileID >= 288 && tileID <= 303) {
        return true;
    }
    // Verifica se o ID está na terceira faixa (304 a 319)
    if (tileID >= 304 && tileID <= 319) {
        return true;
    }
    // Se não estiver em nenhuma das faixas, não é isento
    return false;
}

void Game::LoadTilemapLayer(const std::string& csvPath,
                            const std::string& spritesheetPath,
                            int tileSize,
                            int spritesheetColumns,
                            int drawOrder)
{
    // 1. Carregue os dados do mapa a partir do arquivo CSV.
    std::vector<std::vector<int>> tileIDs = LoadMapFromCSV(csvPath);

    // Se o CSV não pôde ser lido, tileIDs estará vazio.
    if (tileIDs.empty()) {
        SDL_Log("Falha ao carregar ou CSV vazio para: %s", csvPath.c_str());
        return;
    }

    int tilesCreated = 0;

    // 2. Itere sobre a matriz de IDs para criar cada tile.
    for (size_t y = 0; y < tileIDs.size(); ++y)
    {
        for (size_t x = 0; x < tileIDs[y].size(); ++x)
        {
            int currentTileID = tileIDs[y][x];

            if (currentTileID != 0)
            {
                // Crie uma nova instância de BlockSpriteSheet para este tile.
                BlockSpriteSheet* tile = new BlockSpriteSheet(this,
                                                              spritesheetPath,
                                                              currentTileID,
                                                              tileSize,
                                                              spritesheetColumns,
                                                              drawOrder);
                if (drawOrder == 15 && !IsCollisionExempt(currentTileID))
                {
                    auto* solidBlock = new InvisibleBlock(this);
                    solidBlock->SetPosition(Vector2(x * tileSize, y * tileSize));
                }

                // Calcule e defina a posição do tile no mundo do jogo.
                tile->SetPosition(Vector2(x * tileSize, y * tileSize));
                tilesCreated++;
            }
        }
    }

    // ADICIONE ESTES LOGS NO FINAL DA FUNÇÃO
    SDL_Log("--------------------------------------------------");
    SDL_Log("Relatorio de LoadTilemapLayer:");
    SDL_Log("Arquivo: %s", csvPath.c_str());
    SDL_Log("Dimensoes do CSV lido: %zu linhas, %zu colunas", tileIDs.size(), tileIDs[0].size());
    SDL_Log("Total de tiles instanciados: %d", tilesCreated);
    SDL_Log("--------------------------------------------------");
}

#include <sstream>    // Necessário para std::stringstream
#include <algorithm>  // Necessário para std::count

// Substitua sua função por esta versão final e flexível
std::vector<std::vector<int>> Game::LoadMapFromCSV(const std::string& filePath)
{
    std::vector<std::vector<int>> mapData;
    std::ifstream file(filePath);

    if (!file.is_open())
    {
        SDL_Log("ERRO FATAL: Nao foi possivel abrir o arquivo CSV: %s", filePath.c_str());
        return mapData;
    }

    // --- LÓGICA DE DETECÇÃO AUTOMÁTICA ---
    char delimiter = ','; // Assume vírgula como padrão
    std::string firstLine;
    if (std::getline(file, firstLine))
    {
        // Conta a ocorrência de cada separador na primeira linha
        size_t commaCount = std::count(firstLine.begin(), firstLine.end(), ',');
        size_t semicolonCount = std::count(firstLine.begin(), firstLine.end(), ';');

        // Usa o separador que aparecer mais vezes
        if (semicolonCount > commaCount)
        {
            delimiter = ';';
        }
        SDL_Log("Arquivo '%s' - Separador detectado: '%c'", filePath.c_str(), delimiter);
    }

    // Reinicia o leitor para o início do arquivo para processá-lo por completo
    file.clear();
    file.seekg(0, std::ios::beg);
    // --- FIM DA LÓGICA DE DETECÇÃO ---


    std::string line;
    // Lê o arquivo linha por linha
    while (std::getline(file, line))
    {
        if (line.empty() || line.find_first_not_of(" \t\n\r") == std::string::npos)
        {
            continue;
        }

        std::vector<int> row;
        std::stringstream ss(line);
        std::string cell;

        // Usa o separador que foi detectado automaticamente
        while (std::getline(ss, cell, delimiter))
        {
            try
            {
                row.push_back(std::stoi(cell));
            }
            catch (const std::exception& e)
            {
                SDL_Log("AVISO: Valor invalido no CSV ('%s')", cell.c_str());
                row.push_back(-1);
            }
        }
        mapData.push_back(row);
    }

    file.close();
    return mapData;
}

void Game::CreateLevelBoundaries(int levelWidthInTiles, int levelHeightInTiles, const Vector2& offset)
{
    const float TILE_SIZE_F = static_cast<float>(TILE_SIZE);
    float levelWidthPixels = levelWidthInTiles * TILE_SIZE_F;
    float levelHeightPixels = levelHeightInTiles * TILE_SIZE_F;

    // Barreiras superior e inferior
    for (int x = -1; x <= levelWidthInTiles; ++x)
    {
        // Posição relativa ao canto + deslocamento
        Vector2 topPos(x * TILE_SIZE_F + offset.x, -TILE_SIZE_F + offset.y);
        auto* topBlock = new InvisibleBlock(this);
        topBlock->SetPosition(topPos);

        Vector2 bottomPos(x * TILE_SIZE_F + offset.x, levelHeightPixels + offset.y);
        auto* bottomBlock = new InvisibleBlock(this);
        bottomBlock->SetPosition(bottomPos);
    }

    // Barreiras laterais
    for (int y = 0; y < levelHeightInTiles; ++y)
    {
        Vector2 leftPos(-TILE_SIZE_F + offset.x, y * TILE_SIZE_F + offset.y);
        auto* leftBlock = new InvisibleBlock(this);
        leftBlock->SetPosition(leftPos);

        Vector2 rightPos(levelWidthPixels + offset.x, y * TILE_SIZE_F + offset.y);
        auto* rightBlock = new InvisibleBlock(this);
        rightBlock->SetPosition(rightPos);
    }
}

// Em Game.cpp
// Em Game.cpp
void Game::LoadBossLevel()
{
    // Dimensões do nível do chefe
    mCurrentLevelPixelWidth = BOSS_LEVEL_WIDTH;
    mCurrentLevelPixelHeight = BOSS_LEVEL_HEIGHT;

    // --- AJUSTE DE POSICIONAMENTO ---
    const float verticalBias = 10.0f; // Ajuste para empurrar a arena para baixo

    // Calcula o deslocamento para centralizar a arena
    mCurrentLevelOffset.x = (static_cast<float>(mWindowWidth) - mCurrentLevelPixelWidth) / 2.0f;
    mCurrentLevelOffset.y = (static_cast<float>(mWindowHeight) - mCurrentLevelPixelHeight) / 2.0f + verticalBias;

    // Garante que o offset não seja negativo
    if (mCurrentLevelOffset.x < 0.0f) mCurrentLevelOffset.x = 0.0f;
    if (mCurrentLevelOffset.y < 0.0f) mCurrentLevelOffset.y = 0.0f;

    // --- ETAPA CRÍTICA: TRAVAR A CÂMERA NA ORIGEM ---
    mCameraPos = Vector2::Zero;
    // mIsCameraLocked = true; // Impede que UpdateCamera mova a câmera

    // --- Configurações de Áudio e HUD ---
    mAudio->StopAllSounds();
    mAudio->PlayMusic("FinalFight_FullConfesion.mp3", true, 13);
    if (!mHUD) { mHUD = new UIHud(this, "Assets/Fonts/SMB.ttf", mRenderer); }
    mGameTimeLimit = 999;

    // --- Configurar o Cenário (com offset) ---
    SetBackgroundImage("Assets/Sprites/spr_chinatown_backroom_bg.png",
                       mCurrentLevelOffset, // Posição no mundo
                       Vector2(mCurrentLevelPixelWidth, mCurrentLevelPixelHeight));

    const int widthInTiles = static_cast<int>(ceil(mCurrentLevelPixelWidth / TILE_SIZE));
    const int heightInTiles = static_cast<int>(ceil(mCurrentLevelPixelHeight / TILE_SIZE));
    CreateLevelBoundaries(widthInTiles, heightInTiles, mCurrentLevelOffset);
    CreateArenaFloorAndCeiling(mCurrentLevelPixelWidth, mCurrentLevelPixelHeight, mCurrentLevelOffset);

    // --- Criar Jogador e Chefe (com offset) ---
    mPlayer = new Player(this);
    mPlayer->SetPosition(Vector2(100.0f, mCurrentLevelPixelHeight - (3.0f * TILE_SIZE)) + mCurrentLevelOffset);

    auto* boss = new DemonBoss(this);
    boss->SetPosition(Vector2(mCurrentLevelPixelWidth - (6.0f * TILE_SIZE), mCurrentLevelPixelHeight - (4.0f * TILE_SIZE)) + mCurrentLevelOffset);
}

// Em Game.cpp
void Game::CreateArenaFloorAndCeiling(float arenaWidth, float arenaHeight, const Vector2& arenaOffset)
{
    const float TILE_SIZE_F = static_cast<float>(TILE_SIZE);
    const int tilesAcross = static_cast<int>(arenaWidth / TILE_SIZE_F);

    // Cria a fileira superior (teto)
    for (int i = 0; i <= tilesAcross; ++i)
    {
        auto* ceilingBlock = new InvisibleBlock(this);
        // Posição Y no topo da arena + offset
        ceilingBlock->SetPosition(Vector2(i * TILE_SIZE_F + arenaOffset.x, arenaOffset.y));
    }

    // Cria a fileira inferior (chão)
    for (int i = 0; i <= tilesAcross; ++i)
    {
        auto* floorBlock = new InvisibleBlock(this);
        // Posição Y na base da arena, menos um tile de altura + offset
        floorBlock->SetPosition(Vector2(i * TILE_SIZE_F + arenaOffset.x, arenaOffset.y + arenaHeight - TILE_SIZE_F));
    }

    SDL_Log("Chão e teto invisíveis da arena criados.");
}