#include "hud.h"

#include "player.h"

#include <algorithm>
#include <cstdio>
#include <string>

namespace galaxy {

namespace {
void drawFallbackGlyph(SDL_Renderer* renderer, int x, int y, char c, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    const int w = 4;
    const int h = 6;
    // Simple blocky pseudo-font fallback: draw border + center stroke variation.
    SDL_Rect border{x, y, w, h};
    SDL_RenderDrawRect(renderer, &border);
    if ((c % 2) == 0) {
        SDL_RenderDrawLine(renderer, x + 1, y + 1, x + 1, y + h - 2);
    } else {
        SDL_RenderDrawLine(renderer, x + 1, y + h / 2, x + w - 2, y + h / 2);
    }
}
} // namespace

void renderText(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y, SDL_Color color) {
    if (!text || !text[0]) return;
    if (font) {
        SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text, color);
        if (!surf) return;
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_Rect dst{x, y, surf->w, surf->h};
        SDL_FreeSurface(surf);
        if (!tex) return;
        SDL_RenderCopy(renderer, tex, nullptr, &dst);
        SDL_DestroyTexture(tex);
        return;
    }

    // Fallback pseudo pixel-font when no TTF font is available.
    int penX = x;
    for (const char* p = text; *p; ++p) {
        if (*p == ' ') {
            penX += 4;
            continue;
        }
        drawFallbackGlyph(renderer, penX, y, *p, color);
        penX += 6;
    }
}

void renderHUD(SDL_Renderer* renderer, const AssetManager& assets, TTF_Font* font, const Player& player, int stageNum, int hiScore) {
    char scoreBuf[64];
    std::snprintf(scoreBuf, sizeof(scoreBuf), "SCORE: %07d", std::max(0, player.score));
    renderText(renderer, font, scoreBuf, 10, 8, {255, 255, 255, 255});
    char hiScoreBuf[64];
    std::snprintf(hiScoreBuf, sizeof(hiScoreBuf), "HI:%07d", std::max(0, hiScore));
    renderText(renderer, font, hiScoreBuf, SCREEN_W - 130, 8, {255, 220, 80, 255});

    char stageBuf[32];
    std::snprintf(stageBuf, sizeof(stageBuf), "STAGE %d", std::max(1, stageNum));
    renderText(renderer, font, stageBuf, SCREEN_W / 2 - 48, 8, {255, 232, 0, 255});

    const int hudY = SCREEN_H - 28;
    SDL_Texture* lifeTex = assets.get(SPR_UI_LIFE);
    SDL_Texture* bombTex = assets.get(SPR_UI_BOMB);
    const int iconW = 14;
    const int iconH = 14;
    int cursorX = 10;

    for (int i = 0; i < player.lives; ++i) {
        SDL_Rect dst{cursorX + i * (iconW + 3), hudY, iconW, iconH};
        if (lifeTex) {
            SDL_RenderCopy(renderer, lifeTex, nullptr, &dst);
        } else {
            SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
            SDL_RenderDrawLine(renderer, dst.x + iconW / 2, dst.y, dst.x, dst.y + iconH);
            SDL_RenderDrawLine(renderer, dst.x + iconW / 2, dst.y, dst.x + iconW, dst.y + iconH);
            SDL_RenderDrawLine(renderer, dst.x, dst.y + iconH, dst.x + iconW, dst.y + iconH);
        }
    }
    cursorX += std::max(0, player.lives) * (iconW + 3) + 10;

    for (int i = 0; i < player.bombStock; ++i) {
        SDL_Rect dst{cursorX + i * (iconW + 3), hudY, iconW, iconH};
        if (bombTex) {
            SDL_RenderCopy(renderer, bombTex, nullptr, &dst);
        } else {
            SDL_SetRenderDrawColor(renderer, 255, 140, 0, 255);
            SDL_RenderFillRect(renderer, &dst);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &dst);
        }
    }

    SDL_Texture* powerBarTex = assets.get(SPR_UI_POWER_BAR);
    SDL_Rect powerFrame{SCREEN_W - 126, SCREEN_H - 24, 116, 12};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &powerFrame);
    const int fillW = std::clamp(player.powerLevel, 0, 4) * 28;
    SDL_Rect fillRect{powerFrame.x + 1, powerFrame.y + 1, fillW, powerFrame.h - 2};
    if (powerBarTex) {
        SDL_RenderCopy(renderer, powerBarTex, nullptr, &fillRect);
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
        SDL_RenderFillRect(renderer, &fillRect);
    }
    renderText(renderer, font, "PWR", SCREEN_W - 156, SCREEN_H - 26, {255, 255, 255, 255});
    char pwrLvlBuf[8];
    std::snprintf(pwrLvlBuf, sizeof(pwrLvlBuf), "Lv%d", std::clamp(player.powerLevel, 1, 4));
    renderText(renderer, font, pwrLvlBuf, SCREEN_W - 126, SCREEN_H - 26, {255, 100, 255, 255});

    if (player.shieldTimer > 0.0f) {
        const float ratio = std::clamp(player.shieldTimer / 5.0f, 0.0f, 1.0f);
        SDL_Rect shieldFrame{8, 28, SCREEN_W - 16, 8};
        SDL_Rect shieldFill{9, 29, static_cast<int>((SCREEN_W - 18) * ratio), 6};
        SDL_SetRenderDrawColor(renderer, 80, 120, 255, 255);
        SDL_RenderDrawRect(renderer, &shieldFrame);
        SDL_SetRenderDrawColor(renderer, 80, 180, 255, 255);
        SDL_RenderFillRect(renderer, &shieldFill);
    }

    if (player.sidecarActive) {
        const float ratio = std::clamp(player.sidecarTimer / 30.0f, 0.0f, 1.0f);
        SDL_Rect scFrame{8, 38, SCREEN_W - 16, 8};
        SDL_Rect scFill{9, 39, static_cast<int>((SCREEN_W - 18) * ratio), 6};
        SDL_SetRenderDrawColor(renderer, 60, 140, 255, 255);
        SDL_RenderDrawRect(renderer, &scFrame);
        SDL_SetRenderDrawColor(renderer, 120, 200, 255, 255);
        SDL_RenderFillRect(renderer, &scFill);
        renderText(renderer, font, "SC", 8, 38, {120, 200, 255, 255});
    }
}

