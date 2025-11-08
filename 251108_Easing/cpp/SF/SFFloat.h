#pragma once

namespace SF {

    // ----------------------------------------
    // 構造体定義 (SFFLOAT2, SFFLOAT3, SFFLOAT4)
    // ----------------------------------------

    struct SFFLOAT2 {
        float x;
        float y;

        SFFLOAT2() : x(0.0f), y(0.0f) {}
        SFFLOAT2(float _x, float _y) : x(_x), y(_y) {}
    };

    struct SFFLOAT3 {
        float x;
        float y;
        float z;

        SFFLOAT3() : x(0.0f), y(0.0f), z(0.0f) {}
        SFFLOAT3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    };

    struct SFFLOAT4 {
        float x;
        float y;
        float z;
        float w;

        SFFLOAT4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
        SFFLOAT4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
    };

    // ----------------------------------------
    // SFFLOAT2 の演算子オーバーロードと計算
    // ----------------------------------------

    // 加算 (V + V)
    inline SFFLOAT2 operator+(const SFFLOAT2& left, const SFFLOAT2& right) {
        return SFFLOAT2(left.x + right.x, left.y + right.y);
    }
    // 減算 (V - V)
    inline SFFLOAT2 operator-(const SFFLOAT2& left, const SFFLOAT2& right) {
        return SFFLOAT2(left.x - right.x, left.y - right.y);
    }
    // スカラー倍 (V * float)
    inline SFFLOAT2 operator*(const SFFLOAT2& left, float scalar) {
        return SFFLOAT2(left.x * scalar, left.y * scalar);
    }
    // スカラー倍 (float * V)
    inline SFFLOAT2 operator*(float scalar, const SFFLOAT2& right) {
        return right * scalar;
    }
    // 割り算 (V / float)
    inline SFFLOAT2 operator/(const SFFLOAT2& left, float scalar) {
        float inv_scalar = 1.0f / scalar;
        return SFFLOAT2(left.x * inv_scalar, left.y * inv_scalar);
    }
    // 内積 (Dot Product)
    inline float DotValue(const SFFLOAT2& left, const SFFLOAT2& right) {
        return left.x * right.x + left.y * right.y;
    }
    // 外積 (Cross Product - スカラ量)
    inline float CrossValue(const SFFLOAT2& left, const SFFLOAT2& right) {
        return left.x * right.y - left.y * right.x;
    }

    // ----------------------------------------
    // SFFLOAT3 の演算子オーバーロードと計算
    // ----------------------------------------

    // 加算 (V + V)
    inline SFFLOAT3 operator+(const SFFLOAT3& left, const SFFLOAT3& right) {
        return SFFLOAT3(left.x + right.x, left.y + right.y, left.z + right.z);
    }
    // 減算 (V - V)
    inline SFFLOAT3 operator-(const SFFLOAT3& left, const SFFLOAT3& right) {
        return SFFLOAT3(left.x - right.x, left.y - right.y, left.z - right.z);
    }
    // スカラー倍 (V * float)
    inline SFFLOAT3 operator*(const SFFLOAT3& left, float scalar) {
        return SFFLOAT3(left.x * scalar, left.y * scalar, left.z * scalar);
    }
    // スカラー倍 (float * V)
    inline SFFLOAT3 operator*(float scalar, const SFFLOAT3& right) {
        return right * scalar;
    }
    // 割り算 (V / float)
    inline SFFLOAT3 operator/(const SFFLOAT3& left, float scalar) {
        float inv_scalar = 1.0f / scalar;
        return SFFLOAT3(left.x * inv_scalar, left.y * inv_scalar, left.z * inv_scalar);
    }
    // 内積 (Dot Product)
    inline float DotValue(const SFFLOAT3& left, const SFFLOAT3& right) {
        return left.x * right.x + left.y * right.y + left.z * right.z;
    }
    // 外積 (Cross Product)
    inline SFFLOAT3 CrossValue(const SFFLOAT3& left, const SFFLOAT3& right) {
        return SFFLOAT3(
            left.y * right.z - left.z * right.y,
            left.z * right.x - left.x * right.z,
            left.x * right.y - left.y * right.x
        );
    }

    // ----------------------------------------
    // SFFLOAT4 の演算子オーバーロードと計算
    // ----------------------------------------

    // 加算 (V + V)
    inline SFFLOAT4 operator+(const SFFLOAT4& left, const SFFLOAT4& right) {
        return SFFLOAT4(left.x + right.x, left.y + right.y, left.z + right.z, left.w + right.w);
    }
    // 減算 (V - V)
    inline SFFLOAT4 operator-(const SFFLOAT4& left, const SFFLOAT4& right) {
        return SFFLOAT4(left.x - right.x, left.y - right.y, left.z - right.z, left.w - right.w);
    }
    // スカラー倍 (V * float)
    inline SFFLOAT4 operator*(const SFFLOAT4& left, float scalar) {
        return SFFLOAT4(left.x * scalar, left.y * scalar, left.z * scalar, left.w * scalar);
    }
    // スカラー倍 (float * V)
    inline SFFLOAT4 operator*(float scalar, const SFFLOAT4& right) {
        return right * scalar;
    }
    // 割り算 (V / float)
    inline SFFLOAT4 operator/(const SFFLOAT4& left, float scalar) {
        float inv_scalar = 1.0f / scalar;
        return SFFLOAT4(left.x * inv_scalar, left.y * inv_scalar, left.z * inv_scalar, left.w * inv_scalar);
    }
    // 内積 (Dot Product)
    inline float DotValue(const SFFLOAT4& left, const SFFLOAT4& right) {
        return left.x * right.x + left.y * right.y + left.z * right.z + left.w * right.w;
    }
    //外積 (Cross Product)
    //* 入力のxyz成分のみを使用し、結果のw成分は0.0f（方向ベクトル）として返します
    inline SFFLOAT4 CrossValue(const SFFLOAT4& left, const SFFLOAT4& right) {
        return SFFLOAT4(
            left.y * right.z - left.z * right.y,
            left.z * right.x - left.x * right.z,
            left.x * right.y - left.y * right.x,
            0.0f // W成分は0（方向ベクトル）
        );
    }



    // ----------------------------------------
    // SFFLOAT用の関数
    // ----------------------------------------

    inline SFFLOAT2 LerpValue(SFFLOAT2 a, SFFLOAT2 b, float t) {
        SFFLOAT2 def = b - a;
        return a + def * t;
    }

    inline SFFLOAT3 LerpValue(SFFLOAT3 a, SFFLOAT3 b, float t) {
        SFFLOAT3 def = b - a;
        return a + def * t;
    }

    inline SFFLOAT4 LerpValue(SFFLOAT4 a, SFFLOAT4 b, float t) {
        SFFLOAT4 def = b - a;
        return a + def * t;
    }

    inline float LerpValue(float a, float b, float t) {
        float def = b - a;
        return a + def * t;
    }



} 

