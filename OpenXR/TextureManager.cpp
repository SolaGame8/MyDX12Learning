#include "TextureManager.h"




TextureManager& TextureManager::GetInstance() {

    static TextureManager instance; //インスタンスを渡す
    return instance;

}


bool TextureManager::LoadTexture(ID3D12Device* device, const std::string& filePath, const std::string& textureMapKey) {


    textureSubresources.clear();
    textureUploadBuffer = nullptr;

    DirectX::TexMetadata metadata = {};							//画像サイズやMip、フォーマットとかの情報
    DirectX::ScratchImage scratchImg = {};						//スクラッチイメージ

    D3D12_RESOURCE_DESC textureDesc = {};						//画像サイズやMip、フォーマットとかの情報

    UINT64 uploadBufferSize = 0;								//リソースのサイズ
    UINT subresoucesize = 1;									//サブリソースのサイズ

    HRESULT hr = S_OK;



    // コマンドリストの作成
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    ComPtr<ID3D12GraphicsCommandList> commandList;
    ComPtr<ID3D12CommandQueue> commandQueue;
    ComPtr<ID3D12Fence> fence;
    HANDLE fenceEvent = nullptr;
    
    
    {


        //Queue
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        hr = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));
        if (FAILED(hr)) OutputDebugStringA("Failed to create temporary Command Queue.");

        //Fence
        if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)))) {
            OutputDebugStringA("Failed to create Fence.\n");
            return false;
        }

        fenceEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
        
        if (!fenceEvent) {
            OutputDebugStringA("Failed to create Fence Event.\n");
            return false;
        }

        hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));//コマンドのメモリ管理
        if (FAILED(hr)) return false;

        hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));//リスト
        if (FAILED(hr)) return false;



    }

    // マルチバイト文字列からワイド文字列へ変換

    setlocale(LC_CTYPE, "jpn");
    wchar_t wFilename[256];
    size_t ret;
    mbstowcs_s(&ret, wFilename, filePath.c_str(), 256);


    //Png読み込み
    LoadFromWICFile(wFilename, DirectX::WIC_FLAGS_NONE, &metadata, scratchImg);

    //アップロード準備
    PrepareUpload(device, scratchImg.GetImages(), scratchImg.GetImageCount(), metadata, textureSubresources);



    // テクスチャバッファの生成 Default

    const int SampleDesc_Count = 1;
    const int SampleDesc_Quality = 0;


    //D3D12_RESOURCE_DESC textureDesc = {};

    textureDesc.Format = metadata.format;
    textureDesc.Width = static_cast<UINT>(metadata.width);
    textureDesc.Height = static_cast<UINT>(metadata.height);
    textureDesc.DepthOrArraySize = static_cast<UINT16>(metadata.arraySize);
    textureDesc.MipLevels = static_cast<UINT16>(metadata.mipLevels);		//ミップ
    textureDesc.SampleDesc.Count = SampleDesc_Count;						//ピクセルあたりのマルチサンプルの数。 
    textureDesc.SampleDesc.Quality = SampleDesc_Quality;					//イメージの品質レベル。 品質が高いほど、パフォーマンスが低下します
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    auto textureHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);    //Defalt


    ComPtr<ID3D12Resource> textureResource; //Default

    //バッファの作成
    device->CreateCommittedResource(
        &textureHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        D3D12_RESOURCE_STATE_COPY_DEST, //コピー対象
        nullptr,
        IID_PPV_ARGS(textureResource.ReleaseAndGetAddressOf())
    );



    UINT64 textureBufferSize;

    textureBufferSize = GetRequiredIntermediateSize(textureResource.Get(), 0, 1);

    
    auto textureUploadHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto textureUploadDesc = CD3DX12_RESOURCE_DESC::Buffer(textureBufferSize);

    //バッファの作成（中間アップロード）
    device->CreateCommittedResource(
        &textureUploadHeapProp, //アップロード
        D3D12_HEAP_FLAG_NONE,
        &textureUploadDesc, //バッファサイズ
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&textureUploadBuffer)
    );


    //サブリソース更新
    UpdateSubresources(
        commandList.Get(),
        textureResource.Get(),	//行き先
        textureUploadBuffer.Get(),	//中間のバッファ
        0, 0,
        static_cast<unsigned int>(textureSubresources.size()), textureSubresources.data());


    //バリア
    auto uploadResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(textureResource.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    commandList->ResourceBarrier(1, &uploadResourceBarrier);


    // コマンドのクローズ
    commandList->Close();


    {

        //実行
        ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
        commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        //処理が終わるのを待機


        // Wait for the GPU to finish   //グラフィックボードの処理が終わったかどうか
        const UINT64 fenceToWaitFor = fenceValue;

        if (FAILED(commandQueue->Signal(fence.Get(), fenceToWaitFor))) {
            OutputDebugStringA("Failed to signal fence.\n");
            return false;
        }
        fenceValue++;

        if (fence->GetCompletedValue() < fenceToWaitFor) {
            if (FAILED(fence->SetEventOnCompletion(fenceToWaitFor, fenceEvent))) {
                OutputDebugStringA("Failed to set event on completion.\n");
                return false;
            }
            WaitForSingleObject(fenceEvent, INFINITE);
        }


    }

    // マップにリソースを保存
    textureResources[textureMapKey] = textureResource;

    OutputDebugStringA(("Successfully loaded " + filePath + "\n").c_str());
    


    return true;

}



ComPtr<ID3D12Resource> TextureManager::GetTextureResource(const std::string& textureName) {


    // マップから指定された名前のテクスチャリソースを検索
    auto it = textureResources.find(textureName);

    // テクスチャが見つかった場合
    if (it != textureResources.end()) {
        return it->second;
    }

    // テクスチャが見つからなかった場合
    return nullptr;

}






