//
// Created by Lucas N. Ferreira on 08/12/23.
//

#include "HUD.h"
#include "Game.h"
#include "UIElements/UIText.h"

HUD::HUD(class Game* game, const std::string& fontName)
    :UIScreen(game, fontName)
{
    const float timeTextPosX = mGame->GetWindowWidth() - WORD_OFFSET - (4 * CHAR_WIDTH);
    const float timeTextPosY = HUD_POS_Y;
    const float timeWidth = 4 * CHAR_WIDTH;
    const float timeHeight = WORD_HEIGHT;
    Vector2 timeDimensions(timeWidth, timeHeight);

    AddText("Time", Vector2(timeTextPosX, timeTextPosY),timeDimensions,POINT_SIZE);

    const float secondsTextPosX = mGame->GetWindowWidth() - WORD_OFFSET - (3 * CHAR_WIDTH);;
    const float secondsTextPosY = HUD_POS_Y * 2.0 + WORD_HEIGHT;
    const float secondsWidth = 3 * CHAR_WIDTH;
    const float secondsHeight = WORD_HEIGHT;
    Vector2 secondsDimensions(secondsWidth, secondsHeight);

    mTimeText = AddText("400", Vector2(secondsTextPosX, secondsTextPosY),secondsDimensions,POINT_SIZE);

    const float worldWidth = 5 * CHAR_WIDTH;
    const float worldHeight = WORD_HEIGHT;
    float worldTextPosX = timeTextPosX - worldWidth - WORD_OFFSET;
    const float worldTextPosY = HUD_POS_Y;
    Vector2 worldDimensions(worldWidth, worldHeight);

    AddText("World", Vector2(worldTextPosX, worldTextPosY),worldDimensions,POINT_SIZE);

    const float one_one_Width = 3 * CHAR_WIDTH;
    const float one_one_Height = WORD_HEIGHT;
    float one_one_TextPosX = worldTextPosX;
    const float one_one_TextPosY = HUD_POS_Y * 2.0 + WORD_HEIGHT;
    Vector2 one_one_Dimensions(one_one_Width, one_one_Height);

    mLevelName = AddText("1-1", Vector2(one_one_TextPosX, one_one_TextPosY),one_one_Dimensions,POINT_SIZE);

    const float mario_Width = 5 * CHAR_WIDTH;
    const float mario_Height = WORD_HEIGHT;
    float mario_TextPosX = WORD_OFFSET;
    const float mario_TextPosY = HUD_POS_Y;
    Vector2 mario_Dimensions(mario_Width, mario_Height);

    AddText("Mario", Vector2(mario_TextPosX, mario_TextPosY),mario_Dimensions,POINT_SIZE);

    const float points_Width = 6 * CHAR_WIDTH;
    const float points_Height = WORD_HEIGHT;
    float points_TextPosX = WORD_OFFSET;
    const float points_TextPosY = HUD_POS_Y * 2 + WORD_HEIGHT;
    Vector2 points_Dimensions(points_Width, points_Height);

    mScoreCounter = AddText("000000", Vector2(points_TextPosX, points_TextPosY),points_Dimensions,POINT_SIZE);

    const float coinsWidth = 3 * CHAR_WIDTH;
    const float coinsHeight = WORD_HEIGHT;
    float coinsTextPosX = (mario_TextPosX + mario_Width) + (worldTextPosX - (mario_TextPosX + mario_Width) )/2.0;
    const float coinsTextPosY = HUD_POS_Y * 2 + WORD_HEIGHT;
    Vector2 coinsDimensions(coinsWidth, coinsHeight);

    mCoinCounter = AddText("x00", Vector2(coinsTextPosX, coinsTextPosY),coinsDimensions,POINT_SIZE);
    const float coinsImageWidth = 1 * CHAR_WIDTH;
    const float coinsImageHeight = WORD_HEIGHT;
    Vector2 coinsImageDimensions(coinsImageWidth, coinsImageHeight);
    mCoinImage = AddImage("/Assets/Sprites/CoinHUD.png",Vector2(coinsTextPosX - coinsImageDimensions.x, coinsTextPosY), coinsImageDimensions);

}

HUD::~HUD()
{

}

void HUD::SetTime(int time)
{
    std::string timeStr = std::to_string(time);
    mTimeText->SetText(timeStr);

    int numDigits = timeStr.length();
    float newWidth = numDigits * CHAR_WIDTH;
    Vector2 newSize(newWidth, WORD_HEIGHT);

    float newPosX = mGame->GetWindowWidth() - WORD_OFFSET - (numDigits * CHAR_WIDTH);

    float newPosY = HUD_POS_Y * 2.0f + WORD_HEIGHT;
    Vector2 newPosition(newPosX, newPosY);

    // 4. Atualizar o objeto de texto com os novos valores.
    mTimeText->SetSize(newSize);
    mTimeText->SetPosition(newPosition);
}

void HUD::SetLevelName(const std::string &levelName)
{
    mLevelName->SetText(levelName);
}

void HUD::SetCoins()
{
    std::string coinsStr = std::to_string(mGame->GetNumberOfCoinsCollected());
    if (mGame->GetNumberOfCoinsCollected() < 10)
    mCoinCounter->SetText("x0" + coinsStr);
}

void HUD::SetScore()
{
    int score = mGame->GetScore();
    std::stringstream ss;
    ss << std::setw(6) << std::setfill('0') << score;

    std::string coinsStr = ss.str();

    mScoreCounter->SetText(coinsStr);
}