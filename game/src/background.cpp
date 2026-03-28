#include "background.h"

#include <algorithm>
#include <cstdlib>

namespace galaxy {

namespace {
float rand01() { return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX); }
float randRange(float a, float b) { return a + (b - a) * rand01(); }
int randRangeInt(int a, int b) { return a + (std::rand() % (b - a + 1)); }
} // namespace

void initBackground(Background& bg) {
    bg.nebulaScrollY = 0.0f;
    for (int i = 0; i < MAX_STARS; ++i) {
        Star& s = bg.stars[i];
        s.layer = (i < 50) ? 0 : (i < 100 ? 1 : 2);
        s.pos = {randRange(0.0f, static_cast<float>(SCREEN_W - 1)),
                 randRange(0.0f, static_cast<float>(SCREEN_H - 1))};
        if (s.layer == 0) {
            s.speed = randRange(30.0f, 60.0f);
            s.brightness = static_cast<Uint8>(randRangeInt(70, 120));
        } else if (s.layer == 1) {
            s.speed = randRange(80.0f, 120.0f);
            s.brightness = static_cast<Uint8>(randRangeInt(130, 190));
        } else {
            s.speed = randRange(150.0f, 220.0f);
            s.brightness = static_cast<Uint8>(randRangeInt(210, 255));
        }
        s.colorVariant = randRangeInt(0, 3);
    }
}

void updateBackground(Background& bg, float dt) {
    bg.nebulaScrollY += 20.0f * dt;
    if (bg.nebulaScrollY >= SCREEN_H) bg.nebulaScrollY -= SCREEN_H;

    for (Star& s : bg.stars) {
        s.pos.y += s.speed * dt;
        if (s.pos.y > SCREEN_H) {
            s.pos.y = -5.0f;
            s.pos.x = randRange(0.0f, static_cast<float>(SCREEN_W - 1));
        }
    }
}

void renderBackground(SDL_Renderer* renderer, const AssetManager& assets, const Background& bg) {
    SDL_Texture* bgTex = assets.get(SPR_BG_SPACE);
    if (bgTex) {
        const int scroll = static_cast<int>(bg.nebulaScrollY) % SCREEN_H;
        SDL_Rect top{0, scroll - SCREEN_H, SCREEN_W, SCREEN_H};
        SDL_Rect bottom{0, scroll, SCREEN_W, SCREEN_H};
        SDL_RenderCopy(renderer, bgTex, nullptr, &top);
        SDL_RenderCopy(renderer, bgTex, nullptr, &bottom);
    } else {
        SDL_SetRenderDrawColor(renderer, 0, 0, 20, 255);
        SDL_Rect fill{0, 0, SCREEN_W, SCREEN_H};
        SDL_RenderFillRect(renderer, &fill);
    }

    for (const Star& s : bg.stars) {
        int size = (s.layer == 2) ? 2 : 1;
        Uint8 b = s.brightness;
        if (s.layer == 0) b = static_cast<Uint8>(b / 4);
        else if (s.layer == 1) b = static_cast<Uint8>(b / 2);

        Uint8 r = b, g = b, bl = b;
        if (s.colorVariant == 1) {
            r = 0;
            g = static_cast<Uint8>(std::max<int>(40, (200 * b) / 255));
            bl = static_cast<Uint8>(std::max<int>(40, (255 * b) / 255));
        } else if (s.colorVariant == 2) {
            r = static_cast<Uint8>((100 * b) / 255);
            g = static_cast<Uint8>((150 * b) / 255);
            bl = b;
        } else if (s.colorVariant == 3) {
            r = b;
            g = b;
            bl = static_cast<Uint8>((150 * b) / 255);
        }

        SDL_Rect px{static_cast<int>(s.pos.x), static_cast<int>(s.pos.y), size, size};
        SDL_SetRenderDrawColor(renderer, r, g, bl, 255);
        SDL_RenderFillRect(renderer, &px);
    }
}

} // namespace galaxy
