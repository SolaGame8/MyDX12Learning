#include "DirectX12App.h"

//#include <Windows.h>
#include <d3d12sdklayers.h>
#include <sstream>



// ���������\�b�h
bool DirectX12App::Initialize(HWND hwnd) {  //�E�C���h�E�̃n���h���̎󂯎��


#if defined(_DEBUG)

    // DirectX 12 �f�o�b�O���C���[��L���ɂ���    �i�f�o�b�O�r���h�ł̂ݗL��

    ID3D12Debug* debugController = nullptr;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();
    }

#endif

    //�𑜓x
    ResResolution.x = 1280;
    ResResolution.y = 720;


    //VR�w�b�h�}�E���g���g��������`�F�b�N

    auto checkSupport = OpenXRManager::CheckVRSupport();	//Static �Ȃ̂�new���Ȃ��ŌĂׂ�

    /*
    switch (VRprobe) {

    case OpenXRManager::VrSupport::Ready:
        OutputDebugStringA("[ProbeXR] Ready\n");
        // VR���[�h��I����
        break;

    case OpenXRManager::VrSupport::NoHmd:
        OutputDebugStringA("[ProbeXR] NoHmd\n");
        // VR���[�h�́uHMD��ڑ����Ă��������v�Ȃ�
        break;

    case OpenXRManager::VrSupport::RuntimeNoD3D12:
        OutputDebugStringA("[ProbeXR] RuntimeNoD3D12\n");
        // ���݁A�ڑ�����Ă���OpenXR�����^�C����D3D12�g���𖢃T�|�[�g
        break;

    case OpenXRManager::VrSupport::RuntimeUnavailable:
    case OpenXRManager::VrSupport::InstanceFailed:
    default:
        OutputDebugStringA("[ProbeXR] InstanceFailed\n");
        // �����^�C��������/���Ă���
        break;
    }
    */

    flg_useVRMode = false;

    if (checkSupport == OpenXRManager::VrSupport::Ready) {

        // VR���[�h�����p�\

        int ret = MessageBoxA(
            nullptr,
            "VR �f�o�C�X�����p�\�ł��B\nVR���[�h�ŋN�����܂����H",
            "VR Mode",
            MB_YESNO | MB_ICONQUESTION
        );

        if (ret == IDYES) {

            //VR���[�h�ŋN��
            flg_useVRMode = true;

            //�������߉𑜓x���X�P�[���_�E���i0.8�j��������
            ResResolution.x = OpenXRManager::recommendedScaledResolution.x;
            ResResolution.y = OpenXRManager::recommendedScaledResolution.y;

            //�E�C���h�E�̃T�C�Y��ς���

            int newHeight = 720;
            int newWidth = (int)floorf(ResResolution.x * newHeight / ResResolution.y);

            SetWindowPos(hwnd, nullptr, 0, 0, newWidth, newHeight, SWP_NOMOVE | SWP_NOZORDER);

        }

    }


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


        //�V�F�[�_�[�쐬   �i0 �|���p, 1 �n�ʗp
        for (size_t i = 0; i < 2; i++) {
            if (!CreatShader(i)) {
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
        for (size_t i = 0; i < 2; i++) {
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


    //OpenXR

    if (flg_useVRMode) {

        XR_Manager = new OpenXRManager();

        XR_Manager->Initialize(dx12Device.Get(), commandQueue.Get());
    }




    return true;
}





void DirectX12App::InitVariable() {


    // �V�[�h�l�ŁA�n�`�p�̃p�[�����m�C�Y��������  �i�V�[�h�l��ς���ƁA�Ⴄ�n�`�����������
    unsigned int seed = 123;
    pNoise = PerlinNoise::getInstance(seed);


    /*
    //�p�[�����m�C�Y �e�X�g�o��
    for (size_t i = 0; i < 100; i++) {
        std::stringstream ss;
        ss << "pNose : " << pNoise->noise(0.051f * i, 0.5f, 0.0f) << "\n";
        OutputDebugStringA(ss.str().c_str());
    }
    */




    //�ϐ���ݒ�
    pipelineState.resize(2);    //�`�惋�[�� �Q�i�|���A�n��
    rootSignature.resize(2);

    vertexShader.resize(2);     //�V�F�[�_�[ �Q�i�|���A�n��
    pixelShader.resize(2);
    errorBlob.resize(2);

    vertexBufferArray.resize(2);        //�`��p�@���_�f�[�^�@�Q�i�|���A�n��
    vertexBufferViewArray.resize(2);

    indexBufferArray.resize(2);         //�`��p�@���_�C���f�b�N�X�f�[�^�i�ǂ̒��_�ԍ����Ȃ����ĎO�p�`�ɂȂ��Ă��邩�̃f�[�^
    indexBufferViewArray.resize(2);

    //���u�A�R�C���̈ʒu��UV
    for (size_t i = 0; i < 2; i++) {

        XMFLOAT3 pos = XMFLOAT3(2.0f * i + 1.0f, 2.0f, 0.0f);
        int uv = 7 + i * 4;
        mobPos.emplace_back(pos);
        mobUVIdx.emplace_back(uv);
    }

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
    UINT resolutionWidth = ResResolution.x;
    UINT resolutionHeight = ResResolution.y;


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
    depthStencilDesc.Width = ResResolution.x; // �𑜓x
    depthStencilDesc.Height = ResResolution.y; 
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







bool DirectX12App::CreatShader(int idx) {

    HRESULT hr = S_OK;

    std::wstring shaderPath;

    switch (idx) {
    case 0:
        shaderPath = L"PlateShader.hlsl";
        break;
    case 1:
        shaderPath = L"FloorShader.hlsl";
        break;
    default:
        shaderPath = L"PlateShader.hlsl";
        break;
    }



    // ���_�V�F�[�_�[�̃R���p�C��    0123
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

    OutputDebugStringA("Finished Shader Compile\n");
    
    // �W�I���g��
    
    // �R���s���[�g�V�F�[�_�[

    return true;

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
    srvHeapDesc.NumDescriptors = 2; // �e�N�X�`���̖����ɍ��킹�ăT�C�Y������
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
    CD3DX12_ROOT_PARAMETER rootParameters[3];
    // �萔�o�b�t�@�r���[�iCBV�j�����[�g�p�����[�^�Ƃ��Ēǉ�
    // b0 ���W�X�^���o�C���h
    rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);  //CBV �萔�o�b�t�@

    CD3DX12_DESCRIPTOR_RANGE ranges[1];
    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0); // ������SRV
    rootParameters[1].InitAsDescriptorTable(1, ranges, D3D12_SHADER_VISIBILITY_PIXEL); // �e�N�X�`���[

    // b1: Root Constants (32bit x 1�j
    rootParameters[2].InitAsConstants( 1, 1, 0,   //num32BitValues, shaderRegister(b1), space
        D3D12_SHADER_VISIBILITY_ALL);


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
        _countof(rootParameters), // �p�����[�^��: 3
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

    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader[idx].Get());          //���_�V�F�[�_�[�i�R���p�C���ς�
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader[idx].Get());           //�s�N�Z���V�F�[�_�[�i�R���p�C���ς�

    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);       //���X�^���C�Y�̐ݒ�

    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;                //�O�p�`�̗��ʂ��`��

    psoDesc.BlendState = blendDesc;                                         //�u�����h�̐ݒ�


    
    psoDesc.DepthStencilState.DepthEnable = TRUE;                           //�[�x�o�b�t�@��L���ɂ���
    //psoDesc.DepthStencilState.DepthEnable = FALSE;                           //�[�x�o�b�t�@��L���ɂ���
    
    
    
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;  //�[�x�̏������݂�L���ɂ���
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;       //�[�x�e�X�g�̏����i�l�����������i�J�����ɋ߂����j��`��
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;                              //�[�x�o�b�t�@�̃t�H�[�}�b�g�i32bit float

    psoDesc.SampleMask = UINT_MAX;

    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; //�O�p�`��`�悷��

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

    OutputDebugStringA("Successfully initialized all components for drawing.\n");


    return true;

}








