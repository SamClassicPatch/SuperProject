/* Copyright (c) 2024 Dreamy Cecil
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

// Define timer events for the plugin

void ITimerEvents_OnTick(void)
{
  // This function is executed once every game tick defined by CTimer::TickQuantum interval.
  // It isn't synchonized with the game simulation and will run at the same rate regardless
  // of the simulation speed and even outside the simulation (e.g. in the menu).

  // For synchronization with game simulation use IProcessingEvents::OnStep() method instead.
};

void ITimerEvents_OnSecond(void)
{
  // This function is executed once every second (after ITimerEvents::OnTick() on the same tick).
  // It isn't synchonized with the game simulation and will run at the same rate regardless
  // of the simulation speed and even outside the simulation (e.g. in the menu).
};
