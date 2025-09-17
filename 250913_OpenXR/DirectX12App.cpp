#include "DirectX12App.h"

//#include <Windows.h>
#include <d3d12sdklayers.h>
#include <sstream>



// 初期化メソッド
bool DirectX12App::Initialize(HWND hwnd) {  //ウインドウのハンドルの受け取り


#if defined(_DEBUG)

    // DirectX 12 デバッグレイヤーを有効にする    （デバッグビルドでのみ有効

    ID3D12Debug* debugController = nullptr;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();
    }

#endif

    //解像度
    ResResolution.x = 1280;
    ResResolution.y = 720;


    //VRヘッドマウントが使える環境かチェック

    auto checkSupport = OpenXRManager::CheckVRSupport();	//Static なのでnewしないで呼べる

    /*
    switch (VRprobe) {

    case OpenXRManager::VrSupport::Ready:
        OutputDebugStringA("[ProbeXR] Ready\n");
        // VRモードを選択可
        break;

    case OpenXRManager::VrSupport::NoHmd:
        OutputDebugStringA("[ProbeXR] NoHmd\n");
        // VRモードは「HMDを接続してください」など
        break;

    case OpenXRManager::VrSupport::RuntimeNoD3D12:
        OutputDebugStringA("[ProbeXR] RuntimeNoD3D12\n");
        // 現在、接続されているOpenXRランタイムがD3D12拡張を未サポート
        break;

    case OpenXRManager::VrSupport::RuntimeUnavailable:
    case OpenXRManager::VrSupport::InstanceFailed:
    default:
        OutputDebugStringA("[ProbeXR] InstanceFailed\n");
        // ランタイムが無い/壊れている
        break;
    }
    */

    flg_useVRMode = false;

    if (checkSupport == OpenXRManager::VrSupport::Ready) {

        // VRモードが利用可能

        int ret = MessageBoxA(
            nullptr,
            "VR デバイスが利用可能です。\nVRモードで起動しますか？",
            "VR Mode",
            MB_YESNO | MB_ICONQUESTION
        );

        if (ret == IDYES) {

            //VRモードで起動
            flg_useVRMode = true;

            //おすすめ解像度をスケールダウン（0.8）したもの
            ResResolution.x = OpenXRManager::recommendedScaledResolution.x;
            ResResolution.y = OpenXRManager::recommendedScaledResolution.y;

            //ウインドウのサイズを変える

            int newHeight = 720;
            int newWidth = (int)floorf(ResResolution.x * newHeight / ResResolution.y);

            SetWindowPos(hwnd, nullptr, 0, 0, newWidth, newHeight, SWP_NOMOVE | SWP_NOZORDER);

        }

    }


    //変数を設定
    InitVariable();




    {
        //ーーーーー 機能の準備 ーーーーー


        //デバイス作成（使用するグラフィックボード取得）

        if (!CreateDevice()) {

            OutputDebugStringA("Failed to create D3D12 Device.\n");
            return false;
        }

        // コマンド関連ツールの作成
        if (!CreateCommandObjects()) {
            return false;
        }

        // スワップチェーンの作成  （画面切り替え）
        if (!CreateSwapChain(hwnd)) {
            return false;
        }

        //レンダーターゲット       （スワップチェーンのリソースに描き込む用）
        if (!CreateRTV()) {
            return false;
        }

        //深度バッファ            （奥行きを比較して、手前にあるポリゴンを描画する用
        if (!CreateDepthBuffer()) {
            return false;
        }

    }


    {
        //ーーーーー 描画の準備 ーーーーー


        //シェーダー作成   （0 板ポリ用, 1 地面用
        for (size_t i = 0; i < 2; i++) {
            if (!CreatShader(i)) {
                return false;
            }
        }

        //＊今回はシェーダー用のCBVとSRVを、板ポリと地面、共用で使っているので１つのデータです

        //CBV 定数バッファビュー作成       （これに設定して、シェーダーに 変数 を渡す
        if (!CreateCBV()) {
            return false;
        }

        //SRV シェーダーリソースビュー作成  （これに設定して、シェーダーに テクスチャー を渡す
        if (!CreateSRV()) {
            return false;
        }


        //パイプラインステート作成  （描画のルール決め   （0 板ポリ用, 1 地面用
        for (size_t i = 0; i < 2; i++) {
            if (!CreatePipelineState(i)) {  //2つ
                return false;
            }
        }

    }

    {
        //ーーーーー 描画するデータの準備 ーーーーー


        //描画用の頂点情報を作成（板ポリ、地面

        if (!CreateVertex()) {
            return false;
        }

        //テクスチャーデータ読み込み
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


    // シード値で、地形用のパーリンノイズを初期化  （シード値を変えると、違う地形が生成される
    unsigned int seed = 123;
    pNoise = PerlinNoise::getInstance(seed);


    /*
    //パーリンノイズ テスト出力
    for (size_t i = 0; i < 100; i++) {
        std::stringstream ss;
        ss << "pNose : " << pNoise->noise(0.051f * i, 0.5f, 0.0f) << "\n";
        OutputDebugStringA(ss.str().c_str());
    }
    */




    //変数を設定
    pipelineState.resize(2);    //描画ルール ２つ（板ポリ、地面
    rootSignature.resize(2);

    vertexShader.resize(2);     //シェーダー ２つ（板ポリ、地面
    pixelShader.resize(2);
    errorBlob.resize(2);

    vertexBufferArray.resize(2);        //描画用　頂点データ　２つ（板ポリ、地面
    vertexBufferViewArray.resize(2);

    indexBufferArray.resize(2);         //描画用　頂点インデックスデータ（どの頂点番号がつながって三角形になっているかのデータ
    indexBufferViewArray.resize(2);

    //モブ、コインの位置とUV
    for (size_t i = 0; i < 2; i++) {

        XMFLOAT3 pos = XMFLOAT3(2.0f * i + 1.0f, 2.0f, 0.0f);
        int uv = 7 + i * 4;
        mobPos.emplace_back(pos);
        mobUVIdx.emplace_back(uv);
    }

    // 描画ループを開始する前に、最初のフレーム時間を記録
    lastFrameTime = std::chrono::steady_clock::now();

    // 現在のマウス位置を記録
    GetCursorPos(&lastMousePos);


}





