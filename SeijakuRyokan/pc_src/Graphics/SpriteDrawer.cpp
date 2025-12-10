/*****************************************************************//**
 * @file   SpriteDrawer.cpp
 * @brief  PC側のSprite描画実装
 *********************************************************************/
#include "SpriteDrawer.h"
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>
#pragma comment(lib, "d3dcompiler.lib")

using Microsoft::WRL::ComPtr;
using namespace DirectX;

struct SpriteVertex
{
    XMFLOAT3 pos;
    XMFLOAT2 uv;
};

struct SpriteParam
{
    XMFLOAT2 pos;
    XMFLOAT2 scale;
    XMFLOAT2 uvPos;
    XMFLOAT2 uvScale;
    XMFLOAT4 color;
    XMFLOAT2 screen;
    float    rad;
    float    dummy;
};

SpriteDrawer::SpriteDrawer() {}
SpriteDrawer::~SpriteDrawer() {}

bool SpriteDrawer::Initialize(ID3D11Device* device, ID3D11DeviceContext* context,
    UINT screenWidth, UINT screenHeight, bool /*isYup*/)
{
    m_device = device;
    m_context = context;
    m_screen = MyGame::Float2((float)screenWidth, (float)screenHeight);

    // ===== VS =====
    const char* VS = R"EOT(
    struct VS_IN { float3 pos:POSITION0; float2 uv:TEXCOORD0; };
    struct VS_OUT{ float4 pos:SV_POSITION; float2 uv:TEXCOORD0; float4 color:COLOR0; };

    cbuffer SpriteParamCB : register(b0) {
        float2 gPos; float2 gScale;
        float2 gUvPos; float2 gUvScale;
        float4 gColor; float2 gScreen;
        float  gRad; float  _pad;
    };

    VS_OUT main(VS_IN i)
    {
        VS_OUT o;
        float2 p = i.pos.xy * gScale;
        float cs = cos(gRad), sn = sin(gRad);
        p = float2(p.x * cs - p.y * sn, p.x * sn + p.y * cs) + gPos;
        float ndcX = (p.x / gScreen.x) * 2.0f - 1.0f;
        float ndcY = 1.0f - (p.y / gScreen.y) * 2.0f;
        o.pos = float4(ndcX, ndcY, 0.0f, 1.0f);
        o.uv  = i.uv * gUvScale + gUvPos;
        o.color = gColor;
        return o;
    })EOT";

    // ===== PS (通常RGBA) =====
    const char* PS_RGBA = R"EOT(
    struct PS_IN { float4 pos:SV_POSITION; float2 uv:TEXCOORD0; float4 color:COLOR0; };
    Texture2D tex0 : register(t0);
    SamplerState samp0 : register(s0);
    float4 main(PS_IN i) : SV_TARGET
    {
        float4 t = tex0.Sample(samp0, i.uv);
        return t * i.color;
    })EOT";

    // ===== PS (SDF) =====
    const char* PS_SDF = R"EOT(
    struct PS_IN { float4 pos:SV_POSITION; float2 uv:TEXCOORD0; float4 color:COLOR0; };
    Texture2D tex0 : register(t0);
    SamplerState samp0 : register(s0);
    float4 main(PS_IN i) : SV_TARGET
    {
        float sd = tex0.Sample(samp0, i.uv).a;
        float w  = fwidth(sd);
        float a  = smoothstep(0.5 - w, 0.5 + w, sd);
        return float4(i.color.rgb, i.color.a * a);
    })EOT";

    // --- コンパイル ---
    ComPtr<ID3DBlob> vsb, psb, psbSdf, err;
    UINT flags = 0;
