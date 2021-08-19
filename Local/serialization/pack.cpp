#include "serialization.cpp"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int main()
{
//initialisation
    database<std::map> DB{};
    read_dataset("../data/units-by-build-1v1.json", DB);
//    std::cout << empty_entries << " out of " << DB.games_count << " have been left empty\n";
    std::ofstream out("../data/results.txt", std::ios_base::out);
    
    out << "Games: " << DB.games_count << std::endl;
    out << "Units: " << DB.units_count << std::endl;
    out << "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n";
    for(const auto& unit : DB.units)
    {
        out << unit.first;
        out << "\n\nUnits: " << unit.second.units_count;
        out << "\n~~~~~~~~~~~~~~~~~~~\n\n";
    }
    out.close();


//figuring out the matchup
//gathering the data
//calculating the stats
//getting the stats
}
