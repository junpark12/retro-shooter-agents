#pragma once
#include "types.h"

namespace galaxy {

// Forward declarations to avoid circular includes.
struct Player;
struct BulletPool;
struct EnemyPool;
struct Boss;
struct PowerUpPool;

// ─── Collision ────────────────────────────────────────────────────────────────

// Player bullets vs enemies: deducts HP, increments player score on kill.
// Killed enemies may drop a power-up into the pool.
void checkBulletEnemyCollision(BulletPool& bullets, EnemyPool& enemies,
                                Player& player, PowerUpPool& powerUps);

// Enemy bullets vs player: deducts player HP if not invincible/shielded.
void checkBulletPlayerCollision(BulletPool& bullets, Player& player);

// Enemy bodies vs player: deducts player HP on contact.
void checkPlayerEnemyCollision(Player& player, EnemyPool& enemies);

// Player bullets vs boss: deducts boss HP.
void checkBulletBossCollision(BulletPool& bullets, Boss& boss, Player& player);

// Player vs power-ups: applies the power-up effect to the player.
void checkPowerUpPickup(Player& player, PowerUpPool& powerUps);

} // namespace galaxy
