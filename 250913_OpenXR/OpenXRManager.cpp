#include "OpenXRManager.h"
#include <sstream>
#include <cstring>
//#include <algorithm>

#include <chrono>
#include <thread>

XMINT2 OpenXRManager::recommendedResolution = { 1024, 1024 };
XMINT2 OpenXRManager::recommendedScaledResolution = { 1024, 1024 };

uint32_t OpenXRManager::xr_viewCount = 0; //ビューの数。両目 = 2 になるはず

OpenXRManager::VrSupport OpenXRManager::ProbeSupportDX12() {


    //VR機器の接続状況をチェックする

    //ランタイムと拡張の列挙
    OutputDebugStringA("ProbeSupportDX12\n");

    //拡張機能の 数のみ を取得
    uint32_t extCount = 0;
    XrResult r = xrEnumerateInstanceExtensionProperties(nullptr, 0, &extCount, nullptr);
    if (!XR_SUCCEEDED(r) || extCount == 0) {
        OutputDebugStringA("[ProbeXR] Runtime unavailable (enumerate failed).\n");
        return VrSupport::RuntimeUnavailable;
    }


    std::vector<XrExtensionProperties> exts;
    exts.resize(extCount);  // 必要な数だけ確保

    for (uint32_t i = 0; i < extCount; ++i) {
        exts[i] = {};  // ゼロ初期化
        exts[i].type = XR_TYPE_EXTENSION_PROPERTIES;
    }

    //拡張機能の列挙データを取得
    r = xrEnumerateInstanceExtensionProperties(nullptr, extCount, &extCount, exts.data());
    if (!XR_SUCCEEDED(r)) {
        OutputDebugStringA("[ProbeXR] enumerate extensions failed.\n");
        return VrSupport::RuntimeUnavailable;
    }



    //DX12の機能拡張が使えるか確認

    bool found = false;
    for (auto& e : exts) {
        if (strcmp(e.extensionName, XR_KHR_D3D12_ENABLE_EXTENSION_NAME) == 0) { //"XR_KHR_D3D12_enable"があるかどうか
            found = true;   // D3D12 が使える
            break;
        }
    }

    if (!found) {
        OutputDebugStringA("[ProbeXR] XR_KHR_D3D12_enable not supported.\n");
        return VrSupport::RuntimeNoD3D12;
    }


    // 最小インスタンスを作ってみて、接続状況を確認

    XrApplicationInfo appInfo{};    //アプリ情報

    {
        strncpy_s(appInfo.applicationName, "VR Probe", XR_MAX_APPLICATION_NAME_SIZE - 1);   //char[128]に文字をコピー
        appInfo.applicationVersion = 1;
        strncpy_s(appInfo.engineName, "Probe", XR_MAX_ENGINE_NAME_SIZE - 1);
        appInfo.engineVersion = 1;
        appInfo.apiVersion = XR_CURRENT_API_VERSION;
    }

    XrInstanceCreateInfo ci = {};   //インスタンス生成情報

    {
        ci.type = XR_TYPE_INSTANCE_CREATE_INFO;  // 必須フィールドを設定
        ci.applicationInfo = appInfo;

        ci.enabledExtensionCount = 0;           //拡張機能（D3D12_enable）を指定しないで、不要な負荷を避ける
        ci.enabledExtensionNames = nullptr;
    }

    //インスタンスの作成

    XrInstance inst = XR_NULL_HANDLE;
    r = xrCreateInstance(&ci, &inst);

    if (!XR_SUCCEEDED(r) || inst == XR_NULL_HANDLE) {
        std::ostringstream oss; oss << "[ProbeXR] xrCreateInstance failed: " << r << "\n";
        OutputDebugStringA(oss.str().c_str());
        return VrSupport::InstanceFailed;
    }

    // HMD（SYSTEM）の有無

    XrSystemId systemId = XR_NULL_SYSTEM_ID;    //システムID

    XrSystemGetInfo si = {};                    //システム取得情報
    {
        si.type = XR_TYPE_SYSTEM_GET_INFO;
        si.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;    //ヘッドマウント
    }
    
    r = xrGetSystem(inst, &si, &systemId);      //システムID情報取得

    VrSupport result = VrSupport::NoHmd;

    if (XR_SUCCEEDED(r)) {  //システムIDが取得出来ていたら

        result = VrSupport::Ready;

        // システムプロパティの取得
        XrSystemProperties props = {};
        props.type = XR_TYPE_SYSTEM_PROPERTIES;
        XrResult r = xrGetSystemProperties(inst, systemId, &props);

        if (XR_SUCCEEDED(r)){

            //システム情報表示
            std::ostringstream oss;
            oss << "[XR] SystemName: " << props.systemName << "\n"
                << "[XR] VendorId : " << props.vendorId << "\n"
                << "[XR] MaxLayerCount: " << props.graphicsProperties.maxLayerCount << "\n"
                << "[XR] MaxSwapchainImageWidth : " << props.graphicsProperties.maxSwapchainImageWidth << "\n"      //扱える最大の解像度
                << "[XR] MaxSwapchainImageHeight: " << props.graphicsProperties.maxSwapchainImageHeight << "\n"
                << "[XR] OrientationTracking: " << (props.trackingProperties.orientationTracking ? "Yes" : "No") << "\n"
                << "[XR] PositionTracking   : " << (props.trackingProperties.positionTracking ? "Yes" : "No") << "\n";
            OutputDebugStringA(oss.str().c_str());

            //おすすめ解像度

            //uint32_t viewCount = 0; //viewの個数を取得
            xrEnumerateViewConfigurationViews(inst, systemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, 0, &xr_viewCount, nullptr);


            std::vector<XrViewConfigurationView> views;
            views.resize(xr_viewCount);

            for (uint32_t i = 0; i < xr_viewCount; i++) {
                views[i] = {};
                views[i].type = XR_TYPE_VIEW_CONFIGURATION_VIEW;
                views[i].next = nullptr;
            }

            xrEnumerateViewConfigurationViews(inst, systemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, xr_viewCount, &xr_viewCount, views.data());


            //スケールダウンした解像度を計算

            const float scaleDownRate = 0.8f;

            for (uint32_t i = 0; i < xr_viewCount; i++) {

                // スケールダウン後の浮動小数を計算
                float fw = (float)views[i].recommendedImageRectWidth * scaleDownRate;
                float fh = (float)views[i].recommendedImageRectHeight * scaleDownRate;

                // 四捨五入して整数に
                int w = (int)std::lround(fw);
                int h = (int)std::lround(fh);

                // 16の倍数に丸める    （気にならない程度の縦横比の誤差は出る）
                int align = 16;

                // w
                int wDown = (w / align) * align;                        //普通に16で丸める
                int wUp = wDown + align;                                //それよりも16多い数値
                if ((w - wDown) <= (wUp - w)) w = wDown; else w = wUp;  //近い方の数値を選ぶ

                // h
                int hDown = (h / align) * align;
                int hUp = hDown + align;
                if ((h - hDown) <= (hUp - h)) h = hDown; else h = hUp;

                // 出力
                char buf[256];
                sprintf_s(buf,
                    "[XR] View %u: Recommended %u x %u, ScaleDown(%0.1f) -> %d x %d\n",
                    i,
                    views[i].recommendedImageRectWidth,
                    views[i].recommendedImageRectHeight,
                    scaleDownRate,
                    w, h);
                    //views[i].maxImageRectWidth,             //maxは swapchain の限界値
                    //views[i].maxImageRectHeight);

                OutputDebugStringA(buf);

                //おすすめ解像度
                recommendedResolution.x = views[i].recommendedImageRectWidth;
                recommendedResolution.y = views[i].recommendedImageRectHeight;

                //スケールダウン(0.8) 16の倍数になっている
                recommendedScaledResolution.x = w;
                recommendedScaledResolution.y = h;

            }

        }
        else {
            OutputDebugStringA("[XR] No Properties...\n");

        }


    }
    else {
        std::ostringstream oss; oss << "[XR] xrGetSystem failed: " << r << "\n";
        OutputDebugStringA(oss.str().c_str());
        result = VrSupport::NoHmd;
    }

    xrDestroyInstance(inst);
    return result;
}


bool OpenXRManager::Initialize(ID3D12Device* d3d12Device, ID3D12CommandQueue* commQueue) {

    //初期化


    //入力チェック
    if (!d3d12Device || !commQueue) {
        OutputDebugStringA("Invalid D3D12 device or queue.\n");
        return false;
    }

    //d3d12Device_ = d3d12Device;


    if (!CreateInstance()) return false;                            //インスタンス作成
    if (!GetSystemId())    return false;                            //システムIDの取得
    if (!CheckGraphicsRequirements(d3d12Device)) return false;      //VR機器の要求スペックを満たしているか確認

    if (!CreateSession(d3d12Device, commQueue)) return false;        //セッション作成




    if (!CreateReferenceSpace(true)) return false;                  //スペースの作成（bool ステージ優先にするかどうか）

    /*
    STAGE
        VR ランタイムが「床の原点」と「ユーザーの身長」を認識している場合に使える座標系
        ルームスケール環境（例: SteamVR のルームセットアップ、Oculus Guardian）があるときに利用可能
        ワールドに「床の高さ」や「部屋の広さ」がしっかり反映される

    LOCAL
        HMD の「初期位置」を基準としたローカル座標系
        頭に追従する感覚で、ルームスケールが無い環境でも必ず使えます
        座位・立位どちらでも使える、最も「安全な選択肢」

    */






    //スワップチェーン
    if (!CreateSwapchains(
        d3d12Device, xr_viewCount,                   //2
        DXGI_FORMAT_R8G8B8A8_UNORM,     //colorフォーマット
        DXGI_FORMAT_D32_FLOAT,          //depthフォーマット
        recommendedScaledResolution     //解像度
        )) return false;


    //セッションスタート
    Start_XR_Session();


    //コントローラー初期化

    (void)InitControllers();

    //InitSimpleControllerTest();



    OutputDebugStringA("OpenXR initialized successfully.\n");

    return true;
}

