// OpenXRController.cpp

#include "OpenXRController.h"
#include <cstring>





// ===== 初期化 =====
bool OpenXRController::Initialize(XrInstance instance, XrSession session, XrSpace appSpace) {

    xrInstance_ = instance;
    xrSession_ = session;
    xrAppSpace_ = appSpace;
    if (xrInstance_ == XR_NULL_HANDLE || xrSession_ == XR_NULL_HANDLE || xrAppSpace_ == XR_NULL_HANDLE) return false;

    if (!CreateActionsAndBindings()) return false;
    if (!CreateSpaces()) return false;

    if (!AttachActionSet(session)) return false;


    return true;
}





bool OpenXRController::CreateActionsAndBindings() {


    // ActionSet

    //アクションセットを作る

    {
        XrActionSetCreateInfo ci{ XR_TYPE_ACTION_SET_CREATE_INFO };
        strncpy_s(ci.actionSetName, XR_MAX_ACTION_SET_NAME_SIZE, "gameplay", _TRUNCATE);
        strncpy_s(ci.localizedActionSetName, XR_MAX_LOCALIZED_ACTION_SET_NAME_SIZE, "Gameplay", _TRUNCATE);

        ci.priority = 0;
        XrResult r = xrCreateActionSet(xrInstance_, &ci, &actionSet_);
        if (!XR_SUCCEEDED(r)) {
            char buf[256];
            sprintf_s(buf, "[XR][ERROR] xrCreateActionSet failed (0x%08X)\n", r);
            OutputDebugStringA(buf);
            return false;
        }
        OutputDebugStringA("[XR] ActionSet 'gameplay' created OK\n");
    }

    // 手へのパス

    pathLeft = StrToPath(xrInstance_, "/user/hand/left");
    pathRight = StrToPath(xrInstance_, "/user/hand/right");

    

    /*
    auto makeAction = [&](XrActionType type, const char* name, const char* loc, XrAction& out) {
        XrActionCreateInfo ai{ XR_TYPE_ACTION_CREATE_INFO };
        ai.actionType = type;
        strncpy_s(ai.actionName, XR_MAX_ACTION_NAME_SIZE, name, _TRUNCATE);
        strncpy_s(ai.localizedActionName, XR_MAX_LOCALIZED_ACTION_NAME_SIZE, loc, _TRUNCATE);

        ai.countSubactionPaths = 2;
        ai.subactionPaths = pathHands;
        XrResult r = xrCreateAction(actionSet_, &ai, &out);
        char buf[256];
        sprintf_s(buf, "[XR] makeAction %-16s -> %s (0x%08X)\n",
            name, XR_SUCCEEDED(r) ? "OK" : "FAILED", r);
        OutputDebugStringA(buf);
        return XR_SUCCEEDED(r);
        };
        */

    //アクションセットにアクションを追加

    // 基本アクション作成
    if (!MakeAction(XR_ACTION_TYPE_BOOLEAN_INPUT, "select_click", "SelectClick", actTriggerClick_)) return false;
    if (!MakeAction(XR_ACTION_TYPE_FLOAT_INPUT, "select_value", "Select", actTriggerValue_)) return false;
    if (!MakeAction(XR_ACTION_TYPE_BOOLEAN_INPUT, "squeeze_click", "SqueezeClick", actSqueezeClick_)) return false;
    if (!MakeAction(XR_ACTION_TYPE_FLOAT_INPUT, "squeeze", "Squeeze", actSqueezeValue_)) return false;
    if (!MakeAction(XR_ACTION_TYPE_VECTOR2F_INPUT, "thumbstick", "ThumbStick", actThumbstick_)) return false;
    if (!MakeAction(XR_ACTION_TYPE_BOOLEAN_INPUT, "thumb_click", "ThumbClick", actThumbClick_)) return false;
    if (!MakeAction(XR_ACTION_TYPE_BOOLEAN_INPUT, "menu", "Menu", actMenu_)) return false;
    if (!MakeAction(XR_ACTION_TYPE_POSE_INPUT, "aim_pose", "AimPose", actAimPose_)) return false;
    if (!MakeAction(XR_ACTION_TYPE_POSE_INPUT, "grip_pose", "GripPose", actGripPose_)) return false;
    if (!MakeAction(XR_ACTION_TYPE_VIBRATION_OUTPUT, "haptic", "Haptic", actHaptic_)) return false;

    // 文字ボタン アクション
    if (!MakeAction(XR_ACTION_TYPE_BOOLEAN_INPUT, "button_a", "Button A", actA_)) return false;
    if (!MakeAction(XR_ACTION_TYPE_BOOLEAN_INPUT, "button_b", "Button B", actB_)) return false;
    if (!MakeAction(XR_ACTION_TYPE_BOOLEAN_INPUT, "button_x", "Button X", actX_)) return false;
    if (!MakeAction(XR_ACTION_TYPE_BOOLEAN_INPUT, "button_y", "Button Y", actY_)) return false;

    /*
    // バインディング提案関数
    auto suggest = [&](const char* profile, std::initializer_list<XrActionSuggestedBinding> b) {

        XrInteractionProfileSuggestedBinding sb{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
        sb.interactionProfile = StrToPath(xrInstance_, profile);
        sb.countSuggestedBindings = (uint32_t)b.size();
        sb.suggestedBindings = const_cast<XrActionSuggestedBinding*>(b.begin());

        XrResult r = xrSuggestInteractionProfileBindings(xrInstance_, &sb);
        char buf[256];
        sprintf_s(buf, "[XR] suggest %-40s -> %s (0x%08X)\n",
            profile, XR_SUCCEEDED(r) ? "OK" : "FAILED", r);
        OutputDebugStringA(buf);
        return XR_SUCCEEDED(r);
        };
        */


    //バインディングの提案（このプロファイルの時は、このようにバインディングして、という提案



    // Khronos Simple Controller（/interaction_profiles/khr/simple_controller）
    SuggestBindings("/interaction_profiles/khr/simple_controller", {
        // ====== Buttons（BOOLEAN）======
        { actTriggerClick_,   StrToPath(xrInstance_, "/user/hand/left/input/select/click") },
        { actTriggerClick_,   StrToPath(xrInstance_, "/user/hand/right/input/select/click") },
        { actMenu_,     StrToPath(xrInstance_, "/user/hand/left/input/menu/click") },
        { actMenu_,     StrToPath(xrInstance_, "/user/hand/right/input/menu/click") },

        // ====== Poses（POSE）======
        { actGripPose_, StrToPath(xrInstance_, "/user/hand/left/input/grip/pose") },
        { actGripPose_, StrToPath(xrInstance_, "/user/hand/right/input/grip/pose") },
        { actAimPose_,  StrToPath(xrInstance_, "/user/hand/left/input/aim/pose") },
        { actAimPose_,  StrToPath(xrInstance_, "/user/hand/right/input/aim/pose") },

        // ====== Haptics（VIBRATION_OUTPUT）======
        { actHaptic_,   StrToPath(xrInstance_, "/user/hand/left/output/haptic") },
        { actHaptic_,   StrToPath(xrInstance_, "/user/hand/right/output/haptic") },
        });

    /*
    // khr/simple_controller
    SuggestBindings("/interaction_profiles/khr/simple_controller", {
        { actSelectClick_, StrToPath(xrInstance_, "/user/hand/left/input/select/click") },
        { actSelectClick_, StrToPath(xrInstance_, "/user/hand/right/input/select/click") },
        { actAimPose_, StrToPath(xrInstance_, "/user/hand/left/input/aim/pose") },
        { actAimPose_, StrToPath(xrInstance_, "/user/hand/right/input/aim/pose") },
        { actHaptic_,  StrToPath(xrInstance_, "/user/hand/left/output/haptic") },
        { actHaptic_,  StrToPath(xrInstance_, "/user/hand/right/output/haptic") },
        });
      */



    // Oculus Touch（/interaction_profiles/oculus/touch_controller）
    SuggestBindings("/interaction_profiles/oculus/touch_controller", {
        // ====== Analog（型: FLOAT）======
        // Trigger
        { actTriggerValue_,  StrToPath(xrInstance_, "/user/hand/left/input/trigger/value") },
        { actTriggerValue_,  StrToPath(xrInstance_, "/user/hand/right/input/trigger/value") },
        // Squeeze（Grip）
        { actSqueezeValue_, StrToPath(xrInstance_, "/user/hand/left/input/squeeze/value") },
        { actSqueezeValue_, StrToPath(xrInstance_, "/user/hand/right/input/squeeze/value") },

        // ====== Trigger touch（型: BOOLEAN）======
        // （必要ならアクションを用意）
        // { actTriggerTouch_, StrToPath(xrInstance_, "/user/hand/left/input/trigger/touch") },
        // { actTriggerTouch_, StrToPath(xrInstance_, "/user/hand/right/input/trigger/touch") },

        // ====== Thumbstick（型: VECTOR2F / 補助のX/Yは FLOAT）======
        { actThumbstick_,   StrToPath(xrInstance_, "/user/hand/left/input/thumbstick") },
        { actThumbstick_,   StrToPath(xrInstance_, "/user/hand/right/input/thumbstick") },
        { actThumbClick_,   StrToPath(xrInstance_, "/user/hand/left/input/thumbstick/click") },
        { actThumbClick_,   StrToPath(xrInstance_, "/user/hand/right/input/thumbstick/click") },
        // 任意：X/Yを個別で取りたい場合
        // { actThumbX_,     StrToPath(xrInstance_, "/user/hand/left/input/thumbstick/x") },
        // { actThumbX_,     StrToPath(xrInstance_, "/user/hand/right/input/thumbstick/x") },
        // { actThumbY_,     StrToPath(xrInstance_, "/user/hand/left/input/thumbstick/y") },
        // { actThumbY_,     StrToPath(xrInstance_, "/user/hand/right/input/thumbstick/y") },
        // 任意：Thumbstick touch
        // { actThumbTouch_, StrToPath(xrInstance_, "/user/hand/left/input/thumbstick/touch") },
        // { actThumbTouch_, StrToPath(xrInstance_, "/user/hand/right/input/thumbstick/touch") },

        // ====== Thumbrest touch（型: BOOLEAN）======
        // { actThumbrestTouch_, StrToPath(xrInstance_, "/user/hand/left/input/thumbrest/touch") },
        // { actThumbrestTouch_, StrToPath(xrInstance_, "/user/hand/right/input/thumbrest/touch") },

        // ====== Buttons（型: BOOLEAN）======
        // Left: X / Y / Menu
        { actX_,            StrToPath(xrInstance_, "/user/hand/left/input/x/click") },
        // { actXTouch_,    StrToPath(xrInstance_, "/user/hand/left/input/x/touch") },
        { actY_,            StrToPath(xrInstance_, "/user/hand/left/input/y/click") },
        // { actYTouch_,    StrToPath(xrInstance_, "/user/hand/left/input/y/touch") },
        { actMenu_,         StrToPath(xrInstance_, "/user/hand/left/input/menu/click") },

        // Right: A / B / System
        { actA_,            StrToPath(xrInstance_, "/user/hand/right/input/a/click") },
        // { actATouch_,    StrToPath(xrInstance_, "/user/hand/right/input/a/touch") },
        { actB_,            StrToPath(xrInstance_, "/user/hand/right/input/b/click") },
        // { actBTouch_,    StrToPath(xrInstance_, "/user/hand/right/input/b/touch") },
        // System（ランタイムがアプリ露出を制限する場合あり）
        // { actSystem_,    StrToPath(xrInstance_, "/user/hand/right/input/system/click") },

        // ====== Poses（型: POSE）======
        { actGripPose_,     StrToPath(xrInstance_, "/user/hand/left/input/grip/pose") },
        { actGripPose_,     StrToPath(xrInstance_, "/user/hand/right/input/grip/pose") },
        { actAimPose_,      StrToPath(xrInstance_, "/user/hand/left/input/aim/pose") },
        { actAimPose_,      StrToPath(xrInstance_, "/user/hand/right/input/aim/pose") },

        // ====== Haptics（型: VIBRATION_OUTPUT）======
        { actHaptic_,       StrToPath(xrInstance_, "/user/hand/left/output/haptic") },
        { actHaptic_,       StrToPath(xrInstance_, "/user/hand/right/output/haptic") },
        });

    /*
    // oculus/touch_controller
    SuggestBindings("/interaction_profiles/oculus/touch_controller", {
        { actSelectValue_, StrToPath(xrInstance_, "/user/hand/left/input/trigger/value") },
        { actSelectValue_, StrToPath(xrInstance_, "/user/hand/right/input/trigger/value") },
        { actSqueezeValue_,StrToPath(xrInstance_, "/user/hand/left/input/squeeze/value") },
        { actSqueezeValue_,StrToPath(xrInstance_, "/user/hand/right/input/squeeze/value") },
        { actThumbstick_,   StrToPath(xrInstance_, "/user/hand/left/input/thumbstick") },
        { actThumbstick_,   StrToPath(xrInstance_, "/user/hand/right/input/thumbstick") },
        { actThumbClick_,   StrToPath(xrInstance_, "/user/hand/left/input/thumbstick/click") },
        { actThumbClick_,   StrToPath(xrInstance_, "/user/hand/right/input/thumbstick/click") },
        { actMenu_,         StrToPath(xrInstance_, "/user/hand/left/input/menu/click") },

        { actAimPose_,      StrToPath(xrInstance_, "/user/hand/left/input/aim/pose") },
        { actAimPose_,      StrToPath(xrInstance_, "/user/hand/right/input/aim/pose") },
        { actGripPose_,     StrToPath(xrInstance_, "/user/hand/left/input/grip/pose") },
        { actGripPose_,     StrToPath(xrInstance_, "/user/hand/right/input/grip/pose") },

        { actHaptic_,       StrToPath(xrInstance_, "/user/hand/left/output/haptic") },
        { actHaptic_,       StrToPath(xrInstance_, "/user/hand/right/output/haptic") },

        { actA_,            StrToPath(xrInstance_, "/user/hand/right/input/a/click") },
        { actB_,            StrToPath(xrInstance_, "/user/hand/right/input/b/click") },
        { actX_,            StrToPath(xrInstance_, "/user/hand/left/input/x/click") },
        { actY_,            StrToPath(xrInstance_, "/user/hand/left/input/y/click") },
        });
        */


    
    // PICO 4 向け（Android / Link いずれも同じパス）
    // ※ A/B/X/Y は PICO4 の OpenXR プロファイルでは公開されていないため外しています。
    //   代わりに基本操作は trigger / squeeze / thumbstick / menu でカバーします。
    SuggestBindings("/interaction_profiles/bytedance/pico4_controller", {
        // ====== Analog / Click / Touch ======
        // Trigger
        { actTriggerValue_, StrToPath(xrInstance_, "/user/hand/left/input/trigger/value") },
        { actTriggerValue_, StrToPath(xrInstance_, "/user/hand/right/input/trigger/value") },

        // もし「クリック」「タッチ」を扱うアクションを用意しているなら有効化
         { actTriggerClick_, StrToPath(xrInstance_, "/user/hand/left/input/trigger/click") },
         { actTriggerClick_, StrToPath(xrInstance_, "/user/hand/right/input/trigger/click") },
        // { actTriggerTouch_, StrToPath(xrInstance_, "/user/hand/left/input/trigger/touch") },
        // { actTriggerTouch_, StrToPath(xrInstance_, "/user/hand/right/input/trigger/touch") },

        // Squeeze (Grip)
        { actSqueezeValue_, StrToPath(xrInstance_, "/user/hand/left/input/squeeze/value") },
        { actSqueezeValue_, StrToPath(xrInstance_, "/user/hand/right/input/squeeze/value") },
        // { actSqueezeClick_, StrToPath(xrInstance_, "/user/hand/left/input/squeeze/click") },
        // { actSqueezeClick_, StrToPath(xrInstance_, "/user/hand/right/input/squeeze/click") },

        // Thumbstick (vec2) + click (+touch 任意)
        { actThumbstick_,   StrToPath(xrInstance_, "/user/hand/left/input/thumbstick") },
        { actThumbstick_,   StrToPath(xrInstance_, "/user/hand/right/input/thumbstick") },
        { actThumbClick_,   StrToPath(xrInstance_, "/user/hand/left/input/thumbstick/click") },
        { actThumbClick_,   StrToPath(xrInstance_, "/user/hand/right/input/thumbstick/click") },
        // { actThumbTouch_,   StrToPath(xrInstance_, "/user/hand/left/input/thumbstick/touch") },
        // { actThumbTouch_,   StrToPath(xrInstance_, "/user/hand/right/input/thumbstick/touch") },

        // もし X/Y を個別 float としても取りたいなら（任意）
        // { actThumbX_, StrToPath(xrInstance_, "/user/hand/left/input/thumbstick/x") },
        // { actThumbX_, StrToPath(xrInstance_, "/user/hand/right/input/thumbstick/x") },
        // { actThumbY_, StrToPath(xrInstance_, "/user/hand/left/input/thumbstick/y") },
        // { actThumbY_, StrToPath(xrInstance_, "/user/hand/right/input/thumbstick/y") },

        // ====== Buttons (ABXY + Menu/System) ======
        // Left: X, Y, Menu
        { actX_,            StrToPath(xrInstance_, "/user/hand/left/input/x/click") },
        // { actXTouch_,    StrToPath(xrInstance_, "/user/hand/left/input/x/touch") },
        { actY_,            StrToPath(xrInstance_, "/user/hand/left/input/y/click") },
        // { actYTouch_,    StrToPath(xrInstance_, "/user/hand/left/input/y/touch") },
        { actMenu_,         StrToPath(xrInstance_, "/user/hand/left/input/menu/click") },

        // Right: A, B
        { actA_,            StrToPath(xrInstance_, "/user/hand/right/input/a/click") },
        // { actATouch_,    StrToPath(xrInstance_, "/user/hand/right/input/a/touch") },
        { actB_,            StrToPath(xrInstance_, "/user/hand/right/input/b/click") },
        // { actBTouch_,    StrToPath(xrInstance_, "/user/hand/right/input/b/touch") },

        // System（プロファイル定義では user_path なしの system=true）
        // ランタイムが許可していない場合は無効化されます
        // { actSystem_,      StrToPath(xrInstance_, "/input/system/click") },

        // ====== Poses ======
        { actAimPose_,      StrToPath(xrInstance_, "/user/hand/left/input/aim/pose") },
        { actAimPose_,      StrToPath(xrInstance_, "/user/hand/right/input/aim/pose") },
        { actGripPose_,     StrToPath(xrInstance_, "/user/hand/left/input/grip/pose") },
        { actGripPose_,     StrToPath(xrInstance_, "/user/hand/right/input/grip/pose") },

        // ====== Haptics ======
        { actHaptic_,       StrToPath(xrInstance_, "/user/hand/left/output/haptic") },
        { actHaptic_,       StrToPath(xrInstance_, "/user/hand/right/output/haptic") },
        });



    // Valve Index Controller（/interaction_profiles/valve/index_controller）
    SuggestBindings("/interaction_profiles/valve/index_controller", {
        // ===== System（制限されることが多い）======
        // { actSystem_,        StrToPath(xrInstance_, "/user/hand/left/input/system/click") },
        // { actSystem_,        StrToPath(xrInstance_, "/user/hand/right/input/system/click") },
        // { actSystemTouch_,   StrToPath(xrInstance_, "/user/hand/left/input/system/touch") },
        // { actSystemTouch_,   StrToPath(xrInstance_, "/user/hand/right/input/system/touch") },

        // ===== Buttons: A / B（左右とも）======
        { actA_,              StrToPath(xrInstance_, "/user/hand/left/input/a/click") },
        { actA_,              StrToPath(xrInstance_, "/user/hand/right/input/a/click") },
        // { actATouch_,      StrToPath(xrInstance_, "/user/hand/left/input/a/touch") },
        // { actATouch_,      StrToPath(xrInstance_, "/user/hand/right/input/a/touch") },

        { actB_,              StrToPath(xrInstance_, "/user/hand/left/input/b/click") },
        { actB_,              StrToPath(xrInstance_, "/user/hand/right/input/b/click") },
        // { actBTouch_,      StrToPath(xrInstance_, "/user/hand/left/input/b/touch") },
        // { actBTouch_,      StrToPath(xrInstance_, "/user/hand/right/input/b/touch") },

        // ===== Squeeze（つかみ：値 + 圧力）======
        { actSqueezeValue_,   StrToPath(xrInstance_, "/user/hand/left/input/squeeze/value") },
        { actSqueezeValue_,   StrToPath(xrInstance_, "/user/hand/right/input/squeeze/value") },
        // 物理的な握力（圧力）を別途取りたい場合
        // { actSqueezeForce_, StrToPath(xrInstance_, "/user/hand/left/input/squeeze/force") },
        // { actSqueezeForce_, StrToPath(xrInstance_, "/user/hand/right/input/squeeze/force") },

        // ===== Trigger（クリック/値/タッチ）======
        { actTriggerClick_,   StrToPath(xrInstance_, "/user/hand/left/input/trigger/click") },
        { actTriggerClick_,   StrToPath(xrInstance_, "/user/hand/right/input/trigger/click") },

        // あなたの設計に合わせて：trigger の値は従来どおり select に集約
        { actTriggerValue_,    StrToPath(xrInstance_, "/user/hand/left/input/trigger/value") },
        { actTriggerValue_,    StrToPath(xrInstance_, "/user/hand/right/input/trigger/value") },

        // { actTriggerTouch_, StrToPath(xrInstance_, "/user/hand/left/input/trigger/touch") },
        // { actTriggerTouch_, StrToPath(xrInstance_, "/user/hand/right/input/trigger/touch") },

        // ===== Thumbstick（vec2 / x,y / click,touch）======
        { actThumbstick_,     StrToPath(xrInstance_, "/user/hand/left/input/thumbstick") },
        { actThumbstick_,     StrToPath(xrInstance_, "/user/hand/right/input/thumbstick") },
        { actThumbClick_,     StrToPath(xrInstance_, "/user/hand/left/input/thumbstick/click") },
        { actThumbClick_,     StrToPath(xrInstance_, "/user/hand/right/input/thumbstick/click") },
        // { actThumbTouch_,   StrToPath(xrInstance_, "/user/hand/left/input/thumbstick/touch") },
        // { actThumbTouch_,   StrToPath(xrInstance_, "/user/hand/right/input/thumbstick/touch") },
        // { actThumbX_,       StrToPath(xrInstance_, "/user/hand/left/input/thumbstick/x") },
        // { actThumbX_,       StrToPath(xrInstance_, "/user/hand/right/input/thumbstick/x") },
        // { actThumbY_,       StrToPath(xrInstance_, "/user/hand/left/input/thumbstick/y") },
        // { actThumbY_,       StrToPath(xrInstance_, "/user/hand/right/input/thumbstick/y") },

        // ===== Trackpad（vec2 / x,y / force / touch）======
        //{ actTrackpad_,       StrToPath(xrInstance_, "/user/hand/left/input/trackpad") },
        //{ actTrackpad_,       StrToPath(xrInstance_, "/user/hand/right/input/trackpad") },
        // { actTrackpadTouch_,StrToPath(xrInstance_, "/user/hand/left/input/trackpad/touch") },
        // { actTrackpadTouch_,StrToPath(xrInstance_, "/user/hand/right/input/trackpad/touch") },
        // { actTrackpadX_,    StrToPath(xrInstance_, "/user/hand/left/input/trackpad/x") },
        // { actTrackpadX_,    StrToPath(xrInstance_, "/user/hand/right/input/trackpad/x") },
        // { actTrackpadY_,    StrToPath(xrInstance_, "/user/hand/left/input/trackpad/y") },
        // { actTrackpadY_,    StrToPath(xrInstance_, "/user/hand/right/input/trackpad/y") },
        // { actTrackpadForce_,StrToPath(xrInstance_, "/user/hand/left/input/trackpad/force") },
        // { actTrackpadForce_,StrToPath(xrInstance_, "/user/hand/right/input/trackpad/force") },

        // ===== Poses ======
        { actGripPose_,       StrToPath(xrInstance_, "/user/hand/left/input/grip/pose") },
        { actGripPose_,       StrToPath(xrInstance_, "/user/hand/right/input/grip/pose") },
        { actAimPose_,        StrToPath(xrInstance_, "/user/hand/left/input/aim/pose") },
        { actAimPose_,        StrToPath(xrInstance_, "/user/hand/right/input/aim/pose") },

        // ===== Haptics ======
        { actHaptic_,         StrToPath(xrInstance_, "/user/hand/left/output/haptic") },
        { actHaptic_,         StrToPath(xrInstance_, "/user/hand/right/output/haptic") },
        });

    /*
    // valve/index_controller
    SuggestBindings("/interaction_profiles/valve/index_controller", {
        { actSqueezeValue_,      StrToPath(xrInstance_, "/user/hand/left/input/squeeze/value") },
        { actSqueezeValue_,      StrToPath(xrInstance_, "/user/hand/right/input/squeeze/value") },
        { actThumbstick_,   StrToPath(xrInstance_, "/user/hand/left/input/thumbstick") },
        { actThumbstick_,   StrToPath(xrInstance_, "/user/hand/right/input/thumbstick") },
        { actSelectValue_, StrToPath(xrInstance_, "/user/hand/left/input/trigger/value") },
        { actSelectValue_, StrToPath(xrInstance_, "/user/hand/right/input/trigger/value") },
        { actMenu_,         StrToPath(xrInstance_, "/user/hand/left/input/system/click") },
        { actAimPose_,      StrToPath(xrInstance_, "/user/hand/left/input/aim/pose") },
        { actAimPose_,      StrToPath(xrInstance_, "/user/hand/right/input/aim/pose") },
        { actGripPose_,     StrToPath(xrInstance_, "/user/hand/left/input/grip/pose") },
        { actGripPose_,     StrToPath(xrInstance_, "/user/hand/right/input/grip/pose") },
        { actHaptic_,       StrToPath(xrInstance_, "/user/hand/left/output/haptic") },
        { actHaptic_,       StrToPath(xrInstance_, "/user/hand/right/output/haptic") },

        { actA_,            StrToPath(xrInstance_, "/user/hand/left/input/a/click") },
        { actA_,            StrToPath(xrInstance_, "/user/hand/right/input/a/click") },
        { actB_,            StrToPath(xrInstance_, "/user/hand/left/input/b/click") },
        { actB_,            StrToPath(xrInstance_, "/user/hand/right/input/b/click") },
        });
        */



    // HTC Vive Controller（/interaction_profiles/htc/vive_controller）
    SuggestBindings("/interaction_profiles/htc/vive_controller", {
        // ====== Buttons（BOOLEAN）======
        // System（アプリには無効化されることも多い）
        // { actSystem_,   StrToPath(xrInstance_, "/user/hand/left/input/system/click") },
        // { actSystem_,   StrToPath(xrInstance_, "/user/hand/right/input/system/click") },

        { actSqueezeClick_, StrToPath(xrInstance_, "/user/hand/left/input/squeeze/click") },
        { actSqueezeClick_, StrToPath(xrInstance_, "/user/hand/right/input/squeeze/click") },

        { actMenu_,         StrToPath(xrInstance_, "/user/hand/left/input/menu/click") },
        { actMenu_,         StrToPath(xrInstance_, "/user/hand/right/input/menu/click") },

        { actTriggerClick_, StrToPath(xrInstance_, "/user/hand/left/input/trigger/click") },
        { actTriggerClick_, StrToPath(xrInstance_, "/user/hand/right/input/trigger/click") },

        // ====== Trigger Value（FLOAT）======
        { actTriggerValue_, StrToPath(xrInstance_, "/user/hand/left/input/trigger/value") },
        { actTriggerValue_, StrToPath(xrInstance_, "/user/hand/right/input/trigger/value") },

        // ====== Trackpad（VECTOR2F + サブ要素 + Click/Touch）======
        //{ actTrackpad_,     StrToPath(xrInstance_, "/user/hand/left/input/trackpad") },
        //{ actTrackpad_,     StrToPath(xrInstance_, "/user/hand/right/input/trackpad") },

        //{ actTrackpadClick_,StrToPath(xrInstance_, "/user/hand/left/input/trackpad/click") },
        //{ actTrackpadClick_,StrToPath(xrInstance_, "/user/hand/right/input/trackpad/click") },

        //{ actTrackpadTouch_,StrToPath(xrInstance_, "/user/hand/left/input/trackpad/touch") },
        //{ actTrackpadTouch_,StrToPath(xrInstance_, "/user/hand/right/input/trackpad/touch") },

        // 個別X/Yを別アクションにしたい場合（任意）
        // { actTrackpadX_, StrToPath(xrInstance_, "/user/hand/left/input/trackpad/x") },
        // { actTrackpadX_, StrToPath(xrInstance_, "/user/hand/right/input/trackpad/x") },
        // { actTrackpadY_, StrToPath(xrInstance_, "/user/hand/left/input/trackpad/y") },
        // { actTrackpadY_, StrToPath(xrInstance_, "/user/hand/right/input/trackpad/y") },

        // ====== Poses（POSE）======
        { actGripPose_,     StrToPath(xrInstance_, "/user/hand/left/input/grip/pose") },
        { actGripPose_,     StrToPath(xrInstance_, "/user/hand/right/input/grip/pose") },

        { actAimPose_,      StrToPath(xrInstance_, "/user/hand/left/input/aim/pose") },
        { actAimPose_,      StrToPath(xrInstance_, "/user/hand/right/input/aim/pose") },

        // ====== Haptics（VIBRATION_OUTPUT）======
        { actHaptic_,       StrToPath(xrInstance_, "/user/hand/left/output/haptic") },
        { actHaptic_,       StrToPath(xrInstance_, "/user/hand/right/output/haptic") },
        });


    //HTC Vive controller
    /*
    SuggestBindings("/interaction_profiles/htc/vive_controller", {
    { actThumbstick_, StrToPath(xrInstance_, "/user/hand/left/input/trackpad") },
    { actThumbstick_, StrToPath(xrInstance_, "/user/hand/right/input/trackpad") },
    { actThumbClick_, StrToPath(xrInstance_, "/user/hand/left/input/trackpad/click") },
    { actThumbClick_, StrToPath(xrInstance_, "/user/hand/right/input/trackpad/click") },
    { actSelectValue_,   StrToPath(xrInstance_, "/user/hand/left/input/trigger/value") },
    { actSelectValue_,   StrToPath(xrInstance_, "/user/hand/right/input/trigger/value") },
    { actSqueezeClick_, StrToPath(xrInstance_, "/user/hand/left/input/squeeze/click") },
    { actSqueezeClick_, StrToPath(xrInstance_, "/user/hand/right/input/squeeze/click") },
    { actMenu_,       StrToPath(xrInstance_, "/user/hand/left/input/menu/click") },
    { actMenu_,       StrToPath(xrInstance_, "/user/hand/right/input/menu/click") },
    { actAimPose_,    StrToPath(xrInstance_, "/user/hand/left/input/aim/pose") },
    { actAimPose_,    StrToPath(xrInstance_, "/user/hand/right/input/aim/pose") },
    { actGripPose_,   StrToPath(xrInstance_, "/user/hand/left/input/grip/pose") },
    { actGripPose_,   StrToPath(xrInstance_, "/user/hand/right/input/grip/pose") },
    { actHaptic_,     StrToPath(xrInstance_, "/user/hand/left/output/haptic") },
    { actHaptic_,     StrToPath(xrInstance_, "/user/hand/right/output/haptic") },
        });
        */




        // Microsoft Mixed Reality Motion Controller
    // (/interaction_profiles/microsoft/motion_controller)
    SuggestBindings("/interaction_profiles/microsoft/motion_controller", {
        // ===== Buttons / Click =====
        { actMenu_,           StrToPath(xrInstance_, "/user/hand/left/input/menu/click") },
        { actMenu_,           StrToPath(xrInstance_, "/user/hand/right/input/menu/click") },

        { actSqueezeClick_,   StrToPath(xrInstance_, "/user/hand/left/input/squeeze/click") },
        { actSqueezeClick_,   StrToPath(xrInstance_, "/user/hand/right/input/squeeze/click") },

        // ===== Trigger (FLOAT value) =====
        // あなたの設計に合わせて、トリガー値は actSelectValue_ に集約
        { actTriggerValue_,    StrToPath(xrInstance_, "/user/hand/left/input/trigger/value") },
        { actTriggerValue_,    StrToPath(xrInstance_, "/user/hand/right/input/trigger/value") },

        // ===== Thumbstick (VECTOR2F + click) =====
        { actThumbstick_,     StrToPath(xrInstance_, "/user/hand/left/input/thumbstick") },
        { actThumbstick_,     StrToPath(xrInstance_, "/user/hand/right/input/thumbstick") },
        { actThumbClick_,     StrToPath(xrInstance_, "/user/hand/left/input/thumbstick/click") },
        { actThumbClick_,     StrToPath(xrInstance_, "/user/hand/right/input/thumbstick/click") },

        // 任意: X/Y を個別 FLOAT アクションで取りたい場合
        // { actThumbX_,      StrToPath(xrInstance_, "/user/hand/left/input/thumbstick/x") },
        // { actThumbX_,      StrToPath(xrInstance_, "/user/hand/right/input/thumbstick/x") },
        // { actThumbY_,      StrToPath(xrInstance_, "/user/hand/left/input/thumbstick/y") },
        // { actThumbY_,      StrToPath(xrInstance_, "/user/hand/right/input/thumbstick/y") },

        // ===== Trackpad (VECTOR2F + click/touch) =====
        //{ actTrackpad_,       StrToPath(xrInstance_, "/user/hand/left/input/trackpad") },
        //{ actTrackpad_,       StrToPath(xrInstance_, "/user/hand/right/input/trackpad") },
        //{ actTrackpadClick_,  StrToPath(xrInstance_, "/user/hand/left/input/trackpad/click") },
        //{ actTrackpadClick_,  StrToPath(xrInstance_, "/user/hand/right/input/trackpad/click") },
        // 任意: touch / X / Y
        // { actTrackpadTouch_,StrToPath(xrInstance_, "/user/hand/left/input/trackpad/touch") },
        // { actTrackpadTouch_,StrToPath(xrInstance_, "/user/hand/right/input/trackpad/touch") },
        // { actTrackpadX_,   StrToPath(xrInstance_, "/user/hand/left/input/trackpad/x") },
        // { actTrackpadX_,   StrToPath(xrInstance_, "/user/hand/right/input/trackpad/x") },
        // { actTrackpadY_,   StrToPath(xrInstance_, "/user/hand/left/input/trackpad/y") },
        // { actTrackpadY_,   StrToPath(xrInstance_, "/user/hand/right/input/trackpad/y") },

        // ===== Poses =====
        { actGripPose_,       StrToPath(xrInstance_, "/user/hand/left/input/grip/pose") },
        { actGripPose_,       StrToPath(xrInstance_, "/user/hand/right/input/grip/pose") },
        { actAimPose_,        StrToPath(xrInstance_, "/user/hand/left/input/aim/pose") },
        { actAimPose_,        StrToPath(xrInstance_, "/user/hand/right/input/aim/pose") },

        // ===== Haptics =====
        { actHaptic_,         StrToPath(xrInstance_, "/user/hand/left/output/haptic") },
        { actHaptic_,         StrToPath(xrInstance_, "/user/hand/right/output/haptic") },
        });


    /*
    // microsoft/motion_controller (WMR)
    SuggestBindings("/interaction_profiles/microsoft/motion_controller", {
        { actSelectValue_,   StrToPath(xrInstance_, "/user/hand/left/input/trigger/value") },
        { actSelectValue_,   StrToPath(xrInstance_, "/user/hand/right/input/trigger/value") },
        { actSqueezeClick_, StrToPath(xrInstance_, "/user/hand/left/input/squeeze/click") },
        { actSqueezeClick_, StrToPath(xrInstance_, "/user/hand/right/input/squeeze/click") },
        { actThumbstick_,   StrToPath(xrInstance_, "/user/hand/left/input/thumbstick") },
        { actThumbstick_,   StrToPath(xrInstance_, "/user/hand/right/input/thumbstick") },
        { actMenu_,         StrToPath(xrInstance_, "/user/hand/left/input/menu/click") },
        { actAimPose_,      StrToPath(xrInstance_, "/user/hand/left/input/aim/pose") },
        { actAimPose_,      StrToPath(xrInstance_, "/user/hand/right/input/aim/pose") },
        { actGripPose_,     StrToPath(xrInstance_, "/user/hand/left/input/grip/pose") },
        { actGripPose_,     StrToPath(xrInstance_, "/user/hand/right/input/grip/pose") },
        { actHaptic_,       StrToPath(xrInstance_, "/user/hand/left/output/haptic") },
        { actHaptic_,       StrToPath(xrInstance_, "/user/hand/right/output/haptic") },
        });
        */




    return true;
}


