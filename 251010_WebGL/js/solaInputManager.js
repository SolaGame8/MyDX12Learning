

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

        // キー: Gamepad Index, 値: boolean[] (buttons.length)
        this.previousButtonStates = {}; 
        this.currentButtonStates = {}; 

        // キーイベントハンドラを登録
        this._keyHandler = this._onKey.bind(this);
        window.addEventListener('keydown', this._keyHandler);


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

    /**
     * キーボードイベントの処理。バックスラッシュ/円記号が押されたらフルスクリーンを切り替える。
     * @param {KeyboardEvent} e 
     */
    _onKey(e) {
        // バックスラッシュまたは円記号でフルスクリーンを切り替え
        if (e.key === '\\' || e.key === '¥') { 
            this.toggleFullscreen();
            e.preventDefault(); 
        }
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




    onDestroy() {

        //イベント解除
        if (this._keyHandler) {
            window.removeEventListener('keydown', this._keyHandler);
            this._keyHandler = null;
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

    }
}


//BUTTONS 定数を他の js などから利用できるようにするため、このコードの一番下に window オブジェクトに BUTTONS を追加するコードを記述することが一般的です。
window.BUTTONS = BUTTONS;
window.AXES = AXES;




