/*****************************************************************//**
 * @file   DirectX.cpp
 * @brief  DirectXの処理実装
 *
 * @author 浅野勇生
 * @date   2025/7/16
 *********************************************************************/
#include "DirectX.h"
#include <cassert>

#pragma comment(lib, "d3d11.lib")

static ID3D11Device* g_device = nullptr;
static ID3D11DeviceContext* g_context = nullptr;
static IDXGISwapChain* g_swapChain = nullptr;
static ID3D11RenderTargetView* g_renderTargetView = nullptr;

bool InitDirectX(HWND hwnd)
{
    // スワップチェーンの設定
    DXGI_SWAP_CHAIN_DESC scDesc = {};
    scDesc.BufferCount = 1;
    scDesc.BufferDesc.Width = 1920;
    scDesc.BufferDesc.Height = 1080;
    scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.OutputWindow = hwnd;
    scDesc.SampleDesc.Count = 1;
    scDesc.Windowed = TRUE;
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    // デバイスとスワップチェーン作成
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        nullptr, 0, D3D11_SDK_VERSION,
        &scDesc, &g_swapChain, &g_device, nullptr, &g_context
    );
    if (FAILED(hr)) return false;

    // バックバッファ取得
    ID3D11Texture2D* backBuffer = nullptr;
    hr = g_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    if (FAILED(hr)) return false;

    // レンダーターゲットビュー作成
    hr = g_device->CreateRenderTargetView(backBuffer, nullptr, &g_renderTargetView);
    backBuffer->Release();
    if (FAILED(hr)) return false;

    return true;
}

void BeginDraw(float r, float g, float b, float a)
{
    // レンダーターゲットの設定
    g_context->OMSetRenderTargets(1, &g_renderTargetView, nullptr);

    // ビューポートの設定（1920x1080）
    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = 1920;
    viewport.Height = 1080;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    g_context->RSSetViewports(1, &viewport);

    // 背景色でクリア
    float clearColor[] = { r, g, b, a };
    g_context->ClearRenderTargetView(g_renderTargetView, clearColor);
}

void EndDraw()
{
    g_swapChain->Present(1, 0);
}

void CleanupDirectX()
{
    if (g_renderTargetView) g_renderTargetView->Release();
    if (g_swapChain) g_swapChain->Release();
    if (g_context) g_context->Release();
    if (g_device) g_device->Release();
}

// デバイスとコンテキスト取得用
ID3D11Device* GetDevice() { return g_device; }
ID3D11DeviceContext* GetContext() { return g_context; }
