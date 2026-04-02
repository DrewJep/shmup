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

struct LevelEvent {
    enum class Kind { Dialogue, Spawn } kind;
    // Dialogue
    std::string speaker;
    std::string text;
    // Spawn
    std::vector<EnemySpec> enemies;
};

// Very small level loader for our limited JSON format. Not a full JSON parser;
// it looks for `type: "dialogue"` entries with speaker/text and `type: "spawn"`
// entries with an enemies array of simple objects.
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

private:
    std::vector<LevelEvent> m_events;
    size_t m_index = 0;

    // helpers
    static std::string readAll(const std::string& path);
    static void pushDialogue(std::vector<LevelEvent>& out, const std::string& speaker, const std::string& text);
    // Push spawn event from a parsed JSON array of enemy objects
    static void pushSpawn(std::vector<LevelEvent>& out, const Json::Value& arrEnemies);
};

#endif // LEVEL_H
