#include "stage.h"

#include "boss.h"
#include "enemy.h"

#include <cmath>

namespace galaxy {

namespace {
constexpr Wave STAGE1_WAVES[] = {
    {EnemyType::SMALL,  5, 0.35f, BulletPattern::SINGLE,       0.0f, FormationType::LANE},
    {EnemyType::SMALL,  6, 0.30f, BulletPattern::SINGLE,       0.8f, FormationType::V_SHAPE},
    {EnemyType::MEDIUM, 4, 0.55f, BulletPattern::SPREAD_3,     1.0f, FormationType::LANE},
    {EnemyType::MEDIUM, 3, 0.45f, BulletPattern::AIMED,        0.8f, FormationType::DIAGONAL},
    {EnemyType::LARGE,  2, 0.85f, BulletPattern::AIMED_SPREAD, 0.6f, FormationType::LANE},
};

constexpr Wave STAGE2_WAVES[] = {
    {EnemyType::FAST,    6, 0.30f, BulletPattern::AIMED,        0.0f, FormationType::PINCER},
    {EnemyType::MEDIUM,  4, 0.50f, BulletPattern::SPREAD_3,     1.0f, FormationType::V_SHAPE},
    {EnemyType::FAST,    4, 0.28f, BulletPattern::AIMED,        0.7f, FormationType::DIAGONAL},
    {EnemyType::LARGE,   3, 0.70f, BulletPattern::CIRCLE_8,     0.8f, FormationType::LANE},
    {EnemyType::ARMORED, 2, 1.10f, BulletPattern::CURTAIN,      0.8f, FormationType::LANE},
    {EnemyType::FAST,    5, 0.24f, BulletPattern::AIMED,        0.5f, FormationType::CIRCLE},
};

constexpr Wave STAGE3_WAVES[] = {
    {EnemyType::SMALL,   8,  0.22f, BulletPattern::SPREAD_3,     0.0f, FormationType::V_SHAPE},
    {EnemyType::FAST,    6,  0.24f, BulletPattern::AIMED,        0.8f, FormationType::DIAGONAL},
    {EnemyType::MEDIUM,  6,  0.42f, BulletPattern::AIMED_SPREAD, 0.8f, FormationType::CIRCLE},
    {EnemyType::FAST,    8,  0.22f, BulletPattern::AIMED,        0.7f, FormationType::PINCER},
    {EnemyType::LARGE,   4,  0.65f, BulletPattern::CIRCLE_8,     0.8f, FormationType::LANE},
    {EnemyType::ARMORED, 3,  0.95f, BulletPattern::CURTAIN,      0.8f, FormationType::LANE},
    {EnemyType::MEDIUM,  8,  0.35f, BulletPattern::SPIRAL_CW,    0.4f, FormationType::V_SHAPE},
};

constexpr Wave STAGE4_WAVES[] = {
    {EnemyType::FAST,    8,  0.20f, BulletPattern::SPIRAL_CCW,   0.0f, FormationType::CIRCLE},
    {EnemyType::MEDIUM,  6,  0.36f, BulletPattern::CIRCLE_16,    0.8f, FormationType::V_SHAPE},
    {EnemyType::FAST,    8,  0.18f, BulletPattern::AIMED_SPREAD, 0.7f, FormationType::PINCER},
    {EnemyType::LARGE,   4,  0.58f, BulletPattern::CURTAIN,      0.7f, FormationType::DIAGONAL},
    {EnemyType::ARMORED, 3,  0.88f, BulletPattern::CIRCLE_16,    0.7f, FormationType::LANE},
    {EnemyType::FAST,    9,  0.16f, BulletPattern::AIMED,        0.5f, FormationType::CIRCLE},
    {EnemyType::MEDIUM,  8,  0.30f, BulletPattern::SPIRAL_CCW,   0.4f, FormationType::DIAGONAL},
    {EnemyType::LARGE,   5,  0.55f, BulletPattern::CURTAIN,      0.3f, FormationType::PINCER},
};

constexpr Wave STAGE5_WAVES[] = {
    {EnemyType::FAST,    10, 0.16f, BulletPattern::AIMED,        0.0f, FormationType::CIRCLE},
    {EnemyType::MEDIUM,  8,  0.30f, BulletPattern::SPIRAL_CW,    0.7f, FormationType::V_SHAPE},
    {EnemyType::FAST,    10, 0.14f, BulletPattern::AIMED_SPREAD, 0.7f, FormationType::PINCER},
    {EnemyType::LARGE,   5,  0.52f, BulletPattern::CIRCLE_16,    0.7f, FormationType::DIAGONAL},
    {EnemyType::ARMORED, 4,  0.76f, BulletPattern::CURTAIN,      0.7f, FormationType::LANE},
    {EnemyType::MEDIUM,  10, 0.26f, BulletPattern::SPIRAL_CCW,   0.5f, FormationType::CIRCLE},
    {EnemyType::FAST,    12, 0.14f, BulletPattern::AIMED_SPREAD, 0.4f, FormationType::DIAGONAL},
    {EnemyType::LARGE,   6,  0.46f, BulletPattern::CIRCLE_16,    0.3f, FormationType::V_SHAPE},
    {EnemyType::ARMORED, 5,  0.70f, BulletPattern::CURTAIN,      0.3f, FormationType::PINCER},
};

constexpr Wave STAGE6_WAVES[] = {
    {EnemyType::FAST,    12, 0.14f, BulletPattern::AIMED_BURST,  0.0f, FormationType::CIRCLE},
    {EnemyType::MEDIUM,  10, 0.26f, BulletPattern::CIRCLE_16,    0.6f, FormationType::V_SHAPE},
    {EnemyType::FAST,    14, 0.12f, BulletPattern::AIMED_SPREAD, 0.6f, FormationType::PINCER},
    {EnemyType::LARGE,    6, 0.48f, BulletPattern::CURTAIN,      0.6f, FormationType::DIAGONAL},
    {EnemyType::ARMORED,  5, 0.68f, BulletPattern::CIRCLE_16,    0.6f, FormationType::LANE},
    {EnemyType::FAST,    14, 0.12f, BulletPattern::HOMING_LASER, 0.5f, FormationType::CIRCLE},
    {EnemyType::MEDIUM,  12, 0.22f, BulletPattern::SPIRAL_CW,    0.4f, FormationType::DIAGONAL},
    {EnemyType::LARGE,    7, 0.42f, BulletPattern::SPIRAL_CCW,   0.3f, FormationType::V_SHAPE},
    {EnemyType::ARMORED,  6, 0.62f, BulletPattern::CURTAIN,      0.3f, FormationType::PINCER},
    {EnemyType::FAST,    16, 0.10f, BulletPattern::AIMED_BURST,  0.2f, FormationType::CIRCLE},
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
        case 6:
            outCount = static_cast<int>(sizeof(STAGE6_WAVES) / sizeof(STAGE6_WAVES[0]));
            return STAGE6_WAVES;
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
    s.checkpointWave = 0;
    s.checkpointReached = false;
    s.bossWarningActive = false;
    s.bossWarningTimer = 0.0f;
    s.bossWarningJustStarted = false;
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
            float x;
            const int idx = s.spawnedInWave;
            const int total = wave.count;
            switch (wave.formation) {
                case FormationType::V_SHAPE: {
                    const float mid = SCREEN_W * 0.5f;
                    const float halfSpan = 160.0f;
                    float t = (total <= 1) ? 0.5f : static_cast<float>(idx) / (total - 1);
                    // left half mirrors right half in V shape
                    float side = (t <= 0.5f) ? (t * 2.0f) : (1.0f - (t - 0.5f) * 2.0f);
                    x = mid - halfSpan + side * halfSpan * 2.0f - 16.0f;
                    // stagger y by distance from center
                    float distFromMid = std::abs(t - 0.5f) * 2.0f;
                    float yOff = distFromMid * 60.0f;
                    float spawnY = -36.0f - yOff;
                    Enemy* spawned = spawnEnemy(enemies, wave.type, {x, spawnY});
                    if (spawned) spawned->firePattern = wave.pattern;
                    break;
                }
                case FormationType::DIAGONAL: {
                    float t = (total <= 1) ? 0.5f : static_cast<float>(idx) / (total - 1);
                    x = 20.0f + t * (SCREEN_W - 60.0f) - 16.0f;
                    float yOff = static_cast<float>(idx) * 20.0f;
                    Enemy* spawned = spawnEnemy(enemies, wave.type, {x, -36.0f - yOff});
                    if (spawned) spawned->firePattern = wave.pattern;
                    break;
                }
                case FormationType::CIRCLE: {
                    const float cx = SCREEN_W * 0.5f;
                    const float radius = 160.0f;
                    float angle = static_cast<float>(idx) / total * 3.14159265f; // semicircle
                    x = cx + std::cos(angle) * radius - 16.0f;
                    float yOff = -50.0f - std::sin(angle) * 60.0f;
                    Enemy* spawned = spawnEnemy(enemies, wave.type, {x, yOff});
                    if (spawned) spawned->firePattern = wave.pattern;
                    break;
                }
                case FormationType::PINCER: {
                    bool leftSide = (idx % 2 == 0);
                    int halfIdx = idx / 2;
                    float t = (total / 2 <= 1) ? 0.5f : static_cast<float>(halfIdx) / (total / 2);
                    x = leftSide ? (12.0f + t * 100.0f - 16.0f)
                                 : (SCREEN_W - 120.0f + t * 100.0f - 16.0f);
                    Enemy* spawned = spawnEnemy(enemies, wave.type,
                                                {x, -36.0f - static_cast<float>(halfIdx) * 20.0f});
                    if (spawned) spawned->firePattern = wave.pattern;
                    break;
                }
                default: { // LANE
                    float laneGap = static_cast<float>(SCREEN_W) / (wave.count + 1);
                    x = laneGap * (idx + 1) - 16.0f;
                    if (wave.type == EnemyType::LARGE) x += (idx % 2 == 0) ? -40.0f : 40.0f;
                    if (wave.type == EnemyType::FAST) x = (idx % 2 == 0) ? 12.0f : SCREEN_W - 30.0f;
                    Enemy* spawned = spawnEnemy(enemies, wave.type, {x, -36.0f - (idx % 3) * 24.0f});
                    if (spawned) spawned->firePattern = wave.pattern;
                    break;
                }
            }
            s.spawnedInWave++;
            s.spawnTimer = wave.spawnInterval;
        }

