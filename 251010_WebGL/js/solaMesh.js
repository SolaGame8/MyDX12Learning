// solaMesh.js



class SolaMesh {

    
    /**
     * @param {WebGLRenderingContext} gl - WebGLコンテキスト
     * @param {object} geometry - 頂点データと描画に必要な情報
     * @param {number[]} geometry.vertices - 頂点位置 (x, y, z, ...)
     * @param {number[]} geometry.uvs - UV座標 (u, v, ...)
     * @param {number} geometry.vertexCount - 描画する頂点数
     * @param {WebGLTexture} texture - 使用するWebGLTextureオブジェクト
     */


    /**
     * @param {SolaWGL} wglHelper - SolaWGLのインスタンス
     */

    constructor(wglHelper) {

        this.gl = wglHelper.gl;


        // 姿勢データ（ワールドの位置、回転、スケール）
        this.position = vec3.fromValues(0, 0, 0);   // {x, y, z}
        this.rotation = vec3.fromValues(0, 0, 0);   // {roll (x), pitch (y), yaw (z)} (ラジアン)
        this.scale = vec3.fromValues(1, 1, 1);      // {x, y, z}

        // 内部で管理するジオメトリデータ

        this.STRIDE = (4 + 2 + 4 + 4 + 4) * Float32Array.BYTES_PER_ELEMENT;

        this.vertexData = [];
        this.indexData = [];
        
        this.indexDataUint16Array = null;

        this.bind_vertexData = null;
        this.bind_indexData = null;

        this.flg_buildMesh = false;

        this.textureKey = null;

        /*
        this._vertices = [];    //4
        this._uvs = [];         //2
        this._normals = [];     //4 法線データ
        this._boneIDs = [];     //4 ボーンID
        this._boneWeights = []; //4 ボーン影響値


        // 描画データ
        
        this.vertexCount = 0;
        this.vbo = null;
        this.uvbo = null;
        this.nbo = null;   // 法線バッファ
        this.bIdbo = null; // ボーンIDバッファ
        this.bWbo = null;  // ボーン影響値バッファ
        */



    }


    /**
     * 【新規】一つの頂点に関する全てのデータを構造体として追加する
     * @param {object} data - 頂点データ構造体
     * @param {number[]} data.position  [3] → [4]
     * @param {number[]} data.uv        [2]
     * @param {number[]} data.normal    [3] → [4]
     * @param {number[]} data.boneIDs   [4] 4つのボーンID  
     * @param {number[]} data.boneWeights [4] 4つのボーン影響値 (0.0〜1.0)
     */


    //データ生成

    addVertexData(data) {

        if (data.position !== undefined) {
            this.vertexData.push(data.position[0], data.position[1], data.position[2], 1.0);

        } else {
            this.vertexData.push(0.0, 0.0, 0.0, 1.0);
            //this.vertexData.push([0.0, 0.0, 0.0, 1.0]);
        }

        if (data.uv !== undefined) {
            this.vertexData.push(data.uv[0], data.uv[1]);

        } else {
            //this.vertexData.push([0.0, 0.0]);
            this.vertexData.push(0.0, 0.0);
        }

        if (data.normal !== undefined) {
            this.vertexData.push(data.normal[0], data.normal[1], data.normal[2], 0.0);

        } else {
            //this.vertexData.push([0.0, 0.0, 0.0, 0.0]);
            this.vertexData.push(0.0, 0.0, 0.0, 0.0);
        }

        if (data.boneIDs !== undefined) {
            this.vertexData.push(data.boneIDs[0], data.boneIDs[1], data.boneIDs[2], data.boneIDs[3]);

        } else {
            //this.vertexData.push([0.0, 0.0, 0.0, 0.0]);
            this.vertexData.push(0.0, 0.0, 0.0, 0.0);
        }

        if (data.boneWeights !== undefined) {
            this.vertexData.push(data.boneWeights[0], data.boneWeights[1], data.boneWeights[2], data.boneWeights[3]);

        } else {
            //this.vertexData.push([0.0, 0.0, 0.0, 0.0]);
            this.vertexData.push(0.0, 0.0, 0.0, 0.0);
        }


        
    }

    addIndexData(idx1, idx2, idx3) {

        this.indexData.push(idx1, idx2, idx3);

    }

