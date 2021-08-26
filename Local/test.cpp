#include <sstream>      // std::ofstream
#include <chrono>

#include "serialization/serialization.h"

using namespace std;
using namespace std::chrono;

int main () 
{
    //stringstream out;
    //string name("Hello kitty");
    //string name2("Your so pretty");
    //out << name << '\0' << ' ' << name2.c_str() << '\0';
    //getline(out, name, '\0');
    //for (const auto& c: out.view())
    //    cout << c;
    //cin.get();


    //database<std::map> DB{};
    //read_dataset("../data/units-by-build-1v1.json", DB);
    //std::ofstream out("../data/quantitative_results2.txt", std::ios_base::out);
    //out << "Max units in database: " << DB.max << endl << endl;
    //out << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n";
    //out << "Units: " << DB.units.size() << " different units.\n\n";
    //for (const auto& unit: DB.units)
    //    out << unit.second.base.max << endl;
    //out << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n";
    //out << "Max in Patches by units: \n\n";
    //for (const auto& unit: DB.units)
    //    for (const auto& patch: unit.second.base.patch)
    //        out << patch.second.base.max << endl;
    //out << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n";
    //out << "Max in Matchups by units: \n\n";
    //for (const auto& unit: DB.units)
    //    for (const auto& matchup: unit.second.base.patch)
    //        out << matchup.second.base.max << endl;
    //out << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n";
    //out << "Max in Matchups by patches: \n\n";
    //for (const auto& patch: DB.patches)
    //    for (const auto& matchup: patch.second.base.matchup)
    //        out << matchup.second.base.max << endl;
    //out.close();

    // TODO: BUG - find out why 1.26 (\0) patch has 10x games over the DB as a whole


    //database<std::map> DB{};
    //read_dataset("../data/units-by-build-1v1.json", DB);
    //std::ofstream out("../data/quantitative_results_v3.txt", std::ios_base::out);
    //auto units_max = 0, patch_max = 0, matchup_max = 0, patch_by_unit_max = 0, matchup_by_unit_max = 0, matchup_by_patch_max = 0, ll_max1 = 0,
    //ll_max2 = 0, ll_max3 = 0;
    //#define MAX(x, y) x = x > y.second.base.units_count ? x : y.second.base.units_count;
    //#define O(x) out << #x ": " << x << endl;
    //for (const auto& unit: DB.units)
    //{
    //    MAX(units_max, unit);
    //    for (const auto& patch_by_unit: unit.second.base.patch)
    //    {
    //        MAX(patch_by_unit_max, patch_by_unit);
    //        for (const auto& mu_b_p_b_unit: patch_by_unit.second.base.matchups)
    //            MAX(ll_max1, mu_b_p_b_unit);
    //    }
    //    for (const auto& matchup_by_unit: unit.second.base.matchup)
    //    {
    //        MAX(matchup_by_unit_max, matchup_by_unit);
    //        for (const auto& p_b_mu_b_unit: matchup_by_unit.second.base.patch)
    //            MAX(ll_max2, p_b_mu_b_unit);
    //    }
    //}
    //for (const auto& patch: DB.patches)
    //{
    //    MAX(patch_max, patch);
    //    for (const auto& matchup_by_patch: patch.second.base.matchup)
    //    {
    //        MAX(matchup_by_patch_max, matchup_by_patch);
    //        for (const auto& u_b_mu_b_patch: matchup_by_patch.second.base.units)
    //            MAX(ll_max3, u_b_mu_b_patch);
    //    }
    //}
    //for (const auto& matchup: DB.matchups)
    //    MAX(matchup_max, matchup);
    //
    //out << "The max value in dataset: " << DB.units_count << endl << endl;
    //out << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n";
    //O(units_max)
    //O(patch_max)
    //O(matchup_max)
    //O(patch_by_unit_max)
    //O(matchup_by_unit_max)
    //O(matchup_by_patch_max)
    //O(ll_max1)
    //O(ll_max2)
    //O(ll_max3)
    //out.close();

    //database<std::map> DB{};
    //read_dataset("../data/units-by-build-1v1.json", DB);
    //cin.get();

    //auto x = [](auto x){return x(x) > x(x*x) ? x*x : x(x*x);}; // compiles fine!

/*    uint32_t max_units = 0, i = 1;
    for (const auto& [name, unit]: DB.units)
        for (const auto& [id, patch]: unit.patch)
            for (const auto& [mu, matchup]: patch.matchups)
            {
                max_units = max(max_units, matchup.units_count);
                cout << i++ << ")    " << matchup.units_count << "\t max_units = " << max_units << endl;
            } */ 
    
    vector<steady_clock::time_point> time;
    auto T = [&](){ time.push_back(high_resolution_clock::now()); };

    T();
    database<map> DB{};
    T();
    string data_path = "D:/dev/Wc3Stats/Data";
    //string data_path = "..";
    if (!read_dataset(data_path + "/units-by-build-1v1.json", DB))
    {
        cout << "Dataset was not read!\n";
        return 1;
    }
    stringstream out;
    T();
    serialise(out, DB);
    T();
    ofstream datafile("../../data/test3.data", ios_base::out);
    cout << datafile.is_open() << endl;
    datafile << out.view();
    datafile.close();      
    T();

    for (int i(0); i < time.size() - 1; i++)
        cout << duration_cast<microseconds>(time[i + 1] - time[i]).count() / 1000.0 << " ms\n";
    cin.get();
}
