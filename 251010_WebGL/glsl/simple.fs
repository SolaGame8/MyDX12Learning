#version 300 es 

precision mediump float;    //floatの精度

//varying vec2 v_texcoord;      // 頂点シェーダーからUV情報を受け取る
//uniform sampler2D u_sampler;    //テクスチャー

in vec2 v_texcoord;         // 頂点シェーダーからの入力 (varying -> in)
in vec4 v_normal;


out vec4 fragColor;         // フラグメントシェーダーの出力先を宣言 (gl_FragColorの代替)

uniform sampler2D u_sampler;


    // 太陽
    const vec3 c_lightDirection = normalize(vec3(-0.5, 1.0, 0.5));
    const vec3 c_lightColor = vec3(1.0, 1.0, 0.8); 
    const float c_lightIntensity = 1.5;    

    // アンビエント
    const vec3 c_ambientColor = vec3(0.5, 0.5, 0.8); 
    const float c_ambientIntensity = 0.5; 


void main() {
    
    

    vec4 baseColor = texture(u_sampler, v_texcoord);

    vec3 N = normalize(v_normal.xyz); // 法線ベクトル
    vec3 L = normalize(c_lightDirection); // 光源ベクトル

    float diffuseFactor = max(dot(N, L), 0.0);

    vec3 diffuseLight = normalize(c_lightColor) * c_lightIntensity * diffuseFactor;
    
    vec3 ambientLight = normalize(c_ambientColor) * c_ambientIntensity;

    // ディフューズ光 + アンビエント光
    vec3 totalLight = diffuseLight + ambientLight;
    
    // ベースカラー (RGB) に光のカラーを乗算
    vec3 finalColorRGB = baseColor.rgb * totalLight;

    // fragColorに出力
    fragColor = vec4(finalColorRGB, baseColor.a);


    //fragColor = texture(u_sampler, v_texcoord);

}

