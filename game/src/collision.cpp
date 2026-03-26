// Galaxy Storm — Collision detection implementation

#include "collision.h"
#include "player.h"
#include "bullet.h"
#include "enemy.h"
#include "boss.h"
#include "powerup.h"

#include <cstdlib>

namespace galaxy {

// Probability (out of 100) that a LARGE enemy drops a power-up on death
static constexpr int POWERUP_DROP_CHANCE = 60;

// Helper: apply damage to player when not shielded / invincible
static void damagePlayer(Player& player, int dmg) {
    if (player.shieldTimer > 0.0f || player.invincibleTimer > 0.0f) return;
    player.hp -= dmg;
    if (player.hp <= 0) {
        player.lives -= 1;
        player.hp = 3; // reset HP for next life
        player.invincibleTimer = 2.0f; // 2 s of invincibility after a hit
        player.hasPowerUp = false;
        player.powerUpCount = 0;
    } else {
        player.invincibleTimer = 1.0f;
    }
}

void checkBulletEnemyCollision(BulletPool& bullets, EnemyPool& enemies,
                                Player& player, PowerUpPool& powerUps) {
    for (int bi = 0; bi < MAX_BULLETS; ++bi) {
        Bullet& b = bullets.pool[bi];
        if (!b.active || b.owner != BulletOwner::PLAYER) continue;

        Rect bRect = b.worldBounds();

        for (int ei = 0; ei < MAX_ENEMIES; ++ei) {
            Enemy& e = enemies.pool[ei];
            if (!e.active) continue;

            if (rectsOverlap(bRect, e.worldBounds())) {
                b.active = false;
                e.hp -= b.damage;
                if (e.hp <= 0) {
                    player.score += e.pointValue;
                    // LARGE enemies have a chance to drop a power-up
                    if (e.type == EnemyType::LARGE) {
                        if (std::rand() % 100 < POWERUP_DROP_CHANCE) {
                            // Cycle through power-up types
                            PowerUpType pt = static_cast<PowerUpType>(std::rand() % 4);
                            spawnPowerUp(powerUps,
                                         { e.pos.x + e.bounds.w * 0.5f - 10.0f,
                                           e.pos.y + e.bounds.h * 0.5f - 10.0f },
                                         pt);
                        }
                    }
                    e.active = false;
                }
                break; // bullet consumed
            }
        }
    }
}

void checkBulletPlayerCollision(BulletPool& bullets, Player& player) {
    if (!player.active) return;
    Rect pRect = player.worldBounds();

    for (int i = 0; i < MAX_BULLETS; ++i) {
        Bullet& b = bullets.pool[i];
        if (!b.active || b.owner != BulletOwner::ENEMY) continue;

        if (rectsOverlap(b.worldBounds(), pRect)) {
            b.active = false;
            damagePlayer(player, b.damage);
        }
    }
}

void checkPlayerEnemyCollision(Player& player, EnemyPool& enemies) {
    if (!player.active) return;
    Rect pRect = player.worldBounds();

    for (int i = 0; i < MAX_ENEMIES; ++i) {
        Enemy& e = enemies.pool[i];
        if (!e.active) continue;

        if (rectsOverlap(pRect, e.worldBounds())) {
            e.active = false; // enemy is destroyed on contact
            damagePlayer(player, 2);
        }
    }
}

void checkBulletBossCollision(BulletPool& bullets, Boss& boss, Player& player) {
    if (!boss.active) return;
    Rect bossRect = boss.worldBounds();

    for (int i = 0; i < MAX_BULLETS; ++i) {
        Bullet& b = bullets.pool[i];
        if (!b.active || b.owner != BulletOwner::PLAYER) continue;

        if (rectsOverlap(b.worldBounds(), bossRect)) {
            b.active = false;
            boss.hp -= b.damage;
            player.score += 10; // small score per hit
            if (boss.hp <= 0) {
                boss.active = false;
                player.score += 5000; // boss kill bonus
            }
        }
    }
}

void checkPowerUpPickup(Player& player, PowerUpPool& powerUps) {
    if (!player.active) return;
    Rect pRect = player.worldBounds();

    for (int i = 0; i < MAX_POWERUPS; ++i) {
        PowerUp& pu = powerUps.pool[i];
        if (!pu.active) continue;

        if (rectsOverlap(pRect, pu.worldBounds())) {
            pu.active = false;
            player.powerType    = pu.type;
            player.hasPowerUp   = true;
            player.powerUpCount = 30; // 30 shots worth of power-up ammo
            if (pu.type == PowerUpType::SHIELD) {
                player.shieldTimer  = 8.0f; // 8 seconds of shield
                player.hasPowerUp   = false;
                player.powerUpCount = 0;
            }
        }
    }
}

} // namespace galaxy
