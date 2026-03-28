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

void renderHUD(SDL_Renderer* renderer, const AssetManager& assets, TTF_Font* font, const Player& player, int stageNum) {
    char scoreBuf[64];
    std::snprintf(scoreBuf, sizeof(scoreBuf), "SCORE: %07d", std::max(0, player.score));
    renderText(renderer, font, scoreBuf, 10, 8, {255, 255, 255, 255});

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

    if (player.shieldTimer > 0.0f) {
        const float ratio = std::clamp(player.shieldTimer / 5.0f, 0.0f, 1.0f);
        SDL_Rect shieldFrame{8, 28, SCREEN_W - 16, 8};
        SDL_Rect shieldFill{9, 29, static_cast<int>((SCREEN_W - 18) * ratio), 6};
        SDL_SetRenderDrawColor(renderer, 80, 120, 255, 255);
        SDL_RenderDrawRect(renderer, &shieldFrame);
        SDL_SetRenderDrawColor(renderer, 80, 180, 255, 255);
        SDL_RenderFillRect(renderer, &shieldFill);
    }
}

void renderBossHP(SDL_Renderer* renderer, TTF_Font* font, int currentHp, int maxHp, int phase) {
    if (maxHp <= 0) return;
    const float ratio = std::clamp(static_cast<float>(currentHp) / static_cast<float>(maxHp), 0.0f, 1.0f);
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
