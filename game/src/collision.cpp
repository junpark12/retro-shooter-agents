#include "collision.h"

#include "boss.h"
#include "bullet.h"
#include "enemy.h"
#include "player.h"
#include "powerup.h"
#include "audio.h"
#include "particles.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace galaxy {

namespace {
PowerUpType randomDrop() {
    int r = std::rand() % 100;
    if (r < 5)  return PowerUpType::SIDECAR;  // 5%
    if (r < 17) return PowerUpType::SPEEDUP;  // 12%
    if (r < 30) return PowerUpType::POWER;    // 13%
    if (r < 43) return PowerUpType::BOMB;     // 13%
    if (r < 57) return PowerUpType::SHIELD;   // 14%
    if (r < 72) return PowerUpType::MISSILE;  // 15%
    if (r < 86) return PowerUpType::LASER;    // 14%
    return PowerUpType::SPREAD;               // 14%
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
                               AudioManager* audio, ParticleSystem* ps) {
    for (Bullet& b : bullets.pool) {
        if (!b.active || b.owner != BulletOwner::PLAYER) continue;
        Rect bb = b.worldBounds();

        const bool isLaser = (player.powerType == PowerUpType::LASER);
        for (Enemy& e : enemies.pool) {
            if (!e.active) continue;
            if (!rectsOverlap(bb, e.worldBounds())) continue;

            e.hp -= b.damage;
            // Slight knockback on hit (push enemy away from bullet direction)
            const Vec2 pushDir = (e.center() - b.center()).normalized();
            e.pos += pushDir * 3.0f;  // 3px knockback
            if (!isLaser) {
                b.active = false;
            }

            // Spawn hit spark on damage (not only on kill).
            if (ps && e.hp > 0) {
                spawnHitSpark(*ps, b.center(), 255, 200, 50);
            }

            if (e.hp <= 0) {
                e.active = false;
                player.comboCount++;
                player.comboTimer = 3.5f;
                player.scoreMultiplier = 1.0f + static_cast<float>(player.comboCount / 4) * 0.5f;
                player.scoreMultiplier = std::min(player.scoreMultiplier, 8.0f);
                player.score += static_cast<int>(scoreForEnemy(e.type) * player.scoreMultiplier);
                if (audio) {
                    switch (e.type) {
                        case EnemyType::SMALL:
                        case EnemyType::FAST:
                            audio->playSFX(SFX_EXPLODE_SMALL);
                            break;
                        case EnemyType::MEDIUM: {
                            switch (std::rand() % 3) {
                                case 0: audio->playSFX(SFX_EXPLODE_MED1); break;
                                case 1: audio->playSFX(SFX_EXPLODE_MED2); break;
                                default: audio->playSFX(SFX_EXPLODE_MED3); break;
                            }
                            break;
                        }
                        case EnemyType::LARGE:
                        case EnemyType::ARMORED:
                            audio->playSFX(SFX_EXPLODE_BIG);
                            break;
                    }
                    if (player.comboCount > 0 && (player.comboCount % 5) == 0) {
                        audio->playSFX(SFX_COMBO);
                    }
                }
                if (ps) {
                    spawnExplosion(*ps, e.center(),
                                   e.type == EnemyType::LARGE || e.type == EnemyType::ARMORED);
                }

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

void checkBulletPlayerCollision(BulletPool& bullets, Player& player, AudioManager* audio, ParticleSystem* ps) {
    if (!player.active) return;
    if (player.invincibleTimer > 0.0f || player.shieldTimer > 0.0f) return;

    Vec2 pc = player.center();
    for (Bullet& b : bullets.pool) {
        if (!b.active || (b.owner != BulletOwner::ENEMY && b.owner != BulletOwner::BOSS)) continue;
        const Vec2 bc = b.center();
        const float dx = pc.x - bc.x;
        const float dy = pc.y - bc.y;
        const float dist = std::sqrt(dx * dx + dy * dy);
        const float hitDist = player.hitRadius + b.hitRadius;
        const float grazeDist = hitDist + 8.0f;
        if (dist <= grazeDist && dist > hitDist) {
            const bool canPlayGrazeSfx = (player.grazeFlashTimer <= 0.0f);
            player.grazeCount++;
            player.grazeScore += 50;
            player.score += 50;
            player.grazeFlashTimer = 0.15f;
            if (ps) spawnGrazeSpark(*ps, player.center());
            if (canPlayGrazeSfx && audio) {
                audio->playSFX(SFX_GRAZE);
            }
        }
        if (!circlesOverlap(pc.x, pc.y, player.hitRadius, bc.x, bc.y, b.hitRadius)) continue;

        b.active = false;
        if (audio) audio->playSFX(SFX_PLAYER_HIT);
        // Player knockback: push away from bullet direction
        const Vec2 pushDir = (player.center() - bc).normalized();
        player.pos += pushDir * 8.0f;
        player.pos.x = std::clamp(player.pos.x, 0.0f, static_cast<float>(SCREEN_W) - 28.0f);
        player.pos.y = std::clamp(player.pos.y, 0.0f, static_cast<float>(SCREEN_H) - 36.0f);
        damagePlayer(player, b.damage);
        break;
    }
}

void checkPlayerEnemyCollision(Player& player, EnemyPool& enemies,
                               AudioManager* audio, ParticleSystem* ps) {
    if (!player.active) return;
    if (player.invincibleTimer > 0.0f || player.shieldTimer > 0.0f) return;

    Rect pb = player.worldBounds();
    for (Enemy& e : enemies.pool) {
        if (!e.active) continue;
        if (!rectsOverlap(pb, e.worldBounds())) continue;

        e.active = false;
        if (ps) spawnExplosion(*ps, e.center(), false);
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
        if (audio) {
            switch (p.type) {
                case PowerUpType::SPREAD:
                    audio->playSFX(SFX_POWERUP_SPREAD);
                    break;
                case PowerUpType::LASER:
                    audio->playSFX(SFX_POWERUP_LASER);
                    break;
                case PowerUpType::MISSILE:
                    audio->playSFX(SFX_POWERUP_MISSILE);
                    break;
                case PowerUpType::SHIELD:
                    audio->playSFX(SFX_POWERUP_SHIELD);
                    break;
                case PowerUpType::BOMB:
                    audio->playSFX(SFX_POWERUP_BOMB);
                    break;
                case PowerUpType::MAGNET:
                    audio->playSFX(SFX_POWERUP_MAGNET);
                    break;
                case PowerUpType::SPEEDUP:
                    audio->playSFX(SFX_POWERUP);
                    break;
                default:
                    audio->playSFX(SFX_POWERUP);
                    break;
            }
        }
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
            case PowerUpType::MAGNET:
                player.magnetTimer = 8.0f;
                break;
            case PowerUpType::POWER:
                player.powerLevel = std::min(4, player.powerLevel + 1);
                break;
            case PowerUpType::SPEEDUP:
                player.speedLevel = std::min(PLAYER_SPEED_MAX_LVL, player.speedLevel + 1);
                break;
            case PowerUpType::SIDECAR:
                player.sidecarActive = true;
                player.sidecarTimer = 30.0f;
                player.sidecarFireTimer = 0.0f;
                break;
        }
    }
}

} // namespace galaxy
