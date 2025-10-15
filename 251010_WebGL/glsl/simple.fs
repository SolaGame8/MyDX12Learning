#version 300 es 

precision mediump float;    //floatの精度


in vec2 v_texcoord;         // 頂点シェーダーからの入力 (varying -> in)
in vec4 v_normal;


out vec4 fragColor;         // フラグメントシェーダーの出力先を宣言 (gl_FragColorの代替)

uniform sampler2D u_sampler;    //テクスチャー


uniform vec4 u_genericArray[16];    //汎用のパラメータ

// 太陽のパラメータをJavaScript側から受け取るユニフォーム変数
uniform vec3 u_lightDirection;    // 光の方向 (既に正規化されていることを想定)
uniform vec3 u_lightColor;        // 光の色
uniform float u_lightIntensity;   // 光の強度

// アンビエントのパラメータをJavaScript側から受け取るユニフォーム変数
uniform vec3 u_ambientColor;      // 環境光の色
uniform float u_ambientIntensity; // 環境光の強度

    



void main() {
    
    

    vec4 baseColor = texture(u_sampler, v_texcoord);

    vec3 N = normalize(v_normal.xyz); // 法線ベクトル
    vec3 L = normalize(u_lightDirection); // 光源ベクトル

    float diffuseFactor = max(dot(N, L), 0.0);

    vec3 diffuseLight = normalize(u_lightColor) * u_lightIntensity * diffuseFactor;
    
    vec3 ambientLight = normalize(u_ambientColor) * u_ambientIntensity;

    // ディフューズ光 + アンビエント光
    vec3 totalLight = diffuseLight + ambientLight;
    
    // ベースカラー (RGB) に光のカラーを乗算
    vec3 finalColorRGB = baseColor.rgb * totalLight;

    // fragColorに出力
    fragColor = vec4(finalColorRGB, baseColor.a);



}

