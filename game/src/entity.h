#pragma once
#include "types.h"

namespace galaxy {

// Base struct shared by Player, Enemy, Boss, Bullet, and PowerUp.
// All game objects are value types managed by their respective pools.
struct Entity {
    Vec2  pos;            // world position (top-left corner of sprite)
    Vec2  vel;            // velocity in pixels per second
    Rect  bounds;         // AABB for collision, relative to pos
    bool  active = false; // inactive entities are skipped by update/render
    int   hp     = 1;     // reaches 0 → mark inactive and trigger effects

    // Returns the world-space AABB (pos + bounds offset).
    Rect worldBounds() const {
        return { pos.x + bounds.x, pos.y + bounds.y, bounds.w, bounds.h };
    }
};

} // namespace galaxy
