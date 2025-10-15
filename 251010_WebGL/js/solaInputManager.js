

const BUTTONS = {
    // Face Buttons
    A: 0, // XBox A, PS X
    B: 1, // XBox B, PS 〇
    X: 2, // XBox X, PS □
    Y: 3, // XBox Y, PS △

    // Shoulder Buttons
    LB: 4, // Left Bumper
    RB: 5, // Right Bumper
    LT: 6, // Left Trigger (軸として扱われることもあるが、ここではボタンとして定義)
    RT: 7, // Right Trigger

    // Utility Buttons
    START: 9,
    SELECT: 8,
    LSTICK: 10, // 左スティック押し込み
    RSTICK: 11, // 右スティック押し込み
    HOME: 16,

    // D-Pad
    DPAD_UP: 12,
    DPAD_DOWN: 13,
    DPAD_LEFT: 14,
    DPAD_RIGHT: 15,
};


const AXES = {
    L_AXIS_X: 0,
    L_AXIS_Y: 1,
    R_AXIS_X: 2,
    R_AXIS_Y: 3,
};


class SolaInputManager {

    /**
     * @param {SolaWGL} wglHelper - SolaWGLのインスタンス
     */
    constructor(wglHelper) {

        this.wgl = wglHelper;
        this.canvas = wglHelper.canvas;

        this.gamepads = {}; // 接続されているゲームパッドの状態を保持 { index: Gamepadオブジェクト }

        // マウスの状態管理
        this.isPointerLocked = false; 

        this.currentMousePosition = { x: 0, y: 0 };
        this.mouseDelta = { x: 0, y: 0 }; // フレーム内で累積される移動量

        // キー: Gamepad Index, 値: boolean[] (buttons.length)
        this.previousButtonStates = {}; 
        this.currentButtonStates = {}; 

        // 【キーボードの状態管理 (押された瞬間検知用)
        // Key: キー名 (例: ' ', 'Enter') , Value: boolean (true = 押された瞬間があった)
        this.keyPushStates = new Map(); 

        // Key: キー名 (例: 'w', 'a'), Value: boolean (true = 押されている)
        this.keyPressStates = new Map();


        // キーイベントハンドラを登録
        this._keyHandlerDown = this._onKeyDown.bind(this);
        window.addEventListener('keydown', this._keyHandlerDown);

        this._keyHandlerUp = this._onKeyUp.bind(this);
        window.addEventListener('keyup', this._keyHandlerUp);


        // マウスイベントハンドラを登録
        this._mouseHandlerMove = this._onMouseMove.bind(this);
        // windowでマウスを動かしたときだけ反応するようにします
        window.addEventListener('mousemove', this._mouseHandlerMove);


        if (false) { //画面クリックでカーソルを消したい場合、true

            // Pointer Lockイベントハンドラの登録
            this._pointerLockChangeHandler = this._onPointerLockChange.bind(this);
            document.addEventListener('pointerlockchange', this._pointerLockChangeHandler);
            document.addEventListener('mozpointerlockchange', this._pointerLockChangeHandler);
            document.addEventListener('webkitpointerlockchange', this._pointerLockChangeHandler);

            // キャンバスへのクリックでロックを開始
            this._canvasClickHandler = this.requestPointerLock.bind(this);
            this.canvas.addEventListener('click', this._canvasClickHandler); // クリックでロック開始
        }


        // ゲームパッド接続・切断イベントのリスナーを登録
        this._gamepadConnectedHandler = this._onGamepadConnected.bind(this);
        this._gamepadDisconnectedHandler = this._onGamepadDisconnected.bind(this);
        window.addEventListener("gamepadconnected", this._gamepadConnectedHandler);
        window.addEventListener("gamepaddisconnected", this._gamepadDisconnectedHandler);

        //コントローラー情報初期化
        const latestGamepads = this._getLatestGamepads();
        for (const gamepad of latestGamepads) {
            this._initializeGamepadTracking(gamepad);
        }

        console.log("InputManager: キーボードイベントの監視を開始しました。");
    }

    /**
     * canvas要素に対してPointer Lockを要求する。
     */

