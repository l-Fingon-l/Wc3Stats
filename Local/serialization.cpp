#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <charconv>
#include <unordered_set>
#include "serialization.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool read_dataset(fs::path path, database<std::map>& DB) // reading dataset from file
{
    // loading file to memory
    std::ifstream in(path, std::ios::in | std::ios::binary);
    if (!in.is_open()) 
    {
        std::cout << "File " << path << " was not opened!\n";
        return false;
    }
    const auto size = fs::file_size(path);
    std::string base_file(size, '\0');
    in.read(base_file.data(), size);
    in.close();

    std::string_view file = base_file;

    // getting to the first entry
    size_t pos = 2;
    game_entry entry{};

    // reading the entries line by line
    while(read_entry(file, pos, entry))
    {
        if (!process_entry(DB, entry))
        {
            std::cout << "Entry at position " << pos << " was not processed correctly!\n";
            return false;
        }
        entry.units.clear();
        entry.units_count = 0;
        entry.matchup = 0;
        pos++;
    }
    
    // checking for the consistency
    if (pos + 2 != size)
    {
        std::cout << "File was not processed correctly!\n";
        return false;
    }

    // finalising the percentage values in database<std::map>
    DB.finalise();

    return true;
}

bool read_entry(std::string_view& file, size_t& pos, game_entry& entry) // reading a single game entry
{
    // getting to the build number
    std::string_view check("\t{\"buildNumber\":");
    if (file.compare(pos, check.size(), check))
    {
        std::cout << "Entry at position " << pos << " was corrupted!\n";
        return false;
    }
    pos += check.size();

    // reading the build number
    uint16_t patch;
    auto result = std::from_chars(file.data() + pos, file.data() + pos + 4, patch);
    if (result.ec == std::errc::invalid_argument) 
    {
        std::cout << "Error in patch number at position " << pos << std::endl;
        return false;
    }
    pos += 4;

    // compressing build number to 1 byte
    auto found = patch_shortened.find(patch);
    if (found == patch_shortened.end())
    {
        std::cout << "Patch build is abscent in pre-build map\n";
        return false;
    }
    entry.patch = found->second;

    // getting to the first player entry 
    check = ",\"units\":[";
    if (file.compare(pos, check.size(), check))
    {
        std::cout << "Entry at position " << pos << " was corrupted!\n";
        return false;
    }
    pos += check.size();

    // reading the first player entry
    if (!read_player(file, pos, entry))
    {
        std::cout << "Error in reading the entry for player 1\n";
        return false;
    }

    // getting to the second player entry 
    if (file[pos++] != ',')
    {
        std::cout << "Entry at position " << pos - 1 << " after the 1st player entry was corrupted!\n";
        return false;
    }

    // reading the second player entry
    if (!read_player(file, pos, entry))
    {
        std::cout << "Error in reading the entry for player 2\n";
        return false;
    }

    // validating the end of the line
    check = "]}";
    if (file.compare(pos, check.size(), check))
    {
        std::cout << "Entry at position " << pos << "at the end of the line was corrupted!\n";
        return false;
    }
    pos += check.size();

    // end-of-list check
    if (file[pos] == ',') pos++;
    else return false;

    return true;
}

bool read_player(std::string_view& file, size_t& pos, game_entry& entry) // reading a player entry
{
    // getting to the first unit
    std::string_view check("{\"summary\":[");
    if (file.compare(pos, check.size(), check))
    {
        std::cout << "Player entry at position " << pos << " was corrupted!\n";
        return false;
    }
    pos += check.size();

    // reading the first unit (worker) to detect the race
    if (read_unit(file, pos, entry, true))
        // reading the units one by one
        while(read_unit(file, pos, entry, false));
    
    // validating the end of the line
    check = "]}";
    if (file.compare(pos, check.size(), check))
    {
        std::cout << "Entry at position " << pos << "at the end of the line was corrupted!\n";
        return false;
    }
    pos += check.size();

    return true;
}

