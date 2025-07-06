#pragma once

#include <SDL.h>
#include <vector>
#include <unordered_map>
#include <memory>
#include "AudioSystem.h"
#include "GameMath.h"
#include "UIFont.h"

class Game
{
public:
    static constexpr int LEVEL_WIDTH = 215;
    static constexpr int LEVEL_HEIGHT = 15;
    static constexpr int TILE_SIZE = 32;
    static constexpr int SPAWN_DISTANCE = 700;
    static constexpr int TRANSITION_TIME = 1;
    static constexpr int INTRO_TIME = 2;

    enum class GameScene
    {
        Intro,
        MainMenu,
        Level1,
        Level2
    };

    enum class SceneManagerState
    {
        None,
        Entering,
        FadeIn,
        FadeOut,
        Active,
        Exiting
    };

    enum class GamePlayState
    {
        NotStarted,
        Playing,
        Paused,
        GameOver,
        LevelComplete,
        Leaving
    };

    Game(int windowWidth, int windowHeight);

    bool Initialize();
    void RunLoop();
    void Shutdown();
    void Quit() { mIsRunning = false; }

    // Actor functions
    void UpdateActors(float deltaTime);
    void AddActor(class Actor* actor) const;
    void RemoveActor(class Actor* actor) const;

    // Level functions
    void LoadMainMenu();
    void ShowTutorialScreen();
    void LoadLevel(const std::string& levelName, int levelWidth, int levelHeight);

    std::vector<Actor *> GetNearbyActors(const Vector2& position, int range = 1) const;
    std::vector<class AABBColliderComponent *> GetNearbyColliders(const Vector2& position, int range = 2) const;

    void Reinsert(Actor* actor) const;

    // Camera functions
    Vector2& GetCameraPos() { return mCameraPos; };
    void SetCameraPos(const Vector2& position) { mCameraPos = position; };

    // Audio functions
    AudioSystem* GetAudio() const { return mAudio; }

    // UI functions
    void PushUI(class UIScreen* screen) { mUIStack.emplace_back(screen); }
    const std::vector<class UIScreen*>& GetUIStack() { return mUIStack; }

    // Window functions
    int GetWindowWidth() const { return mWindowWidth; }
    int GetWindowHeight() const { return mWindowHeight; }

    // Loading functions
    class UIFont* LoadFont(const std::string& fileName);
    SDL_Texture* LoadTexture(const std::string& texturePath);

    void SetGameScene(GameScene scene, float transitionTime = .0f);
    void ResetGameScene(float transitionTime = .0f);
    void UnloadScene();

    void SetBackgroundImage(const std::string& imagePath, const Vector2 &position = Vector2::Zero, const Vector2& size = Vector2::Zero);
    void TogglePause();

    // Game-specific
    class Player* GetPlayer() const { return mPlayer; }

    void SetGamePlayState(const GamePlayState state) { mGamePlayState = state; }
    GamePlayState GetGamePlayState() const { return mGamePlayState; }

    SDL_Renderer* GetRenderer() const { return mRenderer; }
    class UIHud * GetHUD() const {return mHUD;};

    void LockCamera() { mIsCameraLocked = true; };
    void UnlockCamera() { mIsCameraLocked = false; };

    // Set Pause Menu State
    void SetPauseMenuState(const bool state) { mIsPauseMenuActive = state; }

private:
    void ProcessInput();
    void UpdateGame();
    void UpdateCamera();
    void GenerateOutput() const;

    // Scene Manager
    void UpdateSceneManager(float deltaTime);
    void ChangeScene();
    void UpdateScenes();
    SceneManagerState mSceneManagerState;
    float mSceneManagerTimer;
    bool mIsSceneManagerActive;

    // HUD functions
    void UpdateLevelTime(float deltaTime);

    // Load the level from a CSV file as a 2D array
    static int **ReadLevelData(const std::string& fileName, int width, int height);
    void BuildLevel(int** levelData, int width, int height);

    // Spatial Hashing for collision detection
    class SpatialHashing* mSpatialHashing;

    // All the UI elements
    std::vector<class UIScreen*> mUIStack;
    std::unordered_map<std::string, std::unique_ptr<UIFont>> mFonts;

    // SDL stuff
    SDL_Window* mWindow;
    SDL_Renderer* mRenderer;
    AudioSystem* mAudio;

    // Window properties
    int mWindowWidth;
    int mWindowHeight;

    // Track elapsed time since the game start
    Uint32 mTicksCount;

    // Track actors' state
    bool mIsRunning;
    GamePlayState mGamePlayState;

    // Track level state
    GameScene mGameScene;
    GameScene mNextScene;

    // Background and camera
    Vector3 mBackgroundColor;
    Vector3 mModColor;
    Vector2 mCameraPos;
    bool mIsCameraLocked;

    // Game-specific
    class Player *mPlayer;
    class UIHud *mHUD;
    SoundHandle mMusicHandle;

    float mGameTimer;
    int mGameTimeLimit;

    SDL_Texture *mBackgroundTexture;
    Vector2 mBackgroundSize;
    Vector2 mBackgroundPosition;

    std::unordered_map<std::string, SDL_Texture*> mTextures;  //Manage Textures

    // Pause menu
    bool mIsPauseMenuActive;
};
