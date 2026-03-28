#pragma once
#include <SDL.h>
#include <cmath>

namespace galaxy {

// ─── Screen constants ─────────────────────────────────────────────────────────
constexpr int   SCREEN_W   = 480;
constexpr int   SCREEN_H   = 640;
constexpr int   FPS        = 60;
constexpr float FRAME_MS   = 1000.0f / FPS;

// ─── Pool sizes ───────────────────────────────────────────────────────────────
constexpr int MAX_BULLETS       = 500;   // Bullet Hell requires a large pool
constexpr int MAX_ENEMIES       = 64;
constexpr int MAX_POWERUPS      = 16;
constexpr int MAX_STARS         = 128;
constexpr int MAX_LOCK_TARGETS  = 8;     // max simultaneous lock-on targets

// ─── Score / life system ──────────────────────────────────────────────────────
constexpr int SCORE_PER_EXTRA_LIFE = 100000; // 1UP every 100k points

// ─── 90s neon colour palette ─────────────────────────────────────────────────
struct Color { Uint8 r, g, b; };

constexpr Color COLOR_CYAN    = {  0, 255, 255 };
constexpr Color COLOR_MAGENTA = {255,   0, 255 };
constexpr Color COLOR_GREEN   = { 57, 255,  20 };
constexpr Color COLOR_YELLOW  = {255, 232,   0 };
constexpr Color COLOR_WHITE   = {255, 255, 255 };
constexpr Color COLOR_RED     = {255,  30,  30 };
constexpr Color COLOR_ORANGE  = {255, 140,   0 };
constexpr Color COLOR_BLUE    = { 64, 128, 255 };

// ─── Game state ───────────────────────────────────────────────────────────────
enum class GameState {
    TITLE,
    SHIP_SELECT,  // ship selection screen
    PLAYING,
    STAGE_CLEAR,
    GAMEOVER,
    VICTORY,
    QUIT          // signals the main loop to exit cleanly via running_ = false
};

// ─── Ship types (3 selectable ships) ─────────────────────────────────────────
enum class ShipType {
    BAGON,   // Type A: focused fire, high firepower, cone lock-on → homing laser
    DAMUL,   // Type B: wide area, full-screen lock-on → spread beam
    GUNEX    // Type C: explosive, charge bomb + homing missiles
};

// ─── Entity type enumerations ─────────────────────────────────────────────────
enum class EnemyType {
    SMALL,      // straight-down, low HP, low score
    MEDIUM,     // sine-wave path, medium HP
    LARGE,      // tracks player, high HP, drops power-up on death
    FAST,       // diagonal movement, rapid fire
    ARMORED     // high HP, slow movement, spread pattern
};

enum class PowerUpType {
    SPREAD,    // 5-way spread shot
    LASER,     // piercing laser beam
    MISSILE,   // homing missiles
    SHIELD,    // temporary invincibility bubble
    BOMB,      // extra bomb stock
    POWER      // increase power level
};

enum class BulletOwner {
    PLAYER,
    ENEMY
};

// ─── Bullet patterns (Bullet Hell patterns) ───────────────────────────────────
enum class BulletPattern {
    SINGLE,        // single straight shot
    SPREAD_3,      // 3-way spread
    SPREAD_5,      // 5-way spread
    CIRCLE_8,      // 8-directional ring
    CIRCLE_16,     // 16-directional dense ring
    SPIRAL_CW,     // clockwise spiral
    SPIRAL_CCW,    // counter-clockwise spiral
    AIMED,         // aimed directly at player
    AIMED_SPREAD,  // aimed spread (3 aimed shots)
    CURTAIN,       // dense horizontal curtain
    RANDOM_SPREAD, // random spread burst
    HOMING         // homing shots
};

// ─── Math primitives ──────────────────────────────────────────────────────────
struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2  operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2  operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2  operator*(float s)       const { return {x * s,   y * s};   }
    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }

    float length() const { return std::sqrt(x * x + y * y); }
    Vec2  normalized() const {
        float len = length();
        if (len < 0.0001f) return {0.0f, -1.0f};
        return {x / len, y / len};
    }
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

// ─── Circle overlap test (for small hitbox bullets) ───────────────────────────
inline bool circlesOverlap(float ax, float ay, float ar, float bx, float by, float br) {
    float dx = ax - bx;
    float dy = ay - by;
    float dist2 = dx * dx + dy * dy;
    float radSum = ar + br;
    return dist2 < radSum * radSum;
}

} // namespace galaxy
