#pragma once

#include <string>
#include <vector>
#include "../GameMath.h"
#include "UIText.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UIRect.h"
#include "UITimerBar.h"

class UIScreen
{
public:
    // Tracks if the UI is active or closing
    enum class UIState
    {
        Active,
        Closing
    };

	UIScreen(class Game* game, const std::string& fontName);
	virtual ~UIScreen();

	// UIScreen subclasses can override these
	virtual void Update(float deltaTime);
	virtual void Draw(class SDL_Renderer *renderer);
	virtual void ProcessInput(const uint8_t* keys);
	virtual void HandleKeyPress(int key);

    // Set state to closing
	void Close();

    // Get state of the UI screen
	UIState GetState() const { return mState; }

	// Set state of UI screen
	void SetState(UIState state) { mState = state; }

    // Game getter
    Game* GetGame() const { return mGame; }

    // Add a button to this screen
	UIButton* AddButton(const std::string& name, const Vector2& pos, const Vector2& dims, std::function<void()> onClick);
    UIText* AddText(const std::string& name, const Vector2& pos, const Vector2& dims, int pointSize = 40, int unsigned wrapLength = 1024);
    UIImage* AddImage(const std::string& imagePath, const Vector2& pos, const Vector2& dims, const Vector3& color = Color::White);
	UIRect* AddRect(const Vector2& pos, const Vector2& size, const Vector4& color);
	UIRect* AddRect(const Vector2& pos, const Vector2& size, const Vector3& color);

protected:
    // Sets the mouse mode to relative or not
	Game* mGame;
    UIFont* mFont;

	// Configure positions
	Vector2 mPos;
	Vector2 mSize;

	// State
	UIState mState;

	// List of buttons, texts, and images
    int mSelectedButtonIndex;
	std::vector<UIButton *> mButtons;
    std::vector<UIText *> mTexts;
    std::vector<UIImage *> mImages;
	std::vector<UIRect *> mRects;
};
