#include "stage.h"

#include "boss.h"
#include "enemy.h"

namespace galaxy {

namespace {
constexpr Wave STAGE1_WAVES[] = {
    {EnemyType::SMALL, 5, 0.35f, BulletPattern::SINGLE,       0.0f},
    {EnemyType::MEDIUM, 4, 0.55f, BulletPattern::SPREAD_3,    1.0f},
    {EnemyType::SMALL, 8, 0.24f, BulletPattern::SINGLE,       0.8f},
    {EnemyType::MEDIUM, 3, 0.45f, BulletPattern::AIMED,       0.8f},
    {EnemyType::LARGE, 2, 0.85f, BulletPattern::AIMED_SPREAD, 0.6f},
};

constexpr Wave STAGE2_WAVES[] = {
    {EnemyType::FAST, 6, 0.30f, BulletPattern::AIMED,         0.0f},
    {EnemyType::MEDIUM, 4, 0.50f, BulletPattern::SPREAD_3,    1.0f},
    {EnemyType::LARGE, 3, 0.70f, BulletPattern::CIRCLE_8,     0.8f},
    {EnemyType::ARMORED, 2, 1.10f, BulletPattern::CURTAIN,    0.8f},
    {EnemyType::FAST, 4, 0.24f, BulletPattern::AIMED,         0.6f},
    {EnemyType::SMALL, 6, 0.20f, BulletPattern::SINGLE,       0.3f},
};

constexpr Wave STAGE3_WAVES[] = {
    {EnemyType::SMALL, 10, 0.20f, BulletPattern::SPREAD_3,    0.0f},
    {EnemyType::FAST, 8, 0.22f, BulletPattern::AIMED,         0.8f},
    {EnemyType::MEDIUM, 8, 0.40f, BulletPattern::AIMED_SPREAD,0.8f},
    {EnemyType::LARGE, 5, 0.62f, BulletPattern::CIRCLE_8,     0.8f},
    {EnemyType::ARMORED, 3, 0.95f, BulletPattern::CURTAIN,    0.8f},
    {EnemyType::FAST, 10, 0.18f, BulletPattern::RANDOM_SPREAD,0.4f},
    {EnemyType::MEDIUM, 10, 0.32f, BulletPattern::SPIRAL_CW,  0.3f},
};

constexpr Wave STAGE4_WAVES[] = {
    {EnemyType::FAST, 9, 0.18f, BulletPattern::SPIRAL_CCW,     0.0f},
    {EnemyType::MEDIUM, 8, 0.34f, BulletPattern::CIRCLE_16,    0.8f},
    {EnemyType::LARGE, 4, 0.55f, BulletPattern::CURTAIN,       0.7f},
    {EnemyType::SMALL, 10, 0.16f, BulletPattern::SPREAD_5,     0.5f},
    {EnemyType::FAST, 10, 0.15f, BulletPattern::AIMED_SPREAD,  0.4f},
    {EnemyType::ARMORED, 4, 0.85f, BulletPattern::CIRCLE_16,   0.6f},
    {EnemyType::MEDIUM, 9, 0.28f, BulletPattern::SPIRAL_CCW,   0.4f},
    {EnemyType::LARGE, 5, 0.52f, BulletPattern::CURTAIN,       0.3f},
};

constexpr Wave STAGE5_WAVES[] = {
    {EnemyType::FAST, 12, 0.14f, BulletPattern::AIMED,         0.0f},
    {EnemyType::MEDIUM, 12, 0.26f, BulletPattern::SPIRAL_CW,   0.6f},
    {EnemyType::LARGE, 6, 0.48f, BulletPattern::CIRCLE_16,     0.7f},
    {EnemyType::ARMORED, 5, 0.72f, BulletPattern::CURTAIN,     0.6f},
    {EnemyType::SMALL, 14, 0.13f, BulletPattern::RANDOM_SPREAD,0.4f},
    {EnemyType::FAST, 12, 0.12f, BulletPattern::SPIRAL_CCW,    0.3f},
    {EnemyType::MEDIUM, 13, 0.22f, BulletPattern::AIMED_SPREAD,0.3f},
    {EnemyType::LARGE, 7, 0.44f, BulletPattern::CIRCLE_16,     0.3f},
    {EnemyType::ARMORED, 6, 0.68f, BulletPattern::CURTAIN,     0.2f},
};

const Wave* getWavesForStage(int stageNum, int& outCount) {
    switch (stageNum) {
        case 1:
            outCount = static_cast<int>(sizeof(STAGE1_WAVES) / sizeof(STAGE1_WAVES[0]));
            return STAGE1_WAVES;
        case 2:
            outCount = static_cast<int>(sizeof(STAGE2_WAVES) / sizeof(STAGE2_WAVES[0]));
            return STAGE2_WAVES;
        case 3:
            outCount = static_cast<int>(sizeof(STAGE3_WAVES) / sizeof(STAGE3_WAVES[0]));
            return STAGE3_WAVES;
        case 4:
            outCount = static_cast<int>(sizeof(STAGE4_WAVES) / sizeof(STAGE4_WAVES[0]));
            return STAGE4_WAVES;
        default:
            outCount = static_cast<int>(sizeof(STAGE5_WAVES) / sizeof(STAGE5_WAVES[0]));
            return STAGE5_WAVES;
    }
}
}

void initStage(Stage& s, int num) {
    s.stageNum = num;
    s.waveIndex = 0;
    s.spawnTimer = 0.0f;
    s.waveDelay = 0.0f;
    s.spawnedInWave = 0;
    s.enemiesKilled = 0;
    s.bossSpawned = false;
    s.stageCleared = false;
    s.bossDelay = 3.0f;
}

void updateStage(Stage& s, float dt, EnemyPool& enemies, Boss& boss) {
    if (s.stageCleared) return;

    int waveCount = 0;
    const Wave* waves = getWavesForStage(s.stageNum, waveCount);

    if (s.waveIndex < waveCount) {
        const Wave& wave = waves[s.waveIndex];
        if (s.waveDelay < wave.entryDelay) {
            s.waveDelay += dt;
            return;
        }
        s.spawnTimer -= dt;

        if (s.spawnedInWave < wave.count && s.spawnTimer <= 0.0f) {
            float laneGap = static_cast<float>(SCREEN_W) / (wave.count + 1);
            float x = laneGap * (s.spawnedInWave + 1) - 16.0f;
            if (wave.type == EnemyType::LARGE) x += (s.spawnedInWave % 2 == 0) ? -40.0f : 40.0f;
            if (wave.type == EnemyType::FAST) x = (s.spawnedInWave % 2 == 0) ? 12.0f : SCREEN_W - 30.0f;
            Enemy* spawned = spawnEnemy(enemies, wave.type, {x, -36.0f - (s.spawnedInWave % 3) * 24.0f});
            if (spawned) {
                // Apply wave-specific fire pattern directly to the spawned enemy.
                spawned->firePattern = wave.pattern;
            }
            s.spawnedInWave++;
            s.spawnTimer = wave.spawnInterval;
        }

        if (s.spawnedInWave >= wave.count && allEnemiesDefeated(enemies)) {
            s.waveIndex++;
            s.spawnedInWave = 0;
            s.spawnTimer = 0.0f;
            s.waveDelay = 0.0f;
        }
        return;
    }

    if (!s.bossSpawned) {
        if (!allEnemiesDefeated(enemies)) return;
        s.bossDelay -= dt;
        if (s.bossDelay > 0.0f) return;
        initBoss(boss, s.stageNum);
        s.bossSpawned = true;
        return;
    }

    if (s.bossSpawned && !boss.active) {
        s.stageCleared = true;
    }
}

} // namespace galaxy
