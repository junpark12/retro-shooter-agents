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
    bool  active = false;
};

// ─── ParticleSystem ───────────────────────────────────────────────────────────
struct ParticleSystem {
    Particle pool[MAX_PARTICLES];
};

// Spawn an explosion burst centred at pos.
// big = true produces a larger, denser burst (for boss / large enemy death).
void spawnExplosion(ParticleSystem& ps, Vec2 center, bool big = false);

// Advance all active particles by dt seconds.
void updateParticles(ParticleSystem& ps, float dt);

// Render all active particles with SDL_SetRenderDrawBlendMode + alpha.
void renderParticles(SDL_Renderer* renderer, const ParticleSystem& ps);

} // namespace galaxy