bool DirectX12App::CreateDevice() {



    // DXGIファクトリー（工場）の作成
    if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory)))) {

        OutputDebugStringA("Failed to create DXGI Factory.\n");
        return false;
    }


    // デバイスの作成（グラフィックボードを取得）

    ComPtr<IDXGIAdapter1> hardwareAdapter;  //アダプター
    HRESULT hr = S_OK;
    UINT adapterIndex = 0;


    // ループでアダプターを列挙
    do {

        hr = dxgiFactory->EnumAdapters1(adapterIndex, &hardwareAdapter);    //列挙：ファクトリーからひとつずつアダプターを受け取る

        if (SUCCEEDED(hr)) {    //受け取れたら

            DXGI_ADAPTER_DESC1 desc;
            hardwareAdapter->GetDesc1(&desc);   //情報をもらう

            // アダプター名を出力
            OutputDebugStringW(L"Found Adapter: ");
            OutputDebugStringW(desc.Description);
            OutputDebugStringW(L"\n");

            // ソフトウェアアダプター（WARPデバイス）をスキップ
            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {  //関係ない
                adapterIndex++;
                continue;
            }

            // D3D12デバイスを作成できるか試す
            if (SUCCEEDED(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&dx12Device)
            ))) {

                OutputDebugStringA("Successfully created D3D12 Device.\n");
                break;

            }
            adapterIndex++;
        }

    } while (hr != DXGI_ERROR_NOT_FOUND); // アダプターがなくなるまでループ


    if (!dx12Device) {
        return false;
    }


    return true;

}





bool DirectX12App::CreateCommandObjects() {

    //コマンド関連のツールを作成



    //コマンドアロケータ　（コマンドのメモリ管理。 コマンドをグラフィックボードに保存する役
    if (FAILED(dx12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)))) {
        OutputDebugStringA("Failed to create Command Allocator.\n");
        return false;
    }

    //コマンドリスト（これにコマンドを渡すと、関連付けられたアロケータにデータが渡されて、グラフィックボードに保存される
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

    commandList->Close();   //コマンドはクローズしてから、実行 （描画ループが、まずコマンドリストをクリアするので、ここではCloseで閉じておく


    //コマンドキュー　（実行役
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    if (FAILED(dx12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)))) {
        OutputDebugStringA("Failed to create Command Queue.\n");
        return false;
    }

    //フェンス　（監視役
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
    // ウィンドウサイズを取得する場合
    RECT rect;  //四角形
    GetClientRect(hwnd, &rect); //内側？四角形情報を取得
    UINT windowWidth = rect.right - rect.left;
    UINT windowHeight = rect.bottom - rect.top;
    */

    //今回は、固定の解像度にしています
    UINT resolutionWidth = ResResolution.x;
    UINT resolutionHeight = ResResolution.y;


    //スワップチェーン作成　（画面切り替え機能）

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = resolutionWidth;      //w
    swapChainDesc.Height = resolutionHeight;    //h
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  //0-255, RGBA 8bit
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = FrameBufferCount;   //2枚。切り替えて使う
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;   //拡大縮小可能
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = 0;

    // DXGIスワップチェーンを作成（下位互換？）
    ComPtr<IDXGISwapChain1> swapChain1;
    if (FAILED(dxgiFactory->CreateSwapChainForHwnd(
        commandQueue.Get(),     //スワップチェーンがコマンドキューにアクセスできるように渡す
        hwnd,                   //ウインドウのハンドル
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1
    ))) {
        OutputDebugStringA("Failed to create Swap Chain.\n");
        return false;
    }

    // 古いDXGIメッセージを無効にする
    if (FAILED(dxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER))) {
        OutputDebugStringA("Failed to disable DXGI messages.\n");
        return false;
    }

    // IDXGISwapChain3 インターフェースを取得

    if (FAILED(swapChain1.As(&swapChain))) {    //Asで、Swapchain3に継承
        OutputDebugStringA("Failed to get IDXGISwapChain3.\n");
        return false;
    }

    currentFrameIndex = swapChain->GetCurrentBackBufferIndex();//描画していない方のリソース番号を取得

    OutputDebugStringA("Successfully created Swap Chain.\n");
    return true;
}


bool DirectX12App::CreateRTV() {


    //ヒープ
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = FrameBufferCount;          //2
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;      //レンダーターゲットビューとして使う
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

        dx12Device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvHandle);//ビュー

        rtvHandle.Offset(1, rtvDescriptorSize);
    }

    OutputDebugStringA("Successfully created RTV and RTV heap.\n");

    return true;

}





// 初期化メソッド内で呼び出す
bool DirectX12App::CreateDepthBuffer() {


    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};

    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;  //深度用
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (FAILED(dx12Device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap)))) {
        return false;
    }


    D3D12_RESOURCE_DESC depthStencilDesc = {};
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = ResResolution.x; // 解像度
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
    clearValue.DepthStencil.Depth = 1.0f; // 深度値のクリア設定
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



    // 頂点シェーダーのコンパイル    0123
    hr = D3DCompileFromFile(shaderPath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "VSMain", "vs_5_0", 
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vertexShader[idx], &errorBlob[idx]);

    if (FAILED(hr)) {
        // エラー処理
        if (errorBlob[idx]) {
            OutputDebugStringA("Vertex Shader Compile Error:\n");
            OutputDebugStringA((char*)errorBlob[idx]->GetBufferPointer());
        }
        return false;
    }

    // ピクセルシェーダーのコンパイル
    hr = D3DCompileFromFile(shaderPath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "PSMain", "ps_5_0", 
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pixelShader[idx], &errorBlob[idx]);

    if (FAILED(hr)) {
        // エラー処理
        if (errorBlob[idx]) {
            OutputDebugStringA("Pixel Shader Compile Error:\n");
            OutputDebugStringA((char*)errorBlob[idx]->GetBufferPointer());
        }
        return false;
    }

    OutputDebugStringA("Finished Shader Compile\n");
    
    // ジオメトリ
    
    // コンピュートシェーダー

    return true;

}


