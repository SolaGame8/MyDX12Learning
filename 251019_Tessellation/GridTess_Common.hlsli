

// 共通 (include用)

cbuffer MyConstants : register(b0)
{
    float4 shaderParam[8]; // float4を8つ格納   //[0] xy マウス位置
    
    matrix worldMat;
    matrix viewMat;
    matrix projMat;

    
};

Texture2D texArray[8] : register(t0); // 複数のテクスチャを配列として定義
SamplerState smp : register(s0); //サンプラー（画像の色を取得する）



//頂点データフォーマット
struct VertexIn
{
    float4 pos : POSITION; // ワールド座標想定 (y=0)
    float4 color : COLOR;
    float4 uv : TEXCOORD;
};

//頂点シェーダー出力
struct VS_OUT
{
    float3 posW : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
};

//ハルシェーダー
struct HS_CP_OUT
{
    float3 posW : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
};

struct HS_CONST_OUT
{
    float Edges[3] : SV_TessFactor;
    float Inside[1] : SV_InsideTessFactor;
};

//ドメインシェーダー
struct DS_OUT
{
    float4 posH : SV_Position;
    float4 col : COLOR;
    float2 uv : TEXCOORD0;
};


