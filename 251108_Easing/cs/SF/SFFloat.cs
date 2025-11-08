using System.Runtime.CompilerServices; // AggressiveInlining 用

namespace SF
{
    // --- 構造体定義 (SFFLOAT2, SFFLOAT3, SFFLOAT4) ---

    // C++の構造体は C#の struct で実装し、値型として振る舞うようにします。
    public struct SFFLOAT2
    {
        public float x;
        public float y;

        // コンストラクタ
        public SFFLOAT2(float _x, float _y)
        {
            x = _x;
            y = _y;
        }

        // ----------------------------------------
        // 演算子オーバーロード
        // ----------------------------------------

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static SFFLOAT2 operator +(SFFLOAT2 left, SFFLOAT2 right)
        {
            return new SFFLOAT2(left.x + right.x, left.y + right.y);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static SFFLOAT2 operator -(SFFLOAT2 left, SFFLOAT2 right)
        {
            return new SFFLOAT2(left.x - right.x, left.y - right.y);
        }

        // スカラー倍 (V * float)
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static SFFLOAT2 operator *(SFFLOAT2 left, float scalar)
        {
            return new SFFLOAT2(left.x * scalar, left.y * scalar);
        }

        // スカラー倍 (float * V) - C++コードに合わせて追加
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static SFFLOAT2 operator *(float scalar, SFFLOAT2 right)
        {
            return right * scalar;
        }

        // 割り算 (V / float)
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static SFFLOAT2 operator /(SFFLOAT2 left, float scalar)
        {
            float inv_scalar = 1.0f / scalar;
            return new SFFLOAT2(left.x * inv_scalar, left.y * inv_scalar);
        }
    }

    public struct SFFLOAT3
    {
        public float x;
        public float y;
        public float z;

        public SFFLOAT3(float _x, float _y, float _z)
        {
            x = _x;
            y = _y;
            z = _z;
        }

        // ----------------------------------------
        // 演算子オーバーロード
        // ----------------------------------------

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static SFFLOAT3 operator +(SFFLOAT3 left, SFFLOAT3 right)
        {
            return new SFFLOAT3(left.x + right.x, left.y + right.y, left.z + right.z);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static SFFLOAT3 operator -(SFFLOAT3 left, SFFLOAT3 right)
        {
            return new SFFLOAT3(left.x - right.x, left.y - right.y, left.z - right.z);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static SFFLOAT3 operator *(SFFLOAT3 left, float scalar)
        {
            return new SFFLOAT3(left.x * scalar, left.y * scalar, left.z * scalar);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static SFFLOAT3 operator *(float scalar, SFFLOAT3 right)
        {
            return right * scalar;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static SFFLOAT3 operator /(SFFLOAT3 left, float scalar)
        {
            float inv_scalar = 1.0f / scalar;
            return new SFFLOAT3(left.x * inv_scalar, left.y * inv_scalar, left.z * inv_scalar);
        }
    }

    public struct SFFLOAT4
    {
        public float x;
        public float y;
        public float z;
        public float w;

        public SFFLOAT4(float _x, float _y, float _z, float _w)
        {
            x = _x;
            y = _y;
            z = _z;
            w = _w;
        }

        // ----------------------------------------
        // 演算子オーバーロード
        // ----------------------------------------

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static SFFLOAT4 operator +(SFFLOAT4 left, SFFLOAT4 right)
        {
            return new SFFLOAT4(left.x + right.x, left.y + right.y, left.z + right.z, left.w + right.w);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static SFFLOAT4 operator -(SFFLOAT4 left, SFFLOAT4 right)
        {
            return new SFFLOAT4(left.x - right.x, left.y - right.y, left.z - right.z, left.w - right.w);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static SFFLOAT4 operator *(SFFLOAT4 left, float scalar)
        {
            return new SFFLOAT4(left.x * scalar, left.y * scalar, left.z * scalar, left.w * scalar);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static SFFLOAT4 operator *(float scalar, SFFLOAT4 right)
        {
            return right * scalar;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static SFFLOAT4 operator /(SFFLOAT4 left, float scalar)
        {
            float inv_scalar = 1.0f / scalar;
            return new SFFLOAT4(left.x * inv_scalar, left.y * inv_scalar, left.z * inv_scalar, left.w * inv_scalar);
        }
    }


    // ----------------------------------------
    // SFFLOAT用の関数 (静的クラスとして実装)
    // ----------------------------------------

    public static class SFFloat
    {
        // 内積 (Dot Product)
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float DotValue(SFFLOAT2 left, SFFLOAT2 right)
        {
            return left.x * right.x + left.y * right.y;
        }
        
        // 外積 (Cross Product - スカラ量)
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float CrossValue(SFFLOAT2 left, SFFLOAT2 right)
        {
            return left.x * right.y - left.y * right.x;
        }

        // 内積 (Dot Product)
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float DotValue(SFFLOAT3 left, SFFLOAT3 right)
        {
            return left.x * right.x + left.y * right.y + left.z * right.z;
        }

        // 外積 (Cross Product)
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static SFFLOAT3 CrossValue(SFFLOAT3 left, SFFLOAT3 right)
        {
            return new SFFLOAT3(
                left.y * right.z - left.z * right.y,
                left.z * right.x - left.x * right.z,
                left.x * right.y - left.y * right.x
            );
        }

        // 内積 (Dot Product)
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float DotValue(SFFLOAT4 left, SFFLOAT4 right)
        {
            return left.x * right.x + left.y * right.y + left.z * right.z + left.w * right.w;
        }
        
        // 外積 (Cross Product)
        // 入力のxyz成分のみを使用し、結果のw成分は0.0f（方向ベクトル）として返します
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static SFFLOAT4 CrossValue(SFFLOAT4 left, SFFLOAT4 right)
        {
            return new SFFLOAT4(
                left.y * right.z - left.z * right.y,
                left.z * right.x - left.x * right.z,
                left.x * right.y - left.y * right.x,
                0.0f // W成分は0（方向ベクトル）
            );
        }


        // 線形補間 (Linear Interpolation)

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static SFFLOAT2 LerpValue(SFFLOAT2 a, SFFLOAT2 b, float t)
        {
            // C#の演算子オーバーロードを使用
            SFFLOAT2 def = b - a;
            return a + def * t;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static SFFLOAT3 LerpValue(SFFLOAT3 a, SFFLOAT3 b, float t)
        {
            SFFLOAT3 def = b - a;
            return a + def * t;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static SFFLOAT4 LerpValue(SFFLOAT4 a, SFFLOAT4 b, float t)
        {
            SFFLOAT4 def = b - a;
            return a + def * t;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float LerpValue(float a, float b, float t)
        {
            float def = b - a;
            return a + def * t;
        }
    }
}

