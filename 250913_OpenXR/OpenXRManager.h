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
    #pragma comment(lib, "openxr_loader.lib")   //�������������[�X�p�̃��C�u����
#endif


#include "OpenXRController.h"       //�R���g���[���[�}�l�[�W���[


#include <vector>

using namespace DirectX;


class OpenXRManager {

public:


    // VR�̏�ԗp �񋓃f�[�^
    enum class VrSupport {
        Ready,               // �����^�C���L / D3D12�g������ / HMD���oOK
        NoHmd,               // �����^�C���͂��邪 HMD �����o
        RuntimeNoD3D12,      // �����^�C���� XR_KHR_D3D12_enable ������
        RuntimeUnavailable,  // �����^�C����������Ȃ�/����
        InstanceFailed       // �ŏ��C���X�^���X�쐬�Ɏ��s
    };

    //���ڂ̃J�����̍s��f�[�^
    struct EyeMatrix
    {
        XMMATRIX viewMat;           // �r���[�s�� (World��View)
        XMMATRIX projMat;           // �v���W�F�N�V�����s��
        XrView   xrView;            // OpenXR�̐��f�[�^
    };

    // Swapchain�^�[�Q�b�g���
    struct EyeDirectTarget {
        D3D12_CPU_DESCRIPTOR_HANDLE rtv;  // �����_�[�^�[�Q�b�g
        D3D12_CPU_DESCRIPTOR_HANDLE dsv;  // �[�x�o�b�t�@
        XMINT2 size;                      // ���̃^�[�Q�b�g�̉𑜓x
    };



    // VR�̏�Ԃ��擾
    static VrSupport CheckVRSupport(float resoScale);


    bool Initialize(ID3D12Device* d3d12Device, ID3D12CommandQueue* commQueue);

    bool CreateReferenceSpace(bool preferStage);

    bool CreateSwapchains(ID3D12Device* d3d12Device, 
        uint32_t viewCount,
        DXGI_FORMAT colorFmt,   // DXGI_FORMAT_R8G8B8A8_UNORM
        DXGI_FORMAT depthFmt,   // DXGI_FORMAT_D32_FLOAT
        XMINT2 size);           // ���𑜓x

    bool InitControllers();

    bool Start_XR_Session();    //���Z�b�V�����J�n

    bool BeginFrame(XrTime& predictedDisplayTime);

    XMMATRIX CreateCameraViewMatrix(const XrPosef& pose);
    XMMATRIX CreateProjectionMatrix(const XrFovf& fov, float nearZ, float farZ);

    bool GetEyeMatrix(XrTime predictedDisplayTime, float nearZ, float farZ, std::vector<EyeMatrix>& outEyes);

    bool GetSwapchainDrawTarget(ID3D12GraphicsCommandList* cmd, uint32_t eyeIndex, EyeDirectTarget& out);
    bool FinishSwapchainDrawTarget(ID3D12GraphicsCommandList* cmd, uint32_t eyeIndex);

    bool EndFrame_WithProjection(const std::vector<EyeMatrix>& eyesData, float nearZ, float farZ, XrTime displayTime);

    bool End_XR_Session();      //���Z�b�V�����I��


    void UpdateSessionState();  // �Z�b�V���������X�V   �i����𖈃t���[�����s���Ȃ��ƁA�R���g���[���[�̏����X�V����Ȃ��̂Œ���


    void OnDestroy();


    //�������߉𑜓x
    static float resolutionScale;

    static XMINT2 recommendedResolution;        //�w�b�h�}�E���g����擾���邨�����߉𑜓x
    static XMINT2 recommendedScaledResolution;  //0.8�{�ɂ��āA�g�p���Ă��܂�

    XrExtent2Df stageSize = { 0.0f, 0.0f };

    bool isStageSpace = false;                  //�X�e�[�W�X�y�[�X���ǂ����B�i�X�e�[�W�X�y�[�X�͏��̈ʒu������B���[�J���X�y�[�X�̓w�b�h�}�E���g��̃X�y�[�X�j


    static uint32_t xr_viewCount;               //�r���[�̐��B���� = 2

    //�R���g���[���[
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

    
    bool EndSessionGracefully(uint32_t msTimeout = 2000); // �I���菇

    void DestroySwapchains();        // �X���b�v�`�F�[���j��





};