XrInstance OpenXRManager::GetInstance() const { return xr_instance; }
XrSession  OpenXRManager::GetSession()  const { return xr_session; }
XrSystemId OpenXRManager::GetSystemId() const { return xr_systemId; }

bool OpenXRManager::CreateInstance() {


    //インスタンスの作成

    const char* extensions[] = {
        XR_KHR_D3D12_ENABLE_EXTENSION_NAME      //"XR_KHR_D3D12_enable"
    };


    XrApplicationInfo appInfo{};

    {
        strncpy_s(appInfo.applicationName, "My OpenXR App", XR_MAX_APPLICATION_NAME_SIZE - 1);  //アプリケーション名
        appInfo.applicationVersion = (1u << 16) | (0u << 8) | 0u; // 1.0.0 など任意
        strncpy_s(appInfo.engineName, "DX12 FromScratch", XR_MAX_ENGINE_NAME_SIZE - 1);         //エンジン名（DX12 スクラッチ
        //appInfo.engineVersion = 1;
        appInfo.engineVersion = (1u << 16) | (0u << 8) | 0u; // 1.0.0 など任意
        appInfo.apiVersion = XR_CURRENT_API_VERSION;
    }


    XrInstanceCreateInfo ci = {};
    {
        ci.type = XR_TYPE_INSTANCE_CREATE_INFO; // 構造体の種類を指定
        ci.applicationInfo = appInfo;
        ci.enabledExtensionCount = (uint32_t)(sizeof(extensions) / sizeof(extensions[0]));
        ci.enabledExtensionNames = extensions;
    }

    XrResult r = xrCreateInstance(&ci, &xr_instance);
    
    if (!XR_SUCCEEDED(r)) {
        std::ostringstream oss;
        oss << "xrCreateInstance failed: " << r << "\n";
        OutputDebugStringA(oss.str().c_str());
        return false;
    }
    
    
    
    return true;
}

bool OpenXRManager::GetSystemId() {


    //（ヘッドマウントの）システムIDを取得

    XrSystemGetInfo si = {};
    {
        si.type = XR_TYPE_SYSTEM_GET_INFO;
        si.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;    //ヘッドマウント
    }


    XrResult r = xrGetSystem(xr_instance, &si, &xr_systemId);     //システムIDを取得


    if (!XR_SUCCEEDED(r)) {
        std::ostringstream oss;
        oss << "xrGetSystem failed: " << r << "\n";
        OutputDebugStringA(oss.str().c_str());
        return false;
    }
    return true;
}


/*
bool OpenXRManager::loadD3D12ReqFunc() {

    //関数のアドレスを取得

    XrResult r = xrGetInstanceProcAddr(
        xr_instance,
        "xrGetD3D12GraphicsRequirementsKHR",
        reinterpret_cast<PFN_xrVoidFunction*>(&ptrfunc_D3D12Requirements)
    );

    if (!XR_SUCCEEDED(r) || !ptrfunc_D3D12Requirements) {
        std::ostringstream oss;
        oss << "xrGetInstanceProcAddr(xrGetD3D12GraphicsRequirementsKHR) failed: " << r << "\n";
        OutputDebugStringA(oss.str().c_str());
        return false;
    }
    return true;
}

bool OpenXRManager::checkGraphicsRequirements(ID3D12Device* d3d12Device) {

    //XrGraphicsRequirementsD3D12KHR req{ XR_TYPE_GRAPHICS_REQUIREMENTS_D3D12_KHR };
    XrGraphicsRequirementsD3D12KHR req = {};         // 全フィールドをゼロ初期化
    req.type = XR_TYPE_GRAPHICS_REQUIREMENTS_D3D12_KHR; // 構造体の種類を指定
    req.next = nullptr;                              // 拡張を使わないので null

    XrResult r = ptrfunc_D3D12Requirements(xr_instance, xr_systemId, &req);
    if (!XR_SUCCEEDED(r)) {
        std::ostringstream oss;
        oss << "xrGetD3D12GraphicsRequirementsKHR failed: " << r << "\n";
        OutputDebugStringA(oss.str().c_str());
        return false;
    }

    LUID deviceLuid = d3d12Device->GetAdapterLuid();
    if (std::memcmp(&deviceLuid, &req.adapterLuid, sizeof(LUID)) != 0) {
        OutputDebugStringA("Adapter LUID mismatch.\n");
        return false;
    }

    D3D12_FEATURE_DATA_FEATURE_LEVELS fls{};
    static const D3D_FEATURE_LEVEL levels[] = {
        D3D_FEATURE_LEVEL_12_2,
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };
    fls.NumFeatureLevels = (UINT)(sizeof(levels) / sizeof(levels[0]));
    fls.pFeatureLevelsRequested = levels;
    if (FAILED(d3d12Device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &fls, sizeof(fls)))) {
        OutputDebugStringA("CheckFeatureSupport failed.\n");
        return false;
    }

    if (fls.MaxSupportedFeatureLevel < (D3D_FEATURE_LEVEL)req.minFeatureLevel) {
        OutputDebugStringA("Feature level too low.\n");
        return false;
    }
    return true;
}
*/


bool OpenXRManager::CheckGraphicsRequirements(ID3D12Device* d3d12Device) {

    if (!d3d12Device) return false;

    // VR機器側のD3D12ランタイム関数のポインタを取得（この関数から要求を取得する
    PFN_xrGetD3D12GraphicsRequirementsKHR ptrfunc_D3D12Requirements = nullptr;

    XrResult r = xrGetInstanceProcAddr(
        xr_instance,
        "xrGetD3D12GraphicsRequirementsKHR",
        reinterpret_cast<PFN_xrVoidFunction*>(&ptrfunc_D3D12Requirements)
    );

    if (!XR_SUCCEEDED(r) || !ptrfunc_D3D12Requirements) {
        OutputDebugStringA("xrGetInstanceProcAddr(xrGetD3D12GraphicsRequirementsKHR) failed.\n");
        return false;
    }

    // VR機器が要求する条件の取得（同じGPUを使っているか、レベルは満たしているか）
    XrGraphicsRequirementsD3D12KHR xr_request = {};
    {
        xr_request.type = XR_TYPE_GRAPHICS_REQUIREMENTS_D3D12_KHR;
        xr_request.next = nullptr;
    }

    r = ptrfunc_D3D12Requirements(xr_instance, xr_systemId, &xr_request);  //関数のポインタをつかって、要求を取得

    if (!XR_SUCCEEDED(r)) {
        OutputDebugStringA("xrGetD3D12GraphicsRequirementsKHR failed.\n");
        return false;
    }

    // LUID 一致チェック  //LUID : Locally Unique Identifier
    // （DirectX12と、ヘッドマウントが、きちんと同じGPUを使用できているか確認する）

    /*
        // LUID不一致が起きる可能性があるケース
        
        ノートPC（内蔵 GPU + 外付け GPU）
        デスクトップで複数 GPU が挿さっている場合
        リモート環境や仮想化環境
    */

    LUID deviceLuid = d3d12Device->GetAdapterLuid();
    
    if (std::memcmp(&deviceLuid, &xr_request.adapterLuid, sizeof(LUID)) != 0) {
        OutputDebugStringA("Adapter LUID mismatch.\n");
        return false;
    }

    // Feature Level チェック

    // PCのグラフィックボードがどのレベルまで対応しているか取得
    D3D12_FEATURE_DATA_FEATURE_LEVELS fls{};
    static const D3D_FEATURE_LEVEL levels[] = {
        D3D_FEATURE_LEVEL_12_2,
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
    };
    fls.NumFeatureLevels = (UINT)(sizeof(levels) / sizeof(levels[0]));
    fls.pFeatureLevelsRequested = levels;

    if (FAILED(d3d12Device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &fls, sizeof(fls)))) {
        OutputDebugStringA("CheckFeatureSupport failed.\n");
        return false;
    }

    // 要求レベル比較

    D3D_FEATURE_LEVEL required = xr_request.minFeatureLevel;   //VR機器側が要求する最低限のレベル

    if (fls.MaxSupportedFeatureLevel < required) {      //PCのグラフィックボードの最大レベルより、要求の方が大きかったら失敗
        OutputDebugStringA("Feature level too low.\n"); //PCが、VR機器に必要な条件を満たせなかった
        return false;
    }



    return true;
}



bool OpenXRManager::CreateSession(ID3D12Device* d3d12Device, ID3D12CommandQueue* commQueue) {

    //セッション作成

    if (!d3d12Device || !commQueue) return false;

    XrGraphicsBindingD3D12KHR gb = {};
    gb.type = XR_TYPE_GRAPHICS_BINDING_D3D12_KHR;
    gb.next = nullptr;
    gb.device = d3d12Device;
    gb.queue = commQueue;

    XrSessionCreateInfo sci = {};
    sci.type = XR_TYPE_SESSION_CREATE_INFO;
    sci.next = &gb;
    sci.systemId = xr_systemId;

    XrResult r = xrCreateSession(xr_instance, &sci, &xr_session);

    if (!XR_SUCCEEDED(r)) {
        std::ostringstream oss;
        oss << "xrCreateSession failed: " << r << "\n";
        OutputDebugStringA(oss.str().c_str());
        return false;
    }
    return true;
}




// クラスのメンバにこれがある前提
// XrSpace xr_appSpace = XR_NULL_HANDLE;

