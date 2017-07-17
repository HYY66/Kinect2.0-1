#include <Windows.h>  
#include <Kinect.h>
#include <iostream>  
#include<ctime>  
#include<cassert>  
#include<process.h>  

using namespace std;

#define _DDDEBUG

static int CTRL = 0;

template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease)
{
	if (pInterfaceToRelease != NULL){
		pInterfaceToRelease->Release();
		pInterfaceToRelease = NULL;
	}
}

// ��¼���Ƶ�ǰλ��  
enum GesturePos{ // 
	NonePos = 0,
	Left,
	Right,
	Neutral // �м�
};

// �ж�ʶ��״̬  
enum DetectionState{
	NoneState = 0,
	Success,
	Failed,
	InProgress // ʶ�������
};

// �ж�������Ҫ������  
struct DataState{
	GesturePos Pos;     // ÿ���˵�������λ��  
	DetectionState State; // ʶ��״̬
	int times; // �Ӷ�����
	time_t timestamp; // ʱ���
	void Reset() // ״̬������
	{
		Pos = GesturePos::NonePos;
		State = DetectionState::NoneState;
		times = 0;
		timestamp = 0;
	}
};

// ��������ж��߼�����  
class GestureDetection{
public:
	GestureDetection(float neutral_threshold, int times, double difftimes)
		: neutral_threshold(neutral_threshold)
		, times(times)
		, difftimes(difftimes)
		, left_hand(0)
		, right_hand(1) // ȱʡֵ right_hand = 1
	{
		wave_datas[0][left_hand].Reset();
		wave_datas[0][right_hand].Reset();
	}
	// ���ܣ�ѭ�����չ������ݣ����ʶ���Ϊ���ֶ����������success��  
	// ʶ��ʧ�������failed��  
	void Update(IBody * frame)
	{

		if (frame == NULL)
			return;
		JudgeState(frame, wave_datas[1][right_hand], true);
	}
private:
	DataState wave_datas[1][2];        // ��¼ÿ���ˣ�ÿֻ�ֵ�״̬  
	const int left_hand;                            // ���� ID  
	const int right_hand;                           // ���� ID  
	// �м�λ�÷�ֵ���ڸ÷�Χ�ڵĶ���Ϊ�����м�λ�ã�������ⲿ�� x ���꣩  
	const float neutral_threshold;
	// ���ִ�����ֵ���ﵽ�ô�����Ϊ�ǻ���  
	const int times;
	// ʱ�����ƣ����������ʱ�����Ȼʶ�𲻳����ֶ�������Ϊʶ��ʧ��  
	const double difftimes;
	// �첲����
	int elbow ;
	int hand;

	// �жϵ�ǰ��״̬�ɹ������success���������¼���DetectionEvent   
	// ʧ����� failed���� UpDate ��������  
	void JudgeState(IBody *n_body, DataState& data, int handID )
	{

		Joint joints[JointType_Count]; // ���������Ϣ

		n_body->GetJoints(JointType::JointType_Count, joints);  // ��ȡ������Ϣ�ڵ�

		if (handID == 0)
		{
			elbow = JointType_ElbowLeft;
			hand = JointType_HandLeft;
		}
		else
		{
			elbow = JointType_ElbowRight;
			hand = JointType_HandRight;
		}

		if (!IsSkeletonTrackedWell(n_body, handID))  //  ����ֲ���λ�����ⲿ֮��  ����ΪΪ��
		{
			if (data.State == InProgress)
			{
#ifdef _DDEBUG
				cout << "not a well skeleton, detection failed!\n";
#endif  
				data.Reset();
				return;
			}
		}

		float curpos = joints[hand].Position.X;
		float center = joints[elbow].Position.X;  //  �õ����ֲ����ⲿ��X�����λ��  ��������

		if (!IsNeutral(curpos, center))  //  ����ֲ�������������λ��
		{
			if (data.Pos == NonePos)
			{
#ifdef _DDEBUG  
				cout << "found!\n";
#endif  

				data.times++;


				if (get_length(curpos, center) == -1)
				{
					data.Pos = Left;
				}
				else if (get_length(curpos, center) == 1)
				{
					data.Pos = Right;
				}
				cout << "times:" << data.times << endl;
				if (data.Pos == Left)
				{
					cout << "left !\n";
				}
				else if (data.Pos == Right)
				{
					cout << "right!\n";
				}
				else
					cout << "you can't see me!\n";

				data.State = InProgress;
				data.timestamp = time(NULL);

			}

			else if (((data.Pos == Left) && get_length(curpos, center) == 1) || ((data.Pos == Right) && get_length(curpos, center) == -1))  // ������Ұ�  �Ұ������
			{

				assert(data.State == InProgress);
				data.times++;
				data.Pos = (data.Pos == Left) ? Right : Left;
#ifdef _DDDEBUG  
				cout << "times:" << data.times << endl;
				if (data.Pos == Left)
				{
					cout << "left !\n";
				}
				else if (data.Pos == Right)
				{
					cout << "right!\n";
				}
				else
					cout << "you can't see me!\n";
#endif  
				if (data.times >= times)
				{
#ifdef _DDDEBUG  
					cout << "success!\n";
#endif  
					CTRL = 1;
					data.Reset();
				}
				else if (difftime(time(NULL), data.timestamp) > difftimes)
				{
#ifdef _DDDEBUG  
					cout << "time out, detection failed!\n";
					cout << "data.times : " << data.times << endl;
#endif  
					data.Reset();
				}
			}
		}

	}

