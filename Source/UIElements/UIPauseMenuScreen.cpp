#include "UIPauseMenuScreen.h"
#include "Game.h"
#include <SDL_mixer.h>

UIPauseMenuScreen::UIPauseMenuScreen(Game* game) : UIScreen(game, "Assets/Fonts/SMB.ttf") {
    const float windowWidth = static_cast<float>(game->GetWindowWidth());
    const float windowHeight = static_cast<float>(game->GetWindowHeight());
    const float centerX = windowWidth / 2.0f;
    const float centerY = windowHeight / 2.0f;
    constexpr float menuHeight = 320.0f;
    const float menuY = centerY - menuHeight / 2.0f;

    // Fundo semitransparente cobrindo toda a tela
    AddRect(Vector2(0, 0), Vector2(windowWidth, windowHeight),
        Vector4(0, 0, 0, 180));

    AddText("PAUSADO", Vector2(centerX - 100, menuY + 10), Vector2(200, 40), 24, 0);

    // Texto do volume
    const int vol = Mix_VolumeMusic(-1);
    const int percent = static_cast<int>(vol * 100.0f / 128.0f + 0.5f);
    UIText* txt = AddText("Volume: " + std::to_string(percent), Vector2(centerX - 100, menuY + 60),
        Vector2(200, 40), 24, 0);
    mVolumeTextIdx = txt;

    AddButton("CONTINUAR", Vector2(centerX - 105, menuY + 100), Vector2(200, 40),
        [this]() { SetState(UIState::Closing); mGame->SetPauseMenuState(false); });

    AddButton("REINICIAR", Vector2(centerX - 105, menuY + 140), Vector2(200, 40),
        [this]() { mGame->ResetGameScene(); SetState(UIState::Closing);
            mGame->SetPauseMenuState(false); });

    AddButton("VOLUME +", Vector2(centerX - 180, menuY + 180), Vector2(95, 40),
        [this]() {
            const int currentVolume = Mix_VolumeMusic(-1);
            int volumePercentage = static_cast<int>(currentVolume * 100.0f / 128.0f + 0.5f);
            volumePercentage = std::min(volumePercentage + 5, 100);
            const int newVolume = static_cast<int>(volumePercentage * 128.0f / 100.0f + 0.5f);
            Mix_VolumeMusic(newVolume);
            UpdateVolumeText();
        });

    AddButton("VOLUME -", Vector2(centerX + 5, menuY + 180), Vector2(95, 40),
        [this]() {
            const int currentVolume = Mix_VolumeMusic(-1);
            int newVolumePercentage = static_cast<int>(currentVolume * 100.0f / 128.0f + 0.5f);
            newVolumePercentage = std::max(newVolumePercentage - 5, 0);
            const int newVolume = static_cast<int>(newVolumePercentage * 128.0f / 100.0f + 0.5f);
            Mix_VolumeMusic(newVolume);
            UpdateVolumeText();
        });

    AddButton("MENU PRINCIPAL", Vector2(centerX - 150, menuY + 220), Vector2(200, 40),
        [this]() { mGame->SetGameScene(Game::GameScene::MainMenu); SetState(UIState::Closing);
            mGame->SetPauseMenuState(false); });
}

void UIPauseMenuScreen::UpdateVolumeText() const {
    const int vol = Mix_VolumeMusic(-1);
    const int percent = static_cast<int>(vol * 100.0f / 128.0f + 0.5f);
    SetText(mVolumeTextIdx, "Volume: " + std::to_string(percent));
}

void UIPauseMenuScreen::SetText(UIText* ui_text, const std::string& basic_string) {
    if (ui_text) {
        ui_text->SetText(basic_string);
    }
}
