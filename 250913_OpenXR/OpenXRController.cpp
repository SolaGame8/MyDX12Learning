// OpenXRController.cpp

#include "OpenXRController.h"
#include <cstring>





// ===== ������ =====
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

    //�A�N�V�����Z�b�g�����

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

    // ��ւ̃p�X

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

    //�A�N�V�����Z�b�g�ɃA�N�V������ǉ�

    // ��{�A�N�V�����쐬
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

    // �����{�^�� �A�N�V����
    if (!MakeAction(XR_ACTION_TYPE_BOOLEAN_INPUT, "button_a", "Button A", actA_)) return false;
    if (!MakeAction(XR_ACTION_TYPE_BOOLEAN_INPUT, "button_b", "Button B", actB_)) return false;
    if (!MakeAction(XR_ACTION_TYPE_BOOLEAN_INPUT, "button_x", "Button X", actX_)) return false;
    if (!MakeAction(XR_ACTION_TYPE_BOOLEAN_INPUT, "button_y", "Button Y", actY_)) return false;

    /*
    // �o�C���f�B���O��Ċ֐�
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


    //�o�C���f�B���O�̒�āi���̃v���t�@�C���̎��́A���̂悤�Ƀo�C���f�B���O���āA�Ƃ������



    // Khronos Simple Controller�i/interaction_profiles/khr/simple_controller�j
    SuggestBindings("/interaction_profiles/khr/simple_controller", {
        // ====== Buttons�iBOOLEAN�j======
        { actTriggerClick_,   StrToPath(xrInstance_, "/user/hand/left/input/select/click") },
        { actTriggerClick_,   StrToPath(xrInstance_, "/user/hand/right/input/select/click") },
        { actMenu_,     StrToPath(xrInstance_, "/user/hand/left/input/menu/click") },
        { actMenu_,     StrToPath(xrInstance_, "/user/hand/right/input/menu/click") },

        // ====== Poses�iPOSE�j======
        { actGripPose_, StrToPath(xrInstance_, "/user/hand/left/input/grip/pose") },
        { actGripPose_, StrToPath(xrInstance_, "/user/hand/right/input/grip/pose") },
        { actAimPose_,  StrToPath(xrInstance_, "/user/hand/left/input/aim/pose") },
        { actAimPose_,  StrToPath(xrInstance_, "/user/hand/right/input/aim/pose") },

        // ====== Haptics�iVIBRATION_OUTPUT�j======
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



    // Oculus Touch�i/interaction_profiles/oculus/touch_controller�j
    SuggestBindings("/interaction_profiles/oculus/touch_controller", {
        // ====== Analog�i�^: FLOAT�j======
        // Trigger
        { actTriggerValue_,  StrToPath(xrInstance_, "/user/hand/left/input/trigger/value") },
        { actTriggerValue_,  StrToPath(xrInstance_, "/user/hand/right/input/trigger/value") },
        // Squeeze�iGrip�j
        { actSqueezeValue_, StrToPath(xrInstance_, "/user/hand/left/input/squeeze/value") },
        { actSqueezeValue_, StrToPath(xrInstance_, "/user/hand/right/input/squeeze/value") },

        // ====== Trigger touch�i�^: BOOLEAN�j======
        // �i�K�v�Ȃ�A�N�V������p�Ӂj
        // { actTriggerTouch_, StrToPath(xrInstance_, "/user/hand/left/input/trigger/touch") },
        // { actTriggerTouch_, StrToPath(xrInstance_, "/user/hand/right/input/trigger/touch") },

        // ====== Thumbstick�i�^: VECTOR2F / �⏕��X/Y�� FLOAT�j======
        { actThumbstick_,   StrToPath(xrInstance_, "/user/hand/left/input/thumbstick") },
        { actThumbstick_,   StrToPath(xrInstance_, "/user/hand/right/input/thumbstick") },
        { actThumbClick_,   StrToPath(xrInstance_, "/user/hand/left/input/thumbstick/click") },
        { actThumbClick_,   StrToPath(xrInstance_, "/user/hand/right/input/thumbstick/click") },
        // �C�ӁFX/Y���ʂŎ�肽���ꍇ
        // { actThumbX_,     StrToPath(xrInstance_, "/user/hand/left/input/thumbstick/x") },
        // { actThumbX_,     StrToPath(xrInstance_, "/user/hand/right/input/thumbstick/x") },
        // { actThumbY_,     StrToPath(xrInstance_, "/user/hand/left/input/thumbstick/y") },
        // { actThumbY_,     StrToPath(xrInstance_, "/user/hand/right/input/thumbstick/y") },
        // �C�ӁFThumbstick touch
        // { actThumbTouch_, StrToPath(xrInstance_, "/user/hand/left/input/thumbstick/touch") },
        // { actThumbTouch_, StrToPath(xrInstance_, "/user/hand/right/input/thumbstick/touch") },

        // ====== Thumbrest touch�i�^: BOOLEAN�j======
        // { actThumbrestTouch_, StrToPath(xrInstance_, "/user/hand/left/input/thumbrest/touch") },
        // { actThumbrestTouch_, StrToPath(xrInstance_, "/user/hand/right/input/thumbrest/touch") },

        // ====== Buttons�i�^: BOOLEAN�j======
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
        // System�i�����^�C�����A�v���I�o�𐧌�����ꍇ����j
        // { actSystem_,    StrToPath(xrInstance_, "/user/hand/right/input/system/click") },

        // ====== Poses�i�^: POSE�j======
        { actGripPose_,     StrToPath(xrInstance_, "/user/hand/left/input/grip/pose") },
        { actGripPose_,     StrToPath(xrInstance_, "/user/hand/right/input/grip/pose") },
        { actAimPose_,      StrToPath(xrInstance_, "/user/hand/left/input/aim/pose") },
        { actAimPose_,      StrToPath(xrInstance_, "/user/hand/right/input/aim/pose") },

        // ====== Haptics�i�^: VIBRATION_OUTPUT�j======
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


    
    // PICO 4 �����iAndroid / Link ������������p�X�j
    // �� A/B/X/Y �� PICO4 �� OpenXR �v���t�@�C���ł͌��J����Ă��Ȃ����ߊO���Ă��܂��B
    //   ����Ɋ�{����� trigger / squeeze / thumbstick / menu �ŃJ�o�[���܂��B
    SuggestBindings("/interaction_profiles/bytedance/pico4_controller", {
        // ====== Analog / Click / Touch ======
        // Trigger
        { actTriggerValue_, StrToPath(xrInstance_, "/user/hand/left/input/trigger/value") },
        { actTriggerValue_, StrToPath(xrInstance_, "/user/hand/right/input/trigger/value") },

        // �����u�N���b�N�v�u�^�b�`�v�������A�N�V������p�ӂ��Ă���Ȃ�L����
         { actTriggerClick_, StrToPath(xrInstance_, "/user/hand/left/input/trigger/click") },
         { actTriggerClick_, StrToPath(xrInstance_, "/user/hand/right/input/trigger/click") },
        // { actTriggerTouch_, StrToPath(xrInstance_, "/user/hand/left/input/trigger/touch") },
        // { actTriggerTouch_, StrToPath(xrInstance_, "/user/hand/right/input/trigger/touch") },

        // Squeeze (Grip)
        { actSqueezeValue_, StrToPath(xrInstance_, "/user/hand/left/input/squeeze/value") },
        { actSqueezeValue_, StrToPath(xrInstance_, "/user/hand/right/input/squeeze/value") },
        // { actSqueezeClick_, StrToPath(xrInstance_, "/user/hand/left/input/squeeze/click") },
        // { actSqueezeClick_, StrToPath(xrInstance_, "/user/hand/right/input/squeeze/click") },

        // Thumbstick (vec2) + click (+touch �C��)
        { actThumbstick_,   StrToPath(xrInstance_, "/user/hand/left/input/thumbstick") },
        { actThumbstick_,   StrToPath(xrInstance_, "/user/hand/right/input/thumbstick") },
        { actThumbClick_,   StrToPath(xrInstance_, "/user/hand/left/input/thumbstick/click") },
        { actThumbClick_,   StrToPath(xrInstance_, "/user/hand/right/input/thumbstick/click") },
        // { actThumbTouch_,   StrToPath(xrInstance_, "/user/hand/left/input/thumbstick/touch") },
        // { actThumbTouch_,   StrToPath(xrInstance_, "/user/hand/right/input/thumbstick/touch") },

        // ���� X/Y ���� float �Ƃ��Ă���肽���Ȃ�i�C�Ӂj
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

        // System�i�v���t�@�C����`�ł� user_path �Ȃ��� system=true�j
        // �����^�C���������Ă��Ȃ��ꍇ�͖���������܂�
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



    // Valve Index Controller�i/interaction_profiles/valve/index_controller�j
    SuggestBindings("/interaction_profiles/valve/index_controller", {
        // ===== System�i��������邱�Ƃ������j======
        // { actSystem_,        StrToPath(xrInstance_, "/user/hand/left/input/system/click") },
        // { actSystem_,        StrToPath(xrInstance_, "/user/hand/right/input/system/click") },
        // { actSystemTouch_,   StrToPath(xrInstance_, "/user/hand/left/input/system/touch") },
        // { actSystemTouch_,   StrToPath(xrInstance_, "/user/hand/right/input/system/touch") },

        // ===== Buttons: A / B�i���E�Ƃ��j======
        { actA_,              StrToPath(xrInstance_, "/user/hand/left/input/a/click") },
        { actA_,              StrToPath(xrInstance_, "/user/hand/right/input/a/click") },
        // { actATouch_,      StrToPath(xrInstance_, "/user/hand/left/input/a/touch") },
        // { actATouch_,      StrToPath(xrInstance_, "/user/hand/right/input/a/touch") },

        { actB_,              StrToPath(xrInstance_, "/user/hand/left/input/b/click") },
        { actB_,              StrToPath(xrInstance_, "/user/hand/right/input/b/click") },
        // { actBTouch_,      StrToPath(xrInstance_, "/user/hand/left/input/b/touch") },
        // { actBTouch_,      StrToPath(xrInstance_, "/user/hand/right/input/b/touch") },

        // ===== Squeeze�i���݁F�l + ���́j======
        { actSqueezeValue_,   StrToPath(xrInstance_, "/user/hand/left/input/squeeze/value") },
        { actSqueezeValue_,   StrToPath(xrInstance_, "/user/hand/right/input/squeeze/value") },
        // �����I�Ȉ��́i���́j��ʓr��肽���ꍇ
        // { actSqueezeForce_, StrToPath(xrInstance_, "/user/hand/left/input/squeeze/force") },
        // { actSqueezeForce_, StrToPath(xrInstance_, "/user/hand/right/input/squeeze/force") },

        // ===== Trigger�i�N���b�N/�l/�^�b�`�j======
        { actTriggerClick_,   StrToPath(xrInstance_, "/user/hand/left/input/trigger/click") },
        { actTriggerClick_,   StrToPath(xrInstance_, "/user/hand/right/input/trigger/click") },

        // ���Ȃ��̐݌v�ɍ��킹�āFtrigger �̒l�͏]���ǂ��� select �ɏW��
        { actTriggerValue_,    StrToPath(xrInstance_, "/user/hand/left/input/trigger/value") },
        { actTriggerValue_,    StrToPath(xrInstance_, "/user/hand/right/input/trigger/value") },

        // { actTriggerTouch_, StrToPath(xrInstance_, "/user/hand/left/input/trigger/touch") },
        // { actTriggerTouch_, StrToPath(xrInstance_, "/user/hand/right/input/trigger/touch") },

        // ===== Thumbstick�ivec2 / x,y / click,touch�j======
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

        // ===== Trackpad�ivec2 / x,y / force / touch�j======
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



    // HTC Vive Controller�i/interaction_profiles/htc/vive_controller�j
    SuggestBindings("/interaction_profiles/htc/vive_controller", {
        // ====== Buttons�iBOOLEAN�j======
        // System�i�A�v���ɂ͖���������邱�Ƃ������j
        // { actSystem_,   StrToPath(xrInstance_, "/user/hand/left/input/system/click") },
        // { actSystem_,   StrToPath(xrInstance_, "/user/hand/right/input/system/click") },

        { actSqueezeClick_, StrToPath(xrInstance_, "/user/hand/left/input/squeeze/click") },
        { actSqueezeClick_, StrToPath(xrInstance_, "/user/hand/right/input/squeeze/click") },

        { actMenu_,         StrToPath(xrInstance_, "/user/hand/left/input/menu/click") },
        { actMenu_,         StrToPath(xrInstance_, "/user/hand/right/input/menu/click") },

        { actTriggerClick_, StrToPath(xrInstance_, "/user/hand/left/input/trigger/click") },
        { actTriggerClick_, StrToPath(xrInstance_, "/user/hand/right/input/trigger/click") },

        // ====== Trigger Value�iFLOAT�j======
        { actTriggerValue_, StrToPath(xrInstance_, "/user/hand/left/input/trigger/value") },
        { actTriggerValue_, StrToPath(xrInstance_, "/user/hand/right/input/trigger/value") },

        // ====== Trackpad�iVECTOR2F + �T�u�v�f + Click/Touch�j======
        //{ actTrackpad_,     StrToPath(xrInstance_, "/user/hand/left/input/trackpad") },
        //{ actTrackpad_,     StrToPath(xrInstance_, "/user/hand/right/input/trackpad") },

        //{ actTrackpadClick_,StrToPath(xrInstance_, "/user/hand/left/input/trackpad/click") },
        //{ actTrackpadClick_,StrToPath(xrInstance_, "/user/hand/right/input/trackpad/click") },

        //{ actTrackpadTouch_,StrToPath(xrInstance_, "/user/hand/left/input/trackpad/touch") },
        //{ actTrackpadTouch_,StrToPath(xrInstance_, "/user/hand/right/input/trackpad/touch") },

        // ��X/Y��ʃA�N�V�����ɂ������ꍇ�i�C�Ӂj
        // { actTrackpadX_, StrToPath(xrInstance_, "/user/hand/left/input/trackpad/x") },
        // { actTrackpadX_, StrToPath(xrInstance_, "/user/hand/right/input/trackpad/x") },
        // { actTrackpadY_, StrToPath(xrInstance_, "/user/hand/left/input/trackpad/y") },
        // { actTrackpadY_, StrToPath(xrInstance_, "/user/hand/right/input/trackpad/y") },

        // ====== Poses�iPOSE�j======
        { actGripPose_,     StrToPath(xrInstance_, "/user/hand/left/input/grip/pose") },
        { actGripPose_,     StrToPath(xrInstance_, "/user/hand/right/input/grip/pose") },

        { actAimPose_,      StrToPath(xrInstance_, "/user/hand/left/input/aim/pose") },
        { actAimPose_,      StrToPath(xrInstance_, "/user/hand/right/input/aim/pose") },

        // ====== Haptics�iVIBRATION_OUTPUT�j======
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
        // ���Ȃ��̐݌v�ɍ��킹�āA�g���K�[�l�� actSelectValue_ �ɏW��
        { actTriggerValue_,    StrToPath(xrInstance_, "/user/hand/left/input/trigger/value") },
        { actTriggerValue_,    StrToPath(xrInstance_, "/user/hand/right/input/trigger/value") },

        // ===== Thumbstick (VECTOR2F + click) =====
        { actThumbstick_,     StrToPath(xrInstance_, "/user/hand/left/input/thumbstick") },
        { actThumbstick_,     StrToPath(xrInstance_, "/user/hand/right/input/thumbstick") },
        { actThumbClick_,     StrToPath(xrInstance_, "/user/hand/left/input/thumbstick/click") },
        { actThumbClick_,     StrToPath(xrInstance_, "/user/hand/right/input/thumbstick/click") },

        // �C��: X/Y ���� FLOAT �A�N�V�����Ŏ�肽���ꍇ
        // { actThumbX_,      StrToPath(xrInstance_, "/user/hand/left/input/thumbstick/x") },
        // { actThumbX_,      StrToPath(xrInstance_, "/user/hand/right/input/thumbstick/x") },
        // { actThumbY_,      StrToPath(xrInstance_, "/user/hand/left/input/thumbstick/y") },
        // { actThumbY_,      StrToPath(xrInstance_, "/user/hand/right/input/thumbstick/y") },

        // ===== Trackpad (VECTOR2F + click/touch) =====
        //{ actTrackpad_,       StrToPath(xrInstance_, "/user/hand/left/input/trackpad") },
        //{ actTrackpad_,       StrToPath(xrInstance_, "/user/hand/right/input/trackpad") },
        //{ actTrackpadClick_,  StrToPath(xrInstance_, "/user/hand/left/input/trackpad/click") },
        //{ actTrackpadClick_,  StrToPath(xrInstance_, "/user/hand/right/input/trackpad/click") },
        // �C��: touch / X / Y
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

    //����ւ̃p�X
    XrPath pathHands[2] = { pathLeft, pathRight };

    // �A�N�V�����쐬�p�̍\���̂�����
    XrActionCreateInfo ai{ XR_TYPE_ACTION_CREATE_INFO };
    ai.actionType = type;

    // �A�N�V�����̎��ʖ��ƕ\������ݒ�
    strncpy_s(ai.actionName, XR_MAX_ACTION_NAME_SIZE, name, _TRUNCATE);
    strncpy_s(ai.localizedActionName, XR_MAX_LOCALIZED_ACTION_NAME_SIZE, loc, _TRUNCATE);

    // ���̃A�N�V���������E�̎�̂ǂ���ł��g����悤�ɂ���
    ai.countSubactionPaths = 2;
    ai.subactionPaths = pathHands;

    // �A�N�V�������쐬
    XrResult r = xrCreateAction(actionSet_, &ai, &out);

    // ���ۂ��f�o�b�O�o��
    char buf[256];
    sprintf_s(buf, "[XR] makeAction %-16s -> %s (0x%08X)\n",
        name, XR_SUCCEEDED(r) ? "OK" : "FAILED", r);
    OutputDebugStringA(buf);

    return XR_SUCCEEDED(r);
}



bool OpenXRController::SuggestBindings(const char* profile, std::initializer_list<XrActionSuggestedBinding> bindings) {


    // ��ď�������\���̂�����
    XrInteractionProfileSuggestedBinding sb{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };

    // ���̒�Ă��L���ɂȂ�u�R���g���[���[�v���t�@�C���v�̃p�X�𕶎��񂩂�ϊ�
    sb.interactionProfile = StrToPath(xrInstance_, profile);

    // ��Ă���o�C���f�B���O�ꗗ��ݒ�
    sb.countSuggestedBindings = static_cast<uint32_t>(bindings.size());
    sb.suggestedBindings = const_cast<XrActionSuggestedBinding*>(bindings.begin());

    // OpenXR �����^�C���Ɂu���̃v���t�@�C���ł͂��������o�C���f�B���O�ɂ��āv�ƒ��
    XrResult r = xrSuggestInteractionProfileBindings(xrInstance_, &sb);

    // �����^���s���f�o�b�O�o��
    char buf[256];
    sprintf_s(buf, "[XR] suggest %-40s -> %s (0x%08X)\n",
        profile, XR_SUCCEEDED(r) ? "OK" : "FAILED", r);
    OutputDebugStringA(buf);

    // �����Ȃ� true, ���s�Ȃ� false ��Ԃ�
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

    // ��{�A�N�V����
    //if (!makeAction(XR_ACTION_TYPE_BOOLEAN_INPUT, "select_click", "Select", actSelect_))        return false;
    if (!makeAction(XR_ACTION_TYPE_BOOLEAN_INPUT, "select_click", "SelectClick", actSelectClick_)) return false; // �� �� actSelect_
    if (!makeAction(XR_ACTION_TYPE_FLOAT_INPUT, "select_value", "Select", actSelectValue_)) return false; // �� �ǉ�
    if (!makeAction(XR_ACTION_TYPE_BOOLEAN_INPUT, "squeeze_click", "SqueezeClick", actSqueezeClick_))  return false;
    if (!makeAction(XR_ACTION_TYPE_FLOAT_INPUT, "squeeze", "Squeeze", actSqueeze_))       return false;
    if (!makeAction(XR_ACTION_TYPE_VECTOR2F_INPUT, "thumbstick", "Thumbstick", actThumbstick_))    return false;
    if (!makeAction(XR_ACTION_TYPE_BOOLEAN_INPUT, "thumb_click", "ThumbstickClick", actThumbClick_))    return false;
    if (!makeAction(XR_ACTION_TYPE_BOOLEAN_INPUT, "menu", "Menu", actMenu_))          return false;
    if (!makeAction(XR_ACTION_TYPE_POSE_INPUT, "aim_pose", "Aim Pose", actAimPose_))       return false;
    if (!makeAction(XR_ACTION_TYPE_POSE_INPUT, "grip_pose", "Grip Pose", actGripPose_))      return false;
    if (!makeAction(XR_ACTION_TYPE_VIBRATION_OUTPUT, "haptic", "Haptic", actHaptic_))        return false;

    // [ADDED] �����{�^�� A/B/X/Y �̃A�N�V�����쐬
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

// ===== ���t���[�������^�擾 =====
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

    // �f�o�b�O�\���B�v���t�@�C���m�F���O�i�ω����������Ƃ������j
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

    // ���t���[���̍X�V�O�Ɂu�O�t���[���v��ۑ�
    if (prevValid_) {
        prevLeft_ = stateLeft;
        prevRight_ = stateRight;
    }

    stateLeft = State{};
    stateRight = State{};


    // isActive ����� squeeze �̗L�������̗p�i���Ȃ��Ƃ��Е����A�N�e�B�u���ǂ����̖ڈ��j
    stateLeft.isActive = ReadFloat(actSqueezeValue_, pathLeft, stateLeft.squeezeValue);
    stateRight.isActive = ReadFloat(actSqueezeValue_, pathRight, stateRight.squeezeValue);

    ReadFloat(actTriggerValue_, pathLeft, stateLeft.selectValue);
    ReadFloat(actTriggerValue_, pathRight, stateRight.selectValue);
    // �N���b�N�iboolean�j��������ł͂�����̗p
    ReadBool(actTriggerClick_, pathLeft, stateLeft.selectClick);
    ReadBool(actTriggerClick_, pathRight, stateRight.selectClick);
    // �N���b�N�����Ȃ������^�C�������� value ����⊮�i�������l�͓K�X�����j
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

    // �����{�^�� A/B/X/Y �̎擾
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

    //�f�o�b�O�\��
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


    // ���񂾂��F�ȑO�̒l�������ɂ���iOnPush�ɉe����^���Ȃ��j
    if (!prevValid_) {
        prevLeft_ = stateLeft;
        prevRight_ = stateRight;
        prevValid_ = true;
    }


    return true;
}


XrPosef OpenXRController::GetPose_LeftController() const {

    XrPosef p{};
    p.orientation.w = 1.0f;  // �P�ʎp��

    if (stateLeft.hasGripPose)      p = stateLeft.gripPose;
    else if (stateLeft.hasAimPose)  p = stateLeft.aimPose;

    // RH(OpenXR) �� LH(DirectX) �ϊ�
    // �ʒu�FZ���]
    p.position.z = -p.position.z;

    // ��]�F�N�H�[�^�j�I���� x, y �𔽓]�iz, w �͂��̂܂܁j
    p.orientation.x = -p.orientation.x;
    p.orientation.y = -p.orientation.y;
    // p.orientation.z =  p.orientation.z;
    // p.orientation.w =  p.orientation.w;

    return p;

}

XrPosef OpenXRController::GetPose_RightController() const {

    XrPosef p{};
    p.orientation.w = 1.0f;  // �P�ʎp��

    if (stateRight.hasGripPose)      p = stateRight.gripPose;
    else if (stateRight.hasAimPose)  p = stateRight.aimPose;

    // RH(OpenXR) �� LH(DirectX) �ϊ�
    // �ʒu�FZ���]
    p.position.z = -p.position.z;

    // ��]�F�N�H�[�^�j�I���� x, y �𔽓]�iz, w �͂��̂܂܁j
    p.orientation.x = -p.orientation.x;
    p.orientation.y = -p.orientation.y;
    // p.orientation.z =  p.orientation.z;
    // p.orientation.w =  p.orientation.w;
    
    return p;

}

// ���̃X�e�B�b�N�X��
XrVector2f OpenXRController::GetValue_Left_Stick() const { return stateLeft.thumbstick; }
XrVector2f OpenXRController::GetValue_Right_Stick() const { return stateRight.thumbstick; }

// �f�b�h�]�[���K�p�Łi���adz�j
// ��: dz=0.15f�B���adz������(0,0)�B����ȊO�� 0..1 �ɍă}�b�v�i���ˏ�f�b�h�]�[���j
XrVector2f OpenXRController::GetValue_Left_Stick(float dz) const {

    XrVector2f v = stateLeft.thumbstick;
    float d = (dz < 0.0f ? 0.0f : (dz > 0.99f ? 0.99f : dz));
    float m = std::sqrt(v.x * v.x + v.y * v.y);
    if (m < d) return XrVector2f{ 0.0f, 0.0f };
    float s = (m - d) / (1.0f - d);            // 0..1 �ɍă}�b�v
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


// OnPush�i�����オ��j����Fcurr==true ���� prev==false
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


// ==================== OnPush ���E���b�p�[ ====================
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

// Stick��������
bool OpenXRController::OnPush_Left_Stick() const { return OnPushStick(/*leftHand=*/true); }
bool OpenXRController::OnPush_Right_Stick() const { return OnPushStick(/*leftHand=*/false); }


