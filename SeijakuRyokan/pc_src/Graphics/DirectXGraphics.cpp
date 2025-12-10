#include "DirectXGraphics.h"
#include "sprite.h"
#include "texture.h"
#include "../System/DirectX.h"
#include "../../common_src/VectorTypes.h"
#include "../../common_src/System/PathFinder.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <vector>
#include <cmath>
#include <d3dcompiler.h>
#include <string>
#pragma comment(lib, "d3dcompiler.lib") // ライブラリをリンク


// char*からwchar_t*への変換ユーティリティ（Windows API使用）
static std::wstring to_wstring(const char* str) {
    if (!str || *str == '\0') return L"";

    // 必要なバッファサイズを取得
    int size = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
    if (size <= 0) return L"";

    // 変換
    std::wstring result(size - 1, L'\0'); // -1 for null terminator
    MultiByteToWideChar(CP_UTF8, 0, str, -1, &result[0], size);

    return result;
}

bool DirectXGraphics::Initialize(void* windowHandle, int screenWidth, int screenHeight)
{
    // DirectXデバイス自体の初期化をチェック
    if (!InitDirectX(static_cast<HWND>(windowHandle))) {
        OutputDebugStringA("[ERROR] InitDirectX() failed.\n");
        return false;
    }

    // Sprite初期化（内部で失敗する可能性は低いが念のため）
    InitSprite(GetDevice(), GetContext());

    // デフォルトテクスチャの読み込みをチェック
    m_defaultTexture = LoadTexture("rom/images/white.png");
    if (!m_defaultTexture) {
        OutputDebugStringA("[ERROR] Failed to load default texture 'rom/images/white.png'. Make sure the file exists.\n");
        return false;
    }

    ID3D11Device* device = GetDevice();
    HRESULT hr;

    // 1. シェーダーの作成
    const char* SHADER_CODE = R"EOT(
        struct ArcVertex {
            float3 pos   : POSITION0;
            float2 uv    : TEXCOORD0;
            float4 color : COLOR0;
        };
        struct VsOutput {
            float4 pos   : SV_POSITION;
            float2 uv    : TEXCOORD0;
            float4 color : COLOR0;
        };
        cbuffer ScreenParams : register(b0) {
            float2 screenSize;
        };

        // Vertex Shader
        VsOutput VS(ArcVertex vin) {
            VsOutput vout;
            // スクリーン座標をクリップ座標 (-1.0f ~ 1.0f) に変換
            vout.pos.x = (vin.pos.x / screenSize.x) * 2.0f - 1.0f;
            vout.pos.y = 1.0f - (vin.pos.y / screenSize.y) * 2.0f;
            vout.pos.z = vin.pos.z;
            vout.pos.w = 1.0f;
            vout.uv = vin.uv;
            vout.color = vin.color;
            return vout;
        }

        // Pixel Shader
        Texture2D    g_texture : register(t0);
        SamplerState g_sampler : register(s0);

        float4 PS(VsOutput pin) : SV_TARGET {
            return g_texture.Sample(g_sampler, pin.uv) * pin.color;
        }
    )EOT";

    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, psBlob, errorBlob;
    UINT compileFlags = 0;
