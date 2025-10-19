#include "DirectX12App.h"

//#include <Windows.h>
#include <d3d12sdklayers.h>
#include <sstream>
#include <cmath>

// �R���X�g���N�^
DirectX12App::DirectX12App() {

}

// �f�X�g���N�^
DirectX12App::~DirectX12App() {

}

// ���������\�b�h
bool DirectX12App::Initialize(HWND hwnd) {  //�E�C���h�E�̃n���h���̎󂯎��


#if defined(_DEBUG)

    // DirectX 12 �f�o�b�O���C���[��L���ɂ���    �i�f�o�b�O�r���h�ł̂ݗL��

    ID3D12Debug* debugController = nullptr;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();
    }

#endif


    ref_hwnd = hwnd;


    //�ϐ���ݒ�
    InitVariable();


    {
        //�[�[�[�[�[ �@�\�̏��� �[�[�[�[�[


        //�f�o�C�X�쐬�i�g�p����O���t�B�b�N�{�[�h�擾�j

        if (!CreateDevice()) {

            OutputDebugStringA("Failed to create D3D12 Device.\n");
            return false;
        }

        // �R�}���h�֘A�c�[���̍쐬
        if (!CreateCommandObjects()) {
            return false;
        }

        // �X���b�v�`�F�[���̍쐬  �i��ʐ؂�ւ��j
        if (!CreateSwapChain(hwnd)) {
            return false;
        }

        //�����_�[�^�[�Q�b�g       �i�X���b�v�`�F�[���̃��\�[�X�ɕ`�����ޗp�j
        if (!CreateRTV()) {
            return false;
        }

        //�[�x�o�b�t�@            �i���s�����r���āA��O�ɂ���|���S����`�悷��p
        if (!CreateDepthBuffer()) {
            return false;
        }

    }


    {
        //�[�[�[�[�[ �`��̏��� �[�[�[�[�[

        const int shaderNum = 1;

        //�V�F�[�_�[�쐬   �i0 �n�ʗp
        for (size_t i = 0; i < shaderNum; i++) {
            if (!CreateShader(i)) {
                return false;
            }
        }

        //������̓V�F�[�_�[�p��CBV��SRV���A�|���ƒn�ʁA���p�Ŏg���Ă���̂łP�̃f�[�^�ł�

        //CBV �萔�o�b�t�@�r���[�쐬       �i����ɐݒ肵�āA�V�F�[�_�[�� �ϐ� ��n��
        if (!CreateCBV()) {
            return false;
        }

        //SRV �V�F�[�_�[���\�[�X�r���[�쐬  �i����ɐݒ肵�āA�V�F�[�_�[�� �e�N�X�`���[ ��n��
        if (!CreateSRV()) {
            return false;
        }


        //�p�C�v���C���X�e�[�g�쐬  �i�`��̃��[������   �i0 �|���p, 1 �n�ʗp
        for (size_t i = 0; i < shaderNum; i++) {
            if (!CreatePipelineState(i)) {  //2��
                return false;
            }
        }

    }

    {
        //�[�[�[�[�[ �`�悷��f�[�^�̏��� �[�[�[�[�[


        //�`��p�̒��_�����쐬�i�|���A�n��

        if (!CreateVertex()) {
            return false;
        }

        //�e�N�X�`���[�f�[�^�ǂݍ���
        if (!LoadTextures()) {
            return false;
        }

    }


    return true;
}





void DirectX12App::InitVariable() {



    //�ϐ���ݒ�

    const int shaderNum = 1;

    pipelineState.resize(shaderNum);        //�`�惋�[�� 1�i�n��
    pipelineState_Grid.resize(shaderNum);
    rootSignature.resize(shaderNum);

    vertexShader.resize(shaderNum);     //�V�F�[�_�[ 1�i�n��
    hullShader.resize(shaderNum);
    domainShader.resize(shaderNum);
    pixelShader.resize(shaderNum);
    errorBlob.resize(shaderNum);


    const int meshNum = 1;

    vertexBufferArray.resize(meshNum);        //�`��p�@���_�f�[�^�@�Q�i�|���A�n��
    vertexBufferViewArray.resize(meshNum);

    indexBufferArray.resize(meshNum);         //�`��p�@���_�C���f�b�N�X�f�[�^�i�ǂ̒��_�ԍ����Ȃ����ĎO�p�`�ɂȂ��Ă��邩�̃f�[�^
    indexBufferViewArray.resize(meshNum);



    // �`�惋�[�v���J�n����O�ɁA�ŏ��̃t���[�����Ԃ��L�^
    lastFrameTime = std::chrono::steady_clock::now();

    // ���݂̃}�E�X�ʒu���L�^
    GetCursorPos(&lastMousePos);


}