// ===== �n�v�e�B�N�X =====
bool OpenXRController::ApplyHaptics(bool leftHand, float amplitude, float seconds, float frequencyHz) {

    XrHapticVibration vib{ XR_TYPE_HAPTIC_VIBRATION };
    vib.amplitude = std::clamp(amplitude, 0.0f, 1.0f);
    vib.duration = (XrDuration)(seconds * 1e9); // ns
    vib.frequency = frequencyHz; // 0=�����^�C���C��

    XrHapticActionInfo hi{ XR_TYPE_HAPTIC_ACTION_INFO };
    hi.action = actHaptic_;
    hi.subactionPath = leftHand ? pathLeft : pathRight;
    return XR_SUCCEEDED(xrApplyHapticFeedback(xrSession_, &hi, (XrHapticBaseHeader*)&vib));

}

// ===== ���[�e�B���e�B =====
bool OpenXRController::ReadBool(XrAction a, XrPath sub, bool& dst) const {


    XrActionStateBoolean st{ XR_TYPE_ACTION_STATE_BOOLEAN };

    XrActionStateGetInfo gi{ XR_TYPE_ACTION_STATE_GET_INFO };
    gi.action = a; gi.subactionPath = sub;
    
    if (XR_SUCCEEDED(xrGetActionStateBoolean(xrSession_, &gi, &st))) {
        dst = (st.isActive && st.currentState);

        // ���O�o��
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
        // �����Ń��O�o��
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

// OpenXR(RH)��Pose��DirectX(LH)�s��ɕϊ�
// M_lh = FlipZ * M_rh * FlipZ �iFlipZ=diag(1,1,-1,1)�j
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

    // ������
    char buf[256] = {};
    uint32_t len = 0;
    if (XR_SUCCEEDED(xrPathToString(xrInstance_, st.interactionProfile, 256, &len, buf))) {
        return std::string(buf, buf + (len ? len - 1 : 0));
    }
    return {};
}

// ===== �����w���p =====
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


// ===== �j�� =====
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



