#ifndef TEXTURECACHE_H
#define TEXTURECACHE_H

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// Loads each path at most once; returns shared sf::Texture instances for sprites.
class TextureCache {
public:
    static TextureCache& instance();

    // Returns nullptr if the file could not be loaded.
    sf::Texture* load(const std::string& path);

    bool preload(const std::vector<std::string>& paths);

private:
    TextureCache() = default;

    std::unordered_map<std::string, std::unique_ptr<sf::Texture>> textures_;
};

#endif
