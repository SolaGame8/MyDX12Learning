

precision mediump float;

varying vec2 v_texcoord;      // 頂点シェーダーからUV情報を受け取る
uniform sampler2D u_sampler;


void main() {
    
    //gl_FragColor = vec4(1, 0, 0, 1);  // 赤色で塗りつぶす

     //gl_FragColor = vec4(v_texcoord.x, v_texcoord.y, 0.0, 1.0);

     gl_FragColor = texture2D(u_sampler, v_texcoord);
}

