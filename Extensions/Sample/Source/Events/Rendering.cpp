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

#include "StdH.h"

// Define rendering events for the plugin

void IRenderingEvents_OnPreDraw(CDrawPort *pdp)
{
  // This function is executed before drawing all of the game elements, like the world, HUD and NETRICSA.
};

void IRenderingEvents_OnPostDraw(CDrawPort *pdp)
{
  // This function is executed after drawing all of the game elements, like the world, HUD and NETRICSA.

  // EXAMPLE: Display counter of received extension packets
  const FLOAT fScaling = HEIGHT_SCALING(pdp);

  pdp->SetFont(_pfdDisplayFont);
  pdp->SetTextScaling(fScaling);

  extern INDEX _ctPacketsReceived;

  CTString strMessage;
  strMessage.PrintF("Packets received: %d", _ctPacketsReceived);

  pdp->PutText(strMessage, 16 * fScaling, 64 * fScaling, C_WHITE | 255);
};

void IRenderingEvents_OnRenderView(CWorld &wo, CEntity *penViewer, CAnyProjection3D &apr, CDrawPort *pdp)
{
  // This function is executed after rendering the world through global RenderView() method.
};