bool OpenXRManager::CreateReferenceSpace(bool preferStage) {

    if (xr_session == XR_NULL_HANDLE) {
        OutputDebugStringA("CreateReferenceSpace: session is null.\n");
        return false;
    }

    // 原点そのまま（回転=単位、位置=0）のポーズ
    XrReferenceSpaceCreateInfo ci = {};
    {
        ci.type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO;
        ci.poseInReferenceSpace.orientation.x = 0.0f;
        ci.poseInReferenceSpace.orientation.y = 0.0f;
        ci.poseInReferenceSpace.orientation.z = 0.0f;
        ci.poseInReferenceSpace.orientation.w = 1.0f; // 単位クォータニオン
        ci.poseInReferenceSpace.position.x = 0.0f;
        ci.poseInReferenceSpace.position.y = 0.0f;
        ci.poseInReferenceSpace.position.z = 0.0f;
    }

    // 優先：STAGE → LOCAL（または逆）
    XrReferenceSpaceType tryFirst = preferStage ? XR_REFERENCE_SPACE_TYPE_STAGE : XR_REFERENCE_SPACE_TYPE_LOCAL;
    XrReferenceSpaceType trySecond = preferStage ? XR_REFERENCE_SPACE_TYPE_LOCAL : XR_REFERENCE_SPACE_TYPE_STAGE;

    // まず第一候補を試す
    ci.referenceSpaceType = tryFirst;

    XrResult r = xrCreateReferenceSpace(xr_session, &ci, &xr_appSpace); //スペース作成

    if (XR_SUCCEEDED(r)) {

        if (tryFirst == XR_REFERENCE_SPACE_TYPE_STAGE) {

            isStageSpace = true;

            OutputDebugStringA("[XR] Created STAGE space.\n");

            r = xrGetReferenceSpaceBoundsRect(xr_session, XR_REFERENCE_SPACE_TYPE_STAGE, &stageSize);  //プレイ可能エリアのサイズ（中心からの距離）

            if (XR_SUCCEEDED(r)) {
                char buf[128];
                sprintf_s(buf, "[XR] STAGE bounds: %.2fm x %.2fm\n", stageSize.width, stageSize.height);
                OutputDebugStringA(buf);
            }
        }
        else {
            OutputDebugStringA("[XR] Created LOCAL space.\n");
        }
        return true;
    }

    // 第一候補が未対応などで失敗したら第二候補を試す
    ci.referenceSpaceType = trySecond;

    r = xrCreateReferenceSpace(xr_session, &ci, &xr_appSpace); //スペース作成
    
    if (XR_SUCCEEDED(r)) {

        if (trySecond == XR_REFERENCE_SPACE_TYPE_STAGE) {

            isStageSpace = true;

            OutputDebugStringA("[XR] Created STAGE space (fallback).\n");

            r = xrGetReferenceSpaceBoundsRect(xr_session, XR_REFERENCE_SPACE_TYPE_STAGE, &stageSize);  //プレイ可能エリアのサイズ（中心からの距離）

            if (XR_SUCCEEDED(r)) {
                char buf[128];
                sprintf_s(buf, "[XR] STAGE bounds: %.2fm x %.2fm\n", stageSize.width, stageSize.height);
                OutputDebugStringA(buf);
            }
        }
        else {
            OutputDebugStringA("[XR] Created LOCAL space (fallback).\n");
        }
        return true;
    }

    OutputDebugStringA("[XR] Failed to create reference space (LOCAL/STAGE unsupported?).\n");
    return false;
}




bool OpenXRManager::CreateSwapchains(
    ID3D12Device* d3d12Device, 
    uint32_t viewCount,
    DXGI_FORMAT colorFmt,
    DXGI_FORMAT depthFmt,
    XMINT2 size) {


    //スワップチェーン作成

    if (xr_session == XR_NULL_HANDLE) {
        OutputDebugStringA("CreateSwapchains: session is null.\n");
        return false;
    }

    // 事前クリア（再作成に備える）
    for (auto sc : xr_viewChainsColor) if (sc != XR_NULL_HANDLE) xrDestroySwapchain(sc);
    for (auto sc : xr_viewChainsDepth) if (sc != XR_NULL_HANDLE) xrDestroySwapchain(sc);
    xr_viewChainsColor.clear();
    xr_viewChainsDepth.clear();
    xr_colorImagesPerView.clear();
    xr_depthImagesPerView.clear();

    xr_viewChainsColor.resize(viewCount, XR_NULL_HANDLE);
    xr_viewChainsDepth.resize(viewCount, XR_NULL_HANDLE);
    xr_colorImagesPerView.resize(viewCount);
    xr_depthImagesPerView.resize(viewCount);

    eyeActiveColorIndex.resize(viewCount);
    eyeActiveDepthIndex.resize(viewCount);


    // view ごとに Color / Depth スワップチェーンを作る

    for (uint32_t i = 0; i < viewCount; ++i) {

        // Color chain
        {
            XrSwapchainCreateInfo sci = {};
            sci.type = XR_TYPE_SWAPCHAIN_CREATE_INFO;
            sci.createFlags = 0;
            sci.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT | XR_SWAPCHAIN_USAGE_SAMPLED_BIT;
            sci.format = (int64_t)colorFmt;   // DXGI_FORMAT_* をそのまま
            sci.sampleCount = 1;
            sci.width = size.x;
            sci.height = size.y;
            sci.faceCount = 1;
            sci.arraySize = 1;                   // 片目1本ずつ作る設計
            sci.mipCount = 1;

            XrResult r = xrCreateSwapchain(xr_session, &sci, &xr_viewChainsColor[i]);
            if (!XR_SUCCEEDED(r)) {
                std::ostringstream oss; oss << "xrCreateSwapchain(Color) failed for view " << i << ": " << r << "\n";
                OutputDebugStringA(oss.str().c_str());
                return false;
            }

            // 画像列挙（D3D12 のリソースを受け取る）   //複数枚（2～3枚）リングバッファとして存在します
            uint32_t imgCount = 0;
            xrEnumerateSwapchainImages(xr_viewChainsColor[i], 0, &imgCount, nullptr);

            xr_colorImagesPerView[i].resize(imgCount);
            for (uint32_t k = 0; k < imgCount; ++k) {
                xr_colorImagesPerView[i][k] = {};
                xr_colorImagesPerView[i][k].type = XR_TYPE_SWAPCHAIN_IMAGE_D3D12_KHR;
                xr_colorImagesPerView[i][k].next = nullptr;
            }

            r = xrEnumerateSwapchainImages(
                xr_viewChainsColor[i], imgCount, &imgCount,
                reinterpret_cast<XrSwapchainImageBaseHeader*>(xr_colorImagesPerView[i].data()));
            if (!XR_SUCCEEDED(r)) {
                std::ostringstream oss; oss << "xrEnumerateSwapchainImages(Color) failed for view " << i << ": " << r << "\n";
                OutputDebugStringA(oss.str().c_str());
                return false;
            }

            // ここで xr_colorImagesPerView[i][k].texture に ID3D12Resource* が入っている
            // → アプリ側で RTV を作るときに使う
        }


        // Depth chain（推奨：合成などに使う場合に必要になるみたい）
        {
            XrSwapchainCreateInfo sci = {};
            sci.type = XR_TYPE_SWAPCHAIN_CREATE_INFO;
            sci.createFlags = 0;
            sci.usageFlags = XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            sci.format = (int64_t)depthFmt;   // 例: DXGI_FORMAT_D32_FLOAT
            sci.sampleCount = 1;
            sci.width = size.x;
            sci.height = size.y;
            sci.faceCount = 1;
            sci.arraySize = 1;
            sci.mipCount = 1;

            XrResult r = xrCreateSwapchain(xr_session, &sci, &xr_viewChainsDepth[i]);
            if (!XR_SUCCEEDED(r)) {
                std::ostringstream oss; oss << "xrCreateSwapchain(Depth) failed for view " << i << ": " << r << "\n";
                OutputDebugStringA(oss.str().c_str());
                return false;
            }

            uint32_t imgCount = 0;
            xrEnumerateSwapchainImages(xr_viewChainsDepth[i], 0, &imgCount, nullptr);

            xr_depthImagesPerView[i].resize(imgCount);
            for (uint32_t k = 0; k < imgCount; ++k) {
                xr_depthImagesPerView[i][k] = {};
                xr_depthImagesPerView[i][k].type = XR_TYPE_SWAPCHAIN_IMAGE_D3D12_KHR;
                xr_depthImagesPerView[i][k].next = nullptr;
            }

            r = xrEnumerateSwapchainImages(
                xr_viewChainsDepth[i], imgCount, &imgCount,
                reinterpret_cast<XrSwapchainImageBaseHeader*>(xr_depthImagesPerView[i].data()));

            if (!XR_SUCCEEDED(r)) {
                std::ostringstream oss; oss << "xrEnumerateSwapchainImages(Depth) failed for view " << i << ": " << r << "\n";
                OutputDebugStringA(oss.str().c_str());
                return false;
            }

            // ここで xr_depthImagesPerView[i][k].texture に ID3D12Resource* が入っている
            // → アプリ側で DSV を作るときに使う
        }
    }

    //

    {

        rtvHeaps_.resize(viewCount);
        dsvHeaps_.resize(viewCount);

        rtvHandles_.resize(viewCount);
        dsvHandles_.resize(viewCount);

        for (uint32_t i = 0; i < viewCount; ++i) {
            rtvHandles_[i].resize(xr_colorImagesPerView[i].size());
            dsvHandles_[i].resize(xr_depthImagesPerView[i].size());
        }

        OutputDebugStringA(("viewCount " + std::to_string(viewCount) + "\n").c_str());

        for (uint32_t i = 0; i < viewCount; ++i) {

            OutputDebugStringA(("i " + std::to_string(i) + "\n").c_str());


            // --- Color chain 作成＆列挙（既存） ---
            {
                uint32_t imgCount = xr_colorImagesPerView[i].size();
                if (imgCount == 0) {
                    OutputDebugStringA("[ERROR] RTV imgCount==0\n"); return false;
                }
                OutputDebugStringA(("color imgCount " + std::to_string(imgCount) + "\n").c_str());

                rtvHandles_[i].resize(imgCount);

                D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
                rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
                rtvHeapDesc.NumDescriptors = imgCount;               // その view の画像枚数ぶん
                rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;



                HRESULT hr = d3d12Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeaps_[i]));
                if (FAILED(hr) || !rtvHeaps_[i]) { OutputDebugStringA("[ERROR] Create RTV heap FAILED\n"); return false; }

                const UINT inc = d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
                auto cpuStart = rtvHeaps_[i]->GetCPUDescriptorHandleForHeapStart();
                if (cpuStart.ptr == 0) { OutputDebugStringA("[ERROR] RTV heap cpuStart.ptr==0\n"); return false; }


                D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;          // ←用途に応じて _UNORM_SRGB に変更可
                rtvDesc.Texture2D.MipSlice = 0;
                rtvDesc.Texture2D.PlaneSlice = 0;

                for (uint32_t k = 0; k < imgCount; ++k) {
                    if (!xr_colorImagesPerView[i][k].texture) { OutputDebugStringA("[ERROR] RTV texture null\n"); return false; }


                    D3D12_CPU_DESCRIPTOR_HANDLE h{ cpuStart.ptr + SIZE_T(k) * inc };
                    rtvHandles_[i][k] = h;

                    // RTV を作成（descは nullptr でOK、フォーマットは swapchain 作成時と整合）
                    d3d12Device->CreateRenderTargetView(
                        xr_colorImagesPerView[i][k].texture, &rtvDesc, h);

                    if (rtvHandles_[i][k].ptr == 0) { OutputDebugStringA("[ERROR] RTV handle ptr==0\n"); return false; }
                }
            }

            // --- Depth chain 作成＆列挙（既存） ---
            {

                uint32_t imgCount = xr_depthImagesPerView[i].size();
                if (imgCount == 0) {
                    OutputDebugStringA("[ERROR] DSV imgCount==0\n"); return false;
                }
                OutputDebugStringA(("Depth imgCount " + std::to_string(imgCount) + "\n").c_str());

                dsvHandles_[i].resize(imgCount);

                D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
                dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
                dsvHeapDesc.NumDescriptors = imgCount;
                dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

                HRESULT hr = d3d12Device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeaps_[i]));
                if (FAILED(hr) || !dsvHeaps_[i]) { OutputDebugStringA("[ERROR] Create DSV heap FAILED\n"); return false; }

                const UINT inc = d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
                if (inc == 0) { OutputDebugStringA("[ERROR] DSV increment size == 0\n"); return false; }

                auto cpuStart = dsvHeaps_[i]->GetCPUDescriptorHandleForHeapStart();
                if (cpuStart.ptr == 0) { OutputDebugStringA("[ERROR] DSV heap cpuStart.ptr==0\n"); return false; }

                // DSV のフォーマットは swapchain 作成時の depthFmt と一致させる
                D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
                dsvDesc.Format = depthFmt;
                dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
                dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

                for (uint32_t k = 0; k < imgCount; ++k) {
                    if (!xr_depthImagesPerView[i][k].texture) { OutputDebugStringA("[ERROR] DSV texture null\n"); return false; }


                    D3D12_CPU_DESCRIPTOR_HANDLE h{ cpuStart.ptr + SIZE_T(k) * inc };
                    dsvHandles_[i][k] = h;

                    d3d12Device->CreateDepthStencilView(
                        xr_depthImagesPerView[i][k].texture, &dsvDesc, h);

                    if (dsvHandles_[i][k].ptr == 0) { OutputDebugStringA("[ERROR] DSV handle ptr==0\n"); return false; }
                }
            }
        }


    }


    // ログ
    {
        char buf[256];
        sprintf_s(buf, "[XR] Created %u view swapchains (%d x %d)\n",
            viewCount, size.x, size.y);
        OutputDebugStringA(buf);
    }

    return true;
}




