
class CVirtualKeyboard {
private:
    bool isCapsLock = false;
    int keySizeW;
    int keySizeH;
    int padding;

    int selectedLayout = 0;

    PIX clickX = -1;
    PIX clickY = -1;

    void PutKey(const CDrawPort &pdp, char key, int posX, PIX posY, int dimW, int dimH);

    MSG currentMsg;

public:
    MSG DoKeyboard(CDrawPort *pdpParent);
    void MouseDown(PIX pixX, PIX pixY);

    CFontData *fontData = nullptr;
    float keyboardHeight = 0.4;
};
