#ifndef LEVEL_H
#define LEVEL_H

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <json/json.h>

struct EnemySpec {
    int hp = 1;
    char type = 'U'; // 'U' = UFO, 'T' = Tank
    int shot = 1;
    int path = 1;
    std::string start = "center"; // left/center/right
};

struct ObstacleSpec {
    float x = 0.f;
    float y = 0.f;
    float width = 0.f;  // 0 = use texture size
    float height = 0.f;
    std::string texturePath;
};

struct LevelEvent {
    enum class Kind { Dialogue, Spawn } kind;
    std::string speaker;
    std::string text;
    std::vector<EnemySpec> enemies;
};

class Level {
public:
    Level() = default;
    bool loadFromFile(const std::string& path);

    bool hasNext() const { return m_index < m_events.size(); }
    const LevelEvent* peek() const { return hasNext() ? &m_events[m_index] : nullptr; }
    std::optional<LevelEvent> next() {
        if (!hasNext()) return std::nullopt;
        return m_events[m_index++];
    }

    // True when every event has been consumed (level had at least one event).
    bool finishedAllEvents() const { return !hasNext() && !m_events.empty(); }

    const std::vector<ObstacleSpec>& obstacleSpecs() const { return m_obstacles; }
    const std::string& levelId() const { return m_levelId; }

private:
    std::vector<LevelEvent> m_events;
    std::vector<ObstacleSpec> m_obstacles;
    std::string m_levelId;
    size_t m_index = 0;

    static std::string readAll(const std::string& path);
    static void pushDialogue(std::vector<LevelEvent>& out, const std::string& speaker, const std::string& text);
    static void pushSpawn(std::vector<LevelEvent>& out, const Json::Value& arrEnemies);
    static void pushObstacle(std::vector<ObstacleSpec>& out, const Json::Value& obj);
};

#endif