void renderChargeBar(SDL_Renderer* renderer, int playerX, int playerY, float chargeRatio) {
    if (chargeRatio <= 0.0f) return;
    chargeRatio = std::clamp(chargeRatio, 0.0f, 1.0f);

    SDL_Rect frame{playerX, playerY + 52, 28, 4};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &frame);

    const int fillW = static_cast<int>((frame.w - 2) * chargeRatio);
    SDL_Rect fill{frame.x + 1, frame.y + 1, fillW, frame.h - 2};

    if (chargeRatio >= 1.0f) {
        const bool blink = ((SDL_GetTicks() / 80) % 2) == 0;
        if (blink) SDL_SetRenderDrawColor(renderer, 255, 245, 80, 255);
        else SDL_SetRenderDrawColor(renderer, 255, 180, 30, 255);
    } else {
        const Uint8 g = static_cast<Uint8>(140 + 100 * chargeRatio);
        const Uint8 b = static_cast<Uint8>(255 - 110 * chargeRatio);
        SDL_SetRenderDrawColor(renderer, 0, g, b, 255);
    }
    SDL_RenderFillRect(renderer, &fill);
}

void renderPowerUpTimer(SDL_Renderer* renderer, TTF_Font* font,
                        int powerUpCount, float shieldTimer, bool hasPowerUp) {
    if (hasPowerUp && powerUpCount > 0) {
        SDL_Color pwrColor{80, 255, 120, 255};
        if (powerUpCount <= 30) {
            const bool blink = ((SDL_GetTicks() / 200) % 2) == 0;
            pwrColor = blink ? SDL_Color{255, 80, 80, 255} : SDL_Color{160, 40, 40, 255};
        } else if (powerUpCount <= 60) {
            pwrColor = {255, 170, 40, 255};
        }

        char buf[32];
        std::snprintf(buf, sizeof(buf), "PWR: %d", powerUpCount);
        renderText(renderer, font, buf, SCREEN_W - 120, SCREEN_H - 44, pwrColor);
    }

    if (shieldTimer > 0.0f) {
        const int sec = std::max(1, static_cast<int>(shieldTimer + 0.99f));
        char shlBuf[16];
        std::snprintf(shlBuf, sizeof(shlBuf), "SHL: %ds", sec);
        renderText(renderer, font, shlBuf, 10, 40, {120, 200, 255, 255});
    }
}