    setMeshDataList(meshDataList) {

        //glTFから作ったデータリスト
        
        meshDataList.forEach((meshData, primIndex) => {

            const vertexData = meshData.vertexData;
            const indexDataTyped = meshData.indexData;
            const STRIDE_FLOATS = 18; 
            const totalVertices = vertexData.length / STRIDE_FLOATS;
            
            // インターリーブ配列を頂点ごとに分解し、addVertexDataで追加
            console.log(`[myApp] プリミティブ #${primIndex}: 頂点データ ${totalVertices} 個を addVertexData で追加中...`);

            for (let i = 0; i < totalVertices; i++) {

                const offset = i * STRIDE_FLOATS;
                
                // 頂点属性を格納するための一時変数 (配列リテラルを使わず、Arrayオブジェクトで初期化)
                let tempPosition = new Array(3);
                tempPosition[0] = vertexData[offset + 0];
                tempPosition[1] = vertexData[offset + 1];
                tempPosition[2] = vertexData[offset + 2];
                //3

                let tempUV = new Array(2);
                tempUV[0] = vertexData[offset + 4];
                tempUV[1] = vertexData[offset + 5];

                let tempNormal = new Array(3);
                tempNormal[0] = vertexData[offset + 6];
                tempNormal[1] = vertexData[offset + 7];
                tempNormal[2] = vertexData[offset + 8];
                //9

                let tempBoneId = new Array(4);
                tempBoneId[0] = vertexData[offset + 10];
                tempBoneId[1] = vertexData[offset + 11];
                tempBoneId[2] = vertexData[offset + 12];
                tempBoneId[2] = vertexData[offset + 13];
                
                let tempBoneWeight = new Array(4);
                tempBoneWeight[0] = vertexData[offset + 14];
                tempBoneWeight[1] = vertexData[offset + 15];
                tempBoneWeight[2] = vertexData[offset + 16];
                tempBoneWeight[2] = vertexData[offset + 17];


                // SolaMeshの addVertexData(data) に準拠したオブジェクトを作成し、呼び出す
                this.addVertexData({
                    position: tempPosition,
                    uv: tempUV,
                    normal: tempNormal,
                    boneIDs: tempBoneId,
                    boneWeights: tempBoneWeight
                });

                // デバッグ表示 (最初の50項目のみ)
                //if (i < 50) {
                //    console.log(`[Add #${i}] P:${tempPosition.map(n=>n.toFixed(3))} UV:${tempUV.map(n=>n.toFixed(3))} N:${tempNormal.map(n=>n.toFixed(3))}`);
                //}
            }
            
            // 3. インデックスデータを追加
            // addIndexData(idx1, idx2, idx3) に合わせて3つずつ渡します
            for (let i = 0; i < indexDataTyped.length; i += 3) {
                this.addIndexData(indexDataTyped[i], indexDataTyped[i + 1], indexDataTyped[i + 2]);
            }
            
        });




    }




    //バッファの作成    データバインディング

    buildMesh(wglHelper) {   //使えるような状態になる

        this.gl = wglHelper.gl;

        if (!this.gl) {
            console.error("WebGLコンテキストが初期化されていません。処理を中断します。");
            return;
        }

        const type = this.gl.ARRAY_BUFFER;
        const type_element = this.gl.ELEMENT_ARRAY_BUFFER;
        const usage = this.gl.STATIC_DRAW;

        //頂点バッファの生成
        this.bind_vertexData = this.gl.createBuffer();
        this.gl.bindBuffer(type, this.bind_vertexData);

        //データをバッファへ転送
        this.gl.bufferData(type, new Float32Array(this.vertexData), usage);
        this.gl.bindBuffer(type, null);

        //インデックスバッファの生成
        this.bind_indexData = this.gl.createBuffer();
        this.gl.bindBuffer(type_element, this.bind_indexData);

        this.indexDataUint16Array = new Uint16Array(this.indexData)

        //データをバッファへ転送
        this.gl.bufferData(type_element, this.indexDataUint16Array, usage);
        this.gl.bindBuffer(type_element, null);


        this.flg_buildMesh = true;

    }



    /*
        createBuffer(gl, data, type = gl.ARRAY_BUFFER, usage = gl.STATIC_DRAW) {

        const buffer = gl.createBuffer();
        gl.bindBuffer(type, buffer);
        // データはFloat32Arrayに変換して転送
        gl.bufferData(type, new Float32Array(data), usage);
        gl.bindBuffer(type, null);


        return buffer;
    }
    */

