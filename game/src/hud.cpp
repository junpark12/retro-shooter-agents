#include "hud.h"

#include "player.h"

#include <array>
#include <string>
#include <unordered_map>

namespace galaxy {

namespace {

using Glyph = std::array<const char*, 7>;

const std::unordered_map<char, Glyph>& font5x7() {
    static const std::unordered_map<char, Glyph> kFont = {
        {' ', {"00000","00000","00000","00000","00000","00000","00000"}},
        {'!', {"00100","00100","00100","00100","00100","00000","00100"}},
        {':', {"00000","00100","00100","00000","00100","00100","00000"}},
        {'0', {"01110","10001","10011","10101","11001","10001","01110"}},
        {'1', {"00100","01100","00100","00100","00100","00100","01110"}},
        {'2', {"01110","10001","00001","00010","00100","01000","11111"}},
        {'3', {"11110","00001","00001","01110","00001","00001","11110"}},
        {'4', {"00010","00110","01010","10010","11111","00010","00010"}},
        {'5', {"11111","10000","11110","00001","00001","10001","01110"}},
        {'6', {"00110","01000","10000","11110","10001","10001","01110"}},
        {'7', {"11111","00001","00010","00100","01000","01000","01000"}},
        {'8', {"01110","10001","10001","01110","10001","10001","01110"}},
        {'9', {"01110","10001","10001","01111","00001","00010","11100"}},
        {'A', {"01110","10001","10001","11111","10001","10001","10001"}},
        {'B', {"11110","10001","10001","11110","10001","10001","11110"}},
        {'C', {"01110","10001","10000","10000","10000","10001","01110"}},
        {'D', {"11110","10001","10001","10001","10001","10001","11110"}},
        {'E', {"11111","10000","10000","11110","10000","10000","11111"}},
        {'F', {"11111","10000","10000","11110","10000","10000","10000"}},
        {'G', {"01110","10001","10000","10111","10001","10001","01110"}},
        {'H', {"10001","10001","10001","11111","10001","10001","10001"}},
        {'I', {"01110","00100","00100","00100","00100","00100","01110"}},
        {'J', {"00001","00001","00001","00001","10001","10001","01110"}},
        {'K', {"10001","10010","10100","11000","10100","10010","10001"}},
        {'L', {"10000","10000","10000","10000","10000","10000","11111"}},
        {'M', {"10001","11011","10101","10101","10001","10001","10001"}},
        {'N', {"10001","11001","10101","10011","10001","10001","10001"}},
        {'O', {"01110","10001","10001","10001","10001","10001","01110"}},
        {'P', {"11110","10001","10001","11110","10000","10000","10000"}},
        {'Q', {"01110","10001","10001","10001","10101","10010","01101"}},
        {'R', {"11110","10001","10001","11110","10100","10010","10001"}},
        {'S', {"01111","10000","10000","01110","00001","00001","11110"}},
        {'T', {"11111","00100","00100","00100","00100","00100","00100"}},
        {'U', {"10001","10001","10001","10001","10001","10001","01110"}},
        {'V', {"10001","10001","10001","10001","01010","01010","00100"}},
        {'W', {"10001","10001","10001","10101","10101","11011","10001"}},
        {'X', {"10001","01010","00100","00100","00100","01010","10001"}},
        {'Y', {"10001","01010","00100","00100","00100","00100","00100"}},
        {'Z', {"11111","00001","00010","00100","01000","10000","11111"}},
    };
    return kFont;
}

inline void setColor(SDL_Renderer* renderer, Color c, Uint8 a = 255) {
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, a);
}

void renderChar(SDL_Renderer* renderer, int x, int y, char c, int scale, Color color) {
    const auto& font = font5x7();
    auto it = font.find(c);
    if (it == font.end()) it = font.find(' ');
    const Glyph& g = it->second;

    setColor(renderer, color);
    for (int row = 0; row < 7; ++row) {
        for (int col = 0; col < 5; ++col) {
            if (g[row][col] != '1') continue;
            SDL_Rect px{x + col * scale, y + row * scale, scale, scale};
            SDL_RenderFillRect(renderer, &px);
        }
    }
}

void renderText(SDL_Renderer* renderer, int x, int y, const std::string& text, int scale, Color color) {
    int penX = x;
    for (char c : text) {
        renderChar(renderer, penX, y, c, scale, color);
        penX += (5 + 1) * scale;
    }
}

void renderMiniShip(SDL_Renderer* renderer, int x, int y) {
    setColor(renderer, COLOR_CYAN);
    SDL_Rect a{x + 3, y + 0, 2, 2};
    SDL_Rect b{x + 2, y + 2, 4, 2};
    SDL_Rect c{x + 1, y + 4, 6, 2};
    SDL_Rect d{x + 3, y + 6, 2, 2};
    SDL_RenderFillRect(renderer, &a);
    SDL_RenderFillRect(renderer, &b);
    SDL_RenderFillRect(renderer, &c);
    SDL_RenderFillRect(renderer, &d);
}

std::string powerName(const Player& player) {
    if (!player.hasPowerUp && player.powerType == PowerUpType::SPREAD) {
        return "NORMAL";
    }
    switch (player.powerType) {
        case PowerUpType::SPREAD:  return "SPREAD";
        case PowerUpType::LASER:   return "LASER";
        case PowerUpType::MISSILE: return "MISSILE";
        case PowerUpType::SHIELD:  return "SHIELD";
    }
    return "NORMAL";
}

void renderOverlay(SDL_Renderer* renderer, const std::string& title, const std::string& scoreLine, Color titleColor) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect overlay{0, 0, SCREEN_W, SCREEN_H};
    SDL_RenderFillRect(renderer, &overlay);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    const int titleScale = 4;
    const int titleW = static_cast<int>(title.size()) * 6 * titleScale;
    renderText(renderer, (SCREEN_W - titleW) / 2, SCREEN_H / 2 - 70, title, titleScale, titleColor);

