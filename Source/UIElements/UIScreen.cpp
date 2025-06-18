// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#include "UIScreen.h"
#include "../Game.h"
#include "UIFont.h"

UIScreen::UIScreen(Game* game, const std::string& fontName)
	:mGame(game)
	,mPos(0.f, 0.f)
	,mSize(0.f, 0.f)
	,mState(UIState::Active)
    ,mSelectedButtonIndex(-1)
{
    mGame->PushUI(this);

    mFont = mGame->LoadFont(fontName);
}

UIScreen::~UIScreen()
{
    for (int i=0;i<mTexts.size();i++)
        delete mTexts[i];
    mTexts.clear();

    for (int i=0;i<mButtons.size();i++)
        delete mButtons[i];
    mButtons.clear();

    for (int i=0;i<mImages.size();i++)
        delete mImages[i];
    mImages.clear();
}

void UIScreen::Update(float deltaTime)
{
	
}

void UIScreen::Draw(SDL_Renderer *renderer)
{
    for (int i=0;i<mTexts.size();i++)
        mTexts[i]->Draw(renderer,mPos);


    for (int i=0;i<mButtons.size();i++)
        mButtons[i]->Draw(renderer,mPos);

    for (int i=0;i<mImages.size();i++)
        mImages[i]->Draw(renderer,mPos);
}

void UIScreen::ProcessInput(const uint8_t* keys)
{

}

void UIScreen::HandleKeyPress(int key)
{
    if (key == SDLK_w && mButtons.size() > 0 && mSelectedButtonIndex != -1) {
        mButtons[mSelectedButtonIndex]->SetHighlighted(false);
        mSelectedButtonIndex--;
        if (mSelectedButtonIndex < 0) {mSelectedButtonIndex = mButtons.size()-1;}
        mButtons[mSelectedButtonIndex]->SetHighlighted(true);
    }
    else if (key == SDLK_s && mButtons.size() > 0 && mSelectedButtonIndex != -1) {
        mButtons[mSelectedButtonIndex]->SetHighlighted(false);
        mSelectedButtonIndex++;
        if (mSelectedButtonIndex >= mButtons.size()) {mSelectedButtonIndex = 0;}
        mButtons[mSelectedButtonIndex]->SetHighlighted(true);
    }
    else if (key == SDLK_RETURN && mButtons.size() > 0 && mSelectedButtonIndex != -1) {
        mButtons[mSelectedButtonIndex]->OnClick();
    }
}

void UIScreen::Close()
{
	mState = UIState::Closing;
}

UIText* UIScreen::AddText(const std::string &name, const Vector2 &pos, const Vector2 &dims, const int pointSize, const int unsigned wrapLength)
{
    int pS = static_cast<int>(pointSize/3.0);
    UIText * t = new UIText(name,mFont,pS,wrapLength,pos,dims);
    mTexts.push_back(t);
    return t;
}

UIButton* UIScreen::AddButton(const std::string& name, const Vector2 &pos, const Vector2& dims, std::function<void()> onClick)
{
    UIButton* b = new UIButton(this, name, mFont, onClick, pos, dims,mGame,true, Vector3(218, 109, 0));
    mButtons.push_back(b);

    if (mButtons.size() == 1)
    {
        mSelectedButtonIndex = 0;
        b->SetHighlighted(true);
    }

    return b;
}

UIImage* UIScreen::AddImage(const std::string &imagePath, const Vector2 &pos, const Vector2 &dims, const Vector3 &color)
{
    UIImage * img = new UIImage(mGame->GetRenderer(),imagePath, pos, dims, color);
    mImages.push_back(img);
    return img;
}