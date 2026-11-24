#include "components.hpp"
#include "game_parameters.hpp"
#include "renderer.hpp"
#include "level_system.hpp"
#include "scenes.hpp"
#include <cmath>

using ls = LevelSystem;
using param = Parameters;

ShapeComponent::ShapeComponent(Entity *const p) : Component(p) {}

void ShapeComponent::update(const float &dt) {
    if (_shape) {
        _shape->setPosition(_parent->get_position());
        _shape->setRotation(_parent->get_rotation());
    }
}

void ShapeComponent::render() {
    if (_shape && _parent->is_visible()) {
        _shape->setPosition(_parent->get_position());
        _shape->setRotation(_parent->get_rotation());
        Renderer::queue(_shape.get());
    }
}

sf::Shape &ShapeComponent::get_shape() const {
    return *_shape;
}

SpriteComponent::SpriteComponent(Entity *const p) : Component(p) {}

void SpriteComponent::set_texture(const sf::Texture &texture, float target_size) {
    _sprite.setTexture(texture, true);
    const sf::Vector2u sz = texture.getSize();
    if (sz.x > 0 && sz.y > 0) {
        _sprite.setOrigin(sz.x * 0.5f, sz.y * 0.5f);
        const float max_dim = static_cast<float>(std::max(sz.x, sz.y));
        const float scale = target_size / max_dim;
        _base_scale = {scale, scale};
        _sprite.setScale(_flip_x ? -_base_scale.x : _base_scale.x, _base_scale.y);
    }
}

void SpriteComponent::set_flip_x(bool flip) {
    _flip_x = flip;
    _sprite.setScale(_flip_x ? -_base_scale.x : _base_scale.x, _base_scale.y);
}

sf::Sprite &SpriteComponent::get_sprite() {
    return _sprite;
}

void SpriteComponent::update(const float &dt) {
    _sprite.setPosition(_parent->get_position());
    _sprite.setRotation(_parent->get_rotation());
}

void SpriteComponent::render() {
    if (_parent->is_visible()) {
        _sprite.setPosition(_parent->get_position());
        _sprite.setRotation(_parent->get_rotation());
        Renderer::queue(&_sprite);
    }
}

ActorMovementComponent::ActorMovementComponent(Entity *p, uint64_t category_bits, uint64_t mask_bits)
    : Component(p), _speed(100.0f) {
    b2BodyDef body_def = b2DefaultBodyDef();
    body_def.type = b2_dynamicBody;
    body_def.position = Physics::sv2_to_bv2(_parent->get_position());
    body_def.fixedRotation = true;
    body_def.linearDamping = param::actor_linear_damping;
    body_def.enableSleep = false;
    _body_id = b2CreateBody(Physics::get_world_id(), &body_def);

    b2Circle circle = {};
    circle.center = {0.0f, 0.0f};
    circle.radius = Physics::sv2_to_bv2({param::actor_collider_radius, 0.0f}).x;

    b2ShapeDef shape_def = b2DefaultShapeDef();
    shape_def.density = 1.0f;
    shape_def.material.friction = 0.0f;
    shape_def.material.restitution = 0.15f;
    shape_def.filter.categoryBits = category_bits;
    shape_def.filter.maskBits = mask_bits;
    _shape_id = b2CreateCircleShape(_body_id, &shape_def, &circle);
}

ActorMovementComponent::~ActorMovementComponent() {
    if (b2Body_IsValid(_body_id)) {
        b2DestroyBody(_body_id);
    }
    _shape_id = b2_nullShapeId;
    _body_id = b2_nullBodyId;
}

bool ActorMovementComponent::_has_wall_ahead(const sf::Vector2f &dir, float probe_dist) const {
    return Physics::cast_ray(_parent->get_position(), dir * probe_dist, CAT_WALL).hit;
}

void ActorMovementComponent::steer_velocity(const sf::Vector2f &desired_velocity, float dt) {
    if (!b2Body_IsValid(_body_id)) return;
    const sf::Vector2f cur_vel = get_velocity();
    const float blend = std::min(1.0f, param::actor_accel_rate * dt);
    const sf::Vector2f next_vel = cur_vel + (desired_velocity - cur_vel) * blend;
    b2Body_SetLinearVelocity(_body_id, Physics::sv2_to_bv2(next_vel));
}

void ActorMovementComponent::apply_impulse(const sf::Vector2f &velocity_impulse) {
    if (!b2Body_IsValid(_body_id)) return;
    // Scale desired delta-v by body mass because Box2D expects linear momentum (kg * m/s)
    const float mass = b2Body_GetMass(_body_id);
    const b2Vec2 dv = Physics::sv2_to_bv2(velocity_impulse);
    b2Body_ApplyLinearImpulseToCenter(_body_id, {dv.x * mass, dv.y * mass}, true);
}

