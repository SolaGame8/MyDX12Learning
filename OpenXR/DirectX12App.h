#pragma once

#include <Windows.h>

#include <d3d12.h>
#include "d3dx12.h"
#include <dxgi1_6.h> // DXGI (DirectX Graphics Infrastructure)
#include <DirectXMath.h>

#include <wrl.h>    //ComPtr

#pragma comment(lib, "d3d12.lib")       //DirectX12 ���C�u����
#pragma comment(lib, "dxgi.lib")

#include "d3dcompiler.h"                //�V�F�[�_�[�R���p�C��
#pragma comment(lib, "d3dcompiler.lib") 

#include <chrono>   //����

#include <cstdlib>  // rand()��srand()�̂���
#include <ctime>    // time()�̂���
#include <random>   // �������C�u����


#include "TextureManager.h" //�e�N�X�`���[�ǂݍ��݁imap�Ńf�[�^�ێ��j

#include "PerlinNoise.h"    //�p�[�����m�C�Y�i�n�`�p�� �Ȃ��炩�ɂȂ��闐���j

#include "OpenXRManager.h"  //OpenXR


// using �錾��ǉ�
using Microsoft::WRL::ComPtr;
using namespace DirectX;


class DirectX12App
{
public:
    //DirectX12App();
    //~DirectX12App();


    bool Initialize(HWND hwnd);                 //������

    void InitVariable();                        //�ϐ��@�����ݒ�

    bool CreateDevice();                        //�f�o�C�X�쐬�i�g�p����O���t�B�b�N�{�[�h���擾
    
    bool CreateCommandObjects();                //�R�}���h�֘A�c�[��

    bool CreateSwapChain(HWND hwnd);            //�X���b�v�`�F�[���i��ʐ؂�ւ��j
    bool CreateRTV();                           //�����_�[�^�[�Q�b�g�i�X���b�v�`�F�[���̃��\�[�X�j
    bool CreateDepthBuffer();                   //�[�x�o�b�t�@

    float GetFloorHeight(float x, float y);     //�n�ʂ̍������擾

    bool LoadTextures();

    bool CreatShader(int idx);                  //�V�F�[�_�[
    bool CreateCBV();                           //CBV �萔�o�b�t�@�r���[       �i����ɐݒ肵�āA�V�F�[�_�[�� �ϐ� ��n��
    bool CreateSRV();                           //SRV �V�F�[�_�[���\�[�X�r���[  �i����ɐݒ肵�āA�V�F�[�_�[�� �e�N�X�`���[ ��n��

    bool CreatePipelineState(int idx);          //�p�C�v���C���X�e�[�g�i�`��̃��[������


    bool CreateVertex();                        //�`��p�̒��_�f�[�^���쐬
    void CreatePlate();                         //�|��
    void CreateFloor();                         //�n�ʃ|���S���i�n�ʂ̍����f�[�^���z��ŕێ��j



    void CalcCamera();                          //�J�����̌v�Z
    void CalculateFollowPosition();
    void CalcKey();                             //�L�[����

    float GetDeltaTime();                       //DeltaTime�쐬

    void OnUpdate();                            //���X�V


    void Render();                              //�`�揈��


    void WaitForGPUFinish();                    //�O���t�B�b�N�{�[�h�̏�����҂�

    void OnDestroy();                           //�A�v���P�[�V�����I������


private:


    //�[�[�[�[�[ �@�\�̏����p �[�[�[�[�[


    ComPtr<IDXGIFactory4> dxgiFactory;  //�t�@�N�g���[
    ComPtr<ID3D12Device> dx12Device;    //�O���t�B�b�N�{�[�h(�f�o�C�X)


    //�R�}���h�֘A�̃c�[��
    ComPtr<ID3D12GraphicsCommandList> commandList;      //�R�}���h������
    ComPtr<ID3D12CommandAllocator> commandAllocator;    //�R�}���h�̃������Ǘ�

    ComPtr<ID3D12CommandQueue> commandQueue;            //�R�}���h���s��

    ComPtr<ID3D12Fence> fence;      //�R�}���h���I���������Ď�
    UINT64 fenceValue = 0;
    HANDLE fenceEvent = nullptr;


    //�X���b�v�`�F�[���i��ʂ̐؂�ւ��j
    ComPtr<IDXGISwapChain3> swapChain;

    static const UINT FrameBufferCount = 2; //�Q���Ő؂�ւ�
    UINT currentFrameIndex = 0;     //�������݂Ɏg�����\�[�X�ԍ�

    //�����_�[�^�[�Q�b�g
    ComPtr<ID3D12DescriptorHeap> rtvHeap;                       //�q�[�v       �i���\�[�X�Q�����̏�񂪓���
    ComPtr<ID3D12Resource> renderTargets[FrameBufferCount];     //���\�[�X �Q��    1280 x 720

    //�[�x�o�b�t�@
    ComPtr<ID3D12DescriptorHeap> dsvHeap;                       //�q�[�v       �i�[�x�p�̃��\�[�X1�����̏�񂪓���
    ComPtr<ID3D12Resource> depthBuffer;

    //�[�[�[�[�[ OpenXR �[�[�[�[�[

    OpenXRManager* XR_Manager;

