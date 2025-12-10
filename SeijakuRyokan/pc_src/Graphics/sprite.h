/*****************************************************************//**
 * @file   sprite.h
 * @brief  PC用のSprite描画実装
 *
 * @author 山賀圭
 * @date   2025/7/16
 *********************************************************************/
#pragma once

#include <d3d11.h>
#include "../../common_src/VectorTypes.h"

// Switch互換構造体（QUAD_2D / TRIANGLE_2D など）
struct QUAD_2D
{
    MyGame::Float3 quadPos;
    MyGame::Float2 quadSize;
    float angleDeg = 0.0f; //回転角

    MyGame::Float4 quadColor;

    ID3D11ShaderResourceView* pTexture = nullptr; //nullptrで初期化

    MyGame::Float2 posTexCoord;
    MyGame::Float2 sizeTexCoord;

    bool use = true;
    int currentAnimNo = 0;

    QUAD_2D() : quadPos(), quadSize(), angleDeg(0.0f), quadColor(), pTexture(nullptr), posTexCoord(), sizeTexCoord(), use(true), currentAnimNo(0) {}
};

struct TRIANGLE_2D
{
    float x1, y1;
    float x2, y2;
    float x3, y3;
    float r, g, b, a;
    float angleDeg;
};

// 初期化・終了
void InitSprite(ID3D11Device* device, ID3D11DeviceContext* context);
void UninitSprite();

// スプライト描画（Switchと同様）
void DrawSpriteQuad(QUAD_2D* quad);
void DrawSpriteTriangle(TRIANGLE_2D* tri);