void renderBossHP(SDL_Renderer* renderer, TTF_Font* font, int currentHp, int maxHp, int phase, float displayHp) {
    if (maxHp <= 0) return;
    const float ratio = (displayHp >= 0.0f)
                            ? std::clamp(displayHp / static_cast<float>(maxHp), 0.0f, 1.0f)
                            : std::clamp(static_cast<float>(currentHp) / static_cast<float>(maxHp), 0.0f, 1.0f);
    SDL_Rect border{8, 6, SCREEN_W - 16, 14};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &border);
    const bool blink = (ratio < 0.12f) && ((SDL_GetTicks() / 80) % 2 == 0);
    SDL_SetRenderDrawColor(renderer, blink ? 255 : 230, blink ? 255 : 30, blink ? 255 : 30, 255);
    SDL_Rect fill{border.x + 1, border.y + 1, static_cast<int>((border.w - 2) * ratio), border.h - 2};
    SDL_RenderFillRect(renderer, &fill);
    renderText(renderer, font, "BOSS", 10, 22, {255, 255, 255, 255});
    std::string stars(phase, '*');
    renderText(renderer, font, stars.c_str(), SCREEN_W - 50, 22, {255, 232, 0, 255});
}

void renderBossAttackWarning(SDL_Renderer* renderer, int bossX, int bossY, int bossW, int bossH,
                             float warningRatio) {
    warningRatio = std::clamp(warningRatio, 0.0f, 1.0f);
    const Uint8 alpha = static_cast<Uint8>(120.0f * warningRatio);
    if (alpha == 0) return;

    SDL_BlendMode prevBlend = SDL_BLENDMODE_NONE;
    SDL_GetRenderDrawBlendMode(renderer, &prevBlend);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, alpha);
    SDL_Rect area{bossX, bossY, bossW, bossH};
    SDL_RenderFillRect(renderer, &area);
    SDL_SetRenderDrawBlendMode(renderer, prevBlend);
}

void renderStageClear(SDL_Renderer* renderer, TTF_Font* font, int stageNum, int score) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 190);
    SDL_Rect r{0, 0, SCREEN_W, SCREEN_H};
    SDL_RenderFillRect(renderer, &r);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    std::string clearText = "STAGE " + std::to_string(stageNum) + " CLEAR!";
    renderText(renderer, font, clearText.c_str(), 120, 260, {255, 232, 0, 255});
    renderText(renderer, font, ("SCORE: " + std::to_string(score)).c_str(), 145, 305, {255, 255, 255, 255});
}

void renderGameOver(SDL_Renderer* renderer, TTF_Font* font, int score) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
    SDL_Rect r{0, 0, SCREEN_W, SCREEN_H};
    SDL_RenderFillRect(renderer, &r);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    renderText(renderer, font, "GAME OVER", 148, 270, {255, 30, 30, 255});
    renderText(renderer, font, ("SCORE: " + std::to_string(score)).c_str(), 150, 312, {255, 255, 255, 255});
    renderText(renderer, font, "PRESS ENTER", 154, 346, {255, 255, 255, 255});
}

void renderWarning(SDL_Renderer* renderer, TTF_Font* font, float timer) {
    const int tick = static_cast<int>(timer * 8.0f);
    if (tick % 2 == 0) return;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 200, 0, 0, 60);
    SDL_Rect overlay{0, 0, SCREEN_W, SCREEN_H};
    SDL_RenderFillRect(renderer, &overlay);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    renderText(renderer, font, "!! WARNING !!", SCREEN_W / 2 - 78, SCREEN_H / 2 - 20, {255, 50, 50, 255});
    renderText(renderer, font, "BOSS INCOMING", SCREEN_W / 2 - 78, SCREEN_H / 2 + 10, {255, 200, 0, 255});

    SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
    SDL_RenderDrawLine(renderer, 10, SCREEN_H / 2, 30, SCREEN_H / 2 - 15);
    SDL_RenderDrawLine(renderer, 10, SCREEN_H / 2, 30, SCREEN_H / 2 + 15);
    SDL_RenderDrawLine(renderer, 10, SCREEN_H / 2, 40, SCREEN_H / 2);
    SDL_RenderDrawLine(renderer, SCREEN_W - 10, SCREEN_H / 2, SCREEN_W - 30, SCREEN_H / 2 - 15);
    SDL_RenderDrawLine(renderer, SCREEN_W - 10, SCREEN_H / 2, SCREEN_W - 30, SCREEN_H / 2 + 15);
    SDL_RenderDrawLine(renderer, SCREEN_W - 10, SCREEN_H / 2, SCREEN_W - 40, SCREEN_H / 2);
}

