#include "DirectX12App.h"

//#include <Windows.h>
#include <d3d12sdklayers.h>
#include <sstream>
#include <cmath>

// コンストラクタ
DirectX12App::DirectX12App() {

}

// デストラクタ
DirectX12App::~DirectX12App() {

}

// 初期化メソッド
bool DirectX12App::Initialize(HWND hwnd) {  //ウインドウのハンドルの受け取り


#if defined(_DEBUG)

    // DirectX 12 デバッグレイヤーを有効にする    （デバッグビルドでのみ有効

    ID3D12Debug* debugController = nullptr;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();
    }

#endif


    ref_hwnd = hwnd;


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

        const int shaderNum = 1;

        //シェーダー作成   （0 地面用
        for (size_t i = 0; i < shaderNum; i++) {
            if (!CreateShader(i)) {
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
        for (size_t i = 0; i < shaderNum; i++) {
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


    return true;
}





void DirectX12App::InitVariable() {



    //変数を設定

    const int shaderNum = 1;

    pipelineState.resize(shaderNum);        //描画ルール 1つ（地面
    pipelineState_Grid.resize(shaderNum);
    rootSignature.resize(shaderNum);

    vertexShader.resize(shaderNum);     //シェーダー 1つ（地面
    hullShader.resize(shaderNum);
    domainShader.resize(shaderNum);
    pixelShader.resize(shaderNum);
    errorBlob.resize(shaderNum);


    const int meshNum = 1;

    vertexBufferArray.resize(meshNum);        //描画用　頂点データ　２つ（板ポリ、地面
    vertexBufferViewArray.resize(meshNum);

    indexBufferArray.resize(meshNum);         //描画用　頂点インデックスデータ（どの頂点番号がつながって三角形になっているかのデータ
    indexBufferViewArray.resize(meshNum);



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
    UINT resolutionWidth = 1280;
    UINT resolutionHeight = 720;


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
    depthStencilDesc.Width = 1280; // ウィンドウ幅
    depthStencilDesc.Height = 720; // ウィンドウ高さ
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



    // 頂点シェーダーのコンパイル
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
        path,                      // HLSL ファイル
        nullptr,                   // マクロ
        D3D_COMPILE_STANDARD_FILE_INCLUDE, // #include 対応
        entry, target,             // エントリ名 / プロファイル
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

    srvHeapDesc.NumDescriptors = 8; // テクスチャの枚数に合わせてサイズを決定

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
    CD3DX12_ROOT_PARAMETER rootParameters[2];
    // 定数バッファビュー（CBV）をルートパラメータとして追加
    // b0 レジスタをバインド
    rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);  //CBV 定数バッファ

    CD3DX12_DESCRIPTOR_RANGE ranges[1];
    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 8, 0); // 複数のSRV

    rootParameters[1].InitAsDescriptorTable(1, ranges, D3D12_SHADER_VISIBILITY_PIXEL); // テクスチャー


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
        _countof(rootParameters), // パラメータ数: 2
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


    //テッセレーション用シェーダー

    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader[idx].Get());          //頂点シェーダー（コンパイル済み
    psoDesc.HS = CD3DX12_SHADER_BYTECODE(hullShader[idx].Get());            //ハルシェーダー（コンパイル済み
    psoDesc.DS = CD3DX12_SHADER_BYTECODE(domainShader[idx].Get());          //ドメインシェーダー（コンパイル済み
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader[idx].Get());           //ピクセルシェーダー（コンパイル済み




    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);       //ラスタライズの設定
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;                //三角形の裏面も描画
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;               //通常モード     //ワイヤー D3D12_FILL_MODE_WIREFRAME

    psoDesc.BlendState = blendDesc;                                         //ブレンドの設定


    
    psoDesc.DepthStencilState.DepthEnable = TRUE;                           //深度バッファを有効にする
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;  //深度の書き込みを有効にする
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;       //深度テストの条件（値が小さい方（カメラに近い方）を描画
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;                              //深度バッファのフォーマット（32bit float

    psoDesc.SampleMask = UINT_MAX;



    //psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; //三角形を描画する（通常）

    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH; // ＊＊＊＊＊（重要）テッセレーションは PATCH



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

    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;// ワイヤーフレーム

    hr = dx12Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState_Grid[idx])); //グリッド用パイプライン

    if (FAILED(hr)) {
        OutputDebugStringA("Failed to create Pipeline State Object.\n");
        return false;
    }

    OutputDebugStringA("Successfully initialized all components for drawing.\n");


    return true;

}








