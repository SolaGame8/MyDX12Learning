#include "GridTess_Common.hlsli"

//�s�N�Z���V�F�[�_�[

float4 PS_Main(DS_OUT i) : SV_Target
{
    
    float4 col = texArray[0].Sample(smp, i.uv.xy);
    
    return col;
    
}