sf::Vector2f ActorMovementComponent::get_velocity() const {
    if (!b2Body_IsValid(_body_id)) return {0.0f, 0.0f};
    return Physics::bv2_to_sv2(b2Body_GetLinearVelocity(_body_id));
}

void ActorMovementComponent::teleport(const sf::Vector2f &pos) {
    _parent->set_position(pos);
    if (b2Body_IsValid(_body_id)) {
        b2Body_SetTransform(_body_id, Physics::sv2_to_bv2(pos), b2MakeRot(0.0f));
        b2Body_SetLinearVelocity(_body_id, {0.0f, 0.0f});
    }
}

void ActorMovementComponent::sync_from_physics() {
    if (b2Body_IsValid(_body_id)) {
        _parent->set_position(Physics::bv2_to_sv2(b2Body_GetPosition(_body_id)));
    }
}

float ActorMovementComponent::get_speed() const {
    return _speed;
}

void ActorMovementComponent::set_speed(float speed) {
    _speed = speed;
}

void ActorMovementComponent::update(const float &dt) {
    sync_from_physics();
}

PlayerMovementComponent::PlayerMovementComponent(Entity *p)
    : ActorMovementComponent(p, CAT_PLAYER, CAT_WALL | CAT_GUARD),
      _stamina(param::sneak_max_duration),
      _max_stamina(param::sneak_max_duration),
      _exhausted(false) {
    _visor.setRadius(3.5f);
    _visor.setOrigin(3.5f, 3.5f);
    _visor.setFillColor(sf::Color(255, 235, 100));

    _stealth_ring.setRadius(param::entity_size + 4.0f);
    _stealth_ring.setOrigin(param::entity_size + 4.0f, param::entity_size + 4.0f);
    _stealth_ring.setFillColor(sf::Color::Transparent);
    _stealth_ring.setOutlineColor(sf::Color(60, 240, 140, 130));
    _stealth_ring.setOutlineThickness(1.5f);
}

void PlayerMovementComponent::update(const float &dt) {
    sync_from_physics();

    bool want_sneak = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ||
                      sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);

    // Lockout after full stamina drain until recharged to 25% (1.0s)
    if (_exhausted) {
        if (_stamina >= 1.0f) {
            _exhausted = false;
        }
    }

    if (want_sneak && !_exhausted && _stamina > 0.0f) {
        _sneaking = true;
        _stamina -= dt;
        if (_stamina <= 0.0f) {
            _stamina = 0.0f;
            _sneaking = false;
            _exhausted = true;
        }
    } else {
        _sneaking = false;
        if (_stamina < _max_stamina) {
            _stamina = std::min(_max_stamina, _stamina + dt * param::sneak_recharge_rate);
        }
    }

    _speed = _sneaking ? param::player_sneak_speed : param::player_speed;

    sf::Vector2f dir(0.f, 0.f);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        dir.y -= 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        dir.y += 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        dir.x -= 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        dir.x += 1.0f;

    float len = sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len > 0.1f) {
        dir /= len;
        _facing = dir;
        steer_velocity(dir * _speed, dt);
        _making_noise = !_sneaking;
    } else {
        steer_velocity({0.0f, 0.0f}, dt);
        _making_noise = false;
    }

    sf::Vector2f p_pos = _parent->get_position();
    _visor.setPosition(p_pos + _facing * (param::entity_size + 1.0f));
    _stealth_ring.setPosition(p_pos);
}

void PlayerMovementComponent::render() {
    if (_sneaking) {
        Renderer::queue(&_stealth_ring);
    }
}

GuardComponent::GuardComponent(Entity *p)
    : ActorMovementComponent(p, CAT_GUARD, CAT_WALL | CAT_PLAYER) {
    _speed = param::guard_patrol_speed;
    _vision_cone.setPrimitiveType(sf::TriangleFan);
    _vision_cone.resize(param::guard_vision_rays + 1);
}

void GuardComponent::set_patrol_direction(sf::Vector2f dir) {
    float len = sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len > 0.001f) {
        _direction = dir / len;
        _patrol_base_dir = _direction;
    }
}

bool GuardComponent::_can_see_player(const sf::Vector2f &player_pos, bool player_sneaking) {
    sf::Vector2f g_pos = _parent->get_position();
    sf::Vector2f diff = player_pos - g_pos;
    float dist = sqrt(diff.x * diff.x + diff.y * diff.y);

    float max_range = player_sneaking 
        ? (param::guard_vision_range * param::sneak_vision_reduction) 
        : param::guard_vision_range;

    if (dist > max_range || dist < 0.001f) return false;

    // Sneaking narrows the effective detection cone (0.88 dot ~= 28 deg half-angle)
    const float half_angle_rad = param::guard_fov_half_angle_deg * param::pi / 180.0f;
    float min_dot = player_sneaking ? 0.88f : std::cos(half_angle_rad);
    float dot = (diff.x * _direction.x + diff.y * _direction.y) / dist;
    if (dot < min_dot) return false;

    return !Physics::cast_ray(g_pos, diff).hit;
}

