#include "sprites.h"

#include <algorithm>

namespace galaxy {

namespace {

inline void setColor(SDL_Renderer* renderer, Color c, Uint8 a = 255) {
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, a);
}

inline void fillRect(SDL_Renderer* renderer, int x, int y, int w, int h) {
    SDL_Rect r{x, y, w, h};
    SDL_RenderFillRect(renderer, &r);
}

void drawDiamond(SDL_Renderer* renderer, int x, int y, int size) {
    const int half = size / 2;
    for (int row = 0; row < size; ++row) {
        const int d = (row <= half) ? row : (size - 1 - row);
        const int w = d * 2 + 1;
        fillRect(renderer, x + half - d, y + row, w, 1);
    }
}

void drawExplosionParticle(SDL_Renderer* renderer, int cx, int cy, int ox, int oy, int sz) {
    fillRect(renderer, cx + ox - sz / 2, cy + oy - sz / 2, sz, sz);
}

} // namespace

void renderPlayerSprite(SDL_Renderer* renderer, int x, int y) {
    // Cyan delta-wing body (approx 28x36)
    setColor(renderer, COLOR_CYAN);
    for (int row = 0; row < 26; ++row) {
        const int d = row / 2;
        const int w = std::max(2, 28 - d * 2);
        fillRect(renderer, x + (28 - w) / 2, y + row, w, 1);
    }
    // Wing bar
    fillRect(renderer, x + 2, y + 18, 24, 3);

    // White cockpit core
    setColor(renderer, COLOR_WHITE);
    fillRect(renderer, x + 12, y + 8, 4, 12);
    fillRect(renderer, x + 10, y + 20, 8, 4);

    // Orange thrusters
    setColor(renderer, COLOR_ORANGE);
    fillRect(renderer, x + 7, y + 28, 3, 6);
    fillRect(renderer, x + 18, y + 28, 3, 6);
    fillRect(renderer, x + 13, y + 30, 2, 5);
}

void renderEnemySprite(SDL_Renderer* renderer, int x, int y, EnemyType type) {
    switch (type) {
        case EnemyType::SMALL: {
            setColor(renderer, COLOR_MAGENTA);
            drawDiamond(renderer, x, y, 16);
            setColor(renderer, COLOR_WHITE);
            fillRect(renderer, x + 7, y + 7, 2, 2);
            break;
        }
        case EnemyType::MEDIUM: {
            // Neon-green octagon-ish body (~24x24)
            setColor(renderer, COLOR_GREEN);
            for (int row = 0; row < 24; ++row) {
                int inset = 0;
                if (row < 4) inset = 4 - row;
                else if (row > 19) inset = row - 19;
                const int w = 24 - inset * 2;
                fillRect(renderer, x + inset, y + row, w, 1);
            }
            setColor(renderer, COLOR_WHITE);
            fillRect(renderer, x + 10, y + 9, 4, 4);
            break;
        }
        case EnemyType::LARGE: {
            // Red angular wedge (~32x28)
            setColor(renderer, COLOR_RED);
            for (int row = 0; row < 28; ++row) {
                const int inset = row / 3;
                const int w = std::max(8, 32 - inset);
                fillRect(renderer, x + inset / 2, y + row, w, 1);
            }
            fillRect(renderer, x + 2, y + 9, 28, 6);
            setColor(renderer, COLOR_ORANGE);
            fillRect(renderer, x + 13, y + 4, 6, 18);
            break;
        }
    }
}

void renderBossSprite(SDL_Renderer* renderer, int x, int y, int stageNum) {
    if (stageNum <= 1) {
        // Stage 1: cyan battleship (~64x48)
        setColor(renderer, COLOR_CYAN);
        fillRect(renderer, x + 4, y + 8, 56, 28);
        fillRect(renderer, x + 0, y + 18, 64, 12);
        fillRect(renderer, x + 12, y + 0, 40, 8);
        setColor(renderer, COLOR_WHITE);
        fillRect(renderer, x + 28, y + 10, 8, 18);
        fillRect(renderer, x + 10, y + 24, 10, 6);
        fillRect(renderer, x + 44, y + 24, 10, 6);
        return;
    }

    if (stageNum == 2) {
        // Stage 2: yellow cruiser (~72x56)
        setColor(renderer, COLOR_YELLOW);
        for (int row = 0; row < 56; ++row) {
            int inset = 0;
            if (row < 10) inset = 10 - row;
            else if (row > 45) inset = row - 45;
            const int w = 72 - inset * 2;
            fillRect(renderer, x + inset, y + row, w, 1);
        }
        setColor(renderer, COLOR_WHITE);
        fillRect(renderer, x + 31, y + 14, 10, 28);
        setColor(renderer, COLOR_ORANGE);
        fillRect(renderer, x + 8, y + 24, 12, 8);
        fillRect(renderer, x + 52, y + 24, 12, 8);
        return;
    }

    // Stage 3: magenta fortress (~80x64)
    setColor(renderer, COLOR_MAGENTA);
    fillRect(renderer, x + 6, y + 10, 68, 44);
    fillRect(renderer, x + 0, y + 20, 80, 20);
    fillRect(renderer, x + 18, y + 2, 44, 10);
    fillRect(renderer, x + 10, y + 54, 18, 8);
    fillRect(renderer, x + 52, y + 54, 18, 8);
    setColor(renderer, COLOR_WHITE);
    fillRect(renderer, x + 37, y + 18, 6, 28);
    setColor(renderer, COLOR_RED);
    fillRect(renderer, x + 14, y + 24, 10, 10);
    fillRect(renderer, x + 56, y + 24, 10, 10);
}