bool DirectX12App::CreateDevice() {



    // DXGI�t�@�N�g���[�i�H��j�̍쐬
    if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory)))) {

        OutputDebugStringA("Failed to create DXGI Factory.\n");
        return false;
    }


    // �f�o�C�X�̍쐬�i�O���t�B�b�N�{�[�h���擾�j

    ComPtr<IDXGIAdapter1> hardwareAdapter;  //�A�_�v�^�[
    HRESULT hr = S_OK;
    UINT adapterIndex = 0;


    // ���[�v�ŃA�_�v�^�[���
    do {

        hr = dxgiFactory->EnumAdapters1(adapterIndex, &hardwareAdapter);    //�񋓁F�t�@�N�g���[����ЂƂ��A�_�v�^�[���󂯎��

        if (SUCCEEDED(hr)) {    //�󂯎�ꂽ��

            DXGI_ADAPTER_DESC1 desc;
            hardwareAdapter->GetDesc1(&desc);   //�������炤

            // �A�_�v�^�[�����o��
            OutputDebugStringW(L"Found Adapter: ");
            OutputDebugStringW(desc.Description);
            OutputDebugStringW(L"\n");

            // �\�t�g�E�F�A�A�_�v�^�[�iWARP�f�o�C�X�j���X�L�b�v
            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {  //�֌W�Ȃ�
                adapterIndex++;
                continue;
            }

            // D3D12�f�o�C�X���쐬�ł��邩����
            if (SUCCEEDED(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&dx12Device)
            ))) {

                OutputDebugStringA("Successfully created D3D12 Device.\n");
                break;

            }
            adapterIndex++;
        }

    } while (hr != DXGI_ERROR_NOT_FOUND); // �A�_�v�^�[���Ȃ��Ȃ�܂Ń��[�v


    if (!dx12Device) {
        return false;
    }


    return true;

}





bool DirectX12App::CreateCommandObjects() {

    //�R�}���h�֘A�̃c�[�����쐬



    //�R�}���h�A���P�[�^�@�i�R�}���h�̃������Ǘ��B �R�}���h���O���t�B�b�N�{�[�h�ɕۑ������
    if (FAILED(dx12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)))) {
        OutputDebugStringA("Failed to create Command Allocator.\n");
        return false;
    }

    //�R�}���h���X�g�i����ɃR�}���h��n���ƁA�֘A�t����ꂽ�A���P�[�^�Ƀf�[�^���n����āA�O���t�B�b�N�{�[�h�ɕۑ������
    if (FAILED(dx12Device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        commandAllocator.Get(),
        nullptr,
        IID_PPV_ARGS(&commandList)
    ))) {
        OutputDebugStringA("Failed to create Graphics Command List.\n");
        return false;
    }

    commandList->Close();   //�R�}���h�̓N���[�Y���Ă���A���s �i�`�惋�[�v���A�܂��R�}���h���X�g���N���A����̂ŁA�����ł�Close�ŕ��Ă���


    //�R�}���h�L���[�@�i���s��
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    if (FAILED(dx12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)))) {
        OutputDebugStringA("Failed to create Command Queue.\n");
        return false;
    }

    //�t�F���X�@�i�Ď���
    if (FAILED(dx12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)))) {
        OutputDebugStringA("Failed to create Fence.\n");
        return false;
    }

    fenceEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
    if (!fenceEvent) {
        OutputDebugStringA("Failed to create Fence Event.\n");
        return false;
    }

    OutputDebugStringA("Successfully created command objects.\n");

    return true;


}


bool DirectX12App::CreateSwapChain(HWND hwnd) {


    /*
    // �E�B���h�E�T�C�Y���擾����ꍇ
    RECT rect;  //�l�p�`
    GetClientRect(hwnd, &rect); //�����H�l�p�`�����擾
    UINT windowWidth = rect.right - rect.left;
    UINT windowHeight = rect.bottom - rect.top;
    */

    //����́A�Œ�̉𑜓x�ɂ��Ă��܂�
    UINT resolutionWidth = 1280;
    UINT resolutionHeight = 720;


    //�X���b�v�`�F�[���쐬�@�i��ʐ؂�ւ��@�\�j

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = resolutionWidth;      //w
    swapChainDesc.Height = resolutionHeight;    //h
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  //0-255, RGBA 8bit
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = FrameBufferCount;   //2���B�؂�ւ��Ďg��
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;   //�g��k���\
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = 0;

    // DXGI�X���b�v�`�F�[�����쐬�i���ʌ݊��H�j
    ComPtr<IDXGISwapChain1> swapChain1;
    if (FAILED(dxgiFactory->CreateSwapChainForHwnd(
        commandQueue.Get(),     //�X���b�v�`�F�[�����R�}���h�L���[�ɃA�N�Z�X�ł���悤�ɓn��
        hwnd,                   //�E�C���h�E�̃n���h��
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1
    ))) {
        OutputDebugStringA("Failed to create Swap Chain.\n");
        return false;
    }

    // �Â�DXGI���b�Z�[�W�𖳌��ɂ���
    if (FAILED(dxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER))) {
        OutputDebugStringA("Failed to disable DXGI messages.\n");
        return false;
    }

    // IDXGISwapChain3 �C���^�[�t�F�[�X���擾

    if (FAILED(swapChain1.As(&swapChain))) {    //As�ŁASwapchain3�Ɍp��
        OutputDebugStringA("Failed to get IDXGISwapChain3.\n");
        return false;
    }

    currentFrameIndex = swapChain->GetCurrentBackBufferIndex();//�`�悵�Ă��Ȃ����̃��\�[�X�ԍ����擾

    OutputDebugStringA("Successfully created Swap Chain.\n");
    return true;
}


bool DirectX12App::CreateRTV() {


    //�q�[�v
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = FrameBufferCount;          //2
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;      //�����_�[�^�[�Q�b�g�r���[�Ƃ��Ďg��
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    if (FAILED(dx12Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)))) {
        OutputDebugStringA("Failed to create RTV Descriptor Heap.\n");
        return false;
    }

    UINT rtvDescriptorSize = dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());

    for (UINT i = 0; i < FrameBufferCount; ++i) {   //2

        if (FAILED(swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i])))) {
            OutputDebugStringA("Failed to get swap chain buffer.\n");
            return false;
        }
        else {
            OutputDebugStringA("Success to get swap chain buffer.\n");

        }

        dx12Device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvHandle);//�r���[

        rtvHandle.Offset(1, rtvDescriptorSize);
    }

    OutputDebugStringA("Successfully created RTV and RTV heap.\n");

    return true;

}