bool OpenXRManager::InitControllers() {

    //コントローラー初期化

    if (xr_instance == XR_NULL_HANDLE || xr_session == XR_NULL_HANDLE || xr_appSpace == XR_NULL_HANDLE) {
        OutputDebugStringA("[XR] InitControllers: instance/session/appSpace not ready.\n");
        controllersReady = false;
        return false;
    }
    controllersReady = controller.Initialize(xr_instance, xr_session, xr_appSpace);

    if (controllersReady) {
        OutputDebugStringA("[XR] Controllers initialized.\n");
    }
    else {
        OutputDebugStringA("[XR] Controllers init FAILED.\n");
    }

    return controllersReady;

}





void OpenXRManager::UpdateSessionState() {

    //VR機器のセッションステート更新

    XrEventDataBuffer ev = {};                  //イベント受け取りバッファ
    ev.type = XR_TYPE_EVENT_DATA_BUFFER;

    XrResult r = xrPollEvent(xr_instance, &ev); //イベントを 1件 だけ取り出す   //＊＊＊コントローラーの情報取得もこれの実行が必要だった。OpenXR は 「イベント駆動型」
    if (r != XR_SUCCESS) return;

    switch (ev.type) {
    case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
        const auto* s = reinterpret_cast<const XrEventDataSessionStateChanged*>(&ev);

        xr_sessionState = s->state;     //セッションステート変更
        
        if (xr_sessionState == XR_SESSION_STATE_FOCUSED ||
            xr_sessionState == XR_SESSION_STATE_VISIBLE ||
            xr_sessionState == XR_SESSION_STATE_SYNCHRONIZED) {

            xr_sessionRunning = true;   //セッションが走っている
        }
        else if (xr_sessionState == XR_SESSION_STATE_IDLE) {

            xr_sessionRunning = false;  //セッションが走っていない
        }
        break;
    }
    default:
        break;
    }




}





//--------------------------





bool OpenXRManager::Start_XR_Session() {


    //セッション開始（アプリケーション終了時に、セッション終了）

    XrSessionBeginInfo bi = {};
    bi.type = XR_TYPE_SESSION_BEGIN_INFO;
    bi.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;


    XrResult r = xrBeginSession(xr_session, &bi);
    if (!XR_SUCCEEDED(r)) {
        OutputDebugStringA("xrBeginSession failed.\n");
        return false;
    }
    OutputDebugStringA("[XR] Session begun.\n");


    return true;
}





bool OpenXRManager::BeginFrame(XrTime& predictedDisplayTime) {


    // 次に描画できるフレームのタイミングを待つ    （＊待ち時間に注意。計画的に適切なタイミングで呼ぶ）

    XrFrameWaitInfo wi = {};
    wi.type = XR_TYPE_FRAME_WAIT_INFO;

    XrFrameState fs = {};
    fs.type = XR_TYPE_FRAME_STATE;

    XrResult r = xrWaitFrame(xr_session, &wi, &fs);
    if (!XR_SUCCEEDED(r)) return false;

    //表示予定時間を渡す
    predictedDisplayTime = fs.predictedDisplayTime;


    // フレームの開始
    XrFrameBeginInfo bi = {};
    bi.type = XR_TYPE_FRAME_BEGIN_INFO;

    r = xrBeginFrame(xr_session, &bi);
    if (!XR_SUCCEEDED(r)) return false;

    // コントローラ同期
    if (controllersReady) {
        controller.Sync(xr_session, predictedDisplayTime);
    }

    //PollSimpleController();



    return true;
}




XMMATRIX OpenXRManager::CreateCameraViewMatrix(const XrPosef& pose) {

    //カメラのビュー行列  //＊OpenXRと、DirectXの座標系の違いを考慮する

    // OpenXR: RH, +Y up, -Z forward
    const XMVECTOR q = XMVectorSet(pose.orientation.x, pose.orientation.y, pose.orientation.z, pose.orientation.w);

    // 位置：RH→LH 変換（Z反転）
    const XMVECTOR posLH = XMVectorSet(pose.position.x, pose.position.y, -pose.position.z, 1.0f);

    // 向き：OpenXRの -Z / +Y を回してから RH→LH へZ反転
    const XMVECTOR fwdRH = XMVector3Rotate(XMVectorSet(0, 0, -1, 0), q); // -Z を回す
    const XMVECTOR upRH = XMVector3Rotate(XMVectorSet(0, 1, 0, 0), q); // +Y を回す

    const XMVECTOR fwdLH = XMVectorSet(XMVectorGetX(fwdRH), XMVectorGetY(fwdRH), -XMVectorGetZ(fwdRH), 0.0f);
    const XMVECTOR upLH = XMVectorSet(XMVectorGetX(upRH), XMVectorGetY(upRH), -XMVectorGetZ(upRH), 0.0f);

    // LHのビュー行列を生成
    return XMMatrixLookToLH(posLH, fwdLH, upLH);

    /*
    // OpenXRの姿勢
    XMVECTOR q = XMVectorSet(pose.orientation.x, pose.orientation.y, pose.orientation.z, pose.orientation.w);
    XMVECTOR pos = XMVectorSet(pose.position.x, pose.position.y, pose.position.z, 0.0f);

    // カメラのローカル前方(+Z)と上方(+Y)を回転で世界に持ち上げる
    XMVECTOR fwd = XMVector3Rotate(XMVectorSet(0, 0, 1, 0), q);
    XMVECTOR up = XMVector3Rotate(XMVectorSet(0, 1, 0, 0), q);

    return XMMatrixLookToLH(pos, fwd, up);
    */

}

/*
DirectX::XMMATRIX OpenXRManager::XMMatrixFromXrPoseLH(const XrPosef& pose) {

    //回転と移動

    XMVECTOR q = XMVectorSet(pose.orientation.x, pose.orientation.y, pose.orientation.z, pose.orientation.w);
    XMVECTOR t = XMVectorSet(pose.position.x, pose.position.y, pose.position.z, 1.0f);
    XMMATRIX R = XMMatrixRotationQuaternion(q);
    XMMATRIX T = XMMatrixTranslationFromVector(t);

    return R * T;
}
*/

