#include "GridTess_Common.hlsli"

//ハルシェーダー

[domain("tri")]
//[partitioning("fractional_even")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("HS_PatchConstants")]
HS_CP_OUT HS_Main(InputPatch<VS_OUT, 3> patch, uint cpId : SV_OutputControlPointID)
{
    HS_CP_OUT o;
    o.posW = patch[cpId].posW;
    o.color = patch[cpId].color;
    o.uv = patch[cpId].uv;
    return o;
}

HS_CONST_OUT HS_PatchConstants(InputPatch<VS_OUT, 3> patch, uint patchId : SV_PrimitiveID)
{

    
    float dv = 1;
    
    float l0 = length(patch[0].posW.xz - shaderParam[0].xy);
    float l1 = length(patch[1].posW.xz - shaderParam[0].xy);
    float l2 = length(patch[2].posW.xz - shaderParam[0].xy);
    
    const float threshold = 1.5f;

    if (shaderParam[0].z > 0.0f)
    {
        if (l0 < threshold || l1 < threshold || l2 < threshold)
        {
            dv = 8;
        }
    }
    //ポリゴンの分割数を設定
    
    HS_CONST_OUT o;

    o.Edges[0] = dv;
    o.Edges[1] = dv;
    o.Edges[2] = dv;
    o.Inside[0] = dv;
    
    
    
    return o;

}