bool OpenXRController::MakeAction(XrActionType type, const char* name, const char* loc, XrAction& out) {

    //両手へのパス
    XrPath pathHands[2] = { pathLeft, pathRight };

    // アクション作成用の構造体を準備
    XrActionCreateInfo ai{ XR_TYPE_ACTION_CREATE_INFO };
    ai.actionType = type;

    // アクションの識別名と表示名を設定
    strncpy_s(ai.actionName, XR_MAX_ACTION_NAME_SIZE, name, _TRUNCATE);
    strncpy_s(ai.localizedActionName, XR_MAX_LOCALIZED_ACTION_NAME_SIZE, loc, _TRUNCATE);

    // このアクションが左右の手のどちらでも使えるようにする
    ai.countSubactionPaths = 2;
    ai.subactionPaths = pathHands;

    // アクションを作成
    XrResult r = xrCreateAction(actionSet_, &ai, &out);

    // 成否をデバッグ出力
    char buf[256];
    sprintf_s(buf, "[XR] makeAction %-16s -> %s (0x%08X)\n",
        name, XR_SUCCEEDED(r) ? "OK" : "FAILED", r);
    OutputDebugStringA(buf);

    return XR_SUCCEEDED(r);
}



bool OpenXRController::SuggestBindings(const char* profile, std::initializer_list<XrActionSuggestedBinding> bindings) {


    // 提案情報を入れる構造体を準備
    XrInteractionProfileSuggestedBinding sb{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };

    // この提案が有効になる「コントローラープロファイル」のパスを文字列から変換
    sb.interactionProfile = StrToPath(xrInstance_, profile);

    // 提案するバインディング一覧を設定
    sb.countSuggestedBindings = static_cast<uint32_t>(bindings.size());
    sb.suggestedBindings = const_cast<XrActionSuggestedBinding*>(bindings.begin());

    // OpenXR ランタイムに「このプロファイルではこういうバインディングにして」と提案
    XrResult r = xrSuggestInteractionProfileBindings(xrInstance_, &sb);

    // 成功／失敗をデバッグ出力
    char buf[256];
    sprintf_s(buf, "[XR] suggest %-40s -> %s (0x%08X)\n",
        profile, XR_SUCCEEDED(r) ? "OK" : "FAILED", r);
    OutputDebugStringA(buf);

    // 成功なら true, 失敗なら false を返す
    return XR_SUCCEEDED(r);
}


