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

| VDFキー名 | 内容 |
|----------|------|
| center_pad_click | タッチパッド中央押し込み |
| left_pad_click | タッチパッド左側押し込み |
| right_pad_click | タッチパッド右側押し込み |
| ps5_mute | ミュートボタン |
| left_fn | ファンクションボタン |
| right_fn | ファンクションボタン |

### Steam Deck

| VDFキー名 | 内容 |
|----------|------|
| l4 | 背面ボタン左（上） |
| l5 | 背面ボタン左（下） |
| r4 | 背面ボタン右（上） |
| r5 | 背面ボタン右（下） |
| left_stick_touch | 左スティック接触 |
| right_stick_touch | 右スティック接触 |
| left_pad_touch | 左パッド接触 |
| right_pad_touch | 右パッド接触 |

### Nintendo Switch

| VDFキー名 | 内容 |
|----------|------|
| button_capture | キャプチャーボタン |
| left_grip_lower | Joy-Con(L) SR |
| left_grip_upper | Joy-Con(L) SL |
| right_grip_lower | Joy-Con(R) SL |
| right_grip_upper | Joy-Con(R) SR |

### Xbox

| VDFキー名 | 内容 |
|----------|------|
| left_grip_upper | 背面パドル左（上） |
| left_grip_lower | 背面パドル左（下） |
| right_grip_upper | 背面パドル右（上） |
| right_grip_lower | 背面パドル右（下） |
| button_share | シェアボタン |

### その他

| VDFキー名 | 内容 |
|----------|------|
| y1 | Legion Go ボタン |
| y2 | Legion Go ボタン |
| m1 | マクロボタン |
| m2 | マクロボタン |

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

### スティック・マウス
| カテゴリ | VDF mode 名 | Enum | 内容・用途 |
|----------|------------|------|------------|
| スティック・マウス | joystick_move | JoystickMove | 移動操作 |
| スティック・マウス | joystick_camera | JoystickCamera | 視点操作 |
| スティック・マウス | absolute_mouse | AbsoluteMouse | マウス扱い |
| スティック・マウス | relative_mouse | RelativeMouse | 相対マウス |
| スティック・マウス | mouse_joystick | MouseJoystick | マウス→スティック変換 |
| スティック・マウス | gyro | - | ジャイロ |

### ボタン
| カテゴリ | VDF mode 名 | Enum | 内容・用途 |
|----------|------------|------|------------|
| ボタン | four_buttons | FourButtons | ABXY |
| ボタン | dpad | Dpad | 十字キー |
| ボタン | single_button | SingleButton | 単一ボタン |
| ボタン | switches | Switches | トグル |
| ボタン | trigger | Trigger | トリガー |

### UI
| カテゴリ | VDF mode 名 | Enum | 内容・用途 |
|----------|------------|------|------------|
| UI | scroll_wheel | ScrollWheel | ホイール |
| UI | radial_menu | RadialMenu | 円形メニュー |
| UI | touch_menu | TouchMenu | グリッドUI |
| UI | mouse_region | MouseRegion | 領域制限 |

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
