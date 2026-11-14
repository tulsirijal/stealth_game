#include "level_system.hpp"
#include <fstream>
#include <iostream>


std::unique_ptr<LevelSystem::Tile[]> LevelSystem::_tiles;
int LevelSystem::_width;
int LevelSystem::_height;
sf::Vector2f LevelSystem::_offset(0.0f, 50.0f);
sf::Vector2f LevelSystem::_start_position;

float LevelSystem::_tile_size(100.f);
std::vector<std::unique_ptr<sf::RectangleShape>> LevelSystem::_sprites;

std::map<LevelSystem::Tile, sf::Color> LevelSystem::_colors{ {WALL, sf::Color(26, 32, 44)}, {END, sf::Color(35, 210, 85)}, {TERMINAL, sf::Color(255, 175, 25)} };

int LevelSystem::get_height(){return _height;}
int LevelSystem::get_width(){return _width;}

sf::Color LevelSystem::get_color(LevelSystem::Tile t) {
  auto it = _colors.find(t);
  if (it == _colors.end()) {
    _colors[t] = sf::Color::Transparent;
  }
  return _colors[t];
}

void LevelSystem::set_color(LevelSystem::Tile t, sf::Color c) {
  _colors[t] = c;
}

void LevelSystem::load_level(const std::string& path, float tile_size) {
  _tile_size = tile_size;
  size_t w = 0, h = 0;
  std::string buffer;

  // Load in file to buffer
  std::ifstream f(path);
  if (f.good()) {
    f.seekg(0, std::ios::end);
    buffer.resize(f.tellg());
    f.seekg(0);
    f.read(&buffer[0], buffer.size());
    f.close();
  } else {
    throw std::string("Couldn't open level file: ") + path;
  }
  if (!buffer.empty() && buffer.back() != '\n') {
    buffer.push_back('\n');
  }
  int x = 0;

  std::vector<Tile> temp_tiles;
  for (int i = 0; i < buffer.size(); ++i) {
    const char c = buffer[i];
    switch (c) {
    case 'w':
      temp_tiles.push_back(WALL);
      break;
    case 's':
      temp_tiles.push_back(START);
      _start_position = get_tile_position({x, static_cast<int>(h)}) + sf::Vector2f(_tile_size * 0.5f, _tile_size * 0.5f);
      break;
    case 'e':
      temp_tiles.push_back(END);
      break;
    case ' ':
      temp_tiles.push_back(EMPTY);
      break;
    case '+':
      temp_tiles.push_back(WAYPOINT);
      break;
    case 't':
      temp_tiles.push_back(TERMINAL);
      break;
    case 'n':
      temp_tiles.push_back(ENEMY);
      break;
    case '\n':      // end of line
      if (w == 0) { // if we haven't written width yet
        w = i;      // set width
      }
      x = 0;
      h++; // increment height
      continue;
    default:
      std::cout << c << std::endl; // Don't know what this tile type is
    }
    x++;
  }
  if (temp_tiles.size() != (w * h)) {
    throw std::string("Can't parse level file") + path;
  }
  _tiles = std::make_unique<Tile[]>(w * h);
  _width = w; //set static class vars
  _height = h;
  std::copy(temp_tiles.begin(), temp_tiles.end(), &_tiles[0]);
  std::cout << "Level " << path << " Loaded. " << w << "x" << h << std::endl;
  build_sprites();
}

void LevelSystem::build_sprites() {
  _sprites.clear();
  for (int y = 0; y < LevelSystem::get_height(); ++y) {
    for (int x = 0; x < LevelSystem::get_width(); ++x) {
      std::unique_ptr<sf::RectangleShape> s = std::make_unique<sf::RectangleShape>();
      s->setPosition(get_tile_position({x, y}));
      s->setSize(sf::Vector2f(_tile_size, _tile_size));
      Tile t = get_tile({x, y});
      s->setFillColor(get_color(t));
      if (t == WALL) {
        s->setOutlineColor(sf::Color(65, 80, 105, 130));
        s->setOutlineThickness(-1.0f);
      }
      _sprites.push_back(std::move(s));
    }
  }
}

sf::Vector2f LevelSystem::get_tile_position(sf::Vector2i p) {
  return (sf::Vector2f(p.x, p.y) * _tile_size) + _offset;
}

void LevelSystem::set_offset(const sf::Vector2f &offset) {
  _offset = offset;
}

sf::Vector2f LevelSystem::get_offset() {
  return _offset;
}

LevelSystem::Tile LevelSystem::get_tile(sf::Vector2i p) {
  if (p.x < 0 || p.y < 0 || p.x >= _width || p.y >= _height) {
    return WALL;
  }
  return _tiles[(p.y * _width) + p.x];
}

LevelSystem::Tile LevelSystem::get_tile_at(sf::Vector2f v) {
  auto a = v - _offset;
  if (a.x < 0 || a.y < 0) {
    return WALL;
  }
  return get_tile(sf::Vector2i((v - _offset) / (_tile_size)));
}

void LevelSystem::render(sf::RenderWindow &window) {
  for (size_t i = 0; i < _width * _height; ++i) {
    window.draw(*_sprites[i]);
  }
}

sf::Vector2f LevelSystem::get_start_position(){return _start_position;}

std::vector<sf::Vector2i> LevelSystem::find_tiles(LevelSystem::Tile type) {
  auto v = std::vector<sf::Vector2i>();
  for (size_t i = 0; i < _width * _height; ++i) {
    if (_tiles[i] == type) {
      v.push_back({static_cast<int>(i % _width), static_cast<int>(i / _width)});
    }
  }

  return v;
}