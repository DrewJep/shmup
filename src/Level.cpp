#include "Level.h"
#include <fstream>
#include <sstream>
#include <regex>
#include <iostream>

// Utility: read whole file
std::string Level::readAll(const std::string& path) {
    std::ifstream in(path);
    if (!in) return {};
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

void Level::pushDialogue(std::vector<LevelEvent>& out,
                         const std::string& speaker,
                         const std::string& text)
{
    LevelEvent e;
    e.kind = LevelEvent::Kind::Dialogue;
    e.speaker = speaker;
    e.text = text;
    out.push_back(std::move(e));
}

void Level::pushSpawn(std::vector<LevelEvent>& out,
                      const std::string& enemiesBlock)
{
    LevelEvent e;
    e.kind = LevelEvent::Kind::Spawn;

    // Capture each { ... } block inside the enemies array
    static const std::regex enemyObjRe(R"(\{([^}]*)\})");
    auto begin = std::sregex_iterator(enemiesBlock.begin(), enemiesBlock.end(), enemyObjRe);
    auto end   = std::sregex_iterator();

    for (auto it = begin; it != end; ++it) {
        EnemySpec spec;
        std::string obj = (*it)[1].str();
        std::smatch m;

        if (std::regex_search(obj, m, std::regex(R"re("hp"\s*:\s*(\d+))re")))
            spec.hp = std::stoi(m[1].str());

        if (std::regex_search(obj, m, std::regex(R"re("type"\s*:\s*"([^"]+)")re")))
            spec.type = m[1].str()[0];

        if (std::regex_search(obj, m, std::regex(R"re("shot"\s*:\s*(\d+))re")))
            spec.shot = std::stoi(m[1].str());

        if (std::regex_search(obj, m, std::regex(R"re("path"\s*:\s*(\d+))re")))
            spec.path = std::stoi(m[1].str());

        if (std::regex_search(obj, m, std::regex(R"re("start"\s*:\s*"([^"]+)")re")))
            spec.start = m[1].str();

        e.enemies.push_back(spec);
    }

    out.push_back(std::move(e));
}

bool Level::loadFromFile(const std::string& path)
{
    std::string text = readAll(path);
    if (text.empty()) {
        std::cerr << "Level::loadFromFile: failed to read file: " << path << std::endl;
        return false;
    }

    m_events.clear();

    // Match any full { ... } event and classify it
    static const std::regex eventRe(R"(\{[^}]*\})");
    auto begin = std::sregex_iterator(text.begin(), text.end(), eventRe);
    auto end   = std::sregex_iterator();

    for (auto it = begin; it != end; ++it) {
        std::string block = (*it)[0].str();
        std::smatch m;

        // =============== DIALOGUE ===============
        if (std::regex_search(block, m, std::regex(R"re("type"\s*:\s*"dialogue")re"))) {
            std::string speaker, msg;

            if (std::regex_search(block, m, std::regex(R"re("speaker"\s*:\s*"([^"]+)")re")))
                speaker = m[1].str();

            if (std::regex_search(block, m, std::regex(R"re("text"\s*:\s*"([^"]+)")re")))
                msg = m[1].str();

            pushDialogue(m_events, speaker, msg);
            continue;
        }

        // =============== SPAWN ===============
        if (std::regex_search(block, m, std::regex(R"re("type"\s*:\s*"spawn")re"))) {
            if (std::regex_search(block, m, std::regex(R"re("enemies"\s*:\s*\[([^\]]*)\])re"))) {
                std::string inner = m[1].str();
                pushSpawn(m_events, inner);
            }
            continue;
        }
    }

    m_index = 0;
    std::cout << "Level::loadFromFile: parsed " << m_events.size() << " events from " << path << std::endl;
    for (size_t i = 0; i < m_events.size(); ++i) {
        const auto &e = m_events[i];
        if (e.kind == LevelEvent::Kind::Dialogue) std::cout << "  event["<<i<<"] = Dialogue: '" << e.speaker << "'\n";
        else std::cout << "  event["<<i<<"] = Spawn: " << e.enemies.size() << " enemies\n";
    }
    return !m_events.empty();
}
