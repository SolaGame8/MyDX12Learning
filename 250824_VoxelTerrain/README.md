# 250824_VoxelTerrain

**DirectX12 初学者** が、 **学習目的で「無計画」** に作ったものです。

２日間でなんとなくつくったものなので、機能的に考えられて作ったものではないことをご了承ください。

### 内容

* 板ポリでテクスチャーアニメーション（待機、移動、ジャンプなど）
* パーリンノイズを使った地形作成

![screenshot](https://github.com/SolaGame8/MyDX12Learning/blob/main/250824_VoxelTerrain/screenshots/image001.jpg)


### 操作説明

wasd　移動
space ジャンプ
mouse　視点移動

＊ゲームではありません。ただ、表示を作っただけです。


### プロジェクト実行に必要なもの

DirectXTex が必要です。

[DirectXTKはこちら](https://github.com/microsoft/DirectXTex)

プロジェクト → プロパティ → C/C++ → 「Additional Include Directories」
に、DirectXTexフォルダへのパスを書いて、ライブラリをインクルードできる状態にしてください。

DirectXTexは、テクスチャーの読み込みに使っています。

### slnファイル

Visual Studio 2022 で作成されています。

