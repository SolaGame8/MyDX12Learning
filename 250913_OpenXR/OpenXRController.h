#pragma once

#include <openxr/openxr.h>

#include <array>
#include <string>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <windows.h> //OutputDebugStringA

// DirectXMath はヘッダーのみ。Windowsに依存しない利用が可能
#include <DirectXMath.h>
using namespace DirectX;

class OpenXRController {
public:
    struct State {
        bool  isActive = false;         // その手の入力が現在有効か
        //bool  select = false;           // 人差し指ボタン 決定（プロファイルによってはtrigger>0相当）
        bool  selectClick = false;   // 人差し指トリガーのクリック（デジタル）
        float selectValue = 0.0f;    // 人差し指トリガーのアナログ値(0..1)
        bool  squeezeClick = false;     // 中指ボタン つかむ(クリック)
        float squeezeValue = 0.0f;      // 中指ボタン つかむ(アナログ値 0..1)
        XrVector2f thumbstick{ 0,0 };   // スティック
        bool  thumbstickClick = false;  // スティック押し込み
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

public:
    OpenXRController() = default;
    ~OpenXRController();

    // 初期化：インスタンス/セッション/アプリ空間を渡す
    bool Initialize(XrInstance instance, XrSession session, XrSpace appSpace);

    bool MakeAction(XrActionType type, const char* name, const char* loc, XrAction& out);

    bool SuggestBindings(const char* profile, std::initializer_list<XrActionSuggestedBinding> bindings);

    // aim/grip のActionSpaceを左右分作成
    bool CreateSpaces();

    // 毎フレーム：アクション同期＆読み取り（predictedDisplayTimeはxrWaitFrameで得たもの）
    bool Sync(XrSession session, XrTime predictedDisplayTime);


    XrPosef GetPose_LeftController() const;
    XrPosef GetPose_RightController() const;

    XrVector2f GetValue_Left_Stick()  const;
    XrVector2f GetValue_Right_Stick() const;
    XrVector2f GetValue_Left_Stick(float deadzone)  const;
    XrVector2f GetValue_Right_Stick(float deadzone) const;

    float GetValue_Left_SelectTrigger()  const;
    float GetValue_Right_SelectTrigger() const;

    float GetValue_Left_SqueezeTrigger()  const;
    float GetValue_Right_SqueezeTrigger() const;

    bool OnPushSelectTrigger(bool leftHand) const;
    bool OnPushSqueezeTrigger(bool leftHand) const;
    bool OnPushA(bool leftHand) const;
    bool OnPushB(bool leftHand) const;
    bool OnPushX(bool leftHand) const;
    bool OnPushY(bool leftHand) const;

    bool OnPushMenu(bool leftHand) const;

    bool OnPushStick(bool leftHand) const;


    // Select（人差し指トリガー）押した瞬間
    bool OnPush_Left_SelectTrigger()  const;
    bool OnPush_Right_SelectTrigger() const;

    // Squeeze（握りのトリガー）押した瞬間
    bool OnPush_Left_SqueezeTrigger()  const;
    bool OnPush_Right_SqueezeTrigger() const;

    // A/B/X/Y ボタン押した瞬間
    bool OnPush_Left_A()  const;
    bool OnPush_Right_A() const;

    bool OnPush_Left_B()  const;
    bool OnPush_Right_B() const;

    bool OnPush_Left_X()  const;
    bool OnPush_Right_X() const;

    bool OnPush_Left_Y()  const;
    bool OnPush_Right_Y() const;

    // Menu（メニューボタン）押した瞬間
    bool OnPush_Left_Menu()  const;
    bool OnPush_Right_Menu() const;

    bool OnPush_Left_Stick()  const;
    bool OnPush_Right_Stick() const;

    // ハプティクス（振動）
    bool ApplyHaptics(bool leftHand, float amplitude, float seconds, float frequencyHz = 0.0f);

    // 現在の左右状態
    const State& Left()  const { return stateLeft; }
    const State& Right() const { return stateRight; }




    // Pose → LHモデル行列（DirectXMath）
    static DirectX::XMMATRIX PoseToLHMatrix(const XrPosef& pose);

    // 任意：現在のインタラクションプロファイルの文字列（/interaction_profiles/...）
    std::string GetCurrentInteractionProfilePath(bool leftHand) const;

    // 破棄（デストラクタで呼ばれるが、明示破棄も可能）
    void OnDestroy();

private:
    // 内部ヘルパ
    static XrPath StrToPath(XrInstance inst, const char* s);

    bool CreateActionsAndBindings();
    bool AttachActionSet(XrSession session);

    // 状態読み取り
    bool ReadBool(XrAction a, XrPath sub, bool& dst) const;
    bool ReadFloat(XrAction a, XrPath sub, float& dst) const;
    bool ReadVec2(XrAction a, XrPath sub, XrVector2f& dst) const;
    bool Locate(XrSpace space, XrPosef& dst, bool& has, XrTime t) const;

private:
    // 外部参照
    XrInstance xrInstance_ = XR_NULL_HANDLE;
    XrSession  xrSession_ = XR_NULL_HANDLE;
    XrSpace    xrAppSpace_ = XR_NULL_HANDLE;

    // Action set / actions
    XrActionSet actionSet_ = XR_NULL_HANDLE;
    //XrAction actSelect_ = XR_NULL_HANDLE; // boolean or trigger>0相当
    XrAction actTriggerClick_ = XR_NULL_HANDLE; // boolean
    XrAction actTriggerValue_ = XR_NULL_HANDLE; // float
    XrAction actSqueezeClick_ = XR_NULL_HANDLE; // boolean
    XrAction actSqueezeValue_ = XR_NULL_HANDLE; // float
    XrAction actThumbstick_ = XR_NULL_HANDLE; // vec2
    XrAction actThumbClick_ = XR_NULL_HANDLE; // boolean
    XrAction actMenu_ = XR_NULL_HANDLE; // boolean
    XrAction actAimPose_ = XR_NULL_HANDLE; // pose
    XrAction actGripPose_ = XR_NULL_HANDLE; // pose
    XrAction actHaptic_ = XR_NULL_HANDLE; // vibration output

    // A/B/X/Y 用のアクション
    XrAction actA_ = XR_NULL_HANDLE;
    XrAction actB_ = XR_NULL_HANDLE;
    XrAction actX_ = XR_NULL_HANDLE;
    XrAction actY_ = XR_NULL_HANDLE;

    // hands
    XrPath pathLeft = XR_NULL_PATH;
    XrPath pathRight = XR_NULL_PATH;

    // Spaces
    XrSpace spaceAimL = XR_NULL_HANDLE, apaceAimR = XR_NULL_HANDLE;
    XrSpace spaceGripL = XR_NULL_HANDLE, spaceGripR = XR_NULL_HANDLE;

    // 読み取り結果（左右）
    State stateLeft, stateRight;

    // OnPush 1フレーム前の状態（エッジ検出用）
    State prevLeft_{}, prevRight_{};
    bool  prevValid_ = false; // prevが有効かどうか（初回はfalse）

};


