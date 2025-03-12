/* Copyright (c) 2023-2024 Dreamy Cecil
This program is free software; you can redistribute it and/or modify
it under the terms of version 2 of the GNU General Public License as published by
the Free Software Foundation


This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

// A pair of integers
typedef SLONG IntPair_t[2];

// Array of path dots
typedef IntPair_t PathDots_t[10];

// Current map type
extern ELevelFormat _eMapType;

// Determine map type from level name
ELevelFormat ScanLevelName(const CTString &strLevelName);

// Determine map type from level name and return its index
ELevelFormat ScanLevelName(INDEX &iLevel, const CTString &strLevelName);

// The Second Encounter map

static const IntPair_t _aIconCoordsTSE[] = {
  {  0,   0}, // 00: Last Episode
  {168, 351}, // 01: Palenque 01
  { 42, 345}, // 02: Palenque 02
  { 41, 263}, // 03: Teotihuacan 01
  {113, 300}, // 04: Teotihuacan 02
  {334, 328}, // 05: Teotihuacan 03
  {371, 187}, // 06: Ziggurat
  {265, 111}, // 07: Atrium
  {119, 172}, // 08: Gilgamesh
  {  0, 145}, // 09: Babel
  { 90,  30}, // 10: Citadel
  {171,  11}, // 11: Land of Damned
  {376,   0}, // 12: Cathedral
};

static const IntPair_t _aPathBetweenLevelsTSE[] = {
  { 0,  1}, // Intro -> first level
  { 1,  2},
  { 2,  3},
  { 3,  4},
  { 4,  5},
  { 5,  6},
  { 6,  7},
  { 7,  8},
  { 8,  9},
  { 9, 10},
  {10, 11},
  {11, 12},
  {-1, -1}, // [Cecil] End
};

static const PathDots_t _aPathDotsTSE[] = {
  // 00: Intro
  { {-1,-1}, },

  // 01: Palenque01 - Palenque02
  { {211,440}, {193,447}, {175,444}, {163,434}, {152,423}, {139,418}, {-1,-1}, },

  // 02: Palenque02 - Teotihuacan01
  { {100,372}, {102,363}, {108,354}, {113,345}, {106,338}, {-1,-1}, },

  // 03: Teotihuacan01 - Teotihuacan02
  { {153,337}, {166,341}, {180,346}, {194,342}, {207,337}, {-1,-1}, },

  // 04: Teotihuacan02 - Teotihuacan03
  { {279,339}, {287,347}, {296,352}, {307,365}, {321,367}, {335,362}, {-1,-1}, },

  // 05: Teotihuacan03 - Ziggurat
  { {-1,-1}, },

  // 06: Ziggurat - Atrium
  { {412,285}, {396,282}, {383,273}, {368,266}, {354,264}, {-1,-1}, },

  // 07: Atrium - Gilgamesh
  { {276,255}, {262,258}, {248,253}, {235,245}, {222,240}, {-1,-1}, },

  // 08: Gilgamesh - Babel
  { {152,245}, {136,248}, {118,253}, {100,251}, {85,246}, {69,243}, {-1,-1}, },

  // 09: Babel - Citadel
  { {-1,-1}, },

  // 10: Citadel - Lod
  { {190,130}, {204,126}, {215,119}, {232,116}, {241,107}, {-1,-1}, },

  // 11: Lod - Cathedral
  { {330,108}, {341,117}, {353,126}, {364,136}, {377,146}, {395,147}, {-1,-1}, },
};

// The First Encounter map

static const IntPair_t _aIconCoordsTFE[] = {
  {  0,   0}, // 00: Intro
  {175, 404}, // 01: Hatshepsut
  { 60, 381}, // 02: Sand Canyon
  { 50, 300}, // 03: Ramses
  {171, 304}, // 04: Canyon
  {190, 225}, // 05: Waterfall
  {303, 305}, // 06: Oasis
  {361, 296}, // 07: Dunes
  {362, 222}, // 08: Suburbs
  {321, 211}, // 09: Sewers
  {316, 156}, // 10: Metropolis
  {194, 157}, // 11: Sphynx
  {160, 111}, // 12: Karnak
  {167,  61}, // 13: Luxor
  { 50,  53}, // 14: Sacred
  {185,   0}, // 15: Pyramid
};

static const IntPair_t _aPathBetweenLevelsTFE[] = {
  { 0,  1}, // Intro -> first level
  { 1,  2},
  { 2,  3},
  { 3,  4}, // To Moon Mountains
  { 4,  5}, // From Moon Mountains
  { 4,  6},
  { 5,  6},
  { 6,  7},
  { 7,  8},
  { 8,  9},
  { 9, 10},
  {10, 11},
  {11, 12},
  {12, 13},
  {13, 14}, // To Sacred Yards
  {14, 15}, // From Sacred Yards
  {13, 15},
  {-1, -1}, // [Cecil] End
};

static const PathDots_t _aPathDotsTFE[] = {
  // 00: Intro
  { {-1,-1}, },

  // 01: Hatshepsut - Sand
  { {207,435}, {196,440}, {184,444}, {172,443}, {162,439}, {156,432}, {-1,-1}, },

  // 02: Sand - Ramses
  { {115,388}, {121,382}, {128,377}, {136,371}, {-1,-1}, },

  // 03: Ramses - Canyon
  { {148,368}, {159,370}, {169,374}, {177,379}, {187,381}, {200,380}, {211,376}, {-1,-1}, },

  // 04: Canyon - Waterfall
  { {273,339}, {276,331}, {278,322}, {280,313}, {279,305}, {273,298}, {266,293}, {260,288}, {-1,-1}, },

  // 05: Canyon - Oasis
  { {288,360}, {295,355}, {302,360}, {310,364}, {319,367}, {328,368}, {-1,-1}, },

  // 06: Waterfall - Oasis
  { {294,279}, {302,282}, {310,287}, {316,294}, {320,302}, {323,310}, {327,318}, {332,326}, {337,333}, {-1,-1}, },

  // 07: Oasis - Dunes
  { {384,360}, {394,358}, {405,353}, {414,347}, {421,339}, {426,329}, {-1,-1}, },

  // 08: Dunes - Suburbs
  { {439,305}, {434,300}, {429,293}, {-1,-1}, },

  // 09: Suburbs - Sewers
  { {403,250}, {402,244}, {401,238}, {398,232}, {-1,-1}, },

  // 10: Sewers - Metropolis
  { {372,266}, {371,221}, {370,216}, {-1,-1}, },

  // 11: Metropolis - Alley
  { {317,211}, {310,215}, {302,219}, {293,222}, {283,222}, {273,221}, {265,218}, {-1,-1}, },

  // 12: Alley - Karnak
  { {260,189}, {259,181}, {255,174}, {249,168}, {241,165}, {233,164}, {-1,-1}, },

  // 13: Karnak - Luxor
  { {228,143}, {228,136}, {226,129}, {221,123}, {-1,-1}, },

  // 14: Luxor - Sacred
  { {175,101}, {169,106}, {162,111}, {154,113}, {145,113}, {136,112}, {-1,-1}, },

  // 15: Sacred - Pyramid
  { {126,59}, {134,55}, {142,52}, {151,49}, {160,47}, {170,47}, {179,48}, {188,51}, {-1,-1}, },

  // 16: Luxor - Pyramid
  { {212,71}, {217,66}, {225,63}, {234,63}, {244,63}, {253,62}, {261,59}, {-1,-1}, },
};
