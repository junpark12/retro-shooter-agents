#include "bullet_pattern.h"

#include "bullet.h"

#include <cmath>
#include <cstdlib>
#include <initializer_list>

namespace galaxy {

namespace {
constexpr float PI = 3.14159265f;

Vec2 dirFromAngle(float rad) {
    return {std::cos(rad), std::sin(rad)};
}

float angleTo(Vec2 a, Vec2 b) {
    return std::atan2(b.y - a.y, b.x - a.x);
}
} // namespace

void patternSingle(BulletPool& bp, Vec2 origin, Vec2, float, float speed, int damage, BulletOwner owner) {
    fireBullet(bp, origin, {0.0f, speed}, owner, damage);
}

void patternSpread3(BulletPool& bp, Vec2 origin, Vec2, float baseAngle, float speed, int damage, BulletOwner owner) {
    for (float deg : {-20.0f, 0.0f, 20.0f}) {
        const Vec2 d = dirFromAngle(baseAngle + deg * PI / 180.0f);
        fireBullet(bp, origin, d * speed, owner, damage);
    }
}

void patternSpread5(BulletPool& bp, Vec2 origin, Vec2, float baseAngle, float speed, int damage, BulletOwner owner) {
    for (float deg : {-40.0f, -20.0f, 0.0f, 20.0f, 40.0f}) {
        const Vec2 d = dirFromAngle(baseAngle + deg * PI / 180.0f);
        fireBullet(bp, origin, d * speed, owner, damage);
    }
}

void patternCircle8(BulletPool& bp, Vec2 origin, Vec2, float baseAngle, float speed, int damage, BulletOwner owner) {
    for (int i = 0; i < 8; ++i) {
        const Vec2 d = dirFromAngle(baseAngle + i * (2.0f * PI / 8.0f));
        fireBullet(bp, origin, d * speed, owner, damage);
    }
}

void patternCircle16(BulletPool& bp, Vec2 origin, Vec2, float baseAngle, float speed, int damage, BulletOwner owner) {
    for (int i = 0; i < 16; ++i) {
        const Vec2 d = dirFromAngle(baseAngle + i * (2.0f * PI / 16.0f));
        fireBullet(bp, origin, d * speed, owner, damage);
    }
}

void patternSpiralCW(BulletPool& bp, Vec2 origin, Vec2, float baseAngle, float speed, int damage, BulletOwner owner) {
    for (int i = 0; i < 4; ++i) {
        const Vec2 d = dirFromAngle(baseAngle - i * (2.0f * PI / 16.0f));
        fireBullet(bp, origin, d * speed, owner, damage);
    }
}

void patternSpiralCCW(BulletPool& bp, Vec2 origin, Vec2, float baseAngle, float speed, int damage, BulletOwner owner) {
    for (int i = 0; i < 4; ++i) {
        const Vec2 d = dirFromAngle(baseAngle + i * (2.0f * PI / 16.0f));
        fireBullet(bp, origin, d * speed, owner, damage);
    }
}

void patternAimed(BulletPool& bp, Vec2 origin, Vec2 toward, float, float speed, int damage, BulletOwner owner) {
    const Vec2 d = (toward - origin).normalized();
    fireBullet(bp, origin, d * speed, owner, damage);
}

void patternAimedSpread(BulletPool& bp, Vec2 origin, Vec2 toward, float, float speed, int damage, BulletOwner owner) {
    const float base = angleTo(origin, toward);
    for (float deg : {-15.0f, 0.0f, 15.0f}) {
        const Vec2 d = dirFromAngle(base + deg * PI / 180.0f);
        fireBullet(bp, origin, d * speed, owner, damage);
    }
}

void patternCurtain(BulletPool& bp, Vec2 origin, Vec2, float, float speed, int damage, BulletOwner owner) {
    constexpr int count = 7;
    constexpr float width = 240.0f;
    for (int i = 0; i < count; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(count - 1);
        const float x = origin.x - width * 0.5f + width * t;
        fireBullet(bp, {x, origin.y}, {0.0f, speed}, owner, damage);
    }
}

void patternRandomSpread(BulletPool& bp, Vec2 origin, Vec2, float baseAngle, float speed, int damage, BulletOwner owner) {
    for (int i = 0; i < 8; ++i) {
        const float r = static_cast<float>(std::rand() % 121 - 60) * PI / 180.0f;
        const Vec2 d = dirFromAngle(baseAngle + r);
        fireBullet(bp, origin, d * speed, owner, damage);
    }
}

void patternHoming(BulletPool& bp, Vec2 origin, Vec2 toward, float baseAngle, float speed, int damage, BulletOwner owner) {
    for (int i = -2; i <= 3; ++i) {
        const float off = i * 8.0f * PI / 180.0f;
        const Vec2 d = dirFromAngle(baseAngle + off);
        fireHomingBullet(bp, origin, d * speed, owner, toward, damage);
    }
}

void firePattern(BulletPool& bp, BulletPattern pattern,
                 Vec2 origin, Vec2 toward,
                 float baseAngle, float speed, int damage, BulletOwner owner) {
    switch (pattern) {
        case BulletPattern::SINGLE:        patternSingle(bp, origin, toward, baseAngle, speed, damage, owner); break;
        case BulletPattern::SPREAD_3:      patternSpread3(bp, origin, toward, baseAngle, speed, damage, owner); break;
        case BulletPattern::SPREAD_5:      patternSpread5(bp, origin, toward, baseAngle, speed, damage, owner); break;
        case BulletPattern::CIRCLE_8:      patternCircle8(bp, origin, toward, baseAngle, speed, damage, owner); break;
        case BulletPattern::CIRCLE_16:     patternCircle16(bp, origin, toward, baseAngle, speed, damage, owner); break;
        case BulletPattern::SPIRAL_CW:     patternSpiralCW(bp, origin, toward, baseAngle, speed, damage, owner); break;
        case BulletPattern::SPIRAL_CCW:    patternSpiralCCW(bp, origin, toward, baseAngle, speed, damage, owner); break;
        case BulletPattern::AIMED:         patternAimed(bp, origin, toward, baseAngle, speed, damage, owner); break;
        case BulletPattern::AIMED_SPREAD:  patternAimedSpread(bp, origin, toward, baseAngle, speed, damage, owner); break;
        case BulletPattern::CURTAIN:       patternCurtain(bp, origin, toward, baseAngle, speed, damage, owner); break;
        case BulletPattern::RANDOM_SPREAD: patternRandomSpread(bp, origin, toward, baseAngle, speed, damage, owner); break;
        case BulletPattern::HOMING:        patternHoming(bp, origin, toward, baseAngle, speed, damage, owner); break;
    }
}

} // namespace galaxy
