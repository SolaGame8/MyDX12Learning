

// ���� (include�p)

cbuffer MyConstants : register(b0)
{
    float4 shaderParam[8]; // float4��8�i�[   //[0] xy �}�E�X�ʒu
    
    matrix worldMat;
    matrix viewMat;
    matrix projMat;

    
};

Texture2D texArray[8] : register(t0); // �����̃e�N�X�`����z��Ƃ��Ē�`
SamplerState smp : register(s0); //�T���v���[�i�摜�̐F���擾����j



//���_�f�[�^�t�H�[�}�b�g
struct VertexIn
{
    float4 pos : POSITION; // ���[���h���W�z�� (y=0)
    float4 color : COLOR;
    float4 uv : TEXCOORD;
};

//���_�V�F�[�_�[�o��
struct VS_OUT
{
    float3 posW : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
};

//�n���V�F�[�_�[
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

//�h���C���V�F�[�_�[
struct DS_OUT
{
    float4 posH : SV_Position;
    float4 col : COLOR;
    float2 uv : TEXCOORD0;
};


