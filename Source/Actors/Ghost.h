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
        Chasing
    };

    Ghost(class Game* game, float patrolSpeed = 70.0f, float chasingSpeed = 140.0f, float deathTime = 1.0f);

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
    float mPatrolSpeed;
    float mChasingSpeed;
    float mDyingTimer;
    bool mIsDying;

    // --- Máquina de Estados e IA ---
    GhostState mCurrentState;
    float mSeePlayerDistance;    // Distância para começar a perseguir
    float mLosePlayerDistance;   // Distância para parar de perseguir
    float mSpawnGracePeriod;     // Período de tolerância ao nascer

    // --- Lógica de Patrulha ---
    Vector2 mPatrolStartPoint;   // O ponto central da área de patrulha
    float mPatrolRadius;         // A distância para patrulhar a partir do ponto inicial

    // --- Lógica de Perseguição (Perlin Noise) ---
    PerlinNoise mPerlin;
    float mNoiseInput;
    float mNoiseScale;
    float mNoiseStrength;

    // --- Modificadores de Movimento ---
    float mTurnSpeed;            // Quão rápido ele vira
    float mOscillationFrequency; // Velocidade do "sobe e desce"
    float mOscillationAmplitude; // Altura do "sobe e desce"
};