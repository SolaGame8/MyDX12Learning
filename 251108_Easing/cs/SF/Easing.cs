using System;
using System.Runtime.CompilerServices; // AggressiveInlining 用

namespace SF
{
    /// <summary>
    /// 時間ファクターとイージング関数を提供する静的クラス。
    /// C++の Easing.h に相当します。
    /// </summary>
    public static class Easing
    {
        // C#では Math.PI が利用可能ですが、元のコードに合わせて定数として定義します。
        // C#の float (System.Single) は f をつけずに定義できます。
        private const float PI = 3.1415926535f;

        // --- TimeFactor ---

        /// <summary>
        /// カウンターを 0.0 から 1.0 の範囲で繰り返す時間ファクターを取得します。
        /// </summary>
        /// <param name="counter">現在のカウンター値。</param>
        /// <param name="transitionTime">遷移時間 (0.0 から 1.0 への変化にかかる時間)。</param>
        /// <param name="idleTime">アイドル時間 (1.0 の状態を維持する時間)。</param>
        /// <param name="counterOffset">カウンターのオフセット。</param>
        /// <returns>正規化された時間ファクター (0.0 ～ 1.0)。</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)] // C++の inline に相当
        public static float GetTimeFactor(float counter, float transitionTime = 1.0f, float idleTime = 0.0f, float counterOffset = 0.0f)
        {
            const float totalPeriod = transitionTime + idleTime;

            if (totalPeriod <= 0.0f)
            {
                return 0.0f;
            }

            float adjustedCounter = counter + counterOffset;
            
            // C++の std::fmod は C#では Math.IEEERemainder や % 演算子で代替されますが、
            // 負の数に対する挙動が異なるため、元のロジックを再現します。
            float currentTime = adjustedCounter % totalPeriod;
            if (currentTime < 0.0f)
            {
                currentTime += totalPeriod; // 負の結果を正の周期に変換
            }

            float normalizedValue = currentTime / transitionTime;
            
            // アイドル時間が transitionTime の範囲外を 1.0 にクランプする
            return (normalizedValue < 1.0f) ? normalizedValue : 1.0f;
        }

        /// <summary>
        /// カウンターを 0.0 から 1.0、1.0 から 0.0 のように往復する時間ファクターを取得します。
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float GetPingPongTimeFactor(float counter, float transitionTime = 1.0f, float idleTime = 0.0f, float counterOffset = 0.0f)
        {
            const float halfCycleTime = transitionTime + idleTime;
            const float totalPeriod = 2.0f * halfCycleTime;

            if (totalPeriod <= 0.0f)
            {
                return 0.0f;
            }

            float adjustedCounter = counter + counterOffset;
            
            float currentTime = adjustedCounter % totalPeriod;
            if (currentTime < 0.0f)
            {
                currentTime += totalPeriod; // 負の結果を正の周期に変換
            }

            if (currentTime > halfCycleTime)
            {
                // 戻り (1.0 -> 0.0)
                currentTime -= halfCycleTime;
                float normalizedValue = currentTime / transitionTime;

                // transitionTime の範囲外を 0.0 にクランプ
                return (normalizedValue < 1.0f) ? (1.0f - normalizedValue) : 0.0f;
            }
            else
            {
                // 往き (0.0 -> 1.0)
                float normalizedValue = currentTime / transitionTime;
                // transitionTime の範囲外 (アイドルタイム) を 1.0 にクランプ
                return (normalizedValue < 1.0f) ? normalizedValue : 1.0f;
            }
        }

        // C++の struct TIMEFACTOR_RESULT に相当
        public struct TimeFactorResult
        {
            public int step;
            public float time;

            public TimeFactorResult()
            {
                step = 0;
                time = 0.0f;
            }
        }

