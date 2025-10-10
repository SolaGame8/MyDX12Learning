


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
    
    

    // テクスチャのロードと完了を待つ
    await wgl.loadTexture('./images/my_texture.png'); 




    wgl.setFpsLimit(80);

    //wgl.update();//情報の更新

    wgl.setClearColor(0.6, 0.8, 0.9, 1.0);  //クリア色の設定 (R, G, B, A)

    //wgl.draw();

    //console.log("WebGLコンテキストが初期化され、キャンバスがクリアされました。");

    
    // 2. メッセージボックスが繰り返し表示されるのを防ぐための状態変数
    let wasAPressed = false; 


    let triangleX = 0.0;
    let triangleY = 0.0;
    let scale = 1.0;


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
            triangleX = stickL.x;
            triangleY = -stickL.y; // WebGLのY軸は上が正なのでY軸を反転
            
            /*
            if (stickL.x > 0.0 && stickL.y > 0.0) {
                //console.log('Left Stick X:', stickL.x, 'Left Stick Y:', stickL.y);
            }
    */


            // --- Aボタン入力チェック ---
            if (typeof BUTTONS !== 'undefined') { // BUTTONS定数が読み込まれているかチェック
                const isAPressed = wgl.inputManager.getGamepadOnPress(BUTTONS.A);

                // Aボタンが押された瞬間 (前回 false -> 今回 true) のみ処理を実行
                if (isAPressed && !wasAPressed) {
                //if (isAPressed) {
                    alert("Aボタンが押されました！");
                }

                // 次のフレームのために現在の状態を保持
                wasAPressed = isAPressed;
            }
            // ------------------------------

            // 描画処理 (画面クリア)
            wgl.draw(triangleX, triangleY, scale);

            //wgl.drawTriangle({x: triangleX, y: triangleY}, scale);
        }

        // 次の描画フレームを要求 (ループの継続)
        requestAnimationFrame(render);


    };

    // ループを開始
    render();
    



});

