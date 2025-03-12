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

#include "StdAfx.h"

#include "CompMessage.h"

extern CTString _strStatsDetails;

CCompMessage::CCompMessage(void)
{
  Clear();
}
void CCompMessage::Clear(void)
{
  UnprepareMessage();
  cm_fnmFileName.Clear();
  cm_pcmiOriginal = NULL;
  cm_bRead = FALSE;
}

// constructs message with a filename
void CCompMessage::SetMessage(CCompMessageID *pcmi)
{
  cm_fnmFileName = pcmi->cmi_fnmFileName;
  cm_bRead = pcmi->cmi_bRead;
  cm_pcmiOriginal = pcmi;
}

// load a message from file
void CCompMessage::Load_t(void)
{
  // if already loaded
  if (cm_bLoaded) {
    // do nothing
    return;
  }
  // open file
  CTFileStream strm;
  strm.Open_t(cm_fnmFileName);
  // read subject line
  strm.ExpectKeyword_t("SUBJECT\r\n");
  strm.GetLine_t(cm_strSubject);
  // rea image type
  strm.ExpectKeyword_t("IMAGE\r\n");
  CTString strImage;
  strm.GetLine_t(strImage);
  if (strImage=="none") {
    cm_itImage = IT_NONE;
  } else if (strImage=="statistics") {
    cm_itImage = IT_STATISTICS;
  } else if (strImage=="picture") {
    cm_itImage = IT_PICTURE;
    cm_fnmPicture.ReadFromText_t(strm);
  } else if (strImage=="model") {
    cm_itImage = IT_MODEL;
    cm_strModel.ReadFromText_t(strm, "");
  } else {
    throw LOCALIZE("Unknown image type!");
  }
  // read text until end of file
  strm.ExpectKeyword_t("TEXT\r\n");
  cm_strText.ReadUntilEOF_t(strm);
  cm_ctFormattedLines = 0;
  cm_strFormattedText = "";
  cm_bLoaded = TRUE;
}

// [Cecil] Format message based on text width
void CCompMessage::Format(CDrawPort *pdp, PIX pixMaxWidth) {
  if (cm_ctFormattedLines > 0) {
    return;
  }

  cm_strFormattedText = "";
  cm_ctFormattedLines = 1;

  // Get stats
  if (strncmp(cm_strText, "$STAT", 5) == 0) {
    cm_strFormattedText = _strStatsDetails;

    // Count line breaks
    INDEX ct = cm_strFormattedText.Length();

    for (INDEX i = 0; i < ct; i++) {
      if (cm_strFormattedText[i] == '\n') {
        cm_ctFormattedLines++;
      }
    }
    return;
  }

  // Find last character that fits
  CTString str = cm_strText;
  INDEX i = IData::TextFitsInWidth(pdp, pixMaxWidth, str);
  INDEX ct = cm_strText.Length();

  // If it's not at the end
  while (i < ct) {
    // Go back until a certain word delimiter
    INDEX iDelimiter = i;

    while (--iDelimiter >= 0) {
      char ch = str[iDelimiter];

      // If found a suitable delimiter
      switch (ch) {
        case ' ': case '\n': case '\t': case '\r':
          // Get the character after it and terminate the loop
          i = iDelimiter + 1;
          iDelimiter = 0;
          break;
      }
    }

    // Get part that fits and save the rest
    CTString strPart;
    str.Split(i, strPart, str);

    // Add this part and go to a new line
    cm_strFormattedText += strPart + "\n";

    // Find next last character of that fits
    i = IData::TextFitsInWidth(pdp, pixMaxWidth, str);
    ct = str.Length();
  }

  // Add the rest of the string
  cm_strFormattedText += str;

  // Count line breaks
  ct = cm_strFormattedText.Length();

  for (i = 0; i < ct; i++) {
    if (cm_strFormattedText[i] == '\n') {
      cm_ctFormattedLines++;
    }
  }
};

// [Cecil] Prepare message for using by just loading it
void CCompMessage::PrepareMessage(void)
{
  // if not loaded
  if (!cm_bLoaded) {
    // try to
    try {
      // load it
      Load_t();
    // if failed
    } catch (char *strError) {
      // report warning
      CPrintF("Cannot load message'%s': %s\n", (const CTString &)cm_fnmFileName, strError);
      // do nothing else
      return;
    }
  }
}

// free memory used by message, but keep message filename
void CCompMessage::UnprepareMessage(void)
{
  // clear everything except filename
  cm_bLoaded = FALSE;
  cm_strSubject.Clear();
  cm_strText.Clear();
  cm_strModel.Clear();
  cm_fnmPicture.Clear();
  cm_itImage = IT_NONE;
  cm_strFormattedText.Clear();
  cm_ctFormattedLines = 0;
}
// mark message as read
void CCompMessage::MarkRead(void)
{
  cm_bRead = TRUE;
  cm_pcmiOriginal->cmi_bRead = TRUE;
}

// get one formatted line
CTString CCompMessage::GetLine(INDEX iLine)
{
  const char *strText = cm_strFormattedText;
  // find first line
  INDEX i = 0; 
  while (i<iLine) {
    strText = strchr(strText, '\n');
    if (strText==NULL) {
      return "";
    } else {
      i++;
      strText++;
    }
  }
  // find end of line
  CTString strLine = strText;
  char *pchEndOfLine = (char *)strchr(strLine, '\n');
  // if found
  if (pchEndOfLine!=NULL) {
    // cut there
    *pchEndOfLine = 0;
  }
  return strLine;
}
