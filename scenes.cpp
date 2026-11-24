#include "scenes.hpp"
#include "game_parameters.hpp"
#include "components.hpp"
#include "renderer.hpp"
#include "level_system.hpp"
#include <iostream>
#include <cmath>
#include <fstream>

using ls = LevelSystem;
using param = Parameters;
using gs = GameSystem;

static std::string res_path(const std::string &rel_path) {
    std::ifstream f(rel_path);
    if (f.good()) return rel_path;
    std::string fallback = "stealth_game/" + rel_path;
    std::ifstream f2(fallback);
    if (f2.good()) return fallback;
    return rel_path;
}

std::shared_ptr<Scene> Scenes::menu;
std::shared_ptr<Scene> Scenes::game;

void MenuScene::load() {
    _font.loadFromFile(res_path("resources/fonts/RobotoMono-Regular.ttf"));

    _title_text.setFont(_font);
    _title_text.setString("CYBER INFILTRATION");
    _title_text.setCharacterSize(42);
    _title_text.setFillColor(sf::Color(255, 195, 45));
    _title_text.setPosition(160.0f, 130.0f);

    _prompt_text.setFont(_font);
    _prompt_text.setString(
        "MISSION BRIEFING:\n"
        "1. Infiltrate 3 consecutive security sectors (Sector 1 -> 2 -> 3)\n"
        "2. Avoid Red Patrolling Guards and their vision cones\n"
        "3. Hold [Shift] to SNEAK: Halves guard sight range (uses stamina battery!)\n"
        "4. Guard hits & detection damage are PERMANENT: Health does not regenerate!\n"
        "5. Approach Amber Terminals and Hold [E] to breach security nodes\n"
        "6. Once all terminals are hacked, exfiltrate through the Green Exit!\n\n"
        ">> PRESS SPACE TO BEGIN CAMPAIGN <<\n\n"
        "Controls: WASD: Move | Shift: Sneak (Limited) | Hold E: Hack | Space: Select/Retry"
    );
    _prompt_text.setCharacterSize(15);
    _prompt_text.setFillColor(sf::Color(220, 230, 245));
    _prompt_text.setPosition(40.0f, 220.0f);
}

void MenuScene::update(const float &dt) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
        gs::set_active_scene(Scenes::game);
    }
}

void MenuScene::render() {
    Renderer::queue(&_title_text);
    Renderer::queue(&_prompt_text);
}