	bool IsLeftSide(float curpos, float center)
	{
		int i = 0;
		i = get_length(curpos, center);
		if (i == -1)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	bool IsRightSide(float curpos, float center)
	{
		int i = 0;
		i = get_length(curpos, center);
		if (i == 1)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	bool IsNeutral(float curpos, float center) // �����ֱ�Ϊ�ֲ���λ�ú��ⲿ��λ��  �ж��Ƿ���������״̬
	{
		int i = 0;
		i = get_length(curpos, center);
		if (i == 0)
		{
			return true;  // ��������״̬
		}
		else
		{
			return false;
		}
	}

	int get_length(float shou, float zhou)   //  ���룺�ֺ����X����  ������ұ��� 1 ����� -1 �м��� 0
	{
		if (shou >= 0 && zhou >= 0) 
		{
			if ((shou - zhou) > neutral_threshold)
			{
				return 1; // ���ұ�
			}
			else if ((shou - zhou) < neutral_threshold || (zhou - shou) > -neutral_threshold)
			{
				return 0;  // ����
			}
			else
			{
				return -1;   // ���
			}
		}
		else if (shou >= 0 && zhou <= 0)
		{
			if ((shou + (-zhou)) > neutral_threshold)
			{
				return 1; // �ұ�
			}
			else
			{
				return 0; // ����
			}
		}
		else if (shou <= 0 && zhou >= 0)
		{
			if (((-shou) + zhou) > neutral_threshold)
			{
				return -1; // ���
			}
			else
			{
				return 0; // ����
			}
		}
		else
		{
			if ((-shou) >= (-zhou))
			{
				if (((-shou) + zhou) > neutral_threshold)
				{
					return -1; // ���
				}
				else if (((-shou) + zhou) < neutral_threshold)
				{
					return 0; // ����
				}
				else
				{
					return 1;  // �ұ�
				}
			}
			else
			{
				if (((-zhou) + shou) > neutral_threshold)
				{
					return 1; // �ұ�
				}
				else
				{
					return 0; // ����
				}
			}
		}

	}

	// �жϹ���׷���������������׷��������ֲ�λ����������  
	bool IsSkeletonTrackedWell(IBody * n_body, int handID )
	{
		Joint joints[JointType_Count];
		n_body->GetJoints(JointType::JointType_Count, joints);
		if (handID == 0)
		{
			elbow = JointType_ElbowLeft;
			hand = JointType_HandLeft;
		}
		else
		{
			elbow = JointType_ElbowRight;
			hand = JointType_HandRight;
		}
		if (joints[hand].Position.Y > joints[elbow].Position.Y) return true;
		else return false;
	}
};

int main()
{
	IKinectSensor *kinect = NULL;
	HRESULT hr = S_OK;
	hr = GetDefaultKinectSensor(&kinect);  //  �õ�Ĭ�ϵ��豸

	if (FAILED(hr) || kinect == NULL)
	{
		cout << "���� sensor ʧ��\n";
		return -1;
	}
	if (kinect->Open() != S_OK) // �Ƿ�򿪳ɹ�
	{
		cout << "Kinect sensor û׼����\n";
		return -1;
	}

	IBodyFrameSource *bady = nullptr;  // ��ȡԴ
	hr = kinect->get_BodyFrameSource(&bady);

	IBodyFrameReader *pBodyReader;

	hr = bady->OpenReader(&pBodyReader); // �򿪻�ȡ������Ϣ��  Reader
	if (FAILED(hr)){
		std::cerr << "Error : IBodyFrameSource::OpenReader()" << std::endl;
		return -1;
	}

	cout << "��ʼ���\n";

	GestureDetection gesture_detection(0.05, 5, 5);

	while (1)
	{
		IBodyFrame* pBodyFrame = nullptr;
		hr = pBodyReader->AcquireLatestFrame(&pBodyFrame);

		if (SUCCEEDED(hr)){
			IBody* pBody[BODY_COUNT] = { 0 }; // Ĭ�ϵ��� 6 ������ ����ʼ�����еĹ�����Ϣ
			//���¹�������  
			hr = pBodyFrame->GetAndRefreshBodyData(BODY_COUNT, pBody); // ˢ�¹�����Ϣ��6����
			if (SUCCEEDED(hr))
			{
				BOOLEAN bTracked = false;

				for (int i = 0; i < 6; i++)
				{
					hr = pBody[i]->get_IsTracked(&bTracked); // ����Ƿ�׷��

					if (SUCCEEDED(hr) && bTracked)
					{

						gesture_detection.Update(pBody[i]);
					}
				}
			}
			for (int count = 0; count < BODY_COUNT; count++){
				SafeRelease(pBody[count]);
			}
		}
		if (CTRL == 1)
		{
			break;  //  ʶ��ɹ��Ժ�  ����ʶ�����
		}

		SafeRelease(pBodyFrame);  // �������ͷ�
	}

	kinect->Close();  // �ر��豸
	system("pause");
	return 0;
}