    const int scoreScale = 2;
    const int scoreW = static_cast<int>(scoreLine.size()) * 6 * scoreScale;
    renderText(renderer, (SCREEN_W - scoreW) / 2, SCREEN_H / 2 + 10, scoreLine, scoreScale, COLOR_WHITE);
}

} // namespace

void renderHUD(SDL_Renderer* renderer, const Player& player, int stageNum) {
    const int scale = 2;
    renderText(renderer, 10, 10, "SCORE:" + std::to_string(player.score), scale, COLOR_GREEN);

    const std::string stageTxt = "STAGE:" + std::to_string(stageNum);
    const int stageW = static_cast<int>(stageTxt.size()) * 6 * scale;
    renderText(renderer, (SCREEN_W - stageW) / 2, 10, stageTxt, scale, COLOR_CYAN);

    renderText(renderer, 10, SCREEN_H - 24, "LIVES", 1, COLOR_WHITE);
    for (int i = 0; i < player.lives; ++i) {
        renderMiniShip(renderer, 48 + i * 12, SCREEN_H - 24);
    }

    const std::string pwr = powerName(player);
    const std::string pwrLine = "PWR:" + pwr;
    const int pwrW = static_cast<int>(pwrLine.size()) * 6;
    renderText(renderer, SCREEN_W - pwrW - 10, SCREEN_H - 24, pwrLine, 1, COLOR_YELLOW);

    if (player.shieldTimer > 0.0f) {
        const int barX = SCREEN_W - 150;
        const int barY = 36;
        const int barW = 130;
        const int barH = 8;
        setColor(renderer, COLOR_WHITE);
        SDL_Rect border{barX, barY, barW, barH};
        SDL_RenderDrawRect(renderer, &border);

        const float ratio = (player.shieldTimer > 5.0f) ? 1.0f : (player.shieldTimer / 5.0f);
        setColor(renderer, COLOR_CYAN);
        SDL_Rect fill{barX + 1, barY + 1, static_cast<int>((barW - 2) * ratio), barH - 2};
        SDL_RenderFillRect(renderer, &fill);
        renderText(renderer, barX - 44, barY - 1, "SHLD", 1, COLOR_CYAN);
    }
}

void renderStageClear(SDL_Renderer* renderer, int stageNum, int score) {
    renderOverlay(renderer,
                  "STAGE CLEAR!",
                  "STAGE:" + std::to_string(stageNum) + " SCORE:" + std::to_string(score),
                  COLOR_GREEN);
}

void renderGameOver(SDL_Renderer* renderer, int score) {
    renderOverlay(renderer, "GAME OVER", "SCORE:" + std::to_string(score), COLOR_RED);
}

void renderVictory(SDL_Renderer* renderer, int score) {
    renderOverlay(renderer, "VICTORY!", "FINAL SCORE:" + std::to_string(score), COLOR_CYAN);
}

} // namespace galaxy
