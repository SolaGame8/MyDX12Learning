using SF;
using System;

public class EasingTest
{


    public void Easing()
    {
        //カウンターを、0.0～1.0に変換
        float tm_1 = Easing.GetTimeFactor(myCounter, 1.0f, 0.5f);				//counter, transition_time, idle_time, counter_offset
        float tm_2 = Easing.GetTimeFactor(myCounter, 1.0f, 0.5f, 0.0f);			//counter, transition_time, idle_time, counter_offset

        float tm_3 = Easing.GetPingPongTimeFactor(myCounter, 1.0f, 0.5f);
        float tm_4 = Easing.GetPingPongTimeFactor(myCounter, 1.0f, 0.5f, 0.0f);//counter, transition_time, idle_time, counter_offset

        /*
        SF.Easing.TimeFactorResult tRes = Easing.GetSteppedTimeFactor(myCounter, 2, 1.0f, 0.5f, 0.0f);//counter, steps// transition_time, idle_time, counter_offset
        tm_3 = tRes.time;
        */

        //ジャンプ テスト用
        float y_1 = 0.0f;
        float y_2 = 0.0f;

        if (false) {
            y_1 = 2.0f * (float)Math.Sin(XM_PI * tm_1);
            y_2 = 2.0f * (float)Math.Sin(XM_PI * tm_2);
        }

        //イージング加工
        {
            const int type = 15;

            //キャラ
            switch (type) {
            case 1:		tm_1 = Easing.EaseIn(tm_1);					break;
            case 2:		tm_1 = Easing.EaseInCubic(tm_1);			break;
            case 3:		tm_1 = Easing.EaseOut(tm_1);				break;
            case 4:		tm_1 = Easing.EaseOutCubic(tm_1);			break;
            case 5:		tm_1 = Easing.EaseInOut(tm_1);				break;
            case 6:		tm_1 = Easing.EaseInOutQuint(tm_1);			break;

            case 7:		tm_1 = Easing.EaseInBack(tm_1);				break;
            case 8:		tm_1 = Easing.EaseInBack(tm_1, 5.0f);		break;
            case 9:		tm_1 = Easing.EaseInBackCubic(tm_1);		break;
            case 10:	tm_1 = Easing.EaseInBackCubic(tm_1, 5.0f);	break;
            case 11:	tm_1 = Easing.EaseOutBack(tm_1);			break;
            case 12:	tm_1 = Easing.EaseOutBack(tm_1, 5.0f);		break;
            case 13:	tm_1 = Easing.EaseOutBackCubic(tm_1);		break;
            case 14:	tm_1 = Easing.EaseOutBackCubic(tm_1, 5.0f);	break;
            
            case 15:	tm_1 = Easing.EaseOutElastic(tm_1);			break;
            case 16:	tm_1 = Easing.EaseOutElastic(tm_1, 10.0f);	break;	//bounces

            case 17:	tm_1 = Easing.EaseOutBounce(tm_1);			break;
            case 18:	tm_1 = Easing.EaseOutBounce(tm_1, 10.0f);	break;	//bounces

            default: break;
            }

            //レバー
            switch (type) {
            case 1:		tm_3 = Easing.EaseIn(tm_3);					break;
            case 2:		tm_3 = Easing.EaseInCubic(tm_3);			break;
            case 3:		tm_3 = Easing.EaseOut(tm_3);				break;
            case 4:		tm_3 = Easing.EaseOutCubic(tm_3);			break;
            case 5:		tm_3 = Easing.EaseInOut(tm_3);				break;
            case 6:		tm_3 = Easing.EaseInOutQuint(tm_3);			break;

            case 7:		tm_3 = Easing.EaseInBack(tm_3);				break;
            case 8:		tm_3 = Easing.EaseInBack(tm_3, 5.0f);		break;
            case 9:		tm_3 = Easing.EaseInBackCubic(tm_3);		break;
            case 10:	tm_3 = Easing.EaseInBackCubic(tm_3, 5.0f);	break;
            case 11:	tm_3 = Easing.EaseOutBack(tm_3);			break;
            case 12:	tm_3 = Easing.EaseOutBack(tm_3, 5.0f);		break;
            case 13:	tm_3 = Easing.EaseOutBackCubic(tm_3);		break;
            case 14:	tm_3 = Easing.EaseOutBackCubic(tm_3, 5.0f);	break;

            case 15:	tm_3 = Easing.EaseOutElastic(tm_3);			break;
            case 16:	tm_3 = Easing.EaseOutElastic(tm_3, 10.0f);	break;	//bounces

            case 17:	tm_3 = Easing.EaseOutBounce(tm_3);			break;
            case 18:	tm_3 = Easing.EaseOutBounce(tm_3, 10.0f);	break;	//bounces

            default: break;
            }
        }

        //0.0～1.0に伴う、位置の遷移
        SFFLOAT3 monPos_1 = SFFloat.LerpValue(new SFFLOAT3(0.0f, 0.0f, 0.0f), new SFFLOAT3(0.0f, 0.0f, 3.0f), tm_1);	//出発点、到着点
        SFFLOAT3 monPos_2 = SFFloat.LerpValue(new SFFLOAT3(2.0f, 0.0f, 0.0f), new SFFLOAT3(2.0f, 0.0f, 3.0f), tm_2);

        /*
        if (tRes.step == 1) {//返り
            tm_3 = 1.0f - tm_3;
        }
        */

        //0.0～1.0に伴う、回転の遷移
        float rot_1 = SFFloat.LerpValue(-30.0f, 30.0f, tm_3);
        float rot_2 = SFFloat.LerpValue(-30.0f, 30.0f, tm_4);

        //位置を設定
        // C++: chara001->SetIndexedPosRot(0, XMFLOAT3{ monPos_1.x, monPos_1.y + y_1, monPos_1.z }, XMFLOAT3{ 0.0f, 0.0f, 0.0f });
        chara001.SetIndexedPosRot(0, monPos_1.x, monPos_1.y + y_1, monPos_1.z, 0.0f, 0.0f, 0.0f);
        // C++: chara001->SetIndexedPosRot(1, XMFLOAT3{ monPos_2.x, monPos_2.y + y_2, monPos_2.z }, XMFLOAT3{ 0.0f, 0.0f, 0.0f });
        chara001.SetIndexedPosRot(1, monPos_2.x, monPos_2.y + y_2, monPos_2.z, 0.0f, 0.0f, 0.0f);
        
        //角度を設定
        // C++: lever001->SetIndexedRot(0, XMFLOAT3{ rot_1, 0.0f, 0.0f });
        lever001.SetIndexedRot(0, rot_1, 0.0f, 0.0f);
        // C++: lever001->SetIndexedRot(1, XMFLOAT3{ rot_2, 0.0f, 0.0f });
        lever001.SetIndexedRot(1, rot_2, 0.0f, 0.0f);
    }
}


