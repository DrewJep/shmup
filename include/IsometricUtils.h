#ifndef ISOMETRIC_UTILS_H
#define ISOMETRIC_UTILS_H

#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>

/**
 * Utility functions for isometric projection and coordinate conversion
 */
namespace IsometricUtils {
    // Isometric tile dimensions (for reference)
    const float TILE_WIDTH = 64.0f;
    const float TILE_HEIGHT = 32.0f;
    
    /**
     * Convert 2D world coordinates to isometric screen coordinates
     * @param worldX World X coordinate
     * @param worldY World Y coordinate
     * @return Screen position in isometric projection
     */
    sf::Vector2f worldToScreen(float worldX, float worldY);
    // Overload accepting vector
    inline sf::Vector2f worldToScreen(const sf::Vector2f& w) { return worldToScreen(w.x, w.y); }
    
    /**
     * Convert isometric screen coordinates to 2D world coordinates
     * @param screenX Screen X coordinate
     * @param screenY Screen Y coordinate
     * @return World position
     */
    sf::Vector2f screenToWorld(float screenX, float screenY);
    // Overload accepting vector
    inline sf::Vector2f screenToWorld(const sf::Vector2f& s) { return screenToWorld(s.x, s.y); }
    
    /**
     * Get isometric tile position from world coordinates
     * @param worldX World X coordinate
     * @param worldY World Y coordinate
     * @return Tile grid position
     */
    sf::Vector2i worldToTile(float worldX, float worldY);
    // Overload accepting vector
    inline sf::Vector2i worldToTile(const sf::Vector2f& w) { return worldToTile(w.x, w.y); }
    
    /**
     * Get world coordinates from tile position
     * @param tileX Tile X coordinate
     * @param tileY Tile Y coordinate
     * @return World position
     */
    sf::Vector2f tileToWorld(int tileX, int tileY);
    // Overload accepting vector
    inline sf::Vector2f tileToWorld(const sf::Vector2i& t) { return tileToWorld(t.x, t.y); }

    // Convert tile coordinates directly to screen coordinates
    inline sf::Vector2f tileToScreen(int tileX, int tileY) { return worldToScreen(tileToWorld(tileX, tileY)); }
    inline sf::Vector2f tileToScreen(const sf::Vector2i& t) { return tileToScreen(t.x, t.y); }

    // Bulk conversions
    std::vector<sf::Vector2f> tilesToWorld(const std::vector<sf::Vector2i>& tiles);
    std::vector<sf::Vector2f> tilesToScreen(const std::vector<sf::Vector2i>& tiles);
}

#endif // ISOMETRIC_UTILS_H

