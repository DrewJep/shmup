#include "Level.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <json/json.h>

std::string Level::readAll(const std::string& path) {
    std::ifstream in(path);
    if (!in) return {};
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

void Level::pushDialogue(std::vector<LevelEvent>& out, const std::string& speaker, const std::string& text) {
    LevelEvent e;
    e.kind = LevelEvent::Kind::Dialogue;
    e.speaker = speaker;
    e.text = text;
    out.push_back(std::move(e));
}

void Level::pushSpawn(std::vector<LevelEvent>& out, const Json::Value& arrEnemies) {
    LevelEvent e;
    e.kind = LevelEvent::Kind::Spawn;

    for (const auto& obj : arrEnemies) {
        EnemySpec spec;

        if (obj.isMember("hp")) spec.hp = obj["hp"].asInt();
        if (obj.isMember("type")) spec.type = obj["type"].asString()[0];
        if (obj.isMember("shot")) spec.shot = obj["shot"].asInt();
        if (obj.isMember("path")) spec.path = obj["path"].asInt();
        if (obj.isMember("start")) spec.start = obj["start"].asString();

        std::cout << "Parsed enemy: hp=" << spec.hp << " type=" << spec.type << " shot=" << spec.shot
                  << " path=" << spec.path << " start=" << spec.start << "\n";

        e.enemies.push_back(spec);
    }

    out.push_back(std::move(e));
}

void Level::pushObstacle(std::vector<ObstacleSpec>& out, const Json::Value& obj) {
    ObstacleSpec s;
    if (obj.isMember("x")) s.x = obj["x"].asFloat();
    if (obj.isMember("y")) s.y = obj["y"].asFloat();
    if (obj.isMember("width")) s.width = obj["width"].asFloat();
    if (obj.isMember("height")) s.height = obj["height"].asFloat();
    if (obj.isMember("texture")) s.texturePath = obj["texture"].asString();
    out.push_back(std::move(s));
}

bool Level::loadFromFile(const std::string& path) {
    std::string text = readAll(path);
    if (text.empty()) {
        std::cerr << "Level::loadFromFile: failed to read " << path << "\n";
        return false;
    }

    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errs;

    std::istringstream ss(text);
    if (!Json::parseFromStream(builder, ss, &root, &errs)) {
        std::cerr << "JSON parse error: " << errs << "\n";
        return false;
    }

    if (!root.isMember("events") || !root["events"].isArray()) {
        std::cerr << "JSON missing events array (use [] if none)\n";
        return false;
    }

    m_events.clear();
    m_obstacles.clear();
    m_levelId.clear();

    if (root.isMember("level")) {
        m_levelId = root["level"].asString();
    }

    if (root.isMember("obstacles") && root["obstacles"].isArray()) {
        for (const auto& o : root["obstacles"]) {
            pushObstacle(m_obstacles, o);
        }
    }

    const auto& events = root["events"];
    for (const auto& obj : events) {
        if (!obj.isMember("type")) continue;
        std::string type = obj["type"].asString();

        if (type == "dialogue") {
            std::string speaker = obj.get("speaker", "").asString();
            std::string dlg = obj.get("text", "").asString();
            pushDialogue(m_events, speaker, dlg);
        } else if (type == "spawn") {
            if (obj.isMember("enemies") && obj["enemies"].isArray()) {
                pushSpawn(m_events, obj["enemies"]);
            }
        }
    }

    m_index = 0;

    bool hasContent = !m_events.empty() || !m_obstacles.empty();
    if (!hasContent) {
        std::cerr << "Level has no events and no obstacles: " << path << "\n";
        return false;
    }

    std::cout << "Parsed " << m_events.size() << " events, " << m_obstacles.size() << " obstacles from " << path
              << "\n";

    return true;
}
