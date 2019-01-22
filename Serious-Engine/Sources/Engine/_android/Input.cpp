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

#include <Engine/StdH.h>

#include <Engine/Base/Timer.h>
#include <Engine/Base/Input.h>
#include <Engine/Base/Translation.h>
#include <Engine/Base/KeyNames.h>
#include <Engine/Math/Functions.h>
#include <Engine/Graphics/ViewPort.h>
#include <Engine/Base/Console.h>
#include <Engine/Base/Synchronization.h>

#include <Engine/Base/ErrorReporting.h>

extern INDEX inp_iKeyboardReadingMethod;
extern FLOAT inp_fMouseSensitivity;
extern INDEX inp_bAllowMouseAcceleration;
extern INDEX inp_bMousePrecision;
extern FLOAT inp_fMousePrecisionFactor;
extern FLOAT inp_fMousePrecisionThreshold;
extern FLOAT inp_fMousePrecisionTimeout;
extern FLOAT inp_bInvertMouse;
extern INDEX inp_bFilterMouse;
extern INDEX inp_bAllowPrescan;

extern INDEX inp_i2ndMousePort;
extern FLOAT inp_f2ndMouseSensitivity;
extern INDEX inp_b2ndMousePrecision;
extern FLOAT inp_f2ndMousePrecisionThreshold;
extern FLOAT inp_f2ndMousePrecisionTimeout;
extern FLOAT inp_f2ndMousePrecisionFactor;
extern INDEX inp_bFilter2ndMouse;
extern INDEX inp_bInvert2ndMouse;

extern INDEX inp_iMButton4Dn = 0x20040;
extern INDEX inp_iMButton4Up = 0x20000;
extern INDEX inp_iMButton5Dn = 0x10020;
extern INDEX inp_iMButton5Up = 0x10000;
extern INDEX inp_bMsgDebugger = FALSE;
extern INDEX inp_bForceJoystickPolling = 0;
extern INDEX inp_ctJoysticksAllowed = 8;
extern INDEX inp_bAutoDisableJoysticks = 0;

static CTString inp_astrAxisTran[MAX_OVERALL_AXES];// translated names for axis

/*

NOTE: Three different types of key codes are used here:
  1) kid - engine internal type - defined in KeyNames.h
  2) scancode - raw PC keyboard scancodes as returned in keydown/keyup messages
  3) virtkey - virtual key codes used by windows

*/

// name that is not translated (international)
#define INTNAME(str) str, ""
// name that is translated
#define TRANAME(str) str, "ETRS" str

