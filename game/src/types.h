#pragma once
#include <SDL.h>

namespace galaxy {

// ─── Screen constants ─────────────────────────────────────────────────────────
constexpr int   SCREEN_W   = 480;
constexpr int   SCREEN_H   = 640;
constexpr int   FPS        = 60;
constexpr float FRAME_MS   = 1000.0f / FPS;

// ─── Pool sizes ───────────────────────────────────────────────────────────────
constexpr int MAX_BULLETS  = 256;
constexpr int MAX_ENEMIES  = 64;
constexpr int MAX_POWERUPS = 16;
constexpr int MAX_STARS    = 128;

// ─── 90s neon colour palette ─────────────────────────────────────────────────
struct Color { Uint8 r, g, b; };

constexpr Color COLOR_CYAN    = {  0, 255, 255 };
constexpr Color COLOR_MAGENTA = {255,   0, 255 };
constexpr Color COLOR_GREEN   = { 57, 255,  20 };
constexpr Color COLOR_YELLOW  = {255, 232,   0 };
constexpr Color COLOR_WHITE   = {255, 255, 255 };
constexpr Color COLOR_RED     = {255,  30,  30 };
constexpr Color COLOR_ORANGE  = {255, 140,   0 };

// ─── Game state ───────────────────────────────────────────────────────────────
enum class GameState {
    TITLE,
    PLAYING,
    STAGE_CLEAR,
    GAMEOVER,
    VICTORY
};

// ─── Entity type enumerations ─────────────────────────────────────────────────
enum class EnemyType {
    SMALL,   // straight-down movement, low HP, low score
    MEDIUM,  // sine-wave path, medium HP
    LARGE    // tracks player, high HP, drops power-up on death
};

enum class PowerUpType {
    SPREAD,   // 5-way spread shot
    LASER,    // piercing laser beam
    MISSILE,  // homing missiles
    SHIELD    // temporary invincibility bubble
};

enum class BulletOwner {
    PLAYER,
    ENEMY
};

// ─── Math primitives ──────────────────────────────────────────────────────────
struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2  operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2  operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2  operator*(float s)       const { return {x * s,   y * s};   }
    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
};

struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
};

// ─── AABB overlap test ────────────────────────────────────────────────────────
inline bool rectsOverlap(const Rect& a, const Rect& b) {
    return a.x < b.x + b.w &&
           a.x + a.w > b.x &&
           a.y < b.y + b.h &&
           a.y + a.h > b.y;
}

} // namespace galaxy
