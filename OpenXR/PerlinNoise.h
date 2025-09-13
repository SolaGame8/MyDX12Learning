#pragma once

#include <vector>
#include <numeric>
#include <random>
#include <algorithm>

class PerlinNoise {
private:
    // シングルトンインスタンス
    static PerlinNoise* instance;

    // コンストラクタを非公開に
    PerlinNoise();

    // シード値を受け取るコンストラクタ
    PerlinNoise(unsigned int seed);

    ~PerlinNoise();

    // コピーと代入を禁止
    PerlinNoise(const PerlinNoise&) = delete;
    PerlinNoise& operator=(const PerlinNoise&) = delete;

    std::vector<int> p;

    void initialize(unsigned int seed); // 初期化処理を共通化

    double fade(double t);
    double lerp(double t, double a, double b);
    double grad(int hash, double x, double y, double z);

public:
    // シングルトンインスタンスを取得するメソッド
    static PerlinNoise* getInstance();

    // シード値付きでシングルトンインスタンスを取得するメソッド
    static PerlinNoise* getInstance(unsigned int seed);

    // シングルトンインスタンスを解放するメソッド
    static void destroyInstance();

    // 2Dパーリンノイズを生成するメソッド
    double noise(double x, double y, double z);
};


