#include "background.h"

#include <cstdlib>

namespace galaxy {

namespace {

float rand01() {
    return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
}

float randRange(float minV, float maxV) {
    return minV + (maxV - minV) * rand01();
}

int randRangeInt(int minV, int maxV) {
    return minV + (std::rand() % (maxV - minV + 1));
}

} // namespace

void initBackground(Background& bg) {
    for (int i = 0; i < MAX_STARS; ++i) {
        Star& s = bg.stars[i];

        // Weighted-ish layer distribution: far > mid > near
        if (i < 64) s.layer = 0;
        else if (i < 108) s.layer = 1;
        else s.layer = 2;

        s.pos.x = randRange(0.0f, static_cast<float>(SCREEN_W - 1));
        s.pos.y = randRange(0.0f, static_cast<float>(SCREEN_H - 1));

        if (s.layer == 0) {
            s.speed = randRange(30.0f, 50.0f);
            s.brightness = static_cast<Uint8>(randRangeInt(60, 120));
        } else if (s.layer == 1) {
            s.speed = randRange(60.0f, 90.0f);
            s.brightness = static_cast<Uint8>(randRangeInt(120, 190));
        } else {
            s.speed = randRange(100.0f, 150.0f);
            s.brightness = static_cast<Uint8>(randRangeInt(200, 255));
        }
    }
}

void updateBackground(Background& bg, float dt) {
    for (Star& s : bg.stars) {
        s.pos.y += s.speed * dt;
        if (s.pos.y > static_cast<float>(SCREEN_H)) {
            s.pos.y = -2.0f;
            s.pos.x = randRange(0.0f, static_cast<float>(SCREEN_W - 1));
        }
    }
}

void renderBackground(SDL_Renderer* renderer, const Background& bg) {
    for (const Star& s : bg.stars) {
        SDL_Rect px;
        px.x = static_cast<int>(s.pos.x);
        px.y = static_cast<int>(s.pos.y);

        if (s.layer == 0) {
            SDL_SetRenderDrawColor(renderer, s.brightness, s.brightness, s.brightness, 255);
            px.w = 1; px.h = 1;
        } else if (s.layer == 1) {
            const bool cyanTint = (s.brightness % 3 == 0);
            if (cyanTint) SDL_SetRenderDrawColor(renderer, 120, 220, 240, 255);
            else SDL_SetRenderDrawColor(renderer, s.brightness, s.brightness, s.brightness, 255);
            px.w = 1; px.h = 1;
        } else {
            const bool cyanFlash = (s.brightness % 2 == 0);
            if (cyanFlash) SDL_SetRenderDrawColor(renderer, COLOR_CYAN.r, COLOR_CYAN.g, COLOR_CYAN.b, 255);
            else SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            px.w = 2; px.h = 2;
        }

        SDL_RenderFillRect(renderer, &px);
    }
}

} // namespace galaxy