        /// <summary>
        /// 指定されたステップ回数で 0.0～1.0 を繰り返す時間ファクターを取得し、現在のステップ数も返します。
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static TimeFactorResult GetSteppedTimeFactor(float counter, int steps, float transitionTime = 1.0f, float idleTime = 0.0f, float counterOffset = 0.0f)
        {
            const float cycleTime = transitionTime + idleTime;
            TimeFactorResult res = new TimeFactorResult();

            if (cycleTime <= 0.0f || steps <= 0)
            {
                return res;
            }

            float adjustedCounter = counter + counterOffset;
            
            float currentTime = adjustedCounter % cycleTime;
            if (currentTime < 0.0f)
            {
                currentTime += cycleTime; // 負の結果を正の周期に変換
            }

            // C++の std::floor は C#では Math.Floor で代替
            int currentStep = (int)Math.Floor(adjustedCounter / cycleTime);
            currentStep %= steps;
            if (currentStep < 0)
            {
                currentStep += steps; // 負の結果を正に変換
            }

            float normalizedValue = currentTime / transitionTime;

            res.step = currentStep;
            res.time = (normalizedValue < 1.0f) ? normalizedValue : 1.0f;

            return res;
        }


        // --- Easing ---

        // 注意: C#の Math クラスの関数は double を受け取るため、float キャストや FPU の考慮が必要な場合があります。
        // ここでは処理負荷軽減の意図を尊重し、可能な限り float 演算のみを使用します。

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float EaseIn(float t)
        {
            // 計算式: t^2
            return t * t;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float EaseInCubic(float t)
        {
            // 計算式: t^3
            return t * t * t;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float EaseOut(float t)
        {
            // 計算式: 1 - (t-1)^2
            float t_minus_1 = t - 1.0f;
            return 1.0f - t_minus_1 * t_minus_1;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float EaseOutCubic(float t)
        {
            // 計算式: 1 + (t-1)^3
            float t_minus_1 = t - 1.0f;
            return 1.0f + t_minus_1 * t_minus_1 * t_minus_1;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float EaseInOut(float t)
        {
            // 計算式: t^2 * (3 - 2 * t)
            return t * t * (3.0f - 2.0f * t);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float EaseInOutQuint(float t)
        {
            // 計算式: t^3 * (6*t^2 - 15*t + 10)
            float t2 = t * t; // t^2
            float t3 = t2 * t; // t^3
            return t3 * (6.0f * t2 - 15.0f * t + 10.0f);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float EaseInBack(float t, float strength = 2.0f)
        {
            // 計算式: t * ((s + 1) * t - s)
            float s = (strength < 0.0f) ? 0.0f : strength;
            float s_plus_one = s + 1.0f;
            return t * (s_plus_one * t - s);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float EaseInBackCubic(float t, float strength = 2.0f)
        {
            // 計算式: t^2 * ((s + 1) * t - s)
            float s = (strength < 0.0f) ? 0.0f : strength;
            float s_plus_one = s + 1.0f;
            return t * t * (s_plus_one * t - s);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float EaseOutBack(float t, float strength = 2.0f)
        {
            // 計算式: t * (s_plus_one + 1 - s_plus_one * t)
            float s = (strength < 0.0f) ? 0.0f : strength;
            float s_plus_one = s + 1.0f;
            return t * (s_plus_one + 1.0f - s_plus_one * t);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float EaseOutBackCubic(float t, float strength = 2.0f)
        {
            // 計算式: 1 + (t-1)^2 * ((s+1)*(t-1) + s)
            float s = (strength < 0.0f) ? 0.0f : strength;
            float s_plus_one = s + 1.0f;
            float t_minus_1 = t - 1.0f;
            return 1.0f + t_minus_1 * t_minus_1 * (s_plus_one * t_minus_1 + s);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float EaseOutElastic(float t, float bounces = 5.0f)
        {
            // C++の std::cos は C#では Math.Cos で代替。
            // Math.Cos は double を返すため、float にキャストします。
            const float base_position = 1.0f;

            float cubic_phase = EaseInCubic(t);
            float angle_rad = cubic_phase * (PI * bounces);

            float oscillation = (float)Math.Cos(angle_rad);

            float damping_factor = 1.0f - t;

            return base_position - oscillation * damping_factor;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float EaseOutBounce(float t, float bounces = 3.0f)
        {
            // C++の std::abs と std::cos は C#では Math.Abs と Math.Cos で代替。
            // Math.Cos は double を返すため、float にキャストします。
            const float base_position = 1.0f;

            float cubic_phase = EaseIn(t);
            float angle_rad = cubic_phase * (PI * bounces);

            float oscillation = Math.Abs((float)Math.Cos(angle_rad));

            float damping_factor = 1.0f - t;

            return base_position - oscillation * damping_factor;
        }
    }
}

