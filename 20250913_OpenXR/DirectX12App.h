#pragma once

#include <Windows.h>

#include <d3d12.h>
#include "d3dx12.h"
#include <dxgi1_6.h> // DXGI (DirectX Graphics Infrastructure)
#include <DirectXMath.h>

#include <wrl.h>    //ComPtr

#pragma comment(lib, "d3d12.lib")       //DirectX12 ライブラリ
#pragma comment(lib, "dxgi.lib")

#include "d3dcompiler.h"                //シェーダーコンパイル
#pragma comment(lib, "d3dcompiler.lib") 

#include <chrono>   //時間

#include <cstdlib>  // rand()とsrand()のため
#include <ctime>    // time()のため
#include <random>   // 乱数ライブラリ


#include "TextureManager.h" //テクスチャー読み込み（mapでデータ保持）

#include "PerlinNoise.h"    //パーリンノイズ（地形用の なだらかにつながる乱数）

#include "OpenXRManager.h"  //OpenXR


// using 宣言を追加
using Microsoft::WRL::ComPtr;
using namespace DirectX;


class DirectX12App
{
public:
    //DirectX12App();
    //~DirectX12App();


    bool Initialize(HWND hwnd);                 //初期化

    void InitVariable();                        //変数　初期設定

    bool CreateDevice();                        //デバイス作成（使用するグラフィックボードを取得
    
    bool CreateCommandObjects();                //コマンド関連ツール

    bool CreateSwapChain(HWND hwnd);            //スワップチェーン（画面切り替え）
    bool CreateRTV();                           //レンダーターゲット（スワップチェーンのリソース）
    bool CreateDepthBuffer();                   //深度バッファ

    float GetFloorHeight(float x, float y);     //地面の高さを取得

    bool LoadTextures();

    bool CreatShader(int idx);                  //シェーダー
    bool CreateCBV();                           //CBV 定数バッファビュー       （これに設定して、シェーダーに 変数 を渡す
    bool CreateSRV();                           //SRV シェーダーリソースビュー  （これに設定して、シェーダーに テクスチャー を渡す

    bool CreatePipelineState(int idx);          //パイプラインステート（描画のルール決め


    bool CreateVertex();                        //描画用の頂点データを作成
    void CreatePlate();                         //板ポリ
    void CreateFloor();                         //地面ポリゴン（地面の高さデータも配列で保持）



    void CalcCamera();                          //カメラの計算
    void CalculateFollowPosition();
    void CalcKey();                             //キー操作

    float GetDeltaTime();                       //DeltaTime作成

    void OnUpdate();                            //情報更新


    void Render();                              //描画処理


    void WaitForGPUFinish();                    //グラフィックボードの処理を待つ

    void OnDestroy();                           //アプリケーション終了処理


private:


    //ーーーーー 機能の準備用 ーーーーー


    ComPtr<IDXGIFactory4> dxgiFactory;  //ファクトリー
    ComPtr<ID3D12Device> dx12Device;    //グラフィックボード(デバイス)


    //コマンド関連のツール
    ComPtr<ID3D12GraphicsCommandList> commandList;      //コマンドを書く
    ComPtr<ID3D12CommandAllocator> commandAllocator;    //コマンドのメモリ管理

    ComPtr<ID3D12CommandQueue> commandQueue;            //コマンド実行役

    ComPtr<ID3D12Fence> fence;      //コマンドが終了したか監視
    UINT64 fenceValue = 0;
    HANDLE fenceEvent = nullptr;


    //スワップチェーン（画面の切り替え）
    ComPtr<IDXGISwapChain3> swapChain;

    static const UINT FrameBufferCount = 2; //２枚で切り替え
    UINT currentFrameIndex = 0;     //書き込みに使うリソース番号

    //レンダーターゲット
    ComPtr<ID3D12DescriptorHeap> rtvHeap;                       //ヒープ       （リソース２枚分の情報が入る
    ComPtr<ID3D12Resource> renderTargets[FrameBufferCount];     //リソース ２枚    1280 x 720

    //深度バッファ
    ComPtr<ID3D12DescriptorHeap> dsvHeap;                       //ヒープ       （深度用のリソース1枚分の情報が入る
    ComPtr<ID3D12Resource> depthBuffer;

    //ーーーーー OpenXR ーーーーー

    OpenXRManager* XR_Manager;

    bool flg_useVRMode = false;

    //ーーーーー 描画の準備用 ーーーーー