    requestPointerLock() {

        this.canvas.requestPointerLock = this.canvas.requestPointerLock || 
                                        this.canvas.mozRequestPointerLock || 
                                        this.canvas.webkitRequestPointerLock;
        
        
        if (this.canvas.requestPointerLock) {
            this.canvas.requestPointerLock();
            console.log("Pointer Lock: Requesting lock...");
        } else {
            console.warn("Pointer Lock: Browser does not support requestPointerLock.");
        }
        

    }

    /**
     * Pointer Lockの状態が変更されたときに呼び出される。
     */
    _onPointerLockChange() {
        const isLocked = document.pointerLockElement === this.canvas ||
                        document.mozPointerLockElement === this.canvas ||
                        document.webkitPointerLockElement === this.canvas;

        
        if (isLocked) {
            this.isPointerLocked = true;
            console.log("Pointer Lock: Activated. Mouse movement is now relative.");
        } else {
            this.isPointerLocked = false;
            console.log("Pointer Lock: Deactivated. Cursor visible.");
        }
            

    }


    /**
     * マウス移動イベントの処理。マウスの相対移動量を累積する。
     * @param {MouseEvent} e 
     */
    _onMouseMove(e) {

        // e.movementX, e.movementY を利用
        const deltaX = e.movementX || 0; 
        const deltaY = e.movementY || 0;

        // 今回の移動量
        this.mouseDelta.x = deltaX;
        this.mouseDelta.y = deltaY;

        // 現在のマウス座標も更新 (カーソルが表示されている場合に便利)
        this.currentMousePosition.x = e.clientX;
        this.currentMousePosition.y = e.clientY;



    }

    /**
     * 直前のフレームからのマウスの移動量（デルタ）を取得する。
     * 呼び出し後、次のフレームの update() でデルタはリセットされる。
     * @returns {{x: number, y: number}} マウスの移動量
     */
    getMouseDelta() {
        // 値渡し（オブジェクトのコピー）をする


        let mDelta = { 
            x: this.mouseDelta.x, 
            y: this.mouseDelta.y 
        };

        this.mouseDelta.x = 0.0;
        this.mouseDelta.y = 0.0;

        return mDelta;
    }


    // 監視対象キーの登録
    /**
     * 監視対象とするキーを登録し、初期状態をリセットする。複数回呼び出し可。
     * @param {string[]} keyNames - 監視したいキーの名前の配列 (例: [' ', 'Enter', 'W'])
     */
    addKeyToTrack(keyNames) {
        for (const key of keyNames) {
            // Map.set() は、キーが存在すれば上書き、存在しなければ追加を行います。
            this.keyPushStates.set(key, false);
            this.keyPressStates.set(key, false);
        }
    }

    // 監視対象キーの削除
    /**
     * 監視対象からキーを削除する。
     * @param {string[]} keyNames - 監視を解除したいキーの名前の配列
     */
    removeKeyToTrack(keyNames) {
        for (const key of keyNames) {
            // Map.delete() は、キーが存在すれば削除し、存在しなければ何もしません。
            this.keyPushStates.delete(key);
            this.keyPressStates.delete(key);
        }
    }






    _initializeGamepadTracking(gamepad) {

        const index = gamepad.index;
        
        if (this.currentButtonStates[index]) return; 

        // 初期状態を false で初期化
        const initialStates = new Array(gamepad.buttons.length).fill(false);
        this.currentButtonStates[index] = [...initialStates];
        this.previousButtonStates[index] = [...initialStates]; 
        this.gamepads[index] = gamepad;
        
        console.log(`Gamepad detected and initialized (Tracking) at index ${index}: ${gamepad.id}.`);
    }


    // ゲームパッド接続時の処理
    _onGamepadConnected(e) {
        console.log(`Gamepad connected at index ${e.gamepad.index}: ${e.gamepad.id}.`);
        this.gamepads[e.gamepad.index] = e.gamepad;

        // 接続時: 前回の状態配列を初期化 (ボタンの数だけ false を格納)

        const initialStates = new Array(e.gamepad.buttons.length).fill(false);
        this.currentButtonStates[e.gamepad.index] = [...initialStates]; //...は、値渡し。ディープコピー（参照ではない）
        this.previousButtonStates[e.gamepad.index] = [...initialStates];


        console.log("InputManager: ゲームパッドと接続しました");
    
    }

