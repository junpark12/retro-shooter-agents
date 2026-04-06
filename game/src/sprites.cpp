#include "sprites.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

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

// 3x5 pixel font bitmap: each char is 3 columns x 5 rows, LSB = leftmost pixel
// bit layout per row: bit0=left, bit1=mid, bit2=right
struct Glyph { uint8_t rows[5]; };

static const Glyph& glyphFor(char c) {
    // A-Z subset needed
    static const Glyph glyphs[] = {
        // 'A' (index 0)
        {0b010, 0b101, 0b111, 0b101, 0b101},
        // 'B'
        {0b110, 0b101, 0b110, 0b101, 0b110},
        // 'C'
        {0b011, 0b100, 0b100, 0b100, 0b011},
        // 'D' (index 3)
        {0b110, 0b101, 0b101, 0b101, 0b110},
        // 'E'
        {0b111, 0b100, 0b110, 0b100, 0b111},
        // 'F' (5)
        {0b111, 0b100, 0b110, 0b100, 0b100},
        // 'G' (6)
        {0b011, 0b100, 0b101, 0b101, 0b011},
        // 'H' (7)
        {0b101, 0b101, 0b111, 0b101, 0b101},
        // 'I' (8)
        {0b111, 0b010, 0b010, 0b010, 0b111},
        // 'J' (9)
        {0b111, 0b001, 0b001, 0b101, 0b010},
        // 'K' (10)
        {0b101, 0b101, 0b110, 0b101, 0b101},
        // 'L' (11)
        {0b100, 0b100, 0b100, 0b100, 0b111},
        // 'M' (12)
        {0b101, 0b111, 0b101, 0b101, 0b101},
        // 'N' (13)
        {0b101, 0b111, 0b111, 0b101, 0b101},
        // 'O' (14)
        {0b010, 0b101, 0b101, 0b101, 0b010},
        // 'P' (15)
        {0b110, 0b101, 0b110, 0b100, 0b100},
        // 'Q' (16)
        {0b010, 0b101, 0b101, 0b011, 0b001},
        // 'R' (17)
        {0b110, 0b101, 0b110, 0b101, 0b101},
        // 'S' (18)
        {0b011, 0b100, 0b010, 0b001, 0b110},
        // 'T' (19)
        {0b111, 0b010, 0b010, 0b010, 0b010},
        // 'U' (20)
        {0b101, 0b101, 0b101, 0b101, 0b010},
        // 'V' (21)
        {0b101, 0b101, 0b101, 0b010, 0b010},
        // 'W' (22)
        {0b101, 0b101, 0b111, 0b111, 0b101},
        // 'X' (23)
        {0b101, 0b101, 0b010, 0b101, 0b101},
        // 'Y' (24)
        {0b101, 0b101, 0b010, 0b010, 0b010},
        // 'Z' (25)
        {0b111, 0b001, 0b010, 0b100, 0b111},
    };
    if (c >= 'A' && c <= 'Z') return glyphs[c - 'A'];
    static const Glyph fallback = {0b111, 0b101, 0b101, 0b101, 0b111};
    return fallback;
}

// Draw a single 3x5 glyph at pixel position (px, py)
void drawGlyph(SDL_Renderer* r, int px, int py, char c) {
    const auto& g = glyphFor(c);
    for (int row = 0; row < 5; ++row) {
        for (int col = 0; col < 3; ++col) {
            if (g.rows[row] & (1 << col)) {
                SDL_RenderDrawPoint(r, px + col, py + row);
            }
        }
    }
}

// Draw a string of uppercase letters starting at (px, py)
void drawLabel(SDL_Renderer* r, int px, int py, const char* text) {
    int x = px;
    for (const char* p = text; *p; ++p) {
        drawGlyph(r, x, py, *p);
        x += 4;  // 3px char + 1px gap
    }
}

const char* powerUpLabel(PowerUpType type) {
    switch (type) {
        case PowerUpType::SPREAD: return "SPR";
        case PowerUpType::LASER: return "LAS";
        case PowerUpType::MISSILE: return "MSL";
        case PowerUpType::SHIELD: return "SHL";
        case PowerUpType::BOMB: return "BOM";
        case PowerUpType::POWER: return "UP";
        case PowerUpType::SIDECAR: return "SC";
    }
    return nullptr;
}