    bool flg_useVRMode = false;

    //�[�[�[�[�[ �`��̏����p �[�[�[�[�[

    //�𑜓x
    XMINT2 ResResolution = { 1280, 720 };


    //�V�F�[�_�[
    vector <ComPtr<ID3DBlob>> vertexShader;
    vector <ComPtr<ID3DBlob>> pixelShader;
    vector <ComPtr<ID3DBlob>> errorBlob;

    //���_�̍\����    �i���̃t�H�[�}�b�g�ŁA�V�F�[�_�[�ɒ��_��񂪑�����
    struct Vertex {
        XMFLOAT4 Pos;       //xyzw  float * 4 
        XMFLOAT4 Color;     //rgba  float * 4
        XMFLOAT4 UV;        //uv    float * 4
    };

    struct VertexInfo
    {
        UINT vertexBufferSize = 0u; //���_���i�@�\�I�ɂ͖��g�p�B�f�[�^��\�����������ȂǂɁj
        UINT indexBufferSize = 0u;  //���_�C���f�b�N�X���i�`�掞�ɂ��̐����g�p�j
    };


    //CBV �萔�o�b�t�@�i�V�F�[�_�[�ɓn���ϐ������Ă������\�[�X
    ComPtr<ID3D12Resource> constantBuffer;

    struct ConstantBufferData    //�i���̃t�H�[�}�b�g�ŁA�V�F�[�_�[�ɕϐ���񂪑�����
    {
        XMFLOAT4 shaderParam[8];

        XMMATRIX worldMat;
        XMMATRIX viewMat;
        XMMATRIX projMat;
    };

    ConstantBufferData conBufData = {};

    void* pConstData = nullptr;   //������constantBuffer�̃��\�[�X�ʒu�B�����ɕϐ��f�[�^���㏑������ƃV�F�[�_�[�ɓn�����l���X�V�����


    //SRV �V�F�[�_�[���\�[�X�r���[
    ComPtr<ID3D12DescriptorHeap> srvHeap;   //�q�[�v   �i�V�F�[�_�[�ɓn���e�N�X�`���[���\�[�X���B���\�[�X��TextureManager�ŊǗ�


    //�`��̃��[���݂����Ȃ̂�����������
    vector <ComPtr<ID3D12PipelineState>> pipelineState;          // �p�C�v���C���X�e�[�g
    vector <ComPtr<ID3D12RootSignature>> rootSignature;          // ���[�g�V�O�l�`��





    //�[�[�[�[�[ �`�悷��f�[�^�̏����p �[�[�[�[�[

    //[0] �|���A[1] �n�`
    vector <ComPtr<ID3D12Resource>> vertexBufferArray;                // ���_�������郊�\�[�X
    vector <D3D12_VERTEX_BUFFER_VIEW> vertexBufferViewArray;          // ���_�������郊�\�[�X�� �r���[�i���j

    vector <VertexInfo> vertexInfoArray;

    vector <ComPtr<ID3D12Resource>> indexBufferArray;                 // ���_�C���f�b�N�X�������郊�\�[�X
    vector <D3D12_INDEX_BUFFER_VIEW> indexBufferViewArray;            // ���_�C���f�b�N�X�������郊�\�[�X�� �r���[�i���j

    //�n�`�̍����ێ��p
    vector<vector<float>> floorHeightArray;




    //�[�[�[�[�[


    PerlinNoise* pNoise = nullptr;    //�p�[�����m�C�Y


    float updateCounter = 0.0f;
    //float shaderCounter = 0.0f;

    std::chrono::steady_clock::time_point lastFrameTime;    //DeltaTime�v�Z�p
    float deltaTime = 0.0f;


    //�J����
    XMFLOAT3 cameraPos = { 0.0f, 3.0f, -5.0f };    // �J�����̈ʒu
    XMFLOAT3 cameraRot = { 0.0f, 0.0f, 0.0f };  // �J�����̉�]
    XMFLOAT3 cameraTarget = { 0.0f, 0.0f, 0.0f };  // �J�����������_

    float distanceFromTarget = 5.0f; //�^�[�Q�b�g�Ǐ]�̋����im�j
    float distanceFromTarget_adjust = 1.0f; //�^�[�Q�b�g�Ǐ]�̍��������im�j

    //�}�E�X�@�i�ȑO�̈ʒu���L�^���Ă����āA�����ňړ��ʂ��v�Z
    POINT lastMousePos = {0, 0};

    //�L�����N�^�[
    XMFLOAT3 charaPos = { 0.0f, 0.0f, 0.0f };

    //�L�����A�j���[�V����
    int charaTexNum = 0;
    int charaTexAnimNum = 0;
    float charaCounter = 0.0f;

    //�d��
    float gravity = 0.98f * 0.5f;

    //�ڒn���Ă邩�ǂ���
    bool isOnGround = false;
    
    //�W�����v
    float charaJumpAcc = 0.0f;

    //���u�A�R�C���p

    vector<XMFLOAT3> mobPos;
    vector<int> mobUVIdx;

    float mobCounter = 0.0f;
    int mobTexAnimNum = 0;

    XMFLOAT3 mobTarget = { 0.0f, 0.0f, 0.0f, };




};


