attribute vec4 a_position;
attribute vec2 a_texcoord;

uniform vec2 u_translation;
uniform float u_scale;

varying vec2 v_texcoord;      // UV情報をフラグメントシェーダーに渡す

void main() {

    // 頂点位置をスケーリングと移動で変換
    gl_Position = vec4(a_position.xy * u_scale + u_translation, 0.0, 1.0);

    // UV情報を出力
    v_texcoord = a_texcoord;

}


