/*****************************************************************//**
 * @file   SpriteDrawer.h
 * @brief  PC—p‚ÌSprite•`‰æŽÀ‘•
 *
 * @author ŽR‰êŒ\
 * @date   2025/7/16
 *********************************************************************/
#pragma once
#include <d3d11.h>
#include "../../common_src/VectorTypes.h"

class SpriteDrawer
{
public:
    SpriteDrawer();
    ~SpriteDrawer();

    bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, UINT screenWidth, UINT screenHeight, bool isYup = false);
    void Finalize();

    void Draw(ID3D11ShaderResourceView* texture,
        const MyGame::Float2& pos,
        const MyGame::Float2& scale,
        const MyGame::Float4& color,
        float angleDeg,
        const MyGame::Float2& uvPos,
        const MyGame::Float2& uvScale);

public:
    void SetSdfMode(bool enable) { m_useSdf = enable; }

private:
    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_context = nullptr;

    ID3D11InputLayout* m_inputLayout = nullptr;
    ID3D11VertexShader* m_vs = nullptr;
    ID3D11PixelShader* m_ps = nullptr;
    ID3D11Buffer* m_vtxBuffer = nullptr;
    ID3D11Buffer* m_constBuffer = nullptr;
    ID3D11RasterizerState* m_rasterizer = nullptr;
    ID3D11SamplerState* m_sampler = nullptr;

    ID3D11PixelShader* m_psSdf = nullptr;
    bool m_useSdf = false;

    ID3D11BlendState* m_blendState = nullptr;

    MyGame::Float2 m_screen;
};