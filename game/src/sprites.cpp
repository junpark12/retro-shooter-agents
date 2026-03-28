#include "sprites.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace galaxy {

namespace {
inline void setColor(SDL_Renderer* r, Color c, Uint8 a = 255) { SDL_SetRenderDrawColor(r, c.r, c.g, c.b, a); }
inline void fill(SDL_Renderer* r, int x, int y, int w, int h) { SDL_Rect rect{x, y, w, h}; SDL_RenderFillRect(r, &rect); }

void drawFilledCircle(SDL_Renderer* renderer, int cx, int cy, int radius) {
    for (int dy = -radius; dy <= radius; ++dy) {
        const int span = static_cast<int>(std::sqrt(static_cast<float>(radius * radius - dy * dy)));
        SDL_RenderDrawLine(renderer, cx - span, cy + dy, cx + span, cy + dy);
    }
}

const char* playerKey(ShipType ship) {
    switch (ship) {
        case ShipType::BAGON: return SPR_PLAYER_BAGON;
        case ShipType::DAMUL: return SPR_PLAYER_DAMUL;
        case ShipType::GUNEX: return SPR_PLAYER_GUNEX;
    }
    return SPR_PLAYER_BAGON;
}

const char* enemyKey(EnemyType type) {
    switch (type) {
        case EnemyType::SMALL: return SPR_ENEMY_SMALL;
        case EnemyType::MEDIUM: return SPR_ENEMY_MEDIUM;
        case EnemyType::LARGE: return SPR_ENEMY_LARGE;
        case EnemyType::FAST: return SPR_ENEMY_FAST;
        case EnemyType::ARMORED: return SPR_ENEMY_ARMORED;
    }
    return SPR_ENEMY_SMALL;
}

const char* bossKey(int stageNum) {
    if (stageNum == 1) return SPR_BOSS_1;
    if (stageNum == 2) return SPR_BOSS_2;
    return SPR_BOSS_3;
}

const char* explosionKey(int frame) {
    static constexpr std::array<const char*, 4> keys = {
        SPR_EXPLOSION_0, SPR_EXPLOSION_1, SPR_EXPLOSION_2, SPR_EXPLOSION_3
    };
    return keys[std::clamp(frame / 2, 0, 3)];
}

const char* powerUpKey(PowerUpType type) {
    switch (type) {
        case PowerUpType::SPREAD: return SPR_POWERUP_SPREAD;
        case PowerUpType::LASER: return SPR_POWERUP_LASER;
        case PowerUpType::MISSILE: return SPR_POWERUP_MISSILE;
        case PowerUpType::SHIELD: return SPR_POWERUP_SHIELD;
        case PowerUpType::BOMB: return SPR_POWERUP_BOMB;
        case PowerUpType::POWER: return SPR_POWERUP_POWER;
    }
    return SPR_POWERUP_POWER;
}
} // namespace

void renderPlayerPrimitive(SDL_Renderer* renderer, int x, int y, ShipType ship) {
    if (ship == ShipType::BAGON) {
        // 날카로운 시안 삼각형
        setColor(renderer, COLOR_CYAN);
        for (int i = 0; i < 18; ++i) {
            SDL_RenderDrawLine(renderer, x + 14 - i / 2, y + i, x + 14 + i / 2, y + i);
        }
        setColor(renderer, COLOR_WHITE);
        fill(renderer, x + 13, y + 4, 2, 10);
    } else if (ship == ShipType::DAMUL) {
        // 파랑 넓은 삼각형
        setColor(renderer, COLOR_BLUE);
        for (int i = 0; i < 20; ++i) {
            SDL_RenderDrawLine(renderer, x + 14 - i, y + i, x + 14 + i, y + i);
        }
        setColor(renderer, COLOR_CYAN);
        fill(renderer, x + 10, y + 6, 8, 8);
    } else {
        // 초록 Y자형
        setColor(renderer, COLOR_GREEN);
        SDL_RenderDrawLine(renderer, x + 14, y + 2, x + 14, y + 26);
        SDL_RenderDrawLine(renderer, x + 14, y + 10, x + 6, y + 20);
        SDL_RenderDrawLine(renderer, x + 14, y + 10, x + 22, y + 20);
        fill(renderer, x + 11, y + 18, 6, 10);
    }
}

