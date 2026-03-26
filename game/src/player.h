#pragma once
#include "entity.h"

namespace galaxy {

struct BulletPool; // defined in bullet.h

// ─── Player ───────────────────────────────────────────────────────────────────
struct Player : Entity {
    int       lives          = 3;
    int       score          = 0;
    PowerUpType powerType    = PowerUpType::SPREAD;
    bool      hasPowerUp     = false;
    float     shieldTimer    = 0.0f;  // > 0 → invincible
    float     fireTimer      = 0.0f;  // cooldown between shots
    float     invincibleTimer= 0.0f;  // brief invincibility after being hit
    int       powerUpCount   = 0;     // active power-up remaining shots
};

// Initialise the player at the default starting position.
void initPlayer(Player& p);

// Update player state: movement, fire cooldown, timers.
// Fires bullets into pool when the spacebar is held.
void updatePlayer(Player& p, float dt, BulletPool& bullets);

// Render the player sprite (delegates to sprites module).
// Does nothing when p.active == false.
void renderPlayer(SDL_Renderer* renderer, const Player& p);

} // namespace galaxy