// ���������\�b�h���ŌĂяo��
bool DirectX12App::CreateDepthBuffer() {


    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};

    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;  //�[�x�p
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (FAILED(dx12Device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap)))) {
        return false;
    }


    D3D12_RESOURCE_DESC depthStencilDesc = {};
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = 1280; // �E�B���h�E��
    depthStencilDesc.Height = 720; // �E�B���h�E����
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;    //32bit
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    clearValue.DepthStencil.Depth = 1.0f; // �[�x�l�̃N���A�ݒ�
    clearValue.DepthStencil.Stencil = 0;


    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);

    if (FAILED(dx12Device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &depthStencilDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &clearValue,
        IID_PPV_ARGS(&depthBuffer)))) {

        return false;
    }

    dx12Device->CreateDepthStencilView(depthBuffer.Get(), nullptr, dsvHeap->GetCPUDescriptorHandleForHeapStart());


    return true;
}







bool DirectX12App::CreateShader(int idx) {

    HRESULT hr = S_OK;

    std::wstring shaderPath;

    if (idx == 0) {

        CompileShader(L"GridTess_VS.hlsl", "VS_Main", "vs_5_0", &vertexShader[idx]);
        CompileShader(L"GridTess_HS.hlsl", "HS_Main", "hs_5_0", &hullShader[idx]);
        CompileShader(L"GridTess_DS.hlsl", "DS_Main", "ds_5_0", &domainShader[idx]);
        CompileShader(L"GridTess_PS.hlsl", "PS_Main", "ps_5_0", &pixelShader[idx]);

    }



    /*
    switch (idx) {
    case 0:
        shaderPath = L"FloorShader.hlsl";
        break;
    default:
        shaderPath = L"FloorShader.hlsl";
        break;
    }



    // ���_�V�F�[�_�[�̃R���p�C��
    hr = D3DCompileFromFile(shaderPath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "VSMain", "vs_5_0", 
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vertexShader[idx], &errorBlob[idx]);

    if (FAILED(hr)) {
        // �G���[����
        if (errorBlob[idx]) {
            OutputDebugStringA("Vertex Shader Compile Error:\n");
            OutputDebugStringA((char*)errorBlob[idx]->GetBufferPointer());
        }
        return false;
    }

    // �s�N�Z���V�F�[�_�[�̃R���p�C��
    hr = D3DCompileFromFile(shaderPath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "PSMain", "ps_5_0", 
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pixelShader[idx], &errorBlob[idx]);

    if (FAILED(hr)) {
        // �G���[����
        if (errorBlob[idx]) {
            OutputDebugStringA("Pixel Shader Compile Error:\n");
            OutputDebugStringA((char*)errorBlob[idx]->GetBufferPointer());
        }
        return false;
    }
    */

    OutputDebugStringA("Finished Shader Compile\n");
    

    return true;

}


void DirectX12App::CompileShader (
    const wchar_t* path, const char* entry, const char* target, ID3DBlob** outBlob) {


    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    ComPtr<ID3DBlob> shader, error;
    HRESULT hr = D3DCompileFromFile(
        path,                      // HLSL �t�@�C��
        nullptr,                   // �}�N��
        D3D_COMPILE_STANDARD_FILE_INCLUDE, // #include �Ή�
        entry, target,             // �G���g���� / �v���t�@�C��
        flags, 0,                  // compile flags / effect flags
        &shader, &error);

    if (FAILED(hr)) {
        if (error) {
            OutputDebugStringA((const char*)error->GetBufferPointer());
        }
        throw std::runtime_error("D3DCompileFromFile failed.");
    }
    *outBlob = shader.Detach();
}



bool DirectX12App::CreateCBV() {


    // �萔�o�b�t�@�Ƃ��Ďg���A�b�v���[�h�q�[�v���\�[�X���쐬
    CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);    //Upload(CPU��GPU��������ꏊ)    //Default(GPU��p�B����)
    
    
    // �萔�o�b�t�@��256�o�C�g�̔{���ɂ���K�v������
    const UINT constantBufferSize = (sizeof(ConstantBufferData) + 255) & ~255;
    CD3DX12_RESOURCE_DESC cbDesc = CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize);

    if (FAILED(dx12Device->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &cbDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&constantBuffer)))) {
        OutputDebugStringA("Failed to create constant buffer.\n");
        return false;
    }

    // �f�[�^�̃}�b�s���O
    
    CD3DX12_RANGE readRange(0, 0); // CPU�͓ǂݍ��܂Ȃ����ߋ�
    constantBuffer->Map(0, &readRange, &pConstData);
    
    memcpy(pConstData, &conBufData, sizeof(ConstantBufferData));

    // �}�b�v���������Ȃ��i�p�����āAmemcpy�ōX�V���邽��
    //constantBuffer->Unmap(0, nullptr);


    return true;

}


bool DirectX12App::CreateSRV() {

    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};

    srvHeapDesc.NumDescriptors = 8; // �e�N�X�`���̖����ɍ��킹�ăT�C�Y������

    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;  //cbv �ϐ��A SRV �e�N�X�`���[, UAV �����_���A�N�Z�X
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    HRESULT hr = dx12Device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap));

    if (FAILED(hr)) {
        OutputDebugStringA("Failed to create Main Descriptor Heap.\n");
        return false;
    }

    return true;

}







