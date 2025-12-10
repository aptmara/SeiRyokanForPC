#include "DirectInputGamepad.h"

#include <algorithm>
#include <cstring>

DirectInputGamepad::DirectInputGamepad(HINSTANCE hInst, HWND hWnd) {
    Initialize(hInst, hWnd);
}

DirectInputGamepad::~DirectInputGamepad() {
    if (m_joystick) { m_joystick->Unacquire(); m_joystick->Release(); m_joystick = nullptr; }
    if (m_di) { m_di->Release(); m_di = nullptr; }
}

bool DirectInputGamepad::Initialize(HINSTANCE hInst, HWND hWnd) {
    if (FAILED(DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_di, nullptr))) {
        return false;
    }

    IDirectInputDevice8* dev = nullptr;
    if (FAILED(m_di->CreateDevice(GUID_Joystick, &dev, nullptr))) {
        return false;
    }
    m_joystick = dev;

    m_joystick->SetDataFormat(&c_dfDIJoystick2);
    m_joystick->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    m_connected = (SUCCEEDED(m_joystick->Acquire()));
    std::memset(&m_state, 0, sizeof(m_state));
    std::memset(&m_prev, 0, sizeof(m_prev));
    return true;
}

bool DirectInputGamepad::IsConnected() const { return m_connected; }

void DirectInputGamepad::Update() { Poll(); }

void DirectInputGamepad::Poll() {
    if (!m_joystick) { m_connected = false; return; }
    m_prev = m_state;
    if (FAILED(m_joystick->Poll())) {
        if (FAILED(m_joystick->Acquire())) { m_connected = false; return; }
    }
    HRESULT hr = m_joystick->GetDeviceState(sizeof(m_state), &m_state);
    m_connected = SUCCEEDED(hr);
}

float DirectInputGamepad::NormalizeAxis(LONG v, LONG dz) {
    LONG mag = std::abs(v);
    if (mag <= dz) return 0.0f;
    float sign = (v < 0) ? -1.0f : 1.0f;
    float range = 32767.0f - static_cast<float>(dz);
    float n = (static_cast<float>(mag - dz)) / range;
    return sign * std::clamp(n, 0.0f, 1.0f);
}

float DirectInputGamepad::GetLeftStickX() { Poll(); return NormalizeAxis(m_state.lX, m_leftDeadzone); }
float DirectInputGamepad::GetLeftStickY() { Poll(); return NormalizeAxis(m_state.lY, m_leftDeadzone); }
float DirectInputGamepad::GetRightStickX() { Poll(); return NormalizeAxis(m_state.lZ, m_rightDeadzone); }
float DirectInputGamepad::GetRightStickY() { Poll(); return NormalizeAxis(m_state.lRz, m_rightDeadzone); }
Stick2D DirectInputGamepad::GetLeftStick() { Poll(); return { GetLeftStickX(), GetLeftStickY() }; }
Stick2D DirectInputGamepad::GetRightStick() { Poll(); return { GetRightStickX(), GetRightStickY() }; }

float DirectInputGamepad::GetLeftTrigger() { Poll(); return std::clamp((m_state.lRx + 32767.0f) / 65535.0f, 0.0f, 1.0f); }
float DirectInputGamepad::GetRightTrigger() { Poll(); return std::clamp((m_state.lRy + 32767.0f) / 65535.0f, 0.0f, 1.0f); }

static WORD MapButton(Button b) {
    switch (b) {
    case Button::A: return 0; // customize mapping as needed
    case Button::B: return 1;
    case Button::X: return 2;
    case Button::Y: return 3;
    case Button::LB: return 4;
    case Button::RB: return 5;
    case Button::BACK: return 6;
    case Button::START: return 7;
    case Button::L_THUMB: return 8;
    case Button::R_THUMB: return 9;
    case Button::DPAD_UP: return 10;
    case Button::DPAD_DOWN: return 11;
    case Button::DPAD_LEFT: return 12;
    case Button::DPAD_RIGHT: return 13;
    default: return 0xFFFF;
    }
}

bool DirectInputGamepad::IsButtonDown(Button button) {
    Poll();
    WORD idx = MapButton(button);
    if (idx == 0xFFFF) return false;
    return (m_state.rgbButtons[idx] & 0x80) != 0;
}

bool DirectInputGamepad::WasButtonPressed(Button button) {
    Poll();
    WORD idx = MapButton(button);
    if (idx == 0xFFFF) return false;
    return ((m_prev.rgbButtons[idx] ^ m_state.rgbButtons[idx]) & m_state.rgbButtons[idx] & 0x80) != 0;
}

bool DirectInputGamepad::WasButtonReleased(Button button) {
    Poll();
    WORD idx = MapButton(button);
    if (idx == 0xFFFF) return false;
    return ((m_prev.rgbButtons[idx] & 0x80) != 0) && ((m_state.rgbButtons[idx] & 0x80) == 0);
}

void DirectInputGamepad::SetDeadzone(short leftThumb, short rightThumb, unsigned char trigger) {
    m_leftDeadzone = leftThumb;
    m_rightDeadzone = rightThumb;
    m_triggerDeadzone = trigger;
}

void DirectInputGamepad::SetVibration(float, float) {
    // Not supported by DirectInput generally; noop
}