bool DirectX12App::CreateCBV() {


    // 定数バッファとして使うアップロードヒープリソースを作成
    CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);    //Upload(CPUとGPUがさわれる場所)    //Default(GPU専用。高速)
    
    
    // 定数バッファは256バイトの倍数にする必要がある
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

    // データのマッピング
    
    CD3DX12_RANGE readRange(0, 0); // CPUは読み込まないため空
    constantBuffer->Map(0, &readRange, &pConstData);
    
    memcpy(pConstData, &conBufData, sizeof(ConstantBufferData));

    // マップを解除しない（継続して、memcpyで更新するため
    //constantBuffer->Unmap(0, nullptr);


    return true;

}


bool DirectX12App::CreateSRV() {

    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = 2; // テクスチャの枚数に合わせてサイズを決定
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;  //cbv 変数、 SRV テクスチャー, UAV ランダムアクセス
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
    
    // ルートシグネチャの作成

    
    // ルートパラメータの定義
    CD3DX12_ROOT_PARAMETER rootParameters[3];
    // 定数バッファビュー（CBV）をルートパラメータとして追加
    // b0 レジスタをバインド
    rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);  //CBV 定数バッファ

    CD3DX12_DESCRIPTOR_RANGE ranges[1];
    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0); // 複数のSRV
    rootParameters[1].InitAsDescriptorTable(1, ranges, D3D12_SHADER_VISIBILITY_PIXEL); // テクスチャー

    // b1: Root Constants (32bit x 1）
    rootParameters[2].InitAsConstants( 1, 1, 0,   //num32BitValues, shaderRegister(b1), space
        D3D12_SHADER_VISIBILITY_ALL);


    // サンプラーの定義
    CD3DX12_STATIC_SAMPLER_DESC samplerDesc(
        0, // サンプラーレジスタ s0
        D3D12_FILTER_MIN_MAG_MIP_LINEAR,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP);


    // ルートシグネチャの作成
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(
        _countof(rootParameters), // パラメータ数: 3
        rootParameters,
        1, 
        &samplerDesc,   //サンプラー
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    );

    // シリアライズ（バイナリデータへの変換）
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

    // ルートシグネチャの作成
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





    //描画のルール決め


    //頂点レイアウト　（シェーダーに送る頂点データのフォーマット
    D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };


    //ブレンドの設定
    D3D12_BLEND_DESC blendDesc = {};

    blendDesc.AlphaToCoverageEnable = true;     //テクスチャーの透明部分を透過させる

    blendDesc.IndependentBlendEnable = FALSE;   //全てのレンダーターゲットで同じ設定を使用

    blendDesc.RenderTarget[0].BlendEnable = false;

    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;  // アルファ値自体のブレンドを制御（通常はそのまま）
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

    blendDesc.RenderTarget[0].LogicOpEnable = FALSE;    
    blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;// 書き込むカラーマスク


    // パイプラインステートオブジェクト (PSO) の作成   ＊描画のルール

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

    psoDesc.InputLayout = { inputElementDesc, _countof(inputElementDesc) }; //頂点レイアウト

    psoDesc.pRootSignature = rootSignature[idx].Get();                      //ルートシグネチャ

    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader[idx].Get());          //頂点シェーダー（コンパイル済み
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader[idx].Get());           //ピクセルシェーダー（コンパイル済み

    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);       //ラスタライズの設定

    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;                //三角形の裏面も描画

    psoDesc.BlendState = blendDesc;                                         //ブレンドの設定


    
    psoDesc.DepthStencilState.DepthEnable = TRUE;                           //深度バッファを有効にする
    //psoDesc.DepthStencilState.DepthEnable = FALSE;                           //深度バッファを有効にする
    
    
    
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;  //深度の書き込みを有効にする
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;       //深度テストの条件（値が小さい方（カメラに近い方）を描画
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;                              //深度バッファのフォーマット（32bit float

    psoDesc.SampleMask = UINT_MAX;

    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; //三角形を描画する

    psoDesc.NumRenderTargets = 1;                                           //描画するリソース数

    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;                     //描画するリソースのフォーマット（8bit 0-255, RGBA

    psoDesc.SampleDesc.Count = 1;
    psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;


    //パイプラインステート作成（グラフィックボードに描画ルールが保存される
    hr = dx12Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState[idx]));

    if (FAILED(hr)) {
        OutputDebugStringA("Failed to create Pipeline State Object.\n");
        return false;
    }

    OutputDebugStringA("Successfully initialized all components for drawing.\n");


    return true;

}








bool DirectX12App::CreateVertex() {

    //描画する頂点情報を作成

    CreatePlate();  //板ポリ
    CreateFloor();  //地面

    return true;

}


void DirectX12App::CreatePlate() {

    //頂点情報を作る

    HRESULT hr = S_OK;

    int myIdx = 0;


    //vertexInfoArray

    VertexInfo vertInfo;


    {


        // 頂点データの定義 (四角形)
        Vertex vertices[] = {
            // 位置                                  色                               UV
            { XMFLOAT4(-0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f) }, //0
            { XMFLOAT4(0.5f,  1.0f, 0.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 0.0f) }, //1
            { XMFLOAT4(-0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 2.0f, 0.0f) }, //2
            { XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 3.0f, 0.0f) }   //3
        };

        vertInfo.vertexBufferSize = sizeof(vertices); //4つ

        // 頂点バッファの作成
        CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);        //アップロード（CPU）     //デフォルト（GPU）
        CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertInfo.vertexBufferSize);

        hr = dx12Device->CreateCommittedResource(
            &uploadHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,  //読み取り
            nullptr,
            IID_PPV_ARGS(&vertexBufferArray[myIdx]));

        if (FAILED(hr)) {
            OutputDebugStringA("Failed to create vertex buffer.\n");
            return;
        }

        // データをバッファにコピー
        void* pVertexData;
        CD3DX12_RANGE readRange(0, 0);
        vertexBufferArray[myIdx]->Map(0, &readRange, &pVertexData);
        memcpy(pVertexData, vertices, vertInfo.vertexBufferSize);    //書き込み場所, コピーする頂点データ
        vertexBufferArray[myIdx]->Unmap(0, nullptr);

        // 頂点バッファビューの設定
        vertexBufferViewArray[myIdx].BufferLocation = vertexBufferArray[myIdx]->GetGPUVirtualAddress();
        vertexBufferViewArray[myIdx].StrideInBytes = sizeof(Vertex);
        vertexBufferViewArray[myIdx].SizeInBytes = vertInfo.vertexBufferSize;


    }

    //インデックスバッファ

    {

        WORD indices[] = {  //
            0, 1, 2, // 1つ目の三角形 (左上、右上、左下)
            1, 3, 2  // 2つ目の三角形 (右上、右下、左下)
        };


        UINT indexBufferSize = sizeof(indices);   //6
        vertInfo.indexBufferSize = 6;   //6

        // インデックスバッファの作成
        CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);    //アップロード（CPU）     //デフォルト（GPU）
        CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);  //6

        hr = dx12Device->CreateCommittedResource(
            &uploadHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,  //読み取り
            nullptr,
            IID_PPV_ARGS(&indexBufferArray[myIdx]));

        if (FAILED(hr)) {
            OutputDebugStringA("Failed to create index buffer.\n");
            return;
        }

        // データをインデックスバッファにコピー
        void* pIndexData;
        CD3DX12_RANGE readRange(0, 0);

        indexBufferArray[myIdx]->Map(0, &readRange, &pIndexData);
        memcpy(pIndexData, indices, indexBufferSize);   //書き込み場所, コピーする頂点データ
        indexBufferArray[myIdx]->Unmap(0, nullptr);

        // インデックスバッファ ビューの設定
        indexBufferViewArray[myIdx].BufferLocation = indexBufferArray[myIdx]->GetGPUVirtualAddress();
        indexBufferViewArray[myIdx].Format = DXGI_FORMAT_R16_UINT; // WORD (16ビット) のためR16_UINT  正
        indexBufferViewArray[myIdx].SizeInBytes = indexBufferSize;

    }

    vertexInfoArray.emplace_back(vertInfo);


}






