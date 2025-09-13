#pragma once

// まずマクロ定義（OpenXR ヘッダより前）
#define XR_USE_PLATFORM_WIN32       //WindowsプラットフォームでのOpenXR開発を有効にします。
#define XR_USE_GRAPHICS_API_D3D12   //DirectX12グラフィックスAPIのサポートを有効にします。

// Windows / D3D12 を先に読み込む

#include <windows.h>    //OutputDebugStringAを使うのに必要
#include <d3d12.h>
#include <DirectXMath.h>
#include <dxgi1_6.h>
#pragma comment(lib, "dxgi.lib")

// 最後に OpenXR
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#ifdef _DEBUG
    #pragma comment(lib, "openxr_loaderd.lib")  //デバッグ用はライブラリ名が違っていた
#else
    #pragma comment(lib, "openxr_loader.lib")
#endif

#include <vector>

using namespace DirectX;


class OpenXRManager {

public:


    // PCVR（D3D12）向け：VRが使える状態かの軽量判定
    // 依存: OpenXRランタイムが見えること。D3D12デバイスは不要。
    enum class VrSupport {
        Ready,               // ランタイム有 / D3D12拡張あり / HMD検出OK
        NoHmd,               // ランタイムはあるが HMD 未検出
        RuntimeNoD3D12,      // ランタイムに XR_KHR_D3D12_enable が無い
        RuntimeUnavailable,  // ランタイムが見つからない/無効
        InstanceFailed       // 最小インスタンス作成に失敗
    };

    // インスタンス不要で呼べる軽量プローブ（探査する）
    static VrSupport ProbeSupportDX12();

    // 「Readyかどうか」だけ知りたい簡易版
    static bool IsVrReadyForDX12() { return ProbeSupportDX12() == VrSupport::Ready; }


    bool Initialize(ID3D12Device* d3d12Device, ID3D12CommandQueue* commandQ);

    XrInstance GetInstance() const;
    XrSession  GetSession() const;
    XrSystemId GetSystemId() const;

    bool RequestEnd();  // アプリ側から終了要求する（xrEndSession実行）

    void OnDestroy();

    //おすすめ解像度
    static XMINT2 recommendedResolution;
    static XMINT2 recommendedScaledResolution;


private:

    XrInstance m_instance = XR_NULL_HANDLE;
    XrSystemId m_systemId = XR_NULL_SYSTEM_ID;
    XrSession  m_session = XR_NULL_HANDLE;

    // KHR_d3d12 要件取得関数ポインタ
    PFN_xrGetD3D12GraphicsRequirementsKHR m_pfnGetD3D12GraphicsRequirementsKHR = nullptr;

    bool createInstance();
    //bool checkAndLoadExtensions();
    bool getSystemId();
    bool loadD3D12ReqFunc();
    bool checkGraphicsRequirements(ID3D12Device* d3d12Device);
    bool createSession(ID3D12Device* d3d12Device, ID3D12CommandQueue* graphicsQueue);

    XrSessionState m_sessionState = XR_SESSION_STATE_UNKNOWN;
    bool m_sessionRunning = false;

    void PumpEventsOnce();                  // イベント1回ポーリング
    bool EndSessionGracefully(uint32_t msTimeout = 2000); // 終了手順

    



};