    /**
     * モデルの位置 (position) を設定します。
     * @param {number} x - X座標
     * @param {number} y - Y座標
     * @param {number} z - Z座標
     */
    setPosition(x, y, z) {
        vec3.set(this.position, x, y, z);
    }

    /**
     * モデルの回転 (rotation: ラジアン) を設定します。
     * @param {number} x - X軸 (ロール) 回転 (ラジアン)
     * @param {number} y - Y軸 (ピッチ) 回転 (ラジアン)
     * @param {number} z - Z軸 (ヨー) 回転 (ラジアン)
     */
    setRotation(x, y, z) {
        vec3.set(this.rotation, Math.PI * x / 180.0, Math.PI * y / 180.0, Math.PI * z / 180.0);
    }

    /**
     * モデルのスケール (scale) を設定します。
     * @param {number} x - X軸スケール
     * @param {number} y - Y軸スケール
     * @param {number} z - Z軸スケール
     */
    setScale(x, y, z) {
        vec3.set(this.scale, x, y, z);
    }


    setTextureKey(key) {
        this.textureKey = key;
    }

    /*
    // ----------------------------------------------------
    // 外部でバッファ作成するためのGetter (Normal, Skinning用を追加)
    // ----------------------------------------------------
    getVertices() { return this._vertices; }
    getUVs() { return this._uvs; }
    getNormals() { return this._normals; }
    getBoneIDs() { return this._boneIDs; }
    getBoneWeights() { return this._boneWeights; }
*/

    /**
     * 姿勢データ (位置、回転、スケール) から Model 行列 (mat4) を計算して返す。
     * 計算順序は通常、Scale -> Rotation -> Translation (TRS) です。
     * @returns {mat4} - 計算されたモデル行列
     */

    getModelMatrix() {
        // 1. mat4.create() で単位行列を初期化する
        const modelMatrix = mat4.create();

                // 4. 平行移動を適用 (T)
        // mat4.translate(out, a, v) : out = a * T
        mat4.translate(modelMatrix, modelMatrix, this.position);


        // 3. 回転を適用 (R) - XYZの順で適用
        // mat4.rotate*(out, a, rad) : out = a * R
        if (this.rotation[0] !== 0) { mat4.rotateX(modelMatrix, modelMatrix, this.rotation[0]); }
        if (this.rotation[1] !== 0) { mat4.rotateY(modelMatrix, modelMatrix, this.rotation[1]); }
        if (this.rotation[2] !== 0) { mat4.rotateZ(modelMatrix, modelMatrix, this.rotation[2]); }
        


        // 2. スケールを適用 (S)
        // mat4.scale(out, a, v) : out = a * S
        mat4.scale(modelMatrix, modelMatrix, this.scale);



        // 結果は M = T * R * S となります。
        return modelMatrix;
    }


    /**
     * WebGLバッファを作成・初期化するプライベートメソッド
    _initBuffer(data, target, usage) {
        const gl = this.gl;
        const buffer = gl.createBuffer();
        gl.bindBuffer(target, buffer);
        gl.bufferData(target, new Float32Array(data), usage);
        gl.bindBuffer(target, null);
        return buffer;
    }
     */

    /**
     * モデル行列（姿勢行列）を計算し、WebGLの描画に必要なバインディングを実行する。
     * 実際にgl.drawArraysを実行するのはglHelper側。
     * @param {SolaWGL} glHelper - SolaWGLのインスタンス (ロケーション情報などを取得するため)
     */








