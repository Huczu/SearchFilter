#ifndef VERSION
#define VERSION ""
#endif

#include <iostream>
#include <string>
#include <unistd.h>
#include <memory>

#include "common/display.hpp"
#include "tools.hpp"

int main(int argc, char** argv)
{
    bool show_display = false;
    bool clean_all = false;
    string mode = "unknown";
    
    for (int i = 1; i < argc; i++) {
        if (string(argv[i]) == "--version") {
            std::cout << VERSION;
            exit(0);
        }
        else if (string(argv[i]) == "--display")
            show_display = true;
        else if (string(argv[i]) == "--clean_all")
            clean_all = true;
        else if (mode == "unknown")
            mode = argv[i];
        else {
            std::cerr << "Unknown argument: " << argv[i] << std::endl;
            exit(1);
        }
    }

    std::unique_ptr<Display> display;
    if(show_display) {
        display = std::make_unique<Display>();
    }
    auto flipText = [&display](const string& text) {
        if (display) {
            display->flipText(text);
            sleep(1);
        }
    };

    if (mode == "boxart" || mode == "favfix") {
        tools::fixFavorites();
        flipText("Favorites fixed");
    }
    else if (mode == "favsort") {
        tools::sortFavorites();
        flipText("Favorites sorted");
    }
    else if (mode == "favsort2") {
        tools::sortFavoritesBySystem();
        flipText("Favorites sorted");
    }
    else if (mode == "favtools") {
        tools::addFavoritesTools();
        flipText("Shortcuts added");
    }
    else if (mode == "recents") {
        tools::cleanRecentList(RECENTLIST_PATH, clean_all);
        tools::cleanRecentList(RECENTLIST_HIDDEN_PATH, clean_all);
        flipText("Recent list cleaned");
    }

    return 0;
}
