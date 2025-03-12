/* Copyright (c) 2024-2025 Dreamy Cecil
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

#ifndef CECIL_INCL_VOTINGTYPES_H
#define CECIL_INCL_VOTINGTYPES_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "ActiveClients.h"

// One map in the pool
struct SVoteMap {
  CTFileName fnmWorld;
  CTString strName;

  // Clear map
  void Clear(void) {
    fnmWorld.Clear();
    strName.Clear();
  };

  // Assignment operator
  SVoteMap &operator=(const SVoteMap &mapOther) {
    fnmWorld = mapOther.fnmWorld;
    strName = mapOther.strName;
    return *this;
  };
};

// Current vote
class CORE_API CGenericVote {
  public:
    CTimerValue vt_tvUntil; // When voting ends
    CTimerValue vt_tvReport; // When to remind about voting

    CActiveClient::List vt_Yes; // Clients who voted for
    CActiveClient::List vt_No;  // Clients who voted against

    BOOL vt_bOver; // Voting is over

  public:
    // Constructor
    CGenericVote() {
      vt_tvUntil.Clear();
      vt_tvReport.Clear();
      vt_bOver = FALSE;
    };

    // Copy constructor
    CGenericVote(const CGenericVote &vtOther) :
      vt_tvUntil(vtOther.vt_tvUntil), vt_tvReport(vtOther.vt_tvReport), vt_bOver(vtOther.vt_bOver)
    {
    };

    // Destructor
    virtual ~CGenericVote()
    {
    };

    // Check how much time is left to vote
    CTimerValue GetTimeLeft(void) const;

    // Check how much time is left before the next report
    CTimerValue GetReportTimeLeft(void) const;

    // Set how long the vote is going to go for
    void SetTime(CTimerValue tvTime);

    // Set when to remind about voting next time
    void SetReportTime(void);

  public:
    // Make copy of this class
    virtual CGenericVote *MakeCopy(void) const = 0;

    // Vote description
    virtual CTString VoteMessage(void) const = 0;

    // Vote result
    virtual CTString ResultMessage(void) const = 0;

    // Perform action after voting
    virtual void VotingOver(void) = 0;
};

// Vote to change current map
class CORE_API CMapVote : public CGenericVote {
  public:
    SVoteMap vt_map; // Voting for this map from the pool

  public:
    // Constructor from a pool map
    CMapVote(const SVoteMap &map = SVoteMap()) : CGenericVote(), vt_map(map)
    {
    };

    // Copy constructor
    CMapVote(const CMapVote &vtOther) : CGenericVote(vtOther), vt_map(vtOther.vt_map)
    {
    };

    virtual CGenericVote *MakeCopy(void) const {
      return new CMapVote(*this);
    };

    virtual CTString VoteMessage(void) const;
    virtual CTString ResultMessage(void) const;
    virtual void VotingOver(void);
};

// Vote to perform some action on a client
class CORE_API IClientVote : public CGenericVote {
  public:
    CClientIdentity *vt_pciIdentity; // Client identity to perform an action on
    CTString vt_strPlayers; // Client's players

  public:
    // Constructor from an active client
    IClientVote(CActiveClient &ac);

    // Copy constructor
    IClientVote(const IClientVote &vtOther);
};

// Vote to kick a client
class CORE_API CKickVote : public IClientVote {
  public:
    __forceinline CKickVote(CActiveClient &ac) : IClientVote(ac)
    {
    };

    __forceinline CKickVote(const IClientVote &vtOther) : IClientVote(vtOther)
    {
    };

    virtual CGenericVote *MakeCopy(void) const {
      return new CKickVote(*this);
    };

    virtual CTString VoteMessage(void) const;
    virtual CTString ResultMessage(void) const;
    virtual void VotingOver(void);
};

// Vote to mute a client
class CORE_API CMuteVote : public IClientVote {
  public:
    __forceinline CMuteVote(CActiveClient &ac) : IClientVote(ac)
    {
    };

    __forceinline CMuteVote(const IClientVote &vtOther) : IClientVote(vtOther)
    {
    };

    virtual CGenericVote *MakeCopy(void) const {
      return new CMuteVote(*this);
    };

    virtual CTString VoteMessage(void) const;
    virtual CTString ResultMessage(void) const;
    virtual void VotingOver(void);
};

// Vote to skip current round on a dedicated server
class CORE_API CSkipRoundVote : public CGenericVote {
  public:
    virtual CGenericVote *MakeCopy(void) const {
      return new CSkipRoundVote(*this);
    };

    virtual CTString VoteMessage(void) const;
    virtual CTString ResultMessage(void) const;
    virtual void VotingOver(void);
};

#endif
