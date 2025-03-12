/* Copyright (c) 2002-2012 Croteam Ltd.
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

#include "StdH.h"
#include "MGHighScore.h"

#define HSCOLUMNS 6
CTString strHighScores[HIGHSCORE_COUNT + 1][HSCOLUMNS];
FLOAT afI[HSCOLUMNS] = {
  0.12f, 0.15f, 0.6f, 0.7f, 0.78f, 0.9f
};

void CMGHighScore::Render(CDrawPort *pdp) {
  SetFontMedium(pdp, 1.0f);

  COLOR colHeader = _pGame->LCDGetColor(C_GREEN | 255, "hiscore header");
  COLOR colData = _pGame->LCDGetColor(C_mdGREEN | 255, "hiscore data");
  COLOR colLastSet = _pGame->LCDGetColor(C_mlGREEN | 255, "hiscore last set");
  INDEX iLastSet = GetGameAPI()->GetLastSetHighScore();

  CTString strText;

  strHighScores[0][0] = LOCALIZE("No.");
  strHighScores[0][1] = LOCALIZE("Player Name");
  strHighScores[0][2] = LOCALIZE("Difficulty");
  strHighScores[0][3] = LOCALIZE("Time");
  strHighScores[0][4] = LOCALIZE("Kills");
  strHighScores[0][5] = LOCALIZE("Score");

  const INDEX ctDiffs = ClassicsModData_CountNamedDiffs();

  {for (INDEX i = 0; i < HIGHSCORE_COUNT; i++) {
    CHighScoreEntry &hse = *GetGameAPI()->GetHighScore(i);

    // [Cecil] +1 because Tourist difficulty is -1
    INDEX iDifficulty = hse.hse_gdDifficulty + 1;

    // [Cecil] Invalid difficulty
    if (iDifficulty < 0 || iDifficulty >= ctDiffs) {
      strHighScores[i + 1][1] = "---";
      continue;

    } else {
      // [Cecil] Get difficulty name from the API
      strHighScores[i + 1][2] = ClassicsModData_GetDiff(iDifficulty)->m_strName;
    }

    strHighScores[i + 1][0].PrintF("%d", i + 1);
    strHighScores[i + 1][1] = hse.hse_strPlayer;
    strHighScores[i + 1][3] = TimeToString(hse.hse_tmTime);
    strHighScores[i + 1][4].PrintF("%03d", hse.hse_ctKills);
    strHighScores[i + 1][5].PrintF("%9d", hse.hse_ctScore);
  }}

  PIX pixJ = pdp->GetHeight() * 0.25f;
  {for (INDEX iRow = 0; iRow < HIGHSCORE_COUNT + 1; iRow++) {
    COLOR col = (iRow == 0) ? colHeader : colData;
    if (iLastSet != -1 && iRow - 1 == iLastSet) {
      col = colLastSet;
    }
    {for (INDEX iColumn = 0; iColumn < HSCOLUMNS; iColumn++) {
      PIX pixI = pdp->GetWidth() * afI[iColumn];
      if (iColumn == 1) {
        pdp->PutText(strHighScores[iRow][iColumn], pixI, pixJ, col);
      } else {
        pdp->PutTextR(strHighScores[iRow][iColumn], pixI, pixJ, col);
      }
    }}
    if (iRow == 0) {
      pixJ += pdp->GetHeight() * 0.06f;
    } else {
      pixJ += pdp->GetHeight() * 0.04f;
    }
  }}
}