void DirectX12App::CreateFloor() {

    //床

    //頂点情報を作る

    HRESULT hr = S_OK;

    int myIdx = 1;

    VertexInfo vertInfo;

    floorHeightArray.clear();


    {
        vector<Vertex> vertexArr;

        vector<WORD> indiceArr;


        float gridSize = 1.0f;  //１マスのサイズ
        int floor_length = 64;  //64 * 64 * 4   頂点
        float startX = -gridSize * floor_length * 0.5f;
        float startY = -gridSize * floor_length * 0.5f;

        int indiceCounter = 0;

        float margin = 1.0f / 16.0f / 16.0f;
        float blockSize = 1.0f / 16.0f - margin;

        const float shiftRate = 0.0673f;
        const float heightRate = 10.0f;

        float sideX, sideZ;



        for (size_t y = 0; y < floor_length; y++) { //64

            vector<float>heightArr; //高さ情報

            for (size_t x = 0; x < floor_length; x++) { //64

                //パーリンノイズ関数に位置を渡して、なめらかにつながる乱数を取得   （これを地面の高さとして使う

                float fh = pNoise->noise(
                    shiftRate * (startX + gridSize * x),
                    shiftRate * (startY + gridSize * y), 0.0f) * heightRate;



                //ーーーーーーーーーーーーーーーーーーーーーーーーーーーー　上面

                //頂点情報

                {
                    //0 左上
                    Vertex vert;
                    vert.Pos = XMFLOAT4(startX + gridSize * x, fh, startY + gridSize * y, 1.0f);
                    vert.UV = XMFLOAT4(margin, margin, 0.0f, 0.0f);
                    vert.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

                    vertexArr.emplace_back(vert);
                }
                {
                    //1 右上
                    Vertex vert;
                    vert.UV = XMFLOAT4(blockSize, margin, 0.0f, 0.0f);
                    vert.Pos = XMFLOAT4(startX + gridSize * (x + 1.0f), fh, startY + gridSize * (y + 0.0f), 1.0f);
                    vert.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

                    vertexArr.emplace_back(vert);
                }
                {
                    //2 左下
                    Vertex vert;
                    vert.UV = XMFLOAT4(margin, blockSize, 0.0f, 0.0f);
                    vert.Pos = XMFLOAT4(startX + gridSize * (x + 0.0f), fh, startY + gridSize * (y + 1.0f), 1.0f);
                    vert.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

                    vertexArr.emplace_back(vert);
                }
                {
                    //3 右下
                    Vertex vert;
                    vert.UV = XMFLOAT4(blockSize, blockSize, 0.0f, 0.0f);
                    vert.Pos = XMFLOAT4(startX + gridSize * (x + 1.0f), fh, startY + gridSize * (y + 1.0f), 1.0f);
                    vert.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

                    vertexArr.emplace_back(vert);
                }

                //頂点インデックス情報（どの頂点がつながって三角形になるか）

                indiceArr.emplace_back(indiceCounter + 0);
                indiceArr.emplace_back(indiceCounter + 1);
                indiceArr.emplace_back(indiceCounter + 2);

                indiceArr.emplace_back(indiceCounter + 1);
                indiceArr.emplace_back(indiceCounter + 3);
                indiceArr.emplace_back(indiceCounter + 2);

                indiceCounter += 4;



                //ーーーーーーーーーーーーーーーーーーーーーーーーーーーー　側面（上）


                {
                    //0 左上
                    Vertex vert;
                    vert.Pos = XMFLOAT4(startX + gridSize * (x + 0.0f), fh, startY + gridSize * (y + 0.0f), 1.0f);
                    vert.UV = XMFLOAT4(blockSize + margin, margin, 0.0f, 0.0f);
                    vert.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

                    vertexArr.emplace_back(vert);
                }
                {
                    //1 右上
                    Vertex vert;
                    vert.Pos = XMFLOAT4(startX + gridSize * (x + 1.0f), fh, startY + gridSize * (y + 0.0f), 1.0f);
                    vert.UV = XMFLOAT4(blockSize + blockSize, margin, 0.0f, 0.0f);
                    vert.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

                    vertexArr.emplace_back(vert);
                }
                {
                    //2 左下
                    Vertex vert;
                    vert.Pos = XMFLOAT4(startX + gridSize * (x + 0.0f), fh - gridSize, startY + gridSize * (y + 0.0f), 1.0f);
                    vert.UV = XMFLOAT4(blockSize + margin, blockSize, 0.0f, 0.0f);
                    vert.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

                    vertexArr.emplace_back(vert);
                }
                {
                    //3 右下
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




                //ーーーーーーーーーーーーーーーーーーーーーーーーーーーー　側面（右）


                {
                    //0 左上
                    Vertex vert;
                    vert.Pos = XMFLOAT4(startX + gridSize * (x + 1.0f), fh, startY + gridSize * (y + 0.0f), 1.0f);
                    vert.UV = XMFLOAT4(blockSize + margin, margin, 0.0f, 0.0f);
                    vert.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

                    vertexArr.emplace_back(vert);
                }
                {
                    //1 右上
                    Vertex vert;
                    vert.Pos = XMFLOAT4(startX + gridSize * (x + 1.0f), fh, startY + gridSize * (y + 1.0f), 1.0f);
                    vert.UV = XMFLOAT4(blockSize + blockSize, margin, 0.0f, 0.0f);
                    vert.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

                    vertexArr.emplace_back(vert);
                }
                {
                    //2 左下
                    Vertex vert;
                    vert.Pos = XMFLOAT4(startX + gridSize * (x + 1.0f), fh - gridSize, startY + gridSize * (y + 0.0f), 1.0f);
                    vert.UV = XMFLOAT4(blockSize + margin, blockSize, 0.0f, 0.0f);
                    vert.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

                    vertexArr.emplace_back(vert);
                }
                {
                    //3 右下
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




                //ーーーーーーーーーーーーーーーーーーーーーーーーーーーー　側面（左）


                {
                    //0 左上
                    Vertex vert;
                    vert.Pos = XMFLOAT4(startX + gridSize * (x + 0.0f), fh, startY + gridSize * (y + 0.0f), 1.0f);
                    vert.UV = XMFLOAT4(blockSize + margin, margin, 0.0f, 0.0f);
                    vert.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

                    vertexArr.emplace_back(vert);
                }
                {
                    //1 右上
                    Vertex vert;
                    vert.Pos = XMFLOAT4(startX + gridSize * (x + 0.0f), fh, startY + gridSize * (y + 1.0f), 1.0f);
                    vert.UV = XMFLOAT4(blockSize + blockSize, margin, 0.0f, 0.0f);
                    vert.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

                    vertexArr.emplace_back(vert);
                }
                {
                    //2 左下
                    Vertex vert;
                    vert.Pos = XMFLOAT4(startX + gridSize * (x + 0.0f), fh - gridSize, startY + gridSize * (y + 0.0f), 1.0f);
                    vert.UV = XMFLOAT4(blockSize + margin, blockSize, 0.0f, 0.0f);
                    vert.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

                    vertexArr.emplace_back(vert);
                }
                {
                    //3 右下
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





                //高さ情報を配列に入れる
                heightArr.emplace_back(fh);

            }

            //高さ情報を配列に入れる
            floorHeightArray.emplace_back(heightArr);

        }




        vertInfo.vertexBufferSize = sizeof(Vertex) * vertexArr.size(); //64 * 64 * 4   頂点サイズ


        // 頂点バッファ（リソース）の作成
        CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);    //アップロード（CPU）     //デフォルト（GPU）
        CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertInfo.vertexBufferSize);

        hr = dx12Device->CreateCommittedResource(
            &uploadHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,  //読み取り
            nullptr,
            IID_PPV_ARGS(&vertexBufferArray[myIdx]));

        if (FAILED(hr)) {
            OutputDebugStringA("Failed to create vertex buffer.\n");
            return;
        }

        // データをバッファ（リソース）にコピー
        void* pVertexData;
        CD3DX12_RANGE readRange(0, 0);
        vertexBufferArray[myIdx]->Map(0, &readRange, &pVertexData);
        memcpy(pVertexData, vertexArr.data(), vertInfo.vertexBufferSize);    //書き込み場所, コピーする頂点データ
        vertexBufferArray[myIdx]->Unmap(0, nullptr);

        // 頂点バッファの ビュー（リソースの情報）を作成
        vertexBufferViewArray[myIdx].BufferLocation = vertexBufferArray[myIdx]->GetGPUVirtualAddress();
        vertexBufferViewArray[myIdx].StrideInBytes = sizeof(Vertex);
        vertexBufferViewArray[myIdx].SizeInBytes = vertInfo.vertexBufferSize;


        //インデックスバッファ

        UINT indexBufferSize = sizeof(WORD) * indiceArr.size();    //6
        vertInfo.indexBufferSize = indiceArr.size();    //64 * 64 * 6 

        // インデックスバッファ（リソース）の作成
        CD3DX12_HEAP_PROPERTIES uploadHeapProps2(D3D12_HEAP_TYPE_UPLOAD);    //アップロード（CPU）     //デフォルト（GPU）
        CD3DX12_RESOURCE_DESC bufferDesc2 = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);  //6

        hr = dx12Device->CreateCommittedResource(
            &uploadHeapProps2,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc2,
            D3D12_RESOURCE_STATE_GENERIC_READ,  //読み取り
            nullptr,
            IID_PPV_ARGS(&indexBufferArray[myIdx]));

        if (FAILED(hr)) {
            OutputDebugStringA("Failed to create index buffer.\n");
            return;
        }

        // データをインデックスバッファ（リソース）にコピー
        void* pIndexData;
        //CD3DX12_RANGE readRange(0, 0);

        indexBufferArray[myIdx]->Map(0, &readRange, &pIndexData);
        memcpy(pIndexData, indiceArr.data(), indexBufferSize);   //書き込み場所, コピーする頂点データ
        indexBufferArray[myIdx]->Unmap(0, nullptr);

        // インデックスバッファの ビュー（リソースの情報）を作成
        indexBufferViewArray[myIdx].BufferLocation = indexBufferArray[myIdx]->GetGPUVirtualAddress();
        indexBufferViewArray[myIdx].Format = DXGI_FORMAT_R16_UINT; // WORD (16ビット) のためR16_UINT  正
        indexBufferViewArray[myIdx].SizeInBytes = indexBufferSize;

    }

    //頂点数の情報を配列に入れる
    vertexInfoArray.emplace_back(vertInfo);


}