#ifdef _DEBUG
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    if (FAILED(D3DCompile(VS, strlen(VS), nullptr, nullptr, nullptr, "main", "vs_5_0", flags, 0, vsb.GetAddressOf(), err.GetAddressOf()))) {
        if (err) OutputDebugStringA((const char*)err->GetBufferPointer());
        return false;
    }
    err.Reset();
    if (FAILED(D3DCompile(PS_RGBA, strlen(PS_RGBA), nullptr, nullptr, nullptr, "main", "ps_5_0", flags, 0, psb.GetAddressOf(), err.GetAddressOf()))) {
        if (err) OutputDebugStringA((const char*)err->GetBufferPointer());
        return false;
    }
    err.Reset();
    if (FAILED(D3DCompile(PS_SDF, strlen(PS_SDF), nullptr, nullptr, nullptr, "main", "ps_5_0", flags, 0, psbSdf.GetAddressOf(), err.GetAddressOf()))) {
        if (err) OutputDebugStringA((const char*)err->GetBufferPointer());
        return false;
    }

    // --- シェーダ ---
    if (FAILED(m_device->CreateVertexShader(vsb->GetBufferPointer(), vsb->GetBufferSize(), nullptr, &m_vs))) return false;
    if (FAILED(m_device->CreatePixelShader(psb->GetBufferPointer(), psb->GetBufferSize(), nullptr, &m_ps))) return false;
    if (FAILED(m_device->CreatePixelShader(psbSdf->GetBufferPointer(), psbSdf->GetBufferSize(), nullptr, &m_psSdf))) return false;

    // --- 入力レイアウト ---
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    if (FAILED(m_device->CreateInputLayout(layout, _countof(layout), vsb->GetBufferPointer(), vsb->GetBufferSize(), &m_inputLayout))) return false;

    // --- 定数バッファ ---
    D3D11_BUFFER_DESC cbd = {};
    cbd.ByteWidth = sizeof(SpriteParam); // 16倍数
    cbd.Usage = D3D11_USAGE_DEFAULT;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    if (FAILED(m_device->CreateBuffer(&cbd, nullptr, &m_constBuffer))) return false;

    // --- 頂点バッファ（TRIANGLESTRIP 4頂点） ---
    SpriteVertex verts[4] = {
        { XMFLOAT3(-0.5f,  0.5f, 0), XMFLOAT2(0,1) },
        { XMFLOAT3(-0.5f, -0.5f, 0), XMFLOAT2(0,0) },
        { XMFLOAT3(0.5f,  0.5f, 0), XMFLOAT2(1,1) },
        { XMFLOAT3(0.5f, -0.5f, 0), XMFLOAT2(1,0) },
    };
    D3D11_BUFFER_DESC vbd = {};
    vbd.ByteWidth = sizeof(verts);
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA srd = { verts };
    if (FAILED(m_device->CreateBuffer(&vbd, &srd, &m_vtxBuffer))) return false;

    // --- ラスタライザ ---
    D3D11_RASTERIZER_DESC rs = {};
    rs.FillMode = D3D11_FILL_SOLID;
    rs.CullMode = D3D11_CULL_NONE;
    rs.FrontCounterClockwise = FALSE;
    if (FAILED(m_device->CreateRasterizerState(&rs, &m_rasterizer))) return false;

    // --- サンプラ ---
    D3D11_SAMPLER_DESC smp = {};
    smp.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    smp.AddressU = smp.AddressV = smp.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    smp.MinLOD = 0.0f;
    smp.MaxLOD = 0.0f; // ミップ不要
    if (FAILED(m_device->CreateSamplerState(&smp, &m_sampler))) return false;

    // --- ブレンド（Straight Alpha 前提） ---
    D3D11_BLEND_DESC b = {};
    b.RenderTarget[0].BlendEnable = TRUE;
    b.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    b.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    b.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    b.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    b.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    b.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    b.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    if (FAILED(m_device->CreateBlendState(&b, &m_blendState))) return false;

    m_useSdf = false;
    return true;
}

void SpriteDrawer::Finalize()
{
    if (m_sampler) { m_sampler->Release();      m_sampler = nullptr; }
    if (m_rasterizer) { m_rasterizer->Release();   m_rasterizer = nullptr; }
    if (m_constBuffer) { m_constBuffer->Release();  m_constBuffer = nullptr; }
    if (m_vtxBuffer) { m_vtxBuffer->Release();    m_vtxBuffer = nullptr; }
    if (m_psSdf) { m_psSdf->Release();        m_psSdf = nullptr; } // ← 追加
    if (m_ps) { m_ps->Release();           m_ps = nullptr; }
    if (m_vs) { m_vs->Release();           m_vs = nullptr; }
    if (m_inputLayout) { m_inputLayout->Release();  m_inputLayout = nullptr; }
    if (m_blendState) { m_blendState->Release();   m_blendState = nullptr; }
}

void SpriteDrawer::Draw(ID3D11ShaderResourceView* texture,
    const MyGame::Float2& pos, const MyGame::Float2& scale,
    const MyGame::Float4& color, float angleDeg,
    const MyGame::Float2& uvPos, const MyGame::Float2& uvScale)
{
    //  ここで OM にブレンドをセット 
    float blendFactor[4] = { 0,0,0,0 };
    UINT  mask = 0xffffffff;
    m_context->OMSetBlendState(m_blendState, blendFactor, mask); // ← これが無いと透過しない

    SpriteParam p = {};
    p.pos = XMFLOAT2(pos.x, pos.y);
    p.scale = XMFLOAT2(scale.x, scale.y);
    p.uvPos = XMFLOAT2(uvPos.x, uvPos.y);
    p.uvScale = XMFLOAT2(uvScale.x, uvScale.y);
    p.color = XMFLOAT4(color.x, color.y, color.z, color.w);
    p.screen = XMFLOAT2(m_screen.x, m_screen.y);
    p.rad = XMConvertToRadians(angleDeg);

    m_context->IASetInputLayout(m_inputLayout);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    UINT stride = sizeof(SpriteVertex), offset = 0;
    m_context->IASetVertexBuffers(0, 1, &m_vtxBuffer, &stride, &offset);

    m_context->VSSetShader(m_vs, nullptr, 0);
    m_context->PSSetShader(m_useSdf ? m_psSdf : m_ps, nullptr, 0);
    m_context->VSSetConstantBuffers(0, 1, &m_constBuffer);
    m_context->PSSetSamplers(0, 1, &m_sampler);
    m_context->PSSetShaderResources(0, 1, &texture);
    m_context->RSSetState(m_rasterizer);

    m_context->UpdateSubresource(m_constBuffer, 0, nullptr, &p, 0, 0);
    m_context->Draw(4, 0);

    // 必要なら状態を戻す
    // m_context->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}
