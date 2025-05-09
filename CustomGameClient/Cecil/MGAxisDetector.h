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

#ifndef CECIL_INCL_AXISDETECTOR_GADGET_H
#define CECIL_INCL_AXISDETECTOR_GADGET_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "GUI/Components/MGButton.h"

// Button for detecting axis input
class CMGAxisDetector : public CMGButton {
  public:
    BOOL mg_bDetecting;

    CMGAxisDetector();
    virtual void OnActivate(void);
    virtual void Think(void);
    virtual void Render(CDrawPort *pdp);

    // Select detected axis
    void FinishDetection(INDEX iSetAxis);
};

#endif
