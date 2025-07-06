#pragma once

#include "UIElements/UIScreen.h"

class UIPauseMenuScreen final : public UIScreen {
public:
    explicit UIPauseMenuScreen(Game* game);

    static void SetText(UIText * ui_text, const std::string & basic_string);

    void UpdateVolumeText() const;
private:
    UIText *mVolumeTextIdx;
};

