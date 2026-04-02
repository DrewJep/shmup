#ifndef GAMERESOURCES_H
#define GAMERESOURCES_H

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

namespace GameResources {

void configureWindow(sf::RenderWindow& window);

// Loads shared gameplay textures into TextureCache; returns true if any path succeeded.
bool preloadCoreTextures();

bool tryLoadUiFont(sf::Font& font);

// Tries common paths; returns true if music opened and caller should setLooping/play.
bool tryOpenBackgroundMusic(sf::Music& music);

} // namespace GameResources

#endif
