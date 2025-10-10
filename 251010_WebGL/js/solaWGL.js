class SolaWGL {

    constructor(canvasId) {
        
        // キャンバス要素の取得
        const canvas = document.getElementById(canvasId);
        if (!canvas) {
            console.error(`Canvas ID '${canvasId}' が見つかりません。`);
            return;
        }

        // WebGLコンテキストの初期化
        const gl = canvas.getContext('webgl') || canvas.getContext('experimental-webgl');
        if (!gl) {
            console.error('WebGLコンテキストの初期化に失敗しました。お使いのブラウザはWebGLに対応していない可能性があります。');
            return;
        }

        this.gl = gl;
        this.canvas = canvas;

        // FPS 制御用のプロパティを追加
        this._lastRenderTime = performance.now();
        this._frameInterval = 1000 / 60;  // 60FPS (約 16.666ms)

        this.FPS_SAMPLE_COUNT = 10; // 平均を計算するサンプル数
        this.deltaTimeHistory = []; 


        this.deltaTime = 0.0;
        this.needDraw = false;

        this.clearColor = [0.0, 0.0, 0.0, 1.0];     // 既定値は黒(0, 0, 0, 1)
        
        // 画面サイズに合わせてビューポートを設定
        this.resizeCanvas();


        // イベントリスナーを登録
        
        this._resizeHandler = this._onResize.bind(this);
        window.addEventListener('resize', this._resizeHandler);

        // キーボード関連の処理を InputManager に委譲
        this.inputManager = new SolaInputManager(this); 



        // 三角形描画用のプロパティ
        this.triangleProgram = null;
        this.triangleVertexBuffer = null;
        this.aPositionLocation = -1;

        //this._initTriangleProgram();

        // テクスチャ用のプロパティを追加
        this.texture = null; 
        this.uSamplerLocation = -1;



    }

    /**
     * シェーダーファイルを非同期で読み込む
     */
    async _loadShaderSource(url) {
        try {
            const response = await fetch(url);
            if (!response.ok) {
                throw new Error(`Failed to load shader: ${url} (${response.statusText})`);
            }
            return response.text();
        } catch (e) {
            console.error(e);
            return null;
        }
    }

    /**
     * 非同期の初期化処理
     */

    async init() {

        // シェーダーソースを非同期で読み込み
        const vsSource = await this._loadShaderSource('./glsl/simple.vs');
        const fsSource = await this._loadShaderSource('./glsl/simple.fs');

        if (!vsSource || !fsSource) {
            console.error("シェーダーソースの読み込みに失敗しました。");
            return false;
        }

        // 三角形描画プログラムの初期化（ソースコードを引数として渡す）
        this._initTriangleProgram(vsSource, fsSource);

        // InputManagerの初期化
        /*
        if (typeof SolaInputManager !== 'undefined') {
            this.inputManager = new SolaInputManager(this); 
        }
        */

        //WebGLコンテキストが作成されているかの確認
        if (this.gl) {
            return true;
        }
        return false;
    }

    /*
    async isReady() {


        //const initSuccess = await this.initAsync();
        //const flg = await this.init();

        if (this.gl) {
            return true;
        }
        return false;
    }
*/

    /**
     * シェーダーをコンパイルする
     */
    _compileShader(type, source) {
        const gl = this.gl;
        const shader = gl.createShader(type);
        gl.shaderSource(shader, source);
        gl.compileShader(shader);

        if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
            console.error('シェーダーのコンパイルエラー:', gl.getShaderInfoLog(shader));
            gl.deleteShader(shader);
            return null;
        }
        return shader;
    }

    /**
     * プログラムを作成する
     */
    _createProgram(vsSource, fsSource) {
        const gl = this.gl;
        const vertexShader = this._compileShader(gl.VERTEX_SHADER, vsSource);
        const fragmentShader = this._compileShader(gl.FRAGMENT_SHADER, fsSource);

        if (!vertexShader || !fragmentShader) return null;

        const program = gl.createProgram();
        gl.attachShader(program, vertexShader);
        gl.attachShader(program, fragmentShader);
        gl.linkProgram(program);

        if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
            console.error('プログラムのリンクエラー:', gl.getProgramInfoLog(program));
            gl.deleteProgram(program);
            return null;
        }
        return program;
    }

    /**
     * 三角形描画に必要なリソースを初期化する（初回のみ実行）
     */
    _initTriangleProgram(vsSource, fsSource) {
        const gl = this.gl;

        /*
        // 1. シェーダーソース
        const vsSource = `
            attribute vec4 a_position;
            uniform vec2 u_translation;
            uniform float u_scale;
            void main() {
                gl_Position = vec4(a_position.xy * u_scale + u_translation, 0.0, 1.0);
            }
        `;
        const fsSource = `
            precision mediump float;
            void main() {
                gl_FragColor = vec4(1, 0, 0, 1); // 赤色
            }
        `;
        */

        // 2. プログラム作成
        this.triangleProgram = this._createProgram(vsSource, fsSource);
        if (!this.triangleProgram) return;

        // 3. 属性とユニフォームの位置を取得
        this.aPositionLocation = gl.getAttribLocation(this.triangleProgram, 'a_position');
        this.aTexcoordLocation = gl.getAttribLocation(this.triangleProgram, 'a_texcoord');
        this.uTranslationLocation = gl.getUniformLocation(this.triangleProgram, 'u_translation');
        this.uScaleLocation = gl.getUniformLocation(this.triangleProgram, 'u_scale');

        this.uSamplerLocation = gl.getUniformLocation(this.triangleProgram, 'u_sampler'); 
        

        // 頂点データ: (位置X, 位置Y, UV_U, UV_V) のセット
        const positions = new Float32Array([
            0.0,  0.1,  0.5, 1.0,  // 頂点 (U=0.5, V=1.0)
            -0.1, -0.1,  0.0, 0.0,  // 左下 (U=0.0, V=0.0)
            0.1, -0.1,  1.0, 0.0,  // 右下 (U=1.0, V=0.0)
        ]);
        this.triangleVertexBuffer = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, this.triangleVertexBuffer);
        gl.bufferData(gl.ARRAY_BUFFER, positions, gl.STATIC_DRAW);
    }


    async loadTexture(url) {
        const gl = this.gl;
        
        // ご自身の texture-util.js を使用する場合は、この Promise ブロックを置き換えてください
        return new Promise(resolve => {
            const image = new Image();
            image.onload = () => {
                const texture = gl.createTexture();
                gl.bindTexture(gl.TEXTURE_2D, texture);

                // Y軸反転設定
                gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
                
                // テクスチャに画像データをコピー
                gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, image);

                // ミップマップとフィルタリング設定
                gl.generateMipmap(gl.TEXTURE_2D);
                gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_LINEAR);
                gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
                gl.bindTexture(gl.TEXTURE_2D, null);

                this.texture = texture; // インスタンスに保存
                resolve(texture);
            };
            image.onerror = () => {
                console.error(`Failed to load image: ${url}`);
                resolve(null);
            };
            image.src = url;
        });
    }



    /**
     * 新規追加: 三角形を描画する
     * @param {{x: number, y: number}} position - 画面中央(0,0)からの相対座標(-1.0から1.0)
     * @param {number} scale - スケール
     */
    drawTriangle(position = {x: 0, y: 0}, scale = 1.0) {
        if (!this.gl) return;

        // 初回実行時に初期化を行う
        if (!this.triangleProgram) {
            this._initTriangleProgram();
            if (!this.triangleProgram) return; // 初期化失敗
        }
        
        const gl = this.gl;

        gl.useProgram(this.triangleProgram);

        // 属性(a_position)の設定

        const STRIDE = 4 * Float32Array.BYTES_PER_ELEMENT; // 1頂点あたり (X, Y, U, V) で 16バイト

        gl.bindBuffer(gl.ARRAY_BUFFER, this.triangleVertexBuffer);
        gl.enableVertexAttribArray(this.aPositionLocation);
        gl.vertexAttribPointer(
            this.aPositionLocation, 
            2,            // 2つの要素 (x, y)
            gl.FLOAT,     // データの型
            false,        // 正規化しない
            STRIDE,            // ストライド (0 = 連続データ)
            0             // オフセット
        );


        gl.enableVertexAttribArray(this.aTexcoordLocation);
        gl.vertexAttribPointer(
            this.aTexcoordLocation,
            2,            // 2つの要素 (U, V)
            gl.FLOAT,
            false,
            STRIDE,       // ★ ストライド: 16バイトに変更
            2 * Float32Array.BYTES_PER_ELEMENT // ★ オフセット: 位置 (X, Y) の 8バイト後から開始
        );

        // ユニフォーム変数の設定
        gl.uniform2f(this.uTranslationLocation, position.x, position.y);
        gl.uniform1f(this.uScaleLocation, scale);


        // テクスチャ処理を追加
        if (this.texture) {
            // 1. テクスチャユニット0をアクティブ化
            gl.activeTexture(gl.TEXTURE0);
            // 2. テクスチャをバインド
            gl.bindTexture(gl.TEXTURE_2D, this.texture);
            // 3. フラグメントシェーダーの u_sampler にユニット0（最初のテクスチャ）を設定
            gl.uniform1i(this.uSamplerLocation, 0); 
        }


        // 描画
        gl.drawArrays(gl.TRIANGLES, 0, 3);
    }




    _onResize() {
        this.resizeCanvas();
        this.draw(); // リサイズ後も画面をクリア（再描画）
    }




    /**
     * キャンバスのクライアントサイズに合わせて内部解像度(width/height)を更新し、
     * WebGLのビューポートを設定する。
     */
    resizeCanvas() {
        const displayWidth  = this.canvas.clientWidth;
        const displayHeight = this.canvas.clientHeight;

        if (this.canvas.width !== displayWidth || this.canvas.height !== displayHeight) {
            this.canvas.width = displayWidth;
            this.canvas.height = displayHeight;
            if (this.gl) {
                this.gl.viewport(0, 0, this.gl.canvas.width, this.gl.canvas.height);
            }
        }
    }


    update() {

        if (!this.gl) return false;

        
        if (!this.checkFPS()) {
            return false;
        }
        this.needDraw = true;


        // InputManager の状態更新（ゲームパッドのボタン状態の保存）
        if (this.inputManager && typeof this.inputManager.update === 'function') {
            this.inputManager.update();
        }
        
        // ここに将来、モデルの更新、物理計算などのロジックを追加する


        return true;
    }



    setClearColor(r, g, b, a) {
        this.clearColor = [r, g, b, a];
    }



    checkFPS() {

        if (!this.gl) return false;

        const currentTime = performance.now();
        this.deltaTime = currentTime - this._lastRenderTime;

        // 規定の時間（16.67ms）が経過していない場合、描画をスキップ
        if (this.deltaTime < this._frameInterval) {
            return false; 
        }

        // 経過時間（deltaTime）を調整し、次のフレームでの遅延を補正
        this._lastRenderTime = currentTime - (this.deltaTime % this._frameInterval); 
        

        // 配列に最新のFPS値を追加
        this.deltaTimeHistory.push(this.deltaTime); 
        
        // サンプル数を超えた場合、配列の先頭（最も古いデータ）を削除
        if (this.deltaTimeHistory.length > this.FPS_SAMPLE_COUNT) {
            this.deltaTimeHistory.shift(); 
        }

        return true; // 描画実行を許可
    }

    getDeltaTime() {

        return this.deltaTime / 1000.0;
    }

    setFpsLimit(fps) {

        this._frameInterval = 1000 / fps;
    }

    getFps() {

        const totalDeltaTime = this.deltaTimeHistory.reduce((sum, fps) => sum + fps, 0);
        const averageDeltaTime = Math.round(totalDeltaTime / this.deltaTimeHistory.length);

        return Math.round(1.0/averageDeltaTime * 1000);
    }


    draw(triangleX, triangleY, scale) {

        if (!this.gl) return;

        //if (!this.needDraw) return;


        const gl = this.gl;

        // gl.clearColor() でクリア色をセット
        gl.clearColor(this.clearColor[0], this.clearColor[1], this.clearColor[2], this.clearColor[3]);

        // カラーバッファをクリア
        gl.clear(gl.COLOR_BUFFER_BIT);


        this.drawTriangle({x: triangleX, y: triangleY}, scale);

        this.needDraw = false;


    }


    /**
     * 登録したイベントリスナーの解除と、保持している変数の破棄を行う。
     */

    onDestroy() {

        // イベントリスナーの解除
        if (this._resizeHandler) {
            window.removeEventListener('resize', this._resizeHandler);
            this._resizeHandler = null;
        }

        if (this.inputManager) {
            this.inputManager.onDestroy();
            this.inputManager = null;
        }

        // 変数・オブジェクトの参照の破棄
        this.gl = null;
        this.canvas = null;
        this.clearColor = null;
    }


}



