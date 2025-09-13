#include "PerlinNoise.h"

// 静的メンバ変数の初期化
PerlinNoise* PerlinNoise::instance = nullptr;

// デフォルトコンストラクタ (ランダムシード)
PerlinNoise::PerlinNoise() {
    std::random_device rd;
    initialize(rd());
}

// シード値付きコンストラクタ
PerlinNoise::PerlinNoise(unsigned int seed) {
    initialize(seed);
}

// 初期化処理
void PerlinNoise::initialize(unsigned int seed) {
    // 0から255までの順列配列を作成
    p.resize(256);
    std::iota(p.begin(), p.end(), 0);

    // 指定されたシードで乱数ジェネレータを初期化
    std::mt19937 g(seed);
    // 順列をシャッフル
    std::shuffle(p.begin(), p.end(), g);

    // 配列を2倍に拡張して境界条件をなくす
    p.insert(p.end(), p.begin(), p.end());
}

PerlinNoise::~PerlinNoise() {}

// デフォルトのgetInstance
PerlinNoise* PerlinNoise::getInstance() {
    if (instance == nullptr) {
        instance = new PerlinNoise();
    }
    return instance;
}

// シード値付きのgetInstance
PerlinNoise* PerlinNoise::getInstance(unsigned int seed) {
    if (instance == nullptr) {
        instance = new PerlinNoise(seed);
    }
    // 注意: 既にインスタンスが存在する場合、シード値は変更されません。
    // シングルトンの性質上、インスタンスは一つしか存在しないためです。
    return instance;
}

void PerlinNoise::destroyInstance() {
    if (instance != nullptr) {
        delete instance;
        instance = nullptr;
    }
}

// 減衰関数、補間関数、勾配関数、noise関数は変更なし
double PerlinNoise::fade(double t) {
    // 6t^5 - 15t^4 + 10t^3
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}
double PerlinNoise::lerp(double t, double a, double b) { 
    // aとbの間をtの割合で補間
    return a + t * (b - a);

}
double PerlinNoise::grad(int hash, double x, double y, double z) { 
    int h = hash & 15;
    double u = h < 8 ? x : y;
    double v = h < 4 ? y : (h == 12 || h == 14 ? x : z);

    // 勾配ベクトルの成分を選択し、符号を決定
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);

}

double PerlinNoise::noise(double x, double y, double z) { 


    // 座標を整数部と小数部に分割
    int X = (int)floor(x) & 255;
    int Y = (int)floor(y) & 255;
    int Z = (int)floor(z) & 255;

    x -= floor(x);
    y -= floor(y);
    z -= floor(z);

    // 減衰関数を適用
    // これは、ノイズを滑らかにするための重要なステップ
    double u = fade(x);
    double v = fade(y);
    double w = fade(z);

    // 8つの格子点のハッシュ値を計算
    // 3次元空間の立方体にある8つの頂点に対応
    int A = p[X] + Y;
    int AA = p[A] + Z;
    int AB = p[A + 1] + Z;
    int B = p[X + 1] + Y;
    int BA = p[B] + Z;
    int BB = p[B + 1] + Z;




    // 線形補間
    // 8つの格子点それぞれで勾配ベクトルとの内積を計算し、
    // それらを線形補間して最終的なノイズ値を生成
    return lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z),
        grad(p[BA], x - 1, y, z)),
        lerp(u, grad(p[AB], x, y - 1, z),
            grad(p[BB], x - 1, y - 1, z))),
        lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1),
            grad(p[BA + 1], x - 1, y, z - 1)),
            lerp(u, grad(p[AB + 1], x, y - 1, z - 1),
                grad(p[BB + 1], x - 1, y - 1, z - 1))));


}

