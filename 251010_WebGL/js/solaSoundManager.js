

/**
 * BGMチャンネルを管理するための内部構造
 */
class MusicChannel {

    constructor(audioContext) {
        this.sourceNode = null;
        this.gainNode = audioContext.createGain();
        this.gainNode.connect(audioContext.destination);
    }
    
    // 現在のチャンネルに新しい曲を設定して再生準備
    setup(buffer, loop, volume) {
        // 古いノードを停止
        this.stop(); 
        
        this.sourceNode = this.gainNode.context.createBufferSource();
        this.sourceNode.buffer = buffer;
        this.sourceNode.loop = loop;
        
        // Source -> GainNode の接続を確立
        this.sourceNode.connect(this.gainNode);
        
        // 音量を即座に設定
        this.gainNode.gain.setValueAtTime(volume, this.gainNode.context.currentTime);
        
        // 再生はメソッド外で行う（start(0)）
    }

    // ノードを停止し、SourceNodeを破棄
    stop() {
        if (this.sourceNode) {
            try {
                this.sourceNode.stop(0); 
            } catch (e) {}
            // 接続を解除し、ノードを破棄（メモリ解放）
            this.sourceNode.disconnect();
            this.sourceNode = null;
        }
    }
}






class SolaSoundManager {

    /** @type {AudioContext} */
    audioContext;
    
    /** @type {Map<string, AudioBuffer>} キーとAudioBufferを紐付けるマップ */
    soundMap = new Map();

    /** @type {number} 同時再生可能なSEの最大数 */
    MAX_SOUNDS = 8; 

    /** * @type {Array<{sourceNode: AudioBufferSourceNode, startTime: number}>} 
     * 現在再生中のSEノードと開始時刻のリスト
     */
    activeSounds = []; 


    // BGMノードの管理を2つに拡張
    /** @type {MusicChannel | null} 現在再生中（またはフェードアウト中）のノード */
    currentMusicChannel = null; 
    /** @type {MusicChannel | null} 次に使用するための待機ノード */
    nextMusicChannel = null;



    /**
     * コンストラクタ
     */
    constructor() {

        // AudioContextの初期化
        this.audioContext = new (window.AudioContext || window.webkitAudioContext)();
        
        // BGMチャンネルを初期化
        this.currentMusicChannel = new MusicChannel(this.audioContext);
        this.nextMusicChannel = new MusicChannel(this.audioContext);

        // ブラウザの自動再生ポリシーに対応するため、ユーザー操作を待って再開できるように準備
        // 多くのブラウザは、最初のユーザー操作までAudioContextを中断（suspended）します。

        //スタートボタンなどを押させることで
        // ブラウザーセキュリティから音を鳴らせる権利を得る
        document.addEventListener('click', () => {

            if (this.audioContext.state === 'suspended') {

                this.audioContext.resume().then(() => {
                    console.log('AudioContextを再開しました。');
                });

            }

        }, { once: true });

    }


/**
     * BGMを再生します。既存のBGMがあれば即座に停止して新しい曲を再生します。
     * @param {string} key - 再生したいBGMのキー
     * @param {number} vol - 音量レベル (0.0 から 1.0)
     * @param {boolean} loop - ループ再生を行うかどうか
     * @returns {void}
     */

    playMusic(key, vol = 1.0, loop = true) {

        const buffer = this.soundMap.get(key);

        if (!buffer) {
            console.warn(`エラー: BGMキー「${key}」のバッファが見つかりません。`);
            return;
        }

        // 現在再生中のノードを停止 (フェードアウトなしで即時停止)
        this.stopMusic();

        // MusicChannelに設定し、音量を即座に設定
        this.currentMusicChannel.setup(buffer, loop, vol);
        
        // 再生開始
        this.currentMusicChannel.sourceNode.start(0);

        console.log(`BGM「${key}」の再生を開始しました。`);
    }

    /**
     * 現在再生中のBGMを、指定時間でフェードアウトさせ、停止します。
     * @param {number} fadeoutTime - フェードアウトにかける時間（秒）
     * @returns {void}
     */

    fadeoutMusic(fadeoutTime) {

        const channel = this.currentMusicChannel;

        if (!channel || !channel.sourceNode) {
            console.warn('現在再生中のBGMはありません。');
            return;
        }
        
        const now = this.audioContext.currentTime;

        // 修正ポイント: フェードアウト開始時のゲイン値を明示的に設定
        //    これにより、ランプの開始点が確定します。（ここでは最後の設定値から開始と仮定）
        //    ゲイン値に影響を与えうる他のオートメーションをキャンセルするためにも有効です。
        const currentGain = channel.gainNode.gain.value;
        channel.gainNode.gain.cancelScheduledValues(now); // 既に予約されたオートメーションをキャンセル
        channel.gainNode.gain.setValueAtTime(currentGain, now); // 現在のゲイン値で設定を確定
       


        // フェードアウトのランプを設定
        channel.gainNode.gain.linearRampToValueAtTime(0.0, now + fadeoutTime);

        // フェードアウト完了後にノードを停止（メモリ解放）
        channel.sourceNode.stop(now + fadeoutTime);

        // 停止後に currentMusicChannel をリセット
        channel.sourceNode.onended = () => {
            channel.sourceNode = null;
            console.log('BGMをフェードアウト停止しました。');
        };


    }

