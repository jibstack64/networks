#pragma once

#include <iostream>
#include <string>
#include "log.hpp"
#include <fstream>
#include "../ext/toml.hpp"
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

struct Config {
    std::string database_path;
    std::string target_supplicant;
    bool create_backups;
    Config(std::string d, std::string s, bool b) : database_path(d), target_supplicant(s), create_backups(b) {}
};

// returns the default configuration object
Config defaultConfig() {
    return Config(
        "~/.networks", "/etc/wpa_supplicant/wpa_supplicant.conf", true
    );
}

// loads the configuration file in its respective Config struct, also
// ensures that the .config directory and networks.conf files are present.
Config loadConfig(std::string directory) {
    std::string filepath;
    if (directory[directory.length()] == '/') {
        filepath = directory + "networks.toml";
    } else {
        filepath = directory + "/networks.toml";
    }
    // ensure that config exists
    if (!fs::exists(directory)) {
        log("'"+directory+"' directory does not exist, creating...");
        bool success = fs::create_directory(directory);
        if (!success) {
            log("Failed to create '"+directory+"' directory! Using default configuration.", State::Error);
        }
    }
    // create configuration if absent
    if (!fs::exists(filepath)) {
        log("'"+filepath+"' file does not exist, creating and inserting default configuration...");
        std::fstream file(filepath, std::ios::out);
        if (!file.is_open()) {
            log("Failed to create '"+filepath+"' file! Using default configuration.", State::Error);
        } else {
            Config de = defaultConfig();
            file << "\ndatabase='"+de.database_path+"'\ntarget='"+de.target_supplicant+"'\nbackup="+(de.create_backups ? "true" : "false")+"\n";
        }
    }
    auto data = toml::parse(filepath); // attempt regardless - default key values ¯\_(;])_/¯
    Config config = Config {
        toml::find_or<std::string>(data, "database", "~/.networks"),
        toml::find_or<std::string>(data, "target", "/etc/wpa_supplicant/wpa_supplicant.conf"),
        toml::find_or<bool>(data, "backup", true)
    };
    return config;
}