void GameScene::load() {
    _font.loadFromFile(res_path("resources/fonts/RobotoMono-Regular.ttf"));

    if (!_audio_loaded) {
        if (_buf_hack.loadFromFile(res_path("resources/audio/hack_tick.wav"))) _snd_hack.setBuffer(_buf_hack);
        if (_buf_done.loadFromFile(res_path("resources/audio/terminal_done.wav"))) _snd_done.setBuffer(_buf_done);
        if (_buf_alert.loadFromFile(res_path("resources/audio/alert.wav"))) _snd_alert.setBuffer(_buf_alert);
        if (_buf_hit.loadFromFile(res_path("resources/audio/impact.wav"))) _snd_hit.setBuffer(_buf_hit);
        if (_buf_sneak.loadFromFile(res_path("resources/audio/sneak_on.wav"))) _snd_sneak.setBuffer(_buf_sneak);
        if (_buf_win.loadFromFile(res_path("resources/audio/level_win.wav"))) _snd_win.setBuffer(_buf_win);
        if (_buf_defeat.loadFromFile(res_path("resources/audio/defeat.wav"))) _snd_defeat.setBuffer(_buf_defeat);
        _audio_loaded = true;
    }

    if (!_textures_loaded) {
        _tex_hacker.loadFromFile(res_path("resources/8bit/hacker.png"));
        _tex_guard.loadFromFile(res_path("resources/8bit/guard.png"));
        _tex_computer.loadFromFile(res_path("resources/8bit/computer.png"));
        _tex_hacked_computer.loadFromFile(res_path("resources/8bit/hacked_computer.png"));
        _tex_closed_door.loadFromFile(res_path("resources/8bit/closed_door.png"));
        _tex_open_door.loadFromFile(res_path("resources/8bit/open_door.png"));

        _tex_hacker.setSmooth(true);
        _tex_guard.setSmooth(true);
        _tex_computer.setSmooth(true);
        _tex_hacked_computer.setSmooth(true);
        _tex_closed_door.setSmooth(true);
        _tex_open_door.setSmooth(true);
        _textures_loaded = true;
    }

    _hud_panel.setSize(sf::Vector2f(static_cast<float>(param::game_width), 32.0f));
    _hud_panel.setPosition(0.0f, 0.0f);
    _hud_panel.setFillColor(sf::Color(12, 16, 24, 240));
    _hud_panel.setOutlineColor(sf::Color(55, 70, 95));
    _hud_panel.setOutlineThickness(1.0f);

    _hud_text.setFont(_font);
    _hud_text.setCharacterSize(13);
    _hud_text.setFillColor(sf::Color(255, 195, 45));
    _hud_text.setPosition(15.0f, 7.0f);

    _hp_label.setFont(_font);
    _hp_label.setCharacterSize(13);
    _hp_label.setString("HP:");
    _hp_label.setFillColor(sf::Color(255, 80, 80));
    _hp_label.setPosition(390.0f, 7.0f);

    _health_bar_bg.setSize(sf::Vector2f(75.0f, 13.0f));
    _health_bar_bg.setFillColor(sf::Color(25, 15, 20));
    _health_bar_bg.setOutlineColor(sf::Color(255, 80, 80));
    _health_bar_bg.setOutlineThickness(1.0f);
    _health_bar_bg.setPosition(420.0f, 9.0f);

    _health_bar.setSize(sf::Vector2f(75.0f, 13.0f));
    _health_bar.setFillColor(sf::Color(50, 220, 100));
    _health_bar.setPosition(420.0f, 9.0f);

    _hp_val_text.setFont(_font);
    _hp_val_text.setCharacterSize(12);
    _hp_val_text.setFillColor(sf::Color(255, 210, 210));
    _hp_val_text.setPosition(502.0f, 7.0f);

    _stamina_label.setFont(_font);
    _stamina_label.setCharacterSize(13);
    _stamina_label.setString("SNEAK:");
    _stamina_label.setFillColor(sf::Color(60, 240, 140));
    _stamina_label.setPosition(555.0f, 7.0f);

    _stamina_bar_bg.setSize(sf::Vector2f(75.0f, 13.0f));
    _stamina_bar_bg.setFillColor(sf::Color(15, 25, 20));
    _stamina_bar_bg.setOutlineColor(sf::Color(60, 240, 140));
    _stamina_bar_bg.setOutlineThickness(1.0f);
    _stamina_bar_bg.setPosition(612.0f, 9.0f);

    _stamina_bar.setSize(sf::Vector2f(75.0f, 13.0f));
    _stamina_bar.setFillColor(sf::Color(60, 240, 140));
    _stamina_bar.setPosition(612.0f, 9.0f);

    _stamina_val_text.setFont(_font);
    _stamina_val_text.setCharacterSize(12);
    _stamina_val_text.setFillColor(sf::Color(200, 255, 220));
    _stamina_val_text.setPosition(694.0f, 7.0f);

    _status_panel.setSize(sf::Vector2f(static_cast<float>(param::game_width), 34.0f));
    _status_panel.setPosition(0.0f, 616.0f);
    _status_panel.setFillColor(sf::Color(12, 16, 24, 240));
    _status_panel.setOutlineColor(sf::Color(55, 70, 95));
    _status_panel.setOutlineThickness(1.0f);

    _status_text.setFont(_font);
    _status_text.setCharacterSize(14);
    _status_text.setPosition(15.0f, 623.0f);

    _debrief_box.setSize(sf::Vector2f(560.0f, 240.0f));
    _debrief_box.setOrigin(280.0f, 120.0f);
    _debrief_box.setPosition(param::game_width * 0.5f, param::game_height * 0.5f);
    _debrief_box.setFillColor(sf::Color(16, 22, 34, 245));
    _debrief_box.setOutlineColor(sf::Color(255, 185, 40));
    _debrief_box.setOutlineThickness(2.0f);

    _debrief_title.setFont(_font);
    _debrief_title.setCharacterSize(22);
    _debrief_title.setFillColor(sf::Color(50, 240, 100));

    _debrief_body.setFont(_font);
    _debrief_body.setCharacterSize(15);
    _debrief_body.setFillColor(sf::Color(230, 240, 250));

    _current_level = 1;
    Physics::initialise();
    load_level(_current_level);
}