void GuardComponent::_update_vision_cone() {
    const sf::Vector2f g_pos = _parent->get_position();
    const float range = param::guard_vision_range;
    const float half_angle = param::guard_fov_half_angle_deg * param::pi / 180.0f;
    const float base_angle = std::atan2(_direction.y, _direction.x);

    sf::Color colour;
    if (_seeing_player) {
        colour = sf::Color(255, 40, 40, 110);
    } else if (_state == CHASE) {
        colour = sf::Color(255, 140, 20, 85);
    } else {
        colour = sf::Color(255, 215, 50, 50);
    }
    sf::Color edge_colour = colour;
    edge_colour.a = static_cast<sf::Uint8>(colour.a / 3);

    // TriangleFan layout: vertex 0 is the guard, vertices 1..N are ray-clipped perimeter points
    _vision_cone[0] = sf::Vertex(g_pos, colour);

    const int rays = param::guard_vision_rays;
    for (int i = 0; i < rays; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(rays - 1);
        const float angle = base_angle - half_angle + (2.0f * half_angle * t);
        const sf::Vector2f ray_dir(std::cos(angle), std::sin(angle));

        const RayHit hit = Physics::cast_ray(g_pos, ray_dir * range);
        const sf::Vector2f end_point = hit.hit ? hit.point : (g_pos + ray_dir * range);
        _vision_cone[i + 1] = sf::Vertex(end_point, edge_colour);
    }
}

void GuardComponent::update(const float &dt) {
    sync_from_physics();

    auto game = std::dynamic_pointer_cast<GameScene>(Scenes::game);
    _seeing_player = false;

    if (game && game->get_player()) {
        sf::Vector2f p_pos = game->get_player()->get_position();

        bool player_sneaking = false;
        bool player_noisy = false;
        auto pmc = game->get_player()->get_compatible_components<PlayerMovementComponent>();
        if (!pmc.empty()) {
            player_sneaking = pmc[0]->is_sneaking();
            player_noisy = pmc[0]->is_making_noise();
        }

        if (_can_see_player(p_pos, player_sneaking)) {
            _seeing_player = true;
            _state = CHASE;
            _last_seen_pos = p_pos;
            _search_timer = 1.2f;
        } else if (player_noisy) {
            sf::Vector2f diff = p_pos - _parent->get_position();
            float dist_sq = diff.x * diff.x + diff.y * diff.y;
            if (dist_sq <= param::player_noise_radius * param::player_noise_radius) {
                _state = CHASE;
                _last_seen_pos = p_pos;
                _search_timer = 1.0f;
            }
        }
    }

    if (_state == CHASE) {
        _speed = param::guard_chase_speed;

        if (_seeing_player) {
            if (game && game->get_player()) {
                sf::Vector2f diff = game->get_player()->get_position() - _parent->get_position();
                float len = sqrt(diff.x * diff.x + diff.y * diff.y);
                if (len > 5.0f) {
                    sf::Vector2f target_dir = diff / len;
                    // Gradual turn rate gives the player a window to break line-of-sight around corners
                    float turn_speed = 3.5f * dt;
                    _direction.x += (target_dir.x - _direction.x) * turn_speed;
                    _direction.y += (target_dir.y - _direction.y) * turn_speed;
                    float d_len = sqrt(_direction.x * _direction.x + _direction.y * _direction.y);
                    if (d_len > 0.001f) {
                        _direction /= d_len;
                    }
                    steer_velocity(_direction * _speed, dt);
                } else {
                    steer_velocity({0.0f, 0.0f}, dt);
                }
            }
        } else {
            _search_timer -= dt;
            sf::Vector2f diff = _last_seen_pos - _parent->get_position();
            float len = sqrt(diff.x * diff.x + diff.y * diff.y);

            if (len > 15.0f) {
                sf::Vector2f move_dir = diff / len;
                float turn_speed = 3.5f * dt;
                _direction.x += (move_dir.x - _direction.x) * turn_speed;
                _direction.y += (move_dir.y - _direction.y) * turn_speed;
                float d_len = sqrt(_direction.x * _direction.x + _direction.y * _direction.y);
                if (d_len > 0.001f) {
                    _direction /= d_len;
                }

                if (_has_wall_ahead(_direction, param::actor_collider_radius + 4.0f)) {
                    _search_timer = 0.0f;
                    steer_velocity({0.0f, 0.0f}, dt);
                } else {
                    steer_velocity(_direction * _speed, dt);
                }
            } else {
                steer_velocity({0.0f, 0.0f}, dt);
            }

            if (_search_timer <= 0.0f || len <= 15.0f) {
                _state = PATROL;
                _direction = _patrol_base_dir;
            }
        }
    } else {
        _speed = param::guard_patrol_speed;
        const float probe_dist = param::actor_collider_radius + 3.0f;
        if (_has_wall_ahead(_direction, probe_dist)) {
            _direction = -_direction;
            _patrol_base_dir = _direction;
        }
        steer_velocity(_direction * _speed, dt);
    }

    _update_vision_cone();
}

