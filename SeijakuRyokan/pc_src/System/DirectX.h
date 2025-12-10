/*****************************************************************//**
 * @file   DirectX.h
 * @brief  DirectX処理定義
 *
 * @author 浅野勇生
 * @date   2025/7/16
 *********************************************************************/
#pragma once
#include <d3d11.h>

// DirectX初期化・描画用関数
bool InitDirectX(HWND hwnd);
void BeginDraw(float r, float g, float b, float a);
void EndDraw();
void CleanupDirectX();

// デバイスやコンテキストを取得する関数（必要に応じて追加）
ID3D11Device* GetDevice();
ID3D11DeviceContext* GetContext();
