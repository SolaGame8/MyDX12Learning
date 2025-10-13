

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

        // ロードしたモデルデータをキャッシュするMap
        this._modelCache = new Map(); 

        // 最後にロードしたglTFファイルのURL
        this._lastLoadedUrl = null; 

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


            // ボーンツリー、アニメーションなどの追加情報のパース (Placeholder)
            const nodeData = this._parseNodes(gltf); // ノードツリー
            const skinData = this._parseSkins(gltf, arrayBuffer); // スキン/ボーン情報
            const animationData = this._parseAnimations(gltf, arrayBuffer); // アニメーション情報
            

            // アニメーションデータ処理とベイク処理
            const bakedAnimationData = this._processAnimationData(gltf, skinData, animationData, nodeData);





            // ここで、パース済みの全データをキャッシュに保存する
            const cachedData = {
                gltf: gltf, // glTF JSON全体
                arrayBuffer: arrayBuffer, // バイナリデータ全体
                meshDataList: meshDataList, // SolaMesh用ジオメトリ
                nodeData: nodeData,
                skinData: skinData,
                animationData: animationData,
                bakedAnimationData: bakedAnimationData,
            };
            this._modelCache.set(gltfUrl, cachedData);

            this._lastLoadedUrl = gltfUrl; 

            console.log(`[SolaGltfParser.loadModel] ロードとパース完了。メッシュ数: ${meshDataList.length}`);
            return meshDataList;

            } catch (error) {
            console.error('[SolaGltfParser.loadModel] ロード中にエラー発生:', error);
            throw error;
        }
    }






    // キャッシュからデータを取得
    getModelData() {

        const gltfUrl = this._lastLoadedUrl;

        if (!gltfUrl) {
            console.warn('[SolaGltfParser.getModelData] 取得対象のURLがありません。loadModelが正常に完了していません。');
            return undefined;
        }

        const data = this._modelCache.get(gltfUrl);
        
        if (!data) {
            console.warn(`[SolaGltfParser.getModelData] ${gltfUrl} のデータはキャッシュにありません。`);
        }

        return data;
    }
    
    // キャッシュからデータを削除
    removeModelData() {
        
        const gltfUrl = this._lastLoadedUrl;

        if (!gltfUrl) {
            console.warn('[SolaGltfParser.removeModelData] 削除対象のURLがありません。loadModelが正常に完了していません。');
            return false;
        }
        if (this._modelCache.has(gltfUrl)) {
            const success = this._modelCache.delete(gltfUrl);
            console.log(`[SolaGltfParser.removeModelData] ${gltfUrl} のキャッシュを削除しました: ${success}`);
            return success;
        }
        console.warn(`[SolaGltfParser.removeModelData] ${gltfUrl} のデータはキャッシュにありません。`);
        return false;
    }



    /* * glTFのNodeツリーをパースするPlaceholder
    * ノードの親子関係、ローカルトランスフォーム、ボーンインデックスを抽出します
    */
   
    _parseNodes(gltf) {

        if (!gltf.nodes) return [];
        console.log('[SolaGltfParser._parseNodes] ノード情報パース開始');

        // ノードの配列を、必要な情報を持つオブジェクトにマッピング
        const nodeData = gltf.nodes.map(node => {
            const nodeInfo = {
                name: node.name,
                children: node.children || [], // 子ノードのインデックス配列
                matrix: node.matrix, // 4x4トランスフォーム行列
                translation: node.translation,
                rotation: node.rotation, // クォータニオン [x, y, z, w]
                scale: node.scale,
                skin: node.skin, // このノードに適用される skin のインデックス
            };
            return nodeInfo;
        });


        /*
        // パース後にコンソール出力を行う
        console.log('[SolaGltfParser._parseNodes] パース結果 (最大10件):');
        nodeData.slice(0, 10).forEach((node, index) => {
            console.log(`  [${index}] Name: ${node.name || 'N/A'}, Children Count: ${node.children.length}, Has Matrix: ${!!node.matrix}`);
        });
        */

        // ノードツリーの構築（親子関係の解決）は、このデータ構造を使用して外部で行うか、
        // ここで Node オブジェクトのインスタンス化とツリー構造の構築ロジックを追加する必要があります。

        return nodeData;
    }

    /* * glTFのSkins情報をパースするPlaceholder
    * Inverse Bind Matrices (IBM) とジョイントインデックスを抽出します
    */

    _parseSkins(gltf, arrayBuffer) {

        if (!gltf.skins) return [];
        console.log('🚨[SolaGltfParser._parseSkins] スキン情報パース開始');

        // 1. skinDataの生成（既存ロジック）
        const skinData = gltf.skins.map(skin => {
            let inverseBindMatrices = null;

            if (skin.inverseBindMatrices !== undefined) {
                try {
                    inverseBindMatrices = this._getTypedArrayFromAccessor(gltf, arrayBuffer, skin.inverseBindMatrices);
                } catch (e) {
                    console.error(`[SolaGltfParser._parseSkins] Inverse Bind Matricesの抽出に失敗: ${e.message}`);
                }
            }
            
            return {
                name: skin.name,
                joints: skin.joints, 
                inverseBindMatrices: inverseBindMatrices, 
            };
        });

        // 2. 🚨 【追加】すべてのボーン名（ノード名）の抽出と出力
        const uniqueJointNames = new Set();
        const allJointIndices = new Set();

        // 全てのスキンからジョイントのノードインデックスを収集
        skinData.forEach(skin => {
            if (skin.joints) {
                skin.joints.forEach(nodeIndex => {
                    allJointIndices.add(nodeIndex);
                });
            }
        });

        // ノードインデックスからノード名を取得
        allJointIndices.forEach(nodeIndex => {
            const node = gltf.nodes[nodeIndex];
            // ノード名がない場合は、インデックスを使って識別名を生成
            const boneName = node ? node.name || `Node ${nodeIndex} (No Name)` : `Node ${nodeIndex} (Not Found)`;
            uniqueJointNames.add(boneName);
        });

        // 3. ボーン名のリストを出力
        console.log('--- ボーン名のリスト (ユニークなジョイントノード名) ---');
        if (uniqueJointNames.size === 0) {
            console.log('  ボーン情報（スキン）が見つかりませんでした。');
        } else {
            let count = 0;
            const limit = 100; // ログが長くなりすぎるのを防ぐため、最大100件に制限
            uniqueJointNames.forEach(name => {
                if (count < limit) {
                    console.log(`🚨 bone  - ${name}`);
                }
                count++;
            });
            
            if (count > limit) {
                console.log(`  ...(残り ${count - limit} 件のボーン名が省略されました)`);
            }
        }
        console.log(`------------------------------------------- (合計 ${uniqueJointNames.size} 件)`);

        // 4. 既存のスキンパース結果概要の出力（最大10件）
        console.log('[SolaGltfParser._parseSkins] スキンパース結果概要 (最大10件):');
        skinData.slice(0, 10).forEach((skin, index) => {
            console.log(`  [${index}] Name: ${skin.name || 'N/A'}, Joints Count: ${skin.joints.length}, IBM Size: ${skin.inverseBindMatrices ? skin.inverseBindMatrices.byteLength : 0} bytes`);
        });

        return skinData;





    }

    /* * glTFのAnimations情報をパースするPlaceholder
    * アニメーションカーブのデータ（時間と値）を抽出します
    */

    _parseAnimations(gltf, arrayBuffer) {
        
        
        if (!gltf.animations) return [];
    console.log('[SolaGltfParser._parseAnimations] アニメーション情報パース開始');

    const animationData = gltf.animations.map(animation => {
        
        const samplers = animation.samplers.map(sampler => {
            const input = this._getTypedArrayFromAccessor(gltf, arrayBuffer, sampler.input);
            const output = this._getTypedArrayFromAccessor(gltf, arrayBuffer, sampler.output);
            
            return {
                interpolation: sampler.interpolation || 'LINEAR', 
                input: input, 
                output: output 
            };
        });

        const channels = animation.channels.map(channel => ({
            samplerIndex: channel.sampler, 
            targetNodeIndex: channel.target.node, 
            targetPath: channel.target.path, 
        }));
        
        // 🚨 【変更】最大キーフレーム数を計算
        let maxKeyframeCount = 0;
        for (const sampler of samplers) {
            // キーフレーム数は input 配列の長さ
            const keyframeCount = sampler.input.length; 
            if (keyframeCount > maxKeyframeCount) {
                maxKeyframeCount = keyframeCount;
            }
        }

        return {
            name: animation.name,
            channels: channels, 
            samplers: samplers, 
            maxKeyframeCount: maxKeyframeCount, // 🚨 【変更】最大キーフレーム数を格納
        };
    });

    // 🚨 【修正】ロギングに maxKeyframeCount を含める
    console.log('🚨[SolaGltfParser._parseAnimations] パース結果 (最大10件):');
    
    console.log('--- アニメーション名のリスト (最大10件) ---');
    animationData.slice(0, 10).forEach((anim, index) => {
        const name = anim.name || `Animation ${index}`;
        
        // Max Keyframesを出力
        console.log(`🚨  [${index}] Name: ${name} (Channels: ${anim.channels.length}, Max Keyframes: ${anim.maxKeyframeCount})`);
    });
    console.log('-------------------------------------------');
    
    return animationData;
    }



    /**
     * glTFのキーフレームデータを読み取り、フレームごとの最終ジョイント行列をベイクします。
     * @returns {object} ベイク済みアニメーションデータを含むオブジェクト
     */
    _processAnimationData(gltf, skinData, animationData, nodeData) {
        if (skinData.length === 0 || animationData.length === 0) {
            return { 
                bakedAnimations: new Map(), // animationMatrixArray を格納
                inverseMatrixArray: null,    // inverseMatrix を格納
                boneIndices: [] 
            };
        }

        // ----------------------------------------------------
        // 1. inverseMatrixArray の取得 (ボーンの姿勢逆行列)
        // ----------------------------------------------------
        // 最初のスキン（通常、モデルは1つのスキンを持つ）のデータを使用
        const firstSkin = skinData[0]; 
        const boneIndices = firstSkin.joints; // ボーンとして使われるノードインデックスのリスト
        const numBones = boneIndices.length;

        // glTFから抽出したIBMデータ。これはすでに Float32Array であり、そのまま使用できます。
        const inverseMatrixArray = firstSkin.inverseBindMatrices; 
        
        if (!inverseMatrixArray) {
            console.warn('Inverse Bind Matrices (IBM) データが見つかりませんでした。スキニングは実行できません。');
            return { bakedAnimations: new Map(), inverseMatrixArray: null, boneIndices: [] };
        }


        // ----------------------------------------------------
        // 2. animationMatrixArray のベイク処理
        // ----------------------------------------------------
        const bakedAnimations = new Map();
        const FRAME_RATE = 30.0; // 🚨 フレームレートを仮に30 FPSと設定

        for (const anim of animationData) {
            const animationName = anim.name || `Animation ${bakedAnimations.size}`;
            const maxKeyframeCount = anim.maxKeyframeCount; // _parseAnimationsで計算済み (例: 30)
            
            // 全フレーム * 全ボーン * 16成分 (mat4) のための Float32Array を確保
            const animationMatrixArray = new Float32Array(maxKeyframeCount * numBones * 16);

            // フレームごとのベイク処理
            for (let frame = 0; frame < maxKeyframeCount; frame++) {
                const time = frame / FRAME_RATE; // 現在のフレームに対応する時間（秒）

                // 🚨 【プレースホルダー】現在の時間におけるノードのローカル行列を計算するためのツリー走査
                // この関数内で、現在のフレームの T/R/S 値に基づき、モデル内の全ノードのワールド行列 (JointModelMatrix) を計算する
                const nodeModelMatrices = this._calculateNodeModelMatrices(
                    gltf, anim, nodeData, time
                );

                // 各ボーンの最終行列を計算 (FinalJointMatrix = JointModelMatrix * IBM)
                for (let boneID = 0; boneID < numBones; boneID++) {
                    const nodeIndex = boneIndices[boneID];
                    const boneModelMatrix = nodeModelMatrices.get(nodeIndex); // JointModelMatrix
                    
                    // IBM (Inverse Bind Matrix) を取得
                    const ibm = inverseMatrixArray.subarray(boneID * 16, (boneID + 1) * 16);

                    // 🚨 【プレースホルダー】最終行列を計算
                    // ここで外部の math ライブラリを使用して行列乗算を行う
                    const finalJointMatrix = this._calculateFinalJointMatrix(
                        boneModelMatrix, ibm
                    );
                    
                    // 結果を最終的な配列に格納 (16 floats)
                    const matrixStartOffset = (frame * numBones * 16) + (boneID * 16);
                    animationMatrixArray.set(finalJointMatrix, matrixStartOffset);
                }
            } // End frame loop

            // アニメーション名をキーとしてベイク済みデータを Map に格納
            bakedAnimations.set(animationName, {
                animationMatrixArray: animationMatrixArray,
                maxKeyframeCount: maxKeyframeCount
            });
            console.log(`[SolaGltfParser._processAnimationData] ${animationName} を ${maxKeyframeCount} フレームでベイク完了。`);
        } // End animation loop

        return {
            bakedAnimations: bakedAnimations,
            inverseMatrixArray: inverseMatrixArray,
            boneIndices: boneIndices
        };
    }


        /**
     * 🚨 【実装】指定時間における全ノードのモデル空間行列（JointModelMatrix）を計算します。
     * ノードツリーを走査し、アニメーションを適用して行列を累積します。
     * * @param {object} gltf - glTF JSON データ
     * @param {object} anim - 現在処理中のアニメーションデータ (bakedAnimationData.animationDataより)
     * @param {Array<object>} nodeData - パース済みのノード情報
     * @param {number} time - 現在のアニメーション時間（秒）
     * @returns {Map<number, Float32Array>} ノードインデックス -> JointModelMatrix の Map
     */
    _calculateNodeModelMatrices(gltf, anim, nodeData, time) {
        const nodeModelMatrices = new Map();
        const scene = gltf.scenes[gltf.scene || 0]; // デフォルトシーンを取得
        
        // ----------------------------------------------------
        // 1. ツリー走査と行列累積の開始
        // ----------------------------------------------------
        if (scene.nodes) {
            scene.nodes.forEach(rootNodeIndex => {
                // ルートノードは単位行列（または、自身のローカル行列）から累積を開始
                const identityMatrix = mat4.create(); // 単位行列
                this._traverseNode(
                    gltf, anim, nodeData, time, rootNodeIndex, identityMatrix, nodeModelMatrices
                );
            });
        }

        return nodeModelMatrices;
    }

    /**
     * ノードツリーを再帰的に走査し、モデル行列を計算・累積します。
     */
    _traverseNode(gltf, anim, nodeData, time, nodeIndex, parentModelMatrix, nodeModelMatrices) {
        const node = gltf.nodes[nodeIndex];
        const localMatrix = mat4.create(); // このノードのローカル行列
        const currentModelMatrix = mat4.create(); // 計算中のモデル行列
        
        // ----------------------------------------------------
        // 2. ローカル変換行列の計算（アニメーション適用済み）
        // ----------------------------------------------------
        
        // アニメーションデータからT/R/S値を取得
        const { translation, rotation, scale, hasAnim } = this._getAnimatedNodeTransform(gltf, anim, nodeIndex, time);

        if (node.matrix) {
            // ノードに matrix が定義されている場合、通常はアニメーションで上書きしない
            mat4.copy(localMatrix, node.matrix);
        } else {
            // T/R/S からローカル行列を生成
            
            // アニメーションが適用されているか、ノード自身がT/R/Sを持っている場合
            if (hasAnim || node.translation || node.rotation || node.scale) {
                mat4.fromRotationTranslationScale(
                    localMatrix,
                    rotation,
                    translation,
                    scale
                );
            } else {
                // T/R/S/matrix の定義がない場合、単位行列
                mat4.identity(localMatrix);
            }
        }
        
        // ----------------------------------------------------
        // 3. 親の行列を累積 (ParentModelMatrix * LocalMatrix)
        // ----------------------------------------------------
        // currentModelMatrix = parentModelMatrix * localMatrix
        mat4.multiply(currentModelMatrix, parentModelMatrix, localMatrix);
        
        // 最終的なモデル行列を Map に格納
        nodeModelMatrices.set(nodeIndex, currentModelMatrix);
        
        // ----------------------------------------------------
        // 4. 子ノードの走査
        // ----------------------------------------------------
        if (node.children) {
            node.children.forEach(childIndex => {
                this._traverseNode(
                    gltf, anim, nodeData, time, childIndex, currentModelMatrix, nodeModelMatrices
                );
            });
        }
    }

    /**
     * 🚨 【実装必須】現在の時間におけるノードのT/R/S値を、アニメーションサンプラーから補間して取得します。
     * (ここでは線形補間のみを仮定)
     * @returns {{translation: Float32Array, rotation: Float32Array, scale: Float32Array, hasAnim: boolean}}
     */
    _getAnimatedNodeTransform(gltf, anim, nodeIndex, time) {
        const node = gltf.nodes[nodeIndex];
        
        // ノードのデフォルト値
        const defaultTranslation = node.translation ? vec3.clone(node.translation) : vec3.fromValues(0, 0, 0);
        const defaultRotation = node.rotation ? quat.clone(node.rotation) : quat.fromValues(0, 0, 0, 1);
        const defaultScale = node.scale ? vec3.clone(node.scale) : vec3.fromValues(1, 1, 1);
        
        let translation = defaultTranslation;
        let rotation = defaultRotation;
        let scale = defaultScale;
        let hasAnim = false;
        
        // アニメーションチャンネルを検索し、補間を実行
        for (const channel of anim.channels) {
            if (channel.targetNodeIndex === nodeIndex) {
                const sampler = anim.samplers[channel.samplerIndex];
                const inputTimes = sampler.input; // 時間キー
                const outputValues = sampler.output; // 値
                
                // 補間処理
                // 1. time に対応する前後のキーフレームインデックス (k0, k1) を見つける
                // 2. 補間係数 t を計算 (t = (time - inputTimes[k0]) / (inputTimes[k1] - inputTimes[k0]))
                // 3. outputValues[k0] と outputValues[k1] の間で補間を実行 (lerp, slerp)
                
                // 💡 補間ロジックのプレースホルダー
                // 簡略化のため、ここでは最初のキーフレームの値（静止ポーズ）を使用、または補間を実行しない
                const k0 = 0; // 最初のキーフレームインデックス
                
                if (k0 < inputTimes.length) {
                    hasAnim = true;
                    if (channel.targetPath === 'translation') {
                        vec3.set(translation, outputValues[k0 * 3 + 0], outputValues[k0 * 3 + 1], outputValues[k0 * 3 + 2]);
                        // 🚨 実際には: vec3.lerp(translation, outputValues[k0*3], outputValues[k1*3], t);
                    } else if (channel.targetPath === 'rotation') {
                        quat.set(rotation, outputValues[k0 * 4 + 0], outputValues[k0 * 4 + 1], outputValues[k0 * 4 + 2], outputValues[k0 * 4 + 3]);
                        // 🚨 実際には: quat.slerp(rotation, outputValues[k0*4], outputValues[k1*4], t);
                    } else if (channel.targetPath === 'scale') {
                        vec3.set(scale, outputValues[k0 * 3 + 0], outputValues[k0 * 3 + 1], outputValues[k0 * 3 + 2]);
                        // 🚨 実際には: vec3.lerp(scale, outputValues[k0*3], outputValues[k1*3], t);
                    }
                }
            }
        }
        
        return { translation, rotation, scale, hasAnim };
    }



    /**
     * 🚨 【実装済み】JointModelMatrix と InverseBindMatrix を乗算して FinalJointMatrix を計算します。
     * FinalJointMatrix = JointModelMatrix * InverseBindMatrix
     * @param {Float32Array} jointModelMatrix - そのフレームにおけるボーンのモデル空間行列
     * @param {Float32Array} ibm - Inverse Bind Matrix
     * @returns {Float32Array} FinalJointMatrix (16成分)
     */
    _calculateFinalJointMatrix(jointModelMatrix, ibm) {
        // 戻り値となる結果行列用の Float32Array (16成分) を gl-matrix で確保
        const finalJointMatrix = mat4.create(); 

        // JointModelMatrix に InverseBindMatrix を乗算し、結果を finalJointMatrix に格納
        // mat4.multiply(out, a, b) は a * b の順序で乗算されます。
        mat4.multiply(finalJointMatrix, jointModelMatrix, ibm);

        return finalJointMatrix; 
    }


    /**
     * 指定されたアニメーションキーに対応する、フレームごとの最終ジョイント行列の配列を取得します。
     * @param {string} animeKey - 取得したいアニメーションの名前
     * @returns {Float32Array|null} - ベイク済みジョイント行列の配列、またはデータがない場合は null
     */
    getAnimationMatrixArray(animeKey) {
        const gltfUrl = this._lastLoadedUrl;
        if (!gltfUrl || !this._modelCache.has(gltfUrl)) {
            console.warn('[getAnimationMatrixArray] モデルがロードされていません。');
            return null;
        }

        const cachedData = this._modelCache.get(gltfUrl);
        const bakedAnimations = cachedData.bakedAnimationData.bakedAnimations;

        if (bakedAnimations && bakedAnimations.has(animeKey)) {
            // animationMatrixArray を取得
            return bakedAnimations.get(animeKey).animationMatrixArray;
        }

        console.warn(`[getAnimationMatrixArray] アニメーションキー "${animeKey}" は見つかりませんでした。`);
        return null;
    }

    /**
     * モデルに共通のInverse Bind Matrix (IBM) 配列を取得します。
     * @returns {Float32Array|null} - IBM行列の配列、またはデータがない場合は null
     */
    getInverseBoneMatrixArray() {
        const gltfUrl = this._lastLoadedUrl;
        if (!gltfUrl || !this._modelCache.has(gltfUrl)) {
            console.warn('[getInverseBoneMatrixArray] モデルがロードされていません。');
            return null;
        }

        const cachedData = this._modelCache.get(gltfUrl);
        
        // inverseMatrixArray は bakedAnimationData のトップレベルに格納されている
        return cachedData.bakedAnimationData.inverseMatrixArray || null;
    }



    /**
     * 現在ロードされているモデルに含まれるすべてのアニメーションキー（名前）をコンソールに出力します。
     */
    logAvailableAnimationKeys() {
        const gltfUrl = this._lastLoadedUrl;
        
        if (!gltfUrl || !this._modelCache.has(gltfUrl)) {
            console.warn('[logAvailableAnimationKeys] モデルがロードされていません。アニメーションキーを取得できません。');
            return;
        }

        const cachedData = this._modelCache.get(gltfUrl);
        const bakedAnimations = cachedData.bakedAnimationData.bakedAnimations;

        if (!bakedAnimations || bakedAnimations.size === 0) {
            console.log('[logAvailableAnimationKeys] ロードされたモデルにはアニメーションデータが含まれていません。');
            return;
        }

        console.log('--- 🚨利用可能なアニメーションキー ---');
        let index = 0;
        
        // Mapのキーをすべて取得し、コンソールに出力
        bakedAnimations.forEach((value, key) => {
            const maxFrames = value.maxKeyframeCount;
            console.log(` 🚨 [${index}] Key: "${key}", (Max Frames: ${maxFrames})`);
            index++;
        });
        
        console.log(`------------------------------------ (合計 ${bakedAnimations.size} 件)`);
    }



    getNumBones() {
        const gltfUrl = this._lastLoadedUrl;
        if (!gltfUrl || !this._modelCache.has(gltfUrl)) {
            return null;
        }
        const cachedData = this._modelCache.get(gltfUrl);
        return cachedData.bakedAnimationData.boneIndices.length;
    }


    /**
     * 指定されたアニメーションキーの最大フレーム数（0から始まるインデックスの最大値+1）を取得します。
     * @param {string} animeKey - 取得したいアニメーションの名前
     * @returns {number|null} - 最大フレーム数、またはデータがない場合は null
     */
    getMaxFrameNum(animeKey) {
        const gltfUrl = this._lastLoadedUrl;
        if (!gltfUrl || !this._modelCache.has(gltfUrl)) {
            console.warn('[getMaxFrameNum] モデルがロードされていません。');
            return null;
        }

        const cachedData = this._modelCache.get(gltfUrl);
        const bakedAnimations = cachedData.bakedAnimationData.bakedAnimations;

        if (bakedAnimations && bakedAnimations.has(animeKey)) {
            // _processAnimationDataで格納した maxKeyframeCount を取得
            return bakedAnimations.get(animeKey).maxKeyframeCount;
        }

        console.warn(`[getMaxFrameNum] アニメーションキー "${animeKey}" は見つかりませんでした。`);
        return null;
    }


    
    /**
     * 指定されたアニメーション、フレーム、ボーンIDの最終変換行列を取得し、コンソールに出力します。
     * @param {string} animeKey - アニメーション名
     * @param {number} frame - フレーム番号 (0から始まる)
     * @param {number} boneID - ボーンID (0から始まる)
     * @returns {Float32Array|null} - 最終変換行列 (16成分)、または null
     */
    getAnimatedBoneMatrix(animeKey, frame, boneID) {
        const matrixArray = this.getAnimationMatrixArray(animeKey);
        const numBones = this.getNumBones();
        
        if (!matrixArray || numBones === null) {
            console.warn(`[getAnimatedBoneMatrix] データ取得に失敗: アニメーションキー "${animeKey}" またはボーン情報が不足しています。`);
            return null;
        }

        const MATRIX_SIZE = 16; 
        
        // 行列が始まる配列のインデックスを計算
        const offset = (frame * numBones + boneID) * MATRIX_SIZE;
        
        if (offset < 0 || offset + MATRIX_SIZE > matrixArray.length) {
            console.warn(`[getAnimatedBoneMatrix] 指定されたフレーム(${frame})またはボーンID(${boneID})は範囲外です。`);
            return null;
        }

        // 行列を配列から切り出す
        const targetMatrix = matrixArray.slice(offset, offset + MATRIX_SIZE);
        
        // -----------------------------------------------------------------
        // ログ出力
        // -----------------------------------------------------------------
        console.log(`\n--- アニメーション行列 (Frame ${frame}, Bone ID ${boneID}) ---`);
        console.log(`  アニメーション: "${animeKey}"`);
        console.log(`  配列オフセット: ${offset}`);
        
        // 4x4 行列の形式で出力
        console.log(`  | ${targetMatrix[0].toFixed(4)}  ${targetMatrix[4].toFixed(4)}  ${targetMatrix[8].toFixed(4)}  ${targetMatrix[12].toFixed(4)} |`);
        console.log(`  | ${targetMatrix[1].toFixed(4)}  ${targetMatrix[5].toFixed(4)}  ${targetMatrix[9].toFixed(4)}  ${targetMatrix[13].toFixed(4)} |`);
        console.log(`  | ${targetMatrix[2].toFixed(4)}  ${targetMatrix[6].toFixed(4)}  ${targetMatrix[10].toFixed(4)}  ${targetMatrix[14].toFixed(4)} |`);
        console.log(`  | ${targetMatrix[3].toFixed(4)}  ${targetMatrix[7].toFixed(4)}  ${targetMatrix[11].toFixed(4)}  ${targetMatrix[15].toFixed(4)} |`);
        console.log('------------------------------------------------------------');

        return targetMatrix;
    }



    /**
     * Base64文字列をArrayBufferにデコードするヘルパー関数
     * @param {string} base64 - Base64エンコードされた文字列
     * @returns {ArrayBuffer} - デコードされたバイナリデータ
     */

    _base64ToArrayBuffer(base64) {

        console.log('[SolaGltfParser._base64ToArrayBuffer] Base64デコード開始');
        
        // ブラウザの atob() 関数を使ってBase64をデコードし、バイナリ文字列を得る
        const binaryString = atob(base64); 
        const len = binaryString.length;
        
        // バイナリ文字列を格納するための符号なし8ビット整数配列 (Uint8Array) を作成
        const bytes = new Uint8Array(len);
        
        // 文字列からUint8Arrayにデータをコピー (charCodeAtで文字コードを取得)
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
                
                // 🚨 【修正/追加】ボーン/ウェイト属性の抽出
                const jointData = primitive.attributes.JOINTS_0 
                    ? this._getTypedArrayFromAccessor(gltf, arrayBuffer, primitive.attributes.JOINTS_0)
                    : null; // VEC4, 通常はUint8ArrayまたはUint16Array
                const weightData = primitive.attributes.WEIGHTS_0 
                    ? this._getTypedArrayFromAccessor(gltf, arrayBuffer, primitive.attributes.WEIGHTS_0)
                    : null; // VEC4, 通常はFloat32Array
                
                // インデックスを取得
                const indexAccessorIndex = primitive.indices;
                const indexData = this._getTypedArrayFromAccessor(gltf, arrayBuffer, indexAccessorIndex);
                
                // SolaMesh用のインターリーブ配列を作成
                // 🚨 【修正】jointDataとweightDataを引数に追加
                const interleavedArray = this._createInterleavedArray(positionData, normalData, uvData, jointData, weightData); 

                meshDataList.push({
                    vertexData: interleavedArray,
                    indexData: indexData,
                    drawMode: primitive.mode || this.gl.TRIANGLES,
                });
            }
        }
        return meshDataList;
    }


    /**
     * 抽出した属性データからSolaMesh用のインターリーブ配列を作成する
     */
    _createInterleavedArray(positionData, normalData, uvData, jointData, weightData) {

        console.log('[SolaGltfParser._createInterleavedArray] インターリーブ配列の作成開始');
        
        const vertexCount = positionData.length / 3;
        const STRIDE_FLOATS = 18; 
        const interleavedArray = new Float32Array(vertexCount * STRIDE_FLOATS);

        console.log('--- インターリーブ配列格納データ (最初の10頂点のみ) ---');

        for (let i = 0; i < vertexCount; i++) {
            let offset = i * STRIDE_FLOATS;
            
            // ----------------------------------------------------
            // 🚨 【修正】すべてのローカル変数をここで定義
            // ----------------------------------------------------
            // 1. POSITION (Posデータはローカル変数化せずに直接格納)
            
            // 2. UV (u, v)
            const u = uvData ? uvData[i * 2 + 0] : 0.0;
            const v = uvData ? uvData[i * 2 + 1] : 0.0;

            // 3. NORMAL (nx, ny, nz)
            const nx = normalData ? normalData[i * 3 + 0] : 0.0;
            const ny = normalData ? normalData[i * 3 + 1] : 1.0;
            const nz = normalData ? normalData[i * 3 + 2] : 0.0;

            // 4. BoneID (joint0-3)
            const joint0 = jointData ? jointData[i * 4 + 0] : 0.0;
            const joint1 = jointData ? jointData[i * 4 + 1] : 0.0;
            const joint2 = jointData ? jointData[i * 4 + 2] : 0.0;
            const joint3 = jointData ? jointData[i * 4 + 3] : 0.0;
            
            // 5. BoneWeight (weight0-3)
            const weight0 = weightData ? weightData[i * 4 + 0] : 1.0;
            const weight1 = weightData ? weightData[i * 4 + 1] : 0.0;
            const weight2 = weightData ? weightData[i * 4 + 2] : 0.0;
            const weight3 = weightData ? weightData[i * 4 + 3] : 0.0;
            // ----------------------------------------------------

            // 1. POSITION (格納)
            interleavedArray[offset + 0] = positionData[i * 3 + 0];
            interleavedArray[offset + 1] = positionData[i * 3 + 1];
            interleavedArray[offset + 2] = positionData[i * 3 + 2];
            interleavedArray[offset + 3] = 1.0; 

            // 2. UV (格納)
            interleavedArray[offset + 4] = u;
            interleavedArray[offset + 5] = v;

            // 3. NORMAL (格納)
            interleavedArray[offset + 6] = nx;
            interleavedArray[offset + 7] = ny;
            interleavedArray[offset + 8] = nz;
            interleavedArray[offset + 9] = 0.0; 
            
            // 4. BoneID (格納)
            interleavedArray[offset + 10] = joint0; 
            interleavedArray[offset + 11] = joint1;
            interleavedArray[offset + 12] = joint2;
            interleavedArray[offset + 13] = joint3;
            
            // 5. BoneWeight (格納)
            interleavedArray[offset + 14] = weight0; 
            interleavedArray[offset + 15] = weight1;
            interleavedArray[offset + 16] = weight2;
            interleavedArray[offset + 17] = weight3;

            /*
            // 🚨 【修正】console.log内でローカル変数を使用
            if (i < 10) {
                // Posデータはローカル変数化していないため、配列から取得
                console.log(`[Vtx ${i}] Pos: (${interleavedArray[offset + 0].toFixed(2)}, ${interleavedArray[offset + 1].toFixed(2)}, ${interleavedArray[offset + 2].toFixed(2)})`);
                
                // UV, Normal, JointID, Weightはローカル変数を使用
                console.log(`      UV: (${u.toFixed(2)}, ${v.toFixed(2)}), Normal: (${nx.toFixed(2)}, ${ny.toFixed(2)}, ${nz.toFixed(2)})`);
                console.log(`      JointID: [${joint0}, ${joint1}, ${joint2}, ${joint3}], Weight: [${weight0.toFixed(2)}, ${weight1.toFixed(2)}, ${weight2.toFixed(2)}, ${weight3.toFixed(2)}]`);
            }
                */

        }
        
        console.log('-------------------------------------------');
        return interleavedArray;
    }




    onDestroy() {


    }



}