bool read_unit(std::string_view& file, size_t& pos, game_entry& entry, bool first) // reading a unit entry 
{
    // getting to the name
    std::string_view check("{\"name\":\"");
    if (file.compare(pos, check.size(), check))
    {
        std::cout << "Player entry at position " << pos << " was corrupted!\n";
        return false;
    }
    pos += check.size();

    // reading the name
    std::string_view name = file.substr(pos, file.find('"', pos) - pos);
    pos += name.size();

    // detecting the race in case of the first unit
    if (first) switch (name[0])
    {
        case 'W': entry.matchup += N; // wisp - NE
        break;

        case 'A': entry.matchup += U; // acolyte - UD
        break;

        default: if (name[2] == 'a') entry.matchup += H; // peasant - Hum
        else entry.matchup += O; // peon - Orc
    }

    // getting to count
    check = std::string_view("\",\"count\":");
    if (file.compare(pos, check.size(), check))
    {
        std::cout << "Player entry at position " << pos << " was corrupted!\n";
        return false;
    }
    pos += check.size();

    // reading the count
    uint16_t count;
    size_t end_pos = file.find('}', pos);
    auto result = std::from_chars(file.data() + pos, file.data() + end_pos, count);
    if (result.ec == std::errc::invalid_argument) 
    {
        std::cout << "Error in unit count at position " << pos << std::endl;
        return false;
    }
    pos = end_pos;

    // validating the end of the line
    if (file[pos] != '}')
    {
        std::cout << "Entry at position " << pos << " was corrupted at the end of the line!\n";
        return false;
    }
    pos++;

    // adding the unit record to the entry data structure
    auto found = entry.units.find(name);
    if (found == entry.units.end())
        entry.units[name] = count;
    else found->second += count;
    entry.units_count += count;

    // end-of-list check
    if (file[pos] == ',') pos++;
    else return false;

    return true;
}

