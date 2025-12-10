/*****************************************************************//**
 * @file   texture.cpp
 * @brief  PC側のテクスチャ読み込み実装
 *********************************************************************/
#include "texture.h"
#include "../System/DirectX.h" // GetDevice(), GetContext()
#include "../../DirectXTex/DirectXTex.h"
#include <cassert>

using namespace DirectX;

#ifdef _DEBUG
#pragma comment(lib, "DirectXTex/x64/Debug/DirectXTex.lib")
#else
#pragma comment(lib, "DirectXTex/x64/Release/DirectXTex.lib")
#endif

ID3D11ShaderResourceView* LoadTexture(const wchar_t* filename)
{
    ID3D11Device* device = GetDevice();
    assert(device != nullptr);

    TexMetadata metadata = {};
    ScratchImage scratch = {};

    // WIC_FLAGS_FORCE_RGBA32 は存在しない。標準ロード後に明示変換する。
    HRESULT hr = LoadFromWICFile(filename, WIC_FLAGS_IGNORE_SRGB, &metadata, scratch);
    if (FAILED(hr))
    {
        OutputDebugString(L"[LoadTexture] Failed to load image file\n");
        return nullptr;
    }

    // 32bit RGBA へ明示変換（Straight Alpha 前提）
    const DXGI_FORMAT TARGET = DXGI_FORMAT_R8G8B8A8_UNORM;
    if (metadata.format != TARGET &&
        metadata.format != DXGI_FORMAT_B8G8R8A8_UNORM &&
        metadata.format != DXGI_FORMAT_R8G8B8A8_UNORM_SRGB &&
        metadata.format != DXGI_FORMAT_B8G8R8A8_UNORM_SRGB)
    {
        ScratchImage converted;
        hr = Convert(
            scratch.GetImages(), scratch.GetImageCount(), scratch.GetMetadata(),
            TARGET,
            TEX_FILTER_DEFAULT, TEX_THRESHOLD_DEFAULT, converted);
        if (SUCCEEDED(hr))
        {
            scratch = std::move(converted);
            metadata = scratch.GetMetadata();
        }
        else
        {
            OutputDebugString(L"[LoadTexture] Convert to RGBA8 failed, continue with original format.\n");
        }
    }

    ID3D11ShaderResourceView* textureView = nullptr;
    hr = CreateShaderResourceView(device, scratch.GetImages(), scratch.GetImageCount(), metadata, &textureView);

    return textureView;
}

void SetTexture(ID3D11ShaderResourceView* texture)
{
    ID3D11DeviceContext* context = GetContext();
    assert(context != nullptr);
    context->PSSetShaderResources(0, 1, &texture);
}

void UnloadTexture(ID3D11ShaderResourceView* texture)
{
    if (texture)
    {
        texture->Release();
    }
}