    draw(glHelper) {

        const gl = this.gl;

        
        //＜シェーダーに変数を渡す＞


        // 行列の計算
        /*
        const modelMatrix = mat4.create();
        mat4.scale(modelMatrix, modelMatrix, this.scale);
        if (this.rotation[2] !== 0) { mat4.rotateZ(modelMatrix, modelMatrix, this.rotation[2]); }
        if (this.rotation[1] !== 0) { mat4.rotateY(modelMatrix, modelMatrix, this.rotation[1]); }
        if (this.rotation[0] !== 0) { mat4.rotateX(modelMatrix, modelMatrix, this.rotation[0]); }
        mat4.translate(modelMatrix, modelMatrix, this.position);
        */



        




        
        //＜頂点情報＞

        if (this.flg_buildMesh) {   //ビルド完了してた場合


            //const STRIDE = 18 * Float32Array.BYTES_PER_ELEMENT; 

            //頂点バッファのバインド
            gl.bindBuffer(gl.ARRAY_BUFFER, this.bind_vertexData);

            let offset = 0;

            // a_position (頂点座標: 4要素)
            gl.vertexAttribPointer(glHelper.aPositionLocation, 4, gl.FLOAT, false, this.STRIDE, offset);
            gl.enableVertexAttribArray(glHelper.aPositionLocation);
            offset += 4 * Float32Array.BYTES_PER_ELEMENT;

            // a_texcoord (UV座標: 2要素)
            gl.vertexAttribPointer(glHelper.aTexcoordLocation, 2, gl.FLOAT, false, this.STRIDE, offset);
            gl.enableVertexAttribArray(glHelper.aTexcoordLocation);
            offset += 2 * Float32Array.BYTES_PER_ELEMENT;

            // a_normal (法線: 4要素 - 存在する場合のみバインド)
            gl.vertexAttribPointer(glHelper.aNormalLocation, 4, gl.FLOAT, false, this.STRIDE, offset);
            gl.enableVertexAttribArray(glHelper.aNormalLocation);
            offset += 4 * Float32Array.BYTES_PER_ELEMENT;

            // a_boneID (ボーンID: 4要素 - 存在する場合のみバインド)
            gl.vertexAttribPointer(glHelper.aBoneIDLocation, 4, gl.FLOAT, false, this.STRIDE, offset); 
            gl.enableVertexAttribArray(glHelper.aBoneIDLocation);
            offset += 4 * Float32Array.BYTES_PER_ELEMENT;

            // a_boneWeight (ボーン影響値: 4要素 - 存在する場合のみバインド)
            gl.vertexAttribPointer(glHelper.aBoneWeightLocation, 4, gl.FLOAT, false, this.STRIDE, offset);
            gl.enableVertexAttribArray(glHelper.aBoneWeightLocation);
            offset += 4 * Float32Array.BYTES_PER_ELEMENT;


            //インデックスバッファのバインド
            gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.bind_indexData);

            if(this.textureKey != null) {

                let modelTexture = glHelper.textureManager.getTexture(this.textureKey);

                gl.activeTexture(gl.TEXTURE0);
                gl.bindTexture(gl.TEXTURE_2D, modelTexture);
                // サンプラーユニフォームは、シェーダー切り替え時にロケーションを取得済み
                gl.uniform1i(glHelper.uSamplerLocation, 0);
            }

            //const indexCount = this.indexDataUint16Array.length;
            //console.log(`[Draw Check] Drawing ${indexCount} elements (triangles: ${indexCount / 3})`);


            //シェーダー変数
            const modelMatrix = this.getModelMatrix();  //モデルの姿勢行列
            gl.uniformMatrix4fv(glHelper.uModelMatrixLocation, false, modelMatrix);


            gl.drawElements(gl.TRIANGLES, this.indexDataUint16Array.length, this.gl.UNSIGNED_SHORT, 0); 
    
            // バインド解除
            gl.bindBuffer(gl.ARRAY_BUFFER, null);            // 頂点バッファをバインド解除
            gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, null);   // インデックスバッファをバインド解除   




            //gl.flush();
            //gl.finish();

        }

 









        //ーーーーーーーーーーーーーーーーーー保留

        /*
        // テクスチャのバインド
        if (this.texture) {
            gl.activeTexture(gl.TEXTURE0);
            gl.bindTexture(gl.TEXTURE_2D, this.texture);
            gl.uniform1i(glHelper.uSamplerLocation, 0); 
        }

        // 頂点属性バッファのバインド
        // a_position (頂点座標)
        gl.bindBuffer(gl.ARRAY_BUFFER, this.vbo);
        // 頂点座標は3要素 (X, Y, Z)
        gl.vertexAttribPointer(glHelper.aPositionLocation, 3, gl.FLOAT, false, 0, 0);
        gl.enableVertexAttribArray(glHelper.aPositionLocation);

        // a_texcoord (UV座標)
        gl.bindBuffer(gl.ARRAY_BUFFER, this.uvbo);
        // UV座標は2要素 (U, V)
        gl.vertexAttribPointer(glHelper.aTexcoordLocation, 2, gl.FLOAT, false, 0, 0);
        gl.enableVertexAttribArray(glHelper.aTexcoordLocation);
*/

    }

    onDestroy() {



    }


}