    //解像度
    XMINT2 ResResolution = { 1280, 720 };


    //シェーダー
    vector <ComPtr<ID3DBlob>> vertexShader;
    vector <ComPtr<ID3DBlob>> pixelShader;
    vector <ComPtr<ID3DBlob>> errorBlob;

    //頂点の構造体    （このフォーマットで、シェーダーに頂点情報が送られる
    struct Vertex {
        XMFLOAT4 Pos;       //xyzw  float * 4 
        XMFLOAT4 Color;     //rgba  float * 4
        XMFLOAT4 UV;        //uv    float * 4
    };

    struct VertexInfo
    {
        UINT vertexBufferSize = 0u; //頂点数（機能的には未使用。データを表示したい時などに）
        UINT indexBufferSize = 0u;  //頂点インデックス数（描画時にこの数を使用）
    };


    //CBV 定数バッファ（シェーダーに渡す変数を入れておくリソース
    ComPtr<ID3D12Resource> constantBuffer;

    struct ConstantBufferData    //（このフォーマットで、シェーダーに変数情報が送られる
    {
        XMFLOAT4 shaderParam[8];

        XMMATRIX worldMat;
        XMMATRIX viewMat;
        XMMATRIX projMat;
    };

    ConstantBufferData conBufData = {};

    void* pConstData = nullptr;   //ここがconstantBufferのリソース位置。ここに変数データを上書きするとシェーダーに渡される値が更新される


    //SRV シェーダーリソースビュー
    ComPtr<ID3D12DescriptorHeap> srvHeap;   //ヒープ   （シェーダーに渡すテクスチャーリソース情報。リソースはTextureManagerで管理


    //描画のルールみたいなのを書いたもの
    vector <ComPtr<ID3D12PipelineState>> pipelineState;          // パイプラインステート
    vector <ComPtr<ID3D12RootSignature>> rootSignature;          // ルートシグネチャ





    //ーーーーー 描画するデータの準備用 ーーーーー

    //[0] 板ポリ、[1] 地形
    vector <ComPtr<ID3D12Resource>> vertexBufferArray;                // 頂点情報を入れるリソース
    vector <D3D12_VERTEX_BUFFER_VIEW> vertexBufferViewArray;          // 頂点情報を入れるリソースの ビュー（情報）

    vector <VertexInfo> vertexInfoArray;

    vector <ComPtr<ID3D12Resource>> indexBufferArray;                 // 頂点インデックス情報を入れるリソース
    vector <D3D12_INDEX_BUFFER_VIEW> indexBufferViewArray;            // 頂点インデックス情報を入れるリソースの ビュー（情報）

    //地形の高さ保持用
    vector<vector<float>> floorHeightArray;




    //ーーーーー


    PerlinNoise* pNoise = nullptr;    //パーリンノイズ


    float updateCounter = 0.0f;
    //float shaderCounter = 0.0f;

    std::chrono::steady_clock::time_point lastFrameTime;    //DeltaTime計算用
    float deltaTime = 0.0f;


    //カメラ
    XMFLOAT3 cameraPos = { 0.0f, 3.0f, -5.0f };    // カメラの位置
    XMFLOAT3 cameraRot = { 0.0f, 0.0f, 0.0f };  // カメラの回転
    XMFLOAT3 cameraTarget = { 0.0f, 0.0f, 0.0f };  // カメラが向く点

    float distanceFromTarget = 5.0f; //ターゲット追従の距離（m）
    float distanceFromTarget_adjust = 1.0f; //ターゲット追従の高さ調整（m）

    //マウス　（以前の位置を記録しておいて、差分で移動量を計算
    POINT lastMousePos = {0, 0};

    //キャラクター
    XMFLOAT3 charaPos = { 0.0f, 0.0f, 0.0f };

    //キャラアニメーション
    int charaTexNum = 0;
    int charaTexAnimNum = 0;
    float charaCounter = 0.0f;

    //重力
    float gravity = 0.98f * 0.5f;

    //接地してるかどうか
    bool isOnGround = false;
    
    //ジャンプ
    float charaJumpAcc = 0.0f;

    //モブ、コイン用

    vector<XMFLOAT3> mobPos;
    vector<int> mobUVIdx;

    float mobCounter = 0.0f;
    int mobTexAnimNum = 0;

    XMFLOAT3 mobTarget = { 0.0f, 0.0f, 0.0f, };




};