bool DirectX12App::CreatePipelineState(int idx) {
    


    HRESULT hr = S_OK;
    
    // ���[�g�V�O�l�`���̍쐬

    
    // ���[�g�p�����[�^�̒�`
    CD3DX12_ROOT_PARAMETER rootParameters[2];
    // �萔�o�b�t�@�r���[�iCBV�j�����[�g�p�����[�^�Ƃ��Ēǉ�
    // b0 ���W�X�^���o�C���h
    rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);  //CBV �萔�o�b�t�@

    CD3DX12_DESCRIPTOR_RANGE ranges[1];
    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 8, 0); // ������SRV

    rootParameters[1].InitAsDescriptorTable(1, ranges, D3D12_SHADER_VISIBILITY_PIXEL); // �e�N�X�`���[


    // �T���v���[�̒�`
    CD3DX12_STATIC_SAMPLER_DESC samplerDesc(
        0, // �T���v���[���W�X�^ s0
        D3D12_FILTER_MIN_MAG_MIP_LINEAR,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP);


    // ���[�g�V�O�l�`���̍쐬
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(
        _countof(rootParameters), // �p�����[�^��: 2
        rootParameters,
        1, 
        &samplerDesc,   //�T���v���[
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    );

    // �V���A���C�Y�i�o�C�i���f�[�^�ւ̕ϊ��j
    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;

    hr = D3D12SerializeRootSignature(
        &rootSignatureDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &signature,
        &error
    );

    if (FAILED(hr)) {
        if (error) {
            OutputDebugStringA((char*)error->GetBufferPointer());
        }
        OutputDebugStringA("Failed to serialize Root Signature.\n");
        return false;
    }

    // ���[�g�V�O�l�`���̍쐬
    hr = dx12Device->CreateRootSignature(
        0,
        signature->GetBufferPointer(),
        signature->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature[idx])
    );

    if (FAILED(hr)) {
        OutputDebugStringA("Failed to create Root Signature.\n");
        return false;
    }





    //�`��̃��[������


    //���_���C�A�E�g�@�i�V�F�[�_�[�ɑ��钸�_�f�[�^�̃t�H�[�}�b�g
    D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };


    //�u�����h�̐ݒ�
    D3D12_BLEND_DESC blendDesc = {};

    blendDesc.AlphaToCoverageEnable = true;     //�e�N�X�`���[�̓��������𓧉߂�����

    blendDesc.IndependentBlendEnable = FALSE;   //�S�Ẵ����_�[�^�[�Q�b�g�œ����ݒ���g�p

    blendDesc.RenderTarget[0].BlendEnable = false;

    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;  // �A���t�@�l���̂̃u�����h�𐧌�i�ʏ�͂��̂܂܁j
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

    blendDesc.RenderTarget[0].LogicOpEnable = FALSE;    
    blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;// �������ރJ���[�}�X�N


    // �p�C�v���C���X�e�[�g�I�u�W�F�N�g (PSO) �̍쐬   ���`��̃��[��

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

    psoDesc.InputLayout = { inputElementDesc, _countof(inputElementDesc) }; //���_���C�A�E�g

    psoDesc.pRootSignature = rootSignature[idx].Get();                      //���[�g�V�O�l�`��


    //�e�b�Z���[�V�����p�V�F�[�_�[

    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader[idx].Get());          //���_�V�F�[�_�[�i�R���p�C���ς�
    psoDesc.HS = CD3DX12_SHADER_BYTECODE(hullShader[idx].Get());            //�n���V�F�[�_�[�i�R���p�C���ς�
    psoDesc.DS = CD3DX12_SHADER_BYTECODE(domainShader[idx].Get());          //�h���C���V�F�[�_�[�i�R���p�C���ς�
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader[idx].Get());           //�s�N�Z���V�F�[�_�[�i�R���p�C���ς�




    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);       //���X�^���C�Y�̐ݒ�
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;                //�O�p�`�̗��ʂ��`��
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;               //�ʏ탂�[�h     //���C���[ D3D12_FILL_MODE_WIREFRAME

    psoDesc.BlendState = blendDesc;                                         //�u�����h�̐ݒ�


    
    psoDesc.DepthStencilState.DepthEnable = TRUE;                           //�[�x�o�b�t�@��L���ɂ���
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;  //�[�x�̏������݂�L���ɂ���
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;       //�[�x�e�X�g�̏����i�l�����������i�J�����ɋ߂����j��`��
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;                              //�[�x�o�b�t�@�̃t�H�[�}�b�g�i32bit float

    psoDesc.SampleMask = UINT_MAX;



    //psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; //�O�p�`��`�悷��i�ʏ�j

    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH; // �����������i�d�v�j�e�b�Z���[�V������ PATCH



    psoDesc.NumRenderTargets = 1;                                           //�`�悷�郊�\�[�X��

    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;                     //�`�悷�郊�\�[�X�̃t�H�[�}�b�g�i8bit 0-255, RGBA

    psoDesc.SampleDesc.Count = 1;
    psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;


    //�p�C�v���C���X�e�[�g�쐬�i�O���t�B�b�N�{�[�h�ɕ`�惋�[�����ۑ������
    hr = dx12Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState[idx]));

    if (FAILED(hr)) {
        OutputDebugStringA("Failed to create Pipeline State Object.\n");
        return false;
    }

    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;// ���C���[�t���[��

    hr = dx12Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState_Grid[idx])); //�O���b�h�p�p�C�v���C��

    if (FAILED(hr)) {
        OutputDebugStringA("Failed to create Pipeline State Object.\n");
        return false;
    }

    OutputDebugStringA("Successfully initialized all components for drawing.\n");


    return true;

}