void renderCombo(SDL_Renderer* renderer, TTF_Font* font, int comboCount, float comboTimer) {
    if (comboCount < 2) return;
    char buf[32];
    std::snprintf(buf, sizeof(buf), "COMBO x%d", comboCount);
    const Uint8 alpha = static_cast<Uint8>(std::min(255.0f, comboTimer * 255.0f));
    SDL_Color color{255, 255, 255, alpha};
    if (comboCount >= 5 && comboCount <= 9) {
        color = {255, 232, 0, alpha};
    } else if (comboCount >= 10 && comboCount <= 19) {
        color = {255, 140, 0, alpha};
    } else if (comboCount >= 20) {
        const bool blink = ((SDL_GetTicks() / 100) % 2) == 0;
        color = blink ? SDL_Color{255, 40, 40, alpha} : SDL_Color{140, 20, 20, alpha};
    }
    renderText(renderer, font, buf, SCREEN_W - 140, 50, color);

    if (comboCount >= 5) {
        char mulBuf[16];
        const float mul = 1.0f + static_cast<float>(comboCount / 4) * 0.5f;
        std::snprintf(mulBuf, sizeof(mulBuf), "%.1fx SCORE", std::min(mul, 8.0f));
        SDL_Color mulColor{255, 100, 255, alpha};
        renderText(renderer, font, mulBuf, SCREEN_W - 140, 66, mulColor);
    }
}

void renderPaused(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_BlendMode prevBlend = SDL_BLENDMODE_NONE;
    SDL_GetRenderDrawBlendMode(renderer, &prevBlend);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
    SDL_Rect overlay{0, 0, SCREEN_W, SCREEN_H};
    SDL_RenderFillRect(renderer, &overlay);
    SDL_SetRenderDrawBlendMode(renderer, prevBlend);

    renderText(renderer, font, "PAUSED", SCREEN_W / 2 - 48, SCREEN_H / 2 - 24, {255, 232, 0, 255});
    renderText(renderer, font, "PRESS P TO RESUME", SCREEN_W / 2 - 96, SCREEN_H / 2 + 8, {255, 255, 255, 255});
}

void renderContinue(SDL_Renderer* renderer, TTF_Font* font, int countdown) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 190);
    SDL_Rect r{0, 0, SCREEN_W, SCREEN_H};
    SDL_RenderFillRect(renderer, &r);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    renderText(renderer, font, "CONTINUE?", SCREEN_W / 2 - 55, 240, {255, 232, 0, 255});
    char numBuf[4];
    std::snprintf(numBuf, sizeof(numBuf), "%d", countdown);
    renderText(renderer, font, numBuf, SCREEN_W / 2 - 6, 290, {255, 100, 100, 255});
    renderText(renderer, font, "PRESS ENTER TO CONTINUE", SCREEN_W / 2 - 135, 340, {200, 200, 200, 255});
    renderText(renderer, font, "ESC: GIVE UP", SCREEN_W / 2 - 70, 365, {150, 150, 150, 255});
}

void renderVictory(SDL_Renderer* renderer, TTF_Font* font, int score) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
    SDL_Rect r{0, 0, SCREEN_W, SCREEN_H};
    SDL_RenderFillRect(renderer, &r);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    renderText(renderer, font, "VICTORY!", 158, 260, {255, 232, 0, 255});
    renderText(renderer, font, ("FINAL SCORE: " + std::to_string(score)).c_str(), 90, 310, {255, 255, 255, 255});
    renderText(renderer, font, "PRESS ENTER", 154, 346, {255, 255, 255, 255});
}

} // namespace galaxy
