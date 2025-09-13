#include "WindowManager.h"


// �E�B���h�E�v���V�[�W��
LRESULT CALLBACK WindowManager::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    //�E�B���h�E�̃C�x���g

    // �����ŁA�E�B���h�E�ʒm �ꗗ������܂�
    //https://learn.microsoft.com/ja-jp/windows/win32/winmsg/window-notifications

    switch (uMsg) {             //�����Ă����E�B���h�E�ʒm(uMsg)�ɂ���ĕ���

    case WM_CLOSE:              //�E�C���h�E�����
    case WM_DESTROY:            //�E�C���h�E��x�{�^���Ƃ��ŏ�������
        PostQuitMessage(0);     //�I�����b�Z�[�W
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


// �R���X�g���N�^
WindowManager::WindowManager(int width, int height, const wchar_t* title)
    : m_width(width)
    , m_height(height)
    , m_hwnd(nullptr) {


    // �E�B���h�E�N���X�̓o�^


    WNDCLASSEX wc = { 0 };

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;

    wc.lpfnWndProc = WindowProc;                            //��L�̊֐��iWindowProc�j�ŃC�x���g���󂯎��A�Ǝw��

    wc.hInstance = GetModuleHandle(nullptr);                //�C���X�^���X
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);            //�J�[�\��

    wc.lpszClassName = L"DirectX12WindowClass";             //�����Ō��߂�B���̃N���X�������݂��邩�`�F�b�N����΁A��d�N����h����

    wc.hbrBackground = CreateSolidBrush(RGB(64, 64, 64));   //�w�i�F



    if (!RegisterClassEx(&wc)) {    //�N���X���V�X�e���ɓo�^

        // �o�^�Ɏ��s�����ꍇ
        MessageBox(nullptr, L"Window class registration failed!", L"Error", MB_ICONERROR | MB_OK);  //���b�Z�[�W�{�b�N�X
        return;
    }



    // �E�B���h�E�̍쐬


    m_hwnd = CreateWindowEx(
        0,                              
        wc.lpszClassName,               //�N���X��
        title,                          //�^�C�g���o�[�ɕ\�������
        WS_OVERLAPPEDWINDOW,            //�X�^�C��
        CW_USEDEFAULT, CW_USEDEFAULT,   // x, y
        width, height,                  // w, h
        nullptr,                        //�e�E�B���h�E
        nullptr,                        //���j���[
        wc.hInstance,                   //�C���X�^���X
        nullptr
    );

    if (!m_hwnd) {

        // �쐬�Ɏ��s�����ꍇ
        MessageBox(nullptr, L"Window creation failed!", L"Error", MB_ICONERROR | MB_OK);
        return;
    }

    // �E�B���h�E��\��
    ShowWindow(m_hwnd, SW_SHOW);

}

// �f�X�g���N�^
WindowManager::~WindowManager() {

    //�E�C���h�E��j��
    DestroyWindow(m_hwnd);

}

// ���b�Z�[�W����
bool WindowManager::ProcessMessages() const {              //main.cpp�̃��C�����[�v�� while ���� �Ŏg�p����Ă���

    MSG msg = { 0 };
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {   //���b�Z�[�W�̎擾

        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (msg.message == WM_QUIT) {   //�I��        //��L�� PostQuitMessage(0); �����s�����ƁAWM_QUIT��������
            return false;
        }
    }
    return true;

}


