#include "StdH.h"
#include <Engine/Base/ErrorReporting.h>
#include "Engine/Graphics/Font.h"
#include "Engine/VirtualKeyboard/VirtualKeyboard.h"
#include "Engine/Graphics/DrawPort.h"
#include "Engine/Graphics/Color.h"

#define C_KEYBOARD_BACK    0x22222200L | CT_OPAQUE
#define C_KEY_BACK    0x44444400L | CT_OPAQUE

/*
 * special keys:
 * \1: OK
 * \2: hide keyboard
 * \3: Caps lock
 * \4: Backspace
 * \5: Tab
 */

struct LayoutRow {
    int numElements;
    const char *keys;
    const char *keysUpper;
};

const LayoutRow layouts[1][4] = {
  {
    // standard
    {10, "qwertyuiop", "QWERTYUIOP"},
    {9, "asdfghjkl", "ASDFGHJKL"},
    {9, "\3zxcvbnm\4", "\3ZXCVBNM\4"},
    {9, "\2\5/ \1"},
  },
};

MSG CVirtualKeyboard::DoKeyboard(CDrawPort *pdpParent) {
  currentMsg.message = 0;

  CDrawPort pdp(pdpParent, 0.0, 1.0 - keyboardHeight, 1.0, keyboardHeight);
  if (!pdp.Lock()) return currentMsg;
  if (!fontData) fontData = _pfdConsoleFont;

  // init some field
  keySizeW = pdp.GetWidth() / 10;
  keySizeH = pdp.GetHeight() / 4;
  padding = Min(keySizeW, keySizeH) / 10;

  pdp.SetFont(fontData);
  pdp.SetTextScaling(Min(keySizeW * 1.5f, (float) keySizeH) / fontData->fd_pixCharHeight * 0.5f);

  pdp.Fill(C_KEYBOARD_BACK);

  PIX posY = 0;

  const LayoutRow *layout = &layouts[selectedLayout][0];
  for (int i = 0; i < 4; i++) {
    const LayoutRow &row = layout[i];
    PIX posX = (pdp.GetWidth() - keySizeW * row.numElements) / 2;

    for (int j = 0;; j++) {
      char key = row.keys[j];
      if (isCapsLock && row.keysUpper) key = row.keysUpper[j];
      if (!key) break;
      int size = keySizeW;

      if (key == 3 || key == 2) {
        size += posX;
        posX = 0;
      }
      if (key == 4) {
        size = pdp.GetWidth() - posX;
      }
      if (key == ' ') {
        size = keySizeW * 3;
      }

      PutKey(pdp, key, posX, posY, size, keySizeH);
      posX += size;
    }

    posY += keySizeH;
  }

  pdp.Unlock();

  clickX = -1;
  clickY = -1;
  return currentMsg;
}

void CVirtualKeyboard::PutKey(const CDrawPort &pdp, char key, int posX, PIX posY, int dimW, int dimH) {
  // click detector
  bool isInBound = false;
  if (clickX >= 0 && clickY >= 0) {
    if (clickX - pdp.dp_MinI >= posX && clickX - pdp.dp_MinI < posX + dimW) {
      if (clickY - pdp.dp_MinJ >= posY && clickY - pdp.dp_MinJ < posY + dimH) {
        isInBound = true;
      }
    }
  }

  if (isInBound) {
    currentMsg.message = WM_CHAR;
    currentMsg.wParam = (WPARAM) key;
  }

  pdp.Fill(posX + padding, posY + padding, dimW - padding * 2, dimH - padding * 2, C_KEY_BACK);

  char t[2] = "-";
  t[0] = key;
  CTString str = t;
  int centerX = posX + dimW / 2;
  PIX centerY = posY + dimH / 2;

  switch (key) {
    case 1: {
      str = "ok";
      if (isInBound) {
        currentMsg.message = WM_KEYDOWN;
        currentMsg.wParam = VK_RETURN;
      }
      break;
    }

    case 2: {
      str = "close";
      if (isInBound) {
        currentMsg.message = WM_KEYDOWN;
        currentMsg.wParam = VK_ESCAPE;
      }
      break;
    }

    case 3: {
      str = isCapsLock ? "abc" : "ABC";
      if (isInBound) {
        isCapsLock = !isCapsLock;
        currentMsg.message = 0;
      }
      break;
    }

    case 4: {
      str = "back";
      if (isInBound) {
        currentMsg.message = WM_KEYDOWN;
        currentMsg.wParam = VK_BACK;
      }
      break;
    }

    case 5: {
      str = "tab";
      if (isInBound) {
        currentMsg.message = WM_KEYDOWN;
        currentMsg.wParam = VK_TAB;
      }
      break;
    }

  }

  pdp.PutTextCXY(str, centerX, centerY, C_WHITE | CT_OPAQUE);
}

void CVirtualKeyboard::MouseDown(PIX pixX, PIX pixY) {
  clickX = pixX;
  clickY = pixY;
}
