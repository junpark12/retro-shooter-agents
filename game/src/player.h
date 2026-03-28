#pragma once
#include "entity.h"

namespace galaxy {

struct BulletPool;  // defined in bullet.h
struct EnemyPool;   // defined in enemy.h

// ─── Lock-on target ───────────────────────────────────────────────────────────
struct LockTarget {
    bool  active   = false;
    int   enemyIdx = -1;   // index into EnemyPool::pool (-1 = boss)
    Vec2  pos;             // cached world position
};

// ─── Player ───────────────────────────────────────────────────────────────────
struct Player : Entity {
    ShipType  shipType       = ShipType::BAGON;
    int       lives          = 3;
    int       score          = 0;
    int       nextLifeScore  = SCORE_PER_EXTRA_LIFE;  // threshold for next 1UP
    int       powerLevel     = 1;    // 1–4, increases with POWER pickups
    int       bombStock      = 3;    // number of bombs remaining
    PowerUpType powerType    = PowerUpType::SPREAD;
    bool      hasPowerUp     = false;
    float     shieldTimer    = 0.0f;   // > 0 → invincible (shield active)
    float     fireTimer      = 0.0f;   // cooldown between shots
    float     invincibleTimer= 0.0f;   // brief invincibility after being hit
    int       powerUpCount   = 0;      // active power-up remaining shots

    // Lock-on system
    bool       lockOnActive    = false; // lock-on button held
    LockTarget lockTargets[MAX_LOCK_TARGETS];
    int        lockTargetCount = 0;
    float      lockOnTimer     = 0.0f;  // cooldown for lock-on acquisition

    // Charge shot (hold fire button)
    float      chargeTimer     = 0.0f;  // how long fire button has been held
    bool       chargeReady     = false; // true when fully charged

    // Bomb activation
    bool       bombActive      = false;
    float      bombTimer       = 0.0f;  // bomb screen-clear duration

    // Sprite animation
    int        animFrame       = 0;
    float      animTimer       = 0.0f;

    // Previous-frame button states (stored in Player to reset on initPlayer).
    // Avoids stale state when the game is restarted.
    bool       prevLockHeld    = false;
    bool       prevFireHeld    = false;
    bool       prevBombHeld    = false;
};

// Initialise the player at the default starting position with the given ship type.
void initPlayer(Player& p, ShipType ship = ShipType::BAGON);

// Update player state: movement, fire cooldown, lock-on, charge, bomb.
// Fires bullets into pool when the spacebar is held.
void updatePlayer(Player& p, float dt, BulletPool& bullets, EnemyPool& enemies);

// Release lock-on: fire homing shots at all locked targets.
void releaseLockOn(Player& p, BulletPool& bullets);

// Activate bomb: set bombActive = true, bombTimer, clear nearby bullets.
void activateBomb(Player& p, BulletPool& bullets);

// Render the player sprite (delegates to sprites module).
// Does nothing when p.active == false.
void renderPlayer(SDL_Renderer* renderer, const Player& p);

} // namespace galaxy
