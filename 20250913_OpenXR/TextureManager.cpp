#include "TextureManager.h"




TextureManager& TextureManager::GetInstance() {

    static TextureManager instance; //�C���X�^���X��n��
    return instance;

}


bool TextureManager::LoadTexture(ID3D12Device* device, const std::string& filePath, const std::string& textureMapKey) {


    textureSubresources.clear();
    textureUploadBuffer = nullptr;

    DirectX::TexMetadata metadata = {};							//�摜�T�C�Y��Mip�A�t�H�[�}�b�g�Ƃ��̏��
    DirectX::ScratchImage scratchImg = {};						//�X�N���b�`�C���[�W

    D3D12_RESOURCE_DESC textureDesc = {};						//�摜�T�C�Y��Mip�A�t�H�[�}�b�g�Ƃ��̏��

    UINT64 uploadBufferSize = 0;								//���\�[�X�̃T�C�Y
    UINT subresoucesize = 1;									//�T�u���\�[�X�̃T�C�Y

    HRESULT hr = S_OK;



    // �R�}���h���X�g�̍쐬
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

        hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));//�R�}���h�̃������Ǘ�
        if (FAILED(hr)) return false;

        hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));//���X�g
        if (FAILED(hr)) return false;



    }

    // �}���`�o�C�g�����񂩂烏�C�h������֕ϊ�

    setlocale(LC_CTYPE, "jpn");
    wchar_t wFilename[256];
    size_t ret;
    mbstowcs_s(&ret, wFilename, filePath.c_str(), 256);


    //Png�ǂݍ���
    LoadFromWICFile(wFilename, DirectX::WIC_FLAGS_NONE, &metadata, scratchImg);

    //�A�b�v���[�h����
    PrepareUpload(device, scratchImg.GetImages(), scratchImg.GetImageCount(), metadata, textureSubresources);



    // �e�N�X�`���o�b�t�@�̐��� Default

    const int SampleDesc_Count = 1;
    const int SampleDesc_Quality = 0;


    //D3D12_RESOURCE_DESC textureDesc = {};

    textureDesc.Format = metadata.format;
    textureDesc.Width = static_cast<UINT>(metadata.width);
    textureDesc.Height = static_cast<UINT>(metadata.height);
    textureDesc.DepthOrArraySize = static_cast<UINT16>(metadata.arraySize);
    textureDesc.MipLevels = static_cast<UINT16>(metadata.mipLevels);		//�~�b�v
    textureDesc.SampleDesc.Count = SampleDesc_Count;						//�s�N�Z��������̃}���`�T���v���̐��B 
    textureDesc.SampleDesc.Quality = SampleDesc_Quality;					//�C���[�W�̕i�����x���B �i���������قǁA�p�t�H�[�}���X���ቺ���܂�
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    auto textureHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);    //Defalt


    ComPtr<ID3D12Resource> textureResource; //Default

    //�o�b�t�@�̍쐬
    device->CreateCommittedResource(
        &textureHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        D3D12_RESOURCE_STATE_COPY_DEST, //�R�s�[�Ώ�
        nullptr,
        IID_PPV_ARGS(textureResource.ReleaseAndGetAddressOf())
    );



    UINT64 textureBufferSize;

    textureBufferSize = GetRequiredIntermediateSize(textureResource.Get(), 0, 1);

    
    auto textureUploadHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto textureUploadDesc = CD3DX12_RESOURCE_DESC::Buffer(textureBufferSize);

    //�o�b�t�@�̍쐬�i���ԃA�b�v���[�h�j
    device->CreateCommittedResource(
        &textureUploadHeapProp, //�A�b�v���[�h
        D3D12_HEAP_FLAG_NONE,
        &textureUploadDesc, //�o�b�t�@�T�C�Y
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&textureUploadBuffer)
    );


    //�T�u���\�[�X�X�V
    UpdateSubresources(
        commandList.Get(),
        textureResource.Get(),	//�s����
        textureUploadBuffer.Get(),	//���Ԃ̃o�b�t�@
        0, 0,
        static_cast<unsigned int>(textureSubresources.size()), textureSubresources.data());


    //�o���A
    auto uploadResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(textureResource.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    commandList->ResourceBarrier(1, &uploadResourceBarrier);


    // �R�}���h�̃N���[�Y
    commandList->Close();


    {

        //���s
        ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
        commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        //�������I���̂�ҋ@


        // Wait for the GPU to finish   //�O���t�B�b�N�{�[�h�̏������I��������ǂ���
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

    // �}�b�v�Ƀ��\�[�X��ۑ�
    textureResources[textureMapKey] = textureResource;

    OutputDebugStringA(("Successfully loaded " + filePath + "\n").c_str());
    


    return true;

}



ComPtr<ID3D12Resource> TextureManager::GetTextureResource(const std::string& textureName) {


    // �}�b�v����w�肳�ꂽ���O�̃e�N�X�`�����\�[�X������
    auto it = textureResources.find(textureName);

    // �e�N�X�`�������������ꍇ
    if (it != textureResources.end()) {
        return it->second;
    }

    // �e�N�X�`����������Ȃ������ꍇ
    return nullptr;

}






