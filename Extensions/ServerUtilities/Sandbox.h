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

class IServerSandbox {
  public:
    // Scheduled commands to be executed after the world loads
    static CStringStack astrScheduled;

  public:
    // Schedule one command
    static void ScheduleCommand(const CTString &strCommand);

    // List all scheduled commands in order
    static void ListScheduledCommands(void);
    
    // Clear scheduled commands
    static void ClearScheduledCommands(void);

    // Delete an entity from the world
    static void DeleteEntity(SHELL_FUNC_ARGS);

    // Initialize/reinitialize an entity
    static void InitEntity(SHELL_FUNC_ARGS);

    // Set new absolute position of an entity
    static void SetEntityPosition(SHELL_FUNC_ARGS);

    // Set new absolute rotation of an entity
    static void SetEntityRotation(SHELL_FUNC_ARGS);

    // Set new value to some property by its name of an entity
    static void SetEntityProperty(SHELL_FUNC_ARGS);

    // Parent an entity to another entity
    static void ParentEntity(SHELL_FUNC_ARGS);
};
