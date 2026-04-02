#include "TextureCache.h"
#include <iostream>

TextureCache& TextureCache::instance() {
    static TextureCache cache;
    return cache;
}

sf::Texture* TextureCache::load(const std::string& path) {
    auto it = textures_.find(path);
    if (it != textures_.end()) {
        return it->second.get();
    }
    auto tex = std::make_unique<sf::Texture>();
    if (!tex->loadFromFile(path)) {
        std::cerr << "TextureCache: failed to load: " << path << std::endl;
        return nullptr;
    }
    sf::Texture* ptr = tex.get();
    textures_[path] = std::move(tex);
    return ptr;
}

bool TextureCache::preload(const std::vector<std::string>& paths) {
    bool ok = true;
    for (const auto& p : paths) {
        if (!load(p)) {
            ok = false;
        }
    }
    return ok;
}