    /**
     * 現在のBGMをフェードアウトさせながら、新しいBGMを再生開始します。
     * (古いBGMのみがフェードアウトし、新しいBGMはパラメータ音量で即座に鳴る)
     * @param {string} key - 新しく再生するBGMのキー
     * @param {number} vol - 新しいBGMの音量レベル (0.0 から 1.0)
     * @param {boolean} loop - ループ再生を行うかどうか
     * @param {number} fadeoutTime - 古いBGMのフェードアウトにかける時間（秒）
     * @returns {void}
     */

    crossFadeMusic(key, vol = 1.0, loop = true, fadeoutTime = 2.0) {

        const buffer = this.soundMap.get(key);

        if (!buffer) {
            console.warn(`エラー: BGMキー「${key}」のバッファが見つかりません。`);
            return;
        }
        
        // チャンネルの役割を入れ替え
        // nextMusicChannel が oldChannel（フェードアウトさせるノード）になる
        // currentMusicChannel が newChannel（新しい曲を再生するノード）になる
        [this.currentMusicChannel, this.nextMusicChannel] = 
            [this.nextMusicChannel, this.currentMusicChannel];
        
        const oldChannel = this.nextMusicChannel;    // フェードアウトさせるノード
        const newChannel = this.currentMusicChannel; // 新しい曲を設定するノード

        // 新しいチャンネルの準備（再生は onended 内で行うため、ここでは start は呼ばない）
        newChannel.setup(buffer, loop, vol);
        
        // 古いチャンネルのフェードアウト処理
        if (oldChannel.sourceNode) {

            const now = this.audioContext.currentTime;
            
            // 修正ポイント: フェードアウト開始時のゲイン値を明示的に設定
            const currentGain = oldChannel.gainNode.gain.value;
            oldChannel.gainNode.gain.cancelScheduledValues(now); 
            oldChannel.gainNode.gain.setValueAtTime(currentGain, now);



            // 現在の音量から 0.0 までフェードアウト
            oldChannel.gainNode.gain.linearRampToValueAtTime(0.0, now + fadeoutTime);
            
            // フェードアウト完了後にノードの停止を予約
            oldChannel.sourceNode.stop(now + fadeoutTime);
            
            // 停止後のクリーンアップと、新しい曲の再生を予約
            oldChannel.sourceNode.onended = () => {
                // 古いチャンネルのクリーンアップ
                oldChannel.sourceNode = null;
                console.log(`BGMをフェードアウト停止しました。`);

                // 新しいチャンネルの再生を開始
                newChannel.sourceNode.start(0); 
                console.log(`BGM「${key}」の再生を開始しました (前曲停止後)。`);
            };

        } else {

            // 古い曲が再生されていなかった場合 (即時再生)
            newChannel.sourceNode.start(0);
            console.log(`BGM「${key}」を即時再生しました (前曲なし)。`);
        }
        
        console.log(`BGM「${key}」へのシーケンシャル切り替えを開始しました。`);
    }

    /**
     * BGM全体を停止します。アクティブなBGMは指定時間でフェードアウトします。
     * @param {number} fadeoutTime - フェードアウトにかける時間（秒）。デフォルトは1.0秒
     */

    stopMusic(fadeoutTime = 1.0) { // ⭐ 引数とデフォルト値を追加

        const activeChannel = this.currentMusicChannel;
        const inactiveChannel = this.nextMusicChannel;

        const now = this.audioContext.currentTime;

        // 1. アクティブなBGMチャンネルの処理
        if (activeChannel && activeChannel.sourceNode) {
            
            // 修正ポイント: ランプ開始を確実にするための処理
            const currentGain = activeChannel.gainNode.gain.value;
            // 既に予約されたオートメーションを全てキャンセル
            activeChannel.gainNode.gain.cancelScheduledValues(now); 
            // 現在のゲイン値で設定を確定し、ランプの開始点を定める
            activeChannel.gainNode.gain.setValueAtTime(currentGain, now); 


            // フェードアウトのランプを設定: 現在の音量から 0.0 まで
            // 現在のゲイン値を取得し、そこからフェードを開始することが正確ですが、
            // シンプル化のため、現在の音量設定から開始すると仮定します。
            
            // ゲインを 0.0 に直線的に変化させる
            activeChannel.gainNode.gain.linearRampToValueAtTime(0.0, now + fadeoutTime);
            
            // フェードアウト完了後にノードの停止を予約
            activeChannel.sourceNode.stop(now + fadeoutTime);

            // 停止後のクリーンアップ処理
            activeChannel.sourceNode.onended = () => {
                activeChannel.sourceNode = null;
                console.log('BGMをフェードアウト停止しました。');
            };
        }
        
        // 2. 非アクティブなBGMチャンネルの処理 (即時停止)
        // クロスフェード中にstopMusicが呼ばれた場合などに、待機ノードを確実に停止させる
        if (inactiveChannel && inactiveChannel.sourceNode) {
            inactiveChannel.stop(); // MusicChannel内部のstop()が即時停止を担当
        }

        console.log(`BGMを${fadeoutTime}秒かけて停止します。`);
    }