DirectX::XMMATRIX OpenXRManager::CreateProjectionMatrix(const XrFovf& fov, float nearZ, float farZ) {

    // OpenXRのFOVから、カメラのプロジェクション行列を生成

    float l = nearZ * tanf(fov.angleLeft);
    float r = nearZ * tanf(fov.angleRight);
    float b = nearZ * tanf(fov.angleDown);
    float t = nearZ * tanf(fov.angleUp);

    return XMMatrixPerspectiveOffCenterLH(l, r, b, t, nearZ, farZ);
}


// 片目ごとのカメラ行列を取得
bool OpenXRManager::GetEyeMatrix(XrTime predictedDisplayTime, float nearZ, float farZ, std::vector<EyeMatrix>& outEyes) {

    // viewState
    XrViewState viewState{ XR_TYPE_VIEW_STATE };

    // view配列を確保
    std::vector<XrView> views(xr_viewCount, { XR_TYPE_VIEW });

    // LocateInfo
    XrViewLocateInfo li{ XR_TYPE_VIEW_LOCATE_INFO };
    li.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
    li.displayTime = predictedDisplayTime;
    li.space = xr_appSpace;

    uint32_t outCount = xr_viewCount;
    XrResult r = xrLocateViews(xr_session, &li, &viewState, xr_viewCount, &outCount, views.data());
    if (!XR_SUCCEEDED(r) || outCount != xr_viewCount) {
        OutputDebugStringA("xrLocateViews failed.\n");
        return false;
    }

    // 出力準備
    outEyes.clear();
    outEyes.resize(xr_viewCount);

    for (uint32_t i = 0; i < xr_viewCount; ++i) {

        const XrView& v = views[i];

        XMMATRIX view = CreateCameraViewMatrix(v.pose);
        XMMATRIX proj = CreateProjectionMatrix(v.fov, nearZ, farZ);

        outEyes[i] = EyeMatrix{
            view,
            proj,
            v
        };
    }

    return true;
}


/*
bool OpenXRManager::GetStereoViews(XrTime predictedDisplayTime, EyeViews& out) {


    //VRで両目のカメラ位置・向きを求める

    out.views.resize(xr_viewCount);
    for (uint32_t i = 0; i < xr_viewCount; ++i) {
        out.views[i] = {};
        out.views[i].type = XR_TYPE_VIEW;
    }

    out.viewState = {};
    out.viewState.type = XR_TYPE_VIEW_STATE;

    XrViewLocateInfo li = {};
    li.type = XR_TYPE_VIEW_LOCATE_INFO;
    li.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
    li.displayTime = predictedDisplayTime;
    li.space = xr_appSpace;  // 参照スペース

    uint32_t outCount = xr_viewCount;
    
    XrResult r = xrLocateViews(xr_session, &li, &out.viewState, outCount, &outCount, out.views.data());

    if (!XR_SUCCEEDED(r) || outCount != xr_viewCount) {
        OutputDebugStringA("xrLocateViews failed.\n");
        return false;
    }

    return true;
}
*/



/*
bool OpenXRManager::BeginEyeDirect(
    ID3D12GraphicsCommandList* cmd,
    uint32_t eyeIndex,
    EyeDirectTarget& out) {


    if (eyeIndex >= xr_viewCount) return false;

    // Acquire/Wait (Color)
    XrSwapchainImageAcquireInfo acqInfo{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
    uint32_t colorIdx = 0;
    XrResult r = xrAcquireSwapchainImage(xr_viewChainsColor[eyeIndex], &acqInfo, &colorIdx);
    if (!XR_SUCCEEDED(r)) return false;

    XrSwapchainImageWaitInfo waitInfo{ XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
    waitInfo.timeout = XR_INFINITE_DURATION;
    r = xrWaitSwapchainImage(xr_viewChainsColor[eyeIndex], &waitInfo);
    if (!XR_SUCCEEDED(r)) return false;

    // Acquire/Wait (Depth)
    uint32_t depthIdx = 0;
    r = xrAcquireSwapchainImage(xr_viewChainsDepth[eyeIndex], &acqInfo, &depthIdx);
    if (!XR_SUCCEEDED(r)) return false;

    r = xrWaitSwapchainImage(xr_viewChainsDepth[eyeIndex], &waitInfo);
    if (!XR_SUCCEEDED(r)) return false;

    // 列挙済みのD3D12テクスチャ
    ID3D12Resource* colorTex = xr_colorImagesPerView[eyeIndex][colorIdx].texture;  // 色
    ID3D12Resource* depthTex = xr_depthImagesPerView[eyeIndex][depthIdx].texture;  // 深度

    // ★ d3dx12 ヘルパーでバリア（COMMON -> RT/DS）
    {
        auto b0 = CD3DX12_RESOURCE_BARRIER::Transition(
            colorTex, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
        auto b1 = CD3DX12_RESOURCE_BARRIER::Transition(
            depthTex, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
        CD3DX12_RESOURCE_BARRIER barriers[] = { b0, b1 };
        cmd->ResourceBarrier(_countof(barriers), barriers);
    }

    // このフレームで使う index を保存（EndEyeDirect で使う）
    eyeActiveColorIndex[eyeIndex] = colorIdx;
    eyeActiveDepthIndex[eyeIndex] = depthIdx;

    
    // RTV/DSV を返す
    out.rtv = rtvHandles_[eyeIndex][colorIdx];
    out.dsv = dsvHandles_[eyeIndex][depthIdx];
    out.size = recommendedScaledResolution;  // ビューポート/シザー用サイズ :contentReference[oaicite:2]{index=2}
    
    
    return true;
}
*/

/*
bool OpenXRManager::BeginEyeDirect(
    ID3D12GraphicsCommandList* cmd,
    uint32_t eyeIndex,
    EyeDirectTarget& out) {
    // out を必ず初期化
    out.rtv = D3D12_CPU_DESCRIPTOR_HANDLE{ 0 };
    out.dsv = D3D12_CPU_DESCRIPTOR_HANDLE{ 0 };
    out.size = recommendedScaledResolution;

    if (!cmd) {
        OutputDebugStringA("BeginEyeDirect: cmd is null\n");
        return false;
    }
    if (eyeIndex >= xr_viewCount) {
        OutputDebugStringA("BeginEyeDirect: eyeIndex out of range\n");
        return false;
    }

    // Acquire color
    XrSwapchainImageAcquireInfo acqInfo{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
    uint32_t colorIdx = 0;
    XrResult r = xrAcquireSwapchainImage(xr_viewChainsColor[eyeIndex], &acqInfo, &colorIdx);
    if (!XR_SUCCEEDED(r)) {
        OutputDebugStringA("BeginEyeDirect: xrAcquireSwapchainImage color failed\n");
        return false;
    }
    XrSwapchainImageWaitInfo waitInfo{ XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
    waitInfo.timeout = XR_INFINITE_DURATION;
    r = xrWaitSwapchainImage(xr_viewChainsColor[eyeIndex], &waitInfo);
    if (!XR_SUCCEEDED(r)) {
        OutputDebugStringA("BeginEyeDirect: xrWaitSwapchainImage color failed\n");
        return false;
    }

    // Acquire depth
    uint32_t depthIdx = 0;
    r = xrAcquireSwapchainImage(xr_viewChainsDepth[eyeIndex], &acqInfo, &depthIdx);
    if (!XR_SUCCEEDED(r)) {
        OutputDebugStringA("BeginEyeDirect: xrAcquireSwapchainImage depth failed\n");
        return false;
    }
    r = xrWaitSwapchainImage(xr_viewChainsDepth[eyeIndex], &waitInfo);
    if (!XR_SUCCEEDED(r)) {
        OutputDebugStringA("BeginEyeDirect: xrWaitSwapchainImage depth failed\n");
        return false;
    }

    // テクスチャチェック
    ID3D12Resource* colorTex = xr_colorImagesPerView[eyeIndex][colorIdx].texture;
    ID3D12Resource* depthTex = xr_depthImagesPerView[eyeIndex][depthIdx].texture;
    if (!colorTex) {
        OutputDebugStringA("BeginEyeDirect: colorTex is null\n");
        return false;
    }
    if (!depthTex) {
        OutputDebugStringA("BeginEyeDirect: depthTex is null\n");
        return false;
    }

    // ハンドル存在確認
    if (eyeIndex >= rtvHandles_.size() || eyeIndex >= dsvHandles_.size()) {
        OutputDebugStringA("BeginEyeDirect: handle array not sized\n");
        return false;
    }
    if (colorIdx >= rtvHandles_[eyeIndex].size()) {
        OutputDebugStringA("BeginEyeDirect: rtvHandles index out of range\n");
        return false;
    }
    if (depthIdx >= dsvHandles_[eyeIndex].size()) {
        OutputDebugStringA("BeginEyeDirect: dsvHandles index out of range\n");
        return false;
    }

    if (rtvHandles_[eyeIndex][colorIdx].ptr == 0) {
        OutputDebugStringA("BeginEyeDirect: RTV handle is null\n");
        return false;
    }
    if (dsvHandles_[eyeIndex][depthIdx].ptr == 0) {
        OutputDebugStringA("BeginEyeDirect: DSV handle is null\n");
    }

    // バリア設定
    {
        CD3DX12_RESOURCE_BARRIER barriers[] = {
            CD3DX12_RESOURCE_BARRIER::Transition(colorTex, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET),
            CD3DX12_RESOURCE_BARRIER::Transition(depthTex, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE),
        };
        cmd->ResourceBarrier(_countof(barriers), barriers);
    }

    // index 保存
    eyeActiveColorIndex[eyeIndex] = colorIdx;
    eyeActiveDepthIndex[eyeIndex] = depthIdx;

    // 出力設定
    out.rtv = rtvHandles_[eyeIndex][colorIdx];
    out.dsv = dsvHandles_[eyeIndex][depthIdx];
    out.size = recommendedScaledResolution;

    if (out.rtv.ptr == 0) {
        OutputDebugStringA("BeginEyeDirect: out.rtv is null\n");
        return false;
    }
    if (out.dsv.ptr == 0) {
        OutputDebugStringA("BeginEyeDirect: out.dsv is null\n");
    }

    return true;
}
*/



