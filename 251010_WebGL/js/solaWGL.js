


const mat4 = glMatrix.mat4; 
const vec2 = glMatrix.vec2; 
const vec3 = glMatrix.vec3; 
const vec4 = glMatrix.vec4; 
const quat = glMatrix.quat;


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
        //サウンド
        this.soundManager = new SolaSoundManager();

        //glTF
        this.gltfParser = new SolaGltfParser(this);

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
        this.uBoneMatricesLocation  = null;   //ボーン

        this.uGenericDataLocation = null;

        this.uLightDirectionLocation = null;
        this.uLightColorLocation = null;
        this.uLightIntensityLocation = null;
        this.uAmbientColorLocation = null;
        this.uAmbientIntensityLocation = null;



        this.uSamplerLocation = null;   //テクスチャー

        // 現在使用中のプログラムを保持
        this.currentProgram = null; 




        //汎用シェーダー変数
        this.genericArray = new Float32Array(16 * 4);


        //ライト
        this.lightDirection = new Float32Array([-0.5, 1.0, 0.5]);
        this.lightColor = new Float32Array([1.0, 1.0, 0.8]);
        this.lightIntensity = 1.8;
        this.ambientColor = new Float32Array([0.5, 0.5, 0.8]);
        this.ambientIntensity = 0.7;

        //ボーンの単位行列で初期化するための配列
        this._identityBoneMatrixArray = this._createIdentityBoneMatrixArray();
        this.flg_identityBoneMat = false;   //最初に一回だけ、ボーンの行列を単位行列で初期化

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




    }



    /**
     * 非同期の初期化処理
     */

    async init() {


        //デフォルトのシェーダーを読み込んでおく

        const key = "DefaultShader";

        await this.loadShaderProgram(key, './glsl/simple.vs', './glsl/simple.fs');

        this.useShaderProgram(key);



        //WebGLコンテキストが作成されているかの確認
        if (this.gl) {
            return true;
        }
        return false;
    }


    // 配列インデックス 8 の、1番目の要素 (Y成分またはG成分) に 0.9 を設定
    //setGenericArray(8, 1, 0.9); 

    setShaderGenericArray(index, componentIndex, value) { //index: 0-15, componentIndex: 0-3, value: float
        
        // インデックスの検証 (0から15の範囲か)
        if (index < 0 || index >= 16 || !Number.isInteger(index)) {
            console.error(`setGenericArrayElementComponent: 無効な配列インデックスです: ${index} (0から15の整数である必要があります)。`);
            return;
        }

        // 成分インデックスの検証 (0から3の範囲か)
        if (componentIndex < 0 || componentIndex >= 4 || !Number.isInteger(componentIndex)) {
            console.error(`setGenericArrayElementComponent: 無効な成分インデックスです: ${componentIndex} (0, 1, 2, 3 のいずれかである必要があります)。`);
            return;
        }
        
        // 値の検証 (数値か)
        if (typeof value !== 'number') {
             console.error(`setGenericArrayElementComponent: 設定値は数値である必要があります。渡された値: ${value}`);
            return;
        }

        // データの設定
        const absoluteIndex = (index * 4) + componentIndex; 
        
        this.genericArray[absoluteIndex] = value;


    }



    /**
     * 光の方向 (vec3) を設定します。
     * @param {number} x - X成分。
     * @param {number} y - Y成分。
     * @param {number} z - Z成分。
     */
    setLightDirection(x, y, z) {
        if (typeof x !== 'number' || typeof y !== 'number' || typeof z !== 'number') {
            console.error("setLightDirection: x, y, z は数値である必要があります。");
            return;
        }
        // 既存のFloat32Arrayの各要素を個別に更新
        this.lightDirection[0] = x;
        this.lightDirection[1] = y;
        this.lightDirection[2] = z;
    }

    /**
     * 光の色 (vec3) を設定します。
     * @param {number} r - R成分。
     * @param {number} g - G成分。
     * @param {number} b - B成分。
     */
    setLightColor(r, g, b) {
        if (typeof r !== 'number' || typeof g !== 'number' || typeof b !== 'number') {
            console.error("setLightColor: r, g, b は数値である必要があります。");
            return;
        }
        // 既存のFloat32Arrayの各要素を個別に更新
        this.lightColor[0] = r;
        this.lightColor[1] = g;
        this.lightColor[2] = b;
    }

    /**
     * 環境光の色 (vec3) を設定します。
     * @param {number} r - R成分。
     * @param {number} g - G成分。
     * @param {number} b - B成分。
     */
    setAmbientColor(r, g, b) {
        if (typeof r !== 'number' || typeof g !== 'number' || typeof b !== 'number') {
            console.error("setAmbientColor: r, g, b は数値である必要があります。");
            return;
        }
        // 既存のFloat32Arrayの各要素を個別に更新
        this.ambientColor[0] = r;
        this.ambientColor[1] = g;
        this.ambientColor[2] = b;
    }

    /**
     * 光の強度 (float) を設定します。
     * @param {number} newIntensity - 新しい強度値。
     */
    setLightIntensity(newIntensity) {
        if (typeof newIntensity !== 'number') {
            console.error("setLightIntensity: 数値を渡してください。");
            return;
        }
        this.lightIntensity = newIntensity;
    }

    /**
     * 環境光の強度 (float) を設定します。
     * @param {number} newIntensity - 新しい強度値。
     */
    setAmbientIntensity(newIntensity) {
        if (typeof newIntensity !== 'number') {
            console.error("setAmbientIntensity: 数値を渡してください。");
            return;
        }
        this.ambientIntensity = newIntensity;
    }





    setCameraTarget(x, y, z) {
        vec3.set(this.cameraTarget, x, y, z);
    }

    setCameraPosition(x, y, z) {
        vec3.set(this.cameraPosition, x, y, z);
    }
    setCameraAngle(pitch, yaw, roll) {  //x:pitch y:yaw roll:z
        vec3.set(this.cameraAngles, pitch, yaw, roll);  // [Yaw(Y軸回転), Pitch(X軸回転), Roll(Z軸回転)]
    }
    setCameraDistance(dist) {
        this.cameraDistance = dist; // 注視点 (Target) からの距離
    }

    getCameraPosition() {

        return this.cameraPosition;

    }

    /*
        this.cameraDistance = 5.0; // 注視点 (Target) からの距離
        
        this.cameraAngles = vec3.fromValues(0, 0, 0); // [Yaw(Y軸回転), Pitch(X軸回転), Roll(Z軸回転)]
    */

    /**
     * 距離(cameraDistance)と角度(cameraAngles)に基づき、cameraPosition を計算し、設定する。
     */
    calcCameraPosByDistanceAndAngles() {


        //x:pitch y:yaw roll:z
        // 回転角をローカル変数に格納 
        // Yaw: Y軸回転 (水平方向)
        const yaw = Math.PI * this.cameraAngles[1] / 180.0; 
        // Pitch: X軸回転 (垂直方向)
        const pitch = Math.PI * this.cameraAngles[0] / 180.0;
        
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
        locations.uBoneMatricesLocation = getUniformLocationLogged('u_boneMatrices');

        locations.uSamplerLocation = getUniformLocationLogged('u_sampler');


        locations.uGenericDataLocation = getUniformLocationLogged('u_genericArray');

        locations.uLightDirectionLocation = getUniformLocationLogged('u_lightDirection');
        locations.uLightColorLocation = getUniformLocationLogged('u_lightColor');
        locations.uLightIntensityLocation = getUniformLocationLogged('u_lightIntensity');
        locations.uAmbientColorLocation = getUniformLocationLogged('u_ambientColor');
        locations.uAmbientIntensityLocation = getUniformLocationLogged('u_ambientIntensity');





        return locations;
    }





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


        if (this.uGenericDataLocation != null) {
            this.gl.uniform4fv(this.uGenericDataLocation,this.genericArray, false, 16);
        }

        if (this.uLightDirectionLocation != null) {
            this.gl.uniform3fv(this.uLightDirectionLocation, this.lightDirection);
        }
        if (this.uLightColorLocation != null) {
            this.gl.uniform3fv(this.uLightColorLocation, this.lightColor);
        }
        if (this.uLightIntensityLocation != null) {
            this.gl.uniform1f(this.uLightIntensityLocation, this.lightIntensity);
        }

        if (this.uAmbientColorLocation != null) {
            this.gl.uniform3fv(this.uAmbientColorLocation, this.ambientColor);
        }
        if (this.uAmbientIntensityLocation != null) {
            this.gl.uniform1f(this.uAmbientIntensityLocation, this.ambientIntensity);
        }

        if (this.uBoneMatricesLocation != null) {
            if (!this.flg_identityBoneMat) {
                this.clearBoneMatrices();   //最初だけシェーダーのボーン行列データを、正規行列で初期化
                this.flg_identityBoneMat = true;
            }
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
            this.uBoneMatricesLocation = locations.uBoneMatricesLocation;
            this.uSamplerLocation = locations.uSamplerLocation;

            this.uGenericDataLocation = locations.uGenericDataLocation;

            this.uLightDirectionLocation = locations.uLightDirectionLocation;
            this.uLightColorLocation = locations.uLightColorLocation;
            this.uLightIntensityLocation = locations.uLightIntensityLocation;
            this.uAmbientColorLocation = locations.uAmbientColorLocation;
            this.uAmbientIntensityLocation = locations.uAmbientIntensityLocation;

            


            this.currentProgram = program;





            return true;
        }



        return false;
    }
    

    /**
     * 内部使用：シェーダーの最大ボーン数に対応する単位行列のFloat32Arrayを作成
     * @returns {Float32Array} - MAX_BONES * 16 のサイズの単位行列配列
     */
    _createIdentityBoneMatrixArray() {
        const MAX_BONES = 128;
        const MATRIX_SIZE = 16;
        const identityArray = new Float32Array(MAX_BONES * MATRIX_SIZE);
        const identityMatrix = mat4.create(); // gl-matrixの単位行列 (1,0,0,0, 0,1,0,0, ...)
        
        // すべてのボーン位置に単位行列を設定
        for (let i = 0; i < MAX_BONES; i++) {
            const offset = i * MATRIX_SIZE;
            // set関数は、指定されたオフセットから、コピー元のすべての要素をコピーします
            identityArray.set(identityMatrix, offset); 
        }
        return identityArray;
    }

    /**
     * 🚨 【新規関数】シェーダーの u_boneMatrices を最大数まで単位行列で初期化（クリア）する
     */
    clearBoneMatrices() {

        /*
        if (!this._identityBoneMatrixArray) {
            // 単位行列配列を初回のみ生成し、キャッシュ
            this._identityBoneMatrixArray = this._createIdentityBoneMatrixArray();
        }
        */

        // 最初の行列の16成分を確認
        const firstMatrix = this._identityBoneMatrixArray.subarray(0, 16);
        console.log("🚨Identity Matrix Check (First 16 elements):", firstMatrix);

        // setBoneAnimationMat関数を使ってGPUに転送
        this.setBoneAnimationMat(this._identityBoneMatrixArray);

        console.log(`🚨[clearBoneMatrices] u_boneMatricesを単位行列でクリアしました`);
    }

    /**
     * ボーンアニメーション行列配列をシェーダーユニフォームに渡す
     * @param {Float32Array} matArray - アニメーション行列 (16成分 * Nボーン)
     */
    setBoneAnimationMat(matArray) {

        if (!this.gl || this.uBoneMatricesLocation === null) {
            console.warn("🚨[setBoneAnimationMat] WebGLコンテキストまたはユニフォームロケーションが設定されていません。");
            return;
        }

        if (!(matArray instanceof Float32Array)) {
            console.error("🚨[setBoneAnimationMat] matArrayはFloat32Arrayである必要があります。");
            return;
        }

        //console.log(`🚨[setBoneAnimationMat] ボーン行列 ${matArray.length / 16} 個をアップロード開始`);
        
        const MAX_BONES = 128;
        const MATRIX_SIZE = 16;

        // 現在の行列数
        const currentMatrixCount = matArray.length / MATRIX_SIZE;
        
        let dataToUpload = matArray;

        // 最大ボーン数を超えた場合の処理
        if (currentMatrixCount > MAX_BONES) {
            
            // 最大数に合わせて配列を切り捨てる
            const maxElements = MAX_BONES * MATRIX_SIZE;
            
            // subarray() で新しい Float32ArrayView を作成し、転送データを制限
            dataToUpload = matArray.subarray(0, maxElements); 
            
            console.warn(`[setBoneAnimationMat] ボーン行列数が最大値(${maxMatrixCount})を超えました。${currentMatrixCount}個から${maxMatrixCount}個に制限しました。`);

        }

        // uniformMatrix4fvを使って行列配列をアップロード
        this.gl.uniformMatrix4fv(this.uBoneMatricesLocation, false, dataToUpload);
        
        //console.log(`🚨[setBoneAnimationMat] ボーン行列 ${matArray.length / 16} 個をアップロードしました。`);



    }







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
     * フルスクリーン表示とウィンドウ表示を切り替える。
     */
    
    toggleFullscreen() {
        const doc = document;
        const fullscreenElement = doc.fullscreenElement || doc.mozFullScreenElement || doc.webkitFullscreenElement || doc.msFullscreenElement;

        if (fullscreenElement) {
            // フルスクリーンを解除する
            if (doc.exitFullscreen) {
                doc.exitFullscreen();
            } else if (doc.mozCancelFullScreen) {
                doc.mozCancelFullScreen();
            } else if (doc.webkitExitFullscreen) {
                doc.webkitExitFullscreen();
            } else if (doc.msExitFullscreen) {
                doc.msExitFullscreen();
            }
        } else {
            // キャンバス要素をフルスクリーンにする
            const element = this.canvas;
            if (element.requestFullscreen) {
                element.requestFullscreen();
            } else if (element.mozRequestFullScreen) {
                element.mozRequestFullScreen();
            } else if (element.webkitRequestFullscreen) {
                element.webkitRequestFullscreen();
            } else if (element.msRequestFullscreen) {
                element.msRequestFullscreen();
            }
        }
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
        if (this.soundManager) {
            this.soundManager.onDestroy();
            this.soundManager = null;
        }
        if (this.gltfParser) {
            this.gltfParser.onDestroy();
            this.gltfParser = null;
        }


        // 変数・オブジェクトの参照の破棄
        this.gl = null;
        this.canvas = null;
        this.clearColor = null;
    }


}



