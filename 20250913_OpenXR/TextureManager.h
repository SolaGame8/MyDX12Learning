#pragma once

#include <d3d12.h>
#include "d3dx12.h"
#include <dxgidebug.h>


#pragma comment(lib, "d3d12.lib")   //DirectX12 ライブラリ
#pragma comment(lib, "dxgi.lib")

#pragma comment(lib, "dxguid.lib")  //DirectX12に 必須のライブラリ（GUID）


//＊DirectXTexをダウンロードして、プロジェクトの設定でインクルードディレクトリにその保存場所のパスを入れる。
// プロジェクト設定の、$(DXTEX_DIR);と書いてある部分です
#include <DirectXTex.h>             //DirectXTex テクスチャー読み込み
#pragma comment(lib, "DirectXTex.lib")



#include <locale.h>     //setlocaleで使用

#include <string>
#include <vector>
#include <map>
#include <wrl.h>


// ComPtrのusing宣言
using Microsoft::WRL::ComPtr;
using namespace std;

class TextureManager
{
public:

    // シングルトンパターンを適用
    static TextureManager& GetInstance();

    // テクスチャ読み込み関数
    bool LoadTexture(ID3D12Device* device, const std::string& filePath, const std::string& textureMapKey);

    // 読み込んだテクスチャのリソースを取得する関数
    ComPtr<ID3D12Resource> GetTextureResource(const std::string& textureName);

private:


    TextureManager() = default;
    ~TextureManager() = default;
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    // 名前（string）で、テクスチャリソースを管理
    std::map<std::string, ComPtr<ID3D12Resource>> textureResources; //ここでテクスチャーを保持


    //テクスチャーをグラフィックボードに渡すときに使う変数
    UINT64 fenceValue = 0;

    vector<D3D12_SUBRESOURCE_DATA> textureSubresources;
    ComPtr<ID3D12Resource> textureUploadBuffer;



};