#ifdef _DEBUG
    compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    // 頂点シェーダーのコンパイル
    hr = D3DCompile(SHADER_CODE, strlen(SHADER_CODE), nullptr, nullptr, nullptr, "VS", "vs_5_0", compileFlags, 0, &vsBlob, &errorBlob);
    if (FAILED(hr)) {
        OutputDebugStringA("[ERROR] Failed to compile Vertex Shader.\n");
        if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        return false;
    }

    // ピクセルシェーダーのコンパイル
    hr = D3DCompile(SHADER_CODE, strlen(SHADER_CODE), nullptr, nullptr, nullptr, "PS", "ps_5_0", compileFlags, 0, &psBlob, &errorBlob);
    if (FAILED(hr)) {
        OutputDebugStringA("[ERROR] Failed to compile Pixel Shader.\n");
        if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        return false;
    }

    // 頂点シェーダーオブジェクトの作成
    hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_arcVs);
    if (FAILED(hr)) {
        OutputDebugStringA("[ERROR] Failed to create Vertex Shader object.\n");
        return false;
    }

    // ピクセルシェーダーオブジェクトの作成
    hr = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_arcPs);
    if (FAILED(hr)) {
        OutputDebugStringA("[ERROR] Failed to create Pixel Shader object.\n");
        return false;
    }

    // 2. 入力レイアウトの作成
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    hr = device->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_arcInputLayout);
    if (FAILED(hr)) {
        OutputDebugStringA("[ERROR] Failed to create Input Layout.\n");
        return false;
    }

    // 3. 定数バッファの作成
    D3D11_BUFFER_DESC constDesc = {};
    constDesc.ByteWidth = sizeof(float) * 4; // 実際には16バイトアライメントが必要
    constDesc.Usage = D3D11_USAGE_DEFAULT;
    constDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    hr = device->CreateBuffer(&constDesc, nullptr, &m_arcConstBuffer);
    if (FAILED(hr)) {
        OutputDebugStringA("[ERROR] Failed to create Constant Buffer.\n");
        return false;
    }

    // 定数バッファにスクリーンサイズを設定
    float screenSize[4] = { (float)screenWidth, (float)screenHeight, 0.0f, 0.0f };
    GetContext()->UpdateSubresource(m_arcConstBuffer.Get(), 0, nullptr, screenSize, 0, 0);

    // 4. サンプラーステートの作成
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = device->CreateSamplerState(&sampDesc, &m_arcSamplerState);
    if (FAILED(hr)) {
        OutputDebugStringA("[ERROR] Failed to create Sampler State.\n");
        return false;
    }

    // 5. ブレンドステートの作成
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    hr = device->CreateBlendState(&blendDesc, &m_arcBlendState);
    if (FAILED(hr)) {
        OutputDebugStringA("[ERROR] Failed to create Blend State.\n");
        return false;
    }
    if (!m_spriteDrawer.Initialize(device, GetContext(), screenWidth, screenHeight, true)) {
        return false;
    }

    // すべての初期化が成功
    OutputDebugStringA("[INFO] DirectXGraphics::Initialize() succeeded.\n");
    return true;
}

void DirectXGraphics::Finalize()
{

    m_arcVs.Reset();
    m_arcPs.Reset();
    m_arcInputLayout.Reset();
    m_arcVtxBuffer.Reset();
    m_arcConstBuffer.Reset();
    m_arcSamplerState.Reset();
    m_arcBlendState.Reset();
    m_spriteDrawer.Finalize();

    // デフォルトテクスチャを解放
    if (m_defaultTexture) {
        UnloadTexture(m_defaultTexture);
        m_defaultTexture = nullptr;
    }

    UninitSprite();
    CleanupDirectX();
}

void DirectXGraphics::BeginDraw()
{
    ::BeginDraw(0.1f, 0.1f, 0.1f, 1.0f);
}

void DirectXGraphics::EndDraw()
{
    ::EndDraw();
}

TextureHandle DirectXGraphics::LoadTexture(const char* filePath)
{
    std::wstring widePath = to_wstring(filePath);
    return ::LoadTexture(widePath.c_str());
}

void DirectXGraphics::UnloadTexture(TextureHandle handle)
{
    if (handle) {
        ::UnloadTexture(static_cast<ID3D11ShaderResourceView*>(handle));
    }
}

void DirectXGraphics::DrawQuad(const Quad& quad)
{
    TextureHandle th = quad.texture ? quad.texture : m_defaultTexture;
    ID3D11ShaderResourceView* srv = static_cast<ID3D11ShaderResourceView*>(th);

    // 中心座標・スケールはQuadそのまま
    m_spriteDrawer.Draw(
        srv,
        MyGame::Float2(quad.position.x, quad.position.y),
        MyGame::Float2(quad.size.x, quad.size.y),
        quad.color,
        quad.angleDeg,
        quad.uvPos,
        quad.uvSize
    );
}