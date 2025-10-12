

/**
 * SolaGltfParser クラス
 * glTFファイルのロードと、SolaWGLのSolaMeshで使用する形式へのデータ変換を担当します。
 */
class SolaGltfParser {
    


    /**
     * @param {SolaWGL} wglHelper - SolaWGLのインスタンス
     */
    constructor(wglHelper) {

        
        this.gl = wglHelper.gl;
        this.wglHelper = wglHelper;

            

    }


    /**
     * glTFモデルをロードし、SolaMesh用のジオメトリデータを返します。
     * @param {string} gltfUrl - ロードする .gltf ファイルのURL
     * @returns {Promise<Array<object>>} - SolaMeshの生成に必要なデータオブジェクトの配列
     */

    async loadModel(gltfUrl) {

        console.log(`[SolaGltfParser.loadModel] ロード開始: ${gltfUrl}`);

        try {
            // glTFファイルのパスのベースを取得（.binファイルが相対パスの場合に対応）
            const basePath = gltfUrl.substring(0, gltfUrl.lastIndexOf('/') + 1);

            // gltf (JSON) のロード
            const gltfResponse = await fetch(gltfUrl);
            const gltf = await gltfResponse.json();
            console.log('[SolaGltfParser.loadModel] glTF JSONをロード・パース完了。バージョン:', gltf.asset.version);
            

            // バイナリデータのロード (通常はbuffers[0]を参照)
            const bufferInfo = gltf.buffers[0];
            const bufferUri = bufferInfo.uri;
            
            let arrayBuffer;

            if (bufferUri.startsWith('data:')) {
                // Data URI Scheme (Base64埋め込みデータ) の場合
                console.log('[SolaGltfParser.loadModel] Data URI を検出。fetchを使わず直接デコードします。');
                
                // Base64部分を抽出 (例: "data:application/octet-stream;base64,xxxxxxxx...")
                const base64Data = bufferUri.split(',')[1];
                
                // Base64文字列をバイナリデータ（ArrayBuffer）に変換
                arrayBuffer = this._base64ToArrayBuffer(base64Data);
                
                console.log(`[SolaGltfParser.loadModel] Data URI デコード完了。サイズ: ${arrayBuffer.byteLength} bytes`);

            } else {
                // 外部ファイル (.bin) の場合 (従来の処理)
                const bufferUrl = basePath + bufferUri;
                console.log(`[SolaGltfParser.loadModel] 外部バイナリデータURI: ${bufferUrl} (サイズ: ${bufferInfo.byteLength} bytes)`);

                const bufferResponse = await fetch(bufferUrl);
                arrayBuffer = await bufferResponse.arrayBuffer();
                
                console.log('[SolaGltfParser.loadModel] ArrayBufferロード完了 (外部ファイル)');
            }
            /*
            const bufferInfo = gltf.buffers[0];
            const bufferUrl = basePath + bufferInfo.uri;
            console.log(`[SolaGltfParser.loadModel]  バイナリデータURI: (サイズ: ${bufferInfo.byteLength} bytes)`);

            const bufferResponse = await fetch(bufferUrl);
            const arrayBuffer = await bufferResponse.arrayBuffer(); // ArrayBufferとして取得
            console.log('[SolaGltfParser.loadModel] ArrayBufferロード完了');
            */

            const meshDataList = this._parseMeshes(gltf, arrayBuffer);

            console.log(`[SolaGltfParser.loadModel] ロードとパース完了。メッシュ数: ${meshDataList.length}`);
            return meshDataList;

            } catch (error) {
            console.error('[SolaGltfParser.loadModel] ロード中にエラー発生:', error);
            throw error;
        }
    }


