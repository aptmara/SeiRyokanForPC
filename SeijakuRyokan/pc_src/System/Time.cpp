/*****************************************************************************************
 * @file   Time.cpp
 * @brief  時間管理ユーティリティ（FPS固定・経過時間取得）
 *
 * @details
 * - 高精度タイマー（QueryPerformanceCounter）を使用
 * - busy wait（スリープなし）により、指定FPSで処理制御を行う
 * - ゲームループ内で `ShouldUpdateFrame()` を使って、タイミング制御を行う
 *
 * @author 浅野勇生
 *****************************************************************************************/

#include "Time.h"
#include <windows.h>

static LARGE_INTEGER g_frequency;             ///< 周波数（秒あたりのカウント数）
static LARGE_INTEGER g_prevTick;              ///< 前フレームのTick
static LARGE_INTEGER g_currentTick;           ///< 現在のTick
static LARGE_INTEGER g_nextTick;              ///< 次のフレームの目標Tick
static double g_elapsedTime = 0.0;            ///< 経過時間（秒）
static double g_frameTimeTarget = 1.0 / 60.0; ///< 目標フレーム間隔（初期:60FPS）


/**
 * @brief 時間システムの初期化処理
 *
 * @details
 * - 周波数と現在Tickを記録し、以降の計算基準とする。
 */
void InitTime()
{
    QueryPerformanceFrequency(&g_frequency);
    QueryPerformanceCounter(&g_prevTick);
    g_currentTick = g_prevTick;

    g_nextTick.QuadPart = g_prevTick.QuadPart + static_cast<LONGLONG>(g_frameTimeTarget * g_frequency.QuadPart);
}

/**
 * @brief 経過時間の更新処理（前回との差分を計測）
 *
 * @details
 * - フレーム間の経過時間を秒単位で記録する。
 */
void UpdateTime()
{
    QueryPerformanceCounter(&g_currentTick);
    g_elapsedTime = static_cast<double>(
        g_currentTick.QuadPart - g_prevTick.QuadPart
        ) / g_frequency.QuadPart;

    g_prevTick = g_currentTick;
}

/**
 * @brief 指定FPS間隔で処理を実行するか判定する（busy wait方式）
 *
 * @param targetFps 希望するフレームレート（例：30.0 で 30FPS）
 * @return true 時間が到達し、次の処理を実行すべきタイミング
 *
 * @details
 * - スリープを用いず、指定時間までCPUを使って待機する
 * - 経過後に次の目標時間を更新する
 */
bool ShouldUpdateFrame(double targetFps)
{
    g_frameTimeTarget = 1.0 / targetFps;

    while (true)
    {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);

        if (now.QuadPart >= g_nextTick.QuadPart)
        {
            g_prevTick = now;
            g_nextTick.QuadPart = now.QuadPart + static_cast<LONGLONG>(g_frameTimeTarget * g_frequency.QuadPart);
            return true;
        }
    }
}

/**
 * @brief 経過時間（前フレームからの差分秒）を取得
 *
 * @return 経過時間 [秒]
 */
double GetElapsedTime()
{
    return g_elapsedTime;
}

/**
 * @brief 現在のTick値を取得
 *
 * @return Tickの整数値（QueryPerformanceCounterベース）
 */
uint64_t GetCurrentTick()
{
    return static_cast<uint64_t>(g_currentTick.QuadPart);
}