void setPowerUpLabelColor(SDL_Renderer* renderer, PowerUpType type) {
    switch (type) {
        case PowerUpType::SPREAD: SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); break;
        case PowerUpType::LASER: SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255); break;
        case PowerUpType::MISSILE: SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); break;
        case PowerUpType::SHIELD: SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); break;
        case PowerUpType::BOMB: SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255); break;
        case PowerUpType::POWER: SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255); break;
        case PowerUpType::SIDECAR: SDL_SetRenderDrawColor(renderer, 120, 200, 255, 255); break;
    }
}

void renderPowerUpLabel(SDL_Renderer* renderer, int x, int y, PowerUpType type) {
    const char* label = powerUpLabel(type);
    if (!label) return;

    int len = 0;
    for (const char* p = label; *p; ++p) ++len;
    const int labelW = len > 0 ? (len - 1) * 4 + 3 : 0;
    const int lx = x + 12 - labelW / 2;
    const int ly = y + 10;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
    drawLabel(renderer, lx, ly, label);
    setPowerUpLabelColor(renderer, type);
    drawLabel(renderer, lx - 1, ly - 1, label);
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

const char* bossFallbackKey(int stageNum) {
    if (stageNum == 1) return SPR_BOSS_1_FB;
    if (stageNum == 2) return SPR_BOSS_2_FB;
    return SPR_BOSS_3_FB;
}

const char* enemyBulletStripKey(int colorIdx) {
    switch (colorIdx % 4) {
        case 0: return SPR_BULLET_STRIP_BLUE;
        case 1: return SPR_BULLET_STRIP_RED;
        case 2: return SPR_BULLET_STRIP_YELLOW;
        default: return SPR_BULLET_STRIP_GREEN;
    }
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
        case PowerUpType::SIDECAR: return SPR_POWERUP_POWER;
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
    } else if (owner == BulletOwner::BOSS) {
        SDL_SetRenderDrawColor(renderer, 255, 230, 0, 255);
        drawFilledCircle(renderer, x + 7, y + 14, 12);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawPoint(renderer, x + 7, y + 14);
    } else {
        if (colorIdx % 4 == 0) setColor(renderer, COLOR_MAGENTA);
        else if (colorIdx % 4 == 1) setColor(renderer, COLOR_RED);
        else if (colorIdx % 4 == 2) setColor(renderer, COLOR_YELLOW);
        else setColor(renderer, COLOR_GREEN);
        drawFilledCircle(renderer, x + 10, y + 20, 16);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 200);
        SDL_RenderDrawPoint(renderer, x + 10, y + 20);
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
        case PowerUpType::SIDECAR: SDL_SetRenderDrawColor(renderer, 120, 200, 255, 255); break;
    }
    drawFilledCircle(renderer, x + 12, y + 12, 10);
    renderPowerUpLabel(renderer, x, y, type);
}

