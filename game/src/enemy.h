#pragma once
#include "entity.h"

namespace galaxy {

struct BulletPool; // defined in bullet.h

// ─── Enemy ────────────────────────────────────────────────────────────────────
struct Enemy : Entity {
    EnemyType type        = EnemyType::SMALL;
    float     moveTimer   = 0.0f;   // sine-wave phase or tracking cooldown
    int       pointValue  = 100;
    float     fireTimer   = 0.0f;   // cooldown between enemy shots
    BulletPattern firePattern = BulletPattern::SINGLE;
    float     patternTimer = 0.0f;  // for multi-phase pattern timing
    int       patternPhase = 0;     // current phase in complex pattern
    bool      lockedOn    = false;  // true when player has locked onto this enemy
    int       colorVariant = 0;     // sprite color variant
};

// Fixed-size pool.
struct EnemyPool {
    Enemy pool[MAX_ENEMIES];
};

// Activate the first inactive enemy at the given position.
void spawnEnemy(EnemyPool& ep, EnemyType type, Vec2 pos);

// Update all active enemies: movement patterns and shooting.
// Player position is used for LARGE/FAST enemy tracking.
void updateEnemies(EnemyPool& ep, float dt, BulletPool& bullets, Vec2 playerPos);

// Render all active enemies.
void renderEnemies(SDL_Renderer* renderer, const EnemyPool& ep);

// Returns true when all enemies in the pool are inactive.
bool allEnemiesDefeated(const EnemyPool& ep);

} // namespace galaxy
