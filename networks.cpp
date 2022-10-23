// GCC 10.2.1 20201203 | C++17

// an extensive command-line utility for modifying your wpa_supplicant-*.conf file.
// repo: https://github.com/jibstack64/networks

#include <experimental/filesystem>
#include "networks/log.hpp"
#include "networks/config.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <unistd.h>
#include <array>
#include <map>
#include "ext/toml.hpp"
#include "ext/argh.h"

namespace fs = std::experimental::filesystem;

// credits to dev
#define CRD "https://github.com/jibstack64"

std::string _UNM;
std::string _UID;
std::string _CONFDR;
Config _CONFIG = defaultConfig();
bool _DEBUG = false;
bool _BRUTE = false;

// executes the provided shell command and returns its output. 
std::string execShell(const std::string command, int& exit) {
    auto pipe = ::popen(command.c_str(), "r");
    if (pipe == nullptr) {
        throw;
    }
    std::array<char, 256> buff;
    std::string result;
    while (not std::feof(pipe)) {
        auto bytes = std::fread(buff.data(), 1, buff.size(), pipe);
        result.append(buff.data(), bytes);
    }
    auto rc = ::pclose(pipe);
    if (WIFEXITED(rc)) {
        exit = WEXITSTATUS(rc);
    }
    return result;
}

// assigns values to static, commonly used variables
void loadStatic() {
    _UID = getuid();
    int stat = 0;
    _UNM = execShell("whoami", stat); _UNM.pop_back(); // remove \n, breaks otherwise
    if (stat != 0) {
        logFatal("Failed to get username: exitting.");
    }
    _CONFDR = "/home/"+_UNM+"/.config";
}

// correctly formats the path to an ssid in the database
std::string identify_path(std::string ssid) {
    if (_CONFIG.database_path[_CONFIG.database_path.length()] == '/') {
        return _CONFIG.database_path+ssid+".conf";
    } else {
        return _CONFIG.database_path+"/"+ssid+".conf";
    }
}

// returns true if the provided ssid credentials are present in the database
bool ssid_present(std::string ssid) {
    return fs::exists(identify_path(ssid)) && !ssid.empty();
}

// copies data from f1 to f2
// uses shell, unsafe, but allows sudo to not break code [subject to change]
int transfer(std::string f1, std::string f2) {
    // swaps 2 files
    int stat = 0;
    execShell("sudo cp "+f1+" "+f2, stat);
    return stat;
}

// adds the provided ssid to the database
void add(std::string ssid, std::string password) {
    if (password.empty()) {
        logFatal("No/invalid passphrase! Invalid formatting?");
    }
    if (ssid_present(ssid)) {
        if (_BRUTE) {
            bool success = fs::remove(identify_path(ssid));
            if (!success) {
                logFatal("Failed to forcefully remove credentials of SSID '"+ssid+"'.");
            }
        } else {
            logFatal("The credentials of SSID '"+ssid+"' are already present.");
        }
    }
    std::string filepath = identify_path(ssid);
    std::ofstream file(filepath);
    int stat = 0;
    std::string wpastr = execShell("wpa_passphrase " + ssid + " " + password, stat);
    if (stat != 0) {
        file.close();
        logFatal("Failed to run wpa_passphrase!");
    }
    file << wpastr << std::endl;
    file.close();
    log("Added SSID '"+ssid+"' to the database.", State::Success);
}

// removes the ssid from the database
void rm(std::string ssid) {
    if (!ssid_present(ssid)) {
        logFatal("The credentials of '"+ssid+"' are not present in the database.");
    }
    int stat = 0;
    execShell("rm "+identify_path(ssid), stat);
    if (stat != 0) {
        logFatal("Failed to remove credentials, missing permissions?");
    }
    log("Removed SSID '"+ssid+"' from the database.", State::Success);
}

// switches to the credentials with the given ssid
void sw(std::string ssid) {
    if (!ssid_present(ssid)) {
        logFatal("The credentials of '"+ssid+"' are not present in the database.");
    }
    int stat = 0;
    if (_CONFIG.create_backups) {
        stat = transfer(_CONFIG.target_supplicant, identify_path(".backup"));
        if (stat != 0) {
            logFatal("Failed to create credentials backup, invalid target supplicant?");
        }
    }
    stat = transfer(identify_path(ssid), _CONFIG.target_supplicant);
    if (stat != 0) {
        logFatal("Failed to write credentails to the target supplicant.");
    }
    log("Switched to credentials of SSID '"+ssid+"'.", State::Success);
}


int main(int argc, char** argv) {
    // map of arguments and their descriptors
    const std::map<std::string, std::string> arguments = {
        {"--add", "Adds the provided credentials to the database. The SSID MUST be covered by square brackets and be split with the password by a comma (e.g. [MyWifi],my4m4zingP4ssw0rd)."}, 
        {"--remove", "Identifies the credentials config linked to the provided SSID and removes them from the database."},
        {"--switch", "Switches to the set of credentials matching the provided SSID."},
        {"/--debug", "Enables debug logging."},
        {"/--help", "Presents this text."},
        {"/--brute", "Ignores any present files and forcefully overwrites."}
    };
    argh::parser args;
    for (auto& a : arguments) {
        if (a.first[0] == '/') {
            continue;
        }
        args.add_param(a.first);
    }
    args.parse(argv);
    if (args[{"--debug"}]) {
        _DEBUG = true;
    }
    if (args[{"--help"}]) {
        for (auto arg : arguments) {
            std::string rl = arg.first;
            if (arg.first[0] == '/') {
                rl.replace(0, 1, "");
            }
            std::cout << "# " + rl + "\n? " + arg.second << std::endl; 
        }
        return 0;
    }
    if (args[{"--brute"}]) {
        _BRUTE = true;
    }

    // load configuration and static variables
    loadStatic(); _CONFIG = loadConfig(_CONFDR);
    if (_CONFIG.database_path[0] = '~') {
        _CONFIG.database_path.replace(0, 1, "/home/"+_UNM);
    }

    // ensure that database path is available
    if (!fs::exists(_CONFIG.database_path)) {
        bool success = fs::create_directory(_CONFIG.database_path);
        if (!success) {
            logFatal("Tried to create database directory '"+_CONFIG.database_path+"', failed!");
        }
    }

    // loop through positional arguments
    std::string ssid = "";
    std::string passwd = "";
    bool d = false;
    for (auto& arg : args.params()) {
        if (arg.first == "add" || arg.first == "a") {
            // reset values incase of prev add
            ssid = ""; passwd = "";
            // parse ssid and password from given string
            bool in_ssid = true;
            for (int i = 0; i < arg.second.length(); i++) {
                if (i == 0 && arg.second[i] == '[') {
                    continue;
                } else if (in_ssid) {
                    if (arg.second[i] == ']' && arg.second[i+1] == ',') {
                        i++;
                        in_ssid = false;
                        continue;
                    } else {
                        ssid += arg.second[i];
                    }
                } else {
                    passwd += arg.second[i];
                }
            }
            // add to database
            add(ssid, passwd);
        } else if (arg.first == "remove" || arg.first == "r") {
            rm(arg.second);
        } else if (arg.first == "switch" || arg.first == "s") {
            if (arg.second != "!") {
                sw(arg.second);
            } else {
                if (ssid.empty()) {
                    logFatal("The '!' parameter for switch can only be used when the '--add' parameter is used.");
                } else {
                    sw(ssid);
                }
            }
        }
        d = true;
    }

    if (!d && !_DEBUG) {
        logFatal("Invalid parameters! Run again with '--help' for a list of valid parameters.");
    }

    return 0;
}
