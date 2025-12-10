#include "System/Window.h"
#include "Graphics/DirectXGraphics.h"
#include "../common_src/Game/Game.h"
#include "System/Time.h"
#include <memory>
#include <combaseapi.h>
#include "Input/XInputGamepad.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    // --- 各モジュールの生成 ---
    Window window;
    auto graphics = std::make_unique<DirectXGraphics>();
    auto game = std::make_unique<Game>(graphics.get());

    // --- ウィンドウ作成（ハンドルが必要） ---
    window.Create(hInstance, nCmdShow, 1920, 1080);

    // ゲームパッド: まずは XInput のみで様子を見る
    auto gamepad = std::make_unique<XInputGamepad>();
    game->SetGamepad(gamepad.get());

    // --- 初期化 ---
    if (!graphics->Initialize(window.GetHwnd(), 1920, 1080))
    {
        MessageBox(nullptr, L"Graphics initialization failed!", L"Error", MB_OK);
        return -1;
    }

    InitTime();
    game->Initialize();

    // --- ゲームループ ---
    const double FRAME_RATE = 60.0;
    const double TIME_PER_FRAME = 1.0 / FRAME_RATE;
    double accumulator = 0.0; // 経過時間を溜める変数

    while (!window.ShouldQuit())
    {
        window.ProcessMessage();

        if (game->ShouldQuit()) {
            break;
        }

        // 入力をフレーム先頭で更新
        gamepad->Update();

        UpdateTime();
        // 前のフレームからの経過時間を accumulator に加算
        accumulator += GetElapsedTime();

        // accumulator が1フレーム分の時間を超えている限り、Updateを固定時間で実行
        while (accumulator >= TIME_PER_FRAME)
        {
            game->Update(static_cast<float>(TIME_PER_FRAME));
            accumulator -= TIME_PER_FRAME;
        }

        // 描画は毎フレーム実行する
        game->Draw();
    }

    // --- 終了処理 ---
    game->Terminate();
    graphics->Finalize();
    CoUninitialize();

    return 0;
}