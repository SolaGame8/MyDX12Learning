#pragma once

#include <cmath>

namespace SF {


    namespace Easing {


        const float PI = 3.1415926535f;


        //TimeFactor

        inline float GetTimeFactor(float counter, float transition_time = 1.0f, float idle_time = 0.0f, float counter_offset = 0.0f) {

            //カウンターを0.0～1.0で繰り返す

            const float total_period = transition_time + idle_time;

            if (total_period <= 0.0f) {
                return 0.0f;
            }
            float adjusted_counter = counter + counter_offset;
            float current_time = std::fmod(adjusted_counter, total_period);
            if (current_time < 0.0f) {
                current_time += total_period; // 負の結果を正の周期に変換
            }
            float normalized_value = current_time / transition_time;
            return (normalized_value < 1.0f) ? normalized_value : 1.0f;
        }




        inline float GetPingPongTimeFactor(float counter, float transition_time = 1.0f, float idle_time = 0.0f, float counter_offset = 0.0f) {

            //カウンターを0.0～1.0 ～0.0のように往復する

            const float half_cycle_time = transition_time + idle_time;
            const float total_period = 2.0f * half_cycle_time;

            if (total_period <= 0.0f) {
                return 0.0f;
            }

            float adjusted_counter = counter + counter_offset;
            float current_time = std::fmod(adjusted_counter, total_period);
            if (current_time < 0.0f) {
                current_time += total_period; // 負の結果を正の周期に変換
            }

            if (current_time > half_cycle_time) {
                //戻り
                current_time -= half_cycle_time;
                float normalized_value = current_time / transition_time;

                return (normalized_value < 1.0f) ? (1.0f - normalized_value) : 0.0f;
            }
            else {
                //往き
                float normalized_value = current_time / transition_time;
                return (normalized_value < 1.0f) ? normalized_value : 1.0f;
            }


        }


        struct TIMEFACTOR_RESULT {

            int step = 0;
            float time = 0.0f;

        };


        inline TIMEFACTOR_RESULT GetSteppedTimeFactor(float counter, int steps, float transition_time = 1.0f, float idle_time = 0.0f, float counter_offset = 0.0f) {

            //カウンターを、ステップ回数だけ 0.0～1.0を繰り返す

            const float cycle_time = transition_time + idle_time;

            TIMEFACTOR_RESULT res;

            if (cycle_time <= 0.0f || steps <= 0) {
                return res;
            }

            float adjusted_counter = counter + counter_offset;
            float current_time = std::fmod(adjusted_counter, cycle_time);
            if (current_time < 0.0f) {
                current_time += cycle_time; // 負の結果を正の周期に変換
            }

            int current_step = std::floor(adjusted_counter / cycle_time);
            current_step %= steps;
            if (current_step < 0) {
                current_step += steps;  // 負の結果を正に変換
            }

            float normalized_value = current_time / transition_time;

            res.step = current_step;
            res.time = (normalized_value < 1.0f) ? normalized_value : 1.0f;

            return res;


        }



        //Easing

        //入力は0.0～1.0まで対応。範囲外は使用しない前提にしています。
        //自分用に作ったので、入力を範囲内にクランプするような処理は入れていません


        inline float EaseIn(float t) {
            // 計算式: t^2
            return t * t;
        }
        inline float EaseInCubic(float t) {
            // 計算式: t^3
            return t * t * t;
        }

        inline float EaseOut(float t) {
            // 計算式: -t*t + 2*t      //2.0f * t * (1.0f - 0.5f * t) 
           // return -t * t + 2.0f * t;

            float t_minus_1 = t - 1.0f;
            return 1.0f - t_minus_1 * t_minus_1;
        }
        inline float EaseOutCubic(float t) {
            // 計算式: t^3 - 3*t^2 + 3*t
            //return t * t * t - 3.0f * t * t + 3.0f * t;

            float t_minus_1 = t - 1.0f;
            return 1.0f + t_minus_1 * t_minus_1 * t_minus_1;
            //return 1.0f - t_minus_1 * t_minus_1 * t_minus_1;
        }

        inline float EaseInOut(float t) {
            // 計算式: -2 * t^3 + 3 * t^2
            return t * t * (3.0f - 2.0f * t);
        }

        inline float EaseInOutQuint(float t) {
            // 計算式: 6*t^5 - 15*t^4 + 10*t^3
            // 最適化形式: t*t*t * (6.0f*t*t - 15.0f*t + 10.0f)
            float t2 = t * t; // t^2
            float t3 = t2 * t; // t^3
            return t3 * (6.0f * t2 - 15.0f * t + 10.0f);
        }

        inline float EaseInBack(float t, float strength = 2.0f) { //strength 0.0 : No Back
            // 計算式: t * ((s + 1) * t - s)
            float s = (strength < 0.0f) ? 0.0f : strength;
            float s_plus_one = s + 1.0f;
            return t * (s_plus_one * t - s);
        }

        
        inline float EaseInBackCubic(float t, float strength = 2.0f) { //strength 0.0 : No Back
            // 計算式: t^2 * ((s + 1) * t - s)
            float s = (strength < 0.0f) ? 0.0f : strength;
            float s_plus_one = s + 1.0f;
            return t * t * (s_plus_one * t - s);
        }
        


        inline float EaseOutBack(float t, float strength = 2.0f) { //strength 0.0 : No Back
            // 計算式: t * (s_plus_one + 1 - s_plus_one * t)
            float s = (strength < 0.0f) ? 0.0f : strength;
            float s_plus_one = s + 1.0f;
            return t * (s_plus_one + 1 - s_plus_one * t);
        }

        
        inline float EaseOutBackCubic(float t, float strength = 2.0f) { //strength 0.0 : No Back
            // EaseInBackCubicParam のロジックを反転。
            // 計算式: 1 + (s+1)*(t-1)^3 + s*(t-1)^2
            // 最適化: 1 + (t-1)^2 * ((s+1)*(t-1) + s)
            float s = (strength < 0.0f) ? 0.0f : strength;
            float s_plus_one = s + 1.0f;
            float t_minus_1 = t - 1.0f;
            return 1.0f + t_minus_1 * t_minus_1 * (s_plus_one * t_minus_1 + s);
        }
        

        inline float EaseOutElastic(float t, float bounces = 5.0f) {

            // 基本移動（バウンドの中心線となる位置）
            const float base_position = 1.0f;

            // 振動位相の計算
            float cubic_phase = EaseInCubic(t);
            //float cubic_phase = EaseIn(t);
            float angle_rad = cubic_phase * (PI * bounces);

            // 振動値の計算
            float oscillation = std::cos(angle_rad);

            // 減衰（終点に向かって振動を止める）
            float damping_factor = 1.0f - t;

            // 合成（基本位置 + 振幅 * 減衰）
            return base_position - oscillation * damping_factor;
        }


        inline float EaseOutBounce(float t, float bounces = 3.0f) {

            // 基本移動（バウンドの中心線となる位置）
            const float base_position = 1.0f;

            // 振動位相の計算
            //float cubic_phase = EaseInCubic(t);
            float cubic_phase = EaseIn(t);
            float angle_rad = cubic_phase * (PI * bounces);

            // 振動値の計算
            float oscillation = std::abs(std::cos(angle_rad));

            // 減衰（終点に向かって振動を止める）
            float damping_factor = 1.0f - t;

            // 合成（基本位置 + 振幅 * 減衰）
            return base_position - oscillation * damping_factor;
        }



    }
}