bool DirectX12App::CreateVertex() {

    //�`�悷�钸�_�����쐬

    CreateFloor();  //�n��

    return true;

}





void DirectX12App::CreateFloor() {

    //��

    //���_�������

    HRESULT hr = S_OK;

    int myIdx = 0;

    VertexInfo vertInfo;



    {
        vector<Vertex> vertexArr;

        vector<WORD> indiceArr;


        float gridSize = 1.0f;  //�P�}�X�̃T�C�Y
        int floor_length = 64;  //64 * 64 * 4   ���_
        float startX = -gridSize * floor_length * 0.5f;
        float startY = -gridSize * floor_length * 0.5f;

        int indiceCounter = 0;

        float margin = 0.0f;
        float blockSize = 1.0f - margin;


        float sideX, sideZ;
        float fh = 0.0f;


        for (size_t y = 0; y < floor_length; y++) { //64

            vector<float>heightArr; //�������

            for (size_t x = 0; x < floor_length; x++) { //64



                //�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�@���

                //���_���

                {
                    //0 ����
                    Vertex vert;
                    vert.Pos = XMFLOAT4(startX + gridSize * x, fh, startY + gridSize * y, 1.0f);
                    vert.UV = XMFLOAT4(margin, margin, 0.0f, 0.0f);
                    vert.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

                    vertexArr.emplace_back(vert);
                }
                {
                    //1 �E��
                    Vertex vert;
                    vert.UV = XMFLOAT4(blockSize, margin, 0.0f, 0.0f);
                    vert.Pos = XMFLOAT4(startX + gridSize * (x + 1.0f), fh, startY + gridSize * (y + 0.0f), 1.0f);
                    vert.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

                    vertexArr.emplace_back(vert);
                }
                {
                    //2 ����
                    Vertex vert;
                    vert.UV = XMFLOAT4(margin, blockSize, 0.0f, 0.0f);
                    vert.Pos = XMFLOAT4(startX + gridSize * (x + 0.0f), fh, startY + gridSize * (y + 1.0f), 1.0f);
                    vert.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

                    vertexArr.emplace_back(vert);
                }
                {
                    //3 �E��
                    Vertex vert;
                    vert.UV = XMFLOAT4(blockSize, blockSize, 0.0f, 0.0f);
                    vert.Pos = XMFLOAT4(startX + gridSize * (x + 1.0f), fh, startY + gridSize * (y + 1.0f), 1.0f);
                    vert.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

                    vertexArr.emplace_back(vert);
                }

                //���_�C���f�b�N�X���i�ǂ̒��_���Ȃ����ĎO�p�`�ɂȂ邩�j

                indiceArr.emplace_back(indiceCounter + 0);
                indiceArr.emplace_back(indiceCounter + 1);
                indiceArr.emplace_back(indiceCounter + 2);

                indiceArr.emplace_back(indiceCounter + 1);
                indiceArr.emplace_back(indiceCounter + 3);
                indiceArr.emplace_back(indiceCounter + 2);

                indiceCounter += 4;




                //��������z��ɓ����
                heightArr.emplace_back(fh);

            }


        }




        vertInfo.vertexBufferSize = sizeof(Vertex) * vertexArr.size(); //64 * 64 * 4   ���_�T�C�Y


        // ���_�o�b�t�@�i���\�[�X�j�̍쐬
        CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);    //�A�b�v���[�h�iCPU�j     //�f�t�H���g�iGPU�j
        CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertInfo.vertexBufferSize);

        hr = dx12Device->CreateCommittedResource(
            &uploadHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,  //�ǂݎ��
            nullptr,
            IID_PPV_ARGS(&vertexBufferArray[myIdx]));

        if (FAILED(hr)) {
            OutputDebugStringA("Failed to create vertex buffer.\n");
            return;
        }

        // �f�[�^���o�b�t�@�i���\�[�X�j�ɃR�s�[
        void* pVertexData;
        CD3DX12_RANGE readRange(0, 0);
        vertexBufferArray[myIdx]->Map(0, &readRange, &pVertexData);
        memcpy(pVertexData, vertexArr.data(), vertInfo.vertexBufferSize);    //�������ݏꏊ, �R�s�[���钸�_�f�[�^
        vertexBufferArray[myIdx]->Unmap(0, nullptr);

        // ���_�o�b�t�@�� �r���[�i���\�[�X�̏��j���쐬
        vertexBufferViewArray[myIdx].BufferLocation = vertexBufferArray[myIdx]->GetGPUVirtualAddress();
        vertexBufferViewArray[myIdx].StrideInBytes = sizeof(Vertex);
        vertexBufferViewArray[myIdx].SizeInBytes = vertInfo.vertexBufferSize;


        //�C���f�b�N�X�o�b�t�@

        UINT indexBufferSize = sizeof(WORD) * indiceArr.size();    //6
        vertInfo.indexBufferSize = indiceArr.size();    //64 * 64 * 6 

        // �C���f�b�N�X�o�b�t�@�i���\�[�X�j�̍쐬
        CD3DX12_HEAP_PROPERTIES uploadHeapProps2(D3D12_HEAP_TYPE_UPLOAD);    //�A�b�v���[�h�iCPU�j     //�f�t�H���g�iGPU�j
        CD3DX12_RESOURCE_DESC bufferDesc2 = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);  //6

        hr = dx12Device->CreateCommittedResource(
            &uploadHeapProps2,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc2,
            D3D12_RESOURCE_STATE_GENERIC_READ,  //�ǂݎ��
            nullptr,
            IID_PPV_ARGS(&indexBufferArray[myIdx]));

        if (FAILED(hr)) {
            OutputDebugStringA("Failed to create index buffer.\n");
            return;
        }

        // �f�[�^���C���f�b�N�X�o�b�t�@�i���\�[�X�j�ɃR�s�[
        void* pIndexData;
        //CD3DX12_RANGE readRange(0, 0);

        indexBufferArray[myIdx]->Map(0, &readRange, &pIndexData);
        memcpy(pIndexData, indiceArr.data(), indexBufferSize);   //�������ݏꏊ, �R�s�[���钸�_�f�[�^
        indexBufferArray[myIdx]->Unmap(0, nullptr);

        // �C���f�b�N�X�o�b�t�@�� �r���[�i���\�[�X�̏��j���쐬
        indexBufferViewArray[myIdx].BufferLocation = indexBufferArray[myIdx]->GetGPUVirtualAddress();
        indexBufferViewArray[myIdx].Format = DXGI_FORMAT_R16_UINT; // WORD (16�r�b�g) �̂���R16_UINT  ��
        indexBufferViewArray[myIdx].SizeInBytes = indexBufferSize;

    }

    //���_���̏���z��ɓ����
    vertexInfoArray.emplace_back(vertInfo);


}




