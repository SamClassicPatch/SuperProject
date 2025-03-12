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

// Affect weapon item at the beginning of the game
void AffectWeaponItem(CEntity *pen);

// Affect ammo item at the beginning of the game
void AffectAmmoItem(CEntity *pen);

// Affect health item at the beginning of the game
void AffectHealthItem(CEntity *pen);

// Affect armor item at the beginning of the game
void AffectArmorItem(CEntity *pen);

// Affect power up item at the beginning of the game
void AffectPowerUpItem(CEntity *pen);

// Affect player start marker at the beginning of the game
void AffectPlayerMarker(CEntity *pen);

// Affect enemy spawner at the beginning of the game
void AffectEnemySpawner(CEntity *pen);
