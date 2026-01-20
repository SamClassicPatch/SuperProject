/* Copyright (c) 2023-2026 Dreamy Cecil
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

#ifndef SE_GAME_SIMPLECONFIGS_H
#define SE_GAME_SIMPLECONFIGS_H

class CSimpleConfig {
  public:
    typedef void (*ProcessorFunc)(CSimpleConfig &cfg, CTString &strLine);

    struct PropertyProcessor {
      CTString strProp;
      ProcessorFunc pFunc;
    };

  public:
    CTString _strConfig;
    CTFileStream _strmFile;
    INDEX _ctLine;

    CStaticStackArray<PropertyProcessor> _aProcessors;

  public:
    CSimpleConfig() : _ctLine(0)
    {
    };

    CSimpleConfig(const CTString &strConfigFile) {
      Open_t(strConfigFile);
    };

    void AddProcessor(const char *strProp, ProcessorFunc pFunc);
    ProcessorFunc FindProcessor(const char *strProp);

    void Open_t(const CTString &strConfigFile);
    void Close(void);

    CTString GetNonEmptyLine_t(void);
    void SkipBlock_t(void);

  public:
    virtual BOOL ProcessProperty(const CTString &strProp, CTString &strValue);
    virtual void Parse_t(void);
};

class CModelConfig : public CSimpleConfig {
  public:
    CModelObject *_pmo; // CModelObject to set from the config
    CAttachmentModelObject *_pamo; // If this CModelObject is an attachment of another model

    BOOL _bPreview;

  public:
    CModelConfig() : CSimpleConfig(), _pmo(NULL), _pamo(NULL), _bPreview(FALSE)
    {
    };

    CModelConfig(const CTString &strConfigFile) :
      CSimpleConfig(strConfigFile), _pmo(NULL), _pamo(NULL), _bPreview(FALSE)
    {
    };

    CModelConfig(const CModelConfig &cfgOther) :
      _pmo(cfgOther._pmo), _pamo(cfgOther._pamo), _bPreview(cfgOther._bPreview)
    {
    };

  public:
    virtual BOOL ProcessProperty(const CTString &strProp, CTString &strValue);

    static BOOL SetModel(CModelConfig &cfg, const CTString &strConfigFile, CTString &strName);
};

#endif
