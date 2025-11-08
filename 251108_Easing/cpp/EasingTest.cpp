#include "EasingTest.h"

#include "Engine/SF/Easing.h"

void EasingTest::Init() {

	//シーン
	myScene001 = AddNewScene("scene001");

	//カメラ
	camera001 = myScene001->GetDefaultCamera();
	camera001->setDistanceFromTarget(1.0f);
	camera001->setFollowTarget(true);
	camera001->setTargetAdjust(XMFLOAT3(0.0f, 1.0f, 0.0f));
	camera001->setLensFocal_fNumber(24.0f, 1.2f);
	camera001->setUseBokeh(false);

	camera001->setTargetDeltaRot(XMFLOAT3(-3.0f, 5.0f, 8.0f), XMFLOAT3(0.0f, 180.0f, 0.0f));

	//空
	sky001 = myScene001->CreateNew_Sphere("sky001", 490.0f, true, 1, 1); //反転  , heapNum, indexedNum
	sky001->SetRendertype(RENDER_UNLIT);	//Unlit描画
	sky001->SetRot(XMFLOAT3(0.0f, 0.0f, 0.0f));
	sky001->SetVisible(true);
	sky001->SetTexture("Resource/test/night001.png", 0, 0);	//MeshIdx, SrvIdx

	//床
	floor001 = myScene001->CreateNew_Plate("floor001", 50.0f, 50.0f, false, 1, 1);//heapNum, IndexedNum
	floor001->SetPos(XMFLOAT3(0.0f, 0.0f, 0.0f));
	floor001->SetVisible(true);
	floor001->SetTexture("Resource/test/easing/grid001.png", 0, 0);	//MeshIdx, SrvIdx


	//太陽
	myScene001->SystemSettings.sunIntensity = 0.8f;
	myScene001->SystemSettings.ambientIntensity = 0.5f;
	myScene001->SystemSettings.sunLightDirection = XMFLOAT4(0.5f, -1.0f, 0.5f, 1.0f);

	//キャラ
	chara001 = myScene001->CreateNew_Character("boy001", "Resource/test/easing/CuteBlue.gltf", 1, 2, 1); //heapNum, IndexedNum, LOD size
	//chara001->SetPosRot(XMFLOAT3{ 0.0f, 0.0f, 0.0f }, XMFLOAT3{ 0.0f, 0.0f, 0.0f });
	chara001->SetVisible(true);
	chara001->SetTexture("Resource/test/easing/CuteMonster_BaseColor.png", 0, POLYTEX_ALBEDO);	//MeshIdx, SrvIdx

	chara001->SetIndexedPosRot(0, XMFLOAT3{ 0.0f, 0.0f, 0.0f }, XMFLOAT3{ 0.0f, 0.0f, 0.0f });
	chara001->SetIndexedPosRot(1, XMFLOAT3{ 2.0f, 0.0f, 0.0f }, XMFLOAT3{ 0.0f, 0.0f, 0.0f });


	//レバー
	lever001 = myScene001->CreateNew_PolygonStatic("lever001", "Resource/test/easing/Lever.gltf", 1, 2, 1);
	lever001->SetVisible(true);
	lever001->SetTexture("Resource/test/easing/LeverAlbedo.png", 0, POLYTEX_ALBEDO);	//MeshIdx, SrvIdx
	lever001->SetIndexedPosRot(0, XMFLOAT3{ 6.0f, 0.0f, 0.0f }, XMFLOAT3{ 0.0f, 0.0f, 0.0f });
	lever001->SetIndexedPosRot(1, XMFLOAT3{ 9.0f, 0.0f, 0.0f }, XMFLOAT3{ 0.0f, 0.0f, 0.0f });

	leverBox001 = myScene001->CreateNew_PolygonStatic("leverBox001", "Resource/test/easing/LeverBox.gltf", 1, 2, 1);
	leverBox001->SetVisible(true);
	leverBox001->SetTexture("Resource/test/easing/LeverAlbedo.png", 0, POLYTEX_ALBEDO);	//MeshIdx, SrvIdx
	leverBox001->SetIndexedPosRot(0, XMFLOAT3{ 6.0f, 0.0f, 0.0f }, XMFLOAT3{ 0.0f, 0.0f, 0.0f });
	leverBox001->SetIndexedPosRot(1, XMFLOAT3{ 9.0f, 0.0f, 0.0f }, XMFLOAT3{ 0.0f, 0.0f, 0.0f });



}