bool OpenXRManager::BeginEyeDirect(
    ID3D12GraphicsCommandList* cmd,
    uint32_t eyeIndex,
    EyeDirectTarget& out) {


    out.rtv = D3D12_CPU_DESCRIPTOR_HANDLE{ 0 };
    out.dsv = D3D12_CPU_DESCRIPTOR_HANDLE{ 0 };
    out.size = recommendedScaledResolution;

    if (!cmd) { OutputDebugStringA("BeginEyeDirect: cmd is null\n"); return false; }
    if (eyeIndex >= xr_viewCount) { OutputDebugStringA("BeginEyeDirect: eyeIndex OOB\n"); return false; }

    // 取得フラグとインデックスを管理して、失敗時に必ず release する
    bool acquiredColor = false;
    bool acquiredDepth = false;
    uint32_t colorIdx = 0;
    uint32_t depthIdx = 0;

    auto cleanup_on_fail = [&]() {

        XrSwapchainImageReleaseInfo ri{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
        if (acquiredColor) xrReleaseSwapchainImage(xr_viewChainsColor[eyeIndex], &ri);
        if (acquiredDepth) xrReleaseSwapchainImage(xr_viewChainsDepth[eyeIndex], &ri);
        };

    // 毎フレームの順序チェック: xrWaitFrame -> xrBeginFrame 済みか
    // 必要ならフラグを OpenXRManager に持ってチェックする
    // if (!inFrame_) { OutputDebugStringA("BeginEyeDirect: BeginFrame not called\n"); return false; }

    // Acquire/Wait Color
    {
        XrSwapchainImageAcquireInfo acq{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
        XrResult r = xrAcquireSwapchainImage(xr_viewChainsColor[eyeIndex], &acq, &colorIdx);
        if (!XR_SUCCEEDED(r)) {
            char buf[128]; sprintf_s(buf, "BeginEyeDirect: Acquire color failed r=%d\n", r);
            OutputDebugStringA(buf);
            cleanup_on_fail();
            return false;
        }
        acquiredColor = true;

        XrSwapchainImageWaitInfo wi{ XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
        wi.timeout = XR_INFINITE_DURATION;
        r = xrWaitSwapchainImage(xr_viewChainsColor[eyeIndex], &wi);
        if (!XR_SUCCEEDED(r)) {
            char buf[128]; sprintf_s(buf, "BeginEyeDirect: Wait color failed r=%d\n", r);
            OutputDebugStringA(buf);
            cleanup_on_fail();
            return false;
        }
    }

    // Acquire/Wait Depth
    {
        XrSwapchainImageAcquireInfo acq{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
        XrResult r = xrAcquireSwapchainImage(xr_viewChainsDepth[eyeIndex], &acq, &depthIdx);
        if (!XR_SUCCEEDED(r)) {
            char buf[128]; sprintf_s(buf, "BeginEyeDirect: Acquire depth failed r=%d\n", r);
            OutputDebugStringA(buf);
            cleanup_on_fail();
            return false;
        }
        acquiredDepth = true;

        XrSwapchainImageWaitInfo wi{ XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
        wi.timeout = XR_INFINITE_DURATION;
        r = xrWaitSwapchainImage(xr_viewChainsDepth[eyeIndex], &wi);
        if (!XR_SUCCEEDED(r)) {
            char buf[128]; sprintf_s(buf, "BeginEyeDirect: Wait depth failed r=%d\n", r);
            OutputDebugStringA(buf);
            cleanup_on_fail();
            return false;
        }
    }

    // リソースとハンドルの検証
    ID3D12Resource* colorTex = xr_colorImagesPerView[eyeIndex][colorIdx].texture;
    ID3D12Resource* depthTex = xr_depthImagesPerView[eyeIndex][depthIdx].texture;
    if (!colorTex || !depthTex) {
        OutputDebugStringA("BeginEyeDirect: colorTex or depthTex is null\n");
        cleanup_on_fail();
        return false;
    }
    if (eyeIndex >= rtvHandles_.size() || colorIdx >= rtvHandles_[eyeIndex].size()
        || eyeIndex >= dsvHandles_.size() || depthIdx >= dsvHandles_[eyeIndex].size()) {
        OutputDebugStringA("BeginEyeDirect: handle index OOB\n");
        cleanup_on_fail();
        return false;
    }
    if (rtvHandles_[eyeIndex][colorIdx].ptr == 0) {
        OutputDebugStringA("BeginEyeDirect: RTV handle is null\n");
        cleanup_on_fail();
        return false;
    }
    // DSV は無くても描画自体は可能にしたい場合、ここは警告のみにする
    if (dsvHandles_[eyeIndex][depthIdx].ptr == 0) {
        OutputDebugStringA("BeginEyeDirect: DSV handle is null\n");
    }

    // バリア
    {
        CD3DX12_RESOURCE_BARRIER bs[] = {
            CD3DX12_RESOURCE_BARRIER::Transition(colorTex, D3D12_RESOURCE_STATE_COMMON,      D3D12_RESOURCE_STATE_RENDER_TARGET),
            CD3DX12_RESOURCE_BARRIER::Transition(depthTex, D3D12_RESOURCE_STATE_COMMON,      D3D12_RESOURCE_STATE_DEPTH_WRITE),
        };
        cmd->ResourceBarrier(_countof(bs), bs);
    }

    // 保存と出力
    eyeActiveColorIndex[eyeIndex] = colorIdx;
    eyeActiveDepthIndex[eyeIndex] = depthIdx;

    out.rtv = rtvHandles_[eyeIndex][colorIdx];
    out.dsv = dsvHandles_[eyeIndex][depthIdx];
    out.size = recommendedScaledResolution;

    return true;
}


bool OpenXRManager::EndEyeDirect(
    ID3D12GraphicsCommandList* cmd,
    uint32_t eyeIndex) {


    if (eyeIndex >= xr_viewCount) return false;

    const uint32_t colorIdx = eyeActiveColorIndex[eyeIndex];
    const uint32_t depthIdx = eyeActiveDepthIndex[eyeIndex];

    ID3D12Resource* colorTex = xr_colorImagesPerView[eyeIndex][colorIdx].texture;
    ID3D12Resource* depthTex = xr_depthImagesPerView[eyeIndex][depthIdx].texture;

    // ★ d3dx12 ヘルパーでバリア（RT/DS -> COMMON）
    {
        auto b0 = CD3DX12_RESOURCE_BARRIER::Transition(
            colorTex, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
        auto b1 = CD3DX12_RESOURCE_BARRIER::Transition(
            depthTex, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COMMON);
        CD3DX12_RESOURCE_BARRIER barriers[] = { b0, b1 };
        cmd->ResourceBarrier(_countof(barriers), barriers);
    }

    // OpenXR 側へリリース
    XrSwapchainImageReleaseInfo ri{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
    xrReleaseSwapchainImage(xr_viewChainsColor[eyeIndex], &ri);
    xrReleaseSwapchainImage(xr_viewChainsDepth[eyeIndex], &ri);

    return true;
}









bool OpenXRManager::CopyOneViewToSwapchain(
    ID3D12GraphicsCommandList* cmd,
    ID3D12Resource* myColorRT, ID3D12Resource* myDepth,
    XrSwapchain colorChain, std::vector<XrSwapchainImageD3D12KHR>& colorImgs,
    XrSwapchain depthChain, std::vector<XrSwapchainImageD3D12KHR>& depthImgs) {


    // ---- COLOR ----

    
    //「書き込み先（提出先）の画像インデックスをもらう
    uint32_t colorIndex = 0;
    XrSwapchainImageAcquireInfo acq{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
    if (!XR_SUCCEEDED(xrAcquireSwapchainImage(colorChain, &acq, &colorIndex))) return false;

    //その画像が使える状態になるまで待つ
    XrSwapchainImageWaitInfo wi{ XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
    wi.timeout = XR_INFINITE_DURATION;
    if (!XR_SUCCEEDED(xrWaitSwapchainImage(colorChain, &wi))) return false;

    ID3D12Resource* dstColor = colorImgs[colorIndex].texture;

    // 事前遷移（まとめて）
    {
        CD3DX12_RESOURCE_BARRIER barriers[] = {
            CD3DX12_RESOURCE_BARRIER::Transition(
                myColorRT,
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                D3D12_RESOURCE_STATE_COPY_SOURCE),
            CD3DX12_RESOURCE_BARRIER::Transition(
                dstColor,
                D3D12_RESOURCE_STATE_COMMON,   // 初期COMMON想定
                D3D12_RESOURCE_STATE_COPY_DEST),
        };
        cmd->ResourceBarrier(_countof(barriers), barriers);
    }

    // コピー
    cmd->CopyResource(dstColor, myColorRT);

    // 事後遷移（まとめて）
    {
        CD3DX12_RESOURCE_BARRIER barriers[] = {
            CD3DX12_RESOURCE_BARRIER::Transition(
                dstColor,
                D3D12_RESOURCE_STATE_COPY_DEST,
                D3D12_RESOURCE_STATE_COMMON),  // ランタイム提出前に戻す
            CD3DX12_RESOURCE_BARRIER::Transition(
                myColorRT,
                D3D12_RESOURCE_STATE_COPY_SOURCE,
                D3D12_RESOURCE_STATE_RENDER_TARGET), // 次フレームに備える
        };
        cmd->ResourceBarrier(_countof(barriers), barriers);
    }

    XrSwapchainImageReleaseInfo ri{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
    if (!XR_SUCCEEDED(xrReleaseSwapchainImage(colorChain, &ri))) return false;




    // ---- DEPTH ----

    //「書き込み先（提出先）の画像インデックスをもらう
    uint32_t depthIndex = 0;
    acq = { XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
    if (!XR_SUCCEEDED(xrAcquireSwapchainImage(depthChain, &acq, &depthIndex))) return false;

    //その画像が使える状態になるまで待つ
    wi = { XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
    wi.timeout = XR_INFINITE_DURATION;
    if (!XR_SUCCEEDED(xrWaitSwapchainImage(depthChain, &wi))) return false;

    ID3D12Resource* dstDepth = depthImgs[depthIndex].texture;

    {
        // 事前遷移（まとめて）
        CD3DX12_RESOURCE_BARRIER barriersBegin[] = {
            CD3DX12_RESOURCE_BARRIER::Transition(
                myDepth,
                D3D12_RESOURCE_STATE_DEPTH_WRITE,
                D3D12_RESOURCE_STATE_COPY_SOURCE),
            CD3DX12_RESOURCE_BARRIER::Transition(
                dstDepth,
                D3D12_RESOURCE_STATE_COMMON,
                D3D12_RESOURCE_STATE_COPY_DEST),
        };
        cmd->ResourceBarrier(_countof(barriersBegin), barriersBegin);

    }

    // コピー（サイズ/フォーマット一致前提）
    cmd->CopyResource(dstDepth, myDepth);

    {
        // 事後遷移（まとめて）
        CD3DX12_RESOURCE_BARRIER barriersEnd[] = {
            CD3DX12_RESOURCE_BARRIER::Transition(
                dstDepth,
                D3D12_RESOURCE_STATE_COPY_DEST,
                D3D12_RESOURCE_STATE_COMMON),
            CD3DX12_RESOURCE_BARRIER::Transition(
                myDepth,
                D3D12_RESOURCE_STATE_COPY_SOURCE,
                D3D12_RESOURCE_STATE_DEPTH_WRITE),
        };
        cmd->ResourceBarrier(_countof(barriersEnd), barriersEnd);
    }


    if (!XR_SUCCEEDED(xrReleaseSwapchainImage(depthChain, &ri))) return false;

    return true;
}



bool OpenXRManager::EndFrameWithProjection(
    const std::vector<EyeMatrix>& eyesData,
    float nearZ, float farZ,
    //uint32_t viewCount,
    //const std::vector<XrSwapchain>& colorChains,
    //const std::vector<XrSwapchain>& depthChains,
    XMINT2 size, XrTime displayTime) {

    //std::vector<XrSwapchain> xr_viewChainsColor;
    //std::vector<XrSwapchain> xr_viewChainsDepth;


    //VRに最終的に描画した情報を渡す（VRはいい感じに見えるように補正とかするので、描画に使った情報が必要らしい

    // viewごとのレイヤー要素
    std::vector<XrCompositionLayerProjectionView> projViews(xr_viewCount);
    std::vector<XrCompositionLayerDepthInfoKHR>   depthInfos(xr_viewCount);

    for (uint32_t i = 0; i < xr_viewCount; ++i) {
        // Depth info（任意だが推奨）
        depthInfos[i] = {};
        depthInfos[i].type = XR_TYPE_COMPOSITION_LAYER_DEPTH_INFO_KHR;
        depthInfos[i].subImage.swapchain = xr_viewChainsDepth[i];
        depthInfos[i].subImage.imageArrayIndex = 0;
        depthInfos[i].subImage.imageRect.offset.x = 0;
        depthInfos[i].subImage.imageRect.offset.y = 0;
        depthInfos[i].subImage.imageRect.extent.width = size.x;
        depthInfos[i].subImage.imageRect.extent.height = size.y;
        depthInfos[i].minDepth = 0.0f;
        depthInfos[i].maxDepth = 1.0f;
        depthInfos[i].nearZ = nearZ;
        depthInfos[i].farZ = farZ;

        // Projection view
        projViews[i] = {};
        projViews[i].type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
        projViews[i].next = &depthInfos[i]; // 深度を添付
        projViews[i].pose = eyesData[i].xrView.pose;      // xrLocateViews の結果
        projViews[i].fov = eyesData[i].xrView.fov;       // 同上

        projViews[i].subImage.swapchain = xr_viewChainsColor[i];
        projViews[i].subImage.imageArrayIndex = 0;
        projViews[i].subImage.imageRect.offset.x = 0;
        projViews[i].subImage.imageRect.offset.y = 0;
        projViews[i].subImage.imageRect.extent.width = size.x;
        projViews[i].subImage.imageRect.extent.height = size.y;
    }

    XrCompositionLayerProjection layer = {};
    layer.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION;
    layer.space = xr_appSpace;
    layer.viewCount = xr_viewCount;
    layer.views = projViews.data();

    const XrCompositionLayerBaseHeader* layers[] = {
        reinterpret_cast<const XrCompositionLayerBaseHeader*>(&layer)
    };

    XrFrameEndInfo fe = {};
    fe.type = XR_TYPE_FRAME_END_INFO;
    fe.displayTime = displayTime; // WaitFrame で使った predictedDisplayTime を入れても良い
    fe.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE; // 例
    fe.layerCount = 1;
    fe.layers = layers;

    XrResult r = xrEndFrame(xr_session, &fe);

    return XR_SUCCEEDED(r);
}








//--------------------------



bool OpenXRManager::End_XR_Session() {

    //終了をリクエスト

    if (xr_session == XR_NULL_HANDLE) return true;  //セッション無し

    switch (xr_sessionState) {
    case XR_SESSION_STATE_FOCUSED:
    case XR_SESSION_STATE_VISIBLE:
    case XR_SESSION_STATE_SYNCHRONIZED: {

        XrResult r = xrEndSession(xr_session);  //セッション終了

        if (!XR_SUCCEEDED(r)) {
            std::ostringstream oss;
            oss << "xrEndSession failed: " << r << "\n";
            OutputDebugStringA(oss.str().c_str());
            return false;
        }
        return true;
    }
    default:
        return true;
    }
}

bool OpenXRManager::EndSessionGracefully(uint32_t msTimeout) {

    if (xr_session == XR_NULL_HANDLE) return true;

    (void)End_XR_Session(); //終了をリクエスト

    const auto t0 = std::chrono::steady_clock::now();
    while (true) {

        UpdateSessionState();   //セッションステート更新

        if (xr_sessionState == XR_SESSION_STATE_IDLE ||
            xr_sessionState == XR_SESSION_STATE_UNKNOWN ||
            xr_sessionState == XR_SESSION_STATE_EXITING ||
            xr_sessionState == XR_SESSION_STATE_LOSS_PENDING) {

            break;  //終了できる状態になった
        }
        const auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - t0).count() >= msTimeout) {
            OutputDebugStringA("EndSessionGracefully timeout.\n");

            break;  //タイムアウトで終了
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1)); //スリープ
    }

    XrResult r = xrDestroySession(xr_session);  //セッションの破棄

    if (!XR_SUCCEEDED(r)) {
        std::ostringstream oss;
        oss << "xrDestroySession failed: " << r << "\n";
        OutputDebugStringA(oss.str().c_str());
        return false;
    }
    xr_session = XR_NULL_HANDLE;
    xr_sessionState = XR_SESSION_STATE_UNKNOWN;
    xr_sessionRunning = false;

    return true;
}


void OpenXRManager::DestroySwapchains() {

    //VRのスワップチェーンの破棄

    // Color
    for (auto sc : xr_viewChainsColor) {
        if (sc != XR_NULL_HANDLE) {
            xrDestroySwapchain(sc);
        }
    }
    // Depth
    for (auto sc : xr_viewChainsDepth) {
        if (sc != XR_NULL_HANDLE) {
            xrDestroySwapchain(sc);
        }
    }

    xr_viewChainsColor.clear();
    xr_viewChainsDepth.clear();
    xr_colorImagesPerView.clear();
    xr_depthImagesPerView.clear();

    OutputDebugStringA("[XR] Destroyed swapchains.\n");
}

void OpenXRManager::OnDestroy() {


    //d3d12Device_ = nullptr;

    //セッションの破棄
    if (xr_session != XR_NULL_HANDLE) {
        EndSessionGracefully(2000); // 2秒待ち
    }

    //コントローラー破棄
    controller.OnDestroy();
    controllersReady = false;

    //スペースの破棄
    if (xr_appSpace != XR_NULL_HANDLE) {
        xrDestroySpace(xr_appSpace);
        xr_appSpace = XR_NULL_HANDLE;
    }

    //スワップチェーン破棄
    DestroySwapchains();

    //インスタンスの破棄
    if (xr_instance != XR_NULL_HANDLE) {
        xrDestroyInstance(xr_instance);
        xr_instance = XR_NULL_HANDLE;
    }

}


















//-----------------








/*




bool OpenXRManager::LogCurrentInteractionProfiles() {
    if (xr_instance == XR_NULL_HANDLE || xr_session == XR_NULL_HANDLE) return false;

    auto dump = [&](const char* userPathStr) {
        XrPath userPath = XR_NULL_PATH;
        if (XR_FAILED(xrStringToPath(xr_instance, userPathStr, &userPath))) return;

        XrInteractionProfileState st{ XR_TYPE_INTERACTION_PROFILE_STATE };
        if (XR_FAILED(xrGetCurrentInteractionProfile(xr_session, userPath, &st))) return;

        char buf[512];
        if (st.interactionProfile != XR_NULL_PATH) {
            char pathStr[256] = {};
            uint32_t outCount = 0;
            xrPathToString(xr_instance, st.interactionProfile, sizeof(pathStr), &outCount, pathStr);
            sprintf_s(buf, "[XR] current profile for %s = %s\n", userPathStr, pathStr);
        }
        else {
            sprintf_s(buf, "[XR] current profile for %s = <none>\n", userPathStr);
        }
        OutputDebugStringA(buf);
        };
    dump("/user/hand/left");
    dump("/user/hand/right");
    return true;
}


bool OpenXRManager::XR_OK_(XrResult r, const char* where) {
    if (XR_FAILED(r)) {
        char name[128] = {};
        if (xr_instance != XR_NULL_HANDLE) xrResultToString(xr_instance, r, name);
        char buf[256];
        sprintf_s(buf, "[XR][ERR] %s -> 0x%08X (%s)\n", where, r, name[0] ? name : "unknown");
        OutputDebugStringA(buf);
        return false;
    }
    return true;
}

bool OpenXRManager::InitSimpleControllerTest() {


    if (xr_instance == XR_NULL_HANDLE || xr_session == XR_NULL_HANDLE) {
        OutputDebugStringA("[XR][ERR] InitSimpleControllerTest: xrInstance_ or xrSession_ is null\n");
        return false;
    }

    if (!XR_OK_(xrStringToPath(xr_instance, "/user/hand/left", &pathLeft_), "xrStringToPath left"))  return false;
    if (!XR_OK_(xrStringToPath(xr_instance, "/user/hand/right", &pathRight_), "xrStringToPath right")) return false;

    {
        XrActionSetCreateInfo ci{ XR_TYPE_ACTION_SET_CREATE_INFO };
        strncpy_s(ci.actionSetName, XR_MAX_ACTION_SET_NAME_SIZE, "test_set", _TRUNCATE);
        strncpy_s(ci.localizedActionSetName, XR_MAX_LOCALIZED_ACTION_SET_NAME_SIZE, "Test Set", _TRUNCATE);
        ci.priority = 0;

        if (!XR_OK_(xrCreateActionSet(xr_instance, &ci, &actionSet_), "xrCreateActionSet"))
            return false;
    }

    {
        XrActionCreateInfo ai{ XR_TYPE_ACTION_CREATE_INFO };
        ai.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
        strncpy_s(ai.actionName, XR_MAX_ACTION_NAME_SIZE, "select", _TRUNCATE);
        strncpy_s(ai.localizedActionName, XR_MAX_LOCALIZED_ACTION_NAME_SIZE, "Select", _TRUNCATE);

        XrPath subpaths[2] = { pathLeft_, pathRight_ };
        ai.countSubactionPaths = 2;
        ai.subactionPaths = subpaths;

        if (!XR_OK_(xrCreateAction(actionSet_, &ai, &actSelect_), "xrCreateAction(select)"))
            return false;
    }

    {
        XrPath ipOculus = XR_NULL_PATH;
        XrPath pathLeftX = XR_NULL_PATH;
        XrPath pathRightA = XR_NULL_PATH;

        //if (!XR_OK_(xrStringToPath(xr_instance, "/interaction_profiles/khr/simple_controller", &ipSimple), "xrStringToPath simple_controller")) return false;
        if (!XR_OK_(xrStringToPath(xr_instance, "/interaction_profiles/oculus/touch_controller", &ipOculus), "xrStringToPath oculus_controller")) return false;


        if (!XR_OK_(xrStringToPath(xr_instance, "/user/hand/left/input/x/click", &pathLeftX),
            "xrStringToPath left x")) return false;
        if (!XR_OK_(xrStringToPath(xr_instance, "/user/hand/right/input/a/click", &pathRightA),
            "xrStringToPath right a")) return false;

        XrActionSuggestedBinding binds[2] = {};
        binds[0].action = actSelect_;
        binds[0].binding = pathLeftX;
        binds[1].action = actSelect_;
        binds[1].binding = pathRightA;

        XrInteractionProfileSuggestedBinding profile{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
        profile.interactionProfile = ipOculus;
        profile.countSuggestedBindings = 2;
        profile.suggestedBindings = binds;

        if (!XR_OK_(xrSuggestInteractionProfileBindings(xr_instance, &profile),
            "xrSuggestInteractionProfileBindings"))
            return false;
    }

    {
        XrSessionActionSetsAttachInfo ai{ XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
        ai.countActionSets = 1;
        ai.actionSets = &actionSet_;
        if (!XR_OK_(xrAttachSessionActionSets(xr_session, &ai), "xrAttachSessionActionSets"))
            return false;
    }

    OutputDebugStringA("[XR] Simple controller test: init OK\n");
    return true;
}

void OpenXRManager::PollSimpleController() {

    if (actionSet_ == XR_NULL_HANDLE || actSelect_ == XR_NULL_HANDLE) {
        return;
    }

    XrActiveActionSet active{};
    active.actionSet = actionSet_;

    XrActionsSyncInfo sync{ XR_TYPE_ACTIONS_SYNC_INFO };
    sync.countActiveActionSets = 1;
    sync.activeActionSets = &active;

    if (!XR_OK_(xrSyncActions(xr_session, &sync), "xrSyncActions"))
        return;

    XrActionStateBoolean left{}; left.type = XR_TYPE_ACTION_STATE_BOOLEAN;
    XrActionStateBoolean right{}; right.type = XR_TYPE_ACTION_STATE_BOOLEAN;

    if (GetSelectState_(pathLeft_, left) && left.isActive && left.currentState) {
        OutputDebugStringA("[XR] LEFT select = DOWN\n");
    }
    if (GetSelectState_(pathRight_, right) && right.isActive && right.currentState) {
        OutputDebugStringA("[XR] RIGHT select = DOWN\n");
    }


    //LogCurrentInteractionProfiles();
    //Diag_CheckPathsAndReSuggest();
    //Diag_LogBasics();
    //PumpEventsOnce();
}

bool OpenXRManager::GetSelectState_(XrPath subPath, XrActionStateBoolean& out) {
    XrActionStateGetInfo gi{ XR_TYPE_ACTION_STATE_GET_INFO };
    gi.action = actSelect_;
    gi.subactionPath = subPath;

    if (!XR_OK_(xrGetActionStateBoolean(xr_session, &gi, &out), "xrGetActionStateBoolean")) {
        out.isActive = XR_FALSE;
        out.currentState = XR_FALSE;
        return false;
    }
    return true;
}

void OpenXRManager::ShutdownSimpleControllerTest() {
    if (actSelect_ != XR_NULL_HANDLE) {
        xrDestroyAction(actSelect_);
        actSelect_ = XR_NULL_HANDLE;
    }
    if (actionSet_ != XR_NULL_HANDLE) {
        xrDestroyActionSet(actionSet_);
        actionSet_ = XR_NULL_HANDLE;
    }
    pathLeft_ = XR_NULL_PATH;
    pathRight_ = XR_NULL_PATH;
}

void OpenXRManager::Diag_CheckPathsAndReSuggest() {
    auto chk = [&](const char* p) {
        XrPath x = XR_NULL_PATH;
        XrResult r = xrStringToPath(xr_instance, p, &x);
        char buf[256];
        sprintf_s(buf, "[XR] path %s -> %s (0x%08X)\n", p, XR_SUCCEEDED(r) ? "OK" : "NG", r);
        OutputDebugStringA(buf);
        };

    // プロファイル
    chk("/interaction_profiles/khr/simple_controller");
    chk("/interaction_profiles/oculus/touch_controller");
    chk("/interaction_profiles/valve/index_controller");
    chk("/interaction_profiles/htc/vive_controller");
    chk("/interaction_profiles/microsoft/motion_controller");

    // 入力パス（あなたが使っているもの）
    chk("/user/hand/left/input/select/click");
    chk("/user/hand/right/input/select/click");
    chk("/user/hand/left/input/x/click");
    chk("/user/hand/right/input/a/click");
    chk("/user/hand/left/input/trigger/value");
    chk("/user/hand/right/input/trigger/value");

    // 念のため Re-Suggest（Attach 済みでも Suggest は呼べます）
    //SuggestSimpleControllerSelect(xrInstance_, actSelect_);
    //SuggestOculusTouchTrigger(xrInstance_, actTriggerF_);
}


void OpenXRManager::Diag_LogBasics() {

    
    // runtime name
    XrInstanceProperties ip{ XR_TYPE_INSTANCE_PROPERTIES };
    if (XR_SUCCEEDED(xrGetInstanceProperties(xr_instance, &ip))) {
        char b[256];
        sprintf_s(b, "[XR] runtime=%s ver=%u.%u.%u\n",
            ip.runtimeName,
            XR_VERSION_MAJOR(ip.runtimeVersion),
            XR_VERSION_MINOR(ip.runtimeVersion),
            XR_VERSION_PATCH(ip.runtimeVersion));
        OutputDebugStringA(b);
    }

    // instance/session handles
    {
        char b[128];
        sprintf_s(b, "[XR] handles instance=0x%p session=0x%p\n",
            (void*)xr_instance, (void*)xr_session);
        OutputDebugStringA(b);
    }
    

    // 既にあなたのコードで保持しているキャッシュを表示してください
    // 例: cachedSessionState_ を 0..n の整数で
    {
        char b[128];
        sprintf_s(b, "[XR] cached session state=%d (expect FOCUSED=7 or VISIBLE=6)\n",
            (int)xr_sessionState);
        OutputDebugStringA(b);
    }
}

void OpenXRManager::PumpEventsOnce() {
    XrEventDataBuffer ev{ XR_TYPE_EVENT_DATA_BUFFER };
    while (xrPollEvent(xr_instance, &ev) == XR_SUCCESS) {
        if (ev.type == XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED) {
            XrEventDataInteractionProfileChanged* pc =
                (XrEventDataInteractionProfileChanged*)&ev;
            OutputDebugStringA("[XR] EVENT: InteractionProfileChanged\n");
            LogCurrentInteractionProfiles(); // ←あなたの既存関数
        }
        else if (ev.type == XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED) {
            XrEventDataSessionStateChanged* sc =
                (XrEventDataSessionStateChanged*)&ev;
            xr_sessionState = sc->state; // 例
            char b[128];
            sprintf_s(b, "[XR] EVENT: SessionStateChanged -> %d\n", (int)xr_sessionState);
            OutputDebugStringA(b);
        }
        ev = {}; ev.type = XR_TYPE_EVENT_DATA_BUFFER; // クリアして継続
    }
}
*/