    // ゲームパッド切断時の処理
    _onGamepadDisconnected(e) {
        console.log(`Gamepad disconnected from index ${e.gamepad.index}: ${e.gamepad.id}.`);
        delete this.gamepads[e.gamepad.index];

        // 切断時: 前回の状態配列を削除
        delete this.currentButtonStates[e.gamepad.index];
        delete this.previousButtonStates[e.gamepad.index];
    }


    _getLatestGamepads() {
        // navigator.getGamepads() を呼び出すことで、ボタンや軸の最新の状態に更新される
        const gamepads = navigator.getGamepads ? navigator.getGamepads() : [];
        
        // nullでない有効なゲームパッドのみをフィルタリングして返す
        return Array.from(gamepads).filter(gp => gp !== null);
    }

    /**
     * 描画ループ内で呼び出し、ボタンの最新状態を更新し、前回状態を保存する
     */
    update() {
        const latestGamepads = this._getLatestGamepads();

        for (const gamepad of latestGamepads) {
            const index = gamepad.index;
            const currentButtons = gamepad.buttons;

            this.previousButtonStates[index] = [...this.currentButtonStates[index]];    //値渡し

            // 現在の状態 (current) を、ブラウザから取得した最新 (latest) の状態に更新する
            this.currentButtonStates[index] = currentButtons.map(btn => btn.pressed);

        }
    }


    getStickValue(stick, deadZone = 0.1) {

        const latestGamepads = this._getLatestGamepads();
        if (latestGamepads.length === 0) {
            return { x: 0.0, y: 0.0 };
        }

        const firstGamepad = latestGamepads[0];
        const axes = firstGamepad.axes;

        let axisX, axisY;

        // スティックに応じて軸インデックスを設定
        if (stick === 'L') {
            axisX = AXES.L_AXIS_X;
            axisY = AXES.L_AXIS_Y;
        } else if (stick === 'R') {
            axisX = AXES.R_AXIS_X;
            axisY = AXES.R_AXIS_Y;
        } else {
            console.warn(`Invalid stick value: ${stick}. Must be 'L' or 'R'.`);
            return { x: 0.0, y: 0.0 };
        }

        let x = axes[axisX] || 0.0; //axes[axisX] にスティックの傾きを示す有効な数値（−1.0 から 1.0）が入っているかを確認し、入っていない場合は 0.0
        let y = axes[axisY] || 0.0;

        // デッドゾーン処理 (中心の微細なブレを無視する)
        const length = Math.sqrt(x * x + y * y);

        if (length <= deadZone) {
            return { x: 0.0, y: 0.0 };
        }
        
        
        return { x: x, y: y };
    }

    getGamepadOnPush(buttonIndex) {


        const firstGamepadIndex = this._getLatestGamepads()[0]?.index;
        if (typeof firstGamepadIndex === 'undefined') return false;

        const currentStates = this.currentButtonStates[firstGamepadIndex];
        const previousStates = this.previousButtonStates[firstGamepadIndex];

        if (!currentStates || !previousStates || buttonIndex < 0 || buttonIndex >= currentStates.length) {
            return false;
        }

        const isPressedNow = currentStates[buttonIndex];
        const wasPressedBefore = previousStates[buttonIndex];

        // 押した瞬間 (今回押されている AND 前回は押されていなかった)
        return isPressedNow && !wasPressedBefore;


    }



    getGamepadOnPress(buttonIndex) {


        //console.log("InputManager: getGamepadOnPress ", buttonIndex);

        const firstGamepadIndex = this._getLatestGamepads()[0]?.index;
        if (typeof firstGamepadIndex === 'undefined') return false;

        const currentStates = this.currentButtonStates[firstGamepadIndex];

        if (!currentStates || buttonIndex < 0 || buttonIndex >= currentStates.length) {
            return false;
        }

        // currentButtonStates は既に boolean の配列なので、そのまま返す
        return currentStates[buttonIndex];



    }