bool process_entry(database<std::map>& DB, game_entry& entry) // processing a game entry
{
    if (entry.units.size() < 3 && entry.units_count < 10)
    {

        //std::unordered_map<std::string, uint32_t> result;
        //for(const auto& [name, amount]: entry.units)
        //    result.insert({std::string(name), amount});
        //auto _entry = new game_entry_1{std::unordered_map{result}, entry.units_count, entry.patch, entry.matchup};
        //DB_shorts.insert(_entry);
        //empty_entries++;
        return true;
    }

    DB.units_count += entry.units_count;
    DB.games_count++;

    //#define MAX1(x) DB.max = std::max(DB.max, x->second.base.units_count)
    //#define MAX2(x, y) x->second.base.max = std::max(x->second.base.max, y->second.base.units_count)

    for (const auto& [_name, count] : entry.units)
    {
        // general units section which stores the strings themselves
        auto& struct1 = DB.units;
        auto _unit = struct1.find(_name);
        if (_unit == struct1.end())
            _unit = struct1.insert({std::string(_name), {}}).first;
        _unit->second.base.units_count += count;
        _unit->second.base.games_count++;
        //MAX1(_unit);

        // save the name view
        const auto name = std::string_view(_unit->first); 

        // patches section
        auto& struct2 = DB.patches;
        auto _patch = struct2.find(entry.patch);
        if (_patch == struct2.end())
            _patch = struct2.insert({entry.patch, {}}).first;
        _patch->second.base.units_count += /* maybe the bug is just right here */ count; //<<<<=======>>>>
        _patch->second.base.games_count++;
        //MAX1(_patch); //<<<<=========>>>>========<<<<<<===========>>>>>>========<<<<=======>>>>
    
        // matchup section
        auto& struct3 = DB.matchups;
        auto _matchup = struct3.find(entry.matchup);
        if (_matchup == struct3.end())
            _matchup = struct3.insert({entry.matchup, {}}).first;
        _matchup->second.base.units_count += count;  //<<<<=======>>>>
        _matchup->second.base.games_count++;
        //MAX1(_matchup);
    
        // matchup_by_patch section
        auto& struct4 = _patch->second.base.matchup;
        auto _matchup_by_patch= struct4.find(entry.matchup);
        if (_matchup_by_patch == struct4.end())
            _matchup_by_patch = struct4.insert({entry.matchup, {}}).first;
        _matchup_by_patch->second.base.units_count += count;  //<<<<=======>>>>
        _matchup_by_patch->second.base.games_count++;
        //MAX2(_patch, _matchup_by_patch);

        // patch_by_unit section
        auto& struct5 = _unit->second.base.patch;
        auto _patch_by_unit = struct5.find(entry.patch);
        if (_patch_by_unit == struct5.end())
            _patch_by_unit = struct5.insert({entry.patch, {}}).first;
        _patch_by_unit->second.base.units_count += count;
        _patch_by_unit->second.base.games_count++;
        //MAX2(_unit, _patch_by_unit);

        // linking units_by_patches by pointer
        _patch->second.base.units.insert({name, {{}, &_patch_by_unit->second.base}});

        // matchup_by_patch_by_unit section
        auto& struct6 = _patch_by_unit->second.base.matchups;
        auto _matchup_by_patch_by_unit = struct6.find(entry.matchup);
        if (_matchup_by_patch_by_unit == struct6.end())
            _matchup_by_patch_by_unit = struct6.insert({entry.matchup, {}}).first;
        _matchup_by_patch_by_unit->second.base.units_count += count;
        _matchup_by_patch_by_unit->second.base.games_count++;
        //MAX2(_patch_by_unit, _matchup_by_patch_by_unit);
        
        // unit_by_matchup_by_patch section
        auto& struct7 = _matchup_by_patch->second.base.units;
        auto _unit_by_matchup_by_patch = struct7.find(name);
        if (_unit_by_matchup_by_patch == struct7.end())
            _unit_by_matchup_by_patch = struct7.insert({name, {}}).first;
        _unit_by_matchup_by_patch->second.base.units_count += count;
        _unit_by_matchup_by_patch->second.base.games_count++;
        //MAX2(_matchup_by_patch, _unit_by_matchup_by_patch);

        // matchup_by_unit section
        auto& struct8 = _unit->second.base.matchup;
        auto _matchup_by_unit = struct8.find(entry.matchup);
        if (_matchup_by_unit == struct8.end())
            _matchup_by_unit = struct8.insert({entry.matchup, {}}).first; // maybe remove inner part of brackets
        _matchup_by_unit->second.base.units_count += count;
        _matchup_by_unit->second.base.games_count++;
        //MAX2(_unit, _matchup_by_unit);

        // linking units_by_matchups by pointer
        _matchup->second.base.units.insert({name, {{}, &_matchup_by_unit->second.base}});

        // patch_by_matchup_by_unit section
        auto& struct9 = _matchup_by_unit->second.base.patch;
        auto _patch_by_matchup_by_unit = struct9.find(entry.patch);
        if (_patch_by_matchup_by_unit == struct9.end())
            _patch_by_matchup_by_unit = struct9.insert({entry.patch, {}}).first;
        _patch_by_matchup_by_unit->second.base.units_count += count;
        _patch_by_matchup_by_unit->second.base.games_count++;
        //MAX2(_matchup_by_unit, _patch_by_matchup_by_unit);
    }

    return true;
}
// maybe remove inner parts of the curly brackets, starting with 'percentage{}, ...'
bool serialise(std::ostringstream& s, database<std::map>& DB) // serialization
{
    s.write((char*)&DB.games_count, 8); // database data
    auto db_units_size = DB.units.size();
    s.write((char*)&db_units_size, 2); // the amount of distinct units (<65536) (<256) (78)

    // now diving deep layer by layer
    std::unordered_map<std::string_view, uint8_t> name_index;
    // start with the units subtree
    // the first layer of names is sequential for the sake of compression
    for (auto index = 0; const auto& [name, unit]: DB.units)
    {
        name_index.insert({std::string_view{name}, index++}); // construct a name index map
        s << name << '\0'; // using 0 as a delimiter for unpacking
    }
    // other layers are not sequential for the sake of perfomance and readability
    for (const auto& [name, unit]: DB.units)
    {
        WRITE_STATS(unit);
        auto units_patch_size = unit.base.patch.size();
        s.write((char*)&units_patch_size, 1); // the amount of underlying patches
        for (const auto& [patch_number, patch]: unit.base.patch)
        {
            auto _patch = patch.base;
            s << patch_shortened[patch_number]; /// NO! we definitely do not do this
            s << _patch.games_count << _patch.units_count;
            s << patch.percentage.games << patch.percentage.units;
            s << _patch.matchups.size();
            for (const auto& [matchup_id, matchup]: patch.base.matchups)
            {
                s << matchup_id;
                s << matchup.base.games_count << matchup.base.units_count;
                s << matchup.percentage.games << matchup.percentage.units;
            }
        }
    }
    /*
    //open file stream for writing
    std::fstream s{path, s.binary | s.out};
    if (!s.is_open()) 
    {
        std::cout << "File " << path << " was not opened!\n";
        return false;
    } */

    return true;
}
