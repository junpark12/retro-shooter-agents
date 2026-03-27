#include "stage.h"

#include "boss.h"
#include "enemy.h"

namespace galaxy {

namespace {
constexpr Wave STAGE1_WAVES[] = {
    {EnemyType::SMALL, 8, 0.35f},
    {EnemyType::MEDIUM, 5, 0.55f},
    {EnemyType::SMALL, 10, 0.25f},
    {EnemyType::LARGE, 3, 0.85f},
};

constexpr Wave STAGE2_WAVES[] = {
    {EnemyType::SMALL, 12, 0.24f},
    {EnemyType::MEDIUM, 7, 0.50f},
    {EnemyType::LARGE, 4, 0.75f},
    {EnemyType::MEDIUM, 8, 0.42f},
};

constexpr Wave STAGE3_WAVES[] = {
    {EnemyType::SMALL, 14, 0.22f},
    {EnemyType::LARGE, 5, 0.70f},
    {EnemyType::MEDIUM, 10, 0.36f},
    {EnemyType::LARGE, 6, 0.62f},
};

const Wave* getWavesForStage(int stageNum, int& outCount) {
    switch (stageNum) {
        case 1:
            outCount = static_cast<int>(sizeof(STAGE1_WAVES) / sizeof(STAGE1_WAVES[0]));
            return STAGE1_WAVES;
        case 2:
            outCount = static_cast<int>(sizeof(STAGE2_WAVES) / sizeof(STAGE2_WAVES[0]));
            return STAGE2_WAVES;
        default:
            outCount = static_cast<int>(sizeof(STAGE3_WAVES) / sizeof(STAGE3_WAVES[0]));
            return STAGE3_WAVES;
    }
}
}

void initStage(Stage& s, int num) {
    s.stageNum = num;
    s.waveIndex = 0;
    s.spawnTimer = 0.0f;
    s.spawnedInWave = 0;
    s.enemiesKilled = 0;
    s.bossSpawned = false;
    s.stageCleared = false;
}

void updateStage(Stage& s, float dt, EnemyPool& enemies, Boss& boss) {
    if (s.stageCleared) return;

    int waveCount = 0;
    const Wave* waves = getWavesForStage(s.stageNum, waveCount);

    if (s.waveIndex < waveCount) {
        const Wave& wave = waves[s.waveIndex];
        s.spawnTimer -= dt;

        if (s.spawnedInWave < wave.count && s.spawnTimer <= 0.0f) {
            float laneGap = static_cast<float>(SCREEN_W) / (wave.count + 1);
            float x = laneGap * (s.spawnedInWave + 1) - 16.0f;
            if (wave.type == EnemyType::LARGE) x += (s.spawnedInWave % 2 == 0) ? -40.0f : 40.0f;
            spawnEnemy(enemies, wave.type, {x, -36.0f - (s.spawnedInWave % 3) * 24.0f});
            s.spawnedInWave++;
            s.spawnTimer = wave.spawnInterval;
        }

        if (s.spawnedInWave >= wave.count && allEnemiesDefeated(enemies)) {
            s.waveIndex++;
            s.spawnedInWave = 0;
            s.spawnTimer = 0.9f;
        }
        return;
    }

    if (!s.bossSpawned) {
        initBoss(boss, s.stageNum);
        s.bossSpawned = true;
        return;
    }

    if (s.bossSpawned && !boss.active) {
        s.stageCleared = true;
    }
}

} // namespace galaxy
