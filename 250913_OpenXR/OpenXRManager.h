#pragma once

// まずマクロ定義（OpenXR ヘッダより前）
#define XR_USE_PLATFORM_WIN32       //WindowsプラットフォームでのOpenXR開発を有効にします。
#define XR_USE_GRAPHICS_API_D3D12   //DirectX12グラフィックスAPIのサポートを有効にします。

// Windows / D3D12 を先に読み込む

#include <windows.h>    //OutputDebugStringAを使うのに必要
#include <d3d12.h>
#include "d3dx12.h"
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


    bool Initialize(ID3D12Device* d3d12Device, ID3D12CommandQueue* commQueue);

    bool CreateReferenceSpace(bool preferStage);

    bool CreateSwapchains(ID3D12Device* d3d12Device, 
        uint32_t viewCount,
        DXGI_FORMAT colorFmt,   // 例: DXGI_FORMAT_R8G8B8A8_UNORM
        DXGI_FORMAT depthFmt,   // 例: DXGI_FORMAT_D32_FLOAT
        XMINT2 size);           // 作る解像度（例: recommendedScaledResolution）


    XrInstance GetInstance() const;
    XrSession  GetSession() const;
    XrSystemId GetSystemId() const;
    XrSpace    GetAppSpace()   const { return xr_appSpace; }


    /*
    struct EyeViews {
        std::vector<XrView> views;
        XrViewState viewState{};
    };
    */

    struct EyeMatrix
    {
        XMMATRIX viewMat;           // ビュー行列 (World→View)
        XMMATRIX projMat;           // プロジェクション行列
        XrView   xrView;            // OpenXRの生データ
    };

    // 片目へ“直描き”するときに使うターゲット情報
    struct EyeDirectTarget {
        D3D12_CPU_DESCRIPTOR_HANDLE rtv;  // このフレームのスワップチェーン画像を指すRTV
        D3D12_CPU_DESCRIPTOR_HANDLE dsv;  // 同じくDepth
        XMINT2 size;                      // ビューポート/シザー用（CreateSwapchains時のサイズ）
    };
    


    bool Start_XR_Session();

    bool BeginFrame(XrTime& predictedDisplayTime);

    XMMATRIX CreateCameraViewMatrix(const XrPosef& pose);
    //XMMATRIX XMMatrixFromXrPoseLH(const XrPosef& pose);

    XMMATRIX CreateProjectionMatrix(const XrFovf& fov, float nearZ, float farZ);

    bool GetEyeMatrix(XrTime predictedDisplayTime, float nearZ, float farZ, std::vector<EyeMatrix>& outEyes);

    //bool GetStereoViews(XrTime predictedDisplayTime, EyeViews& out);

    bool BeginEyeDirect(
        ID3D12GraphicsCommandList* cmd,
        uint32_t eyeIndex,
        EyeDirectTarget& out);

    bool EndEyeDirect(
        ID3D12GraphicsCommandList* cmd,
        uint32_t eyeIndex);


    bool CopyOneViewToSwapchain(
        ID3D12GraphicsCommandList* cmd,
        ID3D12Resource* myColorRT, ID3D12Resource* myDepth,
        XrSwapchain colorChain, std::vector<XrSwapchainImageD3D12KHR>& colorImgs,
        XrSwapchain depthChain, std::vector<XrSwapchainImageD3D12KHR>& depthImgs);

    bool EndFrameWithProjection(
        const std::vector<EyeMatrix>& eyesData,
        float nearZ, float farZ,
        //uint32_t viewCount,
        //const std::vector<XrSwapchain>& colorChains,
        //const std::vector<XrSwapchain>& depthChains,
        XMINT2 size, XrTime displayTime);



    bool End_XR_Session();  // アプリ側から終了要求する（xrEndSession実行）

    void OnDestroy();

    //おすすめ解像度
    static XMINT2 recommendedResolution;
    static XMINT2 recommendedScaledResolution;//0.8

    XrExtent2Df stageSize = { 0.0f, 0.0f };

    bool isStageSpace = false;  //ステージスペースかどうか


    static uint32_t xr_viewCount; //ビューの数。両目 = 2

private:

    //ID3D12Device* d3d12Device_;

    XrInstance xr_instance = XR_NULL_HANDLE;
    XrSystemId xr_systemId = XR_NULL_SYSTEM_ID;
    XrSession  xr_session = XR_NULL_HANDLE;
    XrSpace    xr_appSpace = XR_NULL_HANDLE;

    

    // view ごとに 1本ずつ（片目ずつ）持つ想定
    std::vector<XrSwapchain> xr_viewChainsColor;
    std::vector<XrSwapchain> xr_viewChainsDepth;

    // 各チェーンが持つ D3D12 リソース（ランタイムから受け取る）
    std::vector<std::vector<XrSwapchainImageD3D12KHR>> xr_colorImagesPerView;
    std::vector<std::vector<XrSwapchainImageD3D12KHR>> xr_depthImagesPerView;

    std::vector<uint32_t> eyeActiveColorIndex;
    std::vector<uint32_t> eyeActiveDepthIndex;

    std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> rtvHeaps_;
    std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> dsvHeaps_;

    std::vector<std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>> rtvHandles_;
    std::vector<std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>> dsvHandles_;

    bool CreateInstance();
    //bool checkAndLoadExtensions();
    bool GetSystemId();
    //bool loadD3D12ReqFunc();
    bool CheckGraphicsRequirements(ID3D12Device* d3d12Device);
    bool CreateSession(ID3D12Device* d3d12Device, ID3D12CommandQueue* commQueue);

    XrSessionState xr_sessionState = XR_SESSION_STATE_UNKNOWN;
    bool xr_sessionRunning = false;

    void UpdateSessionState();                  // セッション情報を更新
    bool EndSessionGracefully(uint32_t msTimeout = 2000); // 終了手順

    void DestroySwapchains();        // スワップチェーン破棄



};


