#pragma once

// �܂��}�N����`�iOpenXR �w�b�_���O�j
#define XR_USE_PLATFORM_WIN32       //Windows�v���b�g�t�H�[���ł�OpenXR�J����L���ɂ��܂��B
#define XR_USE_GRAPHICS_API_D3D12   //DirectX12�O���t�B�b�N�XAPI�̃T�|�[�g��L���ɂ��܂��B

// Windows / D3D12 ���ɓǂݍ���

#include <windows.h>    //OutputDebugStringA���g���̂ɕK�v
#include <d3d12.h>
#include "d3dx12.h"
#include <DirectXMath.h>
#include <dxgi1_6.h>
#pragma comment(lib, "dxgi.lib")

// �Ō�� OpenXR
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#ifdef _DEBUG
    #pragma comment(lib, "openxr_loaderd.lib")  //�f�o�b�O�p�̓��C�u������������Ă���
#else
    #pragma comment(lib, "openxr_loader.lib")
#endif

#include <vector>

using namespace DirectX;


class OpenXRManager {

public:


    // PCVR�iD3D12�j�����FVR���g�����Ԃ��̌y�ʔ���
    // �ˑ�: OpenXR�����^�C���������邱�ƁBD3D12�f�o�C�X�͕s�v�B
    enum class VrSupport {
        Ready,               // �����^�C���L / D3D12�g������ / HMD���oOK
        NoHmd,               // �����^�C���͂��邪 HMD �����o
        RuntimeNoD3D12,      // �����^�C���� XR_KHR_D3D12_enable ������
        RuntimeUnavailable,  // �����^�C����������Ȃ�/����
        InstanceFailed       // �ŏ��C���X�^���X�쐬�Ɏ��s
    };

    // �C���X�^���X�s�v�ŌĂׂ�y�ʃv���[�u�i�T������j
    static VrSupport ProbeSupportDX12();

    // �uReady���ǂ����v�����m�肽���ȈՔ�
    static bool IsVrReadyForDX12() { return ProbeSupportDX12() == VrSupport::Ready; }


    bool Initialize(ID3D12Device* d3d12Device, ID3D12CommandQueue* commQueue);

    bool CreateReferenceSpace(bool preferStage);

    bool CreateSwapchains(ID3D12Device* d3d12Device, 
        uint32_t viewCount,
        DXGI_FORMAT colorFmt,   // ��: DXGI_FORMAT_R8G8B8A8_UNORM
        DXGI_FORMAT depthFmt,   // ��: DXGI_FORMAT_D32_FLOAT
        XMINT2 size);           // ���𑜓x�i��: recommendedScaledResolution�j


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
        XMMATRIX viewMat;           // �r���[�s�� (World��View)
        XMMATRIX projMat;           // �v���W�F�N�V�����s��
        XrView   xrView;            // OpenXR�̐��f�[�^
    };

    // �Жڂցg���`���h����Ƃ��Ɏg���^�[�Q�b�g���
    struct EyeDirectTarget {
        D3D12_CPU_DESCRIPTOR_HANDLE rtv;  // ���̃t���[���̃X���b�v�`�F�[���摜���w��RTV
        D3D12_CPU_DESCRIPTOR_HANDLE dsv;  // ������Depth
        XMINT2 size;                      // �r���[�|�[�g/�V�U�[�p�iCreateSwapchains���̃T�C�Y�j
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



    bool End_XR_Session();  // �A�v��������I���v������ixrEndSession���s�j

    void OnDestroy();

    //�������߉𑜓x
    static XMINT2 recommendedResolution;
    static XMINT2 recommendedScaledResolution;//0.8

    XrExtent2Df stageSize = { 0.0f, 0.0f };

    bool isStageSpace = false;  //�X�e�[�W�X�y�[�X���ǂ���


    static uint32_t xr_viewCount; //�r���[�̐��B���� = 2

private:

    //ID3D12Device* d3d12Device_;

    XrInstance xr_instance = XR_NULL_HANDLE;
    XrSystemId xr_systemId = XR_NULL_SYSTEM_ID;
    XrSession  xr_session = XR_NULL_HANDLE;
    XrSpace    xr_appSpace = XR_NULL_HANDLE;

    

    // view ���Ƃ� 1�{���i�Жڂ��j���z��
    std::vector<XrSwapchain> xr_viewChainsColor;
    std::vector<XrSwapchain> xr_viewChainsDepth;

    // �e�`�F�[�������� D3D12 ���\�[�X�i�����^�C������󂯎��j
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

    void UpdateSessionState();                  // �Z�b�V���������X�V
    bool EndSessionGracefully(uint32_t msTimeout = 2000); // �I���菇

    void DestroySwapchains();        // �X���b�v�`�F�[���j��



};


