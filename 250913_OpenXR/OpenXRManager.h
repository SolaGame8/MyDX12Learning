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
    #pragma comment(lib, "openxr_loader.lib")   //こっちがリリース用のライブラリ
#endif


#include "OpenXRController.h"       //コントローラーマネージャー


#include <vector>

using namespace DirectX;


class OpenXRManager {

public:


    // VRの状態用 列挙データ
    enum class VrSupport {
        Ready,               // ランタイム有 / D3D12拡張あり / HMD検出OK
        NoHmd,               // ランタイムはあるが HMD 未検出
        RuntimeNoD3D12,      // ランタイムに XR_KHR_D3D12_enable が無い
        RuntimeUnavailable,  // ランタイムが見つからない/無効
        InstanceFailed       // 最小インスタンス作成に失敗
    };

    //両目のカメラの行列データ
    struct EyeMatrix
    {
        XMMATRIX viewMat;           // ビュー行列 (World→View)
        XMMATRIX projMat;           // プロジェクション行列
        XrView   xrView;            // OpenXRの生データ
    };

    // Swapchainターゲット情報
    struct EyeDirectTarget {
        D3D12_CPU_DESCRIPTOR_HANDLE rtv;  // レンダーターゲット
        D3D12_CPU_DESCRIPTOR_HANDLE dsv;  // 深度バッファ
        XMINT2 size;                      // このターゲットの解像度
    };



    // VRの状態を取得
    static VrSupport CheckVRSupport(float resoScale);


    bool Initialize(ID3D12Device* d3d12Device, ID3D12CommandQueue* commQueue);

    bool CreateReferenceSpace(bool preferStage);

    bool CreateSwapchains(ID3D12Device* d3d12Device, 
        uint32_t viewCount,
        DXGI_FORMAT colorFmt,   // DXGI_FORMAT_R8G8B8A8_UNORM
        DXGI_FORMAT depthFmt,   // DXGI_FORMAT_D32_FLOAT
        XMINT2 size);           // 作る解像度

    bool InitControllers();

    bool Start_XR_Session();    //＊セッション開始

    bool BeginFrame(XrTime& predictedDisplayTime);

    XMMATRIX CreateCameraViewMatrix(const XrPosef& pose);
    XMMATRIX CreateProjectionMatrix(const XrFovf& fov, float nearZ, float farZ);

    bool GetEyeMatrix(XrTime predictedDisplayTime, float nearZ, float farZ, std::vector<EyeMatrix>& outEyes);

    bool GetSwapchainDrawTarget(ID3D12GraphicsCommandList* cmd, uint32_t eyeIndex, EyeDirectTarget& out);
    bool FinishSwapchainDrawTarget(ID3D12GraphicsCommandList* cmd, uint32_t eyeIndex);

    bool EndFrame_WithProjection(const std::vector<EyeMatrix>& eyesData, float nearZ, float farZ, XrTime displayTime);

    bool End_XR_Session();      //＊セッション終了


    void UpdateSessionState();  // セッション情報を更新   （これを毎フレーム実行しないと、コントローラーの情報も更新されないので注意


    void OnDestroy();


    //おすすめ解像度
    static float resolutionScale;

    static XMINT2 recommendedResolution;        //ヘッドマウントから取得するおすすめ解像度
    static XMINT2 recommendedScaledResolution;  //0.8倍にして、使用しています

    XrExtent2Df stageSize = { 0.0f, 0.0f };

    bool isStageSpace = false;                  //ステージスペースかどうか。（ステージスペースは床の位置がある。ローカルスペースはヘッドマウント基準のスペース）


    static uint32_t xr_viewCount;               //ビューの数。両目 = 2

    //コントローラー
    OpenXRController controller;
    bool controllersReady = false;


private:


    XrInstance xr_instance = XR_NULL_HANDLE;
    XrSystemId xr_systemId = XR_NULL_SYSTEM_ID;
    XrSession  xr_session = XR_NULL_HANDLE;
    XrSpace    xr_appSpace = XR_NULL_HANDLE;


    // Swapchain
    std::vector<XrSwapchain> xr_viewChainsColor;
    std::vector<XrSwapchain> xr_viewChainsDepth;

    std::vector<std::vector<XrSwapchainImageD3D12KHR>> xr_colorSwapchainImage;
    std::vector<std::vector<XrSwapchainImageD3D12KHR>> xr_depthSwapchainImage;

    std::vector<uint32_t> eyeActiveColorIndex;
    std::vector<uint32_t> eyeActiveDepthIndex;

    std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> rtvHeaps_;
    std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> dsvHeaps_;

    std::vector<std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>> rtvHandles_;
    std::vector<std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>> dsvHandles_;


    bool CreateInstance();
    bool GetSystemId();
    bool CheckGraphicsRequirements(ID3D12Device* d3d12Device);
    bool CreateSession(ID3D12Device* d3d12Device, ID3D12CommandQueue* commQueue);

    XrSessionState xr_sessionState = XR_SESSION_STATE_UNKNOWN;
    bool xr_sessionRunning = false;

    
    bool EndSessionGracefully(uint32_t msTimeout = 2000); // 終了手順

    void DestroySwapchains();        // スワップチェーン破棄





};


