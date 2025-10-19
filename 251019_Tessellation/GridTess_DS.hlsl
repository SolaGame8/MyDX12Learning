#include "GridTess_Common.hlsli"


//�h���C���V�F�[�_�[

[domain("tri")]
DS_OUT DS_Main(HS_CONST_OUT hsConst,
               const OutputPatch<HS_CP_OUT, 3> patch,
               float3 bary : SV_DomainLocation)
{
    
    //bary = (u,v,w) ���d�݂Ƃ��ĎO�p�`3�_�̐��`��ԁi�����̈ʒu���킩��j
    
    float3 Pw = patch[0].posW * bary.x + patch[1].posW * bary.y + patch[2].posW * bary.z;
    float4 Col = patch[0].color * bary.x + patch[1].color * bary.y + patch[2].color * bary.z;
    float2 Tex = patch[0].uv * bary.x + patch[1].uv * bary.y + patch[2].uv * bary.z;


    //�n�`��ό`

    float len = length(Pw.xz - shaderParam[0].xy);

    if (len < 1.0f && shaderParam[0].z > 0.0f)
    {
        float shift = sin(3.141592 * 0.5f * (1.0 - len)) * shaderParam[0].z;
        Pw.y += shift;
    }
    
    

    DS_OUT o;
    o.posH = mul(projMat, mul(viewMat, float4(Pw, 1.0)));   //�J�����̍s���K�p
    o.col = Col;
    o.uv = Tex;
    
    return o;
}