// basic key conversion table
static struct KeyConversion {
    INDEX kc_iKID;
    INDEX kc_iVirtKey;
    INDEX kc_iScanCode;
    char *kc_strName;
    char *kc_strNameTrans;
} _akcKeys[] = {

        // reserved for 'no-key-pressed'
        {KID_NONE,           -1,        -1,       TRANAME("None")},

// numbers row              
        {KID_1,              '1',       2,        INTNAME("1")},
        {KID_2,              '2',       3,        INTNAME("2")},
        {KID_3,              '3',       4,        INTNAME("3")},
        {KID_4,              '4',       5,        INTNAME("4")},
        {KID_5,              '5',       6,        INTNAME("5")},
        {KID_6,              '6',       7,        INTNAME("6")},
        {KID_7,              '7',       8,        INTNAME("7")},
        {KID_8,              '8',       9,        INTNAME("8")},
        {KID_9,              '9',       10,       INTNAME("9")},
        {KID_0,              '0',       11,       INTNAME("0")},
        {KID_MINUS,          189,       12,       INTNAME("-")},
        {KID_EQUALS,         187,       13,       INTNAME("=")},

// 1st alpha row            
        {KID_Q,              'Q',       16,       INTNAME("Q")},
        {KID_W,              'W',       17,       INTNAME("W")},
        {KID_E,              'E',       18,       INTNAME("E")},
        {KID_R,              'R',       19,       INTNAME("R")},
        {KID_T,              'T',       20,       INTNAME("T")},
        {KID_Y,              'Y',       21,       INTNAME("Y")},
        {KID_U,              'U',       22,       INTNAME("U")},
        {KID_I,              'I',       23,       INTNAME("I")},
        {KID_O,              'O',       24,       INTNAME("O")},
        {KID_P,              'P',       25,       INTNAME("P")},
        {KID_LBRACKET,       219,       26,       INTNAME("[")},
        {KID_RBRACKET,       221,       27,       INTNAME("]")},
        {KID_BACKSLASH,      220,       43,       INTNAME("\\")},

// 2nd alpha row            
        {KID_A,              'A',       30,       INTNAME("A")},
        {KID_S,              'S',       31,       INTNAME("S")},
        {KID_D,              'D',       32,       INTNAME("D")},
        {KID_F,              'F',       33,       INTNAME("F")},
        {KID_G,              'G',       34,       INTNAME("G")},
        {KID_H,              'H',       35,       INTNAME("H")},
        {KID_J,              'J',       36,       INTNAME("J")},
        {KID_K,              'K',       37,       INTNAME("K")},
        {KID_L,              'L',       38,       INTNAME("L")},
        {KID_SEMICOLON,      186,       39,       INTNAME(";")},
        {KID_APOSTROPHE,     222,       40,       INTNAME("'")},
// 3rd alpha row
        {KID_Z,              'Z',       44,       INTNAME("Z")},
        {KID_X,              'X',       45,       INTNAME("X")},
        {KID_C,              'C',       46,       INTNAME("C")},
        {KID_V,              'V',       47,       INTNAME("V")},
        {KID_B,              'B',       48,       INTNAME("B")},
        {KID_N,              'N',       49,       INTNAME("N")},
        {KID_M,              'M',       50,       INTNAME("M")},
        {KID_COMMA,          190,       51,       INTNAME(",")},
        {KID_PERIOD,         188,       52,       INTNAME(".")},
        {KID_SLASH,          191,       53,       INTNAME("/")},

// row with F-keys                     
        {KID_F1,          VK_F1,        59,       INTNAME("F1")},
        {KID_F2,          VK_F2,        60,       INTNAME("F2")},
        {KID_F3,          VK_F3,        61,       INTNAME("F3")},
        {KID_F4,          VK_F4,        62,       INTNAME("F4")},
        {KID_F5,          VK_F5,        63,       INTNAME("F5")},
        {KID_F6,          VK_F6,        64,       INTNAME("F6")},
        {KID_F7,          VK_F7,        65,       INTNAME("F7")},
        {KID_F8,          VK_F8,        66,       INTNAME("F8")},
        {KID_F9,          VK_F9,        67,       INTNAME("F9")},
        {KID_F10,         VK_F10,       68,       INTNAME("F10")},
        {KID_F11,         VK_F11,       87,       INTNAME("F11")},
        {KID_F12,         VK_F12,       88,       INTNAME("F12")},

// extra keys               
        {KID_ESCAPE,      VK_ESCAPE,    1,        TRANAME("Escape")},
        {KID_TILDE,          -1,        41,       TRANAME("Tilde")},
        {KID_BACKSPACE,   VK_BACK,      14,       TRANAME("Backspace")},
        {KID_TAB,         VK_TAB,       15,       TRANAME("Tab")},
        {KID_CAPSLOCK,    VK_CAPITAL,   58,       TRANAME("Caps Lock")},
        {KID_ENTER,       VK_RETURN,    28,       TRANAME("Enter")},
        {KID_SPACE,       VK_SPACE,     57,       TRANAME("Space")},

// modifier keys                            
        {KID_LSHIFT,      VK_LSHIFT,    42,       TRANAME("Left Shift")},
        {KID_RSHIFT,      VK_RSHIFT,    54,       TRANAME("Right Shift")},
        {KID_LCONTROL,    VK_LCONTROL,  29,       TRANAME("Left Control")},
        {KID_RCONTROL,    VK_RCONTROL,  256 + 29, TRANAME("Right Control")},
        {KID_LALT,        VK_LMENU,     56,       TRANAME("Left Alt")},
        {KID_RALT,        VK_RMENU,     256 + 56, TRANAME("Right Alt")},

// navigation keys          
        {KID_ARROWUP,     VK_UP,        256 + 72, TRANAME("Arrow Up")},
        {KID_ARROWDOWN,   VK_DOWN,      256 + 80, TRANAME("Arrow Down")},
        {KID_ARROWLEFT,   VK_LEFT,      256 + 75, TRANAME("Arrow Left")},
        {KID_ARROWRIGHT,  VK_RIGHT,     256 + 77, TRANAME("Arrow Right")},
        {KID_INSERT,      VK_INSERT,    256 + 82, TRANAME("Insert")},
        {KID_DELETE,      VK_DELETE,    256 + 83, TRANAME("Delete")},
        {KID_HOME,        VK_HOME,      256 + 71, TRANAME("Home")},
        {KID_END,         VK_END,       256 + 79, TRANAME("End")},
        {KID_PAGEUP,      VK_PRIOR,     256 + 73, TRANAME("Page Up")},
        {KID_PAGEDOWN,    VK_NEXT,      256 + 81, TRANAME("Page Down")},
        {KID_PRINTSCR,    VK_SNAPSHOT,  256 + 55, TRANAME("Print Screen")},
        {KID_SCROLLLOCK,  VK_SCROLL,    70,       TRANAME("Scroll Lock")},
        {KID_PAUSE,       VK_PAUSE,     69,       TRANAME("Pause")},

// numpad numbers           
        {KID_NUM0,        VK_NUMPAD0,   82,       INTNAME("Num 0")},
        {KID_NUM1,        VK_NUMPAD1,   79,       INTNAME("Num 1")},
        {KID_NUM2,        VK_NUMPAD2,   80,       INTNAME("Num 2")},
        {KID_NUM3,        VK_NUMPAD3,   81,       INTNAME("Num 3")},
        {KID_NUM4,        VK_NUMPAD4,   75,       INTNAME("Num 4")},
        {KID_NUM5,        VK_NUMPAD5,   76,       INTNAME("Num 5")},
        {KID_NUM6,        VK_NUMPAD6,   77,       INTNAME("Num 6")},
        {KID_NUM7,        VK_NUMPAD7,   71,       INTNAME("Num 7")},
        {KID_NUM8,        VK_NUMPAD8,   72,       INTNAME("Num 8")},
        {KID_NUM9,        VK_NUMPAD9,   73,       INTNAME("Num 9")},
        {KID_NUMDECIMAL,  VK_DECIMAL,   83,       INTNAME("Num .")},

// numpad gray keys         
        {KID_NUMLOCK,     VK_NUMLOCK,   256 + 69, INTNAME("Num Lock")},
        {KID_NUMSLASH,    VK_DIVIDE,    256 + 53, INTNAME("Num /")},
        {KID_NUMMULTIPLY, VK_MULTIPLY,  55,       INTNAME("Num *")},
        {KID_NUMMINUS,    VK_SUBTRACT,  74,       INTNAME("Num -")},
        {KID_NUMPLUS,     VK_ADD,       78,       INTNAME("Num +")},
        {KID_NUMENTER,    VK_SEPARATOR, 256 + 28, TRANAME("Num Enter")},

// mouse buttons
        {KID_MOUSE1,      VK_LBUTTON,   -1,       TRANAME("Mouse Button 1")},
        {KID_MOUSE2,      VK_RBUTTON,   -1,       TRANAME("Mouse Button 2")},
        {KID_MOUSE3,      VK_MBUTTON,   -1,       TRANAME("Mouse Button 3")},
        {KID_MOUSE4,         -1,        -1,       TRANAME("Mouse Button 4")},
        {KID_MOUSE5,         -1,        -1,       TRANAME("Mouse Button 5")},
        {KID_MOUSEWHEELUP,   -1,        -1,       TRANAME("Mouse Wheel Up")},
        {KID_MOUSEWHEELDOWN, -1,        -1,       TRANAME("Mouse Wheel Down")},

// 2nd mouse buttons
        {KID_2MOUSE1,        -1,        -1,       TRANAME("2nd Mouse Button 1")},
        {KID_2MOUSE2,        -1,        -1,       TRANAME("2nd Mouse Button 2")},
        {KID_2MOUSE3,        -1,        -1,       TRANAME("2nd Mouse Button 3")},
};


