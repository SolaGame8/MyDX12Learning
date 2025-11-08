# SF::Easing ライブラリ リファレンス

このドキュメントは `Easing.h` に含まれる関数群のリファレンスです。  
補間・トランジション・アニメーション処理などで使用できます。

---


## 時間制御関数

#### `float GetTimeFactor(float counter, float transition_time = 1.0f, float idle_time = 0.0f, float counter_offset = 0.0f)`
カウンターを周期的に正規化し、`0.0〜1.0`の範囲にマッピングします。

- **counter** : 現在の時間やカウント値  
- **transition_time** : 遷移期間（0.0〜1.0へ変換する時間）  
- **idle_time** : 遷移完了後の待機時間  
- **counter_offset** : カウンターのオフセット  




| メンバー | 説明 |
|-----------|-------------|
| `counter` | 現在の時間やカウント値  |
| `transition_time` | 遷移期間（0.0〜1.0へ変換する時間） |
| `idle_time` | 遷移完了後の待機時間 |
| `counter_offset` | カウンターのオフセット |

戻り値: 周期に基づく正規化時間 `[0.0〜1.0]`。

---

#### `float GetPingPongTimeFactor(float counter, float transition_time = 1.0f, float idle_time = 0.0f, float counter_offset = 0.0f)`
カウンターを往復 (`0.0→1.0→0.0`) させる時間制御。

戻り値: ピンポン動作の正規化時間。

---

#### `TIMEFACTOR_RESULT GetSteppedTimeFactor(float counter, int steps, float transition_time = 1.0f, float idle_time = 0.0f, float counter_offset = 0.0f)`
ステップ数を指定して、複数段階で時間を分割します。

戻り値:  
`TIMEFACTOR_RESULT` 構造体  
- **step** : 現在のステップ番号  
- **time** : ステップ内の正規化時間 `[0.0〜1.0]`

---

## イージング関数群

### `float EaseIn(float t)`  
加速的に開始する（t²）。

### `float EaseInCubic(float t)`  
より急な加速で開始（t³）。

### `float EaseOut(float t)`  
減速して終了する。

### `float EaseOutCubic(float t)`  
より強い減速（1 - (1 - t)³）。

### `float EaseInOut(float t)`  
前半加速・後半減速するスムーズなカーブ。  

### `float EaseInOutQuint(float t)`  
滑らかで自然な開始・終了を行う高次補間。  

### `float EaseInBack(float t, float strength = 2.0f)`  
開始時に少し戻る（バック）動作。`strength`で戻り量を調整。

### `float EaseInBackCubic(float t, float strength = 2.0f)`  
より強いカーブのバック動作（t³ベース）。

### `float EaseOutBack(float t, float strength = 2.0f)`  
終了時に少し戻る（バック）動作。

### `float EaseOutBackCubic(float t, float strength = 2.0f)`  
より滑らかなバック終了。`t-1`に対する多項式。

### `float EaseOutElastic(float t, float bounces = 5.0f)`  
弾性運動（バネのように振動して減衰）。`bounces`で振動回数を調整。

### `float EaseOutBounce(float t, float bounces = 3.0f)`  
バウンドするように減衰。`bounces`でバウンド回数を調整。

---

## 構造体

### `struct TIMEFACTOR_RESULT`
| メンバー | 型 | 説明 |
|-----------|----|------|
| `int step` | 現在のステップ番号 |
| `float time` | ステップ内の正規化時間 `[0.0〜1.0]` |

---

## 備考
- 入力 `t` は常に `[0.0〜1.0]` の範囲を前提としています。  
- 範囲外入力に対するクランプ処理は行われていません。

# イージング関数の種類と特徴

このドキュメントは、イージング関数（補間カーブ）に関する用語と動作の特徴をまとめたものです。  
アニメーションやトランジション設計時に、動きの「質感」を選択する際の参考になります。

---

## 一般的な関数形（カーブタイプ）

| 名称 | 数学的モデル | 特徴 | 概要 |
|------|---------------|------|------|
| **Quad** | 2次関数 (t²) | ごくわずかにカーブする | 最もシンプルで軽い加減速。短いモーションに向いている。 |
| **Cubic** | 3次関数 (t³) | Quadよりもカーブが強い | 一般的なアニメーションに最もよく使われる。自然な動き。 |
| **Quart** | 4次関数 (t⁴) | Cubicよりさらに強いカーブ | スムーズで力強い加速・減速。高級感のある印象。 |
| **Quint** | 5次関数 (t⁵) | 非常に強いカーブ | 一瞬で加速・減速させるダイナミックな動き。 |
| **Sine** | 正弦関数 (sin) | スムーズで自然なカーブ | ゆるやかで滑らかな始まりと終わり。人間的な動きに近い。 |
| **Expo** | 指数関数 (2ᵗ) | 非常に急激な加速／減速 | 俊敏な反応や「瞬発的」な表現に使われる。 |
| **Circ** | 円関数（円の一部） | Sineに似ているが、より強い加速／減速感 | 重さや慣性を感じさせるモーションに適する。 |

---

## 特殊な動きに関する用語