void renderEnemyPrimitive(SDL_Renderer* renderer, int x, int y, EnemyType type) {
    if (type == EnemyType::SMALL) {
        // 마젠타 다이아몬드 16x16
        setColor(renderer, COLOR_MAGENTA);
        for (int i = 0; i < 8; ++i) {
            SDL_RenderDrawLine(renderer, x + 8 - i, y + i, x + 8 + i, y + i);
            SDL_RenderDrawLine(renderer, x + i, y + 8 + i, x + 16 - i, y + 8 + i);
        }
    } else if (type == EnemyType::MEDIUM) {
        // 네온그린 육각형 24x24
        setColor(renderer, COLOR_GREEN);
        for (int i = 0; i < 6; ++i) {
            SDL_RenderDrawLine(renderer, x + 6 - i, y + 6 + i, x + 18 + i, y + 6 + i);
            SDL_RenderDrawLine(renderer, x + i, y + 12 + i, x + 24 - i, y + 12 + i);
        }
    } else if (type == EnemyType::LARGE) {
        // 빨강 각진 쐐기 32x28
        setColor(renderer, COLOR_RED);
        for (int i = 0; i < 20; ++i) {
            SDL_RenderDrawLine(renderer, x + 16 - i, y + i, x + 16 + i, y + i);
        }
        fill(renderer, x + 8, y + 18, 16, 10);
    } else if (type == EnemyType::FAST) {
        // 노랑 날카로운 삼각형 20x20
        setColor(renderer, COLOR_YELLOW);
        for (int i = 0; i < 14; ++i) {
            SDL_RenderDrawLine(renderer, x + 10 - i / 2, y + i, x + 10 + i / 2, y + i);
        }
    } else {
        // 오렌지 사각형 36x30
        setColor(renderer, COLOR_ORANGE);
        fill(renderer, x, y, 36, 30);
        setColor(renderer, COLOR_RED);
        fill(renderer, x + 4, y + 4, 28, 6);
    }
}

void renderBossPrimitive(SDL_Renderer* renderer, int x, int y, int stageNum) {
    if (stageNum == 1) {
        // 시안 배틀십 64x48
        setColor(renderer, COLOR_CYAN);
        fill(renderer, x + 4, y + 8, 56, 30);
        fill(renderer, x + 18, y, 28, 12);
    } else if (stageNum == 2) {
        // 노랑 순양함 72x56
        setColor(renderer, COLOR_YELLOW);
        fill(renderer, x + 6, y + 8, 60, 40);
        setColor(renderer, COLOR_ORANGE);
        fill(renderer, x + 20, y, 32, 12);
    } else {
        // 마젠타 요새 80x64
        setColor(renderer, COLOR_MAGENTA);
        fill(renderer, x + 6, y + 6, 68, 52);
        setColor(renderer, COLOR_RED);
        fill(renderer, x + 20, y + 18, 40, 24);
    }
    setColor(renderer, COLOR_WHITE);
    fill(renderer, x + 30, y + 20, 8, 8);
}

void renderBulletPrimitive(SDL_Renderer* renderer, int x, int y, BulletOwner owner, int colorIdx) {
    if (owner == BulletOwner::PLAYER) {
        setColor(renderer, COLOR_CYAN);
        drawFilledCircle(renderer, x + 3, y + 3, 3);
        setColor(renderer, COLOR_WHITE);
        fill(renderer, x + 2, y - 2, 2, 4);
    } else {
        if (colorIdx % 4 == 0) setColor(renderer, COLOR_MAGENTA);
        else if (colorIdx % 4 == 1) setColor(renderer, COLOR_RED);
        else if (colorIdx % 4 == 2) setColor(renderer, COLOR_YELLOW);
        else setColor(renderer, COLOR_GREEN);
        drawFilledCircle(renderer, x + 4, y + 4, 4);
    }
}

void renderPowerUpPrimitive(SDL_Renderer* renderer, int x, int y, PowerUpType type) {
    switch (type) {
        case PowerUpType::SPREAD: setColor(renderer, COLOR_YELLOW); break;
        case PowerUpType::LASER: setColor(renderer, COLOR_CYAN); break;
        case PowerUpType::MISSILE: setColor(renderer, COLOR_GREEN); break;
        case PowerUpType::SHIELD: setColor(renderer, COLOR_WHITE); break;
        case PowerUpType::BOMB: setColor(renderer, COLOR_ORANGE); break;
        case PowerUpType::POWER: setColor(renderer, COLOR_MAGENTA); break;
    }
    drawFilledCircle(renderer, x + 12, y + 12, 10);
}

