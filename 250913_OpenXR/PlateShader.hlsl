

cbuffer MyConstants : register(b0)  //b0に登録した情報を取得
{

    float4 shaderParam[8]; // float4を8つ格納
    
        //[0]   キャラクターの位置             x,y,z
        //[1]   キャラクターの描画するUVの位置   u,v,頂点id
    
        //[2]   モブの位置             x,y,z
        //[3]   モブの描画するUVの位置   u,v,頂点id
        
        //[4]   コインの位置             x,y,z
        //[5]   コインの描画するUVの位置   u,v,頂点id
    
        //＊3D空間での位置は、このようにfloat4ではなく、matrix（位置、回転、スケールが合わさったもの）で情報を渡した方がいいです
        //その情報を、worldMatとして扱う感じです
    
    matrix worldMat;    //不使用（変化なし行列）　3D空間でのキャラクターの移動回転などに使う
    matrix viewMat;     //カメラのビュー行列
    matrix projMat;     //カメラのプロジェクション行列
    
    /*
    このCBV 定数バッファ（変数情報）は、16バイトを意識した方がいいです（16バイト アライメント）
    そうすることで、シェーダーにきちんと情報が渡されるようになります
    高速で処理するために、このようなルールがあるようです
    
    たとえば、
    float4 shaderParam[8];  //float（4バイト）* 4 * 8 = 128（16バイトの8倍）
    matrix worldMat;        //float（4バイト）* 16（4x4行列） = 64（16バイトの4倍） 

    なので、もしもうまくシェーダーに値が渡されないな～って思ったらチェックしてみてください
    */
    
};

/*
//たとえば、b1に登録した情報を受け取りたい場合は、こんな感じです。b2以降も同様です
cbuffer MyConstants : register(b1)
{
    float4 anotherInfo[16];
};
*/



Texture2D texArray[2] : register(t0); // 複数のテクスチャを配列として定義。　配列なので、t0-t1に登録された情報を取得

/*
//たとえば、t2に登録した情報を取得したい場合はこんな感じです
Texture2D anotherTexture : register(t2);
*/



SamplerState smp : register(s0);        //サンプラー（画像の色を取得する）  s0に登録された情報を取得

/*
//たとえば、s1に登録した情報を取得したい場合はこんな感じです
SamplerState smp : register(s1);
*/




// シェーダーの入力構造体
struct VertexIn
{
    float4 pos : POSITION; // 頂点の位置情報
    float4 color : COLOR; // 頂点の色情報
    float4 uv : TEXCOORD; // UV座標情報
    
    uint instanceId : SV_InstanceID; // インスタンスIDを追加 0-7
};

// シェーダーの出力構造体
struct VertexOut
{
    float4 pos : SV_POSITION; // 頂点の最終的な位置（スクリーン座標系）
    float4 color : COLOR; // 補間された色情報
    float2 uv : TEXCOORD; // 補間されたUV座標
};

// 頂点シェーダー (Vertex Shader)
// 各頂点のデータを受け取り、スクリーン座標へ変換
VertexOut VSMain(VertexIn vin)
{
    VertexOut vout;
    
    // インスタンスIDを取得
    uint instanceId = vin.instanceId;   //0-7

    float4 col = vin.color;
    if (instanceId == 1)
    {
        col.r = 1.0f;

    }
    
    
    float4 pos = vin.pos;

    pos.xyz += shaderParam[instanceId * 2].xyz;     //float4のx,y,zの成分にそれぞれのx,y,zの成分を足しています。rgbとかいてもxyzと同じ結果になります。

    int id = (int)vin.uv.z; //0123　頂点のid
    
    float2 uv = shaderParam[instanceId * 2 + 1].xy; //左上の頂点の時は、テクスチャーの指定された位置
    //これはfloat4のx,yの成分を、float2のx,yに入れています。ちなみにfloat4の成分は、xyzw, rgbaなど使いやすい方で取得できます
    
    if (id == 1)                                    //右上の頂点だったら、テクスチャーの指定された位置から、横幅分 右にずらす
    {
        uv.x += shaderParam[instanceId * 2 + 1].z;  //テクスチャーの1ブロック幅
    }
    else if (id == 2)                               //左下の頂点だったら、テクスチャーの指定された位置から、縦幅分 下にずらす
    {
        uv.y += shaderParam[instanceId * 2 + 1].w;  //テクスチャーの1ブロック高さ
    }
    else if (id == 3)                               //右下の頂点だったら、テクスチャーの指定された位置から、右下にずらす
    {
        uv.x += shaderParam[instanceId * 2 + 1].z;  //1ブロック幅
        uv.y += shaderParam[instanceId * 2 + 1].w;  //1ブロック高さ
    }
    
    
    
    //カメラの変換行列（掛ける順番注意）
    pos = mul(worldMat, pos);
    pos = mul(viewMat, pos);
    pos = mul(projMat, pos);
    
    
    //ピクセルシェーダーに渡す値
    vout.pos = pos;     //頂点位置
    vout.color = col;   //色（現在不使用）
    vout.uv = uv;       //UV

    return vout;
}


// ピクセルシェーダー (Pixel Shader)
// 各ピクセルの最終的な色を決定
float4 PSMain(VertexOut pin) : SV_TARGET        //ラスタライズされて情報が渡される
{


    float4 col = texArray[0].Sample(smp, pin.uv);   //t0に登録したテクスチャーから、s0に登録したサンプラー情報（smp）を使って、uvの位置の色情報を取得
    
    return col; //色情報を出力。この色がレンダーターゲットに描画される。
    
    /*
    //たとえば、return col;じゃなくて このように返すと、赤くなります
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
    */
}

