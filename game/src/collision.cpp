#include "collision.h"

#include "boss.h"
#include "bullet.h"
#include "enemy.h"
#include "player.h"
#include "powerup.h"
#include "audio.h"

#include <algorithm>
#include <cstdlib>

namespace galaxy {

namespace {
PowerUpType randomDrop() {
    switch (std::rand() % 6) {
        case 0: return PowerUpType::SPREAD;
        case 1: return PowerUpType::LASER;
        case 2: return PowerUpType::MISSILE;
        case 3: return PowerUpType::SHIELD;
        case 4: return PowerUpType::BOMB;
        default: return PowerUpType::POWER;
    }
}

int scoreForEnemy(EnemyType t) {
    switch (t) {
        case EnemyType::SMALL:   return 100;
        case EnemyType::MEDIUM:  return 300;
        case EnemyType::LARGE:   return 500;
        case EnemyType::FAST:    return 200;
        case EnemyType::ARMORED: return 700;
    }
    return 100;
}

bool shouldDrop(EnemyType t) {
    switch (t) {
        case EnemyType::LARGE:   return true;
        case EnemyType::MEDIUM:  return (std::rand() % 100) < 30;
        case EnemyType::SMALL:   return (std::rand() % 100) < 10;
        case EnemyType::FAST:    return (std::rand() % 100) < 20;
        case EnemyType::ARMORED: return (std::rand() % 100) < 50;
    }
    return false;
}

void damagePlayer(Player& player, int dmg) {
    player.hp -= dmg;
    player.invincibleTimer = 1.0f;
    if (player.hp > 0) return;
    player.lives--;
    if (player.lives > 0) {
        player.hp = 3;
        player.pos = {SCREEN_W * 0.5f - 14.0f, SCREEN_H - 90.0f};
        player.invincibleTimer = 2.0f;
    } else {
        player.active = false;
    }
}
}

void checkBulletEnemyCollision(BulletPool& bullets, EnemyPool& enemies,
                               Player& player, PowerUpPool& powerUps,
                               AudioManager* audio) {
    for (Bullet& b : bullets.pool) {
        if (!b.active || b.owner != BulletOwner::PLAYER) continue;
        Rect bb = b.worldBounds();

        const bool isLaser = (player.powerType == PowerUpType::LASER);
        for (Enemy& e : enemies.pool) {
            if (!e.active) continue;
            if (!rectsOverlap(bb, e.worldBounds())) continue;

            e.hp -= b.damage;
            if (!isLaser) {
                b.active = false;
            }

            if (e.hp <= 0) {
                e.active = false;
                player.score += scoreForEnemy(e.type);
                if (audio) audio->playSFX(SFX_EXPLODE_SMALL);

                if (shouldDrop(e.type)) {
                    spawnPowerUp(powerUps, {e.pos.x + e.bounds.w * 0.5f - 10.0f, e.pos.y}, randomDrop());
                }
            }

            if (!isLaser) {
                break; // non-laser bullet consumed by first enemy hit; stop inner loop
            }
            // laser: continue iterating enemies — piercing shot hits all in path
        }
    }
}

void checkBulletPlayerCollision(BulletPool& bullets, Player& player, AudioManager* audio) {
    if (!player.active) return;
    if (player.invincibleTimer > 0.0f || player.shieldTimer > 0.0f) return;

    Vec2 pc = player.center();
    for (Bullet& b : bullets.pool) {
        if (!b.active || b.owner != BulletOwner::ENEMY) continue;
        const Vec2 bc = b.center();
        if (!circlesOverlap(pc.x, pc.y, player.hitRadius, bc.x, bc.y, b.hitRadius)) continue;

        b.active = false;
        if (audio) audio->playSFX(SFX_PLAYER_HIT);
        damagePlayer(player, b.damage);
        break;
    }
}

void checkPlayerEnemyCollision(Player& player, EnemyPool& enemies, AudioManager* audio) {
    if (!player.active) return;
    if (player.invincibleTimer > 0.0f || player.shieldTimer > 0.0f) return;

    Rect pb = player.worldBounds();
    for (Enemy& e : enemies.pool) {
        if (!e.active) continue;
        if (!rectsOverlap(pb, e.worldBounds())) continue;

        e.active = false;
        if (audio) audio->playSFX(SFX_PLAYER_HIT);
        damagePlayer(player, 1);
        break;
    }
}

void checkBulletBossCollision(BulletPool& bullets, Boss& boss, Player& player,
                              AudioManager* audio) {
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
            player.score += 10000 * boss.stageNum;
            if (audio) audio->playSFX(SFX_EXPLODE_BIG);
        }
    }
}

void checkPowerUpPickup(Player& player, PowerUpPool& powerUps, AudioManager* audio) {
    if (!player.active) return;
    Rect pb = player.worldBounds();

    for (PowerUp& p : powerUps.pool) {
        if (!p.active) continue;
        if (!rectsOverlap(pb, p.worldBounds())) continue;

        p.active = false;
        if (audio) audio->playSFX(SFX_POWERUP);
        switch (p.type) {
            case PowerUpType::SPREAD:
            case PowerUpType::LASER:
            case PowerUpType::MISSILE:
                player.hasPowerUp = true;
                player.powerType = p.type;
                player.powerUpCount = 120;
                break;
            case PowerUpType::SHIELD:
                player.hasPowerUp = true;
                player.powerType = p.type;
                player.shieldTimer = 6.0f;
                break;
            case PowerUpType::BOMB:
                player.bombStock = std::min(9, player.bombStock + 1);
                break;
            case PowerUpType::POWER:
                player.powerLevel = std::min(4, player.powerLevel + 1);
                break;
        }
    }
}

} // namespace galaxy
