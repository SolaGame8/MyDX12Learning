#include "GridTess_Common.hlsli"

//���_�V�F�[�_�[
VS_OUT VS_Main(VertexIn i)
{
    VS_OUT o;
    o.posW = i.pos.xyz;
    o.color = i.color;
    o.uv = i.uv.xy;
    return o;
}


