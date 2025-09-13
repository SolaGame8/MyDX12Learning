#include "WindowManager.h"


// ウィンドウプロシージャ
LRESULT CALLBACK WindowManager::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    //ウィンドウのイベント

    // ここで、ウィンドウ通知 一覧を見れます
    //https://learn.microsoft.com/ja-jp/windows/win32/winmsg/window-notifications

    switch (uMsg) {             //送られてきたウィンドウ通知(uMsg)によって分岐

    case WM_CLOSE:              //ウインドウを閉じた
    case WM_DESTROY:            //ウインドウをxボタンとかで消した時
        PostQuitMessage(0);     //終了メッセージ
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


// コンストラクタ
WindowManager::WindowManager(int width, int height, const wchar_t* title)
    : m_width(width)
    , m_height(height)
    , m_hwnd(nullptr) {


    // ウィンドウクラスの登録


    WNDCLASSEX wc = { 0 };

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;

    wc.lpfnWndProc = WindowProc;                            //上記の関数（WindowProc）でイベントを受け取る、と指定

    wc.hInstance = GetModuleHandle(nullptr);                //インスタンス
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);            //カーソル

    wc.lpszClassName = L"DirectX12WindowClass";             //自分で決める。このクラス名が存在するかチェックすれば、二重起動を防げる

    wc.hbrBackground = CreateSolidBrush(RGB(64, 64, 64));   //背景色



    if (!RegisterClassEx(&wc)) {    //クラスをシステムに登録

        // 登録に失敗した場合
        MessageBox(nullptr, L"Window class registration failed!", L"Error", MB_ICONERROR | MB_OK);  //メッセージボックス
        return;
    }



    // ウィンドウの作成


    m_hwnd = CreateWindowEx(
        0,                              
        wc.lpszClassName,               //クラス名
        title,                          //タイトルバーに表示される
        WS_OVERLAPPEDWINDOW,            //スタイル
        CW_USEDEFAULT, CW_USEDEFAULT,   // x, y
        width, height,                  // w, h
        nullptr,                        //親ウィンドウ
        nullptr,                        //メニュー
        wc.hInstance,                   //インスタンス
        nullptr
    );

    if (!m_hwnd) {

        // 作成に失敗した場合
        MessageBox(nullptr, L"Window creation failed!", L"Error", MB_ICONERROR | MB_OK);
        return;
    }

    // ウィンドウを表示
    ShowWindow(m_hwnd, SW_SHOW);

}

// デストラクタ
WindowManager::~WindowManager() {

    //ウインドウを破棄
    DestroyWindow(m_hwnd);

}

// メッセージ処理
bool WindowManager::ProcessMessages() const {              //main.cppのメインループの while 条件 で使用されている

    MSG msg = { 0 };
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {   //メッセージの取得

        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (msg.message == WM_QUIT) {   //終了        //上記の PostQuitMessage(0); が実行されると、WM_QUITが送られる
            return false;
        }
    }
    return true;

}


