#pragma once

#include <filesystem>

#include "database.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

struct game_entry // a data structure storing the information of 1 game entry
{
    std::map<std::string_view, uint32_t> units;
    uint32_t units_count;
    uint8_t patch;
    uint8_t matchup;
};

//int empty_entries = 0; // for empty games
//struct game_entry_1
//{
//    std::unordered_map<std::string, uint32_t> units;
//    uint32_t units_count;
//    uint8_t patch;
//    uint8_t matchup;
//};
//std::unordered_set<game_entry_1*> DB_shorts; 

bool read_dataset(std::filesystem::path path, database<std::map>& DB); 
bool read_entry(std::string_view& file, size_t& pos, game_entry& entry);
bool read_player(std::string_view& file, size_t& pos, game_entry& entry, bool& race_found);
bool read_unit(std::string_view& file, size_t& pos, game_entry& entry, bool& first);
bool process_entry(database<std::map>& DB, game_entry& entry);
bool serialise(std::stringstream& s, database<std::map>& DB);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
