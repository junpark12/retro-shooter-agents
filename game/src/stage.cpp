// Galaxy Storm — Stage / wave manager implementation

#include "stage.h"
#include "enemy.h"
#include "boss.h"

#include <cstdlib>

namespace galaxy {

// ─── Wave tables per stage ────────────────────────────────────────────────────
// Each stage is defined as an array of Wave descriptors.

static const Wave STAGE1_WAVES[] = {
    { EnemyType::SMALL,  6, 0.6f },
    { EnemyType::SMALL,  8, 0.5f },
    { EnemyType::MEDIUM, 4, 1.0f },
    { EnemyType::SMALL,  6, 0.4f },
    { EnemyType::MEDIUM, 5, 0.8f },
};

static const Wave STAGE2_WAVES[] = {
    { EnemyType::SMALL,  8,  0.4f },
    { EnemyType::MEDIUM, 6,  0.7f },
    { EnemyType::LARGE,  2,  2.0f },
    { EnemyType::SMALL,  10, 0.3f },
    { EnemyType::MEDIUM, 4,  0.6f },
    { EnemyType::LARGE,  3,  1.5f },
};

static const Wave STAGE3_WAVES[] = {
    { EnemyType::MEDIUM, 6,  0.5f },
    { EnemyType::LARGE,  4,  1.2f },
    { EnemyType::SMALL,  12, 0.3f },
    { EnemyType::MEDIUM, 8,  0.4f },
    { EnemyType::LARGE,  5,  1.0f },
    { EnemyType::SMALL,  10, 0.25f },
    { EnemyType::LARGE,  4,  0.9f },
};

struct WaveTable {
    const Wave* waves;
    int         count;
};

static const WaveTable WAVE_TABLES[3] = {
    { STAGE1_WAVES, 5 },
    { STAGE2_WAVES, 6 },
    { STAGE3_WAVES, 7 },
};

// ─── initStage ────────────────────────────────────────────────────────────────

void initStage(Stage& s, int num) {
    s.stageNum      = num;
    s.waveIndex     = 0;
    s.spawnTimer    = 0.0f;
    s.spawnedInWave = 0;
    s.enemiesKilled = 0;
    s.bossSpawned   = false;
    s.stageCleared  = false;
}

// ─── updateStage ──────────────────────────────────────────────────────────────

void updateStage(Stage& s, float dt, EnemyPool& enemies, Boss& boss) {
    if (s.stageCleared) return;

    const WaveTable& wt = WAVE_TABLES[s.stageNum - 1];

    // All regular waves finished — spawn boss if not yet spawned
    if (s.waveIndex >= wt.count) {
        if (!s.bossSpawned && !boss.active) {
            initBoss(boss, s.stageNum);
            boss.active = true;
            s.bossSpawned = true;
        }
        // Stage cleared when boss is defeated
        if (s.bossSpawned && !boss.active) {
            s.stageCleared = true;
        }
        return;
    }

    const Wave& wave = wt.waves[s.waveIndex];

    s.spawnTimer -= dt;
    if (s.spawnTimer <= 0.0f) {
        if (s.spawnedInWave < wave.count) {
            // Spread spawns across the top of the screen
            float xPos = 32.0f + static_cast<float>(std::rand() % (SCREEN_W - 64));
            spawnEnemy(enemies, wave.type, { xPos, -40.0f });
            ++s.spawnedInWave;
            s.spawnTimer = wave.spawnInterval;
        } else {
            // Wave complete — move to next wave once enemies are cleared
            if (allEnemiesDefeated(enemies)) {
                ++s.waveIndex;
                s.spawnedInWave = 0;
                s.spawnTimer    = 1.5f; // brief pause before next wave
            }
        }
    }
}

} // namespace galaxy