void renderPlayerSprite(SDL_Renderer* renderer, const AssetManager& assets,
                        int x, int y, ShipType ship, bool invincible, int animFrame) {
    SDL_Texture* tex = assets.get(playerKey(ship));
    const int bob = static_cast<int>(std::sin(animFrame * 0.7f) * 2.0f);
    SDL_Rect dst{x, y + bob, 28, 36};

    if (tex) {
        if (invincible && (animFrame % 2 == 0)) {
            SDL_SetTextureColorMod(tex, 255, 255, 255);
            SDL_SetTextureAlphaMod(tex, 180);
        } else {
            SDL_SetTextureColorMod(tex, 255, 255, 255);
            SDL_SetTextureAlphaMod(tex, 255);
        }
        SDL_RenderCopy(renderer, tex, nullptr, &dst);
        SDL_SetTextureAlphaMod(tex, 255);
    } else {
        renderPlayerPrimitive(renderer, x, y + bob, ship);
    }
}

void renderEnemySprite(SDL_Renderer* renderer, const AssetManager& assets,
                       int x, int y, EnemyType type, bool lockedOn) {
    SDL_Texture* tex = assets.get(enemyKey(type));
    SDL_Rect dst{};
    if (type == EnemyType::SMALL) dst = {x, y, 16, 16};
    else if (type == EnemyType::MEDIUM) dst = {x, y, 24, 24};
    else if (type == EnemyType::LARGE) dst = {x, y, 32, 28};
    else if (type == EnemyType::FAST) dst = {x, y, 20, 20};
    else dst = {x, y, 36, 30};

    if (tex) {
        SDL_RenderCopy(renderer, tex, nullptr, &dst);
    } else {
        renderEnemyPrimitive(renderer, x, y, type);
    }

    if (lockedOn) {
        setColor(renderer, COLOR_RED);
        SDL_Rect lock = {dst.x - 2, dst.y - 2, dst.w + 4, dst.h + 4};
        SDL_RenderDrawRect(renderer, &lock);
    }
}

void renderBossSprite(SDL_Renderer* renderer, const AssetManager& assets,
                      int x, int y, int stageNum, bool lockedOn, int phase) {
    SDL_Texture* tex = assets.get(bossKey(stageNum));
    SDL_Rect dst = (stageNum == 1) ? SDL_Rect{x, y, 64, 48}
                   : (stageNum == 2) ? SDL_Rect{x, y, 72, 56}
                                     : SDL_Rect{x, y, 80, 64};

    if (tex) {
        if (phase >= 3) SDL_SetTextureColorMod(tex, 255, 120, 120);
        else if (phase == 2) SDL_SetTextureColorMod(tex, 255, 230, 180);
        else SDL_SetTextureColorMod(tex, 255, 255, 255);
        SDL_RenderCopy(renderer, tex, nullptr, &dst);
        SDL_SetTextureColorMod(tex, 255, 255, 255);
    } else {
        renderBossPrimitive(renderer, x, y, stageNum);
    }

    if (lockedOn) {
        renderLockOnReticle(renderer, x - 6, y - 6, dst.w + 12, dst.h + 12, 0.0f);
    }
}

void renderBulletSprite(SDL_Renderer* renderer, const AssetManager& assets,
                        int x, int y, BulletOwner owner, int colorIdx) {
    SDL_Texture* tex = (owner == BulletOwner::PLAYER)
        ? assets.get(SPR_BULLET_PLAYER)
        : assets.get(SPR_BULLET_ENEMY);
    SDL_Rect dst = (owner == BulletOwner::PLAYER) ? SDL_Rect{x, y, 8, 12}
                                                   : SDL_Rect{x, y, 8, 8};

    if (tex) {
        if (owner == BulletOwner::PLAYER) {
            SDL_SetTextureColorMod(tex, 120, 255, 255);
        } else {
            switch (colorIdx % 4) {
                case 0: SDL_SetTextureColorMod(tex, 255, 0, 255); break;
                case 1: SDL_SetTextureColorMod(tex, 255, 40, 40); break;
                case 2: SDL_SetTextureColorMod(tex, 255, 230, 80); break;
                default: SDL_SetTextureColorMod(tex, 80, 255, 80); break;
            }
        }
        SDL_RenderCopy(renderer, tex, nullptr, &dst);
        SDL_SetTextureColorMod(tex, 255, 255, 255);
    } else {
        renderBulletPrimitive(renderer, x, y, owner, colorIdx);
    }
}

void renderPowerUpSprite(SDL_Renderer* renderer, const AssetManager& assets, int x, int y, PowerUpType type) {
    SDL_Texture* tex = assets.get(powerUpKey(type));
    SDL_Rect dst{x, y, 24, 24};
    if (tex) {
        SDL_RenderCopy(renderer, tex, nullptr, &dst);
    } else {
        renderPowerUpPrimitive(renderer, x, y, type);
    }
}