bool DirectX12App::CreateVertex() {

    //�`�悷�钸�_�����쐬

    CreatePlate();  //�|��
    CreateFloor();  //�n��

    return true;

}


void DirectX12App::CreatePlate() {

    //���_�������

    HRESULT hr = S_OK;

    int myIdx = 0;


    //vertexInfoArray

    VertexInfo vertInfo;


    {


        // ���_�f�[�^�̒�` (�l�p�`)
        Vertex vertices[] = {
            // �ʒu                                  �F                               UV
            { XMFLOAT4(-0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f) }, //0
            { XMFLOAT4(0.5f,  1.0f, 0.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 0.0f) }, //1
            { XMFLOAT4(-0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 2.0f, 0.0f) }, //2
            { XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 3.0f, 0.0f) }   //3
        };

        vertInfo.vertexBufferSize = sizeof(vertices); //4��

        // ���_�o�b�t�@�̍쐬
        CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);        //�A�b�v���[�h�iCPU�j     //�f�t�H���g�iGPU�j
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

        // �f�[�^���o�b�t�@�ɃR�s�[
        void* pVertexData;
        CD3DX12_RANGE readRange(0, 0);
        vertexBufferArray[myIdx]->Map(0, &readRange, &pVertexData);
        memcpy(pVertexData, vertices, vertInfo.vertexBufferSize);    //�������ݏꏊ, �R�s�[���钸�_�f�[�^
        vertexBufferArray[myIdx]->Unmap(0, nullptr);

        // ���_�o�b�t�@�r���[�̐ݒ�
        vertexBufferViewArray[myIdx].BufferLocation = vertexBufferArray[myIdx]->GetGPUVirtualAddress();
        vertexBufferViewArray[myIdx].StrideInBytes = sizeof(Vertex);
        vertexBufferViewArray[myIdx].SizeInBytes = vertInfo.vertexBufferSize;


    }

    //�C���f�b�N�X�o�b�t�@

    {

        WORD indices[] = {  //
            0, 1, 2, // 1�ڂ̎O�p�` (����A�E��A����)
            1, 3, 2  // 2�ڂ̎O�p�` (�E��A�E���A����)
        };


        UINT indexBufferSize = sizeof(indices);   //6
        vertInfo.indexBufferSize = 6;   //6

        // �C���f�b�N�X�o�b�t�@�̍쐬
        CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);    //�A�b�v���[�h�iCPU�j     //�f�t�H���g�iGPU�j
        CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);  //6

        hr = dx12Device->CreateCommittedResource(
            &uploadHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,  //�ǂݎ��
            nullptr,
            IID_PPV_ARGS(&indexBufferArray[myIdx]));

        if (FAILED(hr)) {
            OutputDebugStringA("Failed to create index buffer.\n");
            return;
        }

        // �f�[�^���C���f�b�N�X�o�b�t�@�ɃR�s�[
        void* pIndexData;
        CD3DX12_RANGE readRange(0, 0);

        indexBufferArray[myIdx]->Map(0, &readRange, &pIndexData);
        memcpy(pIndexData, indices, indexBufferSize);   //�������ݏꏊ, �R�s�[���钸�_�f�[�^
        indexBufferArray[myIdx]->Unmap(0, nullptr);

        // �C���f�b�N�X�o�b�t�@ �r���[�̐ݒ�
        indexBufferViewArray[myIdx].BufferLocation = indexBufferArray[myIdx]->GetGPUVirtualAddress();
        indexBufferViewArray[myIdx].Format = DXGI_FORMAT_R16_UINT; // WORD (16�r�b�g) �̂���R16_UINT  ��
        indexBufferViewArray[myIdx].SizeInBytes = indexBufferSize;

    }

    vertexInfoArray.emplace_back(vertInfo);


}






