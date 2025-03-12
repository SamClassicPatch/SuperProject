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

// Comparison methods for qsort()

static int qsort_CompareNames(const void *ppPEN0, const void *ppPEN1) {
  CPlayer &en0 = **(CPlayer **)ppPEN0;
  CPlayer &en1 = **(CPlayer **)ppPEN1;
  CTString strName0 = en0.GetPlayerName().Undecorated();
  CTString strName1 = en1.GetPlayerName().Undecorated();

  return strnicmp(strName0, strName1, 8);
};

static int qsort_CompareScores(const void *ppPEN0, const void *ppPEN1) {
  CPlayer &en0 = **(CPlayer **)ppPEN0;
  CPlayer &en1 = **(CPlayer **)ppPEN1;
  SLONG sl0 = en0.m_psGameStats.ps_iScore;
  SLONG sl1 = en1.m_psGameStats.ps_iScore;
  return Sgn(sl1 - sl0);
};

static int qsort_CompareHealth(const void *ppPEN0, const void *ppPEN1) {
  CPlayer &en0 = **(CPlayer **)ppPEN0;
  CPlayer &en1 = **(CPlayer **)ppPEN1;
  SLONG sl0 = (SLONG)ceil(en0.GetHealth());
  SLONG sl1 = (SLONG)ceil(en1.GetHealth());
  return Sgn(sl1 - sl0);
};

static int qsort_CompareManas(const void *ppPEN0, const void *ppPEN1) {
  CPlayer &en0 = **(CPlayer **)ppPEN0;
  CPlayer &en1 = **(CPlayer **)ppPEN1;
  SLONG sl0 = en0.m_iMana;
  SLONG sl1 = en1.m_iMana;
  return Sgn(sl1 - sl0);
};

static int qsort_CompareDeaths(const void *ppPEN0, const void *ppPEN1) {
  CPlayer &en0 = **(CPlayer **)ppPEN0;
  CPlayer &en1 = **(CPlayer **)ppPEN1;
  SLONG sl0 = en0.m_psGameStats.ps_iDeaths;
  SLONG sl1 = en1.m_psGameStats.ps_iDeaths;
  return Sgn(sl1 - sl0);
};

static int qsort_CompareFrags(const void *ppPEN0, const void *ppPEN1) {
  CPlayer &en0 = **(CPlayer **)ppPEN0;
  CPlayer &en1 = **(CPlayer **)ppPEN1;
  SLONG sl0 = en0.m_psGameStats.ps_iKills;
  SLONG sl1 = en1.m_psGameStats.ps_iKills;

  if (sl0 < sl1) {
    return +1;
  } else if (sl0 > sl1) {
    return -1;
  }

  return -qsort_CompareDeaths(ppPEN0, ppPEN1);
};