bool OpenXRController::CreateSpaces() {

    auto make = [&](XrAction act, XrPath sub, XrSpace& out) {

        XrActionSpaceCreateInfo si{ XR_TYPE_ACTION_SPACE_CREATE_INFO };
        si.action = act;
        si.subactionPath = sub;
        si.poseInActionSpace.orientation.w = 1.0f; // identity
        return XR_SUCCEEDED(xrCreateActionSpace(xrSession_, &si, &out));
        };

    if (!make(actAimPose_, pathLeft, spaceAimL)) return false;
    if (!make(actAimPose_, pathRight, apaceAimR)) return false;
    if (!make(actGripPose_, pathLeft, spaceGripL)) return false;
    if (!make(actGripPose_, pathRight, spaceGripR)) return false;
    return true;
}


/*
bool OpenXRController::CreateActionsAndBindings() {
    // ActionSet
    {
        XrActionSetCreateInfo ci{ XR_TYPE_ACTION_SET_CREATE_INFO };
        //std::memset(&ci, 0, sizeof(ci));
        //std::strncpy(ci.actionSetName, "gameplay", XR_MAX_ACTION_SET_NAME_SIZE - 1);
        //std::strncpy(ci.localizedActionSetName, "Gameplay", XR_MAX_LOCALIZED_ACTION_SET_NAME_SIZE - 1);
        strncpy_s(ci.actionSetName, XR_MAX_ACTION_SET_NAME_SIZE, "gameplay", _TRUNCATE);
        strncpy_s(ci.localizedActionSetName, XR_MAX_LOCALIZED_ACTION_SET_NAME_SIZE, "Gameplay", _TRUNCATE);
        ci.priority = 0;
        if (!XR_SUCCEEDED(xrCreateActionSet(xrInstance_, &ci, &actionSet_))) return false;
    }

    // subaction hands
    subLeft_ = StrToPath(xrInstance_, "/user/hand/left");
    subRight_ = StrToPath(xrInstance_, "/user/hand/right");
    XrPath hands[2] = { subLeft_, subRight_ };

    auto makeAction = [&](XrActionType type, const char* name, const char* loc, XrAction& out) {
        XrActionCreateInfo ai{ XR_TYPE_ACTION_CREATE_INFO };
        ai.actionType = type;
        //std::strncpy(ai.actionName, name, XR_MAX_ACTION_NAME_SIZE - 1);
        //std::strncpy(ai.localizedActionName, loc, XR_MAX_LOCALIZED_ACTION_NAME_SIZE - 1);
        strncpy_s(ai.actionName, XR_MAX_ACTION_NAME_SIZE, name, _TRUNCATE);
        strncpy_s(ai.localizedActionName, XR_MAX_LOCALIZED_ACTION_NAME_SIZE, loc, _TRUNCATE);
        ai.countSubactionPaths = 2;
        ai.subactionPaths = hands;
        return XR_SUCCEEDED(xrCreateAction(actionSet_, &ai, &out));
        };

    // 基本アクション
    //if (!makeAction(XR_ACTION_TYPE_BOOLEAN_INPUT, "select_click", "Select", actSelect_))        return false;
    if (!makeAction(XR_ACTION_TYPE_BOOLEAN_INPUT, "select_click", "SelectClick", actSelectClick_)) return false; // ← 旧 actSelect_
    if (!makeAction(XR_ACTION_TYPE_FLOAT_INPUT, "select_value", "Select", actSelectValue_)) return false; // ← 追加
    if (!makeAction(XR_ACTION_TYPE_BOOLEAN_INPUT, "squeeze_click", "SqueezeClick", actSqueezeClick_))  return false;
    if (!makeAction(XR_ACTION_TYPE_FLOAT_INPUT, "squeeze", "Squeeze", actSqueeze_))       return false;
    if (!makeAction(XR_ACTION_TYPE_VECTOR2F_INPUT, "thumbstick", "Thumbstick", actThumbstick_))    return false;
    if (!makeAction(XR_ACTION_TYPE_BOOLEAN_INPUT, "thumb_click", "ThumbstickClick", actThumbClick_))    return false;
    if (!makeAction(XR_ACTION_TYPE_BOOLEAN_INPUT, "menu", "Menu", actMenu_))          return false;
    if (!makeAction(XR_ACTION_TYPE_POSE_INPUT, "aim_pose", "Aim Pose", actAimPose_))       return false;
    if (!makeAction(XR_ACTION_TYPE_POSE_INPUT, "grip_pose", "Grip Pose", actGripPose_))      return false;
    if (!makeAction(XR_ACTION_TYPE_VIBRATION_OUTPUT, "haptic", "Haptic", actHaptic_))        return false;

    // [ADDED] 文字ボタン A/B/X/Y のアクション作成
    if (!makeAction(XR_ACTION_TYPE_BOOLEAN_INPUT, "button_a", "Button A", actA_))             return false;
    if (!makeAction(XR_ACTION_TYPE_BOOLEAN_INPUT, "button_b", "Button B", actB_))             return false;
    if (!makeAction(XR_ACTION_TYPE_BOOLEAN_INPUT, "button_x", "Button X", actX_))             return false;
    if (!makeAction(XR_ACTION_TYPE_BOOLEAN_INPUT, "button_y", "Button Y", actY_))             return false;

    auto suggest = [&](const char* profile, std::initializer_list<XrActionSuggestedBinding> b) {
        XrInteractionProfileSuggestedBinding sb{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
        sb.interactionProfile = StrToPath(xrInstance_, profile);
        sb.countSuggestedBindings = (uint32_t)b.size();
        sb.suggestedBindings = const_cast<XrActionSuggestedBinding*>(b.begin());
        return XR_SUCCEEDED(xrSuggestInteractionProfileBindings(xrInstance_, &sb));
        };

    // khr/simple_controller
    suggest("/interaction_profiles/khr/simple_controller", {
        { actSelectClick_, StrToPath(xrInstance_, "/user/hand/left/input/select/click") },
        { actSelectClick_, StrToPath(xrInstance_, "/user/hand/right/input/select/click") },
        { actAimPose_, StrToPath(xrInstance_, "/user/hand/left/input/aim/pose") },
        { actAimPose_, StrToPath(xrInstance_, "/user/hand/right/input/aim/pose") },
        { actHaptic_,  StrToPath(xrInstance_, "/user/hand/left/output/haptic") },
        { actHaptic_,  StrToPath(xrInstance_, "/user/hand/right/output/haptic") },
        });

    // oculus/touch_controller
    suggest("/interaction_profiles/oculus/touch_controller", {
        { actSelectValue_, StrToPath(xrInstance_, "/user/hand/left/input/trigger/value") },
        { actSelectValue_, StrToPath(xrInstance_, "/user/hand/right/input/trigger/value") },
        { actSqueeze_,      StrToPath(xrInstance_, "/user/hand/left/input/squeeze/value") },
        { actSqueeze_,      StrToPath(xrInstance_, "/user/hand/right/input/squeeze/value") },
        { actThumbstick_,   StrToPath(xrInstance_, "/user/hand/left/input/thumbstick") },
        { actThumbstick_,   StrToPath(xrInstance_, "/user/hand/right/input/thumbstick") },
        { actThumbClick_,   StrToPath(xrInstance_, "/user/hand/left/input/thumbstick/click") },
        { actThumbClick_,   StrToPath(xrInstance_, "/user/hand/right/input/thumbstick/click") },
        { actMenu_,         StrToPath(xrInstance_, "/user/hand/left/input/menu/click") },
        { actAimPose_,      StrToPath(xrInstance_, "/user/hand/left/input/aim/pose") },
        { actAimPose_,      StrToPath(xrInstance_, "/user/hand/right/input/aim/pose") },
        { actGripPose_,     StrToPath(xrInstance_, "/user/hand/left/input/grip/pose") },
        { actGripPose_,     StrToPath(xrInstance_, "/user/hand/right/input/grip/pose") },
        { actHaptic_,       StrToPath(xrInstance_, "/user/hand/left/output/haptic") },
        { actHaptic_,       StrToPath(xrInstance_, "/user/hand/right/output/haptic") },

        { actA_,            StrToPath(xrInstance_, "/user/hand/right/input/a/click") },
        { actB_,            StrToPath(xrInstance_, "/user/hand/right/input/b/click") },
        { actX_,            StrToPath(xrInstance_, "/user/hand/left/input/x/click") },
        { actY_,            StrToPath(xrInstance_, "/user/hand/left/input/y/click") },
        });

    // valve/index_controller
    suggest("/interaction_profiles/valve/index_controller", {
        { actSqueeze_,      StrToPath(xrInstance_, "/user/hand/left/input/squeeze/value") },
        { actSqueeze_,      StrToPath(xrInstance_, "/user/hand/right/input/squeeze/value") },
        { actThumbstick_,   StrToPath(xrInstance_, "/user/hand/left/input/thumbstick") },
        { actThumbstick_,   StrToPath(xrInstance_, "/user/hand/right/input/thumbstick") },
        { actSelectValue_, StrToPath(xrInstance_, "/user/hand/left/input/trigger/value") },
        { actSelectValue_, StrToPath(xrInstance_, "/user/hand/right/input/trigger/value") },
        { actMenu_,         StrToPath(xrInstance_, "/user/hand/left/input/system/click") },
        { actAimPose_,      StrToPath(xrInstance_, "/user/hand/left/input/aim/pose") },
        { actAimPose_,      StrToPath(xrInstance_, "/user/hand/right/input/aim/pose") },
        { actGripPose_,     StrToPath(xrInstance_, "/user/hand/left/input/grip/pose") },
        { actGripPose_,     StrToPath(xrInstance_, "/user/hand/right/input/grip/pose") },
        { actHaptic_,       StrToPath(xrInstance_, "/user/hand/left/output/haptic") },
        { actHaptic_,       StrToPath(xrInstance_, "/user/hand/right/output/haptic") },

        { actA_,            StrToPath(xrInstance_, "/user/hand/left/input/a/click") },
        { actA_,            StrToPath(xrInstance_, "/user/hand/right/input/a/click") },
        { actB_,            StrToPath(xrInstance_, "/user/hand/left/input/b/click") },
        { actB_,            StrToPath(xrInstance_, "/user/hand/right/input/b/click") },
        });

    //HTC Vive controller

    suggest("/interaction_profiles/htc/vive_controller", {
    { actThumbstick_, StrToPath(xrInstance_, "/user/hand/left/input/trackpad") },
    { actThumbstick_, StrToPath(xrInstance_, "/user/hand/right/input/trackpad") },
    { actThumbClick_, StrToPath(xrInstance_, "/user/hand/left/input/trackpad/click") },
    { actThumbClick_, StrToPath(xrInstance_, "/user/hand/right/input/trackpad/click") },
    { actSelectValue_,   StrToPath(xrInstance_, "/user/hand/left/input/trigger/value") },
    { actSelectValue_,   StrToPath(xrInstance_, "/user/hand/right/input/trigger/value") },
    { actSqueezeClick_, StrToPath(xrInstance_, "/user/hand/left/input/squeeze/click") },
    { actSqueezeClick_, StrToPath(xrInstance_, "/user/hand/right/input/squeeze/click") },
    { actMenu_,       StrToPath(xrInstance_, "/user/hand/left/input/menu/click") },
    { actMenu_,       StrToPath(xrInstance_, "/user/hand/right/input/menu/click") },
    { actAimPose_,    StrToPath(xrInstance_, "/user/hand/left/input/aim/pose") },
    { actAimPose_,    StrToPath(xrInstance_, "/user/hand/right/input/aim/pose") },
    { actGripPose_,   StrToPath(xrInstance_, "/user/hand/left/input/grip/pose") },
    { actGripPose_,   StrToPath(xrInstance_, "/user/hand/right/input/grip/pose") },
    { actHaptic_,     StrToPath(xrInstance_, "/user/hand/left/output/haptic") },
    { actHaptic_,     StrToPath(xrInstance_, "/user/hand/right/output/haptic") },
        });


    // microsoft/motion_controller (WMR)
    suggest("/interaction_profiles/microsoft/motion_controller", {
        { actSelectValue_,   StrToPath(xrInstance_, "/user/hand/left/input/trigger/value") },
        { actSelectValue_,   StrToPath(xrInstance_, "/user/hand/right/input/trigger/value") },
        { actSqueezeClick_, StrToPath(xrInstance_, "/user/hand/left/input/squeeze/click") },
        { actSqueezeClick_, StrToPath(xrInstance_, "/user/hand/right/input/squeeze/click") },
        { actThumbstick_,   StrToPath(xrInstance_, "/user/hand/left/input/thumbstick") },
        { actThumbstick_,   StrToPath(xrInstance_, "/user/hand/right/input/thumbstick") },
        { actMenu_,         StrToPath(xrInstance_, "/user/hand/left/input/menu/click") },
        { actAimPose_,      StrToPath(xrInstance_, "/user/hand/left/input/aim/pose") },
        { actAimPose_,      StrToPath(xrInstance_, "/user/hand/right/input/aim/pose") },
        { actGripPose_,     StrToPath(xrInstance_, "/user/hand/left/input/grip/pose") },
        { actGripPose_,     StrToPath(xrInstance_, "/user/hand/right/input/grip/pose") },
        { actHaptic_,       StrToPath(xrInstance_, "/user/hand/left/output/haptic") },
        { actHaptic_,       StrToPath(xrInstance_, "/user/hand/right/output/haptic") },
        });

    return true;
}
*/





