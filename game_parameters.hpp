#pragma once

struct Parameters {
    static constexpr int game_width = 800;
    static constexpr int game_height = 650;
    static constexpr float time_step = 0.017f;
    static constexpr float tile_size = 25.0f;
    static constexpr float entity_size = 10.0f;
    static constexpr float player_speed = 150.0f;
    static constexpr float player_sneak_speed = 75.0f;
    static constexpr float sneak_vision_reduction = 0.55f;
    static constexpr float sneak_max_duration = 4.0f;
    static constexpr float sneak_recharge_rate = 0.8f;
    static constexpr float guard_patrol_speed = 55.0f;
    static constexpr float guard_chase_speed = 85.0f;
    static constexpr float guard_vision_range = 95.0f;
    static constexpr float guard_fov_half_angle_deg = 45.57f;
    static constexpr int guard_vision_rays = 24;
    static constexpr float pi = 3.14159265f;

    static constexpr float actor_collider_radius = 8.5f;
    static constexpr float actor_linear_damping = 10.0f;
    static constexpr float actor_accel_rate = 14.0f;
    static constexpr float knockback_speed = 380.0f;
    static constexpr float player_noise_radius = 105.0f;
};