bool DirectX12App::CreateVertex() {

    //描画する頂点情報を作成

    CreateFloor();  //地面

    return true;

}





void DirectX12App::CreateFloor() {

    //床

    //頂点情報を作る

    HRESULT hr = S_OK;

    int myIdx = 0;

    VertexInfo vertInfo;



    {
        vector<Vertex> vertexArr;

        vector<WORD> indiceArr;


        float gridSize = 1.0f;  //１マスのサイズ
        int floor_length = 64;  //64 * 64 * 4   頂点
        float startX = -gridSize * floor_length * 0.5f;
        float startY = -gridSize * floor_length * 0.5f;

        int indiceCounter = 0;

        float margin = 0.0f;
        float blockSize = 1.0f - margin;


        float sideX, sideZ;
        float fh = 0.0f;


        for (size_t y = 0; y < floor_length; y++) { //64

            vector<float>heightArr; //高さ情報

            for (size_t x = 0; x < floor_length; x++) { //64



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




                //高さ情報を配列に入れる
                heightArr.emplace_back(fh);

            }


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




bool DirectX12App::LoadTextures() {

    //テクスチャーの読み込み


    if (!TextureManager::GetInstance().LoadTexture(
        dx12Device.Get(),
        "Resources/floor001.png",
        "floor001"
    )) {

        OutputDebugStringA("Failed to load Texture........\n");
        return false;
    }


    // SRVのヒープのハンドル
    CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(srvHeap->GetCPUDescriptorHandleForHeapStart());
    UINT srvDescriptorSize = dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);



    {
        // テクスチャの ビュー（情報）を作成
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = TextureManager::GetInstance().GetTextureResource("floor001")->GetDesc().Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        dx12Device->CreateShaderResourceView(TextureManager::GetInstance().GetTextureResource("floor001").Get(), &srvDesc, srvHandle);

        srvHandle.Offset(1, srvDescriptorSize); //次のヒープ情報の頭へ

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

    conBufData.viewMat = XMMatrixLookAtLH(eye, at, up); //ビュー

    float fovAngleY = XM_PIDIV4; // 視野角 (45度)
    float aspectRatio = 1280.0f / 720.0f; // 画面解像度からアスペクト比を計算
    float nearZ = 0.01f; // 近クリップ面
    float farZ = 100.0f; // 遠クリップ面

    conBufData.projMat = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, nearZ, farZ); //プロジェクション

    //モデルの回転用
    conBufData.worldMat = XMMatrixIdentity();



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


XMFLOAT2 DirectX12App::ScreenToFloorXZ() {

    //カーソルが指している床の場所(x,z)   //ヒットなしは (0,0)

    // ウインドウ　クライアント領域サイズ
    RECT rc;
    if (!GetClientRect(ref_hwnd, &rc)) return XMFLOAT2(0.0f, 0.0f);
    const float width = (float)(rc.right - rc.left);
    const float height = (float)(rc.bottom - rc.top);
    if (width <= 0.0f || height <= 0.0f) return XMFLOAT2(0.0f, 0.0f);

    // マウス座標（クライアント座標）
    POINT pt;
    if (!GetCursorPos(&pt))         return XMFLOAT2(0.0f, 0.0f);
    if (!ScreenToClient(ref_hwnd, &pt)) return XMFLOAT2(0.0f, 0.0f);
    const float mx = (float)pt.x;
    const float my = (float)pt.y;

    // ウインドウ座標 -> NDC（Direct3D: 上:+1, 下:-1）
    const float ndcX = (mx / width) * 2.0f - 1.0f;
    const float ndcY = 1.0f - (my / height) * 2.0f;


    XMMATRIX ViewProjMat = conBufData.viewMat * conBufData.projMat;


    // 逆行列(ViewProj)
    XMVECTOR det;
    const XMMATRIX invViewProj = XMMatrixInverse(&det, ViewProjMat);

    XMVECTOR pNear = XMVectorSet(ndcX, ndcY, 0.0f, 1.0f);
    XMVECTOR pFar = XMVectorSet(ndcX, ndcY, 1.0f, 1.0f);

    pNear = XMVector4Transform(pNear, invViewProj);
    pFar = XMVector4Transform(pFar, invViewProj);

    // 同次除算
    pNear = XMVectorScale(pNear, 1.0f / XMVectorGetW(pNear));
    pFar = XMVectorScale(pFar, 1.0f / XMVectorGetW(pFar));

    // レイ
    const XMVECTOR rayOrig = pNear;
    XMVECTOR rayDir = XMVector3Normalize(XMVectorSubtract(pFar, pNear));

    const float origY = XMVectorGetY(rayOrig);
    const float dirY = XMVectorGetY(rayDir);

    // 床(y=0)と平行
    const float eps = 1e-6f;
    if (dirY > -eps && dirY < eps) return XMFLOAT2(0.0f, 0.0f);

    const float t = -origY / dirY;
    
    //背面側はヒットなし
    //if (t < 0.0f) return XMFLOAT2(0.0f, 0.0f);

    // 交点
    const XMVECTOR hit = XMVectorMultiplyAdd(rayDir, XMVectorReplicate(t), rayOrig);
    return XMFLOAT2(XMVectorGetX(hit), XMVectorGetZ(hit)); // (x,z)


}



void DirectX12App::CalcKey() {


    const float pullSpeed = 3.0f;

    //マウス左（地形の変形）
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


    //マウス右（グリッド / テクスチャー表示切り替え）
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


    //キー操作
    CalcKey();


    cameraRot.y += 5.0f * deltaTime;
    cameraRot.x = -30.0f + 10.0f * sin(XM_PI * 0.02f * updateCounter);


    //追従位置の計算（ターゲットを中心に一定距離で回る動き）
    CalculateFollowPosition();

    //カメラの計算
    CalcCamera();


    //カーソルが指している床の場所
    XMFLOAT2 floorCursolPos = ScreenToFloorXZ();


    //シェーダーに渡す、キャラクターの位置情報
    conBufData.shaderParam[0].x = floorCursolPos.x;
    conBufData.shaderParam[0].y = floorCursolPos.y;
    conBufData.shaderParam[0].z = sin(XM_PI * 0.5f *  pullStrength); //0.0 ～ 1.0
    conBufData.shaderParam[0].w = 1.0f;




    //CBV（定数バッファ更新） シェーダーで受け取る

    memcpy(pConstData, &conBufData, sizeof(ConstantBufferData));




}



// 描画メソッド
void DirectX12App::Render() {

    OnUpdate();


    //コマンドのリセット
    commandAllocator->Reset();                              
    commandList->Reset(commandAllocator.Get(), nullptr);


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
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), currentFrameIndex, rtvDescriptorSize);

    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsvHeap->GetCPUDescriptorHandleForHeapStart());

    // レンダーターゲットのクリア
    FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // 深度バッファをクリア
    commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    //レンダーターゲット（描画の出力先）
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // ビューポート
    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, 1280.0f, 720.0f, 0.0f, 1.0f };
    commandList->RSSetViewports(1, &viewport);

    //シザー矩形（切り抜き）
    D3D12_RECT scissorRect = { 0, 0, 1280, 720 };
    commandList->RSSetScissorRects(1, &scissorRect);



    //描画コマンド

    //idx = 0 地面シェーダー

    for (int idx = 0; idx < 1; idx++) {


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

        if (showGrid) {

            commandList->SetPipelineState(pipelineState_Grid[idx].Get());
        }
        else {

            commandList->SetPipelineState(pipelineState[idx].Get());
        }


        {

            //頂点描画

            int drawNum = 1;    //描画数



            //commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);   //三角形で書きます


            // 制御点パッチ（三角形）HS 側 [outputcontrolpoints(3)] と一致させる  ＊＊＊＊＊（重要）
            commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);  //テッセレーションはパッチ



            //頂点バッファ
            commandList->IASetVertexBuffers(0, 1, &vertexBufferViewArray[idx]);   //頂点情報リソースのビュー（情報）

            //インデックスバッファ
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


    commandList->Close();   //コマンドを閉じる


    {

        //コマンドの実行

        ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
        commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);


        //グラフィックボードの処理が終わるのを待機
        WaitForGPUFinish();
        

    }


    // スワップチェーンを切り替える
    swapChain->Present(1, 0);   //垂直同期 (VSync)  0は無効、1は次の垂直同期信号まで待機,  0: 最も一般的な値で、特別なフラグを指定しない

    //表示していない方の、レンダーターゲット番号を取得
    currentFrameIndex = swapChain->GetCurrentBackBufferIndex();//現在、描画されていない方のターゲット番号




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


    ref_hwnd = nullptr;

    //グラフィックボードの処理の終了を待ってから、アプリケーションを終了する

    WaitForGPUFinish();


}



