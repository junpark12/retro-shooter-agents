#pragma once
#include "types.h"

namespace galaxy {

// Forward declarations to avoid circular includes.
struct Player;
struct BulletPool;
struct EnemyPool;
struct Boss;
struct PowerUpPool;
struct AudioManager;

// ─── Collision ────────────────────────────────────────────────────────────────
// Collision uses:
//   - AABB for power-up pickup (generous hitbox)
//   - Circular collision for bullet-player hits (small hitbox for Bullet Hell)
//   - AABB for bullet-enemy and bullet-boss

// Player bullets vs enemies: deducts HP, increments player score on kill.
// Killed enemies may drop a power-up into the pool.
// Plays explosion SFX via audio if non-null.
void checkBulletEnemyCollision(BulletPool& bullets, EnemyPool& enemies,
                                Player& player, PowerUpPool& powerUps,
                                AudioManager* audio = nullptr);

// Enemy bullets vs player: uses circular hitbox (Bullet Hell standard).
// Deducts player HP if not invincible/shielded. Plays hit SFX.
void checkBulletPlayerCollision(BulletPool& bullets, Player& player,
                                 AudioManager* audio = nullptr);

// Enemy bodies vs player: AABB collision, deducts player HP on contact.
void checkPlayerEnemyCollision(Player& player, EnemyPool& enemies,
                                AudioManager* audio = nullptr);

// Player bullets vs boss: deducts boss HP. Triggers phase changes.
void checkBulletBossCollision(BulletPool& bullets, Boss& boss, Player& player,
                               AudioManager* audio = nullptr);

// Player vs power-ups: applies the power-up effect to the player.
void checkPowerUpPickup(Player& player, PowerUpPool& powerUps,
                        AudioManager* audio = nullptr);

} // namespace galaxy
