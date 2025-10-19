

#include "Windows.h"        //ウインドウズ

#include "WindowManager.h"  //ウインドウを開く自作クラス
#include "DirectX12App.h"   //DirectX12の初期化と実行をする自作クラス


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {



    // ウィンドウの作成
    const int windowWidth = 1280;
    const int windowHeight = 720;

    WindowManager windowManager(windowWidth, windowHeight, L"DirectX 12 Learn"); //w, h, ウインドウ名（ヘッダーに表示される）

    // DirectX 12アプリケーションの作成と初期化

    DirectX12App Dx12App;
    if (!Dx12App.Initialize(windowManager.GetHandle())) {  //ウインドウのハンドルを渡して初期化
        return 1; // 初期化失敗（アプリケーション終了）
    }


    // メインループ

    while (windowManager.ProcessMessages()) {      //ウインドウが閉じられた時などにfalseになる

        Dx12App.Render();   // DirectX12描画処理ループ

    }

    //終了処理
    Dx12App.OnDestroy();    //グラフィックボードの処理が完全に終わってから、アプリケーション終了する




    return 0;   //アプリケーション終了
}




