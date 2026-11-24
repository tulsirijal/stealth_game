#pragma once
#include <memory>
#include <vector>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "game_system.hpp"

class MenuScene : public Scene {
private:
    sf::Text _title_text;
    sf::Text _prompt_text;
    sf::Font _font;

public:
    MenuScene() = default;
    void update(const float &dt) override;
    void render() override;
    void load() override;
};

class GameScene : public Scene {
private:
    sf::RectangleShape _hud_panel;
    sf::RectangleShape _status_panel;
    sf::Text _hud_text;
    sf::Text _status_text;

    sf::Text _hp_label;
    sf::Text _hp_val_text;
    sf::RectangleShape _health_bar_bg;
    sf::RectangleShape _health_bar;

    sf::Text _stamina_label;
    sf::Text _stamina_val_text;
    sf::RectangleShape _stamina_bar_bg;
    sf::RectangleShape _stamina_bar;

    sf::RectangleShape _debrief_box;
    sf::Text _debrief_title;
    sf::Text _debrief_body;

    sf::Font _font;
    std::shared_ptr<Entity> _player;
    std::vector<std::shared_ptr<Entity>> _guards;
    std::vector<std::shared_ptr<Entity>> _terminals;
    std::vector<std::shared_ptr<Entity>> _doors;

    sf::Texture _tex_hacker;
    sf::Texture _tex_guard;
    sf::Texture _tex_computer;
    sf::Texture _tex_hacked_computer;
    sf::Texture _tex_closed_door;
    sf::Texture _tex_open_door;
    bool _textures_loaded = false;
    int _total_terminals = 0;
    int _hacked_terminals = 0;
    float _player_health = 100.0f;
    float _hit_cooldown = 0.0f;
    bool _door_unlocked = false;
    bool _game_won = false;
    bool _game_lost = false;

    int _current_level = 1;
    const int _max_levels = 3;
    bool _level_cleared = false;

    sf::SoundBuffer _buf_hack;
    sf::SoundBuffer _buf_done;
    sf::SoundBuffer _buf_alert;
    sf::SoundBuffer _buf_hit;
    sf::SoundBuffer _buf_sneak;
    sf::SoundBuffer _buf_win;
    sf::SoundBuffer _buf_defeat;

    sf::Sound _snd_hack;
    sf::Sound _snd_done;
    sf::Sound _snd_alert;
    sf::Sound _snd_hit;
    sf::Sound _snd_sneak;
    sf::Sound _snd_win;
    sf::Sound _snd_defeat;

    float _hack_audio_timer = 0.0f;
    bool _was_exposed = false;
    bool _was_sneaking = false;
    bool _audio_loaded = false;

    void load_level(int level_num);
    void respawn();

    // Kept separate from _entities so respawn() can reset actors without rebuilding static level walls.
    std::shared_ptr<Entity> _walls;

public:
    GameScene() = default;
    void update(const float &dt) override;
    void render() override;
    void load() override;
    void unload() override;

    std::shared_ptr<Entity> get_player() const { return _player; }
};

struct Scenes {
    static std::shared_ptr<Scene> menu;
    static std::shared_ptr<Scene> game;
};
