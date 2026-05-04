# Steam Input VDF リファレンス整理

## 基本ボタン対応表

| VDFキー名 | 対応する部位 |
|----------|-------------|
| button_a | A / × / B(Switch下) |
| button_b | B / ◯ / A(Switch右) |
| button_x | X / □ / Y(Switch左) |
| button_y | Y / △ / X(Switch上) |
| left_shoulder | LB / L1 / L |
| right_shoulder | RB / R1 / R |
| left_trigger | LT / L2 / ZL (押し込み) |
| right_trigger | RT / R2 / ZR (押し込み) |
| button_start | Menu / Options / Start / Plus |
| button_back | View / Share / Back / Minus |
| left_stick_click | 左スティック押し込み |
| right_stick_click | 右スティック押し込み |
| dpad_up | 十字キー 上 |
| dpad_down | 十字キー 下 |
| dpad_left | 十字キー 左 |
| dpad_right | 十字キー 右 |

---

## プラットフォーム別拡張入力

### PlayStation
- center_pad_click: タッチパッド中央押し込み  
- left_pad_click: 左側押し込み  
- right_pad_click: 右側押し込み  
- ps5_mute: ミュートボタン  
- left_fn / right_fn: ファンクションボタン  

### Steam Deck
- l4 / l5: 背面ボタン左  
- r4 / r5: 背面ボタン右  
- left_stick_touch / right_stick_touch: スティック接触  
- left_pad_touch / right_pad_touch: パッド接触  

### Nintendo Switch
- button_capture: キャプチャーボタン  
- left_grip_lower / upper: Joy-Con(L) SR/SL  
- right_grip_lower / upper: Joy-Con(R) SL/SR  

### Xbox
- left_grip_upper / lower: 背面パドル左  
- right_grip_upper / lower: 背面パドル右  
- button_share: シェアボタン  

### その他
- y1 / y2: Legion Go ボタン  
- m1 / m2: マクロボタン  

---

## VDF Mode 一覧

| mode | 内容 |
|------|------|
| dpad | 十字キー |
| four_buttons | ABXYボタン |
| absolute_mouse | マウス扱い |
| joystick_move | 移動 |
| joystick_camera | 視点 |
| trigger | トリガー |
| radial_menu | 円形メニュー |
| gyro | ジャイロ |

---

## 入力動作とモード

| 動作 | mode | 使用キー | 備考 |
|------|------|----------|------|
| Touch | joystick_move等 | pad_touch系 | 接触中のみ |
| Swipe | scroll_wheel | pad_swipe | ホイール扱い |
| Click | four_buttons等 | pad_click | 押し込み |
| Double Tap | Activator | タッチ系 | 2回タップ |

---

## Gyro 詳細

| 要素 | mode | Origin | 内容 |
|------|------|--------|------|
| Move | gyro | Gyro_Move | 全体移動 |
| Pitch | gyro | Gyro_Pitch | 上下傾き |
| Yaw | gyro | Gyro_Yaw | 左右振り |
| Roll | gyro | Gyro_Roll | 回転 |

---

## Trigger 詳細

| 要素 | mode | Origin | 内容 |
|------|------|--------|------|
| Pull | trigger | LeftTrigger_Pull | アナログ |
| Click | trigger | LeftTrigger_Click | 押し込み |

---

## Mode (詳細カテゴリ)

### スティック・マウス系
- joystick_move (JoystickMove)
- joystick_camera (JoystickCamera)
- absolute_mouse (AbsoluteMouse)
- relative_mouse (RelativeMouse)
- mouse_joystick (MouseJoystick)
- gyro

### ボタン系
- four_buttons (FourButtons)
- dpad (Dpad)
- single_button (SingleButton)
- switches (Switches)
- trigger (Trigger)

### UI系
- scroll_wheel (ScrollWheel)
- radial_menu (RadialMenu)
- touch_menu (TouchMenu)
- mouse_region (MouseRegion)

---

## サンプル: スワイプで武器切替

```vdf
"group"
{
    "id"        "10"
    "mode"      "scroll_wheel"
    "inputs"
    {
        "scroll_clockwise"      { "bindings" { "binding" "FPSControls next_weapon" } }
        "scroll_counterclockwise" { "bindings" { "binding" "FPSControls prev_weapon" } }
    }
    "settings"
    {
        "scroll_wheel_origin"   "right_pad_swipe"
    }
}
```