void renderBulletSprite(SDL_Renderer* renderer, int x, int y, BulletOwner owner) {
    if (owner == BulletOwner::PLAYER) {
        setColor(renderer, COLOR_CYAN);
        fillRect(renderer, x + 1, y, 2, 12);
        fillRect(renderer, x, y + 2, 4, 8);
    } else {
        setColor(renderer, COLOR_MAGENTA);
        fillRect(renderer, x + 1, y, 4, 6);
        fillRect(renderer, x, y + 1, 6, 4);
        setColor(renderer, COLOR_WHITE);
        fillRect(renderer, x + 2, y + 2, 2, 2);
    }
}

void renderPowerUpSprite(SDL_Renderer* renderer, int x, int y, PowerUpType type) {
    switch (type) {
        case PowerUpType::SPREAD: {
            setColor(renderer, COLOR_YELLOW);
            // Cross
            fillRect(renderer, x + 8, y + 2, 2, 14);
            fillRect(renderer, x + 2, y + 8, 14, 2);
            // Diagonal spark bits
            fillRect(renderer, x + 4, y + 4, 2, 2);
            fillRect(renderer, x + 12, y + 4, 2, 2);
            fillRect(renderer, x + 4, y + 12, 2, 2);
            fillRect(renderer, x + 12, y + 12, 2, 2);
            break;
        }
        case PowerUpType::LASER: {
            setColor(renderer, COLOR_CYAN);
            fillRect(renderer, x + 2, y, 4, 24);
            setColor(renderer, COLOR_WHITE);
            fillRect(renderer, x + 3, y + 2, 2, 20);
            break;
        }
        case PowerUpType::MISSILE: {
            setColor(renderer, COLOR_GREEN);
            for (int row = 0; row < 18; ++row) {
                const int d = (row < 9) ? row : (17 - row);
                const int w = std::max(2, d + 2);
                fillRect(renderer, x + (14 - w) / 2, y + row, w, 1);
            }
            setColor(renderer, COLOR_WHITE);
            fillRect(renderer, x + 6, y + 5, 2, 8);
            break;
        }
        case PowerUpType::SHIELD: {
            setColor(renderer, COLOR_WHITE);
            // 20x20 circle-ish ring
            fillRect(renderer, x + 6, y + 0, 8, 1);
            fillRect(renderer, x + 4, y + 1, 12, 1);
            fillRect(renderer, x + 2, y + 3, 16, 1);
            fillRect(renderer, x + 1, y + 5, 18, 1);
            fillRect(renderer, x + 0, y + 8, 1, 4);
            fillRect(renderer, x + 19, y + 8, 1, 4);
            fillRect(renderer, x + 1, y + 14, 18, 1);
            fillRect(renderer, x + 2, y + 16, 16, 1);
            fillRect(renderer, x + 4, y + 18, 12, 1);
            fillRect(renderer, x + 6, y + 19, 8, 1);
            break;
        }
    }
}

void renderExplosion(SDL_Renderer* renderer, int x, int y, int frame) {
    const int f = std::clamp(frame, 0, 7);
    const int radius = 2 + f * 3;
    const int particleSize = (f < 3) ? 2 : 1;

    if (f < 2) setColor(renderer, COLOR_WHITE);
    else if (f < 5) setColor(renderer, COLOR_YELLOW);
    else setColor(renderer, COLOR_ORANGE, 220);

    // Core flash
    fillRect(renderer, x - particleSize, y - particleSize, particleSize * 2 + 1, particleSize * 2 + 1);

    // Cardinal particles
    drawExplosionParticle(renderer, x, y, radius, 0, particleSize);
    drawExplosionParticle(renderer, x, y, -radius, 0, particleSize);
    drawExplosionParticle(renderer, x, y, 0, radius, particleSize);
    drawExplosionParticle(renderer, x, y, 0, -radius, particleSize);

    // Diagonal particles
    const int diag = static_cast<int>(radius * 0.7f);
    drawExplosionParticle(renderer, x, y, diag, diag, particleSize);
    drawExplosionParticle(renderer, x, y, -diag, diag, particleSize);
    drawExplosionParticle(renderer, x, y, diag, -diag, particleSize);
    drawExplosionParticle(renderer, x, y, -diag, -diag, particleSize);
}

} // namespace galaxy
