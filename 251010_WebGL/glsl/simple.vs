#version 300 es 
// WebGL 2.0 / GLSL ES 3.00 を宣言

//attribute vec4 a_position;  //座標
//attribute vec2 a_texcoord;  //UV
//attribute vec4 a_normal;    //法線
//attribute vec4 a_boneID;    //ボーンID
//attribute vec4 a_boneWeight;//ボーンウエイト

layout(location = 0) in vec4 a_position;    //座標をロケーション0に固定
layout(location = 1) in vec2 a_texcoord;    //UVをロケーション1に固定
layout(location = 2) in vec4 a_normal;      //法線をロケーション2に固定
layout(location = 3) in vec4 a_boneID;      //ボーンIDをロケーション3に固定
layout(location = 4) in vec4 a_boneWeight;  //ボーンウエイトをロケーション4に固定



//カメラ、オブジェクト

uniform mat4 u_vpMatrix; //View x Projection 行列
uniform mat4 u_modelMatrix; //Model 行列



//varying vec2 v_texcoord;      // UV情報をフラグメントシェーダーに渡す

out vec2 v_texcoord; // v_texcoordを out に変更
out vec4 v_normal;

void main() {


    mat4 modelViewMatrix = u_vpMatrix * u_modelMatrix;

    // P * V * M * Pos の順
    //gl_Position = u_vpMatrix * u_modelMatrix * a_position;
    gl_Position = modelViewMatrix * a_position;




    // UV情報を出力
    v_texcoord = a_texcoord;
    v_normal = normalize((modelViewMatrix * a_normal).xyzw);


    // ダミー使用: コンパイラに変数を削除されないようにする
    float dummy = a_normal.x + a_boneID.x + a_boneWeight.x;
    if (dummy > 1000.0) { 
        gl_Position.y += dummy * 0.000001;; 
    }


}