// autogenerated fast conversion tables
static INDEX _aiScanToKid[512];
static INDEX _aiVirtToKid[256];

// make fast conversion tables from the general table
static void MakeConversionTables(void) {
    // clear conversion tables
    memset(_aiScanToKid, -1, sizeof(_aiScanToKid));
    memset(_aiVirtToKid, -1, sizeof(_aiVirtToKid));

    // for each Key
    for (INDEX iKey = 0; iKey < ARRAYCOUNT(_akcKeys); iKey++) {
        struct KeyConversion &kc = _akcKeys[iKey];

        // get codes
        INDEX iKID = kc.kc_iKID;
        INDEX iScan = kc.kc_iScanCode;
        INDEX iVirt = kc.kc_iVirtKey;

        // update the tables
        if (iScan >= 0) {
            _aiScanToKid[iScan] = iKID;
        }
        if (iVirt >= 0) {
            _aiVirtToKid[iVirt] = iKID;
        }
    }
}

// variables for message interception
static HHOOK _hGetMsgHook = NULL;
static HHOOK _hSendMsgHook = NULL;
static int _iMouseZ = 0;
static BOOL _bWheelUp = FALSE;
static BOOL _bWheelDn = FALSE;

CTCriticalSection csInput;

// which keys are pressed, as recorded by message interception (by KIDs)
static UBYTE _abKeysPressed[256];