void GameScene::unload() {
    // Destroy wall bodies before shutting down the Box2D world
    _walls.reset();
    Physics::shutdown();
    Scene::unload();
}

void GameScene::load_level(int level_num) {
    _current_level = level_num;
    std::string level_path = res_path("resources/levels/level" + std::to_string(_current_level) + ".txt");
    ls::load_level(level_path, param::tile_size);

    _walls.reset();
    _walls = std::make_shared<Entity>();
    _walls->add_component<WallColliderComponent>();

    respawn();
}

void GameScene::respawn() {
    _entities.list.clear();
    _guards.clear();
    _terminals.clear();
    _doors.clear();
    _hacked_terminals = 0;
    _total_terminals = 0;
    _player_health = 100.0f;
    _hit_cooldown = 0.0f;
    _door_unlocked = false;
    _game_won = false;
    _game_lost = false;
    _level_cleared = false;
    _was_exposed = false;
    _was_sneaking = false;
    _hack_audio_timer = 0.0f;

    std::vector<sf::Vector2i> end_tiles = ls::find_tiles(ls::END);
    for (const auto &e_pos : end_tiles) {
        auto door = std::make_shared<Entity>();
        door->set_position(ls::get_tile_position(e_pos) + sf::Vector2f(param::tile_size * 0.5f, param::tile_size * 0.5f));
        auto d_sprite = door->add_component<SpriteComponent>();
        d_sprite->set_texture(_tex_closed_door, 28.0f);
        _entities.list.push_back(door);
        _doors.push_back(door);
    }

    std::vector<sf::Vector2i> term_tiles = ls::find_tiles(ls::TERMINAL);
    _total_terminals = static_cast<int>(term_tiles.size());
    for (const auto &t_pos : term_tiles) {
        auto term = std::make_shared<Entity>();
        term->set_position(ls::get_tile_position(t_pos) + sf::Vector2f(param::tile_size * 0.5f, param::tile_size * 0.5f));
        term->add_component<TerminalComponent>(&_tex_computer, &_tex_hacked_computer);
        _entities.list.push_back(term);
        _terminals.push_back(term);
    }

    _player = std::make_shared<Entity>();
    std::vector<sf::Vector2i> start_tiles = ls::find_tiles(ls::START);
    if (!start_tiles.empty()) {
        _player->set_position(ls::get_tile_position(start_tiles[0]) + sf::Vector2f(param::tile_size * 0.5f, param::tile_size * 0.5f));
    } else {
        _player->set_position(ls::get_start_position());
    }
    auto p_sprite = _player->add_component<SpriteComponent>();
    p_sprite->set_texture(_tex_hacker, 28.0f);
    _player->add_component<PlayerMovementComponent>();
    _entities.list.push_back(_player);

    std::vector<sf::Vector2i> enemy_tiles = ls::find_tiles(ls::ENEMY);
    for (size_t i = 0; i < enemy_tiles.size(); ++i) {
        auto guard = std::make_shared<Entity>();
        guard->set_position(ls::get_tile_position(enemy_tiles[i]) + sf::Vector2f(param::tile_size * 0.5f, param::tile_size * 0.5f));

        // Add GuardComponent first so the vision cone renders beneath the guard sprite
        auto g_comp = guard->add_component<GuardComponent>();
        auto g_sprite = guard->add_component<SpriteComponent>();
        g_sprite->set_texture(_tex_guard, 30.0f);

        // Assign vertical patrol if the guard stands in an open vertical shaft (>= 5 tiles tall)
        int gx = enemy_tiles[i].x;
        int gy = enemy_tiles[i].y;
        int v_run = 0;
        int dy = -1;
        while (ls::get_tile({gx, gy + dy}) != ls::WALL) { v_run++; dy--; }
        dy = 1;
        while (ls::get_tile({gx, gy + dy}) != ls::WALL) { v_run++; dy++; }

        if (v_run >= 5) {
            g_comp->set_patrol_direction(sf::Vector2f(0.f, -1.f));
        } else if (i % 2 == 0) {
            g_comp->set_patrol_direction(sf::Vector2f(1.f, 0.f));
        } else {
            g_comp->set_patrol_direction(sf::Vector2f(-1.f, 0.f));
        }

        _entities.list.push_back(guard);
        _guards.push_back(guard);
    }
}

