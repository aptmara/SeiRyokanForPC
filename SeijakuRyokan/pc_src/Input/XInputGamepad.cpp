// Minimal XInput-only implementation
#include "XInputGamepad.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Xinput.h>
#pragma comment(lib, "Xinput.lib")

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <algorithm>
#include <cstring>

XInputGamepad::XInputGamepad(unsigned int userIndex)
    : m_userIndex(userIndex) {
    std::memset(&m_state, 0, sizeof(m_state));
    std::memset(&m_prevState, 0, sizeof(m_prevState));
}

bool XInputGamepad::IsConnected() const {
    return m_connected;
}

void XInputGamepad::Update() {
    // 1フレームにつき1回だけ最新状態を取得し、前回との差分を確定させる
    ReadCurrentStateAdvance();
}

void XInputGamepad::PollLevelOnly() {
    XINPUT_STATE s{};
    DWORD r = ::XInputGetState(m_userIndex, &s);
    m_connected = (r == ERROR_SUCCESS);
    if (!m_connected) {
        std::memset(&s, 0, sizeof(s));
    }
    m_state = s;

    m_leftStick.x = NormalizeThumb(m_state.Gamepad.sThumbLX, m_leftDeadzone);
    m_leftStick.y = NormalizeThumb(m_state.Gamepad.sThumbLY, m_leftDeadzone);
    m_rightStick.x = NormalizeThumb(m_state.Gamepad.sThumbRX, m_rightDeadzone);
    m_rightStick.y = NormalizeThumb(m_state.Gamepad.sThumbRY, m_rightDeadzone);

    m_leftTrigger = NormalizeTrigger(m_state.Gamepad.bLeftTrigger, m_triggerDeadzone);
    m_rightTrigger = NormalizeTrigger(m_state.Gamepad.bRightTrigger, m_triggerDeadzone);
}

void XInputGamepad::ReadCurrentStateAdvance() {
    // 前回状態を退避してから現在値を読み直す
    m_prevState = m_state;
    PollLevelOnly();
}

// ゲッターは Update()/ReadCurrentStateAdvance() 済みの値を返すだけにする
float   XInputGamepad::GetLeftStickX() { return m_leftStick.x; }
float   XInputGamepad::GetLeftStickY() { return m_leftStick.y; }
float   XInputGamepad::GetRightStickX() { return m_rightStick.x; }
float   XInputGamepad::GetRightStickY() { return m_rightStick.y; }
Stick2D XInputGamepad::GetLeftStick() { return m_leftStick; }
Stick2D XInputGamepad::GetRightStick() { return m_rightStick; }

float XInputGamepad::GetLeftTrigger() { return m_leftTrigger; }
float XInputGamepad::GetRightTrigger() { return m_rightTrigger; }

bool XInputGamepad::IsButtonDown(Button button) {
    PollLevelOnly();
    WORD mask = ToXInputButton(button);
    return (m_state.Gamepad.wButtons & mask) != 0;
}

bool XInputGamepad::WasButtonPressed(Button button) {
    WORD mask = ToXInputButton(button);
    WORD prev = m_prevState.Gamepad.wButtons;
    WORD curr = m_state.Gamepad.wButtons;
    return ((~prev) & curr & mask) != 0;
}

bool XInputGamepad::WasButtonReleased(Button button) {
    WORD mask = ToXInputButton(button);
    WORD prev = m_prevState.Gamepad.wButtons;
    WORD curr = m_state.Gamepad.wButtons;
    return ((prev) & (~curr) & mask) != 0;
}

unsigned int XInputGamepad::GetUserIndex() const {
    return m_userIndex;
}

void XInputGamepad::SetDeadzone(short leftThumb, short rightThumb, unsigned char trigger) {
    m_leftDeadzone = (std::max<short>)(0, leftThumb);
    m_rightDeadzone = (std::max<short>)(0, rightThumb);
    m_triggerDeadzone = trigger;
}

void XInputGamepad::SetVibration(float leftMotor, float rightMotor) {
    XINPUT_VIBRATION vib{};
    auto clampU16 = [](float v)->WORD {
        v = (std::clamp)(v, 0.0f, 1.0f);
        return static_cast<WORD>(v * 65535.0f + 0.5f);
    };
    vib.wLeftMotorSpeed = clampU16(leftMotor);
    vib.wRightMotorSpeed = clampU16(rightMotor);
    ::XInputSetState(m_userIndex, &vib);
}

float XInputGamepad::NormalizeThumb(short v, short deadzone) {
    int iv = static_cast<int>(v);
    int dz = (std::max<int>)(0, deadzone);
    int mag = std::abs(iv);
    if (mag <= dz) return 0.0f;
    const float sign = (iv < 0) ? -1.0f : 1.0f;
    const float range = 32767.0f - dz;
    float n = (static_cast<float>(mag - dz)) / range;
    n = (std::clamp)(n, 0.0f, 1.0f);
    return sign * n;
}

float XInputGamepad::NormalizeTrigger(unsigned char v, unsigned char deadzone) {
    int iv = static_cast<int>(v);
    int dz = static_cast<int>(deadzone);
    if (iv <= dz) return 0.0f;
    float n = static_cast<float>(iv - dz) / static_cast<float>(255 - dz);
    return (std::clamp)(n, 0.0f, 1.0f);
}

WORD XInputGamepad::ToXInputButton(Button b) {
    switch (b) {
    case Button::A:           return XINPUT_GAMEPAD_A;
    case Button::B:           return XINPUT_GAMEPAD_B;
    case Button::X:           return XINPUT_GAMEPAD_X;
    case Button::Y:           return XINPUT_GAMEPAD_Y;
    case Button::LB:          return XINPUT_GAMEPAD_LEFT_SHOULDER;
    case Button::RB:          return XINPUT_GAMEPAD_RIGHT_SHOULDER;
    case Button::BACK:        return XINPUT_GAMEPAD_BACK;
    case Button::START:       return XINPUT_GAMEPAD_START;
    case Button::L_THUMB:     return XINPUT_GAMEPAD_LEFT_THUMB;
    case Button::R_THUMB:     return XINPUT_GAMEPAD_RIGHT_THUMB;
    case Button::DPAD_UP:     return XINPUT_GAMEPAD_DPAD_UP;
    case Button::DPAD_DOWN:   return XINPUT_GAMEPAD_DPAD_DOWN;
    case Button::DPAD_LEFT:   return XINPUT_GAMEPAD_DPAD_LEFT;
    case Button::DPAD_RIGHT:  return XINPUT_GAMEPAD_DPAD_RIGHT;
    default:                  return 0;
    }
}