    // 【キーが押された瞬間を取得し、状態をリセットする関数
    /**
     * 指定されたキーが「押された瞬間」にのみ true を返し、呼び出し後に状態を false にリセットする。
     * (イベント駆動型の処理)
     * @param {string} keyName - 判定したいキーの名前
     * @returns {boolean} キーが押された瞬間であれば true
     */


    onPushKey(keyName) {

        // 監視対象として登録されているかチェック
        if (!this.keyPushStates.has(keyName)) {
            return false;
        }

        const isPushed = this.keyPushStates.get(keyName);

        // 状態が true であった場合、すぐに false にリセット（次の呼び出しでは false を返すようにする）
        if (isPushed) {
            this.keyPushStates.set(keyName, false);
            return true;
        }
        
        return false;
    }


    /**
     * 指定されたキーが「押されている間」ずっと true を返す。
     * @param {string} keyName - 判定したいキーの名前
     * @returns {boolean} キーが押されていれば true
     */
    onPressKey(keyName) {
        // 監視対象として登録されているかチェックし、状態を返す
        return this.keyPressStates.get(keyName) || false;
    }



    /**
     * キーボードイベントの処理。バックスラッシュ/円記号が押されたらフルスクリーンを切り替える。
     * @param {KeyboardEvent} e 
     */

    _onKeyDown(e) {

        //const key = e.key;

        // キーを小文字に統一 (大文字・小文字の区別をなくす)
        const key = e.key.toLowerCase(); 



        // 監視対象のキーの状態を更新
        // 監視対象のキーかどうかをチェックし、まだ true でない場合のみ true に設定
        // （true に設定することで、onPushKeyの次の呼び出しで検知されるようになる）

        if (this.keyPushStates.has(key) && this.keyPushStates.get(key) === false) {
            this.keyPushStates.set(key, true);
        }

        // 継続状態を true に設定
        if (this.keyPressStates.has(key)) {
            this.keyPressStates.set(key, true);
        }

    }


    /**
     * キーボードイベントの処理。キーが離されたときに呼び出される。
     * @param {KeyboardEvent} e 
     */

    _onKeyUp(e) {

        //const key = e.key;

        // キーを小文字に統一 (大文字・小文字の区別をなくす)
        const key = e.key.toLowerCase();

        // 【継続状態を false に設定
        if (this.keyPressStates.has(key)) {
            this.keyPressStates.set(key, false);
        }
    }






    onDestroy() {

        //イベント解除
        if (this._keyHandlerDown) {
            window.removeEventListener('keydown', this._keyHandlerDown);
            this._keyHandlerDown = null;
        }
        if (this._keyHandlerUp) {
            window.removeEventListener('keyup', this._keyHandlerUp);
            this._keyHandlerUp = null;
        }


        // マウスイベントの解除
        if (this._mouseHandlerMove) {
            window.removeEventListener('mousemove', this._mouseHandlerMove);
            this._mouseHandlerMove = null;
        }

        // Pointer Lock関連のイベント解除
        if (this._pointerLockChangeHandler) {
            document.removeEventListener('pointerlockchange', this._pointerLockChangeHandler);
            document.removeEventListener('mozpointerlockchange', this._pointerLockChangeHandler);
            document.removeEventListener('webkitpointerlockchange', this._pointerLockChangeHandler);
            this._pointerLockChangeHandler = null;
        }
        
        if (this._canvasClickHandler) {
            this.canvas.removeEventListener('click', this._canvasClickHandler);
            this._canvasClickHandler = null;
        }


        if (this._gamepadConnectedHandler) {
            window.removeEventListener("gamepadconnected", this._gamepadConnectedHandler);
            this._gamepadConnectedHandler = null;
        }
        if (this._gamepadDisconnectedHandler) {
            window.removeEventListener("gamepaddisconnected", this._gamepadDisconnectedHandler);
            this._gamepadDisconnectedHandler = null;
        }

        //変数
        this.gamepads = null;
        this.previousButtonStates = null;

        this.keyPushStates = null;
        this.keyPressStates = null;

    }
}


//BUTTONS 定数を他の js などから利用できるようにするため、このコードの一番下に window オブジェクトに BUTTONS を追加するコードを記述することが一般的。
window.BUTTONS = BUTTONS;
window.AXES = AXES;