void renderExplosion(SDL_Renderer* renderer, const AssetManager& assets,
                     int x, int y, int frame, bool big) {
    SDL_Texture* tex = assets.get(explosionKey(frame));
    const int size = big ? 64 : 32;
    SDL_Rect dst{x - size / 2, y - size / 2, size, size};

    if (tex) {
        SDL_RenderCopy(renderer, tex, nullptr, &dst);
        return;
    }

    const int pCount = 10 + std::clamp(frame, 0, 7) * 2;
    const int radius = (big ? 10 : 6) + frame * 3;
    const Uint8 alpha = static_cast<Uint8>(std::max(0, 220 - frame * 24));
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    setColor(renderer, (frame < 4) ? COLOR_YELLOW : COLOR_ORANGE, alpha);
    for (int i = 0; i < pCount; ++i) {
        const float a = static_cast<float>(i) * 6.2831853f / static_cast<float>(pCount);
        const int px = x + static_cast<int>(std::cos(a) * radius);
        const int py = y + static_cast<int>(std::sin(a) * radius);
        fill(renderer, px, py, big ? 3 : 2, big ? 3 : 2);
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void renderLockOnReticle(SDL_Renderer* renderer, int x, int y, int w, int h, float timer) {
    const float pulse = 1.0f - 0.08f * std::sin(timer * 8.0f);
    const int insetX = static_cast<int>((w * (1.0f - pulse)) * 0.5f);
    const int insetY = static_cast<int>((h * (1.0f - pulse)) * 0.5f);
    const SDL_Rect r{x + insetX, y + insetY, w - insetX * 2, h - insetY * 2};
    const bool blink = (static_cast<int>(timer * 8.0f) % 2) == 0;
    setColor(renderer, blink ? COLOR_RED : COLOR_MAGENTA);

    const int l = 8;
    // Top-left
    SDL_RenderDrawLine(renderer, r.x, r.y, r.x + l, r.y);
    SDL_RenderDrawLine(renderer, r.x, r.y, r.x, r.y + l);
    // Top-right
    SDL_RenderDrawLine(renderer, r.x + r.w, r.y, r.x + r.w - l, r.y);
    SDL_RenderDrawLine(renderer, r.x + r.w, r.y, r.x + r.w, r.y + l);
    // Bottom-left
    SDL_RenderDrawLine(renderer, r.x, r.y + r.h, r.x + l, r.y + r.h);
    SDL_RenderDrawLine(renderer, r.x, r.y + r.h, r.x, r.y + r.h - l);
    // Bottom-right
    SDL_RenderDrawLine(renderer, r.x + r.w, r.y + r.h, r.x + r.w - l, r.y + r.h);
    SDL_RenderDrawLine(renderer, r.x + r.w, r.y + r.h, r.x + r.w, r.y + r.h - l);
}

void renderBombFlash(SDL_Renderer* renderer, float bombTimer) {
    if (bombTimer <= 0.0f) return;
    const float ratio = std::clamp(bombTimer / 2.0f, 0.0f, 1.0f);
    const Uint8 alpha = static_cast<Uint8>(220.0f * ratio);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, alpha);
    SDL_Rect r{0, 0, SCREEN_W, SCREEN_H};
    SDL_RenderFillRect(renderer, &r);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void renderShipPreview(SDL_Renderer* renderer, const AssetManager& assets,
                       int x, int y, ShipType ship, bool selected) {
    SDL_Texture* tex = assets.get(playerKey(ship));
    SDL_Rect dst = selected ? SDL_Rect{x - 8, y - 8, 56, 72} : SDL_Rect{x, y, 48, 64};
    if (tex) {
        SDL_RenderCopy(renderer, tex, nullptr, &dst);
    } else {
        const float sx = static_cast<float>(dst.w) / 28.0f;
        const float sy = static_cast<float>(dst.h) / 36.0f;
        // primitive scale approximation
        renderPlayerPrimitive(renderer, dst.x + static_cast<int>(8 * (sx - 1.0f)),
                              dst.y + static_cast<int>(8 * (sy - 1.0f)), ship);
    }

    if (selected) {
        setColor(renderer, COLOR_YELLOW);
        SDL_Rect border{dst.x - 4, dst.y - 4, dst.w + 8, dst.h + 8};
        SDL_RenderDrawRect(renderer, &border);
    }
}

} // namespace galaxy
