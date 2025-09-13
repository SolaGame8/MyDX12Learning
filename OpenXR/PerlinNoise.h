#pragma once

#include <vector>
#include <numeric>
#include <random>
#include <algorithm>

class PerlinNoise {
private:
    // �V���O���g���C���X�^���X
    static PerlinNoise* instance;

    // �R���X�g���N�^�����J��
    PerlinNoise();

    // �V�[�h�l���󂯎��R���X�g���N�^
    PerlinNoise(unsigned int seed);

    ~PerlinNoise();

    // �R�s�[�Ƒ�����֎~
    PerlinNoise(const PerlinNoise&) = delete;
    PerlinNoise& operator=(const PerlinNoise&) = delete;

    std::vector<int> p;

    void initialize(unsigned int seed); // ���������������ʉ�

    double fade(double t);
    double lerp(double t, double a, double b);
    double grad(int hash, double x, double y, double z);

public:
    // �V���O���g���C���X�^���X���擾���郁�\�b�h
    static PerlinNoise* getInstance();

    // �V�[�h�l�t���ŃV���O���g���C���X�^���X���擾���郁�\�b�h
    static PerlinNoise* getInstance(unsigned int seed);

    // �V���O���g���C���X�^���X��������郁�\�b�h
    static void destroyInstance();

    // 2D�p�[�����m�C�Y�𐶐����郁�\�b�h
    double noise(double x, double y, double z);
};


