/**
 * solaPerlinNoise.js
 * C++ PerlinNoiseクラスからWebGl（JavaScript）向けに変換
 * シングルトンパターンを排除し、シード値の再現性を確保
 * Improved Perlin Noiseアルゴリズムを使用
 */
class solaPerlinNoise {


    // 順列配列
    p = [];


    createPrng(seed) {
        let currentSeed = seed >>> 0; // 32bit unsigned intに変換
        
        // Linear Congruential Generator (線形合同法) を使用
        // パラメータは Java の Random クラスに似たものを使用
        const multiplier = 0x5DEECE66D;
        const addend = 0xB;
        const mask = (1 << 48) - 1; // 48ビットマスク

        return () => {
            // シードを更新 (ここでは32ビット版の単純なXORShiftを使用します)
            currentSeed = Math.imul(1664525, currentSeed) + 1013904223 | 0; // 32bit LCG
            
            // 0から1の範囲の浮動小数点数を生成
            // 符号なし32ビット整数を最大値で割る
            return (currentSeed >>> 0) / 0xFFFFFFFF;
        };
    }

    /**
     * コンストラクタ
     * @param {number} seed 初期化に使用するシード値 (省略可能, デフォルトはランダム)
     */
    constructor(seed = null) {
        if (seed === null) {
            // シードが指定されていない場合、タイムスタンプなどからランダムなシードを生成
            seed = Date.now();
        }
        
        // initialize処理を実行
        this.initialize(seed);
    }

    // 初期化処理
    initialize(seed) {
        // 0から255までの順列配列を作成
        this.p = new Array(256);
        for (let i = 0; i < 256; i++) {
            this.p[i] = i;
        }

        // シード値付きPRNGを初期化
        const rng = this.createPrng(seed);

        // Fisher-Yates (Knuth) シャッフルアルゴリズムで順列をシャッフル
        // C++のstd::shuffleとstd::mt19937の処理を再現
        for (let i = 255; i > 0; i--) {
            // i+1 の範囲で乱数インデックスを生成
            const j = Math.floor(rng() * (i + 1)); 
            
            // 要素を交換
            [this.p[i], this.p[j]] = [this.p[j], this.p[i]];
        }

        // 配列を2倍に拡張して境界条件をなくす (p[0]からp[511]まで)
        this.p = this.p.concat(this.p);
    }

    // 減衰関数 (6t^5 - 15t^4 + 10t^3)
    fade(t) {
        return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
    }

    // 線形補間関数
    lerp(t, a, b) {
        return a + t * (b - a);
    }

    // 勾配関数 (Improved Perlin Noiseの勾配ベクトルとの内積)
    grad(hash, x, y, z) {
        const h = hash & 15; // 0から15のハッシュ値
        
        // C++のコードのロジックを忠実に再現
        const u = h < 8 ? x : y;
        const v = h < 4 ? y : (h === 12 || h === 14 ? x : z);
        
        // 勾配ベクトルの成分を選択し、符号を決定
        return ((h & 1) === 0 ? u : -u) + ((h & 2) === 0 ? v : -v);
    }

    /**
     * 3Dパーリンノイズを生成するメソッド
     * @param {number} x X座標
     * @param {number} y Y座標
     * @param {number} z Z座標
     * @returns {number} ノイズ値 (約 -1.0 から 1.0 の間)
     */


    noise(x, y, z) {
        
        // 座標を整数部と小数部に分割
        // JavaScriptではビット演算の前に Math.floor() が必要
        let X = Math.floor(x) & 255;
        let Y = Math.floor(y) & 255;
        let Z = Math.floor(z) & 255;

        x -= Math.floor(x);
        y -= Math.floor(y);
        z -= Math.floor(z);

        // 減衰関数を適用
        const u = this.fade(x);
        const v = this.fade(y);
        const w = this.fade(z);

        // 8つの格子点のハッシュ値を計算 (パーミュテーションテーブルを使用)
        // C++のコードのロジックを再現
        let A = this.p[X] + Y;
        let AA = this.p[A] + Z;
        let AB = this.p[A + 1] + Z;
        let B = this.p[X + 1] + Y;
        let BA = this.p[B] + Z;
        let BB = this.p[B + 1] + Z;

        // 線形補間
        // 勾配ベクトルとの内積を計算し、それらを線形補間
        return this.lerp(w,
            this.lerp(v,
                this.lerp(u, this.grad(this.p[AA], x, y, z),
                    this.grad(this.p[BA], x - 1, y, z)),
                this.lerp(u, this.grad(this.p[AB], x, y - 1, z),
                    this.grad(this.p[BB], x - 1, y - 1, z))),
            this.lerp(v,
                this.lerp(u, this.grad(this.p[AA + 1], x, y, z - 1),
                    this.grad(this.p[BA + 1], x - 1, y, z - 1)),
                this.lerp(u, this.grad(this.p[AB + 1], x, y - 1, z - 1),
                    this.grad(this.p[BB + 1], x - 1, y - 1, z - 1))));
    }
}