void EasingTest::OnUpdate() {

	myCounter += deltaTime;

	//カメラターゲットの位置が動く
	CalcCameraMove_Field(myScene001, 10.0f, 0.1f, 180.0f, true);//移動速度、マウス移動によるカメラ回転レート、カメラの回転速度（角度/s）、　上下空中移動を使うかどうか

	//空はカメラを中心にする
	sky001->SetPos(camera001->pos);

	//イージングの計算
	Easing();

	//Update
	UpdateScene(myScene001);



}


void EasingTest::Easing() {

	//カウンターを、0.0～1.0に変換
	float tm_1 = SF::Easing::GetTimeFactor(myCounter, 1.0f, 0.5f);				//counter, transition_time, idle_time, counter_offset
	float tm_2 = SF::Easing::GetTimeFactor(myCounter, 1.0f, 0.5f, 0.0f);		//counter, transition_time, idle_time, counter_offset

	float tm_3 = SF::Easing::GetPingPongTimeFactor(myCounter, 1.0f, 0.5f);
	float tm_4 = SF::Easing::GetPingPongTimeFactor(myCounter, 1.0f, 0.5f, 0.0f);//counter, transition_time, idle_time, counter_offset

	/*
	SF::Easing::TIMEFACTOR_RESULT tRes = SF::Easing::GetSteppedTimeFactor(myCounter, 2, 1.0f, 0.5f, 0.0f);//counter, steps// transition_time, idle_time, counter_offset
	tm_3 = tRes.time;
	*/

	//ジャンプ テスト用
	float y_1 = 0.0f;
	float y_2 = 0.0f;

	if (false) {
		y_1 = 2.0f * sin(XM_PI * tm_1);
		y_2 = 2.0f * sin(XM_PI * tm_2);
	}

	//イージング加工
	{
		const int type = 15;

		//キャラ
		switch (type) {
		case 1:		tm_1 = SF::Easing::EaseIn(tm_1);					break;
		case 2:		tm_1 = SF::Easing::EaseInCubic(tm_1);				break;
		case 3:		tm_1 = SF::Easing::EaseOut(tm_1);					break;
		case 4:		tm_1 = SF::Easing::EaseOutCubic(tm_1);				break;
		case 5:		tm_1 = SF::Easing::EaseInOut(tm_1);					break;
		case 6:		tm_1 = SF::Easing::EaseInOutQuint(tm_1);			break;

		case 7:		tm_1 = SF::Easing::EaseInBack(tm_1);				break;
		case 8:		tm_1 = SF::Easing::EaseInBack(tm_1, 5.0f);			break;
		case 9:		tm_1 = SF::Easing::EaseInBackCubic(tm_1);			break;
		case 10:	tm_1 = SF::Easing::EaseInBackCubic(tm_1, 5.0f);		break;
		case 11:	tm_1 = SF::Easing::EaseOutBack(tm_1);				break;
		case 12:	tm_1 = SF::Easing::EaseOutBack(tm_1, 5.0f);			break;
		case 13:	tm_1 = SF::Easing::EaseOutBackCubic(tm_1);			break;
		case 14:	tm_1 = SF::Easing::EaseOutBackCubic(tm_1, 5.0f);	break;
		
		case 15:	tm_1 = SF::Easing::EaseOutElastic(tm_1);			break;
		case 16:	tm_1 = SF::Easing::EaseOutElastic(tm_1, 10.0f);		break;	//bounces, amplitude

		case 17:	tm_1 = SF::Easing::EaseOutBounce(tm_1);				break;
		case 18:	tm_1 = SF::Easing::EaseOutBounce(tm_1, 10.0f);		break;	//bounces

		default: break;
		}

		//レバー
		switch (type) {
		case 1:		tm_3 = SF::Easing::EaseIn(tm_3);					break;
		case 2:		tm_3 = SF::Easing::EaseInCubic(tm_3);				break;
		case 3:		tm_3 = SF::Easing::EaseOut(tm_3);					break;
		case 4:		tm_3 = SF::Easing::EaseOutCubic(tm_3);				break;
		case 5:		tm_3 = SF::Easing::EaseInOut(tm_3);					break;
		case 6:		tm_3 = SF::Easing::EaseInOutQuint(tm_3);			break;

		case 7:		tm_3 = SF::Easing::EaseInBack(tm_3);				break;
		case 8:		tm_3 = SF::Easing::EaseInBack(tm_3, 5.0f);			break;
		case 9:		tm_3 = SF::Easing::EaseInBackCubic(tm_3);			break;
		case 10:	tm_3 = SF::Easing::EaseInBackCubic(tm_3, 5.0f);		break;
		case 11:	tm_3 = SF::Easing::EaseOutBack(tm_3);				break;
		case 12:	tm_3 = SF::Easing::EaseOutBack(tm_3, 5.0f);			break;
		case 13:	tm_3 = SF::Easing::EaseOutBackCubic(tm_3);			break;
		case 14:	tm_3 = SF::Easing::EaseOutBackCubic(tm_3, 5.0f);	break;

		case 15:	tm_3 = SF::Easing::EaseOutElastic(tm_3);			break;
		case 16:	tm_3 = SF::Easing::EaseOutElastic(tm_3, 10.0f);		break;	//bounces, amplitude

		case 17:	tm_3 = SF::Easing::EaseOutBounce(tm_3);				break;
		case 18:	tm_3 = SF::Easing::EaseOutBounce(tm_3, 10.0f);		break;	//bounces

		default: break;
		}



	}



	//0.0～1.0に伴う、位置の遷移
	SF::SFFLOAT3 monPos_1 = SF::LerpValue(SF::SFFLOAT3(0.0f, 0.0f, 0.0f), SF::SFFLOAT3(0.0f, 0.0f, 3.0f), tm_1);	//出発点、到着点
	SF::SFFLOAT3 monPos_2 = SF::LerpValue(SF::SFFLOAT3(2.0f, 0.0f, 0.0f), SF::SFFLOAT3(2.0f, 0.0f, 3.0f), tm_2);

	/*
	if (tRes.step == 1) {//返り
		tm_3 = 1.0f - tm_3;
	}
	*/
	//0.0～1.0に伴う、回転の遷移
	float rot_1 = SF::LerpValue(-30.0f, 30.0f, tm_3);
	float rot_2 = SF::LerpValue(-30.0f, 30.0f, tm_4);

	//位置を設定
	chara001->SetIndexedPosRot(0, XMFLOAT3{ monPos_1.x, monPos_1.y + y_1, monPos_1.z }, XMFLOAT3{ 0.0f, 0.0f, 0.0f });
	chara001->SetIndexedPosRot(1, XMFLOAT3{ monPos_2.x, monPos_2.y + y_2, monPos_2.z }, XMFLOAT3{ 0.0f, 0.0f, 0.0f });
	//角度を設定

	lever001->SetIndexedRot(0, XMFLOAT3{ rot_1, 0.0f, 0.0f });
	lever001->SetIndexedRot(1, XMFLOAT3{ rot_2, 0.0f, 0.0f });


}



void EasingTest::OnRender() {

	RenderScene(myScene001, SCENE_RENDERTYPE_NORMAL);

}

void EasingTest::OnDestroy() {

}




