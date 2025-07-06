#include "BlockSpriteSheet.h"
#include "../Game.h"
#include "../Components/DrawComponents/DrawSpriteComponent.h"
#include "../Components/ColliderComponents/AABBColliderComponent.h" // Opcional, se precisar de colisão

BlockSpriteSheet::BlockSpriteSheet(Game* game,
                                     const std::string& texturePath,
                                     int tileID,
                                     int tileSize,
                                     int spritesheetColumns,
                                     int drawOrder)
    : Actor(game)
{
    // Se o ID for -1 (ou qualquer outro valor que você considere "vazio"),
    // não crie o componente de desenho para economizar recursos.
    if (tileID < 0)
    {
        mDrawComponent = nullptr;
        return;
    }

    // 1. Crie o DrawSpriteComponent usando a spritesheet inteira.
    // A largura e altura aqui definem o tamanho do Actor no mundo do jogo.
    mDrawComponent = new DrawSpriteComponent(this, texturePath, tileSize, tileSize, drawOrder);

    // 2. Calcule a posição (linha e coluna) do tile na spritesheet.
    // A divisão inteira nos dá a linha (y).
    int tileRow = tileID / spritesheetColumns;
    // O operador módulo (%) nos dá a coluna (x).
    int tileCol = tileID % spritesheetColumns;

    // 3. Calcule as coordenadas em pixels do retângulo de origem (srcRect).
    int srcX = tileCol * tileSize;
    int srcY = tileRow * tileSize;

    // 4. Configure o srcRect no componente de desenho.
    mDrawComponent->SetSourceRect(srcX, srcY, tileSize, tileSize);

    // 5. Habilite o uso do srcRect personalizado.
    // Sem esta linha, o DrawSpriteComponent desenharia a spritesheet inteira.
    mDrawComponent->EnableSourceRect(true);

    // [Opcional] Adicione um componente de colisão se este tile for sólido.
    // new AABBColliderComponent(this, 0, 0, tileSize, tileSize, ColliderLayer::Blocks, true);
}