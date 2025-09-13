

#include "Windows.h"        //�E�C���h�E�Y

#include "WindowManager.h"  //�E�C���h�E���J������N���X
#include "DirectX12App.h"   //DirectX12�̏������Ǝ��s�����鎩��N���X


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {



    // �E�B���h�E�̍쐬
    const int windowWidth = 1280;
    const int windowHeight = 720;

    WindowManager windowManager(windowWidth, windowHeight, L"DirectX 12 Learn"); //w, h, �E�C���h�E���i�w�b�_�[�ɕ\�������j

    // DirectX 12�A�v���P�[�V�����̍쐬�Ə�����

    DirectX12App Dx12App;
    if (!Dx12App.Initialize(windowManager.GetHandle())) {  //�E�C���h�E�̃n���h����n���ď�����
        return 1; // ���������s�i�A�v���P�[�V�����I���j
    }


    // ���C�����[�v

    while (windowManager.ProcessMessages()) {      //�E�C���h�E������ꂽ���Ȃǂ�false�ɂȂ�

        Dx12App.Render();   // DirectX12�`�揈�����[�v

    }

    //�I������
    Dx12App.OnDestroy();    //�O���t�B�b�N�{�[�h�̏��������S�ɏI����Ă���A�A�v���P�[�V�����I������




    return 0;   //�A�v���P�[�V�����I��
}




