#include "powerup.h"
#include "sprites.h"

namespace galaxy {

void spawnPowerUp(PowerUpPool& pp, Vec2 pos, PowerUpType type) {
    for (PowerUp& p : pp.pool) {
        if (p.active) continue;

        p.active = true;
        p.pos = pos;
        p.vel = {0.0f, 120.0f};
        p.bounds = {0.0f, 0.0f, 20.0f, 20.0f};
        p.hp = 1;
        p.type = type;
        return;
    }
}

void updatePowerUps(PowerUpPool& pp, float dt) {
    for (PowerUp& p : pp.pool) {
        if (!p.active) continue;
        p.bobTimer += dt * 3.0f;
        p.pos.x += static_cast<float>(p.bobDir) * 20.0f * dt;
        if (p.pos.x < 4.0f || p.pos.x > SCREEN_W - 24.0f) p.bobDir *= -1;
        p.pos += p.vel * dt;
        if (p.pos.y > static_cast<float>(SCREEN_H) + 24.0f) {
            p.active = false;
        }
    }
}

void renderPowerUps(SDL_Renderer* renderer, const AssetManager& assets, const PowerUpPool& pp) {
    (void)assets;
    for (const PowerUp& p : pp.pool) {
        if (!p.active) continue;
        renderPowerUpPrimitive(renderer, static_cast<int>(p.pos.x), static_cast<int>(p.pos.y), p.type);
    }
}

} // namespace galaxy
