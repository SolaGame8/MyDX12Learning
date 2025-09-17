
![img](screenshots/runtime_quest.jpg)

# OpenXRManager (DX12) — README

DirectX 12 向けの OpenXR ヘルパークラス **OpenXRManager** の簡易的な使い方まとめです。  
初期化・毎フレーム更新・コントローラー入力/振動・描画手順・終了までを最小構成で記載しています。

---

## 目次
- [前提](#前提)
- [使い方の流れ](#使い方の流れ)
  - [初期化](#初期化)
  - [毎フレーム（必須）](#毎フレーム必須)
  - [コントローラー情報の取得](#コントローラー情報の取得)
  - [コントローラー API 一覧](#コントローラー-api-一覧)
  - [コントローラー振動](#コントローラー振動)
  - [ヘッドマウント情報](#ヘッドマウント情報)
  - [描画の手順](#描画の手順)
  - [終了処理](#終了処理)

---

## 前提

- C++ / DirectX 12
- OpenXR ランタイム（Quest/SteamVR など）
- `OpenXRManager` クラスがプロジェクトに組み込まれていること

---

## 使い方の流れ

### 初期化

```cpp
// 使い方

OpenXRManager* XR_Manager;

// ＜初期化＞
XR_Manager = new OpenXRManager();
XR_Manager->Initialize(dx12Device.Get(), commandQueue.Get());
```

---

### 毎フレーム（必須）

```cpp
// ＜毎フレーム　＊必須＞
XR_Manager->UpdateSessionState(); // OpenXR の情報更新
```

---

### コントローラー情報の取得

```cpp
// ＜コントローラー情報取得＞
if (XR_Manager->controllersReady) {

    const float deadzone = 0.05f; // デッドゾーン（微細な振動とかは取らない）
    XrVector2f stick = XR_Manager->controller.GetValue_Right_Stick(deadzone); // 右スティック xy

    bool flg = XR_Manager->controller.OnPush_Left_X(); // 左 Xボタン

    XrPosef rightControllerPose = XR_Manager->controller.GetPose_RightController(); // 右コントローラーのポーズ
    // rightControllerPose.position  // x, y, z
    // rightControllerPose.rotation  // x, y, z, w（クォータニオン）
}
```

---

### コントローラー API 一覧

```cpp
// 位置・姿勢
XrPosef GetPose_LeftController()  const;
XrPosef GetPose_RightController() const;

// スティック（オプションでデッドゾーン指定可）
XrVector2f GetValue_Left_Stick()              const;
XrVector2f GetValue_Right_Stick()             const;
XrVector2f GetValue_Left_Stick(float dz)      const;
XrVector2f GetValue_Right_Stick(float dz)     const;

// トリガー値（アナログ 0..1）
float GetValue_Left_SelectTrigger()  const;  // 人差し指
float GetValue_Right_SelectTrigger() const;
float GetValue_Left_SqueezeTrigger()  const; // 握り
float GetValue_Right_SqueezeTrigger() const;

// 押下イベント（押した“瞬間”）
bool OnPushSelectTrigger(bool leftHand) const;
bool OnPushSqueezeTrigger(bool leftHand) const;
bool OnPushA(bool leftHand) const;
bool OnPushB(bool leftHand) const;
bool OnPushX(bool leftHand) const;
bool OnPushY(bool leftHand) const;
bool OnPushMenu(bool leftHand) const;
bool OnPushStick(bool leftHand) const;

// 利き手別ショートカット
bool OnPush_Left_SelectTrigger()   const;
bool OnPush_Right_SelectTrigger()  const;
bool OnPush_Left_SqueezeTrigger()  const;
bool OnPush_Right_SqueezeTrigger() const;
bool OnPush_Left_A()  const;  bool OnPush_Right_A()  const;
bool OnPush_Left_B()  const;  bool OnPush_Right_B()  const;
bool OnPush_Left_X()  const;  bool OnPush_Right_X()  const;
bool OnPush_Left_Y()  const;  bool OnPush_Right_Y()  const;
bool OnPush_Left_Menu()  const; bool OnPush_Right_Menu() const;
bool OnPush_Left_Stick() const;  bool OnPush_Right_Stick() const;

// ハプティクス（振動）
bool ApplyHaptics(bool leftHand, float amplitude, float seconds, float frequencyHz = 0.0f);

// 現在の左右状態
const State& Left()  const { return stateLeft; }
const State& Right() const { return stateRight; }
```

`State` 構造体：

```cpp
struct State {
    bool  isActive = false;         // その手の入力が現在有効か
    bool  triggerClick = false;     // 人差し指トリガーのクリック（デジタル）
    float triggerValue = 0.0f;      // 人差し指トリガーのアナログ値(0..1)
    bool  squeezeClick = false;     // 中指ボタン つかむ(クリック)
    float squeezeValue = 0.0f;      // 中指ボタン つかむ(アナログ値 0..1)
    XrVector2f stickValue{ 0,0 };   // スティック
    bool  stickClick = false;       // スティック押し込み
    bool  menu = false;             // メニューボタン

    bool  buttonA = false;          // Aボタン
    bool  buttonB = false;          // Bボタン
    bool  buttonX = false;          // Xボタン
    bool  buttonY = false;          // Yボタン

    XrPosef aimPose{};              // レイ用途の姿勢
    XrPosef gripPose{};             // コントローラ実位置
    bool  hasAimPose = false;
    bool  hasGripPose = false;
};
```

---

### コントローラー振動

```cpp
// ＜コントローラー振動＞
XR_Manager->controller.ApplyHaptics(
    /*leftHand=*/true,
    /*amplitude=*/0.5f,
    /*seconds=*/0.5f,
    /*frequencyHz=*/0.0f // 0.0 でランタイム任せにできる
);
```

---

### ヘッドマウント情報

```cpp
// ＜ヘッドマウント情報＞
int viewNum = XR_Manager->xr_viewCount; // ビューの数（両目なので = 2）
```

---

### 描画の手順

```cpp
// ＜描画の手順＞

XrTime predictedDisplayTime; // 描画予定時間

// ＊フレーム開始（ここで描画予定時間を受け取る）
XR_Manager->BeginFrame(predictedDisplayTime);

// 目ごとの行列格納用
struct EyeMatrix {
    XMMATRIX viewMat; // ビュー行列 (World→View)
    XMMATRIX projMat; // プロジェクション行列
    XrView   xrView;  // OpenXR の生データ
};

std::vector<OpenXRManager::EyeMatrix> eyesData; // ヘッドマウントの両目のカメラ行列（DX12 向け）
float nearZ = 0.01f; // 近クリップ面
float farZ  = 100.0f; // 遠クリップ面

// VR の両目位置のカメラ行列を取得
XR_Manager->GetEyeMatrix(predictedDisplayTime, nearZ, farZ, eyesData);

// ---- ここから各ビュー（目）に対する描画 ----
for (uint32_t viewIdx = 0; viewIdx < eyesData.size(); ++viewIdx) {

    OpenXRManager::EyeDirectTarget tgt{}; // 描画先（RTV/DSV など）
    XR_Manager->GetSwapchainDrawTarget(commandList.Get(), viewIdx, tgt); // スワップチェーンの描画先を取得

    // DX12 の描画設定
    const FLOAT clearColor[4] = {0, 0, 0, 1};
    commandList->ClearRenderTargetView(tgt.rtv, clearColor, 0, nullptr); // レンダーターゲットをクリア
    commandList->ClearDepthStencilView(tgt.dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); // 深度クリア
    commandList->OMSetRenderTargets(1, &tgt.rtv, FALSE, &tgt.dsv); // 出力先セット

    // ＊ここで DX12 コマンドで実際の描画を行う（eyesData[viewIdx].viewMat / projMat を使用）

    // スワップチェーンへの描画終了
    XR_Manager->FinishSwapchainDrawTarget(commandList.Get(), viewIdx);
}

// ＊コマンドを閉じて実行し、ターゲットへの描画を完了させる
commandList->Close();
// → コマンドキューに Execute、フェンスで同期など適宜

// ＊フレーム終了
XR_Manager->EndFrame_WithProjection(
    eyesData,            // 取得した両目のカメラ行列
    nearZ, farZ,         // カメラのニア・ファー
    predictedDisplayTime // 実際に描画した情報をヘッドマウントへ渡す
);
```

> **メモ**: 上記では `viewIdx` ごとに `GetSwapchainDrawTarget` → 描画 → `FinishSwapchainDrawTarget` の順で処理しています。

---

### 終了処理

```cpp
// ＊アプリケーション終了時
XR_Manager->OnDestroy(); // 破棄
delete XR_Manager;
XR_Manager = nullptr;
```