bool DirectX12App::LoadTextures() {

    //�e�N�X�`���[�̓ǂݍ���


    if (!TextureManager::GetInstance().LoadTexture(
        dx12Device.Get(),
        "Resources/floor001.png",
        "floor001"
    )) {

        OutputDebugStringA("Failed to load Texture........\n");
        return false;
    }


    // SRV�̃q�[�v�̃n���h��
    CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(srvHeap->GetCPUDescriptorHandleForHeapStart());
    UINT srvDescriptorSize = dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);



    {
        // �e�N�X�`���� �r���[�i���j���쐬
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = TextureManager::GetInstance().GetTextureResource("floor001")->GetDesc().Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        dx12Device->CreateShaderResourceView(TextureManager::GetInstance().GetTextureResource("floor001").Get(), &srvDesc, srvHandle);

        srvHandle.Offset(1, srvDescriptorSize); //���̃q�[�v���̓���

    }

    return true;

}






void DirectX12App::CalcCamera() {

    //�J�����̕ϊ��s��@�i�V�F�[�_�[�ɓn���ϐ����X�V

    XMFLOAT3 cameraUp = { 0.0f, 1.0f, 0.0f };      // �J�����̏�����x�N�g��

    XMFLOAT3 cameraCurrentTarget;
    cameraCurrentTarget.x = cameraTarget.x;
    cameraCurrentTarget.y = cameraTarget.y + distanceFromTarget_adjust;
    cameraCurrentTarget.z = cameraTarget.z;

    XMVECTOR eye = XMLoadFloat3(&cameraPos);
    XMVECTOR at = XMLoadFloat3(&cameraCurrentTarget);
    XMVECTOR up = XMLoadFloat3(&cameraUp);

    conBufData.viewMat = XMMatrixLookAtLH(eye, at, up); //�r���[

    float fovAngleY = XM_PIDIV4; // ����p (45�x)
    float aspectRatio = 1280.0f / 720.0f; // ��ʉ𑜓x����A�X�y�N�g����v�Z
    float nearZ = 0.01f; // �߃N���b�v��
    float farZ = 100.0f; // ���N���b�v��

    conBufData.projMat = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, nearZ, farZ); //�v���W�F�N�V����

    //���f���̉�]�p
    conBufData.worldMat = XMMatrixIdentity();



}


void DirectX12App::CalculateFollowPosition() {


    //�J�����Ǐ]�ʒu�̌v�Z    �i�^�[�Q�b�g�𒆐S�Ɉ�苗���ł܂��

    float len = distanceFromTarget;

    float cx = -(float)sin(-XM_PI * cameraRot.y / 180.0f) * len * cos(XM_PI * cameraRot.x / 180.0f);//��] �i�E+ ��-�j
    float cy = -(float)cos(-XM_PI * cameraRot.y / 180.0f) * len * cos(XM_PI * cameraRot.x / 180.0f);
    float ch = (float)sin(-XM_PI * cameraRot.x / 180.0f) * len;//���� �i��+ ��-�j

    cameraPos.x = cameraTarget.x + cx;
    cameraPos.y = (cameraTarget.y + distanceFromTarget_adjust) + ch;
    cameraPos.z = cameraTarget.z + cy;


}


