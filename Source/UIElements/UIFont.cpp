#include "UIFont.h"
#include <vector>
#include <SDL_image.h>

UIFont::UIFont(SDL_Renderer* renderer)
    :mRenderer(renderer)
{

}

UIFont::~UIFont()
{

}

bool UIFont::Load(const std::string& fileName)
{
	// We support these font sizes
	std::vector<int> fontSizes = {8,  9,  10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32,
								  34, 36, 38, 40, 42, 44, 46, 48, 52, 56, 60, 64, 68, 72};

	for (int i=0; i < fontSizes.size(); i++) {
		TTF_Font* font = TTF_OpenFont(fileName.c_str(),fontSizes[i]);
		if (font == nullptr)
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Falha ao carregar a fonte '%s' no tamanho %d! Erro TTF: %s",
						 fileName.c_str(), fontSizes[i], TTF_GetError());
			return false;
		}
		mFontData[fontSizes[i]] = font;
	}

	return true;
}

void UIFont::Unload()
{
	for (auto const& pair : mFontData)
	{
		TTF_CloseFont(pair.second);
	}
	mFontData.clear();
}

SDL_Texture* UIFont::RenderText(const std::string& text, const Vector4& color /*= Color::White*/,
                                int pointSize /*= 24*/, unsigned wrapLength /*= 900*/)
{
    if(!mRenderer)
    {
        SDL_Log("Renderer is null. Can't Render Text!");
        return nullptr;
    }

	// Convert to SDL_Color
	SDL_Color sdlColor;

	// Swap red and blue so we get RGBA instead of BGRA
	sdlColor.b = static_cast<Uint8>(color.x * 255);
	sdlColor.g = static_cast<Uint8>(color.y * 255);
	sdlColor.r = static_cast<Uint8>(color.z * 255);
	sdlColor.a = 255;

	auto it = mFontData.find(pointSize);
	SDL_Surface * surf;
	if (it != mFontData.end()) {
		surf = TTF_RenderUTF8_Blended_Wrapped(it->second,
					text.c_str(), sdlColor, wrapLength);
	}else {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Não foi possível criar a superfície");
		return nullptr;
	}

	SDL_Texture * texture = SDL_CreateTextureFromSurface(mRenderer,surf);
	SDL_FreeSurface(surf);
	if (!texture) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Não foi possível criar a texture");
		return nullptr;
	}
    return texture;
}
