#pragma once
#include "../../common_src/IGraphics.h"
#include "../../common_src/VectorTypes.h" // Vec2f, Float4 などの型を利用するため
#include <d3d11.h>
#include <wrl/client.h> // ComPtr を使うため追加
#include "SpriteDrawer.h"

class DirectXGraphics : public IGraphics
{
public:
    DirectXGraphics() = default;
    ~DirectXGraphics() = default;

    // IGraphicsインターフェースの実装
    bool Initialize(void* windowHandle, int screenWidth, int screenHeight) override;
    void Finalize() override;
    void BeginDraw() override;
    void EndDraw() override;
    TextureHandle LoadTexture(const char* filePath) override;
    void UnloadTexture(TextureHandle handle) override;
    void DrawQuad(const Quad& quad) override;
    void SetSdfMode(bool enable) override { m_spriteDrawer.SetSdfMode(enable); }

private:
    SpriteDrawer m_spriteDrawer;
    TextureHandle m_defaultTexture = nullptr;

    // 頂点構造体
    struct ArcVertex
    {
        MyGame::Float3 position;
        MyGame::Float2 uv;
        MyGame::Float4 color;
    };

    // シェーダーと入力レイアウト
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_arcVs;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_arcPs;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_arcInputLayout;

    // 動的頂点バッファ
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_arcVtxBuffer;
    UINT m_arcVtxBufferSize = 0;

    // 定数バッファ (スクリーンサイズをシェーダーに渡すため)
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_arcConstBuffer;

    // パイプラインステート
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_arcSamplerState;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_arcBlendState;
};