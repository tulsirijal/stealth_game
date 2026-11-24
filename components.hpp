#pragma once
#include "ecm.hpp"
#include "physics.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

class ShapeComponent : public Component {
public:
    ShapeComponent() = delete;
    explicit ShapeComponent(Entity *const p);

    void update(const float &dt) override;
    void render() override;

    sf::Shape &get_shape() const;

    template <typename T, typename... Targs>
    void set_shape(Targs... params) {
        _shape.reset(new T(params...));
    }
protected:
    std::shared_ptr<sf::Shape> _shape;
};

class SpriteComponent : public Component {
public:
    SpriteComponent() = delete;
    explicit SpriteComponent(Entity *const p);

    void update(const float &dt) override;
    void render() override;

    void set_texture(const sf::Texture &texture, float target_size);
    void set_flip_x(bool flip);
    sf::Sprite &get_sprite();

protected:
    sf::Sprite _sprite;
    sf::Vector2f _base_scale{1.0f, 1.0f};
    bool _flip_x = false;
};

class ActorMovementComponent : public Component {
public:
    explicit ActorMovementComponent(Entity *p,
                                    uint64_t category_bits = CAT_PLAYER,
                                    uint64_t mask_bits = CAT_WALL);
    ActorMovementComponent() = delete;
    ~ActorMovementComponent() override;

    float get_speed() const;
    void set_speed(float speed);

    // Blends velocity toward target so external impulses (knockback) decay naturally instead of being overwritten.
    void steer_velocity(const sf::Vector2f &desired_velocity, float dt);
    void apply_impulse(const sf::Vector2f &velocity_impulse);

    sf::Vector2f get_velocity() const;
    void teleport(const sf::Vector2f &pos);
    void sync_from_physics();

    void render() override {}
    void update(const float &dt) override;

protected:
    bool _has_wall_ahead(const sf::Vector2f &dir, float probe_dist) const;
    float _speed = 100.0f;
    b2BodyId _body_id = b2_nullBodyId;
    b2ShapeId _shape_id = b2_nullShapeId;
};

class PlayerMovementComponent : public ActorMovementComponent {
public:
    PlayerMovementComponent() = delete;
    explicit PlayerMovementComponent(Entity *p);

    void update(const float &dt) override;
    void render() override;
    bool is_sneaking() const { return _sneaking; }
    bool is_making_noise() const { return _making_noise; }
    sf::Vector2f get_facing() const { return _facing; }
    float get_stamina() const { return _stamina; }
    float get_max_stamina() const { return _max_stamina; }
    bool is_exhausted() const { return _exhausted; }

private:
    bool _sneaking = false;
    bool _making_noise = false;
    bool _exhausted = false;
    float _stamina = 4.0f;
    float _max_stamina = 4.0f;
    sf::Vector2f _facing{1.0f, 0.0f};
    sf::CircleShape _visor;
    sf::CircleShape _stealth_ring;
};

class GuardComponent : public ActorMovementComponent {
public:
    enum State { PATROL, CHASE };

    GuardComponent() = delete;
    explicit GuardComponent(Entity *p);

    void update(const float &dt) override;
    void render() override;

    State get_state() const { return _state; }
    bool is_seeing_player() const { return _seeing_player; }
    sf::Vector2f get_direction() const { return _direction; }
    void set_patrol_direction(sf::Vector2f dir);

private:
    State _state = PATROL;
    bool _seeing_player = false;
    sf::Vector2f _direction{1.0f, 0.0f};
    sf::Vector2f _patrol_base_dir{1.0f, 0.0f};
    sf::Vector2f _last_seen_pos{0.0f, 0.0f};
    sf::VertexArray _vision_cone;
    float _search_timer = 0.0f;

    bool _can_see_player(const sf::Vector2f &player_pos, bool player_sneaking);
    void _update_vision_cone();
};

class TerminalComponent : public Component {
public:
    TerminalComponent() = delete;
    TerminalComponent(Entity *p, const sf::Texture *normal_tex, const sf::Texture *hacked_tex);

    void update(const float &dt) override;
    void render() override;

    bool is_hacked() const { return _hacked; }
    float get_progress() const { return _progress; }
    void add_progress(float delta);

private:
    void _apply_texture(const sf::Texture &tex);

    bool _hacked = false;
    float _progress = 0.0f;
    const sf::Texture *_normal_tex = nullptr;
    const sf::Texture *_hacked_tex = nullptr;
    sf::Sprite _sprite;
    sf::RectangleShape _bar_bg;
    sf::RectangleShape _bar;
};


// Merges horizontal runs of WALL tiles into static Box2D boxes for collision and raycasts.
class WallColliderComponent : public Component {
public:
    WallColliderComponent() = delete;
    explicit WallColliderComponent(Entity *const p);
    ~WallColliderComponent() override;

    void update(const float &dt) override {}
    void render() override {}

private:
    b2BodyId _body_id = b2_nullBodyId;
    void _create_wall_box(int start_x, int y, int run_length);
};
