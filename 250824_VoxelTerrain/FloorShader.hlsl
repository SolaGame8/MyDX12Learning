
cbuffer MyConstants : register(b0)
{
    float4 shaderParam[8]; // float4を8つ格納
    
    matrix worldMat;
    matrix viewMat;
    matrix projMat;
    
    /*
    //たとえば太陽光などを計算したい場合はこのように、太陽の向きのベクトルを受け取ったりします
    float4 lightDirection;
    */
    
};

Texture2D texArray[3] : register(t0); // 複数のテクスチャを配列として定義   //0 chara, 1 map
SamplerState smp : register(s0);        //サンプラー（画像の色を取得する）

// シェーダーの入力構造体
struct VertexIn
{
    float4 pos : POSITION; // 頂点の位置情報
    float4 color : COLOR; // 頂点の色情報
    float4 uv : TEXCOORD; // UV座標情報
    /*
    //たとえば、ライトの処理したい場合は、法線情報をこんな感じで受け取ったりします
    //float4 normal : NORMAL;   //面の向きのベクトル
    //＊このように使用したい場合は、渡す情報や描画ルールもすべて変更します
    */
    
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

    
    float4 pos = vin.pos;
    
    
    //カメラの変換行列
    pos = mul(worldMat, pos);
    pos = mul(viewMat, pos);
    pos = mul(projMat, pos);
    
    
    
    vout.pos = pos;
    vout.color = vin.color;
    vout.uv = vin.uv.xy;

    return vout;
}

// ピクセルシェーダー (Pixel Shader)
// 各ピクセルの最終的な色を決定
float4 PSMain(VertexOut pin) : SV_TARGET        //ラスタライズされて情報が渡される
{
    // 頂点シェーダーから補間されて渡された色をそのまま出力
    
    float4 col = texArray[1].Sample(smp, pin.uv.xy);
    
    /*
    //たとえば、ここで太陽光などを計算したい場合はこんな感じにします
    
    float3 n_lightDirection = normalize(lightDirection.xyz); //太陽のベクトルをノーマライズ（長さ1のベクトルにする）　もともとノーマライズして渡した方がいいです
    float3 n_normal = normalize(pin.normal.xyz);      //これも最初からノーマライズした情報を作った方がいいです
    
    float lightDotValue = dot(n_lightDirection.xyz, n_normal.xyz); //内積を計算 //.xyzは不要ですがイメージしやすいかもなので書いています
    
    //このlightDotValueの結果は、1.0 ～ -1.0になります
    
    //内積は、２つのベクトルが、同じ方向を向いている時が、1.0
    //90度みたいな時は、0.0
    //逆方向を向いている時は、-1.0
    
    //なので、-1.0の時が、太陽が向いている方向に、向き合っている面です（お互い逆方向）
    
    //こんな感じで色を変化させられます
    
    float lightStrings = 0.8f;    //ライトの強さ  これらの値も変数情報として受け取った方が使い勝手が良いです
    float ambientLightVal = 0.2f; //環境の明るさ（これを0.0fにすると、影が真っ黒になります）
    
    float l_dot = saturate(-lightDotValue);    //saturateは、指定した値を 0 から 1 の範囲内でクランプします
    col = col * l_dot * lightStrings + col * ambientLightVal;
    
    */
    
    
    return col;
        

}