void renderPlayerSprite(SDL_Renderer* renderer, const AssetManager& assets,
                        int x, int y, ShipType ship, bool invincible, int animFrame) {
    SDL_Texture* tex = assets.get(playerKey(ship));
    const int bob = static_cast<int>(std::sin(animFrame * 0.7f) * 2.0f);
    SDL_Rect dst{x, y + bob, 48, 48};

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

void renderHitboxIndicator(SDL_Renderer* renderer, int cx, int cy, float radius) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    const float pi = 3.14159265f;
    const float innerRadius = radius * 0.5f;

    // Outer ring
    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 180);
    for (int angle = 0; angle < 360; angle += 10) {
        const float rad = static_cast<float>(angle) * pi / 180.0f;
        const int px = cx + static_cast<int>(radius * std::cos(rad));
        const int py = cy + static_cast<int>(radius * std::sin(rad));
        SDL_RenderDrawPoint(renderer, px, py);
    }

    // Inner ring
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
    for (int angle = 0; angle < 360; angle += 10) {
        const float rad = static_cast<float>(angle) * pi / 180.0f;
        const int px = cx + static_cast<int>(innerRadius * std::cos(rad));
        const int py = cy + static_cast<int>(innerRadius * std::sin(rad));
        SDL_RenderDrawPoint(renderer, px, py);
    }

    // Center point
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect center{cx - 1, cy - 1, 2, 2};
    SDL_RenderFillRect(renderer, &center);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void renderEnemySprite(SDL_Renderer* renderer, const AssetManager& assets,
                       int x, int y, EnemyType type, bool lockedOn) {
    SDL_Texture* tex = assets.get(enemyKey(type));
    SDL_Rect dst{};
    if (type == EnemyType::SMALL) dst = {x, y, 24, 24};
    else if (type == EnemyType::MEDIUM) dst = {x, y, 32, 32};
    else if (type == EnemyType::LARGE) dst = {x, y, 48, 40};
    else if (type == EnemyType::FAST) dst = {x, y, 28, 24};
    else dst = {x, y, 40, 36};

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
    if (!tex) {
        tex = assets.get(bossFallbackKey(stageNum));
    }
    SDL_Rect dst = (stageNum == 1) ? SDL_Rect{x, y, 160, 120}
                   : (stageNum == 2) ? SDL_Rect{x, y, 180, 140}
                                       : SDL_Rect{x, y, 200, 160};

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
    SDL_Texture* tex = nullptr;
    SDL_Rect dst{};
    SDL_Rect src{};
    SDL_Rect* srcPtr = nullptr;
    bool usingStrip = false;

    if (owner == BulletOwner::PLAYER) {
        tex = assets.get(SPR_BULLET_PLAYER);
        dst = SDL_Rect{x, y, 9, 36};
    } else if (owner == BulletOwner::BOSS) {
        tex = assets.get(SPR_BOSS_MISSILE);
        dst = SDL_Rect{x - 8, y - 16, 24, 48};
    } else {
        tex = assets.get(enemyBulletStripKey(colorIdx));
        if (tex) {
            int texW = 0;
            int texH = 0;
            SDL_QueryTexture(tex, nullptr, nullptr, &texW, &texH);
            if (texW > 0 && texH > 0) {
                const int orbW = std::max(1, texW / 3);
                src = SDL_Rect{0, 0, orbW, texH};
                srcPtr = &src;
                dst = SDL_Rect{x - 10, y - 20, 20, 40};
                usingStrip = true;
            }
        }

        if (!usingStrip) {
            tex = assets.get(SPR_BULLET_ENEMY);
            dst = SDL_Rect{x - 10, y - 20, 20, 40};
            srcPtr = nullptr;
        }
    }

    if (tex) {
        if (owner == BulletOwner::PLAYER) {
            SDL_SetTextureColorMod(tex, 120, 255, 255);
        } else if (owner == BulletOwner::BOSS) {
            SDL_SetTextureColorMod(tex, 255, 230, 0);
        } else {
            if (usingStrip) {
                switch (colorIdx % 4) {
                    case 0: SDL_SetTextureColorMod(tex, 255, 80, 255); break;
                    case 1: SDL_SetTextureColorMod(tex, 255, 60, 60); break;
                    case 2: SDL_SetTextureColorMod(tex, 255, 240, 50); break;
                    default: SDL_SetTextureColorMod(tex, 80, 255, 80); break;
                }
            } else {
                switch (colorIdx % 4) {
                    case 0: SDL_SetTextureColorMod(tex, 255, 0, 255); break;
                    case 1: SDL_SetTextureColorMod(tex, 255, 40, 40); break;
                    case 2: SDL_SetTextureColorMod(tex, 255, 230, 80); break;
                    default: SDL_SetTextureColorMod(tex, 80, 255, 80); break;
                }
            }
        }
        if (owner == BulletOwner::ENEMY && usingStrip) {
            SDL_RenderCopyEx(renderer, tex, srcPtr, &dst, 90.0, nullptr, SDL_FLIP_NONE);
        } else {
            SDL_RenderCopy(renderer, tex, srcPtr, &dst);
        }
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
        renderPowerUpLabel(renderer, x, y, type);
    } else {
        renderPowerUpPrimitive(renderer, x, y, type);
    }
}