void GameScene::update(const float &dt) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Tab)) {
        gs::set_active_scene(Scenes::menu);
        return;
    }

    if (_level_cleared) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Return)) {
            if (_current_level < _max_levels) {
                load_level(_current_level + 1);
            } else {
                load_level(1);
            }
            return;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
            load_level(_current_level);
            return;
        }

        return;
    }

    if (_game_lost) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
            respawn();
            return;
        }
        return;
    }

    if (_game_won) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
            load_level(1);
            return;
        }
        return;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
        respawn();
        return;
    }

    Physics::update(dt);
    auto pmc = _player->get_compatible_components<PlayerMovementComponent>();
    if (!pmc.empty()) {
        pmc[0]->sync_from_physics();
    }
    for (auto &g : _guards) {
        auto gc = g->get_compatible_components<GuardComponent>();
        if (!gc.empty()) {
            gc[0]->sync_from_physics();
        }
    }

    sf::Vector2f p_pos = _player->get_position();

    bool player_sneaking = false;
    float player_stamina = param::sneak_max_duration;
    float max_stamina = param::sneak_max_duration;
    bool player_exhausted = false;
    if (!pmc.empty()) {
        player_sneaking = pmc[0]->is_sneaking();
        player_stamina = pmc[0]->get_stamina();
        max_stamina = pmc[0]->get_max_stamina();
        player_exhausted = pmc[0]->is_exhausted();
    }

    if (player_sneaking && !_was_sneaking) {
        _snd_sneak.play();
    }
    _was_sneaking = player_sneaking;

    bool holding_e = sf::Keyboard::isKeyPressed(sf::Keyboard::E);
    bool in_range_of_any = false;
    bool is_hacking = false;
    _hacked_terminals = 0;

    for (auto &t_ent : _terminals) {
        auto tc = t_ent->get_compatible_components<TerminalComponent>();
        if (!tc.empty()) {
            bool was_hacked = tc[0]->is_hacked();
            if (was_hacked) {
                _hacked_terminals++;
            } else {
                sf::Vector2f diff = t_ent->get_position() - p_pos;
                float dist = sqrt(diff.x * diff.x + diff.y * diff.y);
                if (dist < 55.0f) {
                    in_range_of_any = true;
                    if (holding_e) {
                        is_hacking = true;
                        tc[0]->add_progress(dt * 0.9f);
                        if (tc[0]->is_hacked()) {
                            _snd_done.play();
                        }
                    }
                }
            }
        }
    }

    if (is_hacking) {
        _hack_audio_timer -= dt;
        if (_hack_audio_timer <= 0.0f) {
            _snd_hack.play();
            _hack_audio_timer = 0.12f;
        }
    } else {
        _hack_audio_timer = 0.0f;
    }

    if (_hacked_terminals == _total_terminals && _total_terminals > 0 && !_door_unlocked) {
        _door_unlocked = true;
        for (auto &door : _doors) {
            auto d_sprite = door->get_compatible_components<SpriteComponent>();
            if (!d_sprite.empty()) {
                d_sprite[0]->set_texture(_tex_open_door, 28.0f);
            }
        }
    }

    if (_door_unlocked && ls::get_tile_at(p_pos) == ls::END) {
        if (!_level_cleared) {
            _snd_win.play();
        }
        _level_cleared = true;
        if (_current_level < _max_levels) {
            _debrief_title.setString(">> SECTOR " + std::to_string(_current_level) + " SECURED! <<");
            _debrief_title.setPosition(150.0f, 230.0f);
            _debrief_body.setString(
                "ALL SECURITY TERMINALS BREACHED & EXFILTRATED.\n\n"
                "CHOOSE AN OPTION:\n"
                "[SPACE] / [ENTER] : PROCEED TO SECTOR " + std::to_string(_current_level + 1) + "\n"
                "[R]               : REPLAY THIS SECTOR\n"
                "[TAB]             : RETURN TO MAIN MENU"
            );
            _debrief_body.setPosition(150.0f, 275.0f);
        } else {
            _debrief_title.setString(">> CAMPAIGN ACCOMPLISHED! <<");
            _debrief_title.setPosition(150.0f, 230.0f);
            _debrief_body.setString(
                "ALL 3 SECTORS INFILTRATED SUCCESSFULLY!\n"
                "YOU HAVE ATTAINED MASTER INFILTRATOR STATUS.\n\n"
                "CHOOSE AN OPTION:\n"
                "[SPACE] / [ENTER] : RESTART CAMPAIGN (SECTOR 1)\n"
                "[R]               : REPLAY FINAL SECTOR\n"
                "[TAB]             : RETURN TO MAIN MENU"
            );
            _debrief_body.setPosition(150.0f, 275.0f);
        }
        return;
    }

    bool exposed_to_guard = false;
    for (auto &g : _guards) {
        auto gc = g->get_compatible_components<GuardComponent>();
        if (!gc.empty() && gc[0]->is_seeing_player()) {
            exposed_to_guard = true;
            break;
        }
    }

    if (exposed_to_guard && !_was_exposed) {
        _snd_alert.play();
    }
    _was_exposed = exposed_to_guard;

    if (exposed_to_guard) {
        _player_health -= dt * 18.0f;
        if (_player_health <= 0.0f) {
            _player_health = 0.0f;
            if (!_game_lost) {
                _snd_defeat.play();
            }
            _game_lost = true;
        }
    }

    if (_hit_cooldown > 0.0f) {
        _hit_cooldown -= dt;
    }

    for (auto &g : _guards) {
        sf::Vector2f diff = p_pos - g->get_position();
        float dist = sqrt(diff.x * diff.x + diff.y * diff.y);
        const float contact_radius = param::entity_size * 2.0f + 2.0f;
        if (dist < contact_radius && _hit_cooldown <= 0.0f) {
            _snd_hit.play();
            _player_health -= 25.0f;
            _hit_cooldown = 1.0f;

            if (dist > 0.001f) {
                const sf::Vector2f knock_dir = diff / dist;
                if (!pmc.empty()) {
                    pmc[0]->apply_impulse(knock_dir * param::knockback_speed);
                }
                auto gc = g->get_compatible_components<GuardComponent>();
                if (!gc.empty()) {
                    gc[0]->apply_impulse(-knock_dir * (param::knockback_speed * 0.45f));
                }
            }

            if (_player_health <= 0.0f) {
                _player_health = 0.0f;
                if (!_game_lost) {
                    _snd_defeat.play();
                }
                _game_lost = true;
            }
            break;
        }
    }

    auto p_sprite = _player->get_compatible_components<SpriteComponent>();
    if (!p_sprite.empty()) {
        if (_hit_cooldown > 0.0f) {
            p_sprite[0]->get_sprite().setColor(sf::Color(255, 130, 130));
        } else if (exposed_to_guard) {
            p_sprite[0]->get_sprite().setColor(sf::Color(255, 90, 90));
        } else if (player_sneaking) {
            p_sprite[0]->get_sprite().setColor(sf::Color(180, 255, 210, 150));
        } else {
            p_sprite[0]->get_sprite().setColor(sf::Color::White);
        }
        if (!pmc.empty()) {
            const float fx = pmc[0]->get_facing().x;
            if (fx < -0.1f) {
                p_sprite[0]->set_flip_x(true);
            } else if (fx > 0.1f) {
                p_sprite[0]->set_flip_x(false);
            }
        }
    }

    for (auto &g : _guards) {
        auto gc = g->get_compatible_components<GuardComponent>();
        auto gs_comp = g->get_compatible_components<SpriteComponent>();
        if (!gc.empty() && !gs_comp.empty()) {
            const float gx = gc[0]->get_direction().x;
            if (gx < -0.1f) {
                gs_comp[0]->set_flip_x(true);
            } else if (gx > 0.1f) {
                gs_comp[0]->set_flip_x(false);
            }
        }
    }

    _health_bar.setSize(sf::Vector2f(75.0f * (_player_health / 100.0f), 13.0f));
    if (_player_health > 50.0f) {
        _health_bar.setFillColor(sf::Color(50, 220, 100));
    } else if (_player_health > 25.0f) {
        _health_bar.setFillColor(sf::Color(255, 180, 30));
    } else {
        _health_bar.setFillColor(sf::Color(255, 40, 40));
    }
    _hp_val_text.setString(std::to_string(static_cast<int>(_player_health)) + "%");

    _stamina_bar.setSize(sf::Vector2f(75.0f * (player_stamina / max_stamina), 13.0f));
    if (player_exhausted) {
        _stamina_bar.setFillColor(sf::Color(255, 50, 50));
        _stamina_val_text.setString("EXHAUST");
        _stamina_val_text.setFillColor(sf::Color(255, 80, 80));
    } else if (player_sneaking) {
        _stamina_bar.setFillColor(sf::Color(70, 255, 160));
        _stamina_val_text.setString(std::to_string(static_cast<int>((player_stamina / max_stamina) * 100.0f)) + "%");
        _stamina_val_text.setFillColor(sf::Color(100, 255, 180));
    } else {
        _stamina_bar.setFillColor(sf::Color(40, 190, 110));
        _stamina_val_text.setString(std::to_string(static_cast<int>((player_stamina / max_stamina) * 100.0f)) + "%");
        _stamina_val_text.setFillColor(sf::Color(200, 255, 220));
    }

    _entities.update(dt);

    std::string hud = "SECTOR: " + std::to_string(_current_level) + "/" + std::to_string(_max_levels);
    hud += "   NODES: " + std::to_string(_hacked_terminals) + "/" + std::to_string(_total_terminals);
    if (_door_unlocked) {
        hud += "   [EXIT UNLOCKED]";
    } else {
        hud += "   [EXIT LOCKED]";
    }
    if (exposed_to_guard) {
        hud += "  [ALERT!]";
    }
    _hud_text.setString(hud);

    if (_game_lost) {
        _status_text.setString(">> BUSTED! OPERATIVE DETECTED << Press [SPACE] to Retry Sector | [TAB] for Menu");
        _status_text.setFillColor(sf::Color(255, 60, 60));
    } else if (in_range_of_any) {
        _status_text.setString(">> IN RANGE: HOLD [E] TO HACK SECURITY TERMINAL <<");
        _status_text.setFillColor(sf::Color(255, 215, 50));
    } else {
        _status_text.setString("WASD: Move | Shift: Sneak (Uses Stamina) | Hold E: Hack Terminal");
        _status_text.setFillColor(sf::Color(200, 215, 230));
    }
}

void GameScene::render() {
    ls::render(Renderer::get_window());
    _entities.render();

    Renderer::queue(&_hud_panel);
    Renderer::queue(&_hud_text);
    Renderer::queue(&_hp_label);
    Renderer::queue(&_health_bar_bg);
    Renderer::queue(&_health_bar);
    Renderer::queue(&_hp_val_text);
    Renderer::queue(&_stamina_label);
    Renderer::queue(&_stamina_bar_bg);
    Renderer::queue(&_stamina_bar);
    Renderer::queue(&_stamina_val_text);

    Renderer::queue(&_status_panel);
    Renderer::queue(&_status_text);

    if (_level_cleared) {
        Renderer::queue(&_debrief_box);
        Renderer::queue(&_debrief_title);
        Renderer::queue(&_debrief_body);
    }
}