// set a key according to a keydown/keyup message
static void SetKeyFromMsg(MSG *pMsg, BOOL bDown) {
    INDEX iKID = -1;
    // if capturing scan codes
    if (inp_iKeyboardReadingMethod == 2) {
        // get scan code
        INDEX iScan = (pMsg->lParam >> 16) & 0x1FF; // (we use the extended bit too!)
        // convert scan code to kid
        iKID = _aiScanToKid[iScan];
        // if capturing virtual key codes
    } else if (inp_iKeyboardReadingMethod == 1) {
        // get virtualkey
        INDEX iVirt = (pMsg->wParam) & 0xFF;

        if (iVirt == VK_SHIFT) {
            iVirt = VK_LSHIFT;
        }
        if (iVirt == VK_CONTROL) {
            iVirt = VK_LCONTROL;
        }
        if (iVirt == VK_MENU) {
            iVirt = VK_LMENU;
        }
        // convert virtualkey to kid
        iKID = _aiVirtToKid[iVirt];
        // if not capturing
    } else {
        // do nothing
        return;
    }
    if (iKID >= 0 && iKID < ARRAYCOUNT(_abKeysPressed)) {
//    CPrintF("%s: %d\n", _pInput->inp_strButtonNames[iKID], bDown);
        _abKeysPressed[iKID] = bDown;
    }
}

static void CheckMessage(MSG *pMsg) {}


// procedure called when message is retreived
LRESULT CALLBACK GetMsgProc(
        int nCode,      // hook code
        WPARAM wParam,  // message identifier
        LPARAM lParam)  // mouse coordinates
{}


// procedure called when message is sent
LRESULT CALLBACK SendMsgProc(
        int nCode,      // hook code
        WPARAM wParam,  // message identifier
        LPARAM lParam)  // mouse coordinates
{}



// --------- 2ND MOUSE HANDLING

#define MOUSECOMBUFFERSIZE 256L
static HANDLE _h2ndMouse = NONE;
static BOOL _bIgnoreMouse2 = TRUE;
static INDEX _i2ndMouseX, _i2ndMouseY, _i2ndMouseButtons;
static INDEX _iByteNum = 0;
static UBYTE _aubComBytes[4] = {0, 0, 0, 0};
static INDEX _iLastPort = -1;


static void Poll2ndMouse(void) {}


static void Startup2ndMouse(INDEX iPort) {}


static void Shutdown2ndMouse(void) {}


// pointer to global input object
CInput *_pInput = NULL;

// deafult constructor
CInput::CInput(void) {}


// destructor
CInput::~CInput() {}


void CInput::SetJoyPolling(BOOL bPoll) {}

/*
 * Sets names of keys on keyboard
 */
void CInput::SetKeyNames(void) {}

// check if a joystick exists
BOOL CInput::CheckJoystick(INDEX iJoy) {}

// adds axis and buttons for given joystick
void CInput::AddJoystickAbbilities(INDEX iJoy) {}

/*
 * Initializes all available devices and enumerates available controls
 */
void CInput::Initialize(void) {}


/*
 * Enable direct input
 */

void CInput::EnableInput(CViewPort *pvp) {}


void CInput::EnableInput(HWND hwnd) {}


/*
 * Disable direct input
 */
void CInput::DisableInput(void) {}


/*
 * Scan states of all available input sources
 */
void CInput::GetInput(BOOL bPreScan) {}

// Clear all input states (keys become not pressed, axes are reset to zero)
void CInput::ClearInput(void) {}

const CTString &CInput::GetAxisTransName(INDEX iAxisNo) const {}


/*
 * Scans axis and buttons for given joystick
 */
BOOL CInput::ScanJoystick(INDEX iJoy, BOOL bPreScan) {}
