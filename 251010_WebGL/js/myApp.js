





window.addEventListener('DOMContentLoaded', async () => { //読み込み完了後


    // メッセージ表示エリア（HTML）
    const messageArea = document.getElementById('messageArea'); // HTML要素を取得

    // Canvas ID (HTML) を指定して SolaWGL を初期化
    const wgl = new SolaWGL('glCanvas');


    // SolaWGLの非同期初期化を待つ
    const isReady = await wgl.init();
    if (!isReady) {
        console.error("アプリケーションの初期化に失敗しました");
        return;
    }

    
    messageArea.textContent = `loading: 0 %`;    // メッセージ表示（画面左上）


    //キー検知の登録（ここで登録しているもののキーイベントが取得できます）

    wgl.inputManager.addKeyToTrack([' ', '1', '2', '3' , 'w', 'a', 's', 'd', 'f']);


    //サウンド読み込み

    const bgm001Key = "bgm001";
    await wgl.soundManager.loadSound(bgm001Key, "./sound/bgm001.mp3");

    messageArea.textContent = `loading: 2 %`;

    const bgm002Key = "bgm002";
    await wgl.soundManager.loadSound(bgm002Key, "./sound/bgm002.mp3");

    messageArea.textContent = `loading: 4 %`;

    const sound001Key = "sound001";
    await wgl.soundManager.loadSound(sound001Key, "./sound/s001.mp3");

    messageArea.textContent = `loading: 6 %`;

    const sound002Key = "sound002";
    await wgl.soundManager.loadSound(sound002Key, "./sound/s002.mp3");

    messageArea.textContent = `loading: 8 %`;

    const sound003Key = "sound003";
    await wgl.soundManager.loadSound(sound003Key, "./sound/s003.mp3");




    messageArea.textContent = `loading: 10 %`;


    //テクスチャー
    const checkenTextureKey = "checken_texture_key";
    let res = await wgl.textureManager.loadAndRegister(checkenTextureKey, './gltf/chicken_albedo.png');

    messageArea.textContent = `loading: 20 %`;

    const floorTextureKey = "floor_texture_key";
    res = await wgl.textureManager.loadAndRegister(floorTextureKey, './images/land001.jpg');

    const sphereTextureKey = "sphere_texture_key";
    res = await wgl.textureManager.loadAndRegister(sphereTextureKey, './images/sky001.jpg');

    const triangleTextureKey = "triangle_texture_key";
    res = await wgl.textureManager.loadAndRegister(triangleTextureKey, './images/my_texture.png');
    
    messageArea.textContent = `loading: 30 %`;



    //glTFロード

    let parser = wgl.gltfParser; //パーサー


    const meshDataList = await parser.loadModel('./gltf/chicken_walk.gltf'); //歩きのデータ（ポリゴンも入っています）


    //3Dモデル作成
    let gltfMesh = new SolaMesh(this);

    {

        gltfMesh.setMeshDataList(meshDataList); //読みこんだgltfを3Dモデルにセットする
        gltfMesh.buildMesh(wgl);    // モデルをビルド（セットしたデータを使えるようにする）

        gltfMesh.setTextureKey(checkenTextureKey);//テクスチャーをセット（上記で読み込んだもののキーを渡す）

        gltfMesh.setScale(1.0, 1.0, 1.0);


        messageArea.textContent = `loading: 50 %`;
        
    }



    //アニメーションを取得
    let animationData = parser.getAnimationData();  //ポリゴンと一緒に入っている歩きのアニメーションも取得する


    if (animationData && animationData.length > 0) {

        //アニメーションデータを取得出来たら

        gltfMesh.setAnimationData(animationData); //アニメーションデータをモデルにセット


        console.log(`アニメーションデータ読み込み (合計 ${animationData.length} 件)`);

    } else {

        console.log("アニメーションデータは読み込まれていませんでした。");
    }



    await parser.loadModel('./gltf/chicken_jump.gltf'); //引き続き、同じパーサーでジャンプのアニメーションを読み込み

    let animationData2 = parser.getAnimationData();


    if (animationData2 && animationData2.length > 0) {

        //アニメーションデータを取得出来たら

        gltfMesh.setAnimationData(animationData2); //アニメーションデータをモデルにセット
    }


    //もう渡したデータは変数から削除
    animationData = null;
    animationData2 = null;


    //＊これは必ず呼ぶ！
    parser.removeModelData();   //パーサーの情報削除（もう使わない情報を削除）



    let animationKey = gltfMesh.getAnimationKey(); //現在、モデルに入れてある「アニメーション キー」の一覧

    
    console.log("--- 🚨 読み込まれたアニメーションキー一覧 ---");
    
    for (let i = 0; i < animationKey.length; i++) {    //animationData.length アニメーション数

        const animKey = animationKey[i];

        console.log(`🚨 [${i}] Key: "${animKey}"`); //歩き、ジャンプ、を再生する時に使うキーが入っています
        // （もともとアニメーションを作った時の名前がキーとして使われています）
    }






    //床モデル作成（地形）

    let floorMesh = new SolaMesh(this);

    const noiseSeed = 1234; 
    const perlin = new solaPerlinNoise(noiseSeed);  //パーリンノイズ

    // ノイズのスケール（周波数）: 小さいほどノイズが広がり、大きいほど細かくなる
    const noiseScale = 0.1; 

    // ノイズの振幅（高さの強調度）: 大きいほど高さの変化が大きくなる
    const noiseAmplitude = 3.0; 

    // EPSILONを定義（calculateNormal関数より前に置く）
    // 法線計算に使用する微小なオフセット（差分）
    const EPSILON = 1.0; 

    /**
     * ノイズから高さを計算するヘルパー関数
     * @param {number} x ワールドX座標
     * @param {number} z ワールドZ座標
     * @returns {number} ノイズに基づくY座標
     */
    function calculateHeight(x, z) {
        // ノイズ座標にスケールを適用
        const nx = x * noiseScale;
        const nz = z * noiseScale;
        
        // 3Dノイズを使用し、Y軸は時間やオフセットとして使用（ここでは0.0）
        const noiseValue = perlin.noise(nx, 0.0, nz);
        
        // ノイズ値を振幅でスケールしてY座標とする
        // ノイズ値は約 -1.0 から 1.0 の範囲
        return noiseValue * noiseAmplitude;
    }

    /**
     * 頂点の法線ベクトルを有限差分法で簡易的に計算する関数
     * @param {number} x 頂点のX座標
     * @param {number} z 頂点のZ座標
     * @returns {number[]} 正規化された法線ベクトル [nx, ny, nz]
     */
    function calculateNormal(x, z) {

        // 1. X方向の傾き（接線ベクトル Tx）を計算
        // 微小にXをずらした点の高さを取得
        const hX_plus = calculateHeight(x + EPSILON, z);
        const hX_minus = calculateHeight(x - EPSILON, z);
        
        // X方向の差分ベクトル (Tx)
        // 慣例的に (2 * EPSILON, hX_plus - hX_minus, 0) を使用しますが、
        // ここでは差分から直接勾配を計算します。
        // Tx = (dx, dy/dx * dx, 0)
        const Tx_x = 2.0 * EPSILON;
        const Tx_y = hX_plus - hX_minus; 
        const Tx_z = 0.0; 

        // 2. Z方向の傾き（接線ベクトル Tz）を計算
        // 微小にZをずらした点の高さを取得
        const hZ_plus = calculateHeight(x, z + EPSILON);
        const hZ_minus = calculateHeight(x, z - EPSILON);
        
        // Z方向の差分ベクトル (Tz)
        // Tz = (0, dy/dz * dz, dz)
        const Tz_x = 0.0;
        const Tz_y = hZ_plus - hZ_minus; 
        const Tz_z = 2.0 * EPSILON;
        
        // 3. 外積 (Cross Product) で法線を計算: N = Tz x Tx (右上向き)
        // Tx = (Tx_x, Tx_y, 0)
        // Tz = (0, Tz_y, Tz_z)
        
        nx = Tx_y * Tz_z - 0 * Tz_y;      // Tx_y * Tz_z - Tx_z * Tz_y
        ny = 0 * Tz_x - Tx_x * Tz_z;      // Tx_z * Tz_x - Tx_x * Tz_z
        nz = Tx_x * Tz_y - Tx_y * 0;      // Tx_x * Tz_y - Tx_y * Tz_x 

        // 4. 法線を正規化（長さを1にする）
        const length = Math.sqrt(nx * nx + ny * ny + nz * nz);
        
        // 正規化された法線ベクトルを返す
        // ゼロ除算を避ける
        if (length > 1e-6) {
            nx /= length;
            ny /= length;
            nz /= length;
            //return [nx / length, ny / length, nz / length];
        } else {
            // ノイズの変化が非常に小さい場合は、デフォルトの上向き法線を返す
            return [0.0, 1.0, 0.0];
        }

        // 5. Y成分の符号を確認・修正
        // 地形の「上側」が外側を向くように、nyが正であることを保証する
        // Tx x Tz の計算では ny が負になることが多い（右手座標系の場合）ため、
        // nyが負であれば、ベクトル全体を反転させることで、法線を上向き（内側から外側）にする。

        if (ny < 0) {
            nx = -nx;
            ny = -ny;
            nz = -nz;
        }

        return [nx, ny, nz];


    }



    {

        // 床は X軸に沿って -20.0 から 20.0 (40ブロック)
        // Z軸に沿って -20.0 から 20.0 (40ブロック) の範囲
        const size = 40; // ブロックの数 (X, Z 各方向)
        const halfSize = size / 2; // 中心からのオフセット

        let indexCounter = 0; // 頂点インデックスのカウンター

        // Y座標は常に 0.0 (平らな床)
        const y = 0.0;

        // 法線は常に (0.0, 1.0, 0.0) (上向き)
        const normal = [0.0, 1.0, 0.0];
        // ボーンデータは不要なためデフォルト値
        const boneIDs = [0.0, 0.0, 0.0, 0.0];
        const boneWeights = [0.0, 0.0, 0.0, 0.0];


        // Z軸 (-halfSize から halfSize) に沿ってループ
        for (let z = -halfSize; z < halfSize; z++) {
            // X軸 (-halfSize から halfSize) に沿ってループ
            for (let x = -halfSize; x < halfSize; x++) {

                // 現在のブロックの左下隅のワールド座標
                const x0 = x * 1.0;
                const z0 = z * 1.0;
                // 現在のブロックの右上隅のワールド座標
                const x1 = (x + 1) * 1.0;
                const z1 = (z + 1) * 1.0;

                // 頂点座標 (x, z) に対応する Y 座標を計算
                const y00 = calculateHeight(x0, z0); // 左下
                const y10 = calculateHeight(x1, z0); // 右下
                const y01 = calculateHeight(x0, z1); // 左上
                const y11 = calculateHeight(x1, z1); // 右上

                // 各頂点の法線を計算
                const n00 = calculateNormal(x0, z0);
                const n10 = calculateNormal(x1, z0);
                const n01 = calculateNormal(x0, z1);
                const n11 = calculateNormal(x1, z1);


                // ----------------------------------------------------
                // 1ブロック（2つの三角形）の頂点データを追加
                // 頂点順序：左下、右下、左上、右上 (時計回り、y=0面を上から見た場合)
                // ----------------------------------------------------

                // 左下 (P0)
                floorMesh.addVertexData({
                    position: [x0, y00, z0],
                    uv: [0.0, 1.0], // U:0.0, V:1.0 (UVの左下)
                    normal: n00,
                    boneIDs: boneIDs,
                    boneWeights: boneWeights
                });

                // 右下 (P1)
                floorMesh.addVertexData({
                    position: [x1, y10, z0],
                    uv: [1.0, 1.0], // U:1.0, V:1.0 (UVの右下)
                    normal: n10,
                    boneIDs: boneIDs,
                    boneWeights: boneWeights
                });

                // 左上 (P2)
                floorMesh.addVertexData({
                    position: [x0, y01, z1],
                    uv: [0.0, 0.0], // U:0.0, V:0.0 (UVの左上)
                    normal: n01,
                    boneIDs: boneIDs,
                    boneWeights: boneWeights
                });

                // 右上 (P3)
                floorMesh.addVertexData({
                    position: [x1, y11, z1],
                    uv: [1.0, 0.0], // U:1.0, V:0.0 (UVの右上)
                    normal: n11,
                    boneIDs: boneIDs,
                    boneWeights: boneWeights
                });

                // ----------------------------------------------------
                // インデックスデータを追加
                // 1ブロックはP0, P1, P2, P3の4頂点からなる四角形で、2つの三角形で構成
                // ----------------------------------------------------
                // T1: P0 (左下), P2 (左上), P1 (右下)
                floorMesh.addIndexData(indexCounter + 0, indexCounter + 2, indexCounter + 1);

                // T2: P1 (右下), P2 (左上), P3 (右上)
                floorMesh.addIndexData(indexCounter + 1, indexCounter + 2, indexCounter + 3);

                // 次のブロックのためにインデックスカウンターを4増やす
                indexCounter += 4;
            }
        }

        // モデルをビルド
        floorMesh.buildMesh(wgl);

        //テクスチャーをセット
        floorMesh.setTextureKey(floorTextureKey);
    }


    messageArea.textContent = `loading: 60 %`;




    //自然用のテクスチャー
    const natureTextureKey = "nature_texture_key";
    res = await wgl.textureManager.loadAndRegister(natureTextureKey, './gltf/texture_gradient.png');



    //木001 の生成　ーーーーーーーーーーーーー

    //glTFロード
    const tree_meshDataList = await wgl.gltfParser.loadModel('./gltf/tree001.gltf');


    //3Dモデル作成
    let treeMesh001 = new SolaMesh(this);

    {
        treeMesh001.setMeshDataList(tree_meshDataList);
        // モデルをビルド
        treeMesh001.buildMesh(wgl);
        treeMesh001.setScale(1.0, 1.0, 1.0);

        //テクスチャーをセット
        treeMesh001.setTextureKey(natureTextureKey);

    }


    const tree001_posArray = []; // 可変長の配列を初期化 (pushでデータ数を可変にする)


    // 乱数を生成

    const rng = new solaRandomGenerator();  //シード値が同じなら、毎回同じ乱数が出ます
    rng.setSeed(12345);


    const floorSize = 20.0;
    for (let i=0; i<15; i++) {

            let r = rng.getRandom();//0.0 - 1.0
            let x = (r - 0.5) * floorSize * 2.0;

            r = rng.getRandom();//0.0 - 1.0
            let z = (r - 0.5) * floorSize * 2.0;

            let y = calculateHeight(x, z);

            r = rng.getRandom();//0.0 - 1.0
            let rot = r * 360.0;

            tree001_posArray.push({x: x, y: y, z: z, rot : rot});
    }


    //木002 の生成　ーーーーーーーーーーーーー

    //glTFロード
    const tree002_meshDataList = await wgl.gltfParser.loadModel('./gltf/tree002.gltf');


    //3Dモデル作成
    let treeMesh002 = new SolaMesh(this);

    {
        treeMesh002.setMeshDataList(tree002_meshDataList);
        // モデルをビルド
        treeMesh002.buildMesh(wgl);
        treeMesh002.setScale(1.0, 1.0, 1.0);

        //テクスチャーをセット
        treeMesh002.setTextureKey(natureTextureKey);

    }


    const tree002_posArray = []; // 可変長の配列を初期化 (pushでデータ数を可変にする)


    // 乱数を生成
    rng.setSeed(557);   //シード値が同じなら、毎回同じ乱数が出ます

    for (let i=0; i<15; i++) {

            let r = rng.getRandom();//0.0 - 1.0
            let x = (r - 0.5) * floorSize * 2.0;

            r = rng.getRandom();//0.0 - 1.0
            let z = (r - 0.5) * floorSize * 2.0;

            let y = calculateHeight(x, z);

            r = rng.getRandom();//0.0 - 1.0
            let rot = r * 360.0;

            tree002_posArray.push({x: x, y: y, z: z, rot : rot});
    }



    //石001 の生成　ーーーーーーーーーーーーー

    //glTFロード
    const stone001_meshDataList = await wgl.gltfParser.loadModel('./gltf/stone001.gltf');


    //3Dモデル作成
    let stoneMesh001 = new SolaMesh(this);

    {
        stoneMesh001.setMeshDataList(stone001_meshDataList);
        // モデルをビルド
        stoneMesh001.buildMesh(wgl);
        stoneMesh001.setScale(1.0, 1.0, 1.0);

        //テクスチャーをセット
        stoneMesh001.setTextureKey(natureTextureKey);

    }


    const stone001_posArray = []; // 可変長の配列を初期化 (pushでデータ数を可変にする)

    // 乱数を生成
    rng.setSeed(963);

    for (let i=0; i<30; i++) {

            let r = rng.getRandom();//0.0 - 1.0
            let x = (r - 0.5) * floorSize * 2.0;

            r = rng.getRandom();//0.0 - 1.0
            let z = (r - 0.5) * floorSize * 2.0;

            let y = calculateHeight(x, z);

            r = rng.getRandom();//0.0 - 1.0
            let rot = r * 360.0;

            stone001_posArray.push({x: x, y: y, z: z, rot : rot});
    }

    //草001 の生成　ーーーーーーーーーーーーー

    //glTFロード
    const grass001_meshDataList = await wgl.gltfParser.loadModel('./gltf/grass001.gltf');


    //3Dモデル作成
    let grassMesh001 = new SolaMesh(this);

    {
        grassMesh001.setMeshDataList(grass001_meshDataList);
        // モデルをビルド
        grassMesh001.buildMesh(wgl);
        grassMesh001.setScale(1.0, 1.0, 1.0);

        //テクスチャーをセット
        grassMesh001.setTextureKey(natureTextureKey);

    }


    const grass001_posArray = []; // 可変長の配列を初期化 (pushでデータ数を可変にする)

    // 乱数を生成
    rng.setSeed(1379);

    for (let i=0; i<200; i++) {

            let r = rng.getRandom();//0.0 - 1.0
            let x = (r - 0.5) * floorSize * 2.0;

            r = rng.getRandom();//0.0 - 1.0
            let z = (r - 0.5) * floorSize * 2.0;

            let y = calculateHeight(x, z);

            r = rng.getRandom();//0.0 - 1.0
            let rot = r * 360.0;

            grass001_posArray.push({x: x, y: y, z: z, rot : rot});
    }







    //空の球体

    let sphereMesh = new SolaMesh(this);

    {

        // 半径
        const radius = 100.0; // 一般的なシーンで扱いやすいサイズ

        // 経度方向（X-Z平面の円周）の分割数
        // 32または64が一般的。この値が大きいほど、水平方向が滑らかになる。
        const segmentsX = 32;

        // 緯度方向（Y軸の上下）の分割数
        // 16または32が一般的。この値が大きいほど、垂直方向が滑らかになる。
        const segmentsY = 16;

        // ボーンデータは不要なためデフォルト値
        const boneIDs = [0.0, 0.0, 0.0, 0.0];
        const boneWeights = [0.0, 0.0, 0.0, 0.0];

        // 頂点とインデックスを格納する一時配列
        let positions = [];
        let uvs = [];
        let normals = [];
        let indices = [];

        // ----------------------------------------------------
        // 頂点データの生成
        // ----------------------------------------------------
        for (let y = 0; y <= segmentsY; y++) {
            const theta = y * Math.PI / segmentsY; // 緯度 (0 to PI)
            const sinTheta = Math.sin(theta);
            const cosTheta = Math.cos(theta);

            for (let x = 0; x <= segmentsX; x++) {
                const phi = x * 2 * Math.PI / segmentsX; // 経度 (0 to 2*PI)
                const sinPhi = Math.sin(phi);
                const cosPhi = Math.cos(phi);

                // 頂点座標 (x, y, z)
                const px = radius * sinTheta * cosPhi;
                const py = radius * cosTheta;
                const pz = radius * sinTheta * sinPhi;
                positions.push(px, py, pz);

                // UV座標
                // U: 経度方向 (0 to 1)
                // V: 緯度方向 (0 to 1)
                const u = 1 - (x / segmentsX); // テクスチャの巻き付け方向を考慮して反転
                const v = y / segmentsY;
                uvs.push(u, v);

                // 法線ベクトル (内側に向けるため、外側への法線を反転)
                nx = -sinTheta * cosPhi;
                ny = -cosTheta;
                nz = -sinTheta * sinPhi;

                nx = 0;
                ny = 1;
                nz = 0;

                normals.push(nx, ny, nz);

            }
        }

        // 頂点データを SolaMesh に追加
        let currentVertexIndex = 0;
        for (let i = 0; i < positions.length; i += 3) {
            sphereMesh.addVertexData({
                position: [positions[i], positions[i + 1], positions[i + 2]],
                uv: [uvs[currentVertexIndex * 2], uvs[currentVertexIndex * 2 + 1]],
                normal: [normals[i], normals[i + 1], normals[i + 2]],
                boneIDs: boneIDs,
                boneWeights: boneWeights
            });
            currentVertexIndex++;
        }


        // ----------------------------------------------------
        // インデックスデータの生成
        // ----------------------------------------------------
        for (let y = 0; y < segmentsY; y++) {
            for (let x = 0; x < segmentsX; x++) {
                // 現在のクワッドの4つの頂点インデックス
                const p0 = (y * (segmentsX + 1)) + x;          // 左下
                const p1 = (y * (segmentsX + 1)) + x + 1;      // 右下
                const p2 = ((y + 1) * (segmentsX + 1)) + x;      // 左上
                const p3 = ((y + 1) * (segmentsX + 1)) + x + 1;  // 右上

                // 2つの三角形で四角形を構成（内側から見るため、頂点順序を時計回りにする）
                // T1: P0 -> P1 -> P2 (左下、右下、左上)
                sphereMesh.addIndexData(p0, p1, p2);

                // T2: P1 -> P3 -> P2 (右下、右上、左上)
                sphereMesh.addIndexData(p1, p3, p2);
            }
        }

        // モデルをビルド
        sphereMesh.buildMesh(wgl);

        sphereMesh.setTextureKey(sphereTextureKey);

    }



    messageArea.textContent = `loading: 70 %`;



    //四角モデル作成

    let cubeMesh = new SolaMesh(this);

    {
        // 頂点データ
        // 立方体は6面で構成され、各面は2つの三角形（4つの頂点）で構成されます。
        // 面ごとに法線とUV座標を正しく設定するため、頂点は重複して定義します。（合計 6面 * 4頂点 = 24頂点）

        const boneData = { boneIDs: [0.0, 0.0, 0.0, 0.0], boneWeights: [0.0, 0.0, 0.0, 0.0] };

        // -----------------------------------------------------------
        // 正面 (Front: Z+)
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
        // 背面 (Back: Z-)
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
        // 右面 (Right: X+)
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
        // 左面 (Left: X-)
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
        // 上面 (Top: Y+)
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
        // 底面 (Bottom: Y-)
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
            
            cubeMesh.addIndexData(offset + 1, offset + 0, offset + 2); 
            cubeMesh.addIndexData(offset + 2, offset + 0, offset + 3);
            cubeMesh.addIndexData(offset + 1, offset + 0, offset + 2);
            cubeMesh.addIndexData(offset + 1, offset + 2, offset + 3);
        }


        // モデルをビルド
        cubeMesh.buildMesh(wgl);

        //テクスチャーをセット
        cubeMesh.setTextureKey(triangleTextureKey);

    }



    messageArea.textContent = `loading: 80 %`;





    let triangleMesh = new SolaMesh(this);

    {
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
    }

    messageArea.textContent = `loading: 100 %`;


    wgl.setFpsLimit(80);    //FPS制限（時間情報の取得の精度によって実際のfpsが変わるので、実際に60くらいになるように数値を入れています）


    wgl.setClearColor(0.6, 0.8, 0.9, 1.0);  //クリア色の設定 (R, G, B, A)




    //変数初期化

    
    let gameCounter = 0.0;
    let dayCounter = 0.0;


    let charaPos = {x: 0.0, y: 0.0, z: 0.0};
    let charaRot = {x: 0.0, y: 0.0, z: 0.0};
    let charaAcc = {x: 0.0, y: 0.0, z: 0.0};

    let cameraRot = {x: 0.0, y: 0.0, z: 0.0};

    let isOnGround = false;

    let isWalk = false;
    let isJump = false;


    //ループ
    const render = () => {


        const flg_Update = wgl.update();    //ここで、次のフレームの描画が出来る場合

        if (flg_Update) {   //描画処理をする

            //DeltaTime
            const deltaTime = wgl.getDeltaTime();


            // メッセージ欄（HTML）にFPS情報を表示
            if (messageArea) {
                const fps = wgl.getFps();
                messageArea.textContent = `FPS: ${fps}`;
            }



            //キャラクターの移動

            //左スティック
            const stickL = wgl.inputManager.getStickValue('L', 0.15);


            let moveVal = {x: stickL.x, y: stickL.y};

            if (wgl.inputManager.onPressKey('w')) {
                moveVal.x = 0.0;
                moveVal.y = -1.0;

                //console.log(`vec f ${vecForward.x}, ${vecForward.y} , camerarot ${cameraRot.y} `);


            }
            if (wgl.inputManager.onPressKey('s')) {
                moveVal.x = 0.0;
                moveVal.y = 1.0;
            }
            if (wgl.inputManager.onPressKey('d')) {
                moveVal.x = 1.0;
                moveVal.y = 0.0;
            }
            if (wgl.inputManager.onPressKey('a')) {
                moveVal.x = -1.0;
                moveVal.y = 0.0;
            }

            
            //カメラから見た、前方ベクトル、右ベクトル

            let vecForward = { x: -Math.sin(cameraRot.y * Math.PI / 180.0) , y: -Math.cos(cameraRot.y * Math.PI / 180.0) };
            let vecRight = { x:Math.cos(cameraRot.y * Math.PI / 180.0) , y: -Math.sin(cameraRot.y * Math.PI / 180.0) };


            const walkSpeed = 10.0;

            deltaX = 0.0;
            deltaZ = 0.0;

            deltaX += vecForward.x * walkSpeed * deltaTime * (-moveVal.y);
            deltaZ += vecForward.y * walkSpeed * deltaTime * (-moveVal.y);

            deltaX += vecRight.x * walkSpeed * deltaTime * moveVal.x;
            deltaZ += vecRight.y * walkSpeed * deltaTime * moveVal.x;

            charaPos.x += deltaX;
            charaPos.z += deltaZ;

            if (moveVal.x != 0.0 || moveVal.y != 0.0) {

                //移動している場合

                if (!isWalk && isOnGround) {
                    if (animationKey.length > 0) {
                        gltfMesh.playAnimation(animationKey[0], true);//アニメーションキー, Loopフラグ
                    }
                }
                isWalk = true;

                //キャラクターを進行方向に回転
                const moveRotationRad = Math.atan2(deltaX, deltaZ);
                charaRot.y = moveRotationRad * 180.0 / Math.PI;

            } else {
                if (isWalk) {
                    gltfMesh.stopAnimation();   //アニメーションストップ
                }
                isWalk = false;
            }




            // --- コントローラーAボタン入力チェック ---

            let flg_jump = false;
            if (typeof BUTTONS !== 'undefined') { // BUTTONS定数が読み込まれているかチェック
                const isAPressed = wgl.inputManager.getGamepadOnPush(BUTTONS.A);

                if (isAPressed) {

                    flg_jump = true;
                }

            }

            if (wgl.inputManager.onPushKey(' ')) {//スペースキーが押されていたら

                flg_jump = true;

            }

            if (flg_jump){
                if (isOnGround) {


                    isJump = true;

                    charaAcc.y = 0.3;

                    wgl.soundManager.playSound(sound003Key, 0.2, false);   //ジャンプ音

                    if (!isWalk && isOnGround) {
                        if (animationKey.length > 1) {
                            gltfMesh.playAnimation(animationKey[1], false);//アニメーションキー, Loopフラグ
                        }
                    }
                

                }
            }

            const gravity = 9.8 * 0.1;
            charaAcc.y -= gravity * deltaTime;

            charaPos.x += charaAcc.x;
            charaPos.y += charaAcc.y;
            charaPos.z += charaAcc.z;

            //キャラクター接地
            let charaFloorY = calculateHeight(charaPos.x, charaPos.z);
            if (charaPos.y < charaFloorY) {
                charaPos.y = charaFloorY;
                charaAcc.y = 0.0;

                if (isJump) {
                    gltfMesh.stopAnimation();
                }

                isJump = false;
                isOnGround = true;
            } else {
                isOnGround = false;
            }

            //カメラ回転

            let mouseDelta = wgl.inputManager.getMouseDelta();
            const rotRate = 0.5;
            cameraRot.y -= mouseDelta.x * rotRate;
            cameraRot.x += mouseDelta.y * rotRate;

            const stickR = wgl.inputManager.getStickValue('R', 0.15);
            const stickRotRate = 5.0;
            cameraRot.y -= stickR.x * stickRotRate;
            cameraRot.x += stickR.y * stickRotRate;

            
            // 画面のフルスクリーン切り替え
            if (wgl.inputManager.onPushKey('f')) {
                wgl.toggleFullscreen();
            }
            


            //音楽再生

            if (wgl.inputManager.onPushKey('1')) {
                wgl.soundManager.playMusic(bgm001Key, 0.2, true);
            }
            if (wgl.inputManager.onPushKey('2')) {
                wgl.soundManager.crossFadeMusic(bgm002Key, 0.2, true, 1.0);
            }
            if (wgl.inputManager.onPushKey('3')) {
                wgl.soundManager.stopMusic(3.0);
            }

            

            


            // ------------------------------

            gameCounter += 1.0 * deltaTime;


            //カメラ

            wgl.setCameraTarget(charaPos.x, charaPos.y + 1.0, charaPos.z);  //カメラのターゲット（ここを見る）

            wgl.setCameraAngle(cameraRot.x, cameraRot.y, cameraRot.z);  //ターゲットから見たカメラの回転
            wgl.setCameraDistance(8.0);                                 //ターゲットからの距離
            wgl.calcCameraPosByDistanceAndAngles();                     //回転と距離によってカメラの位置を計算

            //カメラの位置を直接指定したい場合は、wgl.setCameraPosition(x, y, z)

            
            let cameraPos = wgl.getCameraPosition();

            let cameraFloorY = calculateHeight(cameraPos[0], cameraPos[2]); //地面の高さ（パーリンノイズから情報を取っています）

            //カメラが地面の下に潜らないようにする
            const floorThreshold = 0.1;
            if (cameraPos[1] < cameraFloorY + floorThreshold) {
                cameraPos[1] = cameraFloorY + floorThreshold;
                wgl.setCameraPosition(cameraPos[0], cameraPos[1], cameraPos[2]);
            }



            //太陽の動き

            dayCounter += 1.0 * deltaTime;

            const wholedayTime = 24.0;
            if (dayCounter > wholedayTime) {
                dayCounter -= wholedayTime;
            }

            let lx = Math.sin(Math.PI * 2.0 * dayCounter / wholedayTime);
            let ly = -Math.cos(Math.PI * 2.0 * dayCounter / wholedayTime);

            wgl.setLightDirection(lx, ly, 0.0);



            wgl.useShaderProgram("Default");    //シェーダーをセット（現在はこの一種類しかありません）

            wgl.clearCanvas();                  //画面をクリア

            
            //キャラクター描画  （＊スケールや位置、回転などは変える必要が無ければ、書かなくても大丈夫です）
            gltfMesh.setScale(10.0, 10.0, 10.0);
            gltfMesh.setPosition(charaPos.x, charaPos.y, charaPos.z);
            gltfMesh.setRotation(charaRot.x, charaRot.y, charaRot.z);
            gltfMesh.draw(wgl);

            
            //空中を回っている立方体 描画
            cubeMesh.setPosition(5.0 * Math.sin(gameCounter * 0.3), 4.0, 5.0 * Math.cos(gameCounter * 0.3));
            cubeMesh.setRotation(gameCounter * 30.0, gameCounter * 50.0, gameCounter * 20.0);
            cubeMesh.draw(wgl);



            //地面 描画
            floorMesh.draw(wgl);



            //tree001   木をランダムでつくった位置に表示
            for (let i=0; i<tree001_posArray.length; i++) {
                treeMesh001.setPosition(tree001_posArray[i].x, tree001_posArray[i].y, tree001_posArray[i].z);
                treeMesh001.setRotation(0.0, tree001_posArray[i].rot, 0.0);
                treeMesh001.setScale(0.5, 0.5, 0.5);
                treeMesh001.draw(wgl);
            }
            //tree002   小さい木
            for (let i=0; i<tree002_posArray.length; i++) {
                treeMesh002.setPosition(tree002_posArray[i].x, tree002_posArray[i].y, tree002_posArray[i].z);
                treeMesh002.setRotation(0.0, tree002_posArray[i].rot, 0.0);
                treeMesh002.setScale(0.5, 0.5, 0.5);
                treeMesh002.draw(wgl);
            }
            //stone001  石
            for (let i=0; i<stone001_posArray.length; i++) {
                stoneMesh001.setPosition(stone001_posArray[i].x, stone001_posArray[i].y, stone001_posArray[i].z);
                stoneMesh001.setRotation(0.0, stone001_posArray[i].rot, 0.0);
                stoneMesh001.setScale(0.3, 0.3, 0.3);
                stoneMesh001.draw(wgl);
            }
            //grass001  草
            for (let i=0; i<grass001_posArray.length; i++) {
                grassMesh001.setPosition(grass001_posArray[i].x, grass001_posArray[i].y, grass001_posArray[i].z);
                grassMesh001.setRotation(0.0, grass001_posArray[i].rot, 0.0);
                grassMesh001.setScale(0.3, 0.3, 0.3);
                grassMesh001.draw(wgl);
            }


            //空の球体 描画
            sphereMesh.draw(wgl);



        }

        // 次の描画フレームを要求 (ループの継続)
        requestAnimationFrame(render);


    };

    // ループを開始
    render();
    



});

