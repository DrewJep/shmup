#ifndef ISOMETRIC_UTILS_H
#define ISOMETRIC_UTILS_H

#include <SFML/Graphics.hpp>
#include <cmath>

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
    
    /**
     * Convert isometric screen coordinates to 2D world coordinates
     * @param screenX Screen X coordinate
     * @param screenY Screen Y coordinate
     * @return World position
     */
    sf::Vector2f screenToWorld(float screenX, float screenY);
    
    /**
     * Get isometric tile position from world coordinates
     * @param worldX World X coordinate
     * @param worldY World Y coordinate
     * @return Tile grid position
     */
    sf::Vector2i worldToTile(float worldX, float worldY);
    
    /**
     * Get world coordinates from tile position
     * @param tileX Tile X coordinate
     * @param tileY Tile Y coordinate
     * @return World position
     */
    sf::Vector2f tileToWorld(int tileX, int tileY);
}

#endif // ISOMETRIC_UTILS_H