    /**
     * Base64文字列をArrayBufferにデコードするヘルパー関数
     * @param {string} base64 - Base64エンコードされた文字列
     * @returns {ArrayBuffer} - デコードされたバイナリデータ
     */
    _base64ToArrayBuffer(base64) {
        console.log('[SolaGltfParser._base64ToArrayBuffer] Base64デコード開始');
        
        // 1. ブラウザの atob() 関数を使ってBase64をデコードし、バイナリ文字列を得る
        const binaryString = atob(base64); 
        const len = binaryString.length;
        
        // 2. バイナリ文字列を格納するための符号なし8ビット整数配列 (Uint8Array) を作成
        const bytes = new Uint8Array(len);
        
        // 3. 文字列からUint8Arrayにデータをコピー (charCodeAtで文字コードを取得)
        for (let i = 0; i < len; i++) {
            bytes[i] = binaryString.charCodeAt(i);
        }
        
        console.log(`[SolaGltfParser._base64ToArrayBuffer] ✅ デコード完了。バイト数: ${len}`);
        // 4. ArrayBufferを返す (Uint8Arrayの基になるArrayBuffer)
        return bytes.buffer;
    }


    // glTFの型定義からTyped Arrayのコンストラクタを返すヘルパー
    _getTypedArray(componentType) {
        const gl = this.gl;
        switch (componentType) {
            case gl.FLOAT: return Float32Array;
            case gl.UNSIGNED_SHORT: return Uint16Array;
            case gl.UNSIGNED_BYTE: return Uint8Array;
            // 他の型（BYTE, SHORT, UNSIGNED_INTなど）も必要に応じて追加
            default: {
                console.error(`[SolaGltfParser._getTypedArray] 未対応の componentType: ${componentType}`);
                throw new Error(`Unsupported componentType: ${componentType}`);
            }
        }
    }


// glTFのtype (SCALAR, VEC2, VEC3, VEC4, MAT2, MAT3, MAT4) から成分数を返す
    _getComponentCount(type) {
        switch (type) {
            case 'SCALAR': return 1;
            case 'VEC2': return 2;
            case 'VEC3': return 3;
            case 'VEC4': return 4;
            case 'MAT2': return 4; // 2x2 行列
            case 'MAT3': return 9; // 3x3 行列
            case 'MAT4': return 16; // 4x4 行列
            default: {
                console.error(`[SolaGltfParser._getComponentCount] 未対応の type: ${type}`);
                throw new Error(`Unsupported type: ${type}`);
            }
        }
    }


/**
     * glTFのaccessor情報に基づき、ArrayBufferからTyped Arrayを抽出
     */
    _getTypedArrayFromAccessor(gltf, arrayBuffer, accessorIndex) {
        const accessor = gltf.accessors[accessorIndex];
        const bufferView = gltf.bufferViews[accessor.bufferView];

        const ComponentType = this._getTypedArray(accessor.componentType);
        const componentCount = this._getComponentCount(accessor.type);
        const elementSize = ComponentType.BYTES_PER_ELEMENT * componentCount;

        // byteOffsetは BufferView + Accessor のオフセットを合算
        const byteOffset = (bufferView.byteOffset || 0) + (accessor.byteOffset || 0);
        // byteLengthは要素数 * 要素のサイズ
        const byteLength = accessor.count * elementSize;
        
        //console.log(`[SolaGltfParser.Accessor] ${attributeName} (Index: ${accessorIndex}) - Type: ${accessor.type} (${componentCount}成分) x ${accessor.count}要素`);
        //console.log(`  └ BufferView: ${accessor.bufferView}, Bufferオフセット: ${byteOffset} bytes, 要素数: ${elementCount}`);

        // ArrayBuffer.slice() や ArrayBufferView のコンストラクタを使用して抽出
        return new ComponentType(arrayBuffer, byteOffset, accessor.count * componentCount);
    }

