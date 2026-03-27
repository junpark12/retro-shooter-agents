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
        p.pos += p.vel * dt;
        if (p.pos.y > static_cast<float>(SCREEN_H) + 24.0f) {
            p.active = false;
        }
    }
}

void renderPowerUps(SDL_Renderer* renderer, const PowerUpPool& pp) {
    for (const PowerUp& p : pp.pool) {
        if (!p.active) continue;
        renderPowerUpSprite(renderer, static_cast<int>(p.pos.x), static_cast<int>(p.pos.y), p.type);
    }
}

} // namespace galaxy
