/* Copyright (c) 2022-2026 Dreamy Cecil
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

#include "StreamBlock.h"

// Constructor for receiving - uninitialized block
CNetStreamBlock::CNetStreamBlock(void) :
  CNetworkMessage(), nsb_iSequenceNumber(-1)
{
};

// Constructor for sending - empty packet with given type and sequence
CNetStreamBlock::CNetStreamBlock(INDEX iType, INDEX iSequenceNumber) :
  CNetworkMessage(MESSAGETYPE(iType)), nsb_iSequenceNumber(iSequenceNumber)
{
};

// Constructor for sending - copied packet with given sequence
CNetStreamBlock::CNetStreamBlock(CNetworkMessage &nmOther, INDEX iSequenceNumber) :
  CNetworkMessage(nmOther), nsb_iSequenceNumber(iSequenceNumber)
{
};

// Read block from a received message
void CNetStreamBlock::ReadFromMessage(CNetworkMessage &nmToRead) {
  // Read sequence number from message
  nmToRead >> nsb_iSequenceNumber;

  // Read the block as a submessage
  nmToRead.ExtractSubMessage(*this);
};

// Add block to a message to send
void CNetStreamBlock::WriteToMessage(CNetworkMessage &nmToWrite) {
  // Write sequence number to message
  nmToWrite << nsb_iSequenceNumber;

  // Write the block as a submessage
  nmToWrite.InsertSubMessage(*this);
};

// Remove block from the stream
void CNetStreamBlock::RemoveFromStream(void) {
  nsb_lnInStream.Remove();
};

// Read block from a stream
void CNetStreamBlock::Write_t(CTStream &strm) {
  // Write sequence number
  strm << nsb_iSequenceNumber;

  // Write block size
  strm << nm_slSize;

  // Write block contents
  strm.Write_t(nm_pubMessage, nm_slSize);
};

// Read block from a stream
void CNetStreamBlock::Read_t(CTStream &strm) {
  // Read sequence number
  strm >> nsb_iSequenceNumber;

  // Read block size
  strm >> nm_slSize;

  // Read block contents
  strm.Read_t(nm_pubMessage, nm_slSize);

  // Init the message read/write pointer
  nm_pubPointer = nm_pubMessage;
  nm_iBit = 0;

  // Get the message type
  UBYTE ubType = 0;
  *this >> ubType;

  nm_mtType = (MESSAGETYPE)ubType;
};

// Add block that's already allocated to the stream
void CNetStream::AddAllocatedBlock(CNetStreamBlock *pnsbBlock) {
  // Preserve iterator for later use
  LISTITER(CNetStreamBlock, nsb_lnInStream) itnsbInList(ns_lhBlocks);

  // Search all blocks already in list
  for (; !itnsbInList.IsPastEnd(); itnsbInList.MoveToNext())
  {
    // If the block in list has same sequence as the one to add
    if (itnsbInList->nsb_iSequenceNumber == pnsbBlock->nsb_iSequenceNumber) {
      // Just discard the new block
      delete pnsbBlock;
      return;
    }

    // If the block in list has lower sequence than the one to add
    if (itnsbInList->nsb_iSequenceNumber < pnsbBlock->nsb_iSequenceNumber) {
      // Stop searching
      break;
    }
  }

  // Add the new block before current one
  itnsbInList.InsertBeforeCurrent(pnsbBlock->nsb_lnInStream);
};

// Add block to the stream (makes a copy of it)
void CNetStream::AddBlock(CNetStreamBlock &nsbBlock) {
  // Create a copy of the block
  CNetStreamBlock *pnsbCopy = new CNetStreamBlock(nsbBlock);

  // Shrink it
  pnsbCopy->Shrink();

  // Add it to the list
  AddAllocatedBlock(pnsbCopy);
};

// Remove all blocks with older sequence number
void CNetStream::RemoveOlderBlocksBySequence(INDEX iLastSequenceToKeep) {
  // Remove from the tail as long as it's not too old
  while (!ns_lhBlocks.IsEmpty()) {
    CNetworkStreamBlock *pnsb = LIST_TAIL(ns_lhBlocks, CNetworkStreamBlock, nsb_lnInStream);

    if (pnsb->nsb_iSequenceNumber >= iLastSequenceToKeep) {
      break;
    }

    delete pnsb;
  }
};
