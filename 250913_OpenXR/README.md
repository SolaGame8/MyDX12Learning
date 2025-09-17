

# 使い方
## 使い方
### 使い方



OpenXRManager* XR_Manager;



＜初期化＞
        XR_Manager = new OpenXRManager();

        XR_Manager->Initialize(dx12Device.Get(), commandQueue.Get());


＜毎フレーム　＊必須＞

        
        XR_Manager->UpdateSessionState();//OpenXR の情報更新


＜コントローラー情報取得＞


	if (XR_Manager->controllersReady) {


		const float deadzone = 0.05f;	//デッドゾーン（微細な振動とかは取らない）
		XrVector2f stick = XR_Manager->controller.GetValue_Right_Stick(deadzone);	//右スティックxy


		bool flg = XR_Manager->controller.OnPush_Left_X();	//左 Xボタン

	
		XrPosef rightControllerPose = XR_Manager->controller.GetPose_RightController();	//右コントローラーのポーズ
		//rightControllerPose.position	//x,y,z
		//rightControllerPose.rotation	//x,y,z,w クォータニオン


	}



    XrPosef GetPose_LeftController() const;
    XrPosef GetPose_RightController() const;

    XrVector2f GetValue_Left_Stick()  const;
    XrVector2f GetValue_Right_Stick() const;
    XrVector2f GetValue_Left_Stick(float deadzone)  const;
    XrVector2f GetValue_Right_Stick(float deadzone) const;

    float GetValue_Left_SelectTrigger()  const;
    float GetValue_Right_SelectTrigger() const;

    float GetValue_Left_SqueezeTrigger()  const;
    float GetValue_Right_SqueezeTrigger() const;

    bool OnPushSelectTrigger(bool leftHand) const;
    bool OnPushSqueezeTrigger(bool leftHand) const;
    bool OnPushA(bool leftHand) const;
    bool OnPushB(bool leftHand) const;
    bool OnPushX(bool leftHand) const;
    bool OnPushY(bool leftHand) const;

    bool OnPushMenu(bool leftHand) const;

    bool OnPushStick(bool leftHand) const;


    // Select（人差し指トリガー）押した瞬間
    bool OnPush_Left_SelectTrigger()  const;
    bool OnPush_Right_SelectTrigger() const;

    // Squeeze（握りのトリガー）押した瞬間
    bool OnPush_Left_SqueezeTrigger()  const;
    bool OnPush_Right_SqueezeTrigger() const;

    // A/B/X/Y ボタン押した瞬間
    bool OnPush_Left_A()  const;
    bool OnPush_Right_A() const;

    bool OnPush_Left_B()  const;
    bool OnPush_Right_B() const;

    bool OnPush_Left_X()  const;
    bool OnPush_Right_X() const;

    bool OnPush_Left_Y()  const;
    bool OnPush_Right_Y() const;

    // Menu（メニューボタン）押した瞬間
    bool OnPush_Left_Menu()  const;
    bool OnPush_Right_Menu() const;

    bool OnPush_Left_Stick()  const;
    bool OnPush_Right_Stick() const;

    // ハプティクス（振動）
    bool ApplyHaptics(bool leftHand, float amplitude, float seconds, float frequencyHz = 0.0f);

    // 現在の左右状態
    const State& Left()  const { return stateLeft; }
    const State& Right() const { return stateRight; }



    struct State {
        bool  isActive = false;         // その手の入力が現在有効か
        bool  triggerClick = false;      // 人差し指トリガーのクリック（デジタル）
        float triggerValue = 0.0f;       // 人差し指トリガーのアナログ値(0..1)
        bool  squeezeClick = false;     // 中指ボタン つかむ(クリック)
        float squeezeValue = 0.0f;      // 中指ボタン つかむ(アナログ値 0..1)
        XrVector2f stickValue{ 0,0 };   // スティック
        bool  stickClick = false;  // スティック押し込み
        bool  menu = false;             // メニューボタン

        bool  buttonA = false;          // Aボタン
        bool  buttonB = false;          // Bボタン
        bool  buttonX = false;          // Xボタン
        bool  buttonY = false;          // Yボタン

        XrPosef aimPose{};              // レイ用途の姿勢
        XrPosef gripPose{};             // コントローラ実位置

        bool  hasAimPose = false;
        bool  hasGripPose = false;
    };



＜コントローラー振動＞


	XR_Manager->controller.ApplyHaptics(true, 0.5f, 0.5f, 0.0f); //bool左手, 強さ、秒数、周波数（0.0でランタイム任せにできる）





＜ヘッドマウント情報＞

int viewNum = XR_Manager->xr_viewCount;	//ビューの数（両目なので = 2）




＜描画の手順＞

	XrTime predictedDisplayTime;    //描画予定時間

	XR_Manager->BeginFrame(predictedDisplayTime);	//＊フレーム開始	（ここで、描画予定時間受け取り


    struct EyeMatrix
    {
        XMMATRIX viewMat;           // ビュー行列 (World→View)
        XMMATRIX projMat;           // プロジェクション行列
        XrView   xrView;            // OpenXRの生データ
    };


	std::vector<OpenXRManager::EyeMatrix> eyesData;	//ここにヘッドマウントの両目のカメラの行列（DX12向け）が入る
	float nearZ = 0.01f; // 近クリップ面
	float farZ = 100.0f; // 遠クリップ面

	XR_Manager->GetEyeMatrix(predictedDisplayTime, nearZ, farZ, eyesData);//VRの両目の位置のカメラ行列を取得



        
        OpenXRManager::EyeDirectTarget tgt{};   //描画先
        XR_Manager->GetSwapchainDrawTarget(commandList.Get(), viewIdx, tgt);//VRのスワップチェーンの描画先を取得


	//DX12の描画の設定
        
        commandList->ClearRenderTargetView(tgt.rtv, clearColor, 0, nullptr);// レンダーターゲットをクリア

        commandList->ClearDepthStencilView(tgt.dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);// 深度バッファをクリア

        commandList->OMSetRenderTargets(1, &tgt.rtv, FALSE, &tgt.dsv);// 描画の出力先


	//＊ここで、コマンドで描画（DX12）をする


	XR_Manager->FinishSwapchainDrawTarget(commandList.Get(), viewIdx);//VRのスワップチェーンの描画を終了


	//＊コマンドを閉じる
	commandList->Close();   //コマンドを閉じる

	// ＊コマンドを実行して、ターゲットへの描画を完了させる


        XR_Manager->EndFrame_WithProjection(	//＊フレーム終了　（ターゲット（両目）への描画が完全に終わってる状態にしてから
            eyesData,			//取得した両目のカメラの行列
            nearZ, farZ,		//カメラのニア、ファー
            predictedDisplayTime);	//実際に描画した情報をヘッドマウントにも渡してあげる




	//アプリケーション終了時

        XR_Manager->OnDestroy();	//破棄
        delete XR_Manager;
        XR_Manager = nullptr;


