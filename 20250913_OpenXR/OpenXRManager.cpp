#include "OpenXRManager.h"
#include <sstream>
#include <cstring>

#include <chrono>
#include <thread>

XMINT2 OpenXRManager::recommendedResolution = { 1024, 1024 };
XMINT2 OpenXRManager::recommendedScaledResolution = { 1024, 1024 };


OpenXRManager::VrSupport OpenXRManager::ProbeSupportDX12() {

    //ランタイムと拡張の列挙
    OutputDebugStringA("ProbeSupportDX12\n");

    //拡張機能の 数のみ を取得
    uint32_t extCount = 0;
    XrResult r = xrEnumerateInstanceExtensionProperties(nullptr, 0, &extCount, nullptr);
    if (!XR_SUCCEEDED(r) || extCount == 0) {
        OutputDebugStringA("[ProbeXR] Runtime unavailable (enumerate failed).\n");
        return VrSupport::RuntimeUnavailable;
    }

    //std::vector<XrExtensionProperties> exts(extCount, { XR_TYPE_EXTENSION_PROPERTIES });

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

    /*
    auto supports = [&](const char* name) { //ラムダ式の書き方
        for (auto& e : exts) if (strcmp(e.extensionName, name) == 0) return true;
        return false;
        };

    if (!supports(XR_KHR_D3D12_ENABLE_EXTENSION_NAME)) {    //"XR_KHR_D3D12_enable"
        OutputDebugStringA("[ProbeXR] XR_KHR_D3D12_enable not supported.\n");
        return VrSupport::RuntimeNoD3D12;
    }
    */

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

    XrApplicationInfo appInfo{};

    {
        strncpy_s(appInfo.applicationName, "VR Probe", XR_MAX_APPLICATION_NAME_SIZE - 1);   //char[128]に文字をコピー
        appInfo.applicationVersion = 1;
        strncpy_s(appInfo.engineName, "Probe", XR_MAX_ENGINE_NAME_SIZE - 1);
        appInfo.engineVersion = 1;
        appInfo.apiVersion = XR_CURRENT_API_VERSION;
    }

    //XrInstanceCreateInfo ci{ XR_TYPE_INSTANCE_CREATE_INFO };
    XrInstanceCreateInfo ci = {};        // ゼロ初期化

    {
        ci.type = XR_TYPE_INSTANCE_CREATE_INFO;  // 必須フィールドを設定
        ci.applicationInfo = appInfo;

        ci.enabledExtensionCount = 0;           //拡張機能（D3D12_enable）を指定しないで、不要な負荷を避ける
        ci.enabledExtensionNames = nullptr;
    }

    XrInstance inst = XR_NULL_HANDLE;
    r = xrCreateInstance(&ci, &inst);
    if (!XR_SUCCEEDED(r) || inst == XR_NULL_HANDLE) {
        std::ostringstream oss; oss << "[ProbeXR] xrCreateInstance failed: " << r << "\n";
        OutputDebugStringA(oss.str().c_str());
        return VrSupport::InstanceFailed;
    }

    // HMD（SYSTEM）の有無
    XrSystemId systemId = XR_NULL_SYSTEM_ID;

    //XrSystemGetInfo si{ XR_TYPE_SYSTEM_GET_INFO };
    XrSystemGetInfo si = {};
    si.type = XR_TYPE_SYSTEM_GET_INFO;
    si.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
    r = xrGetSystem(inst, &si, &systemId);

    VrSupport result = VrSupport::NoHmd;
    if (XR_SUCCEEDED(r)) {
        result = VrSupport::Ready;

        // 基本プロパティ
        XrSystemProperties props = {};
        props.type = XR_TYPE_SYSTEM_PROPERTIES;
        XrResult r = xrGetSystemProperties(inst, systemId, &props);

        if (XR_SUCCEEDED(r)){
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
            uint32_t viewCount = 0;
            xrEnumerateViewConfigurationViews(inst, systemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, 0, &viewCount, nullptr);

            std::vector<XrViewConfigurationView> views(viewCount, { XR_TYPE_VIEW_CONFIGURATION_VIEW });
            xrEnumerateViewConfigurationViews(inst, systemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, viewCount, &viewCount, views.data());

            const float scaleDownRate = 0.8f;
            for (uint32_t i = 0; i < viewCount; i++) {

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


bool OpenXRManager::Initialize(ID3D12Device* d3d12Device, ID3D12CommandQueue* commandQ) {

    //初期化

    //入力チェック
    if (!d3d12Device || !commandQ) {
        OutputDebugStringA("Invalid D3D12 device or queue.\n");
        return false;
    }

    if (!createInstance()) return false;
    if (!getSystemId())    return false;
    if (!loadD3D12ReqFunc()) return false;
    if (!checkGraphicsRequirements(d3d12Device)) return false;
    if (!createSession(d3d12Device, commandQ)) return false;

    OutputDebugStringA("OpenXR initialized successfully.\n");

    return true;
}

/*
bool OpenXRManager::checkAndLoadExtensions() {  //利用可能な拡張機能一覧

    //利用可能な拡張機能一覧を問い合わせる
    //XR_KHR_D3D12_enable が使用できなかったら　false

    //どの拡張がサポートされているかはランタイム依存
    //（Meta Quest の OpenXR ランタイム、SteamVR の OpenXR ランタイムなど、ベンダーごとに違う）

    uint32_t extCount = 0;
    XrResult r = xrEnumerateInstanceExtensionProperties(nullptr, 0, &extCount, nullptr);
    if (!XR_SUCCEEDED(r)) {
        std::ostringstream oss;
        oss << "xrEnumerateInstanceExtensionProperties(count) failed: " << r << "\n";
        OutputDebugStringA(oss.str().c_str());
        return false;
    }

    std::vector<XrExtensionProperties> exts(extCount, { XR_TYPE_EXTENSION_PROPERTIES });
    r = xrEnumerateInstanceExtensionProperties(nullptr, extCount, &extCount, exts.data());
    if (!XR_SUCCEEDED(r)) {
        std::ostringstream oss;
        oss << "xrEnumerateInstanceExtensionProperties(list) failed: " << r << "\n";
        OutputDebugStringA(oss.str().c_str());
        return false;
    }

    auto supports = [&](const char* name) {
        for (auto& e : exts) if (strcmp(e.extensionName, name) == 0) return true;
        return false;
        };

    if (!supports(XR_KHR_D3D12_ENABLE_EXTENSION_NAME)) {
        OutputDebugStringA("Runtime does not support XR_KHR_D3D12_enable.\n");
        return false;
    }
    return true;
}
*/


bool OpenXRManager::createInstance() {


    /*
    //必須拡張（XR_KHR_D3D12_enable）がランタイムで使えるか事前確認
    if (!checkAndLoadExtensions()) {
        OutputDebugStringA("OpenXR: required extension (XR_KHR_D3D12_enable) not supported.\n");
        return false;
    }
    */


    const char* extensions[] = {
        XR_KHR_D3D12_ENABLE_EXTENSION_NAME
    };
    //const char* extensions[] = {
    //"XR_KHR_D3D12_enable" //XR_KHR_D3D12_ENABLE_EXTENSION_NAMEは、これのこと
    //};


    XrApplicationInfo appInfo{};
    strncpy_s(appInfo.applicationName, "My OpenXR App", XR_MAX_APPLICATION_NAME_SIZE - 1);  //アプリケーション名
    appInfo.applicationVersion = (1u << 16) | (0u << 8) | 0u; // 1.0.0 など任意
    strncpy_s(appInfo.engineName, "DX12 FromScratch", XR_MAX_ENGINE_NAME_SIZE - 1);         //エンジン名（DX12 スクラッチ
    //appInfo.engineVersion = 1;
    appInfo.engineVersion = (1u << 16) | (0u << 8) | 0u; // 1.0.0 など任意
    appInfo.apiVersion = XR_CURRENT_API_VERSION;

    XrInstanceCreateInfo ci{ XR_TYPE_INSTANCE_CREATE_INFO };
    ci.applicationInfo = appInfo;
    ci.enabledExtensionCount = (uint32_t)(sizeof(extensions) / sizeof(extensions[0]));
    ci.enabledExtensionNames = extensions;

    XrResult r = xrCreateInstance(&ci, &m_instance);
    if (!XR_SUCCEEDED(r)) {
        std::ostringstream oss;
        oss << "xrCreateInstance failed: " << r << "\n";
        OutputDebugStringA(oss.str().c_str());
        return false;
    }
    return true;
}

bool OpenXRManager::getSystemId() {
    XrSystemGetInfo si{ XR_TYPE_SYSTEM_GET_INFO };
    si.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

    XrResult r = xrGetSystem(m_instance, &si, &m_systemId);
    if (!XR_SUCCEEDED(r)) {
        std::ostringstream oss;
        oss << "xrGetSystem failed: " << r << "\n";
        OutputDebugStringA(oss.str().c_str());
        return false;
    }
    return true;
}

bool OpenXRManager::loadD3D12ReqFunc() {
    XrResult r = xrGetInstanceProcAddr(
        m_instance,
        "xrGetD3D12GraphicsRequirementsKHR",
        reinterpret_cast<PFN_xrVoidFunction*>(&m_pfnGetD3D12GraphicsRequirementsKHR)
    );
    if (!XR_SUCCEEDED(r) || !m_pfnGetD3D12GraphicsRequirementsKHR) {
        std::ostringstream oss;
        oss << "xrGetInstanceProcAddr(xrGetD3D12GraphicsRequirementsKHR) failed: " << r << "\n";
        OutputDebugStringA(oss.str().c_str());
        return false;
    }
    return true;
}

bool OpenXRManager::checkGraphicsRequirements(ID3D12Device* d3d12Device) {
    XrGraphicsRequirementsD3D12KHR req{ XR_TYPE_GRAPHICS_REQUIREMENTS_D3D12_KHR };
    XrResult r = m_pfnGetD3D12GraphicsRequirementsKHR(m_instance, m_systemId, &req);
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

bool OpenXRManager::createSession(ID3D12Device* d3d12Device, ID3D12CommandQueue* graphicsQueue) {
    if (!d3d12Device || !graphicsQueue) return false;

    XrGraphicsBindingD3D12KHR gb{ XR_TYPE_GRAPHICS_BINDING_D3D12_KHR };
    gb.next = nullptr;
    gb.device = d3d12Device;
    gb.queue = graphicsQueue;

    XrSessionCreateInfo sci{ XR_TYPE_SESSION_CREATE_INFO };
    sci.next = &gb;
    sci.systemId = m_systemId;

    XrResult r = xrCreateSession(m_instance, &sci, &m_session);
    if (!XR_SUCCEEDED(r)) {
        std::ostringstream oss;
        oss << "xrCreateSession failed: " << r << "\n";
        OutputDebugStringA(oss.str().c_str());
        return false;
    }
    return true;
}

XrInstance OpenXRManager::GetInstance() const { return m_instance; }
XrSession  OpenXRManager::GetSession()  const { return m_session; }
XrSystemId OpenXRManager::GetSystemId() const { return m_systemId; }

void OpenXRManager::PumpEventsOnce() {
    XrEventDataBuffer ev{ XR_TYPE_EVENT_DATA_BUFFER };
    XrResult r = xrPollEvent(m_instance, &ev);
    if (r != XR_SUCCESS) return;

    switch (ev.type) {
    case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
        const auto* s = reinterpret_cast<const XrEventDataSessionStateChanged*>(&ev);
        m_sessionState = s->state;
        if (m_sessionState == XR_SESSION_STATE_FOCUSED ||
            m_sessionState == XR_SESSION_STATE_VISIBLE ||
            m_sessionState == XR_SESSION_STATE_SYNCHRONIZED) {
            m_sessionRunning = true;
        }
        else if (m_sessionState == XR_SESSION_STATE_IDLE) {
            m_sessionRunning = false;
        }
        break;
    }
    default:
        break;
    }
}

bool OpenXRManager::RequestEnd() {
    if (m_session == XR_NULL_HANDLE) return true;

    switch (m_sessionState) {
    case XR_SESSION_STATE_FOCUSED:
    case XR_SESSION_STATE_VISIBLE:
    case XR_SESSION_STATE_SYNCHRONIZED: {
        XrResult r = xrEndSession(m_session);
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
    if (m_session == XR_NULL_HANDLE) return true;
    (void)RequestEnd();

    const auto t0 = std::chrono::steady_clock::now();
    while (true) {
        PumpEventsOnce();
        if (m_sessionState == XR_SESSION_STATE_IDLE ||
            m_sessionState == XR_SESSION_STATE_UNKNOWN ||
            m_sessionState == XR_SESSION_STATE_EXITING ||
            m_sessionState == XR_SESSION_STATE_LOSS_PENDING) {
            break;
        }
        const auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - t0).count() >= msTimeout) {
            OutputDebugStringA("EndSessionGracefully timeout.\n");
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    XrResult r = xrDestroySession(m_session);
    if (!XR_SUCCEEDED(r)) {
        std::ostringstream oss;
        oss << "xrDestroySession failed: " << r << "\n";
        OutputDebugStringA(oss.str().c_str());
        return false;
    }
    m_session = XR_NULL_HANDLE;
    m_sessionState = XR_SESSION_STATE_UNKNOWN;
    m_sessionRunning = false;
    return true;
}


void OpenXRManager::OnDestroy() {


    if (m_session != XR_NULL_HANDLE) {
        EndSessionGracefully(2000); // 2秒待ち
    }
    if (m_instance != XR_NULL_HANDLE) {
        xrDestroyInstance(m_instance);
        m_instance = XR_NULL_HANDLE;
    }

}

