// Actors/InvisibleBlock.cpp
#include "InvisibleBlock.h"
#include "Game.h"
#include "Components/ColliderComponents/AABBColliderComponent.h"

// Supondo que TILE_SIZE e ColliderLayer estejam acessíveis aqui.
// Se não estiverem, inclua o header necessário (provavelmente "Game.h" ou um de constantes).

InvisibleBlock::InvisibleBlock(Game* game)
    : Actor(game)
{
    // Adiciona um componente de colisão AABB (Axis-Aligned Bounding Box) a este ator.
    // O colisor terá o mesmo tamanho de um tile padrão.
    new AABBColliderComponent(this, 0, 0, Game::TILE_SIZE, Game::TILE_SIZE, ColliderLayer::Blocks,true);
}