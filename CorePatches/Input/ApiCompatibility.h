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

#ifndef CECIL_INCL_INPUTAPICOMPATIBILITY_H
#define CECIL_INCL_INPUTAPICOMPATIBILITY_H

// New Win32 messages
#define WM_MOUSEWHEEL    0x020A
#define WM_XBUTTONDOWN   0x020B
#define WM_XBUTTONUP     0x020C
#define WM_XBUTTONDBLCLK 0x020D

#define GET_XBUTTON_WPARAM(wParam) (HIWORD(wParam))
#define XBUTTON1 0x0001
#define XBUTTON2 0x0002

#define VK_XBUTTON1 0x05
#define VK_XBUTTON2 0x06

#define WM_CTRLAXISMOTION 0x100000
#define WM_CTRLBUTTONDOWN 0x100001
#define WM_CTRLBUTTONUP   0x100002

// These exist purely for recognizing mouse buttons in Win32 input logic
// and should NOT be used akin to Windows' VK_*BUTTON macros!
#define SE1K_LBUTTON      VK_LBUTTON
#define SE1K_RBUTTON      VK_RBUTTON
#define SE1K_MBUTTON      VK_MBUTTON
#define SE1K_XBUTTON1     VK_XBUTTON1
#define SE1K_XBUTTON2     VK_XBUTTON2

#define SE1K_1            '1'
#define SE1K_2            '2'
#define SE1K_3            '3'
#define SE1K_4            '4'
#define SE1K_5            '5'
#define SE1K_6            '6'
#define SE1K_7            '7'
#define SE1K_8            '8'
#define SE1K_9            '9'
#define SE1K_0            '0'
#define SE1K_a            'A'
#define SE1K_b            'B'
#define SE1K_c            'C'
#define SE1K_d            'D'
#define SE1K_e            'E'
#define SE1K_f            'F'
#define SE1K_g            'G'
#define SE1K_h            'H'
#define SE1K_i            'I'
#define SE1K_j            'J'
#define SE1K_k            'K'
#define SE1K_l            'L'
#define SE1K_m            'M'
#define SE1K_n            'N'
#define SE1K_o            'O'
#define SE1K_p            'P'
#define SE1K_q            'Q'
#define SE1K_r            'R'
#define SE1K_s            'S'
#define SE1K_t            'T'
#define SE1K_u            'U'
#define SE1K_v            'V'
#define SE1K_w            'W'
#define SE1K_x            'X'
#define SE1K_y            'Y'
#define SE1K_z            'Z'
#define SE1K_MINUS        0xBD
#define SE1K_EQUALS       0xBB
#define SE1K_COMMA        0xBC
#define SE1K_PERIOD       0xBE
#define SE1K_SLASH        0xBF
#define SE1K_SEMICOLON    0xBA
#define SE1K_QUOTE        0xDE
#define SE1K_ESCAPE       VK_ESCAPE
#define SE1K_TAB          VK_TAB
#define SE1K_CAPSLOCK     VK_CAPITAL
#define SE1K_RETURN       VK_RETURN
#define SE1K_BACKSPACE    VK_BACK
#define SE1K_SPACE        VK_SPACE
#define SE1K_UP           VK_UP
#define SE1K_DOWN         VK_DOWN
#define SE1K_LEFT         VK_LEFT
#define SE1K_RIGHT        VK_RIGHT
#define SE1K_INSERT       VK_INSERT
#define SE1K_DELETE       VK_DELETE
#define SE1K_HOME         VK_HOME
#define SE1K_END          VK_END
#define SE1K_PAGEUP       VK_PRIOR
#define SE1K_PAGEDOWN     VK_NEXT
#define SE1K_PRINTSCREEN  VK_SNAPSHOT
#define SE1K_SCROLLLOCK   VK_SCROLL
#define SE1K_PAUSE        VK_PAUSE
#define SE1K_F1           VK_F1
#define SE1K_F2           VK_F2
#define SE1K_F3           VK_F3
#define SE1K_F4           VK_F4
#define SE1K_F5           VK_F5
#define SE1K_F6           VK_F6
#define SE1K_F7           VK_F7
#define SE1K_F8           VK_F8
#define SE1K_F9           VK_F9
#define SE1K_F10          VK_F10
#define SE1K_F11          VK_F11
#define SE1K_F12          VK_F12
#define SE1K_BACKQUOTE    0xC0
#define SE1K_LEFTBRACKET  0xDB
#define SE1K_RIGHTBRACKET 0xDD
#define SE1K_BACKSLASH    0xE2
#define SE1K_LSHIFT       VK_LSHIFT
#define SE1K_RSHIFT       VK_RSHIFT
#define SE1K_LCTRL        VK_LCONTROL
#define SE1K_RCTRL        VK_RCONTROL
#define SE1K_LALT         VK_LMENU
#define SE1K_RALT         VK_RMENU
#define SE1K_KP_0         VK_NUMPAD0
#define SE1K_KP_1         VK_NUMPAD1
#define SE1K_KP_2         VK_NUMPAD2
#define SE1K_KP_3         VK_NUMPAD3
#define SE1K_KP_4         VK_NUMPAD4
#define SE1K_KP_5         VK_NUMPAD5
#define SE1K_KP_6         VK_NUMPAD6
#define SE1K_KP_7         VK_NUMPAD7
#define SE1K_KP_8         VK_NUMPAD8
#define SE1K_KP_9         VK_NUMPAD9
#define SE1K_KP_PERIOD    VK_DECIMAL
#define SE1K_NUMLOCKCLEAR VK_NUMLOCK
#define SE1K_KP_DIVIDE    VK_DIVIDE
#define SE1K_KP_MULTIPLY  VK_MULTIPLY
#define SE1K_KP_MINUS     VK_SUBTRACT
#define SE1K_KP_PLUS      VK_ADD
#define SE1K_KP_ENTER     VK_SEPARATOR

#endif // include-once check
