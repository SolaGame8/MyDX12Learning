





window.addEventListener('DOMContentLoaded', async () => { //読み込み完了後


    /*
    // 1. FPS計測用変数の定義
    let frameCount = 0;
    let lastTime = performance.now();
    const FPS_UPDATE_INTERVAL = 1000; // 1000ms (1秒) ごとに更新
    */

    //FPS表示
    const fpsElement = document.getElementById('fps-counter'); // HTML要素を取得



    // Canvas ID を指定して SolaWGL ヘルパーを初期化
    const wgl = new SolaWGL('glCanvas');


    // 非同期処理: SolaWGLの非同期初期化を待つ (シェーダーファイルの読み込みを含む)
    const isReady = await wgl.init();
    if (!isReady) {
        console.error("アプリケーションの初期化に失敗しました");
        return;
    }

    /*
    if (!wgl.isReady()) {
        return;
    }
*/
    

    //キー検知の登録

    wgl.inputManager.addKeyToTrack([' ', '1', '2', '3']);


    //サウンド読み込み

    const bgm001Key = "bgm001";
    await wgl.soundManager.loadSound(bgm001Key, "./sound/bgm001.mp3");
    const bgm002Key = "bgm002";
    await wgl.soundManager.loadSound(bgm002Key, "./sound/bgm002.mp3");

    const sound001Key = "sound001";
    await wgl.soundManager.loadSound(sound001Key, "./sound/s001.mp3");
    const sound002Key = "sound002";
    await wgl.soundManager.loadSound(sound002Key, "./sound/s002.mp3");
    const sound003Key = "sound003";
    await wgl.soundManager.loadSound(sound003Key, "./sound/s003.mp3");






    //テクスチャー
    const checkenTextureKey = "checken_texture_key";
    let res = await wgl.textureManager.loadAndRegister(checkenTextureKey, './gltf/chicken_albedo.png');


    const triangleTextureKey = "triangle_texture_key";
    res = await wgl.textureManager.loadAndRegister(triangleTextureKey, './images/my_texture.png');
    

    //glTFロード
    const meshDataList = await wgl.gltfParser.loadModel('./gltf/chicken_walk.gltf');


    //作成
    let gltfMesh = new SolaMesh(this);

    gltfMesh.setMeshDataList(meshDataList);


    /*
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
            gltfMesh.addVertexData({
                position: tempPosition,
                uv: tempUV,
                normal: tempNormal,
                boneIDs: tempBoneId,
                boneWeights: tempBoneWeight
            });

            // デバッグ表示 (最初の50項目のみ)
            if (i < 50) {
                 console.log(`[Add #${i}] P:${tempPosition.map(n=>n.toFixed(3))} UV:${tempUV.map(n=>n.toFixed(3))} N:${tempNormal.map(n=>n.toFixed(3))}`);
            }
        }
        
        // 3. インデックスデータを追加
        // addIndexData(idx1, idx2, idx3) に合わせて3つずつ渡します
        for (let i = 0; i < indexDataTyped.length; i += 3) {
            gltfMesh.addIndexData(indexDataTyped[i], indexDataTyped[i + 1], indexDataTyped[i + 2]);
        }
        
    });
*/

    // モデルをビルド
    gltfMesh.buildMesh(wgl);

    gltfMesh.setScale(1.0, 1.0, 1.0);

    //テクスチャーをセット
    gltfMesh.setTextureKey(checkenTextureKey);





    //モデル作成

    let cubeMesh = new SolaMesh(this);

    // 頂点データ
    // 立方体は6面で構成され、各面は2つの三角形（4つの頂点）で構成されます。
    // 面ごとに法線とUV座標を正しく設定するため、頂点は重複して定義します。（合計 6面 * 4頂点 = 24頂点）

    // 頂点データ: { position: [x, y, z], uv: [u, v], normal: [nx, ny, nz], ... }
    // boneIDs/boneWeights はスケルタルアニメーション用ですが、今回は全て 0.0 で固定
    const boneData = { boneIDs: [0.0, 0.0, 0.0, 0.0], boneWeights: [0.0, 0.0, 0.0, 0.0] };

    // -----------------------------------------------------------
    // 1. 正面 (Front: Z+)
    // -----------------------------------------------------------
    // A: 右上 (1, 1, 1)
    cubeMesh.addVertexData({ position: [ 1.0,  1.0,  1.0 ], uv: [ 1.0, 0.0 ], normal: [ 0.0, 0.0, 1.0 ], ...boneData });
    // B: 左上 (-1, 1, 1)
    cubeMesh.addVertexData({ position: [-1.0,  1.0,  1.0 ], uv: [ 0.0, 0.0 ], normal: [ 0.0, 0.0, 1.0 ], ...boneData });
    // C: 右下 (1, -1, 1)
    cubeMesh.addVertexData({ position: [ 1.0, -1.0,  1.0 ], uv: [ 1.0, 1.0 ], normal: [ 0.0, 0.0, 1.0 ], ...boneData });
    // D: 左下 (-1, -1, 1)
    cubeMesh.addVertexData({ position: [-1.0, -1.0,  1.0 ], uv: [ 0.0, 1.0 ], normal: [ 0.0, 0.0, 1.0 ], ...boneData });

    // -----------------------------------------------------------
    // 2. 背面 (Back: Z-)
    // -----------------------------------------------------------
    // E: 左上 (-1, 1, -1)
    cubeMesh.addVertexData({ position: [-1.0,  1.0, -1.0 ], uv: [ 1.0, 0.0 ], normal: [ 0.0, 0.0, -1.0 ], ...boneData });
    // F: 右上 (1, 1, -1)
    cubeMesh.addVertexData({ position: [ 1.0,  1.0, -1.0 ], uv: [ 0.0, 0.0 ], normal: [ 0.0, 0.0, -1.0 ], ...boneData });
    // G: 左下 (-1, -1, -1)
    cubeMesh.addVertexData({ position: [-1.0, -1.0, -1.0 ], uv: [ 1.0, 1.0 ], normal: [ 0.0, 0.0, -1.0 ], ...boneData });
    // H: 右下 (1, -1, -1)
    cubeMesh.addVertexData({ position: [ 1.0, -1.0, -1.0 ], uv: [ 0.0, 1.0 ], normal: [ 0.0, 0.0, -1.0 ], ...boneData });

    // -----------------------------------------------------------
    // 3. 右面 (Right: X+)
    // -----------------------------------------------------------
    // A: 右上(前) (1, 1, 1)
    cubeMesh.addVertexData({ position: [ 1.0,  1.0,  1.0 ], uv: [ 0.0, 0.0 ], normal: [ 1.0, 0.0, 0.0 ], ...boneData });
    // F: 右上(奥) (1, 1, -1)
    cubeMesh.addVertexData({ position: [ 1.0,  1.0, -1.0 ], uv: [ 1.0, 0.0 ], normal: [ 1.0, 0.0, 0.0 ], ...boneData });
    // C: 右下(前) (1, -1, 1)
    cubeMesh.addVertexData({ position: [ 1.0, -1.0,  1.0 ], uv: [ 0.0, 1.0 ], normal: [ 1.0, 0.0, 0.0 ], ...boneData });
    // H: 右下(奥) (1, -1, -1)
    cubeMesh.addVertexData({ position: [ 1.0, -1.0, -1.0 ], uv: [ 1.0, 1.0 ], normal: [ 1.0, 0.0, 0.0 ], ...boneData });

    // -----------------------------------------------------------
    // 4. 左面 (Left: X-)
    // -----------------------------------------------------------
    // B: 左上(前) (-1, 1, 1)
    cubeMesh.addVertexData({ position: [-1.0,  1.0,  1.0 ], uv: [ 1.0, 0.0 ], normal: [-1.0, 0.0, 0.0 ], ...boneData });
    // E: 左上(奥) (-1, 1, -1)
    cubeMesh.addVertexData({ position: [-1.0,  1.0, -1.0 ], uv: [ 0.0, 0.0 ], normal: [-1.0, 0.0, 0.0 ], ...boneData });
    // D: 左下(前) (-1, -1, 1)
    cubeMesh.addVertexData({ position: [-1.0, -1.0,  1.0 ], uv: [ 1.0, 1.0 ], normal: [-1.0, 0.0, 0.0 ], ...boneData });
    // G: 左下(奥) (-1, -1, -1)
    cubeMesh.addVertexData({ position: [-1.0, -1.0, -1.0 ], uv: [ 0.0, 1.0 ], normal: [-1.0, 0.0, 0.0 ], ...boneData });

    // -----------------------------------------------------------
    // 5. 上面 (Top: Y+)
    // -----------------------------------------------------------
    // B: 左上(前) (-1, 1, 1)
    cubeMesh.addVertexData({ position: [-1.0,  1.0,  1.0 ], uv: [ 0.0, 1.0 ], normal: [ 0.0, 1.0, 0.0 ], ...boneData });
    // A: 右上(前) (1, 1, 1)
    cubeMesh.addVertexData({ position: [ 1.0,  1.0,  1.0 ], uv: [ 1.0, 1.0 ], normal: [ 0.0, 1.0, 0.0 ], ...boneData });
    // E: 左上(奥) (-1, 1, -1)
    cubeMesh.addVertexData({ position: [-1.0,  1.0, -1.0 ], uv: [ 0.0, 0.0 ], normal: [ 0.0, 1.0, 0.0 ], ...boneData });
    // F: 右上(奥) (1, 1, -1)
    cubeMesh.addVertexData({ position: [ 1.0,  1.0, -1.0 ], uv: [ 1.0, 0.0 ], normal: [ 0.0, 1.0, 0.0 ], ...boneData });

    // -----------------------------------------------------------
    // 6. 底面 (Bottom: Y-)
    // -----------------------------------------------------------
    // D: 左下(前) (-1, -1, 1)
    cubeMesh.addVertexData({ position: [-1.0, -1.0,  1.0 ], uv: [ 0.0, 0.0 ], normal: [ 0.0, -1.0, 0.0 ], ...boneData });
    // C: 右下(前) (1, -1, 1)
    cubeMesh.addVertexData({ position: [ 1.0, -1.0,  1.0 ], uv: [ 1.0, 0.0 ], normal: [ 0.0, -1.0, 0.0 ], ...boneData });
    // G: 左下(奥) (-1, -1, -1)
    cubeMesh.addVertexData({ position: [-1.0, -1.0, -1.0 ], uv: [ 0.0, 1.0 ], normal: [ 0.0, -1.0, 0.0 ], ...boneData });
    // H: 右下(奥) (1, -1, -1)
    cubeMesh.addVertexData({ position: [ 1.0, -1.0, -1.0 ], uv: [ 1.0, 1.0 ], normal: [ 0.0, -1.0, 0.0 ], ...boneData });


    // インデックスデータ
    // 頂点インデックスは0から始まり、面ごとに 4 ずつ増加します。
    // 各面は2つの三角形で構成されます: (0, 1, 2) と (2, 1, 3) (または (0, 2, 3) と (0, 3, 1) など)

    for (let i = 0; i < 6; i++) {
        const offset = i * 4; // 各面の開始インデックス (0, 4, 8, 12, 16, 20)
        
        // 1つ目の三角形 (左上、右上、左下)
        cubeMesh.addIndexData(offset + 1, offset + 0, offset + 2); 
        // 2つ目の三角形 (左上、右下、右上) ※時計回りまたは反時計回りで定義

        // 左下、右上、右下
        cubeMesh.addIndexData(offset + 2, offset + 0, offset + 3);

        // ※一般的に (0, 1, 2), (2, 3, 0) と定義しますが、ここでは上の定義に合わせました。
        // (0: 右上, 1: 左上, 2: 右下, 3: 左下) の場合:
        // (1, 0, 2) と (1, 2, 3) が適切です。（面によっては順序が変わります）
        
        // 正しい定義（頂点の並び順に依存）
        // 矩形を構成する4頂点を (v0, v1, v2, v3) とした場合、
        // v0 --- v1
        // |      |
        // v2 --- v3
        // の順なら (0, 2, 3), (0, 3, 1) または (1, 0, 2), (1, 2, 3)

        // 今回の頂点定義順:
        // [0]: 右上, [1]: 左上, [2]: 右下, [3]: 左下

        // 1つ目の三角形 (右上(0), 左上(1), 左下(3))
        // cubeMesh.addIndexData(offset + 0, offset + 1, offset + 3);

        // 2つ目の三角形 (右上(0), 左下(3), 右下(2))
        // cubeMesh.addIndexData(offset + 0, offset + 3, offset + 2);
        
        // 上記のインデックス定義をコメントアウトし、既存のコードに似た形式で定義
        // (0, 1, 2) と (2, 1, 3) の組み合わせ
        // cubeMesh.addIndexData(offset + 0, offset + 1, offset + 2); // 右上、左上、右下
        // cubeMesh.addIndexData(offset + 2, offset + 1, offset + 3); // 右下、左上、左下

        // ※最初の定義 (offset + 1, offset + 0, offset + 2) と (offset + 2, offset + 0, offset + 3) を使用します
        // (1, 0, 2) と (2, 0, 3) - 面ごとに確認しながら調整してください。
        // 多くの場合は (0, 1, 2) と (2, 3, 0) のような順序になります。
        // 正面 (Z+) は、外側から見て反時計回りになるようにします。
        // (1. 左上, 0. 右上, 2. 右下) と (1. 左上, 2. 右下, 3. 左下) が一般的
        cubeMesh.addIndexData(offset + 1, offset + 0, offset + 2); // 左上, 右上, 右下
        cubeMesh.addIndexData(offset + 1, offset + 2, offset + 3); // 左上, 右下, 左下
    }


    // モデルをビルド
    cubeMesh.buildMesh(wgl);

    //テクスチャーをセット
    cubeMesh.setTextureKey(triangleTextureKey);











    let triangleMesh = new SolaMesh(this);

    //頂点
    triangleMesh.addVertexData({
            position: [ 0.0,  1.0, 0.0 ], uv: [ 0.5, 0.0 ], normal: [ 0.0, 0.0, 1.0 ],
            boneIDs: [0.0, 0.0, 0.0, 0.0], boneWeights: [0.0, 0.0, 0.0, 0.0]
        });
    triangleMesh.addVertexData({
            position: [ -1.0, -1.0, 0.0 ], uv: [ 0.0, 1.0 ], normal: [ 0.0, 0.0, 1.0 ],
            boneIDs: [0.0, 0.0, 0.0, 0.0], boneWeights: [0.0, 0.0, 0.0, 0.0]
        });
    triangleMesh.addVertexData({
            position: [ 1.0, -1.0, 0.0 ], uv: [ 1.0, 1.0 ], normal: [ 0.0, 0.0, 1.0 ],
            boneIDs: [0.0, 0.0, 0.0, 0.0], boneWeights: [0.0, 0.0, 0.0, 0.0]
        });

    //インデックス
    triangleMesh.addIndexData(0, 1, 2);

    //モデルをビルド
    triangleMesh.buildMesh(wgl);


    //テクスチャーをセット
    triangleMesh.setTextureKey(triangleTextureKey);



    wgl.setFpsLimit(80);

    //wgl.update();//情報の更新

    wgl.setClearColor(0.6, 0.8, 0.9, 1.0);  //クリア色の設定 (R, G, B, A)

    //wgl.draw();

    //console.log("WebGLコンテキストが初期化され、キャンバスがクリアされました。");

    
    // 2. メッセージボックスが繰り返し表示されるのを防ぐための状態変数
    //let wasAPressed = false; 


    
    let gameCounter = 0.0;


    const render = () => {

        /*
        // 2. FPS計測ロジック
        frameCount++;
        const currentTime = performance.now();
        const deltaTime = currentTime - lastTime;

        if (deltaTime >= FPS_UPDATE_INTERVAL) {
            // FPSを計算: フレーム数 / (経過時間 / 1000)
            const fps = Math.round(frameCount / (deltaTime / 1000));
            
            // HTML要素を更新
            if (fpsElement) {
                fpsElement.textContent = `FPS: ${fps}`;
            }

            // カウンターをリセット
            frameCount = 0;
            lastTime = currentTime; 
        }
        */



        const flg_Update = wgl.update();

        if (flg_Update) {


            


            const deltaTime = wgl.getDeltaTime();

            const fps = wgl.getFps();

                        // HTML要素を更新
            if (fpsElement) {
                fpsElement.textContent = `FPS: ${fps}`;
            }

            const stickL = wgl.inputManager.getStickValue('L', 0.15);
            //triangleX = stickL.x;
            //triangleY = -stickL.y; // WebGLのY軸は上が正なのでY軸を反転
            
            /*
            if (stickL.x > 0.0 && stickL.y > 0.0) {
                //console.log('Left Stick X:', stickL.x, 'Left Stick Y:', stickL.y);
            }
    */


            // --- Aボタン入力チェック ---
            if (typeof BUTTONS !== 'undefined') { // BUTTONS定数が読み込まれているかチェック
                const isAPressed = wgl.inputManager.getGamepadOnPush(BUTTONS.A);

                if (isAPressed) {
                    //wgl.soundManager.playSound(bgm001Key, 0.02, false);
                    wgl.soundManager.playSound(sound003Key, 0.05, false);
                }
                // Aボタンが押された瞬間 (前回 false -> 今回 true) のみ処理を実行
                //if (isAPressed && !wasAPressed) {
                //if (isAPressed) {
                //    alert("Aボタンが押されました！");
                //}

                // 次のフレームのために現在の状態を保持
                //wasAPressed = isAPressed;
            }

            //キー入力チェック

            if (wgl.inputManager.onPushKey('1')) {
                wgl.soundManager.playMusic(bgm001Key, 0.02, true);
            }
            if (wgl.inputManager.onPushKey('2')) {
                wgl.soundManager.crossFadeMusic(bgm002Key, 0.02, true, 1.0);
            }
            if (wgl.inputManager.onPushKey('3')) {
                wgl.soundManager.stopMusic(3.0);
            }


            // ------------------------------

            gameCounter += 1.0 * deltaTime;
            //console.log(`cameraRot ${cameraRot}`);

            wgl.setCameraAngle(-gameCounter * 10.0, Math.sin(gameCounter * 0.3) * 90.0, 0.0);
            wgl.setCameraDistance(1.0);
            wgl.calcCameraPosByDistanceAndAngles();

            wgl.useShaderProgram("Default");

            wgl.clearCanvas();

            //triangleMesh.draw(wgl);
            
            
            cubeMesh.setScale(0.1, 0.1, 0.1);
            cubeMesh.setPosition(0.3 * Math.sin(gameCounter), 0.0, 0.3 * Math.cos(gameCounter));
            cubeMesh.setRotation(0.0, gameCounter * 5.0, 0.0);
            cubeMesh.draw(wgl);


            gltfMesh.setScale(1.0, 1.0, 1.0);
            gltfMesh.setPosition(0.0, 0.0, 0.0);
            gltfMesh.setRotation(0.0, gameCounter * 45.0, 0.0);
            gltfMesh.draw(wgl);

            gltfMesh.setScale(1.0, 1.0, 1.0);
            gltfMesh.setPosition(0.2, 0.0, 0.0);
            gltfMesh.setRotation(0.0, -gameCounter * 145.0, 0.0);
            gltfMesh.draw(wgl);





        }

        // 次の描画フレームを要求 (ループの継続)
        requestAnimationFrame(render);


    };

    // ループを開始
    render();
    



});