void renderExplosion(SDL_Renderer* renderer, const AssetManager& assets,
                     int x, int y, int frame, bool big) {
    SDL_Texture* sheetTex = assets.get(SPR_EXPLOSION_SHEET);
    const int animFrame = std::clamp(frame, 0, 7);
    const int size = big ? 64 : 32;
    SDL_Rect dst{x - size / 2, y - size / 2, size, size};

    if (sheetTex) {
        int sheetW = 0;
        int sheetH = 0;
        SDL_QueryTexture(sheetTex, nullptr, nullptr, &sheetW, &sheetH);

        if (sheetW > 0 && sheetH > 0) {
            const int sectionW = sheetW / 3;
            const int sectionH = sheetH / 3;
            const int frameW = sectionW / 8;
            const int frameH = sectionH / 4;
            if (sectionW > 0 && sectionH > 0 && frameW > 0 && frameH > 0) {
                const int sectionCol = big ? 0 : (animFrame % 3);
                const int sectionRow = big ? 0 : 1;
                const int frameCol = animFrame;
                const int frameRow = 0;

                SDL_Rect src{
                    sectionCol * sectionW + frameCol * frameW,
                    sectionRow * sectionH + frameRow * frameH,
                    frameW,
                    frameH
                };

                const Uint8 alpha = static_cast<Uint8>(std::max(8, 255 - animFrame * 32));
                SDL_SetTextureAlphaMod(sheetTex, alpha);
                SDL_RenderCopy(renderer, sheetTex, &src, &dst);
                SDL_SetTextureAlphaMod(sheetTex, 255);
                return;
            }
        }
    }

    SDL_Texture* tex = assets.get(explosionKey(frame));

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
    SDL_Rect dst = selected ? SDL_Rect{x - 8, y - 8, 72, 72} : SDL_Rect{x, y, 64, 64};
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

void renderEngineExhaust(SDL_Renderer* renderer, const AssetManager& assets,
                         int shipX, int shipY, int shipW, int fireFrame) {
    static constexpr std::array<const char*, 8> fireKeys = {
        SPR_ENGINE_FIRE_0, SPR_ENGINE_FIRE_1, SPR_ENGINE_FIRE_2, SPR_ENGINE_FIRE_3,
        SPR_ENGINE_FIRE_4, SPR_ENGINE_FIRE_5, SPR_ENGINE_FIRE_6, SPR_ENGINE_FIRE_7
    };
    const int idx = std::clamp(fireFrame % 8, 0, 7);
    SDL_Texture* tex = assets.get(fireKeys[idx]);

    const int fw = 20, fh = 24;
    const int fx = shipX + shipW / 2 - fw / 2;
    const int fy = shipY + 36;
    SDL_Rect dst{fx, fy, fw, fh};

    if (tex) {
        SDL_SetTextureAlphaMod(tex, 200);
        SDL_RenderCopy(renderer, tex, nullptr, &dst);
        SDL_SetTextureAlphaMod(tex, 255);
    } else {
        setColor(renderer, COLOR_ORANGE);
        fill(renderer, fx + 6, fy, 8, 12);
        setColor(renderer, COLOR_YELLOW);
        fill(renderer, fx + 8, fy + 2, 4, 6);
    }
}

void renderSidecars(SDL_Renderer* renderer, const AssetManager& assets,
                    int playerX, int playerY, int animFrame) {
    (void)assets;
    // Sidecar companion ships: small cyan triangular ships on left and right
    // Left sidecar at (playerX - 18, playerY + 4)
    // Right sidecar at (playerX + 34, playerY + 4)
    const int bob = static_cast<int>(std::sin(animFrame * 0.7f) * 2.0f);

    // Pulsing effect to indicate timed power-up
    const Uint8 alpha = static_cast<Uint8>(180 + 75 * std::sin(animFrame * 0.5f));
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Left sidecar
    int lx = playerX - 18;
    int ly = playerY + 4 + bob;
    SDL_SetRenderDrawColor(renderer, 100, 200, 255, alpha);
    // Small triangle pointing up (5x8)
    for (int i = 0; i < 8; ++i) {
        SDL_RenderDrawLine(renderer, lx + 2 - i / 3, ly + i, lx + 2 + i / 3, ly + i);
    }
    // Cockpit highlight
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, alpha);
    fill(renderer, lx + 1, ly + 1, 2, 2);

    // Right sidecar
    int rx = playerX + 34;
    int ry = playerY + 4 + bob;
    SDL_SetRenderDrawColor(renderer, 100, 200, 255, alpha);
    for (int i = 0; i < 8; ++i) {
        SDL_RenderDrawLine(renderer, rx + 2 - i / 3, ry + i, rx + 2 + i / 3, ry + i);
    }
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, alpha);
    fill(renderer, rx + 1, ry + 1, 2, 2);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

} // namespace galaxy
