#pragma once
#ifndef DIRECTINPUT_GAMEPAD_H
#define DIRECTINPUT_GAMEPAD_H

#include <windows.h>
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include <cstdint>
#include "../../common_src/IGamepad.h"

class DirectInputGamepad final : public IGamepad {
public:
    DirectInputGamepad(HINSTANCE hInst, HWND hWnd);
    ~DirectInputGamepad();

    bool IsConnected() const override;
    void Update() override;

    float   GetLeftStickX() override;
    float   GetLeftStickY() override;
    float   GetRightStickX() override;
    float   GetRightStickY() override;
    Stick2D GetLeftStick() override;
    Stick2D GetRightStick() override;

    float GetLeftTrigger() override;
    float GetRightTrigger() override;

    bool IsButtonDown(Button button) override;
    bool WasButtonPressed(Button button) override;
    bool WasButtonReleased(Button button) override;

    unsigned int GetUserIndex() const override { return 0; }

    void SetDeadzone(short leftThumb, short rightThumb, unsigned char trigger) override;
    void SetVibration(float leftMotor, float rightMotor) override; // no-op

private:
    bool Initialize(HINSTANCE hInst, HWND hWnd);
    void Poll();

    IDirectInput8* m_di = nullptr;
    IDirectInputDevice8* m_joystick = nullptr;
    bool m_connected = false;

    DIJOYSTATE2 m_state{};
    DIJOYSTATE2 m_prev{};

    short m_leftDeadzone = 7849;
    short m_rightDeadzone = 8689;
    unsigned char m_triggerDeadzone = 30;

    float NormalizeAxis(LONG value, LONG deadzone);
};

#endif // DIRECTINPUT_GAMEPAD_H