XMFLOAT2 DirectX12App::ScreenToFloorXZ() {

    //�J�[�\�����w���Ă��鏰�̏ꏊ(x,z)   //�q�b�g�Ȃ��� (0,0)

    // �E�C���h�E�@�N���C�A���g�̈�T�C�Y
    RECT rc;
    if (!GetClientRect(ref_hwnd, &rc)) return XMFLOAT2(0.0f, 0.0f);
    const float width = (float)(rc.right - rc.left);
    const float height = (float)(rc.bottom - rc.top);
    if (width <= 0.0f || height <= 0.0f) return XMFLOAT2(0.0f, 0.0f);

    // �}�E�X���W�i�N���C�A���g���W�j
    POINT pt;
    if (!GetCursorPos(&pt))         return XMFLOAT2(0.0f, 0.0f);
    if (!ScreenToClient(ref_hwnd, &pt)) return XMFLOAT2(0.0f, 0.0f);
    const float mx = (float)pt.x;
    const float my = (float)pt.y;

    // �E�C���h�E���W -> NDC�iDirect3D: ��:+1, ��:-1�j
    const float ndcX = (mx / width) * 2.0f - 1.0f;
    const float ndcY = 1.0f - (my / height) * 2.0f;


    XMMATRIX ViewProjMat = conBufData.viewMat * conBufData.projMat;


    // �t�s��(ViewProj)
    XMVECTOR det;
    const XMMATRIX invViewProj = XMMatrixInverse(&det, ViewProjMat);

    XMVECTOR pNear = XMVectorSet(ndcX, ndcY, 0.0f, 1.0f);
    XMVECTOR pFar = XMVectorSet(ndcX, ndcY, 1.0f, 1.0f);

    pNear = XMVector4Transform(pNear, invViewProj);
    pFar = XMVector4Transform(pFar, invViewProj);

    // �������Z
    pNear = XMVectorScale(pNear, 1.0f / XMVectorGetW(pNear));
    pFar = XMVectorScale(pFar, 1.0f / XMVectorGetW(pFar));

    // ���C
    const XMVECTOR rayOrig = pNear;
    XMVECTOR rayDir = XMVector3Normalize(XMVectorSubtract(pFar, pNear));

    const float origY = XMVectorGetY(rayOrig);
    const float dirY = XMVectorGetY(rayDir);

    // ��(y=0)�ƕ��s
    const float eps = 1e-6f;
    if (dirY > -eps && dirY < eps) return XMFLOAT2(0.0f, 0.0f);

    const float t = -origY / dirY;
    
    //�w�ʑ��̓q�b�g�Ȃ�
    //if (t < 0.0f) return XMFLOAT2(0.0f, 0.0f);

    // ��_
    const XMVECTOR hit = XMVectorMultiplyAdd(rayDir, XMVectorReplicate(t), rayOrig);
    return XMFLOAT2(XMVectorGetX(hit), XMVectorGetZ(hit)); // (x,z)


}



void DirectX12App::CalcKey() {


    const float pullSpeed = 3.0f;

    //�}�E�X���i�n�`�̕ό`�j
    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {

        if (!flg_pushMouseBtn_L) {
            //OnPush
        }

        pullStrength += pullSpeed * deltaTime;

        if (pullStrength >= 1.0f) {
            pullStrength = 1.0f;
        }

        flg_pushMouseBtn_L = true;
    }
    else {

        pullStrength -= pullSpeed * deltaTime;

        if (pullStrength <= 0.0f) {
            pullStrength = 0.0f;
        }

        flg_pushMouseBtn_L = false;
    }


    //�}�E�X�E�i�O���b�h / �e�N�X�`���[�\���؂�ւ��j
    if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) {

        if (!flg_pushMouseBtn_R) {
            //OnPush
            showGrid = !showGrid;
        }
        flg_pushMouseBtn_R = true;
    }
    else {

        flg_pushMouseBtn_R = false;
    }






}



float DirectX12App::GetDeltaTime() {

    // ���݂̎������擾
    std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();

    // �Ō�ɌĂ΂�Ă���̌o�ߎ��Ԃ��v�Z
    std::chrono::duration<float> deltaTime = currentTime - lastFrameTime;

    // ���̃t���[���̂��߂Ɍ��݂̎�����ۑ�
    lastFrameTime = currentTime;

    // �b�P�ʂ̃f���^�^�C����Ԃ�
    return deltaTime.count();
}



void DirectX12App::OnUpdate() {


    deltaTime = GetDeltaTime();

    updateCounter += deltaTime;


    //�L�[����
    CalcKey();


    cameraRot.y += 5.0f * deltaTime;
    cameraRot.x = -30.0f + 10.0f * sin(XM_PI * 0.02f * updateCounter);


    //�Ǐ]�ʒu�̌v�Z�i�^�[�Q�b�g�𒆐S�Ɉ�苗���ŉ�铮���j
    CalculateFollowPosition();

    //�J�����̌v�Z
    CalcCamera();


    //�J�[�\�����w���Ă��鏰�̏ꏊ
    XMFLOAT2 floorCursolPos = ScreenToFloorXZ();


    //�V�F�[�_�[�ɓn���A�L�����N�^�[�̈ʒu���
    conBufData.shaderParam[0].x = floorCursolPos.x;
    conBufData.shaderParam[0].y = floorCursolPos.y;
    conBufData.shaderParam[0].z = sin(XM_PI * 0.5f *  pullStrength); //0.0 �` 1.0
    conBufData.shaderParam[0].w = 1.0f;




    //CBV�i�萔�o�b�t�@�X�V�j �V�F�[�_�[�Ŏ󂯎��

    memcpy(pConstData, &conBufData, sizeof(ConstantBufferData));




}



