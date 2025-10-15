# SolaWGL WebGL Toolkit

週末の3日間で学習しながら WebGL のユーティリティを作りました。  

## 操作

ゲームパッド 
L Stick : 移動 
A ボタン : ジャンプ 

キーボード 
wasd : 移動 
スペース : ジャンプ 

1 キー : 音楽再生 
2 キー : 次の音楽に切り替え 
3 キー : 音楽停止 

ｆキー : フルスクリーン切り替え 

---

## 目次

- [クイックスタート](#クイックスタート)
- [SolaWGL (`solaWGL.js`)](#solawgl-solawgljs)
- [SolaMesh (`solaMesh.js`)](#solamesh-solameshjs)
- [SolaInputManager (`solaInputManager.js`)](#solainputmanager-solainputmanagerjs)
- [SolaTextureManager (`solaTextureManager.js`)](#solatexturemanager-solatexturemanagerjs)
- [SolaGltfParser (`solaGltfParser.js`)](#solagltfparser-solagltfparserjs)
- [SolaSoundManager (`solaSoundManager.js`)](#solasoundmanager-solasoundmanagerjs)
- [solaRandomGenerator (`solaRandomGenerator.js`)](#solarandomgenerator-solarandomgeneratorjs)
- [solaPerlinNoise (`solaPerlinNoise.js`)](#solaperlinnoise-solaperlinnoisejs)
- [注意事項](#注意事項)

---

## クイックスタート

HTML 側（例: `index.html`）で `canvas` を用意します。

```html
<canvas id="glCanvas" width="1280" height="720"></canvas>

<script src="./js/gl-matrix-min.js"></script>

<script src="./js/solaPerlinNoise.js"></script>
<script src="./js/solaRandomGenerator.js"></script>
<script src="./js/solaMesh.js"></script>
<script src="./js/solaTextureManager.js"></script>
<script src="./js/solaSoundManager.js"></script>
<script src="./js/solaInputManager.js"></script>
<script src="./js/solaGltfParser.js"></script>
<script src="./js/solaWGL.js"></script>
```

`main.js` で初期化とメインループを組みます。

```js

window.addEventListener('DOMContentLoaded', async () => {
  const wgl = new SolaWGL('glCanvas'); // HTML の canvas の ID を渡して初期化

  const isReady = await wgl.init();
  if (!isReady) {
    console.error('アプリケーションの初期化に失敗しました');
    return;
  }

  // ▼ ここでモデルやテクスチャなどを作成・読み込み

  // ループ
  const render = () => {
    const flg_Update = wgl.update(); // FPS 制御。true のときのみ描画を進める

    if (flg_Update) {
      // DeltaTime (秒)
      const deltaTime = wgl.getDeltaTime();

      // 入力に基づく更新処理（移動・アニメ更新など）

      // カメラ・ライト設定

      // 描画
    }

    requestAnimationFrame(render); // 次フレームを要求（ループ継続）
  };

  // ループ開始
  render();
});
```

---

## SolaWGL (`solaWGL.js`)

アプリケーション本体です。

### API

| 関数名 (引数) | 説明 |
|---|---|
| `constructor(canvasId)` | クラスの初期化。キャンバスと WebGL コンテキストの取得、プロパティ設定、イベントリスナー登録を行う。 |
| `init()` | **非同期**の初期化処理。デフォルトシェーダーのロードと使用を行う。 |
| `setLightDirection(x, y, z)` | 光の方向 (vec3) を設定。 |
| `setLightColor(r, g, b)` | 光の色 (vec3) を設定。 |
| `setLightIntensity(newIntensity)` | 光の強度 (float) を設定。 |
| `setAmbientColor(r, g, b)` | 環境光の色 (vec3) を設定。 |
| `setAmbientIntensity(newIntensity)` | 環境光の強度 (float) を設定。 |
| `setCameraTarget(x, y, z)` | カメラの注視点を設定。 |
| `setCameraPosition(x, y, z)` | カメラの位置を設定。 |
| `setCameraAngle(pitch, yaw, roll)` | カメラ角度 (Pitch/Yaw/Roll) を設定。 |
| `setCameraDistance(dist)` | 注視点からの距離を設定。 |
| `getCameraPosition()` | カメラ位置 (cameraPosition) を返す。 |
| `calcCameraPosByDistanceAndAngles()` | 距離と角度からカメラ位置を計算して反映。 |
| `useShaderProgram(key)` | シェーダプログラムをキーで切替え、ユニフォーム値を設定。 |
| `update()` | **ゲームループの更新処理**。**戻り値が `true` のときのみ描画処理**を行う（FPS 制御のため）。 |
| `getDeltaTime()` | 最後の描画からの経過時間（秒）を返す。 |
| `setFpsLimit(fps)` | 最大 FPS をセットします。 |
| `getFps()` | 現在の平均 FPS を返す。 |
| `setClearColor(r, g, b, a)` | キャンバスのクリア色を設定。 |
| `clearCanvas()` | WebGL のカラーバッファをクリア。 |
| `toggleFullscreen()` | フルスクリーン表示とウィンドウ表示を切替え。 |

---

## SolaMesh (`solaMesh.js`)

### 自作データを使う例

```js
let triangleMesh = new SolaMesh(this);

// 頂点
triangleMesh.addVertexData({
  position: [ 0.0,  1.0, 0.0 ], uv: [ 0.5, 0.0 ], normal: [ 0.0, 0.0, 1.0 ],
  boneIDs: [0.0, 0.0, 0.0, 0.0], boneWeights: [0.0, 0.0, 0.0, 0.0]
});
triangleMesh.addVertexData({
  position: [ -1.0, -1.0, 0.0 ], uv: [ 0.0, 1.0 ], normal: [ 0.0, 0.0, 1.0 ],
  boneIDs: [0.0, 0.0, 0.0, 0.0], boneWeights: [0.0, 0.0, 0.0, 0.0]
});
triangleMesh.addVertexData({
  position: [ 1.0, -1.0, 0.0 ], uv: [ 1.0, 1.0 ], normal: [ 0.0, 0.0, 1.0 ],
  boneIDs: [0.0, 0.0, 0.0, 0.0], boneWeights: [0.0, 0.0, 0.0, 0.0]
});

// インデックス
triangleMesh.addIndexData(0, 1, 2);

// ビルド（使えるデータにする）
triangleMesh.buildMesh(wgl);

// 描画
triangleMesh.setScale(1.0, 1.0, 1.0);
triangleMesh.setPosition(0.0, 0.0, 0.0);
triangleMesh.setRotation(0.0, 0.0, 0.0);
triangleMesh.draw(wgl);
```

### glTF を読み込む例

```js
let parser = wgl.gltfParser; // glTF パーサー
const meshDataList = await parser.loadModel('./gltf/character.gltf'); // glTF 読み込み

let gltfMesh = new SolaMesh(this);
gltfMesh.setMeshDataList(meshDataList);

gltfMesh.buildMesh(wgl);　// ビルド（使えるデータにする）

// テクスチャ読み込み
const myTextureKey = 'characterTex_key';
await wgl.textureManager.loadAndRegister(myTextureKey, './gltf/character_albedo.jpg');

gltfMesh.setTextureKey(myTextureKey);//テクスチャーをセット

// 描画
gltfMesh.setScale(1.0, 1.0, 1.0);
gltfMesh.setPosition(charaPos.x, charaPos.y, charaPos.z);
gltfMesh.setRotation(charaRot.x, charaRot.y, charaRot.z);
gltfMesh.draw(wgl);
```

### API

| 関数名 (引数) | 説明 |
|---|---|
| `constructor(SolaWGL)` | クラスの初期化。WebGL コンテキスト、姿勢データ、ジオメトリデータ、アニメーション関連プロパティを初期化。 |
| `addVertexData(data)` | 1 頂点の全データ（位置/UV/法線/ボーン ID/ボーン重み）を内部配列に追加。 |
| `addIndexData(idx1, idx2, idx3)` | インデックス（頂点番号）を内部配列に追加。 |
| `buildMesh(SolaWGL)` | 頂点・インデックスから WebGL の VBO/IBO を作成し GPU に転送。 |
| `setMeshDataList(meshDataList)` | glTF 等からのメッシュデータリストを受け取り、内部配列へ格納。 |
| `setAnimationData(animationDataList)` | `SolaGltfParser` から受け取ったアニメーションデータを保持。 |
| `getAnimationKey()` | 保持しているアニメーションキー（名前）の配列を返す。 |
| `playAnimation(key, loop)` | 指定キーのアニメーションを再生。`loop` でループ有無を指定。 |
| `stopAnimation()` | 再生中のアニメーションを停止し、静的ポーズに戻す。 |
| `hasAnimationData()` | アニメーションデータがセット済みかを返す。 |
| `setPosition(x, y, z)` | 位置を設定。 |
| `setRotation(x, y, z)` | 回転（度をラジアンに変換して保持）を設定。 |
| `setScale(x, y, z)` | スケールを設定。 |
| `setTextureKey(key)` | 使用するテクスチャキーを設定。 |
| `draw(glHelper)` | バッファのバインド、モデル行列・アニメ行列のユニフォーム設定、描画を実行。 |

---

## SolaInputManager (`solaInputManager.js`)

### 使用例

```js
// スティック入力
const stickL = wgl.inputManager.getStickValue('L', 0.15);
let moveVal = { x: stickL.x, y: stickL.y };

// ゲームパッドのボタン
const isAPressed = wgl.inputManager.getGamepadOnPush(BUTTONS.A);
if (isAPressed) {
  let flg_jump = true;
}

// キーボード検知
wgl.inputManager.addKeyToTrack([' ', '1', '2', '3', 'w', 'a', 's', 'd']);//事前に登録します

if (wgl.inputManager.onPressKey('w')) {

}

```

### API

| 関数名 (引数) | 説明 |
|---|---|
| `constructor(SolaWGL)` | クラスの初期化。WebGL ヘルパー、キャンバス、ゲームパッド、マウス、キーボード状態を初期化しイベントを登録。 |
| `addKeyToTrack(keyNames)` | 監視対象のキーを登録し、初期状態をリセット。 |
| `removeKeyToTrack(keyNames)` | 監視対象からキーを削除。 |
| `onPushKey(keyName)` | 指定キーが「押された瞬間」に `true` を返し、呼出後に状態をリセット。 |
| `onPressKey(keyName)` | 指定キーが「押されている間」`true` を返す。 |
| `getStickValue(stick, deadZone = 0.1)` | スティック（`'L'` / `'R'`）の傾きを、デッドゾーン適用で取得。 |
| `getMouseDelta()` | 直前フレームからのマウス移動量（デルタ）を取得。呼出後にデルタはリセット。 |
| `getGamepadOnPush(buttonIndex)` | ゲームパッドのボタンを**押した瞬間**の検知。 |
| `getGamepadOnPress(buttonIndex)` | ゲームパッドのボタンを**押下中**の検知。 |

---

## SolaTextureManager (`solaTextureManager.js`)

```js

// テクスチャ読み込み
const myTextureKey = 'characterTex_key';
await wgl.textureManager.loadAndRegister(myTextureKey, './gltf/character_albedo.jpg');

gltfMesh.setTextureKey(myTextureKey);//テクスチャーをメッシュにセット

```


### API

| 関数名 (引数) | 説明 |
|---|---|
| `constructor(SolaWGL)` | クラスの初期化。WebGL コンテキスト取得と `Map` によるテクスチャ管理を初期化。 |
| `loadAndRegister(key, url)` | 画像を**非同期**でロードし、`WebGLTexture` を作成して `Map` に登録。既に同キーがあれば既存を返す。 |
| `delete(key)` | `Map` から対象テクスチャを削除し、WebGL リソースを解放。 |
| `getTexture(key)` | 指定キーの `WebGLTexture` を取得。 |

---

## SolaGltfParser (`solaGltfParser.js`)

```js

let parser = wgl.gltfParser; // glTF パーサー
const meshDataList = await parser.loadModel('./gltf/character.gltf'); // glTF 読み込み

let gltfMesh = new SolaMesh(this);  //メッシュクラス作成

gltfMesh.setMeshDataList(meshDataList);//gltfから読みこんだ情報をセット

gltfMesh.buildMesh(wgl);　// ビルド（使えるデータにする）

```


### API

| 関数名 (引数) | 説明 |
|---|---|
| `constructor(SolaWGL)` | `SolaWGL` のインスタンスを受け取り、WebGL コンテキスト、キャッシュ、直近ロード URL などを初期化。 |
| `loadModel(gltfUrl)` | glTF を**非同期**ロードし、バイナリ・アニメ等をパース。`SolaMesh` 用ジオメトリ配列を返す。 |
| `getAnimationData()` | 直近ロード済みモデルのアニメーションデータ（キー、ボーン数、最大フレーム、ベイク済み行列配列など）を一括取得。 |

---

## SolaSoundManager (`solaSoundManager.js`)

```js

    const bgm001Key = "bgm001";　//キー
    await wgl.soundManager.loadSound(bgm001Key, "./sound/bgm001.mp3");  //ファイル読み込み

    wgl.soundManager.playSound(sound003Key, 0.2, false);  //音の再生（キー、ボリューム、ループするかどうか）

```


### API

| 関数名 (引数) | 説明 |
|---|---|
| `constructor()` | `AudioContext` と BGM チャンネルを初期化。自動再生ポリシー対応のクリックイベントを登録。 |
| `loadSound(key, url)` | 単一の音声ファイル（MP3）を**非同期**で読み込み、デコードして `soundMap` に保持。 |
| `playSound(key, volume = 1.0, loop = false)` | バッファリング済みのサウンド（SE）を再生（最大同時 8）。 |
| `playMusic(key, vol = 1.0, loop = true)` | BGM として再生。既存 BGM があれば即停止して差し替え。 |
| `fadeoutMusic(fadeoutTime)` | 再生中 BGM を指定時間でフェードアウトし停止。 |
| `crossFadeMusic(key, vol = 1.0, loop = true, fadeoutTime = 2.0)` | 旧 BGM をフェードアウトしつつ新 BGM を再生開始（クロスフェード）。 |
| `stopMusic(fadeoutTime = 1.0)` | BGM 全体を停止（アクティブな BGM は指定時間でフェードアウト）。 |

---

## solaRandomGenerator (`solaRandomGenerator.js`)

```js

    const rng = new solaRandomGenerator();  //シード値が同じなら、毎回同じ乱数が出ます
    rng.setSeed(12345);

    // 乱数を生成
    let r = rng.getRandom();//0.0 - 1.0

```

### API

| 関数名 (引数) | 説明 |
|---|---|
| `constructor()` | シードはデフォルト `1` で初期化。 |
| `setSeed(seed)` | シード値を設定（乱数列をリセット）。 |
| `getRandom()` | 次の乱数を返し、内部状態（シード）を更新。範囲は **\[0.0, 1.0)**。 |

---

## solaPerlinNoise (`solaPerlinNoise.js`)

```js

    const noiseSeed = 1234; 
    const perlin = new solaPerlinNoise(noiseSeed);

    const noiseValue = perlin.noise(x, y, z);  //(xyzの位置によって) float のノイズが返る

```

### API

| 関数名 (引数) | 説明 |
|---|---|
| `constructor(seed)` | シードを受け取り、順列配列 `P`（乱数テーブル）を決定論的にセットアップ。 |
| `noise(x, y, z)` | 3 次元のパーリンノイズ値を計算。 |

---

## 注意事項

- 本リポジトリは学習・検証を目的とした参考実装です。
- 初学者が作ったので、間違った情報が含まれている可能性があります。

---

#
