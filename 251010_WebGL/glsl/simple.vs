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

uniform vec4 u_genericArray[16]; 

// ボーン行列の配列 (最大128個)
// この配列には、フレームごとの「最終変換行列 (JointModelMatrix * InverseBindMatrix)」を格納します。
const int MAX_BONES = 128; 
uniform mat4 u_boneMatrices[MAX_BONES]; 


out vec2 v_texcoord; // v_texcoordを out に変更
out vec4 v_normal;

void main() {

    //ボーンの計算
    mat4 skinningMatrix = mat4(1.0);    //変形しない正規行列

    const float boneMax = 128.0;

    if (a_boneID.x > 0.0 && a_boneID.x - 1.0 < boneMax) { //ボーンの影響（ボーンIDの情報）がある場合

    
        // 頂点に影響を与える最大4つのボーンの最終行列を加重平均
        skinningMatrix = a_boneWeight.x * u_boneMatrices[int(a_boneID.x - 1.0)];

        if (a_boneID.y > 0.0 && a_boneID.y - 1.0 < boneMax) {
            skinningMatrix +=  a_boneWeight.y * u_boneMatrices[int(a_boneID.y - 1.0)];
        }
        if (a_boneID.z > 0.0 && a_boneID.z - 1.0 < boneMax) {
            skinningMatrix +=  a_boneWeight.z * u_boneMatrices[int(a_boneID.z - 1.0)];
        }
        if (a_boneID.w > 0.0 && a_boneID.w - 1.0 < boneMax) {
            skinningMatrix +=  a_boneWeight.w * u_boneMatrices[int(a_boneID.w - 1.0)];
        }
    

    }

    // 頂点位置をボーンの影響によって変更
    vec4 skinnedPosition = skinningMatrix * a_position;
    
    // 法線ベクトルをボーンの影響によって変更
    vec4 skinnedNormal = skinningMatrix * a_normal;

    mat4 modelViewMatrix = u_vpMatrix * u_modelMatrix;
    gl_Position = modelViewMatrix * skinnedPosition;




    // UV情報を出力
    v_texcoord = a_texcoord;
    v_normal = normalize(modelViewMatrix * skinnedNormal); //モデル行列の影響も与える




}


