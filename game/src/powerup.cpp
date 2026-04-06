#include "powerup.h"
#include "sprites.h"

namespace galaxy {

void spawnPowerUp(PowerUpPool& pp, Vec2 pos, PowerUpType type) {
    for (PowerUp& p : pp.pool) {
        if (p.active) continue;

        p.active = true;
        p.pos = pos;
        p.vel = {0.0f, POWERUP_FALL_SPEED};
        p.bounds = {0.0f, 0.0f, 20.0f, 20.0f};
        p.hp = 1;
        p.type = type;
        return;
    }
}

void updatePowerUps(PowerUpPool& pp, float dt, Vec2 playerPos) {
    for (PowerUp& p : pp.pool) {
        if (!p.active) continue;
        p.bobTimer += dt * 3.0f;
        p.pos.x += static_cast<float>(p.bobDir) * 20.0f * dt;
        if (p.pos.x < 4.0f || p.pos.x > SCREEN_W - 24.0f) p.bobDir *= -1;

        Vec2 toPlayer = playerPos - p.center();
        float dist = toPlayer.length();
        if (dist < POWERUP_MAGNET_RADIUS && dist > 1.0f) {
            Vec2 dir = toPlayer.normalized();
            p.vel = dir * POWERUP_MAGNET_SPEED;
        } else {
            // Outside magnet range: reset to default fall velocity.
            // Without this reset, p.vel would retain the previous magnet
            // direction, causing the powerup to fly off-screen (including
            // upward, which bypasses the lower-bounds deactivation check).
            p.vel = {0.0f, POWERUP_FALL_SPEED};
        }

        p.pos += p.vel * dt;
        if (p.pos.y > static_cast<float>(SCREEN_H) + 24.0f) {
            p.active = false;
        }
    }
}

void renderPowerUps(SDL_Renderer* renderer, const AssetManager& assets, const PowerUpPool& pp) {
    for (const PowerUp& p : pp.pool) {
        if (!p.active) continue;
        renderPowerUpSprite(renderer, assets,
                            static_cast<int>(p.pos.x),
                            static_cast<int>(p.pos.y),
                            p.type);
    }
}

} // namespace galaxy
