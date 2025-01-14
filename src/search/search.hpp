#ifndef SEARCH_HPP__
#define SEARCH_HPP__

#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
using std::string;
using std::to_string;
using std::vector;

#include "../common/display.hpp"
#include "../common/utils.hpp"
#include "../common/db_cache.hpp"
#include "../common/ConfigEmu.hpp"

#define ACTIVE_SEARCH "active_search"
#define EMU_CONFIG_ON "/mnt/SDCARD/Emu/SEARCH/config.json"
#define EMU_CONFIG_OFF "/mnt/SDCARD/Emu/SEARCH/_config.json"

const string DB_NAME = "data";
const string DB_DIR = fullpath(DB_NAME);
const string DB_PATH = DB_NAME + "/" + CACHE_NAME(DB_NAME);

static SDL_Surface* search_icon = NULL;

void updateDisplay(Display *display, string msg, string submsg = "")
{
    display->clear();

    if (search_icon)
        display->blit(search_icon, {
            320 - search_icon->w / 2,
            150 - search_icon->h / 2,
            search_icon->w,
            search_icon->h
        });

    display->centerText(msg, {320, 240}, display->fonts.display);

    if (submsg.length() > 0)
        display->centerText(submsg, {320, 310});

    display->flip();
}

void addTools(sqlite3* db)
{
    db::insertRom(db, DB_NAME, {
        .disp = "~Tools",
        .path = "",
        .imgpath = "",
        .type = 1
    });

    auto addTool = [db](string disp, string cmd) {
        db::insertRom(db, DB_NAME, {
            .disp = disp,
            .path = cmd,
            .imgpath = DB_DIR + "/Imgs/" + disp + ".png",
            .type = 0,
            .ppath = "~Tools"
        });
    };

    addTool("1. Fix favorites boxart", "boxart");
    addTool("2. Sort favorites (A-Z)", "favsort");
    addTool("3. Sort favorites (by system)", "favsort2");
    addTool("4. Add tools to favorites", "favtools");
    addTool("5. Clean recent list", "recents");
    addTool("6. Install filter", "install_filter");
    addTool("7. Uninstall filter", "uninstall_filter");
}

string totalTextMessage(int total)
{
    return to_string(total) + " game" + (total == 1 ? "" : "s");
}

void performSearch(Display* display, string keyword)
{
    if (exists(DB_PATH))
        remove(DB_PATH.c_str());

    if (!db::create(DB_PATH, DB_NAME)) {
        std::cerr << "Couldn't create database" << std::endl;
        return;
    }

    sqlite3* db;
    int total_lines = 0;

    // Open the database file
    if (!db::open(&db, DB_PATH))
        return;

    keyword = trim(keyword);

    if (keyword.length() == 0) {
        db::insertRom(db, DB_NAME, {
            .disp = "Enter search term...",
            .path = "search",
            .imgpath = DB_DIR + "/Imgs/Enter search term....png"
        });
        // addTools(db);
        total_lines += 2;
        // db::addEmptyLines(db, DB_NAME, total_lines);
        sqlite3_close(db);
        return;
    }

    vector<ConfigEmu> configs = getEmulatorConfigs();
    int total = 0;

    int current_emu = 0;
    int total_emu = configs.size();

    vector<ConfigEmu> missing_caches;

    string status_main = "0 games";
    string status_sub = "";

    for (auto &config : configs) {
        current_emu++;
        status_sub = "Searching: " + to_string(current_emu) + "/" + to_string(total_emu);
        updateDisplay(display, status_main, status_sub);

        string emu_path = dirname(config.path);
        string rom_path = fullpath(emu_path, config.rompath);
        string launch_cmd = fullpath(emu_path) + "/" + config.launch;
        string name = basename(rom_path);
        string cache_path = rom_path + "/" + CACHE_NAME(name);

        if (name.length() == 0)
            continue;

        // System name
        string label = trim(config.label);
        if (label.length() == 0)
            // Use rom folder name if no label exist
            label = basename(config.rompath);

        if (!exists(cache_path)) {
            missing_caches.push_back(config);
            continue;
        }

        vector<RomEntry> result = db::searchEntries(name, keyword);
        int subtotal = result.size();
        label += " (" + to_string(subtotal) + ")";

        if (subtotal <= 0)
            continue;

        total += subtotal;

        status_main = totalTextMessage(total);
        updateDisplay(display, status_main, status_sub);

        db::insertRom(db, DB_NAME, {
            .disp = label,
            .path = rom_path,
            .imgpath = rom_path,
            .type = 1
        });
        total_lines++;

        for (auto &entry : result) {
            string path = launch_cmd + ":" + entry.path;
            db::insertRom(db, DB_NAME, {
                .disp = entry.disp,
                .path = path,
                .imgpath = entry.imgpath,
                .type = 0,
                .ppath = label
            });
        }
    }

    updateDisplay(display, totalTextMessage(total), "Done");

    string all_label = "All systems (" + to_string(total) + ")";

    db::insertRom(db, DB_NAME, {
        .disp = all_label,
        .path = "",
        .imgpath = "",
        .type = 1
    });
    total_lines++;

    if (total == 0) {
        db::insertRom(db, DB_NAME, {
            .disp = "No results",
            .path = "clear",
            .imgpath = DB_DIR + "/../res/help_clear_search.png",
            .type = 0,
            .ppath = all_label
        });
    }
    else {
        // Copy all rows of type=0 and change ppath -> all_label
        db::duplicateResults(db, DB_NAME, all_label);
    }

    if (missing_caches.size() > 0) {
        string cache_missing_label = "~Missing caches (" + to_string(missing_caches.size()) + ")";

        db::insertRom(db, DB_NAME, {
            .disp = cache_missing_label,
            .path = "",
            .imgpath = "",
            .type = 1
        });
        total_lines++;

        for (auto &config : missing_caches) {
            db::insertRom(db, DB_NAME, {
                .disp = trim(config.label),
                .path = "setstate " + config.label,
                .imgpath = DB_DIR + "/../res/help_unavailable.png",
                .type = 0,
                .ppath = cache_missing_label
            });
        }
    }

    // addTools(db);
    total_lines++;

    db::insertRom(db, DB_NAME, {
        .disp = "Clear search",
        .path = "clear",
        .imgpath = DB_DIR + "/../res/help_clear_search.png"
    });
    total_lines++;

    db::insertRom(db, DB_NAME, {
        .disp = "Search: " + keyword,
        .path = "search",
        .imgpath = DB_DIR + "/Imgs/Enter search term....png"
    });
    total_lines++;

    // db::addEmptyLines(db, DB_NAME, total_lines);

    sqlite3_close(db);
}

#endif // SEARCH_HPP__
