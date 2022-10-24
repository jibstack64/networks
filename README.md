# networks

[![Tag: v0.0.1](https://img.shields.io/badge/tag-v0.0.1-yellowgreen.svg)](https://github.com/jibstack64/networks/releases/tag/v0.0.1)
[![License: MIT](https://img.shields.io/badge/license-MIT-red.svg)](https://opensource.org/licenses/MIT)
![Contributions: Welcome](https://img.shields.io/badge/contributions-welcome-success)

### Overview
networks is a command-line utility designed for managing multiple sets of wpa_supplicant credentials. This is particularly useful if you are working on a server, android shell, or fresh install, and have no graphical interface for creating and changing WiFi connections.
networks is built on a local database; you can append credentials to this database, remove them and switch to them. A backup is made of your wpa_supplicant-*.conf file every time you switch to a set of credentials, providing a level of security, as losing your WiFi credentials becomes more difficult.

### Configuration
networks uses TOML as its configuration format. This configuration file can be found at `~/.config/networks.toml` - it contains settings for the database directory, the target wpa_supplicant configuration file and whether backups should be made apon a switch.

### Parameters
- `--add [SSID],password` : Adds the given set of credentials to the database. The format is very important: the SSID should be covered by square brackets and seperated from the password by a comma.
- `--remove SSID` : Finds and removes the credentials containing the provided SSID.
- `--switch SSID` : Switches to the credentials matching the given SSID.
- `--brute` : Forces `--add` to overwrite any set of credentials with a matching SSID to the one provided.
- `--debug` : Enables debugging messages.
- `--help` : Self-explanatory, provides you with a help page similar to that of this README.

### Future changes
- [ ] Remove shell usage for switch.
- [ ] Create class for printing with colours.
- [ ] Allow SSIDs/passwords with spaces.
- [ ] Create an install script.

### Libraries used
> https://github.com/adishavit/argh | Used for argument parsing.


> https://github.com/ToruNiina/toml11 | Configuration (reading TOML).
