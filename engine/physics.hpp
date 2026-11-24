#pragma once
#include <box2d/box2d.h>
#include <SFML/Graphics.hpp>
#include <cstdint>

// Raycasts default to CAT_WALL so actor circle colliders don't block guard line-of-sight.
enum PhysicsCategory : uint64_t {
    CAT_WALL   = 1ULL << 0,
    CAT_PLAYER = 1ULL << 1,
    CAT_GUARD  = 1ULL << 2,
    CAT_ALL    = 0xFFFFFFFFFFFFFFFFULL
};

struct RayHit {
    bool hit = false;
    sf::Vector2f point{0.f, 0.f};
    float fraction = 1.0f;
};

class Physics {
public:
    static void initialise();
    static void shutdown();
    static void update(const float &dt);
    static bool is_initialised();

    static b2WorldId get_world_id();

    static RayHit cast_ray(const sf::Vector2f &origin,
                           const sf::Vector2f &translation,
                           uint64_t mask_bits = CAT_WALL);

    static sf::Vector2f bv2_to_sv2(const b2Vec2 &in);
    static b2Vec2 sv2_to_bv2(const sf::Vector2f &in);

    static constexpr float physics_scale = 30.0f; // 30 pixels = 1 metre
    static constexpr float physics_scale_inv = 1.0f / physics_scale;
    static constexpr int sub_step_count = 4;
    static constexpr float gravity = 0.0f; // Top-down game no gravity

private:
    static b2WorldId _world_id;
};
