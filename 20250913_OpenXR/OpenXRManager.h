#pragma once

// �܂��}�N����`�iOpenXR �w�b�_���O�j
#define XR_USE_PLATFORM_WIN32       //Windows�v���b�g�t�H�[���ł�OpenXR�J����L���ɂ��܂��B
#define XR_USE_GRAPHICS_API_D3D12   //DirectX12�O���t�B�b�N�XAPI�̃T�|�[�g��L���ɂ��܂��B

// Windows / D3D12 ���ɓǂݍ���

#include <windows.h>    //OutputDebugStringA���g���̂ɕK�v
#include <d3d12.h>
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


    bool Initialize(ID3D12Device* d3d12Device, ID3D12CommandQueue* commandQ);

    XrInstance GetInstance() const;
    XrSession  GetSession() const;
    XrSystemId GetSystemId() const;

    bool RequestEnd();  // �A�v��������I���v������ixrEndSession���s�j

    void OnDestroy();

    //�������߉𑜓x
    static XMINT2 recommendedResolution;
    static XMINT2 recommendedScaledResolution;


private:

    XrInstance m_instance = XR_NULL_HANDLE;
    XrSystemId m_systemId = XR_NULL_SYSTEM_ID;
    XrSession  m_session = XR_NULL_HANDLE;

    // KHR_d3d12 �v���擾�֐��|�C���^
    PFN_xrGetD3D12GraphicsRequirementsKHR m_pfnGetD3D12GraphicsRequirementsKHR = nullptr;

    bool createInstance();
    //bool checkAndLoadExtensions();
    bool getSystemId();
    bool loadD3D12ReqFunc();
    bool checkGraphicsRequirements(ID3D12Device* d3d12Device);
    bool createSession(ID3D12Device* d3d12Device, ID3D12CommandQueue* graphicsQueue);

    XrSessionState m_sessionState = XR_SESSION_STATE_UNKNOWN;
    bool m_sessionRunning = false;

    void PumpEventsOnce();                  // �C�x���g1��|�[�����O
    bool EndSessionGracefully(uint32_t msTimeout = 2000); // �I���菇

    



};


