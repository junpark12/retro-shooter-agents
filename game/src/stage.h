#pragma once
#include "types.h"

namespace galaxy {

struct EnemyPool; // defined in enemy.h
struct Boss;      // defined in boss.h

// Wave descriptor — one entry per enemy wave in a stage.
struct Wave {
    EnemyType     type;
    int           count;
    float         spawnInterval; // seconds between individual spawns within the wave
    BulletPattern pattern;       // attack pattern used by enemies in this wave
    float         entryDelay;    // seconds before this wave begins
    FormationType formation = FormationType::LANE; // spawn formation pattern
};

// ─── Stage ────────────────────────────────────────────────────────────────────
struct Stage {
    int   stageNum      = 1;
    int   waveIndex     = 0;       // index into the current stage's wave table
    float spawnTimer    = 0.0f;    // time until next enemy spawn in the current wave
    float waveDelay     = 0.0f;    // delay before the next wave starts
    int   spawnedInWave = 0;       // enemies spawned so far in the current wave
    int   enemiesKilled = 0;       // total kills this stage
    bool  bossSpawned   = false;
    bool  stageCleared  = false;
    float bossDelay     = 0.0f;    // countdown before boss appears after all waves

    // Checkpoint: saved at the midpoint of each stage.
    int   checkpointWave    = 0;   // wave index of last saved checkpoint
    bool  checkpointReached = false;

    // Boss warning: shown when bossDelay countdown begins.
    bool  bossWarningActive = false;
    float bossWarningTimer  = 0.0f; // duration remaining for warning display
    bool  bossWarningJustStarted = false; // set true for one frame when warning begins
};

// Initialise the stage: reset counters, set wave table for stageNum.
void initStage(Stage& s, int num);

// Advance stage logic: spawn enemies and eventually the boss.
// Requires access to the enemy pool and boss to spawn into.
void updateStage(Stage& s, float dt, EnemyPool& enemies, Boss& boss);

} // namespace galaxy
