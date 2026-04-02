#include "menu.h"

#include "hud.h"
#include "sprites.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

namespace galaxy {

void updateMenu(Menu& m, float dt) {
    m.animTimer += dt;
    m.blinkTimer += dt;
    if (m.blinkTimer >= 0.5f) {
        m.blinkTimer = 0.0f;
        m.blinkVisible = !m.blinkVisible;
    }
}

GameState handleMenuEvent(Menu& m, const SDL_Event& e) {
    if (e.type != SDL_KEYDOWN) return GameState::TITLE;

    const SDL_Scancode key = e.key.keysym.scancode;
    if (key == SDL_SCANCODE_UP || key == SDL_SCANCODE_W) {
        m.selectedOption = (m.selectedOption - 1 + static_cast<int>(MenuOption::COUNT))
                         % static_cast<int>(MenuOption::COUNT);
        return GameState::TITLE;
    }
    if (key == SDL_SCANCODE_DOWN || key == SDL_SCANCODE_S) {
        m.selectedOption = (m.selectedOption + 1) % static_cast<int>(MenuOption::COUNT);
        return GameState::TITLE;
    }

    if (key == SDL_SCANCODE_RETURN || key == SDL_SCANCODE_SPACE) {
        if (m.selectedOption == static_cast<int>(MenuOption::START)) {
            return GameState::SHIP_SELECT;
        }
        if (m.selectedOption == static_cast<int>(MenuOption::HIGH_SCORE)) {
            return GameState::HIGH_SCORE;
        }
        return GameState::QUIT;
    }
    if (key == SDL_SCANCODE_ESCAPE) {
        return GameState::QUIT;
    }
    return GameState::TITLE;
}

void renderMenu(SDL_Renderer* renderer, const AssetManager&, TTF_Font* font, const Menu& m, int hiScore) {
    SDL_SetRenderDrawColor(renderer, 35, 5, 10, 255);
    SDL_Rect bg{0, 0, SCREEN_W, SCREEN_H};
    SDL_RenderFillRect(renderer, &bg);

    // static decorative stars
    SDL_SetRenderDrawColor(renderer, 120, 150, 255, 255);
    SDL_Rect s0{50, 80, 2, 2}; SDL_RenderFillRect(renderer, &s0);
    SDL_SetRenderDrawColor(renderer, 255, 255, 180, 255);
    SDL_Rect s1{390, 140, 2, 2}; SDL_RenderFillRect(renderer, &s1);
    SDL_SetRenderDrawColor(renderer, 0, 220, 255, 255);
    SDL_Rect s2{420, 60, 1, 1}; SDL_RenderFillRect(renderer, &s2);

    const int titleY = 120 + static_cast<int>(std::sin(m.animTimer * 2.0f) * 4.0f);
    renderText(renderer, font, "GALAXY STORM", 92, titleY, SDL_Color{255, 232, 0, 255});
    renderText(renderer, font, "BULLET HELL SHOOTER", 128, titleY + 34, SDL_Color{0, 255, 255, 255});

    const bool startSel = m.selectedOption == static_cast<int>(MenuOption::START);
    const bool highSel = m.selectedOption == static_cast<int>(MenuOption::HIGH_SCORE);
    const bool quitSel = m.selectedOption == static_cast<int>(MenuOption::QUIT);
    renderText(renderer, font, "START", 196, 300, startSel ? SDL_Color{255, 232, 0, 255} : SDL_Color{255, 255, 255, 255});
    renderText(renderer, font, "HIGH SCORE", 156, 336, highSel ? SDL_Color{255, 232, 0, 255} : SDL_Color{255, 255, 255, 255});
    renderText(renderer, font, "QUIT", 204, 372, quitSel ? SDL_Color{255, 232, 0, 255} : SDL_Color{255, 255, 255, 255});

    if (m.blinkVisible) {
        const int cursorY = (m.selectedOption == static_cast<int>(MenuOption::START)) ? 300
                          : (m.selectedOption == static_cast<int>(MenuOption::HIGH_SCORE)) ? 336
                                                                                            : 372;
        renderText(renderer, font, ">", 136, cursorY, SDL_Color{255, 232, 0, 255});
    }

    std::ostringstream oss;
    oss << "HI-SCORE " << std::setw(6) << std::setfill('0') << std::max(0, hiScore);
    renderText(renderer, font, oss.str().c_str(), 124, SCREEN_H - 46, SDL_Color{220, 220, 220, 255});
    renderText(renderer, font, "PRESS SPACE TO START", 120, 404, SDL_Color{220, 220, 220, 255});
    
    // Controls guide
    renderText(renderer, font, "- CONTROLS -", 154, 430, SDL_Color{255, 220, 0, 255});
    renderText(renderer, font, "MOVE  : ARROWS / WASD", 90, 452, SDL_Color{160, 210, 255, 255});
    renderText(renderer, font, "FIRE  : SPACE / X", 90, 468, SDL_Color{160, 210, 255, 255});
    renderText(renderer, font, "LOCK  : Z", 90, 484, SDL_Color{160, 210, 255, 255});
    renderText(renderer, font, "BOMB  : C", 90, 500, SDL_Color{160, 210, 255, 255});
    renderText(renderer, font, "CHARGE: HOLD SPACE/X", 90, 516, SDL_Color{160, 210, 255, 255});
    renderText(renderer, font, "C 2026 GALAXY STORM TEAM", 118, SCREEN_H - 24, SDL_Color{130, 130, 130, 255});
}

GameState handleHighScoreEvent(const SDL_Event& e) {
    if (e.type != SDL_KEYDOWN) return GameState::HIGH_SCORE;
    const SDL_Scancode key = e.key.keysym.scancode;
    if (key == SDL_SCANCODE_ESCAPE || key == SDL_SCANCODE_BACKSPACE ||
        key == SDL_SCANCODE_RETURN || key == SDL_SCANCODE_SPACE) {
        return GameState::TITLE;
    }
    return GameState::HIGH_SCORE;
}

void renderHighScore(SDL_Renderer* renderer, TTF_Font* font, int hiScore) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 20, 255);
    SDL_Rect bg{0, 0, SCREEN_W, SCREEN_H};
    SDL_RenderFillRect(renderer, &bg);

    renderText(renderer, font, "HIGH SCORE", 146, 80, SDL_Color{255, 232, 0, 255});

    std::ostringstream oss;
    oss << std::setw(8) << std::setfill('0') << std::max(0, hiScore);
    renderText(renderer, font, oss.str().c_str(), 142, 200, SDL_Color{255, 255, 255, 255});

    renderText(renderer, font, "PRESS ANY KEY TO RETURN", 80, 450, SDL_Color{220, 220, 220, 255});
}

void updateShipSelect(ShipSelect& ss, float dt) {
    ss.previewTimer += dt;
    ss.blinkTimer += dt;
    if (ss.blinkTimer >= 0.4f) {
        ss.blinkTimer = 0.0f;
        ss.blinkVisible = !ss.blinkVisible;
    }
}

GameState handleShipSelectEvent(ShipSelect& ss, const SDL_Event& e, ShipType& outShip) {
    if (e.type != SDL_KEYDOWN) return GameState::SHIP_SELECT;
    const SDL_Scancode key = e.key.keysym.scancode;
    if (key == SDL_SCANCODE_LEFT || key == SDL_SCANCODE_A) {
        ss.selectedShip = (ss.selectedShip + 2) % 3;
        return GameState::SHIP_SELECT;
    }
    if (key == SDL_SCANCODE_RIGHT || key == SDL_SCANCODE_D) {
        ss.selectedShip = (ss.selectedShip + 1) % 3;
        return GameState::SHIP_SELECT;
    }
    if (key == SDL_SCANCODE_ESCAPE || key == SDL_SCANCODE_BACKSPACE) {
        return GameState::TITLE;
    }
    if (key == SDL_SCANCODE_RETURN || key == SDL_SCANCODE_SPACE) {
        outShip = (ss.selectedShip == 0) ? ShipType::BAGON
                : (ss.selectedShip == 1) ? ShipType::DAMUL
                                         : ShipType::GUNEX;
        return GameState::PLAYING;
    }
    return GameState::SHIP_SELECT;
}

void renderShipSelect(SDL_Renderer* renderer, const AssetManager& assets, TTF_Font* font, const ShipSelect& ss) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 20, 255);
    SDL_Rect bg{0, 0, SCREEN_W, SCREEN_H};
    SDL_RenderFillRect(renderer, &bg);

    renderText(renderer, font, "SELECT YOUR SHIP", 110, 72, SDL_Color{255, 232, 0, 255});

    renderShipPreview(renderer, assets, 48, 176, ShipType::BAGON, ss.selectedShip == 0);
    renderShipPreview(renderer, assets, 190, 176, ShipType::DAMUL, ss.selectedShip == 1);
    renderShipPreview(renderer, assets, 332, 176, ShipType::GUNEX, ss.selectedShip == 2);

    std::array<const char*, 3> stats = {
        "POWER **** SPEED *** AREA **",
        "POWER *** SPEED *** AREA *****",
        "POWER ***** SPEED ** AREA ***"
    };
    std::array<const char*, 3> desc = {
        "BAGON: HIGH FOCUS LASER LOCK-ON",
        "DAMUL: WIDE SPREAD FOR CROWD CONTROL",
        "GUNEX: HEAVY BURST + HOMING MISSILES"
    };
    std::array<const char*, 3> names = {"BAGON", "DAMUL", "GUNEX"};

    const int i = ss.selectedShip;
    renderText(renderer, font, names[i], 206, 300, SDL_Color{255, 255, 255, 255});
    renderText(renderer, font, stats[i], 52, 350, SDL_Color{0, 255, 255, 255});
    renderText(renderer, font, desc[i], 38, 388, SDL_Color{220, 220, 220, 255});

    if (ss.blinkVisible) {
        const int x = (i == 0) ? 66 : (i == 1) ? 206 : 348;
        renderText(renderer, font, "SELECTED", x, 266, SDL_Color{255, 232, 0, 255});
    }
    renderText(renderer, font, "<- -> SELECT | ENTER CONFIRM", 74, 548, SDL_Color{180, 180, 180, 255});
}

} // namespace galaxy
