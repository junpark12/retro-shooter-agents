#include "collision.h"

#include "boss.h"
#include "bullet.h"
#include "enemy.h"
#include "player.h"
#include "powerup.h"

namespace galaxy {

namespace {
PowerUpType dropTypeFromEnemy(const Enemy& e) {
    if (e.type == EnemyType::LARGE) return PowerUpType::MISSILE;
    if (e.type == EnemyType::MEDIUM) return PowerUpType::LASER;
    return PowerUpType::SPREAD;
}
}

void checkBulletEnemyCollision(BulletPool& bullets, EnemyPool& enemies,
                               Player& player, PowerUpPool& powerUps) {
    for (Bullet& b : bullets.pool) {
        if (!b.active || b.owner != BulletOwner::PLAYER) continue;
        Rect bb = b.worldBounds();

        for (Enemy& e : enemies.pool) {
            if (!e.active) continue;
            if (!rectsOverlap(bb, e.worldBounds())) continue;

            e.hp -= b.damage;
            if (player.powerType != PowerUpType::LASER) {
                b.active = false;
            }

            if (e.hp <= 0) {
                e.active = false;
                player.score += e.pointValue;

                if (e.type == EnemyType::LARGE || e.type == EnemyType::MEDIUM) {
                    spawnPowerUp(powerUps, {e.pos.x + e.bounds.w * 0.5f - 10.0f, e.pos.y},
                                 dropTypeFromEnemy(e));
                }
            }
            break;
        }
    }
}

void checkBulletPlayerCollision(BulletPool& bullets, Player& player) {
    if (!player.active) return;
    if (player.invincibleTimer > 0.0f || player.shieldTimer > 0.0f) return;

    Rect pb = player.worldBounds();
    for (Bullet& b : bullets.pool) {
        if (!b.active || b.owner != BulletOwner::ENEMY) continue;
        if (!rectsOverlap(pb, b.worldBounds())) continue;

        b.active = false;
        player.hp -= b.damage;
        player.invincibleTimer = 1.0f;
        if (player.hp <= 0) {
            player.lives--;
            if (player.lives > 0) {
                player.hp = 3;
                player.pos = {SCREEN_W * 0.5f - 14.0f, SCREEN_H - 90.0f};
                player.invincibleTimer = 2.0f;
            } else {
                player.active = false;
            }
        }
        break;
    }
}

void checkPlayerEnemyCollision(Player& player, EnemyPool& enemies) {
    if (!player.active) return;
    if (player.invincibleTimer > 0.0f || player.shieldTimer > 0.0f) return;

    Rect pb = player.worldBounds();
    for (Enemy& e : enemies.pool) {
        if (!e.active) continue;
        if (!rectsOverlap(pb, e.worldBounds())) continue;

        e.active = false;
        player.hp -= 1;
        player.invincibleTimer = 1.0f;
        if (player.hp <= 0) {
            player.lives--;
            if (player.lives > 0) {
                player.hp = 3;
                player.pos = {SCREEN_W * 0.5f - 14.0f, SCREEN_H - 90.0f};
                player.invincibleTimer = 2.0f;
            } else {
                player.active = false;
            }
        }
        break;
    }
}

void checkBulletBossCollision(BulletPool& bullets, Boss& boss, Player& player) {
    if (!boss.active) return;
    Rect bb = boss.worldBounds();

    for (Bullet& b : bullets.pool) {
        if (!b.active || b.owner != BulletOwner::PLAYER) continue;
        if (!rectsOverlap(bb, b.worldBounds())) continue;

        boss.hp -= b.damage;
        if (player.powerType != PowerUpType::LASER) {
            b.active = false;
        }

        if (boss.hp <= 0) {
            boss.hp = 0;
            boss.active = false;
            player.score += 2500;
        }
    }
}

void checkPowerUpPickup(Player& player, PowerUpPool& powerUps) {
    if (!player.active) return;
    Rect pb = player.worldBounds();

    for (PowerUp& p : powerUps.pool) {
        if (!p.active) continue;
        if (!rectsOverlap(pb, p.worldBounds())) continue;

        p.active = false;
        player.hasPowerUp = true;
        player.powerType = p.type;
        if (p.type == PowerUpType::SHIELD) {
            player.shieldTimer = 6.0f;
        } else {
            player.powerUpCount = 120;
        }
    }
}

} // namespace galaxy