    /**
     * 単一の音声ファイル（MP3）を読み込み、デコードしてsoundMapに保持します。
     * @param {string} key - サウンドを識別するためのキー
     * @param {string} url - 音声ファイル（MP3）のパス
     * @returns {Promise<void>} 読み込みが完了したら解決するPromise
     */

    async loadSound(key, url) {

        console.log(`--- [${key}] の事前読み込みを開始 ---`);
        
        try {

            // MP3ファイルを非同期で読み込む
            const response = await fetch(url);

            if (!response.ok) {
                throw new Error(`HTTPエラー! ステータス: ${response.status}`);
            }

            const arrayBuffer = await response.arrayBuffer();

            // AudioContextでデコードし、AudioBufferを生成
            const audioBuffer = await this.audioContext.decodeAudioData(arrayBuffer);
            
            // soundMapにキーとバッファを格納
            this.soundMap.set(key, audioBuffer);
            console.log(`[${key}] (${url}) のバッファリングが完了しました。`);

        } catch (error) {
            console.error(`[${key}] (${url}) の読み込みまたはデコードに失敗しました:`, error);
        }
    }
    
    // 後の再生機能のために、AudioBufferを取得するゲッターを追加しておくと便利です
    getBuffer(key) {
        return this.soundMap.get(key);
    }


    /**
     * バッファリング済みのサウンドを再生します。
     * @param {string} key - 再生したいサウンドのキー
     * @param {boolean} loop - ループ再生を行うかどうか
     * @returns {AudioBufferSourceNode | null} 再生に使用したノード、または失敗した場合はnull
     */

    playSound(key, volume = 1.0, loop = false) {

        // 必須引数の型チェック（防御的プログラミング）
        if (typeof key !== 'string' || key === '') {
            console.error("エラー: playSoundには有効なキー（文字列）が必要です。");
            return null;
        }


        const buffer = this.soundMap.get(key);

        if (!buffer) {
            console.warn(`エラー: キー「${key}」のサウンドバッファが見つかりません。`);
            return null;
        }

        // AudioBufferSourceNodeは一回使い切りなので、再生のたびに新しく作成
        const sourceNode = this.audioContext.createBufferSource();
        
        sourceNode.buffer = buffer;// バッファ（デコード済みデータ）を設定
        sourceNode.loop = loop;// ループ設定
        //sourceNode.connect(this.audioContext.destination);// 出力先（destination）に直接接続
        
        // 音量調整のための GainNode を作成
        const gainNode = this.audioContext.createGain();
        // 再生前に音量を設定 (volume は 0.0〜1.0 の値)
        gainNode.gain.setValueAtTime(volume, this.audioContext.currentTime);

        // 接続: Source -> GainNode -> Destination
        sourceNode.connect(gainNode);
        gainNode.connect(this.audioContext.destination);

        // プーリング管理
        if (this.activeSounds.length >= this.MAX_SOUNDS) {

            // プールが満杯の場合: 最も古いノードを差し替える
            
            // activeSoundsをstartTimeでソートし、最も古いものを取得
            this.activeSounds.sort((a, b) => a.startTime - b.startTime);
            const oldestSounds = this.activeSounds.shift(); // 最も古いものをリストから削除
            
            // 古いノードを停止
            if (oldestSounds.sourceNode) {
                 // 停止時のエラーを避けるためにtry-catchを使用することが一般的です
                try {
                    oldestSounds.sourceNode.stop(); 
                    console.log(`プーリング制限: 最も古いSEを停止し、新しい音「${key}」を再生します。`);
                } catch (e) {
                    // 既に停止している可能性
                }
            }
        }

        // 再生を開始し、アクティブリストに追加
        const startTime = this.audioContext.currentTime;
        sourceNode.start(0);

        this.activeSounds.push({ 
            sourceNode: sourceNode, 
            startTime: startTime 
        });

        // 再生終了時のクリーンアップ処理（ワンショットSEの場合）
        if (!loop) {
            sourceNode.onended = () => {
                // 再生が終わったら、アクティブリストから削除
                this.activeSounds = this.activeSounds.filter(v => v.sourceNode !== sourceNode);
            };
        }

        return sourceNode;
    }

    /**
     * 補足: ノードを返すことで、外部で停止処理を呼ぶことができます。
     * 例: const node = soundManager.playSound('bgm', true); 
     * setTimeout(() => node.stop(), 10000); // 10秒後に停止
     */

    onDestroy() {


    }



}