        if (s.spawnedInWave >= wave.count && allEnemiesDefeated(enemies)) {
            s.waveIndex++;
            if (!s.checkpointReached && s.waveIndex >= waveCount / 2) {
                s.checkpointWave = s.waveIndex;
                s.checkpointReached = true;
            }
            s.spawnedInWave = 0;
            s.spawnTimer = 0.0f;
            s.waveDelay = 0.0f;
        }
        return;
    }

    if (!s.bossSpawned) {
        if (!allEnemiesDefeated(enemies)) return;
        s.bossDelay -= dt;
        if (!s.bossWarningActive && s.bossDelay < 2.5f) {
            s.bossWarningActive = true;
            s.bossWarningJustStarted = true;
            s.bossWarningTimer = 2.5f;
        }
        if (s.bossWarningActive) {
            s.bossWarningTimer = s.bossDelay;
            if (s.bossWarningTimer <= 0.0f) {
                s.bossWarningActive = false;
                s.bossWarningTimer = 0.0f;
            }
        }
        if (s.bossDelay > 0.0f) return;
        initBoss(boss, s.stageNum);
        s.bossWarningActive = false;
        s.bossWarningTimer = 0.0f;
        s.bossWarningJustStarted = false;
        s.bossSpawned = true;
        return;
    }

    if (s.bossSpawned && !boss.active) {
        s.stageCleared = true;
    }
}

} // namespace galaxy
