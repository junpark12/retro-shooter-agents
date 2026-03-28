#pragma once
#include "types.h"
#include "asset_manager.h"
#include <SDL.h>

namespace galaxy {

// ─── Background (parallax starfield + nebula) ─────────────────────────────────

struct Star {
    Vec2  pos;
    float speed;       // pixels per second (layer-dependent)
    Uint8 brightness;  // 60–255
    int   layer;       // 0 = slow/far, 1 = mid, 2 = fast/near
    int   colorVariant; // 0 = white, 1 = cyan, 2 = blue, 3 = yellow
};

struct Background {
    Star  stars[MAX_STARS];
    float nebulaScrollY = 0.0f;  // vertical scroll offset for background texture
};

// Seed stars with random positions, speeds, and brightnesses across 3 layers.
void initBackground(Background& bg);

// Scroll all stars downward; wrap stars that leave the bottom back to the top.
// Also scrolls the nebula background texture.
void updateBackground(Background& bg, float dt);

// Render nebula background texture (if available) then render star layers.
void renderBackground(SDL_Renderer* renderer, const AssetManager& assets,
                      const Background& bg);

} // namespace galaxy