| 名称 | 動作の特徴 | イメージ |
|------|-------------|----------|
| **Back** | 終点に到達する前に、一度オーバーシュート（行き過ぎ）て戻ってくる。 | ゴムを引っ張って離したような、弾力のある動き。 |
| **Elastic** | 終点付近でバウンド（振動）しながら停止する。 | ピアノの弦が振動して止まるような、非常に強い弾性表現。 |
| **Bounce** | 終点に向かって徐々にバウンドの幅が小さくなり停止する。 | 地面にボールを落として跳ねるような、物理的なバウンド。 |

---

## 備考
- これらの関数は「EaseIn（加速）」「EaseOut（減速）」「EaseInOut（加速→減速）」などのバリエーションとして利用されます。  
- アニメーション設計時には、動きの印象（軽快／重厚／自然／弾性など）に応じて使い分けます。  
- Expo や Quint などの高次関数は急激な変化を生み出すため、UIでは短時間モーションに使うのが効果的です。



# SF::SFFloat ライブラリ リファレンス

このドキュメントは `SFFloat.h` に含まれる構造体と関数のリファレンスです。  
ベクトル演算（2D, 3D, 4D）および補間処理を行うための軽量ユーティリティです。

---

## 構造体

### `struct SFFLOAT2`
2次元ベクトルを表します。

| メンバー | 型 | 説明 |
|-----------|----|------|
| `x` | float | X成分 |
| `y` | float | Y成分 |

#### コンストラクタ
- `SFFLOAT2()` — (0.0, 0.0) で初期化  
- `SFFLOAT2(float _x, float _y)` — 任意の値で初期化

---

### `struct SFFLOAT3`
3次元ベクトルを表します。

| メンバー | 型 | 説明 |
|-----------|----|------|
| `x` | float | X成分 |
| `y` | float | Y成分 |
| `z` | float | Z成分 |

#### コンストラクタ
- `SFFLOAT3()` — (0.0, 0.0, 0.0) で初期化  
- `SFFLOAT3(float _x, float _y, float _z)` — 任意の値で初期化

---

### `struct SFFLOAT4`
4次元ベクトル（一般的には同次座標）を表します。

| メンバー | 型 | 説明 |
|-----------|----|------|
| `x` | float | X成分 |
| `y` | float | Y成分 |
| `z` | float | Z成分 |
| `w` | float | W成分（位置 or 方向を表す補助） |

#### コンストラクタ
- `SFFLOAT4()` — (0.0, 0.0, 0.0, 0.0) で初期化  
- `SFFLOAT4(float _x, float _y, float _z, float _w)` — 任意の値で初期化

---

## ベクトル演算

### SFFLOAT2

| 演算子/関数 | 説明 |
|--------------|------|
| `operator+(A, B)` | ベクトル加算 |
| `operator-(A, B)` | ベクトル減算 |
| `operator*(A, scalar)` | スカラー倍 |
| `operator*(scalar, A)` | スカラー倍（逆順） |
| `operator/(A, scalar)` | スカラー除算 |
| `float DotValue(A, B)` | 内積（dot product） |
| `float CrossValue(A, B)` | 外積（2Dではスカラ値） |

---

### SFFLOAT3

| 演算子/関数 | 説明 |
|--------------|------|
| `operator+(A, B)` | ベクトル加算 |
| `operator-(A, B)` | ベクトル減算 |
| `operator*(A, scalar)` | スカラー倍 |
| `operator*(scalar, A)` | スカラー倍（逆順） |
| `operator/(A, scalar)` | スカラー除算 |
| `float DotValue(A, B)` | 内積（A・B） |
| `SFFLOAT3 CrossValue(A, B)` | 外積ベクトル（A×B） |

---

### SFFLOAT4

| 演算子/関数 | 説明 |
|--------------|------|
| `operator+(A, B)` | ベクトル加算 |
| `operator-(A, B)` | ベクトル減算 |
| `operator*(A, scalar)` | スカラー倍 |
| `operator*(scalar, A)` | スカラー倍（逆順） |
| `operator/(A, scalar)` | スカラー除算 |
| `float DotValue(A, B)` | 内積（4成分） |
| `SFFLOAT4 CrossValue(A, B)` | 外積（xyz成分のみを使用、w=0） |

---

## 補間関数

### `SFFLOAT2 LerpValue(SFFLOAT2 a, SFFLOAT2 b, float t)`  
2D線形補間（Lerp）。 `t` は [0.0〜1.0] の範囲。

### `SFFLOAT3 LerpValue(SFFLOAT3 a, SFFLOAT3 b, float t)`  
3D線形補間。

### `SFFLOAT4 LerpValue(SFFLOAT4 a, SFFLOAT4 b, float t)`  
4D線形補間。

### `float LerpValue(float a, float b, float t)`  
スカラー値の線形補間。

---

## 備考
- 全ての演算はインラインで定義されています。  
- `CrossValue(SFFLOAT4, SFFLOAT4)` は xyz成分のみで演算を行い、`w=0.0f` として返します。  
- 計算の高速化を目的としており、エラーチェック（ゼロ除算など）は行っていません。
