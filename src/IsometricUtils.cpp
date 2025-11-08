#include "IsometricUtils.h"

namespace IsometricUtils {
    sf::Vector2f worldToScreen(float worldX, float worldY) {
        // Isometric projection: rotate 45 degrees and scale
        // Standard isometric transformation
        float screenX = (worldX - worldY) * (TILE_WIDTH / 2.0f);
        float screenY = (worldX + worldY) * (TILE_HEIGHT / 2.0f);
        return sf::Vector2f(screenX, screenY);
    }
    
    sf::Vector2f screenToWorld(float screenX, float screenY) {
        // Inverse isometric transformation
        float worldX = (screenX / (TILE_WIDTH / 2.0f) + screenY / (TILE_HEIGHT / 2.0f)) / 2.0f;
        float worldY = (screenY / (TILE_HEIGHT / 2.0f) - screenX / (TILE_WIDTH / 2.0f)) / 2.0f;
        return sf::Vector2f(worldX, worldY);
    }
    
    sf::Vector2i worldToTile(float worldX, float worldY) {
        // Convert world coordinates to tile grid coordinates
        int tileX = static_cast<int>(std::floor(worldX / TILE_WIDTH));
        int tileY = static_cast<int>(std::floor(worldY / TILE_HEIGHT));
        return sf::Vector2i(tileX, tileY);
    }
    
    sf::Vector2f tileToWorld(int tileX, int tileY) {
        // Convert tile grid coordinates to world coordinates (center of tile)
        float worldX = tileX * TILE_WIDTH + TILE_WIDTH / 2.0f;
        float worldY = tileY * TILE_HEIGHT + TILE_HEIGHT / 2.0f;
        return sf::Vector2f(worldX, worldY);
    }
}

