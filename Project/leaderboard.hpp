#pragma once

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <map>
#include <string>
#include <utility>
#include <vector>

struct LeaderboardEntry {
    std::string name;
    float time;
};

class Leaderboard {
public:
    static constexpr int MAX_ENTRIES = 5;

private:
    std::string path;
    std::map<int, std::vector<LeaderboardEntry>> levels;

    static void trim(std::vector<LeaderboardEntry>& entries) {
        if (entries.size() > (size_t)MAX_ENTRIES)
            entries.erase(entries.begin() + MAX_ENTRIES, entries.end());
    }

public:
    explicit Leaderboard(std::string file_path = "leaderboard.ini")
        : path(std::move(file_path)) {
    }

    void load() {
        levels.clear();

        std::ifstream file(path);
        if (!file.is_open())
            return;

        std::string line;
        int current_level = -1;

        while (std::getline(file, line)) {
            if (!line.empty() && line.back() == '\r')
                line.pop_back();

            if (line.empty())
                continue;

            if (line[0] == '[') {
                if (std::sscanf(line.c_str(), "[level_%d]", &current_level) != 1)
                    current_level = -1;
                continue;
            }

            if (current_level < 0)
                continue;

            size_t equals = line.find('=');
            if (equals == std::string::npos)
                continue;

            size_t comma = line.find(',', equals);
            if (comma == std::string::npos)
                continue;

            LeaderboardEntry entry;
            entry.name = line.substr(equals + 1, comma - equals - 1);
            entry.time = std::strtof(line.c_str() + comma + 1, nullptr);

            levels[current_level].push_back(entry);
        }

        for (auto& [level, entries] : levels) {
            std::stable_sort(entries.begin(), entries.end(),
                [](const LeaderboardEntry& a, const LeaderboardEntry& b) { return a.time < b.time; });
            trim(entries);
        }
    }

    void save() const {
        std::ofstream file(path);
        if (!file.is_open())
            return;

        file << std::fixed << std::setprecision(2);

        for (const auto& [level, entries] : levels) {
            file << "[level_" << level << "]\n";
            for (size_t i = 0; i < entries.size(); i++)
                file << (i + 1) << "=" << entries[i].name << "," << entries[i].time << "\n";
            file << "\n";
        }
    }

    bool qualifies(int level, float time) const {
        auto it = levels.find(level);

        if (it == levels.end() || it->second.size() < (size_t)MAX_ENTRIES)
            return true;

        return time < it->second.back().time;
    }

    int add(int level, const std::string& name, float time) {
        auto& entries = levels[level];

        size_t position = 0;
        while (position < entries.size() && entries[position].time <= time)
            position++;

        if (position >= (size_t)MAX_ENTRIES)
            return -1;

        entries.insert(entries.begin() + position, {name, time});
        trim(entries);

        save();
        return (int)position;
    }

    const std::vector<LeaderboardEntry>& getEntries(int level) const {
        static const std::vector<LeaderboardEntry> empty;

        auto it = levels.find(level);
        return it == levels.end() ? empty : it->second;
    }
};