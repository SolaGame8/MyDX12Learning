#include "PerlinNoise.h"

// �ÓI�����o�ϐ��̏�����
PerlinNoise* PerlinNoise::instance = nullptr;

// �f�t�H���g�R���X�g���N�^ (�����_���V�[�h)
PerlinNoise::PerlinNoise() {
    std::random_device rd;
    initialize(rd());
}

// �V�[�h�l�t���R���X�g���N�^
PerlinNoise::PerlinNoise(unsigned int seed) {
    initialize(seed);
}

// ����������
void PerlinNoise::initialize(unsigned int seed) {
    // 0����255�܂ł̏���z����쐬
    p.resize(256);
    std::iota(p.begin(), p.end(), 0);

    // �w�肳�ꂽ�V�[�h�ŗ����W�F�l���[�^��������
    std::mt19937 g(seed);
    // ������V���b�t��
    std::shuffle(p.begin(), p.end(), g);

    // �z���2�{�Ɋg�����ċ��E�������Ȃ���
    p.insert(p.end(), p.begin(), p.end());
}

PerlinNoise::~PerlinNoise() {}

// �f�t�H���g��getInstance
PerlinNoise* PerlinNoise::getInstance() {
    if (instance == nullptr) {
        instance = new PerlinNoise();
    }
    return instance;
}

// �V�[�h�l�t����getInstance
PerlinNoise* PerlinNoise::getInstance(unsigned int seed) {
    if (instance == nullptr) {
        instance = new PerlinNoise(seed);
    }
    // ����: ���ɃC���X�^���X�����݂���ꍇ�A�V�[�h�l�͕ύX����܂���B
    // �V���O���g���̐�����A�C���X�^���X�͈�������݂��Ȃ����߂ł��B
    return instance;
}

void PerlinNoise::destroyInstance() {
    if (instance != nullptr) {
        delete instance;
        instance = nullptr;
    }
}

// �����֐��A��Ԋ֐��A���z�֐��Anoise�֐��͕ύX�Ȃ�
double PerlinNoise::fade(double t) {
    // 6t^5 - 15t^4 + 10t^3
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}
double PerlinNoise::lerp(double t, double a, double b) { 
    // a��b�̊Ԃ�t�̊����ŕ��
    return a + t * (b - a);

}
double PerlinNoise::grad(int hash, double x, double y, double z) { 
    int h = hash & 15;
    double u = h < 8 ? x : y;
    double v = h < 4 ? y : (h == 12 || h == 14 ? x : z);

    // ���z�x�N�g���̐�����I�����A����������
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);

}

double PerlinNoise::noise(double x, double y, double z) { 


    // ���W�𐮐����Ə������ɕ���
    int X = (int)floor(x) & 255;
    int Y = (int)floor(y) & 255;
    int Z = (int)floor(z) & 255;

    x -= floor(x);
    y -= floor(y);
    z -= floor(z);

    // �����֐���K�p
    // ����́A�m�C�Y�����炩�ɂ��邽�߂̏d�v�ȃX�e�b�v
    double u = fade(x);
    double v = fade(y);
    double w = fade(z);

    // 8�̊i�q�_�̃n�b�V���l���v�Z
    // 3������Ԃ̗����̂ɂ���8�̒��_�ɑΉ�
    int A = p[X] + Y;
    int AA = p[A] + Z;
    int AB = p[A + 1] + Z;
    int B = p[X + 1] + Y;
    int BA = p[B] + Z;
    int BB = p[B + 1] + Z;




    // ���`���
    // 8�̊i�q�_���ꂼ��Ō��z�x�N�g���Ƃ̓��ς��v�Z���A
    // ��������`��Ԃ��čŏI�I�ȃm�C�Y�l�𐶐�
    return lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z),
        grad(p[BA], x - 1, y, z)),
        lerp(u, grad(p[AB], x, y - 1, z),
            grad(p[BB], x - 1, y - 1, z))),
        lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1),
            grad(p[BA + 1], x - 1, y, z - 1)),
            lerp(u, grad(p[AB + 1], x, y - 1, z - 1),
                grad(p[BB + 1], x - 1, y - 1, z - 1))));


}

