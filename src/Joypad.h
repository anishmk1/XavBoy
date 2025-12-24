// Joypad.h
#ifndef JOYPAD_H
#define JOYPAD_H


struct KeyMatrix {

    // Map to D-Pad
    bool W,A,S,D;
    bool Up,Down,Left,Right;

    // Map to A, B
    bool J,K;

    // Map to Start
    bool Enter;

    // Map to Select
    bool Backspace;
};

struct ButtonMatrix {
    bool D_UP, D_DOWN, D_LEFT, D_RIGHT;
    bool A, B;
    bool START, SELECT;
};


class Joypad {
private:

public:
    struct KeyMatrix keys {};
    struct ButtonMatrix buttons {};

    Joypad() = default;

    void update_button_state();
};


#endif // Joypad.h