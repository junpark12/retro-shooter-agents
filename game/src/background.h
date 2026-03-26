#pragma once
#include "types.h"
#include <SDL.h>

namespace galaxy {

// ─── Background (parallax starfield) ─────────────────────────────────────────

struct Star {
    Vec2  pos;
    float speed;       // pixels per second (layer-dependent)
    Uint8 brightness;  // 60–255
    int   layer;       // 0 = slow/far, 1 = mid, 2 = fast/near
};

struct Background {
    Star stars[MAX_STARS];
};

// Seed stars with random positions, speeds, and brightnesses across 3 layers.
void initBackground(Background& bg);

// Scroll all stars downward; wrap stars that leave the bottom back to the top.
void updateBackground(Background& bg, float dt);

// Render stars; brightness and size vary by layer:
//   layer 0 — 1×1 px, dim white
//   layer 1 — 1×1 px, brighter white or faint cyan
//   layer 2 — 2×2 px, bright white or neon-cyan flash
void renderBackground(SDL_Renderer* renderer, const Background& bg);

} // namespace galaxy
