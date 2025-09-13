#pragma once

#include <d3d12.h>
#include "d3dx12.h"
#include <dxgidebug.h>


#pragma comment(lib, "d3d12.lib")   //DirectX12 ���C�u����
#pragma comment(lib, "dxgi.lib")

#pragma comment(lib, "dxguid.lib")  //DirectX12�� �K�{�̃��C�u�����iGUID�j


//��DirectXTex���_�E�����[�h���āA�v���W�F�N�g�̐ݒ�ŃC���N���[�h�f�B���N�g���ɂ��̕ۑ��ꏊ�̃p�X������B
// �v���W�F�N�g�ݒ�́A$(DXTEX_DIR);�Ə����Ă��镔���ł�
#include <DirectXTex.h>             //DirectXTex �e�N�X�`���[�ǂݍ���
#pragma comment(lib, "DirectXTex.lib")



#include <locale.h>     //setlocale�Ŏg�p

#include <string>
#include <vector>
#include <map>
#include <wrl.h>


// ComPtr��using�錾
using Microsoft::WRL::ComPtr;
using namespace std;

class TextureManager
{
public:

    // �V���O���g���p�^�[����K�p
    static TextureManager& GetInstance();

    // �e�N�X�`���ǂݍ��݊֐�
    bool LoadTexture(ID3D12Device* device, const std::string& filePath, const std::string& textureMapKey);

    // �ǂݍ��񂾃e�N�X�`���̃��\�[�X���擾����֐�
    ComPtr<ID3D12Resource> GetTextureResource(const std::string& textureName);

private:


    TextureManager() = default;
    ~TextureManager() = default;
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    // ���O�istring�j�ŁA�e�N�X�`�����\�[�X���Ǘ�
    std::map<std::string, ComPtr<ID3D12Resource>> textureResources; //�����Ńe�N�X�`���[��ێ�


    //�e�N�X�`���[���O���t�B�b�N�{�[�h�ɓn���Ƃ��Ɏg���ϐ�
    UINT64 fenceValue = 0;

    vector<D3D12_SUBRESOURCE_DATA> textureSubresources;
    ComPtr<ID3D12Resource> textureUploadBuffer;



};

