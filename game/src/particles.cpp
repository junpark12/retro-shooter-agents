#include "particles.h"

#include <SDL.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace galaxy {

namespace {
float rand01() {
    return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
}

float randRange(float minV, float maxV) {
    return minV + (maxV - minV) * rand01();
}

Uint8 lerpU8(Uint8 a, Uint8 b, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    return static_cast<Uint8>(a + static_cast<int>((b - a) * t));
}
} // namespace

void spawnExplosion(ParticleSystem& ps, Vec2 center, bool big) {
    const int spawnCount = big ? 24 : 12;

    int spawned = 0;
    for (int i = 0; i < MAX_PARTICLES && spawned < spawnCount; ++i) {
        Particle& p = ps.pool[i];
        if (p.active) continue;

        const float angle = randRange(0.0f, 6.2831853f);
        const float speed = randRange(80.0f, 200.0f);
        const float size = big ? randRange(3.0f, 6.0f) : randRange(2.0f, 4.0f);

        p.active = true;
        p.pos = center;
        p.vel = {std::cos(angle) * speed, std::sin(angle) * speed};
        p.lifetime = randRange(0.5f, 1.0f);
        p.age = 0.0f;
        p.r = 255;
        p.g = static_cast<Uint8>(randRange(200.0f, 255.0f));
        p.b = 0;
        p.size = size;
        p.initialSize = size;
        spawned++;
    }
}

void updateParticles(ParticleSystem& ps, float dt) {
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        Particle& p = ps.pool[i];
        if (!p.active) continue;

        p.age += dt;
        if (p.age >= p.lifetime) {
            p.active = false;
            continue;
        }

        p.pos += p.vel * dt;
        p.vel.y += 40.0f * dt;

        const float lifeT = std::clamp(p.age / p.lifetime, 0.0f, 1.0f);

        if (lifeT < (1.0f / 3.0f)) {
            const float t = lifeT / (1.0f / 3.0f);
            p.r = lerpU8(255, 255, t);
            p.g = lerpU8(200, 100, t);
            p.b = lerpU8(0, 0, t);
        } else if (lifeT < (2.0f / 3.0f)) {
            const float t = (lifeT - (1.0f / 3.0f)) / (1.0f / 3.0f);
            p.r = lerpU8(255, 100, t);
            p.g = lerpU8(100, 100, t);
            p.b = lerpU8(0, 100, t);
        } else {
            const float t = (lifeT - (2.0f / 3.0f)) / (1.0f / 3.0f);
            p.r = lerpU8(100, 40, t);
            p.g = lerpU8(100, 40, t);
            p.b = lerpU8(100, 40, t);
        }

        p.size = p.initialSize * (1.0f - lifeT);
    }
}

void renderParticles(SDL_Renderer* renderer, const ParticleSystem& ps) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    for (const Particle& p : ps.pool) {
        if (!p.active) continue;

        const float lifeT = std::clamp(p.age / p.lifetime, 0.0f, 1.0f);
        const Uint8 alpha = static_cast<Uint8>(255.0f * (1.0f - lifeT));
        SDL_SetRenderDrawColor(renderer, p.r, p.g, p.b, alpha);

        if (p.size <= 1.0f) {
            SDL_RenderDrawPoint(renderer, static_cast<int>(p.pos.x), static_cast<int>(p.pos.y));
        } else {
            SDL_Rect rc{
                static_cast<int>(p.pos.x - p.size * 0.5f),
                static_cast<int>(p.pos.y - p.size * 0.5f),
                static_cast<int>(p.size),
                static_cast<int>(p.size)
            };
            SDL_RenderFillRect(renderer, &rc);
        }
    }
}

} // namespace galaxy
