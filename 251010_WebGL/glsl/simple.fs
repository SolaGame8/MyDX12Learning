#version 300 es 

precision mediump float;    //floatの精度

//varying vec2 v_texcoord;      // 頂点シェーダーからUV情報を受け取る
//uniform sampler2D u_sampler;    //テクスチャー

in vec2 v_texcoord;         // 頂点シェーダーからの入力 (varying -> in)
out vec4 fragColor;         // フラグメントシェーダーの出力先を宣言 (gl_FragColorの代替)

uniform sampler2D u_sampler;

void main() {
    
    //gl_FragColor = vec4(1, 0, 0, 1);  // 赤色で塗りつぶす

     //gl_FragColor = vec4(v_texcoord.x, v_texcoord.y, 0.0, 1.0);

    //gl_FragColor = texture2D(u_sampler, v_texcoord);   //0.0～1.0

    // gl_FragColor の代わりに fragColor を使用し、texture2D を texture に変更
    fragColor = texture(u_sampler, v_texcoord);

}

