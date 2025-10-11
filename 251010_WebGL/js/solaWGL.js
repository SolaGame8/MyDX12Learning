


class SolaWGL {

    constructor(canvasId) {
        
        // キャンバス要素の取得
        const canvas = document.getElementById(canvasId);

        if (!canvas) {
            console.error(`Canvas ID '${canvasId}' が見つかりません。`);
            return;
        }

        this.canvas = canvas;


        // WebGLコンテキストの初期化
        const gl = canvas.getContext('webgl2') || // ★ この行を追加して、2.0を優先する
           canvas.getContext('webgl') || 
           canvas.getContext('experimental-webgl');
        //const gl = canvas.getContext('webgl') || canvas.getContext('experimental-webgl');



        if (!gl) {
            console.error('WebGLコンテキストの初期化に失敗しました。お使いのブラウザはWebGLに対応していない可能性があります。');
            return;
        }

        this.gl = gl;


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



        // 必要なクラス作成
        this.inputManager = new SolaInputManager(this); 
        //this.mesh = new SolaMesh(this);
        this.textureManager = new SolaTextureManager(this);



        //シェーダーデータ
        this.shaderPrograms = new Map();

        // シェーダーロケーションのプロパティを初期化

        //頂点データ
        this.aPositionLocation = -1;
        this.aTexcoordLocation = -1;
        this.aNormalLocation = -1;
        this.aBoneIDLocation = -1;
        this.aBoneWeightLocation = -1;


        this.uVpMatrixLocation = null;      //カメラの行列
        this.uModelMatrixLocation = null;   //モデル座標


        this.uSamplerLocation = null;   //テクスチャー

        // 現在使用中のプログラムを保持
        this.currentProgram = null; 


        //カメラ

        this.cameraMatrix = mat4.create();  //カメラの変換行列

        this.cameraPosition = vec3.fromValues(0, 0, 5);  // カメラの位置 (ワールド座標)
        this.cameraTarget = vec3.fromValues(0, 0, 0);    // 注視点
        this.cameraUp = vec3.fromValues(0, 1, 0);        // 上方向ベクトル


        // カメラの姿勢を決定するパラメータ
        this.cameraDistance = 5.0; // 注視点 (Target) からの距離
        
        this.cameraAngles = vec3.fromValues(0, 0, 0); // [Yaw(Y軸回転), Pitch(X軸回転), Roll(Z軸回転)]

        // 初期位置を計算
        this.calcCameraPosByDistanceAndAngles();


        // 投影行列設定
        this.fovy = 45 * Math.PI / 180; // 視野角 (Field of View Y) をラジアンで定義
        // this.aspect は計算時に常に更新されます
        this.near = 0.01;                                // ニアクリップ面
        this.far = 1000.0;                              // ファークリップ面

        //ーーーーーーーーーーーーーーーーーーーーーーーー保留

        



        /*
        // 三角形描画用のプロパティ
        this.triangleProgram = null;
        this.triangleVertexBuffer = null;
        this.aPositionLocation = -1;

        //this._initTriangleProgram();

        // テクスチャ用のプロパティを追加
        this.texture = null; 
        this.uSamplerLocation = -1;
*/



    }


    /**
     * シェーダーファイルを非同期で読み込む
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
     */

    /**
     * 非同期の初期化処理
     */

