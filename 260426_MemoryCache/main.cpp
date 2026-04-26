#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <string>
#include <algorithm>

#include <immintrin.h> // SIMD (SSE/AVX) 命令用

//----------------------------------------- 
// AABBの構造体
// 
// すべて計算する内容は同じ
// ＊変数の構造によって速度が変わるデモ
// ＊最後の６，７はSIMD化して、さらに高速化する
//----------------------------------------- 


// ■ 構造体配列 AoS (Array of Structures)
// 標準的
// 24バイト。AABB計算に必要な最小限のデータのみを持つ

struct AABB {
    float minX, minY, minZ;
    float maxX, maxY, maxZ;
};


// ■ 構造体配列 AoS (Array of Structures)
// キャッシュラインを意識
// 32バイト。キャッシュライン(64B)に2個綺麗に収まるようパディングを付与

struct alignas(32) AABB32 {

    float minX, minY, minZ;
    float maxX, maxY, maxZ;

    float padding[2]; // 24バイトを32バイトに膨らませる
};


// ■ 配列の構造体 SoA　(Structure of Arrays)
// 各成分を個別に分けてvectorで連続的に管理
// 必要な軸のデータだけをスキャンする試み

//vector が6つに分かれています。これはメモリ上の全く別の6つの場所を同時にスキャンすることを意味します。
//ハードウェアの限界 : CPUが同時にプリフェッチ（先行読み込み）できるストリーム数には限界があります。


struct AABBSplitSoA {

    std::vector<float> minX, minY, minZ;    //配列３つ
    std::vector<float> maxX, maxY, maxZ;    //配列３つ

    void resize(int n) {
        minX.resize(n); minY.resize(n); minZ.resize(n);
        maxX.resize(n); maxY.resize(n); maxZ.resize(n);
    }

    /*
    // i番目のデータを取得するアクセサ
    AABBView at(int i) const {
        return { minX[i], minY[i], minZ[i], maxX[i], maxY[i], maxZ[i] };
    }
    */

};


// ■ 配列の構造体 SoA　(Structure of Arrays)
// 1つの大きなメモリブロックを分割して使用
// SoAの利点とメモリの局所性を両立

struct AABBFlatSoA {

    std::vector<float> data;
    float* minX, * minY, * minZ, * maxX, * maxY, * maxZ;

    void init(int n) {

        // 全データ（6つのfloat × n個）を一括で確保
        data.resize(n * 6);

        // 各ポインタを適切なオフセットに配置
        minX = &data[0];            //minXが連続的に並ぶ
        minY = &data[n * 1];        //次に、   minYが連続的に並ぶ
        minZ = &data[n * 2];        //その次に、minZが連続的に並ぶ
        maxX = &data[n * 3];        //以下同様
        maxY = &data[n * 4];
        maxZ = &data[n * 5];
    }

    /*
    // i番目のデータを取得するアクセサ
    AABBView at(int i) const {
        return { minX[i], minY[i], minZ[i], maxX[i], maxY[i], maxZ[i] };
    }
    */

};





// --- オブジェクト指向の構造体 ---    
// 人間にとって、整理されていてわかりやすい
// わざとメモリレイアウトがばらばらになるように使用

struct OOPEntity {
    AABB box;
    char dummyData[1024]; // キャッシュライン(64B)を大幅に超えるデータ
    std::string name;     // 動的確保を伴うデータ
    int id;

    OOPEntity() : name("Object_Name_Extremely_Long_To_Ensure_Heap_Allocation") {
        id = rand();
    }
};





template <typename T>
bool Intersect(const T& a, const T& b) {
    return (a.minX <= b.maxX && a.maxX >= b.minX &&
        a.minY <= b.maxY && a.maxY >= b.minY &&
        a.minZ <= b.maxZ && a.maxZ >= b.minZ);
}



