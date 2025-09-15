#pragma once

#include <openxr/openxr.h>

#include <array>
#include <string>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <windows.h> //OutputDebugStringA

// DirectXMath �̓w�b�_�[�̂݁BWindows�Ɉˑ����Ȃ����p���\
#include <DirectXMath.h>
using namespace DirectX;

class OpenXRController {
public:
    struct State {
        bool  isActive = false;         // ���̎�̓��͂����ݗL����
        //bool  select = false;           // �l�����w�{�^�� ����i�v���t�@�C���ɂ���Ă�trigger>0�����j
        bool  selectClick = false;   // �l�����w�g���K�[�̃N���b�N�i�f�W�^���j
        float selectValue = 0.0f;    // �l�����w�g���K�[�̃A�i���O�l(0..1)
        bool  squeezeClick = false;     // ���w�{�^�� ����(�N���b�N)
        float squeezeValue = 0.0f;      // ���w�{�^�� ����(�A�i���O�l 0..1)
        XrVector2f thumbstick{ 0,0 };   // �X�e�B�b�N
        bool  thumbstickClick = false;  // �X�e�B�b�N��������
        bool  menu = false;             // ���j���[�{�^��

        bool  buttonA = false;          // A�{�^��
        bool  buttonB = false;          // B�{�^��
        bool  buttonX = false;          // X�{�^��
        bool  buttonY = false;          // Y�{�^��

        XrPosef aimPose{};              // ���C�p�r�̎p��
        XrPosef gripPose{};             // �R���g���[�����ʒu
        bool  hasAimPose = false;
        bool  hasGripPose = false;
    };

public:
    OpenXRController() = default;
    ~OpenXRController();

    // �������F�C���X�^���X/�Z�b�V����/�A�v����Ԃ�n��
    bool Initialize(XrInstance instance, XrSession session, XrSpace appSpace);

    bool MakeAction(XrActionType type, const char* name, const char* loc, XrAction& out);

    bool SuggestBindings(const char* profile, std::initializer_list<XrActionSuggestedBinding> bindings);

    // aim/grip ��ActionSpace�����E���쐬
    bool CreateSpaces();

    // ���t���[���F�A�N�V�����������ǂݎ��ipredictedDisplayTime��xrWaitFrame�œ������́j
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


    // Select�i�l�����w�g���K�[�j�������u��
    bool OnPush_Left_SelectTrigger()  const;
    bool OnPush_Right_SelectTrigger() const;

    // Squeeze�i����̃g���K�[�j�������u��
    bool OnPush_Left_SqueezeTrigger()  const;
    bool OnPush_Right_SqueezeTrigger() const;

    // A/B/X/Y �{�^���������u��
    bool OnPush_Left_A()  const;
    bool OnPush_Right_A() const;

    bool OnPush_Left_B()  const;
    bool OnPush_Right_B() const;

    bool OnPush_Left_X()  const;
    bool OnPush_Right_X() const;

    bool OnPush_Left_Y()  const;
    bool OnPush_Right_Y() const;

    // Menu�i���j���[�{�^���j�������u��
    bool OnPush_Left_Menu()  const;
    bool OnPush_Right_Menu() const;

    bool OnPush_Left_Stick()  const;
    bool OnPush_Right_Stick() const;

    // �n�v�e�B�N�X�i�U���j
    bool ApplyHaptics(bool leftHand, float amplitude, float seconds, float frequencyHz = 0.0f);

    // ���݂̍��E���
    const State& Left()  const { return stateLeft; }
    const State& Right() const { return stateRight; }




    // Pose �� LH���f���s��iDirectXMath�j
    static DirectX::XMMATRIX PoseToLHMatrix(const XrPosef& pose);

    // �C�ӁF���݂̃C���^���N�V�����v���t�@�C���̕�����i/interaction_profiles/...�j
    std::string GetCurrentInteractionProfilePath(bool leftHand) const;

    // �j���i�f�X�g���N�^�ŌĂ΂�邪�A�����j�����\�j
    void OnDestroy();

private:
    // �����w���p
    static XrPath StrToPath(XrInstance inst, const char* s);

    bool CreateActionsAndBindings();
    bool AttachActionSet(XrSession session);

    // ��ԓǂݎ��
    bool ReadBool(XrAction a, XrPath sub, bool& dst) const;
    bool ReadFloat(XrAction a, XrPath sub, float& dst) const;
    bool ReadVec2(XrAction a, XrPath sub, XrVector2f& dst) const;
    bool Locate(XrSpace space, XrPosef& dst, bool& has, XrTime t) const;

private:
    // �O���Q��
    XrInstance xrInstance_ = XR_NULL_HANDLE;
    XrSession  xrSession_ = XR_NULL_HANDLE;
    XrSpace    xrAppSpace_ = XR_NULL_HANDLE;

    // Action set / actions
    XrActionSet actionSet_ = XR_NULL_HANDLE;
    //XrAction actSelect_ = XR_NULL_HANDLE; // boolean or trigger>0����
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

    // A/B/X/Y �p�̃A�N�V����
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

    // �ǂݎ�茋�ʁi���E�j
    State stateLeft, stateRight;

    // OnPush 1�t���[���O�̏�ԁi�G�b�W���o�p�j
    State prevLeft_{}, prevRight_{};
    bool  prevValid_ = false; // prev���L�����ǂ����i�����false�j

};


