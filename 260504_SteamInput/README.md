＊AIに聞いたものなので、嘘が含まれる可能性があります 
＊このファイルは、作成中です

# Steam Input VDFの書き方

## Action Sets の定義
ゲーム内の「状態」を定義
```vdf
"Action Sets"
{
    "InGame"    // プレイ中の設定
    {
        "Sticky"        "0"    // アクションセットの持続性    0 (無効)、1 (有効): このアクションセットが「アクティブ」な状態を強く保持しようとします
        "ID"            "0"    // アクションセットを識別するための独自の番号
    }
    "Menu"
    {
        "ID"    "1"   // メニュー画面はID 1
    }
}
```

## Actions (Manifest) の定義
プレイヤーができる具体的なアクションをリストアップ

```vdf
"actions"
{
    "InGame"    // プレイ中の設定
    {
        "Button"    //ボタン関連
        {
            "jump"      "Jump"    //"jump" プログラム内部で識別するためのID。   "Jump" 表示用（ラベル）
            "fire"      "Shoot"
        }
        "StickPadGyro"    //ジョイスティック、トラックパッド、ジャイロ 関連
        {
            "move"      "Move"
        }
    }
}
```


## Group の作成
物理ボタンとアクションを紐付けます
```vdf
"group"
{
    "id"        "0"
    "mode"      "four_buttons"  // 4つのボタンとして扱う設定
    "inputs"
    {
        "button_a"    //ボタンA
        {
            "activators"    //「どう押されたか」を検知する
            {
                "full_push"
                {
                    "action"    "jump"  // Aボタンでjumpアクション発動
                }
            }
        }
    }
}
```



## Mode 一覧

| mode | 対象 | 内容 |
|------|------|------|

| joystick_move | スティック | 移動 |
| joystick_camera | スティック | カメラ操作（視点移動）に最適化された挙動 |
| flick_stick | スティック | スティックを倒した方向に一瞬で視点を向ける |

| four_buttons | A/B/X/Yボタン | 4つの独立したボタンとして扱う |
| switches | 背面ボタンなど | 単発、または複数のスイッチとして扱う |
| trigger | L2 / R2 | トリガー押し込み量（アナログ）を検知する |
| dpad | 十字キー・スティック | 4方向のデジタル入力として扱う |

| absolute_mouse | トラックパッド | マウスそのものとして動作させる |
| scrollwheel | パッド・スティック | 円を描くような動きをマウスホイールとして扱う |

| mouse_joystick | パッド・スティック | スティック入力をマウス信号に変換して出力する |
| radial_menu | パッド・スティック | 円形メニュー |
| directional_pad | パッド・スティック | dpadに似ているが、より細かな判定（8方向など）が可能 |

| touch_menu | パッド | グリッド状のボタンメニュー |

| gyro | ジャイロ | コントローラー本体の傾きを検知する |

| disabled | 全て | そのパーツの入力を完全に無効化する（誤動作防止） |

---

# Steam Input VDF 構造リファレンス

## トップ階層

| キー名 | 内容 |
|--------|------|
| Action Sets | ゲーム内の「状態」を定義 |
| group | 特定の入力ソース（十字キー、スティック、ボタン群など）に対する「挙動のテンプレート」 |
| preset | プリセット。グループをさらに抽象化したもの |
| controller_mappings | コントローラ設定全体 |
| actions | アクション定義 |
| action_layers | アクションレイヤー |
| localization | ローカライズ |
| settings | 全体設定 |
| description | 説明 |
| title | タイトル |
| author | 作者 |
| export_type | 出力タイプ |

---

## group 内キー一覧

| キー名 | 内容 |
|--------|------|
| id | グループID |
| mode | 動作モード |
| inputs | 入力定義 |
| settings | グループ設定 |
| name | 表示名 |
| description | 説明 |
| action_set | 所属アクションセット |
| action_layer | レイヤー |
| source | 入力元 |
| active | 有効状態 |
| visible | 表示制御 |
| group_type | グループ種類 |
| parent | 親グループ |
| priority | 優先度 |

---

## inputs 内キー一覧

| キー名 | 内容 |
|--------|------|
| bindings | バインド定義 |
| activators | 発火条件 |
| mode_shift | モードシフト |
| chorded_button | 同時押し |
| haptic | 振動 |
| settings | 入力単位設定 |
| deadzone | デッドゾーン |
| invert_x | X反転 |
| invert_y | Y反転 |

---

## bindings 内キー一覧

| キー名 | 内容 |
|--------|------|
| binding | 実際のアクション |
| path | 内部パス |
| command | 旧形式コマンド |

---

## activators 内キー一覧

| キー名 | 内容 |
|--------|------|
| full_press | 完全押し込み |
| soft_press | 半押し |
| double_press | ダブルタップ |
| long_press | 長押し |
| release_press | 離した時 |
| start_press | 押した瞬間 |

---

## activator 配下キー

| キー名 | 内容 |
|--------|------|
| bindings | バインド |
| settings | 設定 |

---

## settings 内キー（代表例）

| キー名 | 内容 |
|--------|------|
| sensitivity | 感度 |
| invert_x | X反転 |
| invert_y | Y反転 |
| deadzone | デッドゾーン |
| haptic_intensity | 振動強度 |
| output_joystick | 出力スティック |
| gyro_axis | ジャイロ軸 |
| gyro_mode | ジャイロモード |
| scroll_wheel_origin | スクロール入力元 |
| friction | 摩擦 |
| acceleration | 加速 |
| edge_spin_speed | 端スピン速度 |
| snap_to_center | 中央吸着 |

---

## 特殊ノード

### mode_shift

| キー名 | 内容 |
|--------|------|
| button | 切替ボタン |

### chorded_button

| キー名 | 内容 |
|--------|------|
| button | 同時押しボタン |


---
---
---
---
---
---


# Steam Input VDF リファレンス

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
| Pull | trigger | LeftTrigger_Pull | 引き量（0%〜100%） |
| Click | trigger | LeftTrigger_Click | 押し込み |

---

## Mode (詳細カテゴリ)

### スティック・マウス
| VDF mode 名 | Enum | 内容・用途 |
|------------|------|------------|
| joystick_move | JoystickMove | 移動操作 |
| joystick_camera | JoystickCamera | 視点操作 |
| absolute_mouse | AbsoluteMouse | マウス扱い |
| relative_mouse | RelativeMouse | 相対マウス |
| mouse_joystick | MouseJoystick | マウス→スティック変換 |
| gyro | - | ジャイロ |

### ボタン
| VDF mode 名 | Enum | 内容・用途 |
|------------|------|------------|
| four_buttons | FourButtons | ABXY |
| dpad | Dpad | 十字キー |
| single_button | SingleButton | 単一ボタン |
| switches | Switches | トグル |
| trigger | Trigger | トリガー |

### UI
| VDF mode 名 | Enum | 内容・用途 |
|------------|------|------------|
| scroll_wheel | ScrollWheel | ホイール |
| radial_menu | RadialMenu | 円形メニュー |
| touch_menu | TouchMenu | グリッドUI |
| mouse_region | MouseRegion | 領域制限 |

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
