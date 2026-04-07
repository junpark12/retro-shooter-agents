#pragma once
#include "types.h"
#include <SDL.h>

namespace galaxy {

// ─── Particle ─────────────────────────────────────────────────────────────────
// A single particle used for explosion effects.
struct Particle {
    Vec2  pos;          // world-space position
    Vec2  vel;          // velocity (pixels per second)
    float lifetime;     // total lifespan in seconds
    float age;          // elapsed time since spawn
    Uint8 r, g, b;      // current RGB (interpolated from yellow → orange → grey)
    float size;         // rendered size in pixels (shrinks over time)
    float initialSize;  // original size at spawn (used to compute shrink over lifetime)
    bool  active = false;
};

// ─── ParticleSystem ───────────────────────────────────────────────────────────
struct ParticleSystem {
    Particle pool[MAX_PARTICLES];
};

// Spawn an explosion burst centred at pos.
// big = true produces a larger, denser burst (for boss / large enemy death).
void spawnExplosion(ParticleSystem& ps, Vec2 center, bool big = false);

// Spawn a muzzle-flash burst at the player's weapon muzzle.
void spawnMuzzleFlash(ParticleSystem& ps, Vec2 muzzlePos, bool charged = false);

// Spawn a hit-spark burst when a bullet hits an enemy (small impact effect).
void spawnHitSpark(ParticleSystem& ps, Vec2 hitPos, Uint8 r = 255, Uint8 g = 200, Uint8 b = 50);

// Spawn a graze spark (small blue/white flicker) when a bullet barely misses the player.
void spawnGrazeSpark(ParticleSystem& ps, Vec2 pos);

// Advance all active particles by dt seconds.
void updateParticles(ParticleSystem& ps, float dt);

// Render all active particles with SDL_SetRenderDrawBlendMode + alpha.
void renderParticles(SDL_Renderer* renderer, const ParticleSystem& ps);

} // namespace galaxy
