// Galaxy Storm — Power-up pool implementation

#include "powerup.h"
#include "sprites.h"

namespace galaxy {

void spawnPowerUp(PowerUpPool& pp, Vec2 pos, PowerUpType type) {
    for (int i = 0; i < MAX_POWERUPS; ++i) {
        PowerUp& pu = pp.pool[i];
        if (!pu.active) {
            pu.pos    = pos;
            pu.vel    = { 0.0f, 60.0f }; // falls downward at 60 px/s
            pu.type   = type;
            pu.active = true;
            pu.hp     = 1;
            pu.bounds = { 0.0f, 0.0f, 20.0f, 20.0f };
            return;
        }
    }
}

void updatePowerUps(PowerUpPool& pp, float dt) {
    for (int i = 0; i < MAX_POWERUPS; ++i) {
        PowerUp& pu = pp.pool[i];
        if (!pu.active) continue;

        pu.pos += pu.vel * dt;

        if (pu.pos.y > static_cast<float>(SCREEN_H)) {
            pu.active = false;
        }
    }
}

void renderPowerUps(SDL_Renderer* renderer, const PowerUpPool& pp) {
    for (int i = 0; i < MAX_POWERUPS; ++i) {
        const PowerUp& pu = pp.pool[i];
        if (!pu.active) continue;
        renderPowerUpSprite(renderer,
                            static_cast<int>(pu.pos.x),
                            static_cast<int>(pu.pos.y),
                            pu.type);
    }
}

} // namespace galaxy