int main() {

    const int NUM_OBJECTS = 10000;

    std::cout << "--- メモリ効率比較テスト ---" << std::endl;
    std::cout << "AABB 総当たり当たり判定" << std::endl;
    std::cout << "オブジェクト数: " << NUM_OBJECTS << std::endl;
    std::cout << "準備中..." << std::endl;


    // 乱数
    std::default_random_engine gen;
    std::uniform_real_distribution<float> dist(0.0f, 100.0f);   //0.0～100.0の乱数を発生

    std::vector<void*> fragments; // メモリをわざと散らばらせるためのダミー



    //OOP   //Object-Oriented Programming   オブジェクト思考
    //DOP   //Data-Oriented Programming     データ思考

    //-----------

    // パターン1用：バラバラのメモリに配置されたオブジェクト指向 構造体オブジェクト
    std::vector<OOPEntity*> oopEntity;
    for (int i = 0; i < NUM_OBJECTS; ++i) {
        // わざと小さな空きメモリを作りながら割り当てる（断片化の促進）
        fragments.push_back(new char[rand() % 64]);

        OOPEntity* obj = new OOPEntity();
        obj->box = { dist(gen), dist(gen), dist(gen), dist(gen) + 1, dist(gen) + 1, dist(gen) + 1 };    //ランダムでAABBの値を入力
        oopEntity.push_back(obj);
    }

    // ポインタの配列をバラバラにシャッフルする
    // スポーンなどで、断続的に作られたようなオブジェクトデータを想定
    std::shuffle(oopEntity.begin(), oopEntity.end(), gen);




    //-----------


    // パターン2用：必要なデータだけを連続メモリにコピー
    // （構造体オブジェクトのAABB部分だけをメモリに並べる）
    
    //メモリ転送量：5万個 × 24バイト = 約1.2MB
    std::vector<AABB> leanBoxes;
    for (int i = 0; i < NUM_OBJECTS; ++i) {
        leanBoxes.push_back(oopEntity[i]->box);
    }

    //-----------

    // パターン3用：64バイトアライメントに綺麗に収める

    //メモリ転送量：5万個 × 32バイト = 約1.6MB   ＊高速になったとしても、転送量の速度低下でトレードオフ
    std::vector<AABB32> aligned32Boxes(NUM_OBJECTS);
    for (int i = 0; i < NUM_OBJECTS; ++i) {
        // パターン3: OptimizedAABBへデータを渡す
        aligned32Boxes[i].minX = oopEntity[i]->box.minX;
        aligned32Boxes[i].minY = oopEntity[i]->box.minY;
        aligned32Boxes[i].minZ = oopEntity[i]->box.minZ;
        aligned32Boxes[i].maxX = oopEntity[i]->box.maxX;
        aligned32Boxes[i].maxY = oopEntity[i]->box.maxY;
        aligned32Boxes[i].maxZ = oopEntity[i]->box.maxZ;
    }

    //-----------

    // パターン4用：Split SoA

    AABBSplitSoA splitSoaBoxes;
    splitSoaBoxes.resize(NUM_OBJECTS);
    for (int i = 0; i < NUM_OBJECTS; ++i) {
        splitSoaBoxes.minX[i] = oopEntity[i]->box.minX;
        splitSoaBoxes.minY[i] = oopEntity[i]->box.minY;
        splitSoaBoxes.minZ[i] = oopEntity[i]->box.minZ;
        splitSoaBoxes.maxX[i] = oopEntity[i]->box.maxX;
        splitSoaBoxes.maxY[i] = oopEntity[i]->box.maxY;
        splitSoaBoxes.maxZ[i] = oopEntity[i]->box.maxZ;
    }

    //-----------

    // パターン5用：Flat SoA
    AABBFlatSoA flatSoaBoxes;
    flatSoaBoxes.init(NUM_OBJECTS);
    for (int i = 0; i < NUM_OBJECTS; ++i) {
        flatSoaBoxes.minX[i] = oopEntity[i]->box.minX;
        flatSoaBoxes.minY[i] = oopEntity[i]->box.minY;
        flatSoaBoxes.minZ[i] = oopEntity[i]->box.minZ;
        flatSoaBoxes.maxX[i] = oopEntity[i]->box.maxX;
        flatSoaBoxes.maxY[i] = oopEntity[i]->box.maxY;
        flatSoaBoxes.maxZ[i] = oopEntity[i]->box.maxZ;
    }

    //-----------



    // 衝突判定対象（ランダムな1つのAABB）
    AABB target = { 50.0f, 50.0f, 50.0f, 51.0f, 51.0f, 51.0f };

    // --- 計測開始のトリガー ---
    std::cout << "\n準備が完了しました。" << std::endl;
    std::cout << "\nPush Enter Key...\n";
    
    std::cin.get();


    // --- 計測1: 非効率なパターン ---
    {
        auto start = std::chrono::high_resolution_clock::now();
        int hitCount = 0;
        for (int i = 0; i < NUM_OBJECTS; ++i) {
            for (int j = i + 1; j < NUM_OBJECTS; ++j) {
                if (Intersect(oopEntity[i]->box, oopEntity[j]->box)) hitCount++;
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;
        std::cout << "\t[ 1: 非効率 (AoS 断片的) ] \t" << elapsed.count() << " ms / Hits: " << hitCount << "\n";
    }

    std::cout << "\nPush Enter Key...\n";

    std::cin.get();


    // --- 計測2: 効率的なパターン ---

    {
        auto start = std::chrono::high_resolution_clock::now();
        int hitCount = 0;
        for (int i = 0; i < NUM_OBJECTS; ++i) {
            for (int j = i + 1; j < NUM_OBJECTS; ++j) {
                if (Intersect(leanBoxes[i], leanBoxes[j])) hitCount++;
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;
        std::cout << "\t[ 2: 効率的 (AoS 連続)] \t" << elapsed.count() << " ms / Hits: " << hitCount << "\n";
    }


    std::cout << "\nPush Enter Key...\n";

    std::cin.get();


    // --- 計測3: 32バイトアライメント ---
    //「アライメントを揃えるメリット」よりも、「余計なデータ（パディング）をメモリから持ってくるデメリット」の方が上回ることもある

    {
        auto start = std::chrono::high_resolution_clock::now();
        int hitCount = 0;
        for (int i = 0; i < NUM_OBJECTS; ++i) {
            for (int j = i + 1; j < NUM_OBJECTS; ++j) {
                if (Intersect(aligned32Boxes[i], aligned32Boxes[j])) hitCount++;
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;
        std::cout << "\t[ 3: 32バイトアライメント ] \t" << elapsed.count() << " ms / Hits: " << hitCount << "\n";
    }



    std::cout << "\nPush Enter Key...\n";

    std::cin.get();


    // --- SoA専用の判定処理 ---
    {


        auto start = std::chrono::high_resolution_clock::now();

        int hitCount = 0;


        {
            auto start = std::chrono::high_resolution_clock::now();

            
            for (int i = 0; i < NUM_OBJECTS; ++i) {
                // 外側のiのデータはループの頭で1回だけロード（公平な最適化）
                const float amx = splitSoaBoxes.minX[i]; const float aMx = splitSoaBoxes.maxX[i];
                const float amy = splitSoaBoxes.minY[i]; const float aMy = splitSoaBoxes.maxY[i];
                const float amz = splitSoaBoxes.minZ[i]; const float aMz = splitSoaBoxes.maxZ[i];

                for (int j = i + 1; j < NUM_OBJECTS; ++j) {
                    // && で繋ぐことで、前の条件が偽なら後のメモリは見に行かない
                    // 演算自体は AoS の Intersect と全く同じ順序・回数
                    if (amx <= splitSoaBoxes.maxX[j] && aMx >= splitSoaBoxes.minX[j] &&
                        amy <= splitSoaBoxes.maxY[j] && aMy >= splitSoaBoxes.minY[j] &&
                        amz <= splitSoaBoxes.maxZ[j] && aMz >= splitSoaBoxes.minZ[j]) {
                        hitCount++;
                    }
                }
            }
            


            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = end - start;
            std::cout << "\t[ 4: Split SoA ] \t\t" << elapsed.count() << " ms / Hits: " << hitCount << "\n";
        }

    }


    // --- 計測5: Flat SoA ---
    std::cout << "\nPush Enter Key...\n";
    std::cin.get();
    {

        auto start = std::chrono::high_resolution_clock::now();

        int hitCount = 0;
        {
            auto start = std::chrono::high_resolution_clock::now();

            // ポインタをローカルに持っておく（コンパイラへのヒント）
            const float* mX = flatSoaBoxes.minX; const float* MX = flatSoaBoxes.maxX;
            const float* mY = flatSoaBoxes.minY; const float* MY = flatSoaBoxes.maxY;
            const float* mZ = flatSoaBoxes.minZ; const float* MZ = flatSoaBoxes.maxZ;

            
            for (int i = 0; i < NUM_OBJECTS; ++i) {
                const float amx = mX[i]; const float amy = mY[i]; const float amz = mZ[i];
                const float aMx = MX[i]; const float aMy = MY[i]; const float aMz = MZ[i];

                for (int j = i + 1; j < NUM_OBJECTS; ++j) {
                    // 判定ロジックは他と同じ。配列から直接読み出す
                    if (amx <= MX[j] && aMx >= mX[j] &&
                        amy <= MY[j] && aMy >= mY[j] &&
                        amz <= MZ[j] && aMz >= mZ[j]) {
                        hitCount++;
                    }
                }
            }
            


            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = end - start;
            std::cout << "\t[ 5: Flat SoA ] \t\t" << elapsed.count() << " ms / Hits: " << hitCount << "\n";
        }

    }

    // --- 計測6: SIMD (SSE4.1) SoA ---
    // 一度の命令で4つのAABB判定を同時に行う
    std::cout << "\nPush Enter Key...\n";
    std::cin.get();
    {
        auto start = std::chrono::high_resolution_clock::now();
        int hitCount = 0;

        const float* mX = flatSoaBoxes.minX; const float* MX = flatSoaBoxes.maxX;
        const float* mY = flatSoaBoxes.minY; const float* MY = flatSoaBoxes.maxY;
        const float* mZ = flatSoaBoxes.minZ; const float* MZ = flatSoaBoxes.maxZ;

        for (int i = 0; i < NUM_OBJECTS; ++i) {
            // オブジェクトiのデータをレジスタにコピー（4レーン同じ値を入れる）
            __m128 ai_minX = _mm_set1_ps(mX[i]); __m128 ai_maxX = _mm_set1_ps(MX[i]);
            __m128 ai_minY = _mm_set1_ps(mY[i]); __m128 ai_maxY = _mm_set1_ps(MY[i]);
            __m128 ai_minZ = _mm_set1_ps(mZ[i]); __m128 ai_maxZ = _mm_set1_ps(MZ[i]);

            // 内側のループを4つずつ進める
            int j = i + 1;
            for (; j <= NUM_OBJECTS - 4; j += 4) {
                // オブジェクトj ~ j+3 のデータをロード
                __m128 bj_minX = _mm_loadu_ps(&mX[j]); __m128 bj_maxX = _mm_loadu_ps(&MX[j]);
                __m128 bj_minY = _mm_loadu_ps(&mY[j]); __m128 bj_maxY = _mm_loadu_ps(&MY[j]);
                __m128 bj_minZ = _mm_loadu_ps(&mZ[j]); __m128 bj_maxZ = _mm_loadu_ps(&MZ[j]);

                // AABB判定式: (a.min <= b.max) && (a.max >= b.min)
                // 各軸の比較を一斉に行う
                __m128 resX = _mm_and_ps(_mm_cmple_ps(ai_minX, bj_maxX), _mm_cmpge_ps(ai_maxX, bj_minX));
                __m128 resY = _mm_and_ps(_mm_cmple_ps(ai_minY, bj_maxY), _mm_cmpge_ps(ai_maxY, bj_minY));
                __m128 resZ = _mm_and_ps(_mm_cmple_ps(ai_minZ, bj_maxZ), _mm_cmpge_ps(ai_maxZ, bj_minZ));

                // 全軸の条件が真か確認
                __m128 finalRes = _mm_and_ps(_mm_and_ps(resX, resY), resZ);

                // 結果をビットマスクとして取得 (4ビット分)
                int mask = _mm_movemask_ps(finalRes);
                if (mask != 0) {
                    // 立っているビットの数をカウント（popcount）
                    // 衝突した数だけhitCountを加算
                    if (mask & 0x1) hitCount++;
                    if (mask & 0x2) hitCount++;
                    if (mask & 0x4) hitCount++;
                    if (mask & 0x8) hitCount++;
                }
            }
            // 4で割り切れなかった余りの要素を普通に判定
            for (; j < NUM_OBJECTS; ++j) {
                if (mX[i] <= MX[j] && MX[i] >= mX[j] &&
                    mY[i] <= MY[j] && MY[i] >= mY[j] &&
                    mZ[i] <= MZ[j] && MZ[i] >= mZ[j]) hitCount++;
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;
        std::cout << "\t[ 6: SIMD (SSE4.1) SoA ] \t" << elapsed.count() << " ms / Hits: " << hitCount << "";
        std::cout << "\t（一度の命令で4つのAABB判定）\n";
    }


    // --- 計測7: SIMD (AVX2) SoA ---
    // 一度の命令で8個のAABB判定を同時に行う
    std::cout << "\nPush Enter Key (AVX2)...\n";
    std::cin.get();
    {
        auto start = std::chrono::high_resolution_clock::now();
        int hitCount = 0;

        const float* mX = flatSoaBoxes.minX; const float* MX = flatSoaBoxes.maxX;
        const float* mY = flatSoaBoxes.minY; const float* MY = flatSoaBoxes.maxY;
        const float* mZ = flatSoaBoxes.minZ; const float* MZ = flatSoaBoxes.maxZ;

        for (int i = 0; i < NUM_OBJECTS; ++i) {
            // オブジェクトiのデータを8レーン分セット
            __m256 ai_minX = _mm256_set1_ps(mX[i]); __m256 ai_maxX = _mm256_set1_ps(MX[i]);
            __m256 ai_minY = _mm256_set1_ps(mY[i]); __m256 ai_maxY = _mm256_set1_ps(MY[i]);
            __m256 ai_minZ = _mm256_set1_ps(mZ[i]); __m256 ai_maxZ = _mm256_set1_ps(MZ[i]);

            int j = i + 1;
            // 8個ずつループ
            for (; j <= NUM_OBJECTS - 8; j += 8) {
                // オブジェクトj ~ j+7 のデータをロード
                __m256 bj_minX = _mm256_loadu_ps(&mX[j]); __m256 bj_maxX = _mm256_loadu_ps(&MX[j]);
                __m256 bj_minY = _mm256_loadu_ps(&mY[j]); __m256 bj_maxY = _mm256_loadu_ps(&MY[j]);
                __m256 bj_minZ = _mm256_loadu_ps(&mZ[j]); __m256 bj_maxZ = _mm256_loadu_ps(&MZ[j]);

                // AABB判定: (a.min <= b.max) && (a.max >= b.min)
                // _mm256_cmp_ps の第3引数 _CMP_LE_OQ は「Less or Equal (Ordered, Quiet)」
                __m256 resX = _mm256_and_ps(_mm256_cmp_ps(ai_minX, bj_maxX, _CMP_LE_OQ), _mm256_cmp_ps(ai_maxX, bj_minX, _CMP_GE_OQ));
                __m256 resY = _mm256_and_ps(_mm256_cmp_ps(ai_minY, bj_maxY, _CMP_LE_OQ), _mm256_cmp_ps(ai_maxY, bj_minY, _CMP_GE_OQ));
                __m256 resZ = _mm256_and_ps(_mm256_cmp_ps(ai_minZ, bj_maxZ, _CMP_LE_OQ), _mm256_cmp_ps(ai_maxZ, bj_minZ, _CMP_GE_OQ));

                // 全軸の条件を統合
                __m256 finalRes = _mm256_and_ps(_mm256_and_ps(resX, resY), resZ);

                // 結果をビットマスクとして取得 (8ビット分)
                int mask = _mm256_movemask_ps(finalRes);
                if (mask != 0) {
                    // 立っているビットの数をカウント（popcount）
                    // 最近のCPUなら __popcnt 命令が1サイクルで終わります
                    hitCount += __popcnt(mask);
                }
            }
            // 余りの判定
            for (; j < NUM_OBJECTS; ++j) {
                if (mX[i] <= MX[j] && MX[i] >= mX[j] &&
                    mY[i] <= MY[j] && MY[i] >= mY[j] &&
                    mZ[i] <= MZ[j] && MZ[i] >= mZ[j]) hitCount++;
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;
        std::cout << "\t[ 7: SIMD (AVX2) SoA ] \t\t" << elapsed.count() << " ms / Hits: " << hitCount << "";
        std::cout << "\t（一度の命令で8つのAABB判定）\n";
    }


    // newしたものをdelete
    for (auto p : oopEntity) delete p;

    std::cout << "\n";
    std::cout << "終了するにはEnterキーを押してください...";
    std::cin.get();

    return 0;
}