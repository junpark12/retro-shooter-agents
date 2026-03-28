#pragma once
#include "types.h"

namespace galaxy {

struct BulletPool; // defined in bullet.h

// ─── BulletPatternGenerator ───────────────────────────────────────────────────
// Stateless utility functions for firing standard Bullet Hell patterns.
// All functions emit bullets into the provided BulletPool.
//
// Parameters:
//   bp       – target bullet pool
//   origin   – world-space spawn position (center of the enemy/boss)
//   toward   – target world position (player pos) for aimed patterns
//   baseAngle – starting angle offset in radians (for rotating patterns)
//   speed    – bullet speed in pixels/second
//   damage   – bullet damage value
//   owner    – BulletOwner::ENEMY for enemy patterns

// Single straight-down shot.
void patternSingle(BulletPool& bp, Vec2 origin, Vec2 toward,
                   float baseAngle, float speed, int damage, BulletOwner owner);

// 3-way spread.
void patternSpread3(BulletPool& bp, Vec2 origin, Vec2 toward,
                    float baseAngle, float speed, int damage, BulletOwner owner);

// 5-way spread.
void patternSpread5(BulletPool& bp, Vec2 origin, Vec2 toward,
                    float baseAngle, float speed, int damage, BulletOwner owner);

// 8-directional ring.
void patternCircle8(BulletPool& bp, Vec2 origin, Vec2 toward,
                    float baseAngle, float speed, int damage, BulletOwner owner);

// 16-directional dense ring.
void patternCircle16(BulletPool& bp, Vec2 origin, Vec2 toward,
                     float baseAngle, float speed, int damage, BulletOwner owner);

// Clockwise spiral (fires a subset of a ring rotated by baseAngle each call).
void patternSpiralCW(BulletPool& bp, Vec2 origin, Vec2 toward,
                     float baseAngle, float speed, int damage, BulletOwner owner);

// Counter-clockwise spiral.
void patternSpiralCCW(BulletPool& bp, Vec2 origin, Vec2 toward,
                      float baseAngle, float speed, int damage, BulletOwner owner);

// Aimed directly at player position.
void patternAimed(BulletPool& bp, Vec2 origin, Vec2 toward,
                  float baseAngle, float speed, int damage, BulletOwner owner);

// 3-way aimed spread.
void patternAimedSpread(BulletPool& bp, Vec2 origin, Vec2 toward,
                        float baseAngle, float speed, int damage, BulletOwner owner);

// Dense horizontal curtain (wide spread across the screen).
void patternCurtain(BulletPool& bp, Vec2 origin, Vec2 toward,
                    float baseAngle, float speed, int damage, BulletOwner owner);

// Random spread burst.
void patternRandomSpread(BulletPool& bp, Vec2 origin, Vec2 toward,
                         float baseAngle, float speed, int damage, BulletOwner owner);

// Fire homing bullets.
void patternHoming(BulletPool& bp, Vec2 origin, Vec2 toward,
                   float baseAngle, float speed, int damage, BulletOwner owner);

// Dispatch function — fires a pattern by enum value.
void firePattern(BulletPool& bp, BulletPattern pattern,
                 Vec2 origin, Vec2 toward,
                 float baseAngle, float speed, int damage, BulletOwner owner);

} // namespace galaxy