    async init() {


        //デフォルトのシェーダーを読み込んでおく

        const key = "DefaultShader";

        await this.loadShaderProgram(key, './glsl/simple.vs', './glsl/simple.fs');

        this.useShaderProgram(key);

        /*

        // シェーダーソースを非同期で読み込み
        const vsSource = await this._loadShaderSource('./glsl/simple.vs');
        const fsSource = await this._loadShaderSource('./glsl/simple.fs');

        if (!vsSource || !fsSource) {
            console.error("シェーダーソースの読み込みに失敗しました。");
            return false;
        }

        // 三角形描画プログラムの初期化（ソースコードを引数として渡す）
        this._initTriangleProgram(vsSource, fsSource);

        */

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


    setCameraTarget(x, y, z) {
        vec3.set(this.cameraTarget, x, y, z);
    }

    setCameraPosition(x, y, z) {
        vec3.set(this.cameraPosition, x, y, z);
    }
    setCameraAngle(yaw, pitch, roll) {
        vec3.set(this.cameraAngles, yaw, pitch, roll);  // [Yaw(Y軸回転), Pitch(X軸回転), Roll(Z軸回転)]
    }
    setCameraDistance(dist) {
        this.cameraDistance = dist; // 注視点 (Target) からの距離
    }



    /*
        this.cameraDistance = 5.0; // 注視点 (Target) からの距離
        
        this.cameraAngles = vec3.fromValues(0, 0, 0); // [Yaw(Y軸回転), Pitch(X軸回転), Roll(Z軸回転)]
    */

    /**
     * 距離(cameraDistance)と角度(cameraAngles)に基づき、cameraPosition を計算し、設定する。
     */
    calcCameraPosByDistanceAndAngles() {
        // 回転角をローカル変数に格納 
        // Yaw: Y軸回転 (水平方向)
        const yaw = Math.PI * this.cameraAngles[0] / 360.0; 
        // Pitch: X軸回転 (垂直方向)
        const pitch = Math.PI * this.cameraAngles[1] / 360.0;
        
        const L = this.cameraDistance; // cameraLength -> cameraDistance に変更

        // 1. 球面座標系の計算
        const horzDistance = L * Math.cos(pitch);

        const y = L * Math.sin(pitch);
        const x = horzDistance * Math.sin(yaw);
        const z = horzDistance * Math.cos(yaw);
        
        // 2. 注視点 (cameraTarget) からの相対位置として設定
        const target = this.cameraTarget;

        this.cameraPosition[0] = target[0] + x;
        this.cameraPosition[1] = target[1] + y;
        this.cameraPosition[2] = target[2] + z;
    }


    /**
     * View行列 (カメラ行列) を計算して返す。
     * gl-matrix の mat4.lookAt を使用し、カメラの位置、注視点、上方向から View 行列を生成します。
     * @returns {mat4} View 行列
     */
    createViewMatrix() {
        const viewMatrix = mat4.create();
        mat4.lookAt(viewMatrix, this.cameraPosition, this.cameraTarget, this.cameraUp);
        return viewMatrix;
    }

    /**
     * Projection行列 (投影行列) を計算して返す。
     * アスペクト比は常に現在のキャンバスサイズに合わせて更新されます。
     * @returns {mat4} Projection 行列
     */
    createProjectionMatrix() {
        const gl = this.gl;
        const projectionMatrix = mat4.create();
        
        // アスペクト比を現在のキャンバスサイズに合わせて更新
        this.aspect = gl.canvas.width / gl.canvas.height; 

        // mat4.perspective(out, fovy, aspect, near, far)
        mat4.perspective(projectionMatrix, this.fovy, this.aspect, this.near, this.far);
        return projectionMatrix;
    }

    /**
     * View 行列と Projection 行列を乗算して View-Projection (VP) 行列を計算し、返す。
     * 計算順序は Projection * View です。
     * @returns {mat4} View-Projection 行列
     */
    createVPMatrix() {


        const viewMatrix = this.createViewMatrix();
        const projectionMatrix = this.createProjectionMatrix();
        
        const vpMatrix = mat4.create();
        // WebGL/OpenGLでは P * V の順で乗算 (mat4.multiply(out, a, b) => out = a * b)
        mat4.multiply(vpMatrix, projectionMatrix, viewMatrix); 
        
        return vpMatrix;
    }

    /**
     * シェーダーファイルを非同期で読み込むヘルパー関数
     */

    async _loadShaderFile(url) {

        const response = await fetch(url);

        if (!response.ok) {
            throw new Error(`Failed to load shader file: ${url}`);
        }
        return response.text();

    }

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
     * プログラムから全ての共通ユニフォーム・アトリビュートロケーションを取得し、SolaWGLのプロパティに保存する。
     * @param {WebGLProgram} program - 対象のWebGLプログラム
     */


    _getProgramLocations(program) {

        const gl = this.gl;
        const locations = {};

        // 取得処理を関数化して、ログ出力を追加
        const getAttribLocationLogged = (name) => {
            const loc = gl.getAttribLocation(program, name);
            console.log(`[Shader Check] Attribute '${name}': ${loc} (${loc !== -1 ? '成功' : '失敗'})`);
            return loc;
        };

        const getUniformLocationLogged = (name) => {
            const loc = gl.getUniformLocation(program, name);
            // UniformLocationは取得失敗時 null を返す
            console.log(`[Shader Check] Uniform '${name}': ${loc} (${loc !== null ? '成功' : '失敗'})`);
            return loc;
        };

        // アトリビュート (Meshデータの属性)
        locations.aPositionLocation = getAttribLocationLogged('a_position');
        locations.aTexcoordLocation = getAttribLocationLogged('a_texcoord');
        locations.aNormalLocation = getAttribLocationLogged('a_normal');
        locations.aBoneIDLocation = getAttribLocationLogged('a_boneID');
        locations.aBoneWeightLocation = getAttribLocationLogged('a_boneWeight');

        // ユニフォーム (行列やテクスチャ)
        locations.uModelMatrixLocation = getUniformLocationLogged('u_modelMatrix');
        locations.uVpMatrixLocation = getUniformLocationLogged('u_vpMatrix');
        locations.uSamplerLocation = getUniformLocationLogged('u_sampler');

        return locations;
    }


    /*
    _getProgramLocations(program) {

        const gl = this.gl;
        

        return {
                // アトリビュート (Meshデータの属性)
                aPositionLocation: gl.getAttribLocation(program, 'a_position'),
                aTexcoordLocation: gl.getAttribLocation(program, 'a_texcoord'),
                aNormalLocation: gl.getAttribLocation(program, 'a_normal'),
                aBoneIDLocation: gl.getAttribLocation(program, 'a_boneID'),
                aBoneWeightLocation: gl.getAttribLocation(program, 'a_boneWeight'),

                // ユニフォーム (行列やテクスチャ)
                uModelMatrixLocation: gl.getUniformLocation(program, 'u_modelMatrix'),
                uVpMatrixLocation: gl.getUniformLocation(program, 'u_vpMatrix'),
                uSamplerLocation: gl.getUniformLocation(program, 'u_sampler')
            };


    }
*/


    /**
     * ★ 新規: シェーダーファイルをロードし、コンパイル・リンクしてMapに登録する
     * @param {string} key - Mapに登録するシェーダープログラムのキー
     * @param {string} vsUrl - 頂点シェーダーファイルのURL
     * @param {string} fsUrl - フラグメントシェーダーファイルのURL
     * @returns {Promise<WebGLProgram | null>}
     */


    async loadShaderProgram(key, vsUrl, fsUrl) {

        if (!this.gl) return null;

        if (this.shaderPrograms.has(key)) {
            console.warn(`Shader key "${key}" is already registered.`);
            return this.shaderPrograms.get(key);
        }

        try {
            const vsSource = await this._loadShaderFile(vsUrl);
            const fsSource = await this._loadShaderFile(fsUrl);
            
            const program = this._createProgram(vsSource, fsSource);
            
            if (program) {

                const locations = this._getProgramLocations(program); 

                //this.shaderPrograms.set(key, program);
                this.shaderPrograms.set(key, { program: program, locations: locations });

                console.log(`Shader program "${key}" loaded successfully.`);

            }
            return program;

        } catch (error) {
            console.error(error.message);
            return null;
        }
    }

    /**
     * シェーダープログラムのキーを使って、プログラムを切り替える。
     * @param {string} key - 使用したいシェーダープログラムのキー
     * @returns {boolean} - 切り替えに成功したかどうか
     */
    useShaderProgram(key) {

        if (!this.gl) return false;

        if (key == null || key === "Default" || key === "default") {
            key = "DefaultShader";
        }


        const entry = this.shaderPrograms.get(key);
        if (!entry) return false;
    
        if (this.uVpMatrixLocation != null) {

            // カメラの行列の設定
            this.cameraMatrix = this.createVPMatrix();

            //console.log("[Camera Check] VP Matrix (cameraMatrix):", this.cameraMatrix);
            this.gl.uniformMatrix4fv(this.uVpMatrixLocation, false, this.cameraMatrix);
        }


        const program = entry.program;
        const locations = entry.locations;

        if (program && program !== this.currentProgram) {

            this.gl.useProgram(program);

            // CULL_FACE 機能を無効化する
            this.gl.disable(this.gl.CULL_FACE);

            // 通常はデプスバッファを有効化（描画の前後関係を正しく処理するため）
            this.gl.enable(this.gl.DEPTH_TEST);
            //this.gl.disable(this.gl.DEPTH_TEST);

            // ロケーションをコピー
            this.aPositionLocation = locations.aPositionLocation;
            this.aTexcoordLocation = locations.aTexcoordLocation;
            this.aNormalLocation = locations.aNormalLocation;
            this.aBoneIDLocation = locations.aBoneIDLocation;
            this.aBoneWeightLocation = locations.aBoneWeightLocation;

            this.uModelMatrixLocation = locations.uModelMatrixLocation;
            this.uVpMatrixLocation = locations.uVpMatrixLocation;
            this.uSamplerLocation = locations.uSamplerLocation;
            
            this.currentProgram = program;





            return true;
        }

        /*
        const program = this.shaderPrograms.get(key);

        if (program && program !== this.currentProgram) {
            this.gl.useProgram(program);
            this._getProgramLocations(program); // 新しいプログラムのロケーションを取得
            this.currentProgram = program;
            return true;
        }

        if (!this.currentProgram) {
            console.warn(`Cannot draw: Shader program "${shaderKey}" is not bound.`);
        }
*/

        return false;
    }
    

    /**
     * 三角形描画に必要なリソースを初期化する（初回のみ実行）
    _initTriangleProgram(vsSource, fsSource) {
        const gl = this.gl;



        // 2. プログラム作成
        this.triangleProgram = this._createProgram(vsSource, fsSource);
        if (!this.triangleProgram) return;

        // 3. 属性とユニフォームの位置を取得
        this.aPositionLocation = gl.getAttribLocation(this.triangleProgram, 'a_position');
        this.aTexcoordLocation = gl.getAttribLocation(this.triangleProgram, 'a_texcoord');
        this.uTranslationLocation = gl.getUniformLocation(this.triangleProgram, 'u_translation');
        this.uScaleLocation = gl.getUniformLocation(this.triangleProgram, 'u_scale');

        this.uSamplerLocation = gl.getUniformLocation(this.triangleProgram, 'u_sampler'); 
        


        // 左右 -1.0 ～ 1.0, 上下 1.0 ～ -1.0
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
     */

/*
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
                gl.generateMipmap(gl.TEXTURE_2D);   //MipMap生成

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
*/



    /**
     * 新規追加: 三角形を描画する
     * @param {{x: number, y: number}} position - 画面中央(0,0)からの相対座標(-1.0から1.0)
     * @param {number} scale - スケール
     */
    /*
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

        gl.vertexAttribPointer( //位置
            this.aPositionLocation, 
            2,            // 2つの要素 (x, y)
            gl.FLOAT,     // データの型
            false,        // 正規化しない
            STRIDE,            // ストライド (0 = 連続データ)
            0             // オフセット
        );


        gl.enableVertexAttribArray(this.aTexcoordLocation);
        gl.vertexAttribPointer( //UV
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
    */


    /**
     * 【修正】描画のメインメソッド
     * @param {SolaMesh} mesh - 描画するSolaMeshオブジェクト
     * @param {string} shaderKey - 使用するシェーダープログラムのキー
     */

    /*
    //drawMesh(mesh, shaderKey) { 
    drawMesh(mesh) { 

        if (!this.gl || !mesh) return;
        const gl = this.gl;
        
        // シェーダープログラムをキーで切り替え
        //this.useShaderProgram(shaderKey);
        

        
        // SolaTextureManagerを使ってテクスチャをバインド ---

        const textureKey = mesh.textureKey;

        if (this.textureManager && textureKey) {
            const webglTexture = this.textureManager.get(textureKey);
            if (webglTexture) {
                gl.activeTexture(gl.TEXTURE0);
                gl.bindTexture(gl.TEXTURE_2D, webglTexture);
                // サンプラーユニフォームは、シェーダー切り替え時にロケーションを取得済み
                gl.uniform1i(this.uSamplerLocation, 0); 
            }
        }
        
        // 3. SolaMeshオブジェクトにバインディングとModel行列計算を委譲
        mesh.draw(this);



        // 5. 描画コマンドの実行
        gl.drawArrays(gl.TRIANGLES, 0, mesh.vertexCount);
        
        // 描画後はバッファをアンバインド
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
        gl.bindTexture(gl.TEXTURE_2D, null);
    }
    */




    _onResize() {
        this.resizeCanvas();
        //this.draw(); // リサイズ後も画面をクリア（再描画）
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
        let dTime = currentTime - this._lastRenderTime;

        // 規定の時間（16.67ms）が経過していない場合、描画をスキップ
        if (dTime < this._frameInterval) {
            return false; 
        }


        this.deltaTime = dTime;

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

    
    clearCanvas() {

        if (!this.gl) return;

        //if (!this.needDraw) return;


        const gl = this.gl;

        // gl.clearColor() でクリア色をセット
        gl.clearColor(this.clearColor[0], this.clearColor[1], this.clearColor[2], this.clearColor[3]);

        // カラーバッファをクリア
        gl.clear(gl.COLOR_BUFFER_BIT);


        

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
        if (this.mesh) {
            this.mesh.onDestroy();
            this.mesh = null;
        }
        if (this.textureManage) {
            this.textureManage.onDestroy();
            this.textureManage = null;
        }



        // 変数・オブジェクトの参照の破棄
        this.gl = null;
        this.canvas = null;
        this.clearColor = null;
    }


}



