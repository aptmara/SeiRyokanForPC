/*****************************************************************//**
 * @file   Time.h
 * @brief  PC側のFPS処理定義
 * 
 * @author 浅野勇生
 * @date   2025/7/16
 *********************************************************************/
#pragma once
#include <stdint.h>

// ==============================
// 時間制御（高精度タイマー）
// SwitchのTick制御に近い構成
// ==============================

// 初期化：タイマーの基準時間をリセット（1回だけ呼ぶ）
void InitTime();

// 毎フレーム呼び出し：現在のTickと経過時間を更新
void UpdateTime();

// 指定FPSのフレーム時間を超えたか？（60fps → 16.66ms）
bool ShouldUpdateFrame(double targetFps = 60.0);

// 前回からの経過時間（秒）を取得（例：0.016秒）
double GetElapsedTime();

// 現在のTick値を返す（デバッグや記録用）
uint64_t GetCurrentTick();