float DirectX12App::GetFloorHeight(float x, float y) {

    //座標によって、その位置の地面の高さを返す

    float gridSize = 1.0f;  //１マスのサイズ
    int floor_length = 64;  //64 * 64 * 4   頂点
    float startX = -gridSize * floor_length * 0.5f; //地面の始点
    float startY = -gridSize * floor_length * 0.5f;

    float cx = x - startX;
    float cy = y - startY;

    int ix = floorf(cx / gridSize);
    int iy = floorf(cy / gridSize);

    //高さ配列 の中の位置なら、高さを返す
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

    return 0.0f;    //配列の外は、データが無いので 0.0f の高さを返す


}






bool DirectX12App::LoadTextures() {

    //テクスチャーの読み込み

    if (!TextureManager::GetInstance().LoadTexture(
        dx12Device.Get(),
        "Resources/tex_chara001.png",   //テクスチャーへのパス
        "chara001"                      //mapに保存する名前　（この名前でデータを参照できる
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


    // SRVのヒープのハンドル
    CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(srvHeap->GetCPUDescriptorHandleForHeapStart());
    UINT srvDescriptorSize = dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


    {
        // テクスチャ1の ビュー（情報）を作成
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = TextureManager::GetInstance().GetTextureResource("chara001")->GetDesc().Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        dx12Device->CreateShaderResourceView(TextureManager::GetInstance().GetTextureResource("chara001").Get(), &srvDesc, srvHandle);
        srvHandle.Offset(1, srvDescriptorSize);

    }

    {
        // テクスチャ2の ビュー（情報）を作成
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

    //カメラの変換行列　（シェーダーに渡す変数を更新

    XMFLOAT3 cameraUp = { 0.0f, 1.0f, 0.0f };      // カメラの上方向ベクトル

    XMFLOAT3 cameraCurrentTarget;
    cameraCurrentTarget.x = cameraTarget.x;
    cameraCurrentTarget.y = cameraTarget.y + distanceFromTarget_adjust;
    cameraCurrentTarget.z = cameraTarget.z;

    XMVECTOR eye = XMLoadFloat3(&cameraPos);
    XMVECTOR at = XMLoadFloat3(&cameraCurrentTarget);
    XMVECTOR up = XMLoadFloat3(&cameraUp);

    conBufData.viewMat[0] = XMMatrixLookAtLH(eye, at, up); //ビュー

    float fovAngleY = XM_PIDIV4; // 視野角 (45度)
    float aspectRatio = (float)ResResolution.x / (float)ResResolution.y; // 画面解像度からアスペクト比を計算
    float nearZ = 0.01f; // 近クリップ面
    float farZ = 100.0f; // 遠クリップ面

    conBufData.projMat[0] = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, nearZ, farZ); //プロジェクション

    //モデルの回転用
    conBufData.worldMat[0] = XMMatrixIdentity();


}


void DirectX12App::CalculateFollowPosition() {


    //カメラ追従位置の計算    （ターゲットを中心に一定距離でまわる

    float len = distanceFromTarget;

    float cx = -(float)sin(-XM_PI * cameraRot.y / 180.0f) * len * cos(XM_PI * cameraRot.x / 180.0f);//回転 （右+ 左-）
    float cy = -(float)cos(-XM_PI * cameraRot.y / 180.0f) * len * cos(XM_PI * cameraRot.x / 180.0f);
    float ch = (float)sin(-XM_PI * cameraRot.x / 180.0f) * len;//高さ （上+ 下-）

    cameraPos.x = cameraTarget.x + cx;
    cameraPos.y = (cameraTarget.y + distanceFromTarget_adjust) + ch;
    cameraPos.z = cameraTarget.z + cy;


}



void DirectX12App::CalcKey() {


    charaTexNum = 0;    //キャラクターのテクスチャーを、ひとまず正面に向いた状態にする

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




    // WASDキーの状態をチェック
    float moveSpeed = 10.0f * deltaTime; // 移動速度を調整


    bool onKey_jump = GetAsyncKeyState(VK_SPACE) & 0x8000;

    bool onKey_forward = GetAsyncKeyState('W') & 0x8000;
    bool onKey_back = GetAsyncKeyState('S') & 0x8000;
    bool onKey_left = GetAsyncKeyState('A') & 0x8000;
    bool onKey_right = GetAsyncKeyState('D') & 0x8000;


    // Wキー: 前方向へ移動 (y座標を増やす)
    if (onKey_forward) {
        charaPos.x += vecForward.x * moveSpeed;
        charaPos.z += vecForward.y * moveSpeed;
    }
    // Sキー: 後ろ方向へ移動 (y座標を減らす)
    if (onKey_back) {
        charaPos.x -= vecForward.x * moveSpeed;
        charaPos.z -= vecForward.y * moveSpeed;
    }

    // Aキー: 左方向へ移動 (x座標を減らす)
    if (onKey_left) {
        charaPos.x -= vecRight.x * moveSpeed;
        charaPos.z -= vecRight.y * moveSpeed;

        charaTexNum = 4;    //キャラクターのテクスチャーを、左向きの位置に
    }
    // Dキー: 右方向へ移動 (x座標を増やす)
    if (onKey_right) {
        charaPos.x += vecRight.x * moveSpeed;
        charaPos.z += vecRight.y * moveSpeed;

        charaTexNum = 2;    //キャラクターのテクスチャーを、右向きの位置に
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
                XR_Manager->controller.ApplyHaptics(true, 0.5f, 0.5f, 0.0f); //leftHand, 強さ, 秒数, 周波数（0.0で、ランタイムにまかせる）
            }

        }
    }




    //ジャンプ
    if (onKey_jump) {
        
        if (isOnGround) {
            charaJumpAcc = 0.1f;
        }

        charaTexNum = 6;    //キャラクターのテクスチャーを、ジャンプの位置に
    }

    //重力
    charaJumpAcc -= gravity * deltaTime;
    charaPos.y += charaJumpAcc;

    //isOnGround = (charaPos.y < 0.0f) ? true : false;


    //キャラクターの位置によって、地面の高さを取得
    float h = GetFloorHeight(charaPos.x, charaPos.z);


    if (charaPos.y < h) {

        //地面より下の場合
        isOnGround = true;
        charaPos.y = h;
        charaJumpAcc = 0.0f;
    }
    else {

        //空中の場合
        isOnGround = false;
    }




    //マウス座標

    POINT p;
    POINT distMousePos = { 0, 0 };

    if (GetCursorPos(&p)) {

        // p.x と p.y に画面上のカーソル座標が入る
        // この座標は画面左上が (0, 0)


        //前回との差分で、移動量を計算
        distMousePos.x = p.x - lastMousePos.x;
        distMousePos.y = p.y - lastMousePos.y;

        lastMousePos.x = p.x;
        lastMousePos.y = p.y;


        float mouseRotRate = 80.0f * deltaTime; //マウス移動によってカメラが回転する量

        cameraRot.y -= distMousePos.x * mouseRotRate;
        cameraRot.x -= distMousePos.y * mouseRotRate;

        //カメラ回転制限
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

    // 現在の時刻を取得
    std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();

    // 最後に呼ばれてからの経過時間を計算
    std::chrono::duration<float> deltaTime = currentTime - lastFrameTime;

    // 次のフレームのために現在の時刻を保存
    lastFrameTime = currentTime;

    // 秒単位のデルタタイムを返す
    return deltaTime.count();
}



void DirectX12App::OnUpdate() {


    deltaTime = GetDeltaTime();

    updateCounter += deltaTime;

    if (flg_useVRMode) {
        //OpenXR 情報更新
        XR_Manager->UpdateSessionState();

    }




    //キー操作
    CalcKey();


    //カメラのターゲット（視点）をキャラクターの位置にする
    cameraTarget.x = charaPos.x;
    cameraTarget.y = charaPos.y;
    cameraTarget.z = charaPos.z;


    //追従位置の計算（ターゲットを中心に一定距離で回る動き）
    CalculateFollowPosition();

    //カメラの計算
    CalcCamera();



    //シェーダーに渡す、キャラクターの位置情報
    conBufData.shaderParam[0].x = cameraTarget.x;
    conBufData.shaderParam[0].y = cameraTarget.y;
    conBufData.shaderParam[0].z = cameraTarget.z;
    conBufData.shaderParam[0].w = 1.0f;


    //テクスチャーアニメーションの動き
    charaCounter += deltaTime;

    if (charaCounter > 0.5f) {
        charaCounter = 0.0f;
        charaTexAnimNum = ((charaTexAnimNum + 1) % 2);  //0,1
    }

    int texIdx = charaTexNum;

    if (charaTexNum < 6) {   //0 停止, 2 右, 4 左, 6ジャンプ
        texIdx += charaTexAnimNum;
    }


    //キャラクターのUV情報

    float tx = static_cast<float>(texIdx % 4);
    float ty = floorf((float)texIdx / 4.0f);

    float blockSize = 1.0f / 4.0f;
    conBufData.shaderParam[1].x = blockSize * tx; //U
    conBufData.shaderParam[1].y = blockSize * ty; //V
    conBufData.shaderParam[1].z = blockSize;//W
    conBufData.shaderParam[1].w = blockSize;//H


    //モブ
    mobCounter += deltaTime;

    if (mobCounter > 0.1f) {
        mobCounter = 0.0f;
        mobTexAnimNum = ((mobTexAnimNum + 1) % 6);  //0,1
    }

    //モブがキャラクターの位置を追う

    float m_spd = 2.0f;

    float lx = mobTarget.x - cameraTarget.x;    //追う距離
    float ly = mobTarget.y - cameraTarget.y;
    float lz = mobTarget.z - cameraTarget.z;
    float len = sqrtf(lx * lx + ly * ly + lz * lz);

    if (len > 0.2f) {   //小さい距離を追うと行ったり来たりしてブレるので、しきい値
        mobTarget.x += (mobTarget.x < cameraTarget.x) ? m_spd * deltaTime : -m_spd * deltaTime;
        mobTarget.y += (mobTarget.y < cameraTarget.y) ? m_spd * deltaTime : -m_spd * deltaTime;
        mobTarget.z += (mobTarget.z < cameraTarget.z) ? m_spd * deltaTime : -m_spd * deltaTime;
    }

    //モブがくるくる円の動きをする
    float l = 1.5f;
    mobPos[0].x = mobTarget.x + l * sin(updateCounter);
    mobPos[0].z = mobTarget.z + l * cos(updateCounter);
    mobPos[0].y = mobTarget.y + 1.0f;



    //コイン

    // VRの場合は、右コントローラーの位置
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


    //テクスチャーアニメーションのテーブル
    int aniTable[] = { 0, 1, 2, 3, 2, 1 };


    //0 モブ, 1 コイン
    for (size_t i = 0; i < 2; i++) {

        int tgt = 2 + i * 2;

        //位置
        conBufData.shaderParam[tgt + 0].x = mobPos[i].x;//位置
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



    //CBV（定数バッファ更新） シェーダーで受け取る

    memcpy(pConstData, &conBufData, sizeof(ConstantBufferData));




}



// 描画メソッド
void DirectX12App::Render() {

    OnUpdate();

    //OutputDebugStringA("Render()\n");


    // Reset command allocator and command list
    commandAllocator->Reset();                              //コマンドのメモリ管理リセット
    commandList->Reset(commandAllocator.Get(), nullptr);


    int viewNum = 1;

    if (flg_useVRMode) {
        viewNum = XR_Manager->xr_viewCount;//2
    }


    std::vector<OpenXRManager::EyeMatrix> eyesData;
    float nearZ = 0.01f; // 近クリップ面
    float farZ = 100.0f; // 遠クリップ面

    XrTime predictedDisplayTime;    //描画予定時間

    if (flg_useVRMode) {

        XR_Manager->BeginFrame(predictedDisplayTime);                               // フレーム開始


    }



    for (size_t viewIdx = 0; viewIdx < viewNum; viewIdx++) {




        {
            //バリア変更　（「表示して良い」という状態から「描画します」という状態へ

            CD3DX12_RESOURCE_BARRIER transitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
                renderTargets[currentFrameIndex].Get(),
                D3D12_RESOURCE_STATE_PRESENT,
                D3D12_RESOURCE_STATE_RENDER_TARGET);

            commandList->ResourceBarrier(1, &transitionBarrier);
        }


        // Get descriptor handle for the current render target view
        UINT rtvDescriptorSize = dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // レンダーターゲットのクリア用
        FLOAT clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f }; // 濃い青



        if (!flg_useVRMode) {

            //通常

            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), currentFrameIndex, rtvDescriptorSize);

            CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsvHeap->GetCPUDescriptorHandleForHeapStart());

            // レンダーターゲットをクリア
            commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

            // 深度バッファをクリア
            commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

            // 描画の出力先
            commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);





        }
        else {

            //VR

            //VRの両目 の位置のカメラ行列を取得
            XR_Manager->GetEyeMatrix(predictedDisplayTime, nearZ, farZ, eyesData);

            {
                //CBV（定数バッファ更新） シェーダーで受け取る変数

                XMMATRIX offsetMat = XMMatrixTranslation(0.0f, 1.0f, 5.0f); //原点（0,0,0）から、ヘッドマウントの視点をオフセット

                for (size_t n = 0; n < 2; n++) {    //右目と左目、２つ分のカメラの変換行列

                    if (n < eyesData.size()) {
                        conBufData.viewMat[n] = offsetMat * eyesData[n].viewMat; //ビュー
                        conBufData.projMat[n] = eyesData[n].projMat; //プロジェクション
                        conBufData.worldMat[n] = XMMatrixIdentity();
                    }

                }

                //CBV 更新
                memcpy(pConstData, &conBufData, sizeof(ConstantBufferData));
            }


            //VRのスワップチェーンの描画先を取得    （両目２つ分のターゲット。両方の描画を、それぞれの目の位置で描画する
            OpenXRManager::EyeDirectTarget tgt{};   //描画先
            XR_Manager->GetSwapchainDrawTarget(commandList.Get(), viewIdx, tgt);


            // レンダーターゲットをクリア
            commandList->ClearRenderTargetView(tgt.rtv, clearColor, 0, nullptr);

            // 深度バッファをクリア
            commandList->ClearDepthStencilView(tgt.dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);


            // 描画の出力先
            commandList->OMSetRenderTargets(1, &tgt.rtv, FALSE, &tgt.dsv);
            //commandList->OMSetRenderTargets(1, &tgt.rtv, FALSE, nullptr);



        }


        // ビューポート
        D3D12_VIEWPORT viewport = { 0.0f, 0.0f, (float)ResResolution.x, (float)ResResolution.y, 0.0f, 1.0f };
        commandList->RSSetViewports(1, &viewport);

        //シザー矩形（切り抜き）
        D3D12_RECT scissorRect = { 0, 0, ResResolution.x, ResResolution.y };
        commandList->RSSetScissorRects(1, &scissorRect);



        //描画コマンド

        //idx = 0 板ポリシェーダー, 1 地面シェーダー を使うルールになっている

        for (int idx = 0; idx < 2; idx++) {


            // ルートシグネチャ （描画ルール
            commandList->SetGraphicsRootSignature(rootSignature[idx].Get());


            // CBV 定数バッファ　（シェーダーに渡す変数情報
            commandList->SetGraphicsRootConstantBufferView(0, constantBuffer->GetGPUVirtualAddress());  //シェーダー（hlsl）の、 register(b0) に登録。もっと渡したい場合は、b1, b2となっていく
            //commandList->SetGraphicsRootConstantBufferView(1, constantBuffer2->GetGPUVirtualAddress());   //たとえば、register(b1)に別の変数情報を渡したい場合は、こんな感じです

            // SRV シェーダーリソースビュー　（シェーダーに渡すテクスチャー情報
            ID3D12DescriptorHeap* ppHeaps[] = { srvHeap.Get() };
            commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
            commandList->SetGraphicsRootDescriptorTable(1, srvHeap->GetGPUDescriptorHandleForHeapStart());

            // パイプラインステート （描画ルール
            commandList->SetPipelineState(pipelineState[idx].Get());


            if (!flg_useVRMode) {

                //通常

                // 使用するカメラ変換行列の番号。Root Constant（ルートシグネチャをコマンドに渡した後に使える）

                uint32_t matIndex = 0;
                commandList->SetGraphicsRoot32BitConstants(2, 1, &matIndex, 0); //最初の 2 は、rootParameters[2]に登録した、ということ。rootParameters[2]で、b1を指定してる


            }
            else {

                //VR

                // 使用するカメラ変換行列の番号。Root Constant（ルートシグネチャをコマンドに渡した後に使える）

                uint32_t matIndex = viewIdx;
                commandList->SetGraphicsRoot32BitConstants(2, 1, &matIndex, 0); //最初の 2 は、rootParameters[2]に登録した、ということ。rootParameters[2]で、b1を指定してる

            }



            {

                //頂点描画

                int drawNum = 1;    //描画数

                if (idx == 0) {

                    drawNum = 1 + mobPos.size();    //板ポリ描画時は、3枚に変えている

                }

                commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);   //三角形で書きます

                //
                commandList->IASetVertexBuffers(0, 1, &vertexBufferViewArray[idx]);   //頂点情報リソースのビュー（情報）

                //インデックスバッファ（リソース）
                commandList->IASetIndexBuffer(&indexBufferViewArray[idx]);    //頂点インデックス情報リソースのビュー（情報）


                //描画開始
                commandList->DrawIndexedInstanced(
                    vertexInfoArray[idx].indexBufferSize, // 描画する頂点インデックスの数
                    drawNum,  // 描画数（その同じ情報を何回描画するか。板ポリは３回書いている（0 キャラクター, 1 モブ, 2 コイン
                    0,  // 開始インデックス
                    0,  // ベース頂点
                    0   // 開始インスタンス
                );


            }
        }





        {

            //バリア変更　（「描画します」という状態から「表示して良い」という状態へ

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


    commandList->Close();   //コマンドを閉じる


    {

        //コマンドの実行

        ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
        commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);


        //グラフィックボードの処理が終わるのを待機
        WaitForGPUFinish();
        

    }

    if (flg_useVRMode) {


        //フレーム終了    （ターゲット（両目）への描画が完全に終わってる状態にしてから
        XR_Manager->EndFrame_WithProjection(
            eyesData,
            nearZ, farZ,
            predictedDisplayTime);

    }


    if (!flg_useVRMode) {

        //通常

        // スワップチェーンを切り替える
        swapChain->Present(1, 0);   //垂直同期 (VSync)  0は無効、1は次の垂直同期信号まで待機,  0: 最も一般的な値で、特別なフラグを指定しない

        //表示していない方の、レンダーターゲット番号を取得
        currentFrameIndex = swapChain->GetCurrentBackBufferIndex();//現在、描画されていない方のターゲット番号

    }






}


void DirectX12App::WaitForGPUFinish() {


    //グラフィックボードの処理が終わるのを待機

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


    //グラフィックボードの処理の終了を待ってから、アプリケーションを終了する

    WaitForGPUFinish();


}