// �`�惁�\�b�h
void DirectX12App::Render() {

    OnUpdate();


    //�R�}���h�̃��Z�b�g
    commandAllocator->Reset();                              
    commandList->Reset(commandAllocator.Get(), nullptr);


    {
        //�o���A�ύX�@�i�u�\�����ėǂ��v�Ƃ�����Ԃ���u�`�悵�܂��v�Ƃ�����Ԃ�

        CD3DX12_RESOURCE_BARRIER transitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            renderTargets[currentFrameIndex].Get(),
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET);

        commandList->ResourceBarrier(1, &transitionBarrier);
    }


    // Get descriptor handle for the current render target view
    UINT rtvDescriptorSize = dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), currentFrameIndex, rtvDescriptorSize);

    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsvHeap->GetCPUDescriptorHandleForHeapStart());

    // �����_�[�^�[�Q�b�g�̃N���A
    FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // �[�x�o�b�t�@���N���A
    commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    //�����_�[�^�[�Q�b�g�i�`��̏o�͐�j
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // �r���[�|�[�g
    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, 1280.0f, 720.0f, 0.0f, 1.0f };
    commandList->RSSetViewports(1, &viewport);

    //�V�U�[��`�i�؂蔲���j
    D3D12_RECT scissorRect = { 0, 0, 1280, 720 };
    commandList->RSSetScissorRects(1, &scissorRect);



    //�`��R�}���h

    //idx = 0 �n�ʃV�F�[�_�[

    for (int idx = 0; idx < 1; idx++) {


        // ���[�g�V�O�l�`�� �i�`�惋�[��
        commandList->SetGraphicsRootSignature(rootSignature[idx].Get());


        // CBV �萔�o�b�t�@�@�i�V�F�[�_�[�ɓn���ϐ����
        commandList->SetGraphicsRootConstantBufferView(0, constantBuffer->GetGPUVirtualAddress());  //�V�F�[�_�[�ihlsl�j�́A register(b0) �ɓo�^�B�����Ɠn�������ꍇ�́Ab1, b2�ƂȂ��Ă���
        //commandList->SetGraphicsRootConstantBufferView(1, constantBuffer2->GetGPUVirtualAddress());   //���Ƃ��΁Aregister(b1)�ɕʂ̕ϐ�����n�������ꍇ�́A����Ȋ����ł�

        // SRV �V�F�[�_�[���\�[�X�r���[�@�i�V�F�[�_�[�ɓn���e�N�X�`���[���
        ID3D12DescriptorHeap* ppHeaps[] = { srvHeap.Get() };
        commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
        commandList->SetGraphicsRootDescriptorTable(1, srvHeap->GetGPUDescriptorHandleForHeapStart());

        // �p�C�v���C���X�e�[�g �i�`�惋�[��

        if (showGrid) {

            commandList->SetPipelineState(pipelineState_Grid[idx].Get());
        }
        else {

            commandList->SetPipelineState(pipelineState[idx].Get());
        }


        {

            //���_�`��

            int drawNum = 1;    //�`�搔



            //commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);   //�O�p�`�ŏ����܂�


            // ����_�p�b�`�i�O�p�`�jHS �� [outputcontrolpoints(3)] �ƈ�v������  �����������i�d�v�j
            commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);  //�e�b�Z���[�V�����̓p�b�`



            //���_�o�b�t�@
            commandList->IASetVertexBuffers(0, 1, &vertexBufferViewArray[idx]);   //���_��񃊃\�[�X�̃r���[�i���j

            //�C���f�b�N�X�o�b�t�@
            commandList->IASetIndexBuffer(&indexBufferViewArray[idx]);    //���_�C���f�b�N�X��񃊃\�[�X�̃r���[�i���j


            //�`��J�n
            commandList->DrawIndexedInstanced(
                vertexInfoArray[idx].indexBufferSize, // �`�悷�钸�_�C���f�b�N�X�̐�
                drawNum,  // �`�搔�i���̓�����������`�悷�邩�B�|���͂R�񏑂��Ă���i0 �L�����N�^�[, 1 ���u, 2 �R�C��
                0,  // �J�n�C���f�b�N�X
                0,  // �x�[�X���_
                0   // �J�n�C���X�^���X
            );


        }
    }



    

    {

        //�o���A�ύX�@�i�u�`�悵�܂��v�Ƃ�����Ԃ���u�\�����ėǂ��v�Ƃ�����Ԃ�

        CD3DX12_RESOURCE_BARRIER transitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            renderTargets[currentFrameIndex].Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT);

        commandList->ResourceBarrier(1, &transitionBarrier);
    }


    commandList->Close();   //�R�}���h�����


    {

        //�R�}���h�̎��s

        ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
        commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);


        //�O���t�B�b�N�{�[�h�̏������I���̂�ҋ@
        WaitForGPUFinish();
        

    }


    // �X���b�v�`�F�[����؂�ւ���
    swapChain->Present(1, 0);   //�������� (VSync)  0�͖����A1�͎��̐��������M���܂őҋ@,  0: �ł���ʓI�Ȓl�ŁA���ʂȃt���O���w�肵�Ȃ�

    //�\�����Ă��Ȃ����́A�����_�[�^�[�Q�b�g�ԍ����擾
    currentFrameIndex = swapChain->GetCurrentBackBufferIndex();//���݁A�`�悳��Ă��Ȃ����̃^�[�Q�b�g�ԍ�




}


void DirectX12App::WaitForGPUFinish() {


    //�O���t�B�b�N�{�[�h�̏������I���̂�ҋ@

    const UINT64 fenceToWaitFor = fenceValue;

    if (FAILED(commandQueue->Signal(fence.Get(), fenceToWaitFor))) {
        OutputDebugStringA("Failed to signal fence.\n");
        return;
    }
    fenceValue++;

    if (fence->GetCompletedValue() < fenceToWaitFor) {
        if (FAILED(fence->SetEventOnCompletion(fenceToWaitFor, fenceEvent))) {
            OutputDebugStringA("Failed to set event on completion.\n");
            return;
        }
        WaitForSingleObject(fenceEvent, INFINITE);
    }

}

void DirectX12App::OnDestroy() {


    ref_hwnd = nullptr;

    //�O���t�B�b�N�{�[�h�̏����̏I����҂��Ă���A�A�v���P�[�V�������I������

    WaitForGPUFinish();


}



