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

// [Cecil] Definitions of unexported CPlayerAction methods from the engine

#ifndef CECIL_INCL_PLAYERACTION_DEFS_H
#define CECIL_INCL_PLAYERACTION_DEFS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include <CoreLib/Networking/MessageCompression.h>

#if _PATCHCONFIG_FUNC_DEFINITIONS

// Write player action into a network packet
CNetworkMessage &operator<<(CNetworkMessage &nm, const CPlayerAction &pa) {
  nm.Write(&pa.pa_llCreated, sizeof(pa.pa_llCreated));

  const ULONG *pul = (const ULONG *)&pa.pa_vTranslation;

  for (INDEX i = 0; i < 9; i++)
  {
    if (*pul == 0) {
      UBYTE ub = 0;
      nm.WriteBits(&ub, 1);

    } else {
      UBYTE ub = 1;
      nm.WriteBits(&ub, 1);
      nm.WriteBits(pul, 32);
    }

    pul++;
  }

  INetCompress::Integer(nm, pa.pa_ulButtons);
  return nm;
};

// Read player action from a network packet
CNetworkMessage &operator>>(CNetworkMessage &nm, CPlayerAction &pa) {
  nm.Read(&pa.pa_llCreated, sizeof(pa.pa_llCreated));

  ULONG *pul = (ULONG *)&pa.pa_vTranslation;

  for (INDEX i = 0; i < 9; i++) {
    UBYTE ub = 0;
    nm.ReadBits(&ub, 1);

    if (ub == 0) {
      *pul = 0;
    } else {
      nm.ReadBits(pul, 32);
    }

    pul++;
  }

  INetDecompress::Integer(nm, pa.pa_ulButtons);
  return nm;
};

// Write player action into a stream
CTStream &operator<<(CTStream &strm, const CPlayerAction &pa) {
  strm.Write_t(&pa, sizeof(pa));
  return strm;
};

// Read player action from a stream
CTStream &operator>>(CTStream &strm, CPlayerAction &pa) {
  strm.Read_t(&pa, sizeof(pa));
  return strm;
};

#endif // _PATCHCONFIG_FUNC_DEFINITIONS

#endif