void DirectX12App::CreateFloor() {

    //��

    //���_�������

    HRESULT hr = S_OK;

    int myIdx = 1;

    VertexInfo vertInfo;

    floorHeightArray.clear();


    {
        vector<Vertex> vertexArr;

        vector<WORD> indiceArr;


        float gridSize = 1.0f;  //�P�}�X�̃T�C�Y
        int floor_length = 64;  //64 * 64 * 4   ���_
        float startX = -gridSize * floor_length * 0.5f;
        float startY = -gridSize * floor_length * 0.5f;

        int indiceCounter = 0;

        float margin = 1.0f / 16.0f / 16.0f;
        float blockSize = 1.0f / 16.0f - margin;

        const float shiftRate = 0.0673f;
        const float heightRate = 10.0f;

        float sideX, sideZ;



        for (size_t y = 0; y < floor_length; y++) { //64

            vector<float>heightArr; //�������

            for (size_t x = 0; x < floor_length; x++) { //64

                //�p�[�����m�C�Y�֐��Ɉʒu��n���āA�Ȃ߂炩�ɂȂ��闐�����擾   �i�����n�ʂ̍����Ƃ��Ďg��

                float fh = pNoise->noise(
                    shiftRate * (startX + gridSize * x),
                    shiftRate * (startY + gridSize * y), 0.0f) * heightRate;



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



                //�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�@���ʁi��j


                {
                    //0 ����
                    Vertex vert;
                    vert.Pos = XMFLOAT4(startX + gridSize * (x + 0.0f), fh, startY + gridSize * (y + 0.0f), 1.0f);
                    vert.UV = XMFLOAT4(blockSize + margin, margin, 0.0f, 0.0f);
                    vert.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

                    vertexArr.emplace_back(vert);
                }
                {
                    //1 �E��
                    Vertex vert;
                    vert.Pos = XMFLOAT4(startX + gridSize * (x + 1.0f), fh, startY + gridSize * (y + 0.0f), 1.0f);
                    vert.UV = XMFLOAT4(blockSize + blockSize, margin, 0.0f, 0.0f);
                    vert.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

                    vertexArr.emplace_back(vert);
                }
                {
                    //2 ����
                    Vertex vert;
                    vert.Pos = XMFLOAT4(startX + gridSize * (x + 0.0f), fh - gridSize, startY + gridSize * (y + 0.0f), 1.0f);
                    vert.UV = XMFLOAT4(blockSize + margin, blockSize, 0.0f, 0.0f);
                    vert.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

                    vertexArr.emplace_back(vert);
                }
                {
                    //3 �E��
                    Vertex vert;
                    vert.Pos = XMFLOAT4(startX + gridSize * (x + 1.0f), fh - gridSize, startY + gridSize * (y + 0.0f), 1.0f);
                    vert.UV = XMFLOAT4(blockSize + blockSize, blockSize, 0.0f, 0.0f);
                    vert.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

                    vertexArr.emplace_back(vert);
                }


                indiceArr.emplace_back(indiceCounter + 0);
                indiceArr.emplace_back(indiceCounter + 1);
                indiceArr.emplace_back(indiceCounter + 2);

                indiceArr.emplace_back(indiceCounter + 1);
                indiceArr.emplace_back(indiceCounter + 3);
                indiceArr.emplace_back(indiceCounter + 2);

                indiceCounter += 4;




                //�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�@���ʁi�E�j


                {
                    //0 ����
                    Vertex vert;
                    vert.Pos = XMFLOAT4(startX + gridSize * (x + 1.0f), fh, startY + gridSize * (y + 0.0f), 1.0f);
                    vert.UV = XMFLOAT4(blockSize + margin, margin, 0.0f, 0.0f);
                    vert.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

                    vertexArr.emplace_back(vert);
                }
                {
                    //1 �E��
                    Vertex vert;
                    vert.Pos = XMFLOAT4(startX + gridSize * (x + 1.0f), fh, startY + gridSize * (y + 1.0f), 1.0f);
                    vert.UV = XMFLOAT4(blockSize + blockSize, margin, 0.0f, 0.0f);
                    vert.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

                    vertexArr.emplace_back(vert);
                }
                {
                    //2 ����
                    Vertex vert;
                    vert.Pos = XMFLOAT4(startX + gridSize * (x + 1.0f), fh - gridSize, startY + gridSize * (y + 0.0f), 1.0f);
                    vert.UV = XMFLOAT4(blockSize + margin, blockSize, 0.0f, 0.0f);
                    vert.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

                    vertexArr.emplace_back(vert);
                }
                {
                    //3 �E��
                    Vertex vert;
                    vert.Pos = XMFLOAT4(startX + gridSize * (x + 1.0f), fh - gridSize, startY + gridSize * (y + 1.0f), 1.0f);
                    vert.UV = XMFLOAT4(blockSize + blockSize, blockSize, 0.0f, 0.0f);
                    vert.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

                    vertexArr.emplace_back(vert);
                }



                indiceArr.emplace_back(indiceCounter + 0);
                indiceArr.emplace_back(indiceCounter + 1);
                indiceArr.emplace_back(indiceCounter + 2);

                indiceArr.emplace_back(indiceCounter + 1);
                indiceArr.emplace_back(indiceCounter + 3);
                indiceArr.emplace_back(indiceCounter + 2);

                indiceCounter += 4;




                //�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�[�@���ʁi���j


                {
                    //0 ����
                    Vertex vert;
                    vert.Pos = XMFLOAT4(startX + gridSize * (x + 0.0f), fh, startY + gridSize * (y + 0.0f), 1.0f);
                    vert.UV = XMFLOAT4(blockSize + margin, margin, 0.0f, 0.0f);
                    vert.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

                    vertexArr.emplace_back(vert);
                }
                {
                    //1 �E��
                    Vertex vert;
                    vert.Pos = XMFLOAT4(startX + gridSize * (x + 0.0f), fh, startY + gridSize * (y + 1.0f), 1.0f);
                    vert.UV = XMFLOAT4(blockSize + blockSize, margin, 0.0f, 0.0f);
                    vert.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

                    vertexArr.emplace_back(vert);
                }
                {
                    //2 ����
                    Vertex vert;
                    vert.Pos = XMFLOAT4(startX + gridSize * (x + 0.0f), fh - gridSize, startY + gridSize * (y + 0.0f), 1.0f);
                    vert.UV = XMFLOAT4(blockSize + margin, blockSize, 0.0f, 0.0f);
                    vert.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

                    vertexArr.emplace_back(vert);
                }
                {
                    //3 �E��
                    Vertex vert;
                    vert.Pos = XMFLOAT4(startX + gridSize * (x + 0.0f), fh - gridSize, startY + gridSize * (y + 1.0f), 1.0f);
                    vert.UV = XMFLOAT4(blockSize + blockSize, blockSize, 0.0f, 0.0f);
                    vert.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

                    vertexArr.emplace_back(vert);
                }



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

            //��������z��ɓ����
            floorHeightArray.emplace_back(heightArr);

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


float DirectX12App::GetFloorHeight(float x, float y) {

    //���W�ɂ���āA���̈ʒu�̒n�ʂ̍�����Ԃ�

    float gridSize = 1.0f;  //�P�}�X�̃T�C�Y
    int floor_length = 64;  //64 * 64 * 4   ���_
    float startX = -gridSize * floor_length * 0.5f; //�n�ʂ̎n�_
    float startY = -gridSize * floor_length * 0.5f;

    float cx = x - startX;
    float cy = y - startY;

    int ix = floorf(cx / gridSize);
    int iy = floorf(cy / gridSize);

    //�����z�� �̒��̈ʒu�Ȃ�A������Ԃ�
    if (ix >= 0 && iy >= 0) {

        if (iy < floorHeightArray.size()) {
            if (ix < floorHeightArray[iy].size()) {

                /*
                std::stringstream ss;
                ss << "GetFloorHeight : " << floorHeightArray[iy][ix] << "\n";
                OutputDebugStringA(ss.str().c_str());
                */

                return floorHeightArray[iy][ix];

            }

        }


    }

    return 0.0f;    //�z��̊O�́A�f�[�^�������̂� 0.0f �̍�����Ԃ�


}






bool DirectX12App::LoadTextures() {

    //�e�N�X�`���[�̓ǂݍ���

    if (!TextureManager::GetInstance().LoadTexture(
        dx12Device.Get(),
        "Resources/tex_chara001.png",   //�e�N�X�`���[�ւ̃p�X
        "chara001"                      //map�ɕۑ����閼�O�@�i���̖��O�Ńf�[�^���Q�Ƃł���
    )) {

        OutputDebugStringA("Failed to load Texture........\n");
        return false;
    }

    if (!TextureManager::GetInstance().LoadTexture(
        dx12Device.Get(),
        "Resources/tex_map001.png",
        "map001"
    )) {

        OutputDebugStringA("Failed to load Texture........\n");
        return false;
    }


    // SRV�̃q�[�v�̃n���h��
    CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(srvHeap->GetCPUDescriptorHandleForHeapStart());
    UINT srvDescriptorSize = dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


    {
        // �e�N�X�`��1�� �r���[�i���j���쐬
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = TextureManager::GetInstance().GetTextureResource("chara001")->GetDesc().Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        dx12Device->CreateShaderResourceView(TextureManager::GetInstance().GetTextureResource("chara001").Get(), &srvDesc, srvHandle);
        srvHandle.Offset(1, srvDescriptorSize);

    }

    {
        // �e�N�X�`��2�� �r���[�i���j���쐬
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = TextureManager::GetInstance().GetTextureResource("map001")->GetDesc().Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        dx12Device->CreateShaderResourceView(TextureManager::GetInstance().GetTextureResource("map001").Get(), &srvDesc, srvHandle);
        srvHandle.Offset(1, srvDescriptorSize);

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

    conBufData.viewMat[0] = XMMatrixLookAtLH(eye, at, up); //�r���[

    float fovAngleY = XM_PIDIV4; // ����p (45�x)
    float aspectRatio = (float)ResResolution.x / (float)ResResolution.y; // ��ʉ𑜓x����A�X�y�N�g����v�Z
    float nearZ = 0.01f; // �߃N���b�v��
    float farZ = 100.0f; // ���N���b�v��

    conBufData.projMat[0] = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, nearZ, farZ); //�v���W�F�N�V����

    //���f���̉�]�p
    conBufData.worldMat[0] = XMMatrixIdentity();


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



void DirectX12App::CalcKey() {


    charaTexNum = 0;    //�L�����N�^�[�̃e�N�X�`���[���A�ЂƂ܂����ʂɌ�������Ԃɂ���

    XMFLOAT2 vecForward = { -sin(cameraRot.y * XM_PI / 180.0f) , cos(cameraRot.y * XM_PI / 180.0f) };
    XMFLOAT2 vecRight = { cos(cameraRot.y * XM_PI / 180.0f) , sin(cameraRot.y * XM_PI / 180.0f) };

    if (flg_useVRMode) {

        vecForward = XMFLOAT2(0.0f, 1.0f);
        vecRight = XMFLOAT2(1.0f, 0.0f);

    }
    else {
        vecForward = { -sin(cameraRot.y * XM_PI / 180.0f) , cos(cameraRot.y * XM_PI / 180.0f) };
        vecRight = { cos(cameraRot.y * XM_PI / 180.0f) , sin(cameraRot.y * XM_PI / 180.0f) };
    }




    // WASD�L�[�̏�Ԃ��`�F�b�N
    float moveSpeed = 10.0f * deltaTime; // �ړ����x�𒲐�


    bool onKey_jump = GetAsyncKeyState(VK_SPACE) & 0x8000;

    bool onKey_forward = GetAsyncKeyState('W') & 0x8000;
    bool onKey_back = GetAsyncKeyState('S') & 0x8000;
    bool onKey_left = GetAsyncKeyState('A') & 0x8000;
    bool onKey_right = GetAsyncKeyState('D') & 0x8000;


    // W�L�[: �O�����ֈړ� (y���W�𑝂₷)
    if (onKey_forward) {
        charaPos.x += vecForward.x * moveSpeed;
        charaPos.z += vecForward.y * moveSpeed;
    }
    // S�L�[: �������ֈړ� (y���W�����炷)
    if (onKey_back) {
        charaPos.x -= vecForward.x * moveSpeed;
        charaPos.z -= vecForward.y * moveSpeed;
    }

    // A�L�[: �������ֈړ� (x���W�����炷)
    if (onKey_left) {
        charaPos.x -= vecRight.x * moveSpeed;
        charaPos.z -= vecRight.y * moveSpeed;

        charaTexNum = 4;    //�L�����N�^�[�̃e�N�X�`���[���A�������̈ʒu��
    }
    // D�L�[: �E�����ֈړ� (x���W�𑝂₷)
    if (onKey_right) {
        charaPos.x += vecRight.x * moveSpeed;
        charaPos.z += vecRight.y * moveSpeed;

        charaTexNum = 2;    //�L�����N�^�[�̃e�N�X�`���[���A�E�����̈ʒu��
    }



    // VR
    if (flg_useVRMode) {
        if (XR_Manager->controllersReady) {

            const float deadzone = 0.05f;
            XrVector2f stick = XR_Manager->controller.GetValue_Right_Stick(deadzone);

            charaPos.x += vecForward.x * stick.y * moveSpeed;
            charaPos.z += vecForward.y * stick.y * moveSpeed;

            charaPos.x += vecRight.x * stick.x * moveSpeed;
            charaPos.z += vecRight.y * stick.x * moveSpeed;

            onKey_jump = XR_Manager->controller.OnPush_Right_A();


            if (XR_Manager->controller.OnPush_Left_X()) {
                XR_Manager->controller.ApplyHaptics(true, 0.5f, 0.5f, 0.0f); //leftHand, ����, �b��, ���g���i0.0�ŁA�����^�C���ɂ܂�����j
            }

        }
    }




    //�W�����v
    if (onKey_jump) {
        
        if (isOnGround) {
            charaJumpAcc = 0.1f;
        }

        charaTexNum = 6;    //�L�����N�^�[�̃e�N�X�`���[���A�W�����v�̈ʒu��
    }

    //�d��
    charaJumpAcc -= gravity * deltaTime;
    charaPos.y += charaJumpAcc;

    //isOnGround = (charaPos.y < 0.0f) ? true : false;


    //�L�����N�^�[�̈ʒu�ɂ���āA�n�ʂ̍������擾
    float h = GetFloorHeight(charaPos.x, charaPos.z);


    if (charaPos.y < h) {

        //�n�ʂ�艺�̏ꍇ
        isOnGround = true;
        charaPos.y = h;
        charaJumpAcc = 0.0f;
    }
    else {

        //�󒆂̏ꍇ
        isOnGround = false;
    }




    //�}�E�X���W

    POINT p;
    POINT distMousePos = { 0, 0 };

    if (GetCursorPos(&p)) {

        // p.x �� p.y �ɉ�ʏ�̃J�[�\�����W������
        // ���̍��W�͉�ʍ��オ (0, 0)


        //�O��Ƃ̍����ŁA�ړ��ʂ��v�Z
        distMousePos.x = p.x - lastMousePos.x;
        distMousePos.y = p.y - lastMousePos.y;

        lastMousePos.x = p.x;
        lastMousePos.y = p.y;


        float mouseRotRate = 80.0f * deltaTime; //�}�E�X�ړ��ɂ���ăJ��������]�����

        cameraRot.y -= distMousePos.x * mouseRotRate;
        cameraRot.x -= distMousePos.y * mouseRotRate;

        //�J������]����
        float verticalLimit = 40.0f;
        if (cameraRot.x > verticalLimit) {
            cameraRot.x = verticalLimit;
        }
        if (cameraRot.x < -verticalLimit) {
            cameraRot.x = -verticalLimit;
        }

        float horizontalLimit = 60.0f;
        if (cameraRot.y > horizontalLimit) {
            cameraRot.y = horizontalLimit;
        }
        if (cameraRot.y < -horizontalLimit) {
            cameraRot.y = -horizontalLimit;
        }


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

    if (flg_useVRMode) {
        //OpenXR ���X�V
        XR_Manager->UpdateSessionState();

    }




    //�L�[����
    CalcKey();


    //�J�����̃^�[�Q�b�g�i���_�j���L�����N�^�[�̈ʒu�ɂ���
    cameraTarget.x = charaPos.x;
    cameraTarget.y = charaPos.y;
    cameraTarget.z = charaPos.z;


    //�Ǐ]�ʒu�̌v�Z�i�^�[�Q�b�g�𒆐S�Ɉ�苗���ŉ�铮���j
    CalculateFollowPosition();

    //�J�����̌v�Z
    CalcCamera();



    //�V�F�[�_�[�ɓn���A�L�����N�^�[�̈ʒu���
    conBufData.shaderParam[0].x = cameraTarget.x;
    conBufData.shaderParam[0].y = cameraTarget.y;
    conBufData.shaderParam[0].z = cameraTarget.z;
    conBufData.shaderParam[0].w = 1.0f;


    //�e�N�X�`���[�A�j���[�V�����̓���
    charaCounter += deltaTime;

    if (charaCounter > 0.5f) {
        charaCounter = 0.0f;
        charaTexAnimNum = ((charaTexAnimNum + 1) % 2);  //0,1
    }

    int texIdx = charaTexNum;

    if (charaTexNum < 6) {   //0 ��~, 2 �E, 4 ��, 6�W�����v
        texIdx += charaTexAnimNum;
    }


    //�L�����N�^�[��UV���

    float tx = static_cast<float>(texIdx % 4);
    float ty = floorf((float)texIdx / 4.0f);

    float blockSize = 1.0f / 4.0f;
    conBufData.shaderParam[1].x = blockSize * tx; //U
    conBufData.shaderParam[1].y = blockSize * ty; //V
    conBufData.shaderParam[1].z = blockSize;//W
    conBufData.shaderParam[1].w = blockSize;//H


    //���u
    mobCounter += deltaTime;

    if (mobCounter > 0.1f) {
        mobCounter = 0.0f;
        mobTexAnimNum = ((mobTexAnimNum + 1) % 6);  //0,1
    }

    //���u���L�����N�^�[�̈ʒu��ǂ�

    float m_spd = 2.0f;

    float lx = mobTarget.x - cameraTarget.x;    //�ǂ�����
    float ly = mobTarget.y - cameraTarget.y;
    float lz = mobTarget.z - cameraTarget.z;
    float len = sqrtf(lx * lx + ly * ly + lz * lz);

    if (len > 0.2f) {   //������������ǂ��ƍs�����藈���肵�ău����̂ŁA�������l
        mobTarget.x += (mobTarget.x < cameraTarget.x) ? m_spd * deltaTime : -m_spd * deltaTime;
        mobTarget.y += (mobTarget.y < cameraTarget.y) ? m_spd * deltaTime : -m_spd * deltaTime;
        mobTarget.z += (mobTarget.z < cameraTarget.z) ? m_spd * deltaTime : -m_spd * deltaTime;
    }

    //���u�����邭��~�̓���������
    float l = 1.5f;
    mobPos[0].x = mobTarget.x + l * sin(updateCounter);
    mobPos[0].z = mobTarget.z + l * cos(updateCounter);
    mobPos[0].y = mobTarget.y + 1.0f;



    //�R�C��

    // VR�̏ꍇ�́A�E�R���g���[���[�̈ʒu
    if (flg_useVRMode) {
        if (XR_Manager->controllersReady) {

            XrPosef rightControllerPose = XR_Manager->controller.GetPose_RightController();

            XMFLOAT3  offsetPos = XMFLOAT3(0.0f, -1.0f, -5.0f);

            mobPos[1].x = rightControllerPose.position.x + offsetPos.x;
            mobPos[1].y = rightControllerPose.position.y + offsetPos.y;
            mobPos[1].z = rightControllerPose.position.z + offsetPos.z;

            /*
            {
                char buf[256];
                sprintf_s(buf,
                    "[mob] %f, %f, %f\n",
                    mobPos[0].x,
                    mobPos[0].y,
                    mobPos[0].z
                );
                OutputDebugStringA(buf);
            }
            */

        }
    }


    //�e�N�X�`���[�A�j���[�V�����̃e�[�u��
    int aniTable[] = { 0, 1, 2, 3, 2, 1 };


    //0 ���u, 1 �R�C��
    for (size_t i = 0; i < 2; i++) {

        int tgt = 2 + i * 2;

        //�ʒu
        conBufData.shaderParam[tgt + 0].x = mobPos[i].x;//�ʒu
        conBufData.shaderParam[tgt + 0].y = mobPos[i].y;
        conBufData.shaderParam[tgt + 0].z = mobPos[i].z;
        conBufData.shaderParam[tgt + 0].w = 1.0f;

        texIdx = mobUVIdx[i] + aniTable[mobTexAnimNum];

        tx = static_cast<float>(texIdx % 4);
        ty = floorf((float)texIdx / 4.0f);

        //UV
        conBufData.shaderParam[tgt + 1].x = blockSize * tx; //U
        conBufData.shaderParam[tgt + 1].y = blockSize * ty; //V
        conBufData.shaderParam[tgt + 1].z = blockSize;//W
        conBufData.shaderParam[tgt + 1].w = blockSize;//H

    }



    //CBV�i�萔�o�b�t�@�X�V�j �V�F�[�_�[�Ŏ󂯎��

    memcpy(pConstData, &conBufData, sizeof(ConstantBufferData));




}



// �`�惁�\�b�h
void DirectX12App::Render() {

    OnUpdate();

    //OutputDebugStringA("Render()\n");


    // Reset command allocator and command list
    commandAllocator->Reset();                              //�R�}���h�̃������Ǘ����Z�b�g
    commandList->Reset(commandAllocator.Get(), nullptr);


    int viewNum = 1;

    if (flg_useVRMode) {
        viewNum = XR_Manager->xr_viewCount;//2
    }


    std::vector<OpenXRManager::EyeMatrix> eyesData;
    float nearZ = 0.01f; // �߃N���b�v��
    float farZ = 100.0f; // ���N���b�v��

    XrTime predictedDisplayTime;    //�`��\�莞��

    if (flg_useVRMode) {

        XR_Manager->BeginFrame(predictedDisplayTime);                               // �t���[���J�n


    }



    for (size_t viewIdx = 0; viewIdx < viewNum; viewIdx++) {




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

        // �����_�[�^�[�Q�b�g�̃N���A�p
        FLOAT clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f }; // �Z����



        if (!flg_useVRMode) {

            //�ʏ�

            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), currentFrameIndex, rtvDescriptorSize);

            CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsvHeap->GetCPUDescriptorHandleForHeapStart());

            // �����_�[�^�[�Q�b�g���N���A
            commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

            // �[�x�o�b�t�@���N���A
            commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

            // �`��̏o�͐�
            commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);





        }
        else {

            //VR

            //VR�̗��� �̈ʒu�̃J�����s����擾
            XR_Manager->GetEyeMatrix(predictedDisplayTime, nearZ, farZ, eyesData);

            {
                //CBV�i�萔�o�b�t�@�X�V�j �V�F�[�_�[�Ŏ󂯎��ϐ�

                XMMATRIX offsetMat = XMMatrixTranslation(0.0f, 1.0f, 5.0f); //���_�i0,0,0�j����A�w�b�h�}�E���g�̎��_���I�t�Z�b�g

                for (size_t n = 0; n < 2; n++) {    //�E�ڂƍ��ځA�Q���̃J�����̕ϊ��s��

                    if (n < eyesData.size()) {
                        conBufData.viewMat[n] = offsetMat * eyesData[n].viewMat; //�r���[
                        conBufData.projMat[n] = eyesData[n].projMat; //�v���W�F�N�V����
                        conBufData.worldMat[n] = XMMatrixIdentity();
                    }

                }

                //CBV �X�V
                memcpy(pConstData, &conBufData, sizeof(ConstantBufferData));
            }


            //VR�̃X���b�v�`�F�[���̕`�����擾    �i���ڂQ���̃^�[�Q�b�g�B�����̕`����A���ꂼ��̖ڂ̈ʒu�ŕ`�悷��
            OpenXRManager::EyeDirectTarget tgt{};   //�`���
            XR_Manager->GetSwapchainDrawTarget(commandList.Get(), viewIdx, tgt);


            // �����_�[�^�[�Q�b�g���N���A
            commandList->ClearRenderTargetView(tgt.rtv, clearColor, 0, nullptr);

            // �[�x�o�b�t�@���N���A
            commandList->ClearDepthStencilView(tgt.dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);


            // �`��̏o�͐�
            commandList->OMSetRenderTargets(1, &tgt.rtv, FALSE, &tgt.dsv);
            //commandList->OMSetRenderTargets(1, &tgt.rtv, FALSE, nullptr);



        }


        // �r���[�|�[�g
        D3D12_VIEWPORT viewport = { 0.0f, 0.0f, (float)ResResolution.x, (float)ResResolution.y, 0.0f, 1.0f };
        commandList->RSSetViewports(1, &viewport);

        //�V�U�[��`�i�؂蔲���j
        D3D12_RECT scissorRect = { 0, 0, ResResolution.x, ResResolution.y };
        commandList->RSSetScissorRects(1, &scissorRect);



        //�`��R�}���h

        //idx = 0 �|���V�F�[�_�[, 1 �n�ʃV�F�[�_�[ ���g�����[���ɂȂ��Ă���

        for (int idx = 0; idx < 2; idx++) {


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
            commandList->SetPipelineState(pipelineState[idx].Get());


            if (!flg_useVRMode) {

                //�ʏ�

                // �g�p����J�����ϊ��s��̔ԍ��BRoot Constant�i���[�g�V�O�l�`�����R�}���h�ɓn������Ɏg����j

                uint32_t matIndex = 0;
                commandList->SetGraphicsRoot32BitConstants(2, 1, &matIndex, 0); //�ŏ��� 2 �́ArootParameters[2]�ɓo�^�����A�Ƃ������ƁBrootParameters[2]�ŁAb1���w�肵�Ă�


            }
            else {

                //VR

                // �g�p����J�����ϊ��s��̔ԍ��BRoot Constant�i���[�g�V�O�l�`�����R�}���h�ɓn������Ɏg����j

                uint32_t matIndex = viewIdx;
                commandList->SetGraphicsRoot32BitConstants(2, 1, &matIndex, 0); //�ŏ��� 2 �́ArootParameters[2]�ɓo�^�����A�Ƃ������ƁBrootParameters[2]�ŁAb1���w�肵�Ă�

            }



            {

                //���_�`��

                int drawNum = 1;    //�`�搔

                if (idx == 0) {

                    drawNum = 1 + mobPos.size();    //�|���`�掞�́A3���ɕς��Ă���

                }

                commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);   //�O�p�`�ŏ����܂�

                //
                commandList->IASetVertexBuffers(0, 1, &vertexBufferViewArray[idx]);   //���_��񃊃\�[�X�̃r���[�i���j

                //�C���f�b�N�X�o�b�t�@�i���\�[�X�j
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







        if (flg_useVRMode) {


            XR_Manager->FinishSwapchainDrawTarget(commandList.Get(), viewIdx);


        }


    }


    commandList->Close();   //�R�}���h�����


    {

        //�R�}���h�̎��s

        ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
        commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);


        //�O���t�B�b�N�{�[�h�̏������I���̂�ҋ@
        WaitForGPUFinish();
        

    }

    if (flg_useVRMode) {


        //�t���[���I��    �i�^�[�Q�b�g�i���ځj�ւ̕`�悪���S�ɏI����Ă��Ԃɂ��Ă���
        XR_Manager->EndFrame_WithProjection(
            eyesData,
            nearZ, farZ,
            predictedDisplayTime);

    }


    if (!flg_useVRMode) {

        //�ʏ�

        // �X���b�v�`�F�[����؂�ւ���
        swapChain->Present(1, 0);   //�������� (VSync)  0�͖����A1�͎��̐��������M���܂őҋ@,  0: �ł���ʓI�Ȓl�ŁA���ʂȃt���O���w�肵�Ȃ�

        //�\�����Ă��Ȃ����́A�����_�[�^�[�Q�b�g�ԍ����擾
        currentFrameIndex = swapChain->GetCurrentBackBufferIndex();//���݁A�`�悳��Ă��Ȃ����̃^�[�Q�b�g�ԍ�

    }






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


    if (XR_Manager) {
        XR_Manager->OnDestroy();
        delete XR_Manager;
        XR_Manager = nullptr;
    }


    //�O���t�B�b�N�{�[�h�̏����̏I����҂��Ă���A�A�v���P�[�V�������I������

    WaitForGPUFinish();


}