bool OpenXRController::AttachActionSet(XrSession session) {

    XrSessionActionSetsAttachInfo attach{ XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
    attach.countActionSets = 1;
    attach.actionSets = &actionSet_;

    XrResult r = xrAttachSessionActionSets(session, &attach);
    char buf[128];
    sprintf_s(buf, "[XR] xrAttachSessionActionSets -> %s (0x%08X)\n", (XR_SUCCEEDED(r) ? "OK" : "FAILED"), r);
    OutputDebugStringA(buf);

    return XR_SUCCEEDED(r);
}

// ===== 毎フレーム同期／取得 =====
bool OpenXRController::Sync(XrSession session, XrTime predictedDisplayTime) {

    XrActiveActionSet active{};
    active.actionSet = actionSet_;
    XrActionsSyncInfo sync{ XR_TYPE_ACTIONS_SYNC_INFO };
    sync.countActiveActionSets = 1;
    sync.activeActionSets = &active;

    //if (!XR_SUCCEEDED(xrSyncActions(xrSession_, &sync))) return false;

    xrSession_ = session;

    XrResult sr = xrSyncActions(session, &sync);

    if (!XR_SUCCEEDED(sr)) {
        char buf[128];
        sprintf_s(buf, "[XR][ERROR] xrSyncActions failed (0x%08X)\n", sr);
        OutputDebugStringA(buf);
        return false;
    }

    // デバッグ表示。プロファイル確認ログ（変化があったときだけ）
    /*
    {
        std::string L = GetCurrentInteractionProfilePath(true);
        std::string R = GetCurrentInteractionProfilePath(false);
        char buf[256];
        sprintf_s(buf, "[XR] Profile L=%s, R=%s\n",
            L.empty() ? "(none)" : L.c_str(),
            R.empty() ? "(none)" : R.c_str());
        OutputDebugStringA(buf);
    }
    */

    // 今フレームの更新前に「前フレーム」を保存
    if (prevValid_) {
        prevLeft_ = stateLeft;
        prevRight_ = stateRight;
    }

    stateLeft = State{};
    stateRight = State{};


    // isActive 代わりに squeeze の有効性を採用（少なくとも片方がアクティブかどうかの目安）
    stateLeft.isActive = ReadFloat(actSqueezeValue_, pathLeft, stateLeft.squeezeValue);
    stateRight.isActive = ReadFloat(actSqueezeValue_, pathRight, stateRight.squeezeValue);

    ReadFloat(actTriggerValue_, pathLeft, stateLeft.selectValue);
    ReadFloat(actTriggerValue_, pathRight, stateRight.selectValue);
    // クリック（boolean）が取れる環境ではそれを採用
    ReadBool(actTriggerClick_, pathLeft, stateLeft.selectClick);
    ReadBool(actTriggerClick_, pathRight, stateRight.selectClick);
    // クリックが来ないランタイム向けに value から補完（しきい値は適宜調整）
    if (!stateLeft.selectClick && stateLeft.selectValue > 0.55f) stateLeft.selectClick = true;
    if (!stateRight.selectClick && stateRight.selectValue > 0.55f) stateRight.selectClick = true;

    ReadBool(actSqueezeClick_, pathLeft, stateLeft.squeezeClick);
    ReadBool(actSqueezeClick_, pathRight, stateRight.squeezeClick);
    ReadVec2(actThumbstick_, pathLeft, stateLeft.thumbstick);
    ReadVec2(actThumbstick_, pathRight, stateRight.thumbstick);
    ReadBool(actThumbClick_, pathLeft, stateLeft.thumbstickClick);
    ReadBool(actThumbClick_, pathRight, stateRight.thumbstickClick);
    ReadBool(actMenu_, pathLeft, stateLeft.menu);
    ReadBool(actMenu_, pathRight, stateRight.menu);

    // 文字ボタン A/B/X/Y の取得
    ReadBool(actA_, pathLeft, stateLeft.buttonA);
    ReadBool(actA_, pathRight, stateRight.buttonA);
    ReadBool(actB_, pathLeft, stateLeft.buttonB);
    ReadBool(actB_, pathRight, stateRight.buttonB);
    ReadBool(actX_, pathLeft, stateLeft.buttonX);
    ReadBool(actX_, pathRight, stateRight.buttonX);
    ReadBool(actY_, pathLeft, stateLeft.buttonY);
    ReadBool(actY_, pathRight, stateRight.buttonY);

    Locate(spaceAimL, stateLeft.aimPose, stateLeft.hasAimPose, predictedDisplayTime);
    Locate(apaceAimR, stateRight.aimPose, stateRight.hasAimPose, predictedDisplayTime);
    Locate(spaceGripL, stateLeft.gripPose, stateLeft.hasGripPose, predictedDisplayTime);
    Locate(spaceGripR, stateRight.gripPose, stateRight.hasGripPose, predictedDisplayTime);

    //デバッグ表示
    /*
    {
        char buf[192];
        sprintf_s(buf, "[XR] Pose L aim:%d grip:%d | R aim:%d grip:%d\n",
            (int)left_.hasAimPose, (int)left_.hasGripPose,
            (int)right_.hasAimPose, (int)right_.hasGripPose);
        OutputDebugStringA(buf);
    }
    {
        char buf[256];
        sprintf_s(buf,
            "[XR] L sel(%.2f/%d) sq(%.2f/%d) stick(%.2f,%.2f/%d) M:%d A:%d B:%d X:%d Y:%d  |  "
            "R sel(%.2f/%d) sq(%.2f/%d) stick(%.2f,%.2f/%d) M:%d A:%d B:%d X:%d Y:%d\n",
            left_.selectValue, (int)left_.selectClick,
            left_.squeezeValue, (int)left_.squeezeClick,
            left_.thumbstick.x, left_.thumbstick.y, (int)left_.thumbstickClick,
            (int)left_.menu, (int)left_.buttonA, (int)left_.buttonB, (int)left_.buttonX, (int)left_.buttonY,
            right_.selectValue, (int)right_.selectClick,
            right_.squeezeValue, (int)right_.squeezeClick,
            right_.thumbstick.x, right_.thumbstick.y, (int)right_.thumbstickClick,
            (int)right_.menu, (int)right_.buttonA, (int)right_.buttonB, (int)right_.buttonX, (int)right_.buttonY
        );
        OutputDebugStringA(buf);
    }
    */


    // 初回だけ：以前の値も同じにする（OnPushに影響を与えない）
    if (!prevValid_) {
        prevLeft_ = stateLeft;
        prevRight_ = stateRight;
        prevValid_ = true;
    }


    return true;
}


XrPosef OpenXRController::GetPose_LeftController() const {

    XrPosef p{};
    p.orientation.w = 1.0f;  // 単位姿勢

    if (stateLeft.hasGripPose)      p = stateLeft.gripPose;
    else if (stateLeft.hasAimPose)  p = stateLeft.aimPose;

    // RH(OpenXR) → LH(DirectX) 変換
    // 位置：Z反転
    p.position.z = -p.position.z;

    // 回転：クォータニオンの x, y を反転（z, w はそのまま）
    p.orientation.x = -p.orientation.x;
    p.orientation.y = -p.orientation.y;
    // p.orientation.z =  p.orientation.z;
    // p.orientation.w =  p.orientation.w;

    return p;

}

XrPosef OpenXRController::GetPose_RightController() const {

    XrPosef p{};
    p.orientation.w = 1.0f;  // 単位姿勢

    if (stateRight.hasGripPose)      p = stateRight.gripPose;
    else if (stateRight.hasAimPose)  p = stateRight.aimPose;

    // RH(OpenXR) → LH(DirectX) 変換
    // 位置：Z反転
    p.position.z = -p.position.z;

    // 回転：クォータニオンの x, y を反転（z, w はそのまま）
    p.orientation.x = -p.orientation.x;
    p.orientation.y = -p.orientation.y;
    // p.orientation.z =  p.orientation.z;
    // p.orientation.w =  p.orientation.w;
    
    return p;

}

// 生のスティック傾き
XrVector2f OpenXRController::GetValue_Left_Stick() const { return stateLeft.thumbstick; }
XrVector2f OpenXRController::GetValue_Right_Stick() const { return stateRight.thumbstick; }

// デッドゾーン適用版（半径dz）
// 例: dz=0.15f。半径dz未満は(0,0)。それ以外は 0..1 に再マップ（放射状デッドゾーン）
XrVector2f OpenXRController::GetValue_Left_Stick(float dz) const {

    XrVector2f v = stateLeft.thumbstick;
    float d = (dz < 0.0f ? 0.0f : (dz > 0.99f ? 0.99f : dz));
    float m = std::sqrt(v.x * v.x + v.y * v.y);
    if (m < d) return XrVector2f{ 0.0f, 0.0f };
    float s = (m - d) / (1.0f - d);            // 0..1 に再マップ
    float invm = (m > 0.0f) ? (s / m) : 0.0f;

    return XrVector2f{ v.x * invm, v.y * invm };
}
XrVector2f OpenXRController::GetValue_Right_Stick(float dz) const {
    XrVector2f v = stateRight.thumbstick;
    float d = (dz < 0.0f ? 0.0f : (dz > 0.99f ? 0.99f : dz));
    float m = std::sqrt(v.x * v.x + v.y * v.y);
    if (m < d) return XrVector2f{ 0.0f, 0.0f };
    float s = (m - d) / (1.0f - d);
    float invm = (m > 0.0f) ? (s / m) : 0.0f;
    return XrVector2f{ v.x * invm, v.y * invm };
}


float OpenXRController::GetValue_Left_SelectTrigger()  const { return stateLeft.selectValue; }
float OpenXRController::GetValue_Right_SelectTrigger() const { return stateRight.selectValue; }

float OpenXRController::GetValue_Left_SqueezeTrigger()  const { return stateLeft.squeezeValue; }
float OpenXRController::GetValue_Right_SqueezeTrigger() const { return stateRight.squeezeValue; }


// OnPush（立ち上がり）判定：curr==true かつ prev==false
bool OpenXRController::OnPushSelectTrigger(bool leftHand) const {
    if (leftHand)  return (stateLeft.selectClick && !prevLeft_.selectClick);
    else           return (stateRight.selectClick && !prevRight_.selectClick);
}

bool OpenXRController::OnPushSqueezeTrigger(bool leftHand) const {
    if (leftHand)  return (stateLeft.squeezeClick && !prevLeft_.squeezeClick);
    else           return (stateRight.squeezeClick && !prevRight_.squeezeClick);
}

bool OpenXRController::OnPushA(bool leftHand) const {
    if (leftHand)  return (stateLeft.buttonA && !prevLeft_.buttonA);
    else           return (stateRight.buttonA && !prevRight_.buttonA);
}

bool OpenXRController::OnPushB(bool leftHand) const {
    if (leftHand)  return (stateLeft.buttonB && !prevLeft_.buttonB);
    else           return (stateRight.buttonB && !prevRight_.buttonB);
}

bool OpenXRController::OnPushX(bool leftHand) const {
    if (leftHand)  return (stateLeft.buttonX && !prevLeft_.buttonX);
    else           return (stateRight.buttonX && !prevRight_.buttonX);
}

bool OpenXRController::OnPushY(bool leftHand) const {
    if (leftHand)  return (stateLeft.buttonY && !prevLeft_.buttonY);
    else           return (stateRight.buttonY && !prevRight_.buttonY);
}

bool OpenXRController::OnPushMenu(bool leftHand) const {
    if (leftHand)  return (stateLeft.menu && !prevLeft_.menu);
    else           return (stateRight.menu && !prevRight_.menu);
}

bool OpenXRController::OnPushStick(bool leftHand) const {
    if (leftHand)  return (stateLeft.thumbstickClick && !prevLeft_.thumbstickClick);
    else           return (stateRight.thumbstickClick && !prevRight_.thumbstickClick);
}


// ==================== OnPush 左右ラッパー ====================
// Select
bool OpenXRController::OnPush_Left_SelectTrigger() const { return OnPushSelectTrigger(/*leftHand=*/true); }
bool OpenXRController::OnPush_Right_SelectTrigger() const { return OnPushSelectTrigger(/*leftHand=*/false); }

// SqueezeClick
bool OpenXRController::OnPush_Left_SqueezeTrigger() const { return OnPushSqueezeTrigger(/*leftHand=*/true); }
bool OpenXRController::OnPush_Right_SqueezeTrigger() const { return OnPushSqueezeTrigger(/*leftHand=*/false); }

// A
bool OpenXRController::OnPush_Left_A() const { return OnPushA(/*leftHand=*/true); }
bool OpenXRController::OnPush_Right_A() const { return OnPushA(/*leftHand=*/false); }

// B
bool OpenXRController::OnPush_Left_B() const { return OnPushB(/*leftHand=*/true); }
bool OpenXRController::OnPush_Right_B() const { return OnPushB(/*leftHand=*/false); }

// X
bool OpenXRController::OnPush_Left_X() const { return OnPushX(/*leftHand=*/true); }
bool OpenXRController::OnPush_Right_X() const { return OnPushX(/*leftHand=*/false); }

// Y
bool OpenXRController::OnPush_Left_Y() const { return OnPushY(/*leftHand=*/true); }
bool OpenXRController::OnPush_Right_Y() const { return OnPushY(/*leftHand=*/false); }

// Menu
bool OpenXRController::OnPush_Left_Menu() const { return OnPushMenu(/*leftHand=*/true); }
bool OpenXRController::OnPush_Right_Menu() const { return OnPushMenu(/*leftHand=*/false); }

// Stick押し込み
bool OpenXRController::OnPush_Left_Stick() const { return OnPushStick(/*leftHand=*/true); }
bool OpenXRController::OnPush_Right_Stick() const { return OnPushStick(/*leftHand=*/false); }


// ===== ハプティクス =====
bool OpenXRController::ApplyHaptics(bool leftHand, float amplitude, float seconds, float frequencyHz) {

    XrHapticVibration vib{ XR_TYPE_HAPTIC_VIBRATION };
    vib.amplitude = std::clamp(amplitude, 0.0f, 1.0f);
    vib.duration = (XrDuration)(seconds * 1e9); // ns
    vib.frequency = frequencyHz; // 0=ランタイム任意

    XrHapticActionInfo hi{ XR_TYPE_HAPTIC_ACTION_INFO };
    hi.action = actHaptic_;
    hi.subactionPath = leftHand ? pathLeft : pathRight;
    return XR_SUCCEEDED(xrApplyHapticFeedback(xrSession_, &hi, (XrHapticBaseHeader*)&vib));

}

// ===== ユーティリティ =====
bool OpenXRController::ReadBool(XrAction a, XrPath sub, bool& dst) const {


    XrActionStateBoolean st{ XR_TYPE_ACTION_STATE_BOOLEAN };

    XrActionStateGetInfo gi{ XR_TYPE_ACTION_STATE_GET_INFO };
    gi.action = a; gi.subactionPath = sub;
    
    if (XR_SUCCEEDED(xrGetActionStateBoolean(xrSession_, &gi, &st))) {
        dst = (st.isActive && st.currentState);

        // ログ出力
        /*
        char buf[256];
        sprintf_s(buf,
            "[XR] ReadBool path=%llu active=%d state=%d changed=%d\n",
            static_cast<unsigned long long>(sub),
            st.isActive,
            st.currentState,
            st.changedSinceLastSync
        );
        OutputDebugStringA(buf);
        */


        return st.isActive != 0;
    }
    return false;
}

bool OpenXRController::ReadFloat(XrAction a, XrPath sub, float& dst) const {

    XrActionStateFloat st{ XR_TYPE_ACTION_STATE_FLOAT };
    XrActionStateGetInfo gi{ XR_TYPE_ACTION_STATE_GET_INFO };
    gi.action = a; gi.subactionPath = sub;
    if (XR_SUCCEEDED(xrGetActionStateFloat(xrSession_, &gi, &st))) {
        dst = st.currentState;
        return st.isActive != 0;
    }
    
    return false;
}

bool OpenXRController::ReadVec2(XrAction a, XrPath sub, XrVector2f& dst) const {
    XrActionStateVector2f st{ XR_TYPE_ACTION_STATE_VECTOR2F };
    XrActionStateGetInfo gi{ XR_TYPE_ACTION_STATE_GET_INFO };
    gi.action = a; gi.subactionPath = sub;
    if (XR_SUCCEEDED(xrGetActionStateVector2f(xrSession_, &gi, &st))) {
        dst = st.currentState;
        
        /*
        // ここでログ出力
        char buf[256];
        sprintf_s(buf,
            "[XR] ReadVec2 path=%llu  active=%d  x=%.3f  y=%.3f\n",
            static_cast<unsigned long long>(sub),
            st.isActive,
            st.currentState.x,
            st.currentState.y
        );
        OutputDebugStringA(buf);
        */


        
        return st.isActive != 0;



    }
    return false;
}

bool OpenXRController::Locate(XrSpace space, XrPosef& dst, bool& has, XrTime t) const {

    if (space == XR_NULL_HANDLE) { has = false; return false; }

    XrSpaceLocation loc{ XR_TYPE_SPACE_LOCATION };
    xrLocateSpace(space, xrAppSpace_, t, &loc);
    
    const XrSpaceLocationFlags req =
        XR_SPACE_LOCATION_ORIENTATION_VALID_BIT | XR_SPACE_LOCATION_POSITION_VALID_BIT;
    
    has = ((loc.locationFlags & req) == req);
    if (has) dst = loc.pose;

    return has;
}

// OpenXR(RH)のPoseをDirectX(LH)行列に変換
// M_lh = FlipZ * M_rh * FlipZ （FlipZ=diag(1,1,-1,1)）
DirectX::XMMATRIX OpenXRController::PoseToLHMatrix(const XrPosef& pose) {

    using namespace DirectX;
    const XMVECTOR q = XMVectorSet(pose.orientation.x, pose.orientation.y, pose.orientation.z, pose.orientation.w);
    const XMVECTOR t = XMVectorSet(pose.position.x, pose.position.y, pose.position.z, 1.0f);

    XMMATRIX R = XMMatrixRotationQuaternion(q);
    XMMATRIX T = XMMatrixTranslationFromVector(t);
    XMMATRIX M_rh = R * T;

    static const XMMATRIX FlipZ = XMMatrixScaling(1.0f, 1.0f, -1.0f);
    XMMATRIX M_lh = FlipZ * M_rh * FlipZ;

    return M_lh;
}

std::string OpenXRController::GetCurrentInteractionProfilePath(bool leftHand) const {

    if (xrSession_ == XR_NULL_HANDLE) return {};
    XrPath top = StrToPath(xrInstance_, leftHand ? "/user/hand/left" : "/user/hand/right");
    XrInteractionProfileState st{ XR_TYPE_INTERACTION_PROFILE_STATE };
    if (!XR_SUCCEEDED(xrGetCurrentInteractionProfile(xrSession_, top, &st))) return {};
    if (st.interactionProfile == XR_NULL_PATH) return {};

    // 文字列化
    char buf[256] = {};
    uint32_t len = 0;
    if (XR_SUCCEEDED(xrPathToString(xrInstance_, st.interactionProfile, 256, &len, buf))) {
        return std::string(buf, buf + (len ? len - 1 : 0));
    }
    return {};
}

// ===== 内部ヘルパ =====
XrPath OpenXRController::StrToPath(XrInstance inst, const char* s) {
    XrPath p = XR_NULL_PATH;
    XrResult result = xrStringToPath(inst, s, &p);
    if (XR_FAILED(result)) {
        char buf[256];
        sprintf_s(buf, "[XR][ERROR] StrToPath failed (0x%08X)\n", result);
        OutputDebugStringA(buf);
    }
    return p;
}


// ===== 破棄 =====
OpenXRController::~OpenXRController() { OnDestroy(); }

void OpenXRController::OnDestroy() {

    if (spaceAimL) { xrDestroySpace(spaceAimL); spaceAimL = XR_NULL_HANDLE; }
    if (apaceAimR) { xrDestroySpace(apaceAimR); apaceAimR = XR_NULL_HANDLE; }
    if (spaceGripL) { xrDestroySpace(spaceGripL); spaceGripL = XR_NULL_HANDLE; }
    if (spaceGripR) { xrDestroySpace(spaceGripR); spaceGripR = XR_NULL_HANDLE; }

    if (actionSet_ != XR_NULL_HANDLE) {
        xrDestroyActionSet(actionSet_);
        actionSet_ = XR_NULL_HANDLE;
    }
    xrInstance_ = XR_NULL_HANDLE;
    xrSession_ = XR_NULL_HANDLE;
    xrAppSpace_ = XR_NULL_HANDLE;

}



