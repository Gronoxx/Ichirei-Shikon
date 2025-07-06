#pragma once
#include "Actor.h"
#include <string>

class BlockSpriteSheet : public Actor
{
public:
    /**
     * Construtor para um tile a partir de uma spritesheet.
     * @param game Ponteiro para a instância do jogo.
     * @param texturePath Caminho para o arquivo da spritesheet.
     * @param tileID O ID do tile a ser desenhado (começando em 0).
     * @param tileSize O tamanho (largura e altura) de um único tile em pixels.
     * @param spritesheetColumns O número de colunas na spritesheet.
     * @param drawOrder A ordem de desenho (opcional).
     */
    BlockSpriteSheet(class Game* game,
                     const std::string& texturePath,
                     int tileID,
                     int tileSize,
                     int spritesheetColumns,
                     int drawOrder = 5);

private:
    class DrawSpriteComponent* mDrawComponent;
};