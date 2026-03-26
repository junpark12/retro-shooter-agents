#include "menu.h"

#include <array>
#include <cstdlib>
#include <string>
#include <unordered_map>

namespace galaxy {

namespace {

using Glyph = std::array<const char*, 7>;

const std::unordered_map<char, Glyph>& font5x7() {
    static const std::unordered_map<char, Glyph> kFont = {
        {' ', {"00000","00000","00000","00000","00000","00000","00000"}},
        {'!', {"00100","00100","00100","00100","00100","00000","00100"}},
        {'>', {"10000","01000","00100","00010","00100","01000","10000"}},
        {':', {"00000","00100","00100","00000","00100","00100","00000"}},
        {'1', {"00100","01100","00100","00100","00100","00100","01110"}},
        {'4', {"00010","00110","01010","10010","11111","00010","00010"}},
        {'9', {"01110","10001","10001","01111","00001","00010","11100"}},
        {'A', {"01110","10001","10001","11111","10001","10001","10001"}},
        {'B', {"11110","10001","10001","11110","10001","10001","11110"}},
        {'C', {"01110","10001","10000","10000","10000","10001","01110"}},
        {'E', {"11111","10000","10000","11110","10000","10000","11111"}},
        {'F', {"11111","10000","10000","11110","10000","10000","10000"}},
        {'G', {"01110","10001","10000","10111","10001","10001","01110"}},
        {'I', {"01110","00100","00100","00100","00100","00100","01110"}},
        {'L', {"10000","10000","10000","10000","10000","10000","11111"}},
        {'M', {"10001","11011","10101","10101","10001","10001","10001"}},
        {'O', {"01110","10001","10001","10001","10001","10001","01110"}},
        {'Q', {"01110","10001","10001","10001","10101","10010","01101"}},
        {'R', {"11110","10001","10001","11110","10100","10010","10001"}},
        {'S', {"01111","10000","10000","01110","00001","00001","11110"}},
        {'T', {"11111","00100","00100","00100","00100","00100","00100"}},
        {'U', {"10001","10001","10001","10001","10001","10001","01110"}},
        {'X', {"10001","01010","00100","00100","00100","01010","10001"}},
        {'Y', {"10001","01010","00100","00100","00100","00100","00100"}},
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

void renderText(SDL_Renderer* renderer, int x, int y, const std::string& txt, int scale, Color color) {
    int penX = x;
    for (char c : txt) {
        renderChar(renderer, penX, y, c, scale, color);
        penX += 6 * scale;
    }
}

} // namespace

void updateMenu(Menu& m, float dt) {
    m.blinkTimer += dt;
    while (m.blinkTimer >= 0.5f) {
        m.blinkTimer -= 0.5f;
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
            return GameState::PLAYING;
        }

        SDL_Quit();
        std::exit(0);
    }

    return GameState::TITLE;
}

void renderMenu(SDL_Renderer* renderer, const Menu& m) {
    setColor(renderer, {0, 0, 0});
    SDL_Rect bg{0, 0, SCREEN_W, SCREEN_H};
    SDL_RenderFillRect(renderer, &bg);

    // Decorative static stars
    setColor(renderer, COLOR_WHITE);
    const SDL_Point stars[] = {
        {24, 34}, {78, 96}, {130, 56}, {214, 88}, {302, 72}, {426, 44},
        {442, 118}, {60, 164}, {172, 138}, {260, 176}, {344, 154}, {410, 192}
    };
    for (const SDL_Point& p : stars) {
        SDL_Rect s{p.x, p.y, 2, 2};
        SDL_RenderFillRect(renderer, &s);
    }

    // Title
    const std::string title = "GALAXY STORM";
    const int titleScale = 3;
    const int titleW = static_cast<int>(title.size()) * 6 * titleScale;
    renderText(renderer, (SCREEN_W - titleW) / 2, SCREEN_H / 3 - 36, title, titleScale, COLOR_CYAN);

    // Options
    const int optScale = 2;
    const int optionY0 = SCREEN_H / 2 + 40;
    renderText(renderer, SCREEN_W / 2 - 35, optionY0, "START", optScale, COLOR_WHITE);
    renderText(renderer, SCREEN_W / 2 - 35, optionY0 + 40, "QUIT", optScale, COLOR_WHITE);

    if (m.blinkVisible) {
        const int cursorY = optionY0 + (m.selectedOption == static_cast<int>(MenuOption::START) ? 0 : 40);
        renderText(renderer, SCREEN_W / 2 - 55, cursorY, ">", optScale, COLOR_GREEN);
    }

    // Footer (© 1994 GALAXY SOFT) with primitive copyright mark
    const int fy = SCREEN_H - 26;
    const int fx = SCREEN_W / 2 - 67;
    setColor(renderer, COLOR_YELLOW);
    SDL_Rect c1{fx + 1, fy + 1, 3, 1};
    SDL_Rect c2{fx + 0, fy + 2, 1, 3};
    SDL_Rect c3{fx + 1, fy + 5, 3, 1};
    SDL_Rect ring1{fx + 0, fy + 0, 5, 1};
    SDL_Rect ring2{fx + 0, fy + 6, 5, 1};
    SDL_Rect ring3{fx - 1, fy + 1, 1, 5};
    SDL_Rect ring4{fx + 5, fy + 1, 1, 5};
    SDL_RenderFillRect(renderer, &c1);
    SDL_RenderFillRect(renderer, &c2);
    SDL_RenderFillRect(renderer, &c3);
    SDL_RenderFillRect(renderer, &ring1);
    SDL_RenderFillRect(renderer, &ring2);
    SDL_RenderFillRect(renderer, &ring3);
    SDL_RenderFillRect(renderer, &ring4);
    renderText(renderer, SCREEN_W / 2 - 57, fy, "1994 GALAXY SOFT", 1, COLOR_YELLOW);
}

} // namespace galaxy
