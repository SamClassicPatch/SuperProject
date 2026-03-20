/* Copyright (c) 2026 Dreamy Cecil
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

#ifndef CECIL_INCL_RAWDATABUFFER_H
#define CECIL_INCL_RAWDATABUFFER_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Buffer class for storing simple data as raw bytes in a fixed-size array
class CRawDataBuffer {
  public:
    CStaticArray<UBYTE> aData;

  public:
    virtual ~CRawDataBuffer() {};

    // Compare with another buffer
    inline bool operator==(const CRawDataBuffer &other) const {
      const INDEX ct = aData.Count();
      if (ct != other.aData.Count()) return false; // Mismatching data
      if (ct == 0) return true; // Both empty

      return memcmp(aData.sa_Array, other.aData.sa_Array, ct) == 0;
    };

    // Copy bytes from another buffer
    inline CRawDataBuffer &operator=(const CRawDataBuffer &other) {
      if (&other == this) return *this;
      aData.CopyArray(other.aData);
      return *this;
    };

    // Create a new array of bytes
    virtual void New(INDEX ct) {
      aData.Clear();
      aData.New(ct);
      Reset();
    };

    // Reset all bytes to zero
    virtual void Reset(void) {
      const INDEX ct = aData.Count();

      for (INDEX i = 0; i < ct; i++) {
        aData[i] = 0;
      }
    };

  public:
    // Callbacks for signaling whenever data is being retrieved and set
    virtual bool AfterGet(INDEX iOffset, INDEX iSize) const { return true; };
    virtual bool AfterSet(INDEX iOffset, INDEX iSize) { return true; };

    template<class Type> inline
    bool GetRawData(INDEX iOffset, Type &val) const {
      const INDEX iMaxOffset = aData.Count() - sizeof(Type);
      if (iOffset < 0 || iOffset > iMaxOffset) return false; // Out of bounds or not enough space (if iMaxOffset < 0)

      val = *reinterpret_cast<const Type *>(aData.sa_Array + iOffset);
      return AfterGet(iOffset, sizeof(Type));
    };

    template<class Type> inline
    bool SetRawData(INDEX iOffset, Type val) {
      const INDEX iMaxOffset = aData.Count() - sizeof(Type);
      if (iOffset < 0 || iOffset > iMaxOffset) return false; // Out of bounds or not enough space (if iMaxOffset < 0)

      *reinterpret_cast<Type *>(aData.sa_Array + iOffset) = val;
      return AfterSet(iOffset, sizeof(Type));
    };

    __forceinline bool GetByte (INDEX iOffset, UBYTE &val) const { return GetRawData(iOffset, val); };
    __forceinline bool GetBool (INDEX iOffset, BOOL  &val) const { return GetRawData(iOffset, val); };
    __forceinline bool GetIndex(INDEX iOffset, INDEX &val) const { return GetRawData(iOffset, val); };
    __forceinline bool GetFloat(INDEX iOffset, FLOAT &val) const { return GetRawData(iOffset, val); };

    __forceinline bool SetByte (INDEX iOffset, UBYTE val) { return SetRawData(iOffset, val); };
    __forceinline bool SetBool (INDEX iOffset, BOOL  val) { return SetRawData(iOffset, val); };
    __forceinline bool SetIndex(INDEX iOffset, INDEX val) { return SetRawData(iOffset, val); };
    __forceinline bool SetFloat(INDEX iOffset, FLOAT val) { return SetRawData(iOffset, val); };

    inline bool GetString(INDEX iOffset, CTString &str, INDEX ctMaxChars) const {
      if (ctMaxChars < 0) return false; // Invalid length
      const INDEX iMaxOffset = aData.Count() - ctMaxChars;
      if (iOffset < 0 || iOffset > iMaxOffset) return false; // Out of bounds or not enough space (if iMaxOffset < 0)

      // Empty string
      if (ctMaxChars == 0) {
        str = "";
        return true;
      }

      // Copy string bytes into a null-terminated buffer
      char *pChars = (char *)alloca(ctMaxChars + 1);
      pChars[ctMaxChars] = '\0';
      memcpy(pChars, aData.sa_Array + iOffset, ctMaxChars);

      str = pChars;
      return AfterGet(iOffset, ctMaxChars);
    };

    inline bool SetString(INDEX iOffset, const CTString &str) {
      const INDEX ctLen = str.Length();
      const INDEX iMaxOffset = aData.Count() - ctLen;
      if (iOffset < 0 || iOffset > iMaxOffset) return false; // Out of bounds or not enough space (if iMaxOffset < 0)

      memcpy(aData.sa_Array + iOffset, str.str_String, ctLen);
      return AfterSet(iOffset, ctLen);
    };
};

#endif
