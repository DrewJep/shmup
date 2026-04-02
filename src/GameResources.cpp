#include "GameResources.h"

#include <iostream>

#include "TextureCache.h"

namespace GameResources {

void configureWindow(sf::RenderWindow& window) {
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);
}

bool preloadCoreTextures() {
    return TextureCache::instance().preload({
        "assets/characters/player/player_sky.png",
        "assets/characters/player/player_ground_down_d.png",
        "assets/characters/player/player_ground_straight.png",
        "assets/characters/player/player_ground_up_d.png",
        "assets/characters/ufo.png",
        "assets/characters/tank.png",
        "assets/characters/shot.png",
        "assets/characters/ufo_beam.png",
        "assets/background/objects/building.png",
    });
}

bool tryLoadUiFont(sf::Font& font) {
    if (font.openFromFile("assets/fonts/Qager-zrlmw.ttf")) {
        return true;
    }
    return false;
}

bool tryOpenBackgroundMusic(sf::Music& music) {
    static const char* paths[] = {
        "assets/sounds/music/test_song.mp3",
        "assets/sounds/test_song.mp3",
        "assets/sound/music/test_song.mp3",
        "assets/sound/test_song.mp3",
        "assets/music/test_song.mp3",
    };
    for (const char* p : paths) {
        if (music.openFromFile(p)) {
            std::cout << "Loaded background music: " << p << std::endl;
            return true;
        }
    }
    std::cout << "Background music not found in expected paths." << std::endl;
    return false;
}

} // namespace GameResources
