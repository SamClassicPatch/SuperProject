/* Copyright (c) 2022-2024 Dreamy Cecil
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

#ifndef CECIL_INCL_STREAMBLOCK_H
#define CECIL_INCL_STREAMBLOCK_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// A message block used for streaming data across network
// Reimplementation of Serious Engine's CNetworkStreamBlock
class CORE_API CNetStreamBlock : public CNetworkMessage {
  public:
    CListNode nsb_lnInStream; // Node in list of blocks in stream
    INDEX nsb_iSequenceNumber; // Index for sorting in list

  public:
    // Constructor for receiving - uninitialized block
    CNetStreamBlock(void);

    // Constructor for sending - empty packet with given type and sequence
    CNetStreamBlock(INDEX iType, INDEX iSequenceNumber);

    // Constructor for sending - copied packet with given sequence
    CNetStreamBlock(CNetworkMessage &nmOther, INDEX iSequenceNumber);

    // Read block from a received message
    void ReadFromMessage(CNetworkMessage &nmToRead);

    // Add block to a message to send
    void WriteToMessage(CNetworkMessage &nmToWrite);

    // Remove block from the stream
    void RemoveFromStream(void);

    // Write block into a stream
    void Write_t(CTStream &strm);

    // Read block from a stream
    void Read_t(CTStream &strm);
};

// Stream of message blocks that can be sent across network
// Reimplementation of Serious Engine's CNetworkStream
class CORE_API CNetStream {
  public:
    CListHead ns_lhBlocks; // List of blocks of this stream (higher sequences first)

  public:
    // Add block that's already allocated to the stream
    void AddAllocatedBlock(CNetStreamBlock *pnsbBlock);

    // Add block to the stream (makes a copy of it)
    void AddBlock(CNetStreamBlock &nsbBlock);

    // Remove all blocks with older sequence number
    void RemoveOlderBlocksBySequence(INDEX iLastSequenceToKeep);
};

#endif
