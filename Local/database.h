#pragma once

#include <map>
#include <unordered_map>
#include <string>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

const uint8_t N = 1, H = 2, O = 4, U = 8;
const std::map<uint8_t, std::string_view> MUS { { N+H, "NxH" }, { N+O, "NxO" }, { N+U, "NxU" }, { N+N, "NxN" }, 
    { H+H, "HxH" }, { H+O, "HxO" }, { H+U, "HxU" }, { O+U, "OxU" }, { O+O, "OxO" }, { U+U, "UxU" } };
const std::map<uint8_t, uint8_t> mu_shortened_pack { { N+N, 0 }, { N+H, 1 }, { H+H, 2 }, { N+O, 3 }, { H+O, 4 }, 
    { O+O, 5 }, { N+U, 6 }, { H+U, 7 }, { O+U, 8 }, { U+U, 9 } }; // in an ascending order of X+Xs
const std::map<uint16_t, std::string_view> patch_by_build { { 6059, "1.26a" }, { 6060, "1.29" }, { 6061, "1.30" }, { 6072, "1.31" }, { 6105, "1.32.0-1" },
    { 6106, "1.32.2" }, { 6108, "1.32.3" }, { 6109, "1.32.4-5" }, { 6110, "1.32.6-7" }, { 6111, "1.32.8" }, { 6112, "1.32.9" }, { 6114, "1.32.10" } };
const std::map<uint16_t, uint8_t> patch_shortened { { 6059, 0 }, { 6060, 1 }, { 6061, 2 }, { 6072, 3 }, { 6105, 4 },
    { 6106, 5 }, { 6108, 6 }, { 6109, 7 }, { 6110, 8 }, { 6111, 9 }, { 6112, 10 }, { 6114, 11 } };

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

struct basic_entry // e.g. 1.32.2 in NvH, or HvO for footman
{
    uint32_t games_count;
    uint32_t units_count;
//    uint32_t max;              //<<<<=========>>>>========<<<<<<===========>>>>>>========<<<<=======>>>>
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

template<template<class ...> class Map_Type = std::unordered_map>
struct database : basic_entry // the data structure as a whole for packing
{
    struct matchup_by_unit : basic_entry // e.g. NvH for an archer 
    {
        std::map<uint8_t, basic_entry> patch; // e.g. 1.32.2 in NvH
    };

    struct matchup_by_patch : basic_entry // e.g. HvO in 1.26a
    {
        std::map<std::string_view, basic_entry> units; // e.g. shaman in HvO
    };

    struct patch_by_unit : basic_entry // e.g. 1.32.7 for footman 
    {
        std::map<uint8_t, basic_entry> matchups; // e.g. HvO in 1.32.7
    };

    struct unit : basic_entry // e.g. footman
    {
        std::map<uint8_t, patch_by_unit> patch;
        std::map<uint8_t, matchup_by_unit> matchup;
    };

    struct patch : basic_entry // e.g. 1.32.7
    {
        Map_Type<std::string_view, patch_by_unit*> units;
        std::map<uint8_t, matchup_by_patch> matchup;
    };

    struct matchup : basic_entry // e.g. NvH
    {
        Map_Type<std::string_view, matchup_by_unit*> units;
        std::map<uint8_t, matchup_by_patch*> patch; 
    };

    Map_Type<std::string, unit, std::less<>> units;
    std::map<uint8_t, patch> patches;
    std::map<uint8_t, matchup> matchups;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
struct database : basic_entry // the data structure as a whole, ready for use
{
    struct matchup_by_unit : basic_entry // e.g. NvH for an archer 
    {
        std::map<uint8_t, basic_entry> patch; // e.g. 1.32.2 in NvH
    };

    struct matchup_by_patch : basic_entry // e.g. HvO in 1.26a
    {
        std::map<std::string_view, basic_entry> units; // e.g. shaman in HvO
    };

    struct patch_by_unit : basic_entry // e.g. 1.32.7 for footman 
    {
        std::map<uint8_t, basic_entry> matchups; // e.g. HvO in 1.32.7
    };

    struct unit : basic_entry // e.g. footman
    {
        std::map<uint8_t, patch_by_unit> patch;
        std::map<uint8_t, matchup_by_unit> matchup;
    };

    struct patch : basic_entry // e.g. 1.32.7
    {
        std::unordered_map<std::string_view, patch_by_unit>> units;
        std::map<uint8_t, matchup_by_patch> matchup;
    };

    struct matchup : basic_entry // e.g. NvH
    {
        std::unordered_map<std::string_view, matchup_by_unit>> units;
    };

    std::unordered_map<std::string, unit, std::less<>> units;
    std::map<uint8_t, patch> patches;
    std::map<uint8_t, matchup> matchups;
}; */