    /**
     * メッシュとプリミティブをパースし、SolaMesh用のデータを作成
     */
    _parseMeshes(gltf, arrayBuffer) {
        console.log('[SolaGltfParser._parseMeshes] メッシュパース開始');
        
        const meshDataList = [];
        
        // glTFは複数のメッシュ、メッシュは複数のプリミティブを持つ
        for (const mesh of gltf.meshes) {
            for (const primitive of mesh.primitives) {
                
                // 必須の属性 (POSITION) を取得
                const posAccessorIndex = primitive.attributes.POSITION;
                const positionData = this._getTypedArrayFromAccessor(gltf, arrayBuffer, posAccessorIndex);
                
                // オプションの属性 (NORMAL, TEXCOORD_0) を取得
                const normalData = primitive.attributes.NORMAL 
                    ? this._getTypedArrayFromAccessor(gltf, arrayBuffer, primitive.attributes.NORMAL)
                    : null;
                const uvData = primitive.attributes.TEXCOORD_0 
                    ? this._getTypedArrayFromAccessor(gltf, arrayBuffer, primitive.attributes.TEXCOORD_0)
                    : null;
                
                // インデックスを取得
                const indexAccessorIndex = primitive.indices;
                const indexData = this._getTypedArrayFromAccessor(gltf, arrayBuffer, indexAccessorIndex);
                
                // SolaMesh用のインターリーブ配列を作成
                const interleavedArray = this._createInterleavedArray(positionData, normalData, uvData); // (4)の処理

                meshDataList.push({
                    vertexData: interleavedArray,
                    indexData: indexData,
                    drawMode: primitive.mode || this.gl.TRIANGLES, // modeがなければTRIANGLES(4)
                });
            }
        }
        return meshDataList;
    }


/**
     * 抽出した属性データからSolaMesh用のインターリーブ配列を作成する
     */
    _createInterleavedArray(positionData, normalData, uvData) {
        console.log('[SolaGltfParser._createInterleavedArray] インターリーブ配列の作成開始');
        
        const vertexCount = positionData.length / 3; // glTFのPOSITIONはVEC3
        const STRIDE_FLOATS = 18; // SolaMesh.jsのSTRIDEから推測される値
        const interleavedArray = new Float32Array(vertexCount * STRIDE_FLOATS);

        for (let i = 0; i < vertexCount; i++) {
            let offset = i * STRIDE_FLOATS;
            
            // 1. POSITION (3成分を4成分に拡張: x, y, z, 1.0)
            interleavedArray[offset + 0] = positionData[i * 3 + 0];
            interleavedArray[offset + 1] = positionData[i * 3 + 1];
            interleavedArray[offset + 2] = positionData[i * 3 + 2];
            interleavedArray[offset + 3] = 1.0; // W成分

            // 2. UV (2成分: u, v)
            if (uvData) {
                interleavedArray[offset + 4] = uvData[i * 2 + 0];
                interleavedArray[offset + 5] = uvData[i * 2 + 1];
            } else {
                interleavedArray[offset + 4] = 0.0;
                interleavedArray[offset + 5] = 0.0;
            }

            // 3. NORMAL (3成分を4成分に拡張: x, y, z, 0.0)
            if (normalData) {
                interleavedArray[offset + 6] = normalData[i * 3 + 0];
                interleavedArray[offset + 7] = normalData[i * 3 + 1];
                interleavedArray[offset + 8] = normalData[i * 3 + 2];
                interleavedArray[offset + 9] = 0.0; // W成分
            } else {
                // デフォルト値を設定
                interleavedArray[offset + 6] = 0.0;
                interleavedArray[offset + 7] = 1.0;
                interleavedArray[offset + 8] = 0.0;
                interleavedArray[offset + 9] = 0.0;
            }

            // 4. TANGENT と COLOR は glTF に存在しない場合が多いので、ここではデフォルト値
            // TANGENT (4成分)
            interleavedArray[offset + 10] = 0.0;
            interleavedArray[offset + 11] = 0.0;
            interleavedArray[offset + 12] = 0.0;
            interleavedArray[offset + 13] = 1.0;
            
            // COLOR (4成分)
            interleavedArray[offset + 14] = 1.0;
            interleavedArray[offset + 15] = 1.0;
            interleavedArray[offset + 16] = 1.0;
            interleavedArray[offset + 17] = 1.0;
        }

        return interleavedArray;
    }




    onDestroy() {


    }



}

