#pragma once
#ifndef XINPUT_GAMEPAD_H
#define XINPUT_GAMEPAD_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Xinput.h>

#include <cstdint>
#include <cmath>
#include <algorithm>

#include "../../common_src/IGamepad.h"

class XInputGamepad final : public IGamepad {
public:
    explicit XInputGamepad(unsigned int userIndex = 0);

    bool IsConnected() const override;
    void Update() override; // エッジ検出が必要な時だけ呼ぶ

    // ※ あなたのコードはゲッターを直接呼ぶので、自動ポーリングで最新化します
    float   GetLeftStickX()  override;
    float   GetLeftStickY()  override;
    float   GetRightStickX() override;
    float   GetRightStickY() override;
    Stick2D GetLeftStick()   override;
    Stick2D GetRightStick()  override;

    float GetLeftTrigger()  override;
    float GetRightTrigger() override;

    bool IsButtonDown(Button button) override;
    bool WasButtonPressed(Button button)  override;
    bool WasButtonReleased(Button button) override;

    unsigned int GetUserIndex() const override;

    void SetDeadzone(short leftThumb, short rightThumb, unsigned char trigger) override;
    void SetVibration(float leftMotor, float rightMotor) override;

private:
    // 自動ポーリング用（ゲッターが呼ばれたら都度最新化：前回状態は触らない）
    void PollLevelOnly();

    // Update() 用（前回/今回を進める：エッジ検出に使用）
    void ReadCurrentStateAdvance();

    static float NormalizeThumb(short v, short deadzone);
    static float NormalizeTrigger(unsigned char v, unsigned char deadzone);
    static WORD  ToXInputButton(Button b);

private:
    unsigned int m_userIndex = 0;
    bool         m_connected = false;

    // XInput raw states
    XINPUT_STATE m_state{};     // 現在
    XINPUT_STATE m_prevState{}; // 前回（エッジ用）

    // 正規化済み（レベル）
    Stick2D m_leftStick{};
    Stick2D m_rightStick{};
    float   m_leftTrigger = 0.0f;
    float   m_rightTrigger = 0.0f;

    // デッドゾーン
    short         m_leftDeadzone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;   // 7849
    short         m_rightDeadzone = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE; // 8689
    unsigned char m_triggerDeadzone = 30;
};

#endif // XINPUT_GAMEPAD_H
