#include "physics.hpp"
#include <cmath>

b2WorldId Physics::_world_id = b2_nullWorldId;

void Physics::initialise() {
    if (is_initialised()) {
        return;
    }
    b2WorldDef world_def = b2DefaultWorldDef();
    world_def.gravity = b2Vec2{0.0f, gravity};
    _world_id = b2CreateWorld(&world_def);
}

void Physics::shutdown() {
    if (!is_initialised()) {
        return;
    }
    b2DestroyWorld(_world_id);
    _world_id = b2_nullWorldId;
}

void Physics::update(const float &dt) {
    if (is_initialised()) {
        b2World_Step(_world_id, dt, sub_step_count);
    }
}

bool Physics::is_initialised() {
    return b2World_IsValid(_world_id);
}

b2WorldId Physics::get_world_id() {
    return _world_id;
}

RayHit Physics::cast_ray(const sf::Vector2f &origin,
                         const sf::Vector2f &translation,
                         uint64_t mask_bits) {
    RayHit result;
    const float length_sq = translation.x * translation.x + translation.y * translation.y;
    if (!is_initialised() || length_sq < 1e-6f) {
        return result;
    }

    b2QueryFilter filter = b2DefaultQueryFilter();
    filter.maskBits = mask_bits;

    const b2RayResult ray = b2World_CastRayClosest(
        _world_id, sv2_to_bv2(origin), sv2_to_bv2(translation), filter);

    if (ray.hit) {
        result.hit = true;
        result.point = bv2_to_sv2(ray.point);
        result.fraction = ray.fraction;
    }
    return result;
}

sf::Vector2f Physics::bv2_to_sv2(const b2Vec2 &in) {
    return sf::Vector2f(in.x * physics_scale, in.y * physics_scale);
}

b2Vec2 Physics::sv2_to_bv2(const sf::Vector2f &in) {
    return b2Vec2{in.x * physics_scale_inv, in.y * physics_scale_inv};
}
