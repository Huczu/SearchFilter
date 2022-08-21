#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <json/json.h>

using std::string;
using std::to_string;
using std::getline;
using std::ifstream;
using std::vector;

#include "utils.hpp"
static constexpr auto RECENTLIST_PATH = "/mnt/SDCARD/Roms/recentlist.json";
static constexpr auto RECENTLIST_HIDDEN_PATH = "/mnt/SDCARD/Roms/recentlist-hidden.json";
static constexpr auto FAVORITES_PATH = "/mnt/SDCARD/Roms/favourite.json";

struct GameJsonEntry
{
    string label = "";
    string launch = "";
    int type = 5;
    string rompath = "";
    string imgpath = "";
    string emupath = "";

    static GameJsonEntry fromJson(const string& json_str)
    {
        GameJsonEntry entry;
        Json::Value root;
        Json::Reader reader;

        if (!reader.parse(json_str, root)) {
            std::cerr << "Error parsing the string" << std::endl;
            return entry;
        }

        auto getString = [&root](const string& key) {
            if (root.isMember(key))
                return root[key].asString();
        };

        auto getInt = [&root](const string& key) {
            if (root.isMember(key))
                return root[key].asInt();
        };

        entry.label = getString("label");
        entry.launch = getString("launch");
        entry.type = getInt("type");
        entry.rompath = getString("rompath");
        entry.imgpath = getString("imgpath");

        if (entry.rompath.find(":") != string::npos) {
            vector<string> tokens = split(entry.rompath, ":");
            entry.launch = tokens[0];
            entry.rompath = tokens[1];
        }

        int pos;
        if ((pos = findNth(entry.rompath, "/", 4)) != std::string::npos)
            entry.emupath = entry.rompath.substr(0, pos);

        return entry;
    }

    string toJson() const
    {
        string json_str = "{";
        json_str += "\"label\":\"" + label + "\",";
        json_str += "\"launch\":\"" + launch + "\",";
        json_str += "\"type\":" + to_string(type) + ",";
        if (imgpath.length() > 0)
            json_str += "\"imgpath\":\"" + imgpath + "\",";
        json_str += "\"rompath\":\"" + rompath + "\"";
        json_str += "}";
        return json_str;
    }
};

vector<GameJsonEntry> loadGameJsonEntries(const string& json_path)
{
    vector<GameJsonEntry> entries;    
    ifstream infile(json_path);
    if (!infile.is_open())
        return entries;

    string line;
    while (getline(infile, line)) {
        GameJsonEntry entry = GameJsonEntry::fromJson(line);
        if (entry.label.length() == 0)
            continue;
        entries.push_back(entry);
    }

    infile.close();

    return entries;
}