void GuardComponent::render() {
    Renderer::queue(&_vision_cone);
}

TerminalComponent::TerminalComponent(Entity *p, const sf::Texture *normal_tex, const sf::Texture *hacked_tex)
    : Component(p), _normal_tex(normal_tex), _hacked_tex(hacked_tex) {
    if (_normal_tex) {
        _apply_texture(*_normal_tex);
    }

    _bar_bg.setSize(sf::Vector2f(32.0f, 5.0f));
    _bar_bg.setFillColor(sf::Color(15, 15, 20));
    _bar_bg.setOutlineColor(sf::Color(180, 180, 190));
    _bar_bg.setOutlineThickness(1.0f);

    _bar.setSize(sf::Vector2f(0.f, 5.f));
    _bar.setFillColor(sf::Color(255, 185, 40));
}

void TerminalComponent::_apply_texture(const sf::Texture &tex) {
    _sprite.setTexture(tex, true);
    const sf::Vector2u sz = tex.getSize();
    if (sz.x > 0 && sz.y > 0) {
        _sprite.setOrigin(sz.x * 0.5f, sz.y * 0.5f);
        const float max_dim = static_cast<float>(std::max(sz.x, sz.y));
        const float scale = 26.0f / max_dim;
        _sprite.setScale(scale, scale);
    }
}

void TerminalComponent::add_progress(float delta) {
    if (_hacked) return;
    _progress += delta;
    if (_progress >= 1.0f) {
        _progress = 1.0f;
        _hacked = true;
        if (_hacked_tex) {
            _apply_texture(*_hacked_tex);
        }
    }
}

void TerminalComponent::update(const float &dt) {
    sf::Vector2f p = _parent->get_position();
    _sprite.setPosition(p);

    if (!_hacked && _progress > 0.0f) {
        _bar_bg.setPosition(p.x - 16.0f, p.y - 20.0f);
        _bar.setSize(sf::Vector2f(32.0f * _progress, 5.0f));
        _bar.setPosition(p.x - 16.0f, p.y - 20.0f);
    }
}

void TerminalComponent::render() {
    _sprite.setPosition(_parent->get_position());
    Renderer::queue(&_sprite);
    if (!_hacked && _progress > 0.0f) {
        Renderer::queue(&_bar_bg);
        Renderer::queue(&_bar);
    }
}


WallColliderComponent::WallColliderComponent(Entity *const p) : Component(p) {
    b2BodyDef body_def = b2DefaultBodyDef();
    body_def.type = b2_staticBody;
    _body_id = b2CreateBody(Physics::get_world_id(), &body_def);

    // Merge contiguous horizontal WALL runs into single Box2D boxes to reduce shape count
    for (int y = 0; y < ls::get_height(); ++y) {
        int x = 0;
        while (x < ls::get_width()) {
            if (ls::get_tile({x, y}) != ls::WALL) {
                ++x;
                continue;
            }
            int run_length = 0;
            while (x + run_length < ls::get_width() &&
                   ls::get_tile({x + run_length, y}) == ls::WALL) {
                ++run_length;
            }
            _create_wall_box(x, y, run_length);
            x += run_length;
        }
    }
}

void WallColliderComponent::_create_wall_box(int start_x, int y, int run_length) {
    const sf::Vector2f size(param::tile_size * static_cast<float>(run_length), param::tile_size);
    const sf::Vector2f centre = ls::get_tile_position({start_x, y}) + size * 0.5f;

    const b2Vec2 half_extents = Physics::sv2_to_bv2(size * 0.5f);
    b2Polygon box = b2MakeOffsetBox(half_extents.x, half_extents.y,
                                    Physics::sv2_to_bv2(centre), b2MakeRot(0.0f));
    b2ShapeDef shape_def = b2DefaultShapeDef();
    shape_def.material.friction = 0.0f;
    shape_def.filter.categoryBits = CAT_WALL;
    b2CreatePolygonShape(_body_id, &shape_def, &box);
}

WallColliderComponent::~WallColliderComponent() {
    if (b2Body_IsValid(_body_id)) {
        b2DestroyBody(_body_id);
    }
    _body_id = b2_nullBodyId;
}
