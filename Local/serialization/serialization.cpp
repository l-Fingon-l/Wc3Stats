#include "serialization.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool read_dataset(std::filesystem::path path, database<std::map>& DB) // reading dataset from file
{
    // loading file to memory
    std::ifstream in(path, std::ios::in | std::ios::binary);
    if (!in.is_open()) 
    {
        std::cout << "File " << path << " was not opened!\n";
        return false;
    }
    const auto size = std::filesystem::file_size(path);
    std::string base_file(size, '\0');
    in.read(base_file.data(), size);
    in.close();

    std::string_view file = base_file;

    // getting to the first entry
    size_t pos = 2;
    game_entry entry{};
    int row = 2; // to keep track of the rows

    // reading the entries line by line
    while(read_entry(file, pos, entry))
    {
        if (entry.matchup == 1)
        {
            std::cout << "Weird replay at row " << row << ". Skipped\n.";
        }
        else 
        {
            entry.matchup = mu_shortened_pack.find(entry.matchup)->second; // matchup packing
            if (!process_entry(DB, entry))
            {
                std::cout << "Entry at position " << pos << " was not processed correctly!\n";
                return false;
            }
        }
        
        entry.units.clear();
        entry.units_count = 0;
        entry.matchup = 0;
        pos++;
        row++;
    }
    
    // checking for the consistency
    if (pos + 2 != size)
    {
        std::cout << "File was not processed correctly!\n";
        return false;
    }

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

    bool race_found = true; // to check if the matchup is valid

    // reading the first player entry
    if (!read_player(file, pos, entry, race_found))
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
    if (!read_player(file, pos, entry, race_found))
    {
        std::cout << "Error in reading the entry for player 2\n";
        return false;
    }

    if (!race_found) entry.matchup = 1; // indicate that the matchup is invalid

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

bool read_player(std::string_view& file, size_t& pos, game_entry& entry, bool& race_found) // reading a player entry
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
    bool first = true;
    // reading the units one by one
    while(read_unit(file, pos, entry, first));
    if (first) race_found &= false;
    
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

bool read_unit(std::string_view& file, size_t& pos, game_entry& entry, bool& first) // reading a unit entry 
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
    const std::string_view name = file.substr(pos, file.find('"', pos) - pos);
    pos += name.size();

    // detecting the race in case of the first unit
    if (first) 
    {
        first = false;
        if (name == "Wisp" || name == "Archer") // wisp - NE
            entry.matchup += N;
        else if (name == "Acolyte" || name == "Ghoul") // acolyte - UD
            entry.matchup += U; 
        else if (name == "Peasant" || name == "Footman") // peasant - Hum
            entry.matchup += H;
        else if (name == "Peon" || name == "Grunt") // peon - Orc
            entry.matchup += O;
        else first = true; // in case the first unit is not a worker
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

    // patches section
    auto& struct2 = DB.patches;
    auto _patch = struct2.find(entry.patch);
    if (_patch == struct2.end())
        _patch = struct2.insert({entry.patch, {}}).first;
    _patch->second.units_count += entry.units_count;
    _patch->second.games_count++;
    //MAX1(_patch);

    // matchup section
    auto& struct3 = DB.matchups;
    auto _matchup = struct3.find(entry.matchup);
    if (_matchup == struct3.end())
        _matchup = struct3.insert({entry.matchup, {}}).first;
    _matchup->second.units_count += entry.units_count;
    _matchup->second.games_count++;
    //MAX1(_matchup);

    // matchup_by_patch section
    auto& struct4 = _patch->second.matchup;
    auto _matchup_by_patch= struct4.find(entry.matchup);
    if (_matchup_by_patch == struct4.end())
        _matchup_by_patch = struct4.insert({entry.matchup, {}}).first;
    _matchup_by_patch->second.units_count += entry.units_count;
    _matchup_by_patch->second.games_count++;
    //MAX2(_patch, _matchup_by_patch);

    // linking patches_by_matchups by pointer
    _matchup->second.patch.insert({entry.patch, {&_matchup_by_patch->second}});

    //#define MAX1(x) DB.max = std::max(DB.max, x->second.units_count)
    //#define MAX2(x, y) x->second.max = std::max(x->second.max, y->second.units_count)

    for (const auto& [_name, count] : entry.units)
    {
        // general units section which stores the strings themselves
        auto& struct1 = DB.units;
        auto _unit = struct1.find(_name);
        if (_unit == struct1.end())
            _unit = struct1.insert({std::string(_name), {}}).first;
        _unit->second.units_count += count;
        _unit->second.games_count++;
        //MAX1(_unit);

        // save the name view
        const auto name = std::string_view(_unit->first); 

        // patch_by_unit section
        auto& struct5 = _unit->second.patch;
        auto _patch_by_unit = struct5.find(entry.patch);
        if (_patch_by_unit == struct5.end())
            _patch_by_unit = struct5.insert({entry.patch, {}}).first;
        _patch_by_unit->second.units_count += count;
        _patch_by_unit->second.games_count++;
        //MAX2(_unit, _patch_by_unit);

        // linking units_by_patches by pointer
        _patch->second.units.insert({name, {&_patch_by_unit->second}});

        // matchup_by_patch_by_unit section
        auto& struct6 = _patch_by_unit->second.matchups;
        auto _matchup_by_patch_by_unit = struct6.find(entry.matchup);
        if (_matchup_by_patch_by_unit == struct6.end())
            _matchup_by_patch_by_unit = struct6.insert({entry.matchup, {}}).first;
        _matchup_by_patch_by_unit->second.units_count += count;
        _matchup_by_patch_by_unit->second.games_count++;
        //MAX2(_patch_by_unit, _matchup_by_patch_by_unit);
        
        // unit_by_matchup_by_patch section
        auto& struct7 = _matchup_by_patch->second.units;
        auto _unit_by_matchup_by_patch = struct7.find(name);
        if (_unit_by_matchup_by_patch == struct7.end())
            _unit_by_matchup_by_patch = struct7.insert({name, {}}).first;
        _unit_by_matchup_by_patch->second.units_count += count;
        _unit_by_matchup_by_patch->second.games_count++;
        //MAX2(_matchup_by_patch, _unit_by_matchup_by_patch);

        // matchup_by_unit section
        auto& struct8 = _unit->second.matchup;
        auto _matchup_by_unit = struct8.find(entry.matchup);
        if (_matchup_by_unit == struct8.end())
            _matchup_by_unit = struct8.insert({entry.matchup, {}}).first;
        _matchup_by_unit->second.units_count += count;
        _matchup_by_unit->second.games_count++;
        //MAX2(_unit, _matchup_by_unit);

        // linking units_by_matchups by pointer
        _matchup->second.units.insert({name, {&_matchup_by_unit->second}});

        // patch_by_matchup_by_unit section
        auto& struct9 = _matchup_by_unit->second.patch;
        auto _patch_by_matchup_by_unit = struct9.find(entry.patch);
        if (_patch_by_matchup_by_unit == struct9.end())
            _patch_by_matchup_by_unit = struct9.insert({entry.patch, {}}).first;
        _patch_by_matchup_by_unit->second.units_count += count;
        _patch_by_matchup_by_unit->second.games_count++;
        //MAX2(_matchup_by_unit, _patch_by_matchup_by_unit);
    }

    return true;
}

struct write_map // our functor for writing down the maps of our DB
{
    // "compile"-time packing constants
    const uint8_t R;
    const uint8_t length;
    const uint8_t bit_width;
    const uint8_t bit_length;
    const uint8_t N;
    const uint16_t break_entry;
    char* const result;
    std::stringstream& s;
    write_map(uint8_t R, auto& s) : R(R), length(R / 8 + 1), bit_width(std::bit_width((uint8_t)(R + 1))), 
        bit_length(length * 8), N((bit_length - 1) / bit_width), result(new char[length]), s(s),
        break_entry([&](){ 
            uint16_t _full = 0xFFFF; 
            _full <<= 16 - bit_width;
            _full >>= 16 - bit_width;
            return _full; 
        }()) {}

    ~write_map() { delete[] result; }
    
    auto operator()(auto& map, uint8_t games_size = 2, uint8_t units_size = 4) const // maybe just rewrite as loop //<<<<=========>>>>========<<<<<<===========>>>>>>========<<<<=======>>>>
    {
        uint8_t n = N;
        memset(result, 0, length);
        auto entry = map.begin();
/*        auto chbit = [](int x, int i, bool v) 
        {
            if(v) return x | (1 << (7 - i));
            return x & ~(1 << (7 - i));
        };

        auto write = [&](int index, int bits, uint32_t data = 0xFFFFFFFF) 
        {
            index += bits - 1;
            while(bits--) // try < bits-- > instead of < data >
            {
                result[index / 8] = chbit(result[index / 8], index % 8, data & 1);
                data >>= 1; // try < /= 2 > instead of < >>= 1 > as well
                index--;
            }
        };
        
        // writing index-header
        if (map.size() <= N) // single_value_packaging
        {
            auto shift = 1; // opening bit is 0
            auto single_value_packaging = [&](const auto& impl)->void
            {
                write(shift, bit_width, entry->first);
                shift += bit_width;
                n--;
                if (++entry != map.end())
                    impl(impl);
                else if (n) write(shift, bit_width, 0xFFFFFFFF);
            };
            single_value_packaging(single_value_packaging);
        }
        else // bit field packing
        {
            write(0, 1); // opening bit is 1 
            for (int i(0); i < R; i++)
            {
                if (entry->first == i)
                {
                    write(i + 1, 1, 1);
                    entry++;
                }
            } 
        }
        s.write(result, length); */
/*        uint8_t index = 0;
       auto chbit = [](int x, int i, bool v) 
        {
            if(v) return x | (1 << (7 - i));
            return x & ~(1 << (7 - i));
        };

        auto write = [&](int index, int bits, uint32_t data = 0xFFFFFFFF) 
        {
            index += bits - 1;
            while(bits--)
            {
                result[index / 8] = chbit(result[index / 8], index % 8, data & 1);
                data >>= 1; 
                index--;
            }
        };

        // writing index-header
        if (map.size() <= N) // single_value_packaging
        {
            index++; // opening bit is 0
            do
            {
                write(index, bit_width, entry->first);
                index += bit_width;
                n--;
                if (++entry == map.end())
                {
                    if(n) write(index, bit_width, 0xFFFFFFFF);
                    break;
                }
            } while (true);
        } */
        uint8_t index = 0;
        auto write = [&](uint8_t bits, uint16_t value)
        {
            auto write_impl = [&](const auto buf)
            {
                uint8_t buf_width = (sizeof *buf) * 8;
                uint16_t res = value << buf_width - bits - index % buf_width;
                if (buf_width == 16) res = std::rotl(res, 8);
                *buf |= *std::bit_cast<decltype(buf)>(&res);
                //std::copy_n(std::bit_cast<uint8_t*>(&res), sizeof buf, std::bit_cast<uint8_t*>(buf));
                index += bits;
            };

            if (index / 8 + 1 == length) write_impl(std::bit_cast<uint8_t*>(&result[index / 8]));
            else write_impl(std::bit_cast<uint16_t*>(&result[index / 8]));
        };
        // writing index-header
        if (map.size() <= N) // single_value_packaging
        {
            index++; // opening bit is 0
            do
            {
                write(bit_width, entry->first);
                n--;
                if (++entry == map.end())
                {
                    if(n) write(bit_width, break_entry);
                    break;
                }
            } while (true);
        }
        else // bit field packing
        {
            result[index++] = (char)0x80; // opening bit is 1
            for (auto res = result; entry != map.end(); index++)
            {
                if (!(index % 8)) res++;
                if (entry->first + 1 == index)
                {
                    *res |= 1 << (7 - index % 8);
                    entry++;
                }
            } 
        }
        s.write(result, length); 

        // writing the data entries themselves
        for (const auto& [key, val]: map)
        {
            s.write((char*)&val.games_count, games_size);
            s.write((char*)&val.units_count, units_size);
        }
    }
};

bool serialise(std::stringstream& s, database<std::map>& DB) // serialization                  // TODO: MATCHUPS MAPS ARENT ENCODED PROPERLY! done
{
    write_map write_patch{(uint8_t)DB.patches.size(), s};
    write_map write_matchup{(uint8_t)DB.matchups.size(), s};

    s.write((char*)&DB.games_count, 8); // database data
    write_matchup(DB.matchups);
    write_patch(DB.patches);
    for (const auto& [id, patch]: DB.patches)
        write_matchup(patch.matchup);
    
    auto db_units_size = DB.units.size();
    s.write((char*)&db_units_size, 1); // the amount of distinct units (<256) (78)

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
        s.write((char*)&unit.games_count, 2);
        s.write((char*)&unit.units_count, 4);

        write_matchup(unit.matchup);
        write_patch(unit.patch);

        for (const auto& [patch_number, patch]: unit.patch)
            write_matchup(patch.matchups, 2, 2);
    }

    return true;
}


// game 45367 (original json) is quite weird. Acolytes after ghouls and both are really late
