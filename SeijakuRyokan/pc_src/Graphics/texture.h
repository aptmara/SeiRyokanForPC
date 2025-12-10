/*****************************************************************//**
 * @file   texture.h
 * @brief  PC側のテクスチャ読み込み定義
 * 
 * @author 浅野勇生
 * @date   2025/7/16
 *********************************************************************/
#pragma once
#include <d3d11.h>

// テクスチャの読み込み（filenameはL"sample.png" のようなパス）
ID3D11ShaderResourceView* LoadTexture(const wchar_t* filename);

// テクスチャのセット（ピクセルシェーダにバインド）
void SetTexture(ID3D11ShaderResourceView* texture);

// テクスチャの解放
void UnloadTexture(ID3D11ShaderResourceView* texture);
