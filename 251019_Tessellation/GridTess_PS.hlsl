#include "GridTess_Common.hlsli"

//ピクセルシェーダー

float4 PS_Main(DS_OUT i) : SV_Target
{
    
    float4 col = texArray[0].Sample(smp, i.uv.xy);
    
    return col;
    
}
