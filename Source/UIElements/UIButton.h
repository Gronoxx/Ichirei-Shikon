//
// Created by Lucas N. Ferreira on 22/05/25.
//

#pragma once

#include <functional>
#include <string>
#include "UIFont.h"
#include "UIText.h"
#include "UIElement.h"
#include "../GameMath.h"
#include "../Game.h"
#include "UIImage.h"

class UIButton : public UIElement
{
public:
    const int iconPadding = 15.0;

    UIButton(UIScreen * MainMenu,
             const std::string& text, class UIFont* font, std::function<void()> onClick,
             const Vector2& pos, const Vector2& size,Game *game, bool isSizeDynamic , const Vector3& color = Vector3(30, 30, 30),
             int pointSize = 20, const unsigned wrapLength = 1024,
             const Vector2 &textPos = Vector2::Zero,
             const Vector2 &textSize = Vector2(220.f, 17.0f),
             const Vector3& textColor = Color::White);

    ~UIButton();

    // Set the name of the button
    void SetText(const std::string& text);
    void Draw(SDL_Renderer* renderer, const Vector2 &screenPos) override;

    void SetHighlighted(bool sel) { mHighlighted = sel; }
    bool GetHighlighted() const { return mHighlighted; }

    // Returns true if the point is within the button's bounds
    bool ContainsPoint(const Vector2& pt) const;

    // Called when button is clicked
    void OnClick();

private:
    // Callback funtion
    std::function<void()> mOnClick;

    // Button name
    UIText mText;
    Game * mGame;
    UIImage * mIcon;
    UIScreen * mMainMenu;

    // Check if the button is highlighted
    bool mHighlighted;
    bool mIsSizeDynamic;
};