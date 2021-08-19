#include "database.h"

#define COMPUTE() \
    y.percentage.games = 100.0 * y.base.games_count / games_count; \
    y.percentage.units = 100.0 * y.base.units_count / units_count;

#define COMPUTE1() \
    y.percentage.games = 100.0 * y.base->games_count / games_count; \
    y.percentage.units = 100.0 * y.base->units_count / units_count;

inline void database<std::map>::matchup_by_unit::finalise()
{
    for (auto& [x, y] : patch)
    {
        COMPUTE();
    }
}

inline void database<std::map>::matchup_by_patch::finalise()
{
    for (auto& [x, y] : units)
    {
        COMPUTE();
    }
}

inline void database<std::map>::patch_by_unit::finalise()
{
    for (auto& [x, y] : matchups)
    {
        COMPUTE();
    }
}

void database<std::map>::unit::finalise()
{
    for (auto& [x, y] : patch)
    {
        COMPUTE();
        y.base.finalise();
    }

    for (auto& [x, y] : matchup)
    {
        COMPUTE();
        y.base.finalise();
    }
}

void database<std::map>::patch::finalise()
{
    for (auto& [x, y] : units)
    {
        COMPUTE1();
        y.base->finalise();
    }

    for (auto& [x, y] : matchup)
    {
        COMPUTE();
        y.base.finalise();
    }
}

void database<std::map>::matchup::finalise()
{
    for (auto&[x, y] : units)
    {
        COMPUTE1();
        y.base->finalise();
    }
}

void database<std::map>::finalise()
{
    for (auto&[x, y] : units)
    {
        COMPUTE();
        y.base.finalise();
    }

    for (auto&[x, y] : patches)
    {
        COMPUTE();
        y.base.finalise();
    }

    for (auto&[x, y] : matchups)
    {
        COMPUTE();
        y.base.finalise();
    }
}
