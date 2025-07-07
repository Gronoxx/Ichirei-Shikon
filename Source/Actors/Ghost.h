#pragma once

#include "Actor.h"
#include <random> // Para o gerador de números aleatórios

// Inclui a implementação do Perlin Noise que criamos
#include "PerlinNoise.h"

// Forward Declarations para evitar includes circulares
class AABBColliderComponent;
class DrawAnimatedComponent;
class RigidBodyComponent;

class Ghost : public Actor
{
public:
    // Enum para a máquina de estados do comportamento do fantasma
    enum class GhostState {
        Patrolling,
        Pursuing,
        Retreating
    };

    /**
     * Construtor do Ghost.
     * @param game Ponteiro para a classe Game principal.
     * @param forwardSpeed Velocidade base de movimento.
     * @param deathTime Tempo que a animação de morte leva.
     */
    Ghost(class Game* game, float forwardSpeed = 80.0f, float deathTime = 1.0f);

    // Funções override da classe Actor
    void OnUpdate(float deltaTime) override;

    // Função para iniciar o processo de morte do fantasma
    void Kill();

private:
    // Funções de colisão override da classe Actor
    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;

    // --- Componentes ---
    DrawAnimatedComponent* mDrawComponent;
    AABBColliderComponent* mColliderComponent;
    RigidBodyComponent* mRigidBodyComponent;

    // --- Atributos Principais ---
    float mForwardSpeed;
    float mDyingTimer;
    bool mIsDying;

    // --- Máquina de Estados e IA ---
    GhostState mCurrentState;
    float mRetreatTimer;
    float mSeekingRadius;
    float mSpawnGracePeriod; // Período de tolerância ao nascer

    // --- Lógica de Patrulha ---
    Vector2 mPatrolAnchorPoint;
    float mPatrolRadius;
    float mPatrolDirectionChangeInterval;
    float mPatrolTimer;
    Vector2 mCurrentPatrolDirection;

    // --- Geradores de Aleatoriedade ---
    std::mt19937 mRandomGenerator; // Motor de aleatoriedade
    std::uniform_real_distribution<float> mDistribution; // Mapeia para um ângulo

    // --- Lógica de Perseguição (Perlin Noise) ---
    PerlinNoise mPerlin;
    float mNoiseInput;
    float mNoiseScale;
    float mNoiseStrength;

    // --- Modificadores de Movimento ---
    float mTurnSpeed; // Quão rápido ele vira
    float mOscillationFrequency; // Velocidade do "sobe e desce"
    float mOscillationAmplitude; // Altura do "sobe e desce"
};