#include "menu.h"

#include "hud.h"
#include "input.h"
#include "sprites.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

namespace galaxy {

namespace {
void renderMenuBackdrop(SDL_Renderer* renderer, const AssetManager& assets) {
    SDL_SetRenderDrawColor(renderer, 4, 8, 24, 255);
    SDL_Rect background{0, 0, SCREEN_W, SCREEN_H};
    SDL_RenderFillRect(renderer, &background);
    if (SDL_Texture* texture = assets.get(SPR_UI_MENU_BACKDROP)) {
        SDL_RenderCopy(renderer, texture, nullptr, &background);
    }
}
} // namespace

void updateMenu(Menu& m, float dt) {
    m.animTimer += dt;
    m.blinkTimer += dt;
    if (m.blinkTimer >= 0.5f) {
        m.blinkTimer = 0.0f;
        m.blinkVisible = !m.blinkVisible;
    }
}

GameState handleMenuEvent(Menu& m, const SDL_Event& e) {
    if (!isInitialKeyPress(e)) return GameState::TITLE;

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

void renderMenu(SDL_Renderer* renderer, const AssetManager& assets, TTF_Font* font, const Menu& m, int hiScore) {
    renderMenuBackdrop(renderer, assets);

    const int titleY = 120 + static_cast<int>(std::sin(m.animTimer * 2.0f) * 4.0f);
    renderTextCentered(renderer, font, "GALAXY STORM", titleY, SDL_Color{255, 232, 0, 255});
    renderTextCentered(renderer, font, "BULLET HELL SHOOTER", titleY + 34, SDL_Color{0, 255, 255, 255});

    SDL_Rect selection{120, 292 + m.selectedOption * 36, 240, 28};
    SDL_SetRenderDrawColor(renderer, 10, 28, 48, 255);
    SDL_RenderFillRect(renderer, &selection);
    SDL_SetRenderDrawColor(renderer, 55, 180, 210, 255);
    SDL_RenderDrawRect(renderer, &selection);

    const bool startSel = m.selectedOption == static_cast<int>(MenuOption::START);
    const bool highSel = m.selectedOption == static_cast<int>(MenuOption::HIGH_SCORE);
    const bool quitSel = m.selectedOption == static_cast<int>(MenuOption::QUIT);
    renderTextCentered(renderer, font, "START", 300, startSel ? SDL_Color{255, 232, 0, 255} : SDL_Color{255, 255, 255, 255});
    renderTextCentered(renderer, font, "HIGH SCORE", 336, highSel ? SDL_Color{255, 232, 0, 255} : SDL_Color{255, 255, 255, 255});
    renderTextCentered(renderer, font, "QUIT", 372, quitSel ? SDL_Color{255, 232, 0, 255} : SDL_Color{255, 255, 255, 255});

    if (m.blinkVisible) {
        const int cursorY = (m.selectedOption == static_cast<int>(MenuOption::START)) ? 300
                          : (m.selectedOption == static_cast<int>(MenuOption::HIGH_SCORE)) ? 336
                                                                                            : 372;
        renderText(renderer, font, ">", 136, cursorY, SDL_Color{255, 232, 0, 255});
    }

    std::ostringstream oss;
    oss << "HI-SCORE " << std::setw(6) << std::setfill('0') << std::max(0, hiScore);
    renderTextCentered(renderer, font, oss.str().c_str(), SCREEN_H - 46, SDL_Color{220, 220, 220, 255});
    renderTextCentered(renderer, font, "ENTER / SPACE: SELECT", 404, SDL_Color{220, 220, 220, 255});
    
    // Controls guide
    renderTextCentered(renderer, font, "- CONTROLS -", 430, SDL_Color{255, 220, 0, 255});
    renderText(renderer, font, "MOVE  : ARROWS / WASD", 90, 452, SDL_Color{160, 210, 255, 255});
    renderText(renderer, font, "FIRE  : SPACE / X", 90, 468, SDL_Color{160, 210, 255, 255});
    renderText(renderer, font, "LOCK  : Z", 90, 484, SDL_Color{160, 210, 255, 255});
    renderText(renderer, font, "BOMB  : C", 90, 500, SDL_Color{160, 210, 255, 255});
    renderText(renderer, font, "CHARGE: HOLD, THEN RELEASE", 90, 516, SDL_Color{160, 210, 255, 255});
    renderText(renderer, font, "PAUSE : P / ESC", 90, 532, SDL_Color{160, 210, 255, 255});
    renderTextCentered(renderer, font, "C 2026 GALAXY STORM TEAM", SCREEN_H - 24, SDL_Color{130, 130, 130, 255});
}

GameState handleHighScoreEvent(const SDL_Event& e) {
    if (!isInitialKeyPress(e)) return GameState::HIGH_SCORE;
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

    renderTextCentered(renderer, font, "HIGH SCORE", 80, SDL_Color{255, 232, 0, 255});

    std::ostringstream oss;
    oss << std::setw(8) << std::setfill('0') << std::max(0, hiScore);
    renderTextCentered(renderer, font, oss.str().c_str(), 200, SDL_Color{255, 255, 255, 255});

    renderTextCentered(renderer, font, "ENTER / ESC: RETURN", 450, SDL_Color{220, 220, 220, 255});
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
    if (!isInitialKeyPress(e)) return GameState::SHIP_SELECT;
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
    renderMenuBackdrop(renderer, assets);

    renderTextCentered(renderer, font, "SELECT YOUR SHIP", 72, SDL_Color{255, 232, 0, 255});

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
    renderTextCentered(renderer, font, names[i], 300, SDL_Color{255, 255, 255, 255});
    renderTextCentered(renderer, font, stats[i], 350, SDL_Color{0, 255, 255, 255});
    renderTextCentered(renderer, font, desc[i], 388, SDL_Color{220, 220, 220, 255});

    if (ss.blinkVisible) {
        const int x = (i == 0) ? 66 : (i == 1) ? 206 : 348;
        renderText(renderer, font, "SELECTED", x, 266, SDL_Color{255, 232, 0, 255});
    }
    renderTextCentered(renderer, font, "<- -> SELECT | ENTER CONFIRM", 548, SDL_Color{180, 180, 180, 255});
}

} // namespace galaxy
