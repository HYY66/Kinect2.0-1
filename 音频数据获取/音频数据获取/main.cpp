#include "iostream"  
#include "kinect.h"  
#define _USE_MATH_DEFINES  
#include <math.h>  

int main()
{
	printf("Hello, Wellcome to kinect world!\n");
	IKinectSensor* pKinectSensor; //����һ��Sensorָ��  
	HRESULT hr = GetDefaultKinectSensor(&pKinectSensor); // ��ȡһ��Ĭ�ϵ�Sensor  
	if (FAILED(hr))
	{
		printf("No Kinect connect to your pc!\n");
		goto endstop;
	}
	BOOLEAN bIsOpen = 0;
	pKinectSensor->get_IsOpen(&bIsOpen); // �鿴���Ƿ��Ѿ���  
	printf("bIsOpen�� %d\n", bIsOpen);

	if (!bIsOpen) // û�򿪣����Դ�  
	{
		hr = pKinectSensor->Open();
		if (FAILED(hr))
		{
			printf("Kinect Open Failed!\n");
			goto endstop;
		}
		printf("Kinect opened! But it need sometime to work!\n");
		// ����һ��Ҫ��Ȼᣬ����������ж϶��Ǵ����  
		printf("Wait For 3000 ms...\n");
		Sleep(3000);
	}
	bIsOpen = 0;
	pKinectSensor->get_IsOpen(&bIsOpen); // �Ƿ��Ѿ���  
	printf("bIsOpen�� %d\n", bIsOpen);
	BOOLEAN bAvaliable = 0;
	pKinectSensor->get_IsAvailable(&bAvaliable); // �Ƿ����  
	printf("bAvaliable�� %d\n", bAvaliable);

	DWORD dwCapability = 0;
	pKinectSensor->get_KinectCapabilities(&dwCapability); // ��ȡ����  
	printf("dwCapability�� %d\n", dwCapability);
	TCHAR bbuid[256] = { 0 };
	pKinectSensor->get_UniqueKinectId(256, bbuid); // ��ȡΨһID  
	printf("UID: %s\n", bbuid);

	// ��Ƶ���ݻ�ȡ  
	IAudioSource* pAudioSource = nullptr;
	// ��Ƶ֡��ȡ��
	IAudioBeamFrameReader*  pAudioBeamFrameReader = nullptr;
	// ��Ƶ֡����
	IAudioBeamFrameReference* pABFrameRef = nullptr;
	// ��Ƶ֡����
	IAudioBeamFrameList* pAudioBeamFrameList = nullptr;
	// ��Ƶ֡
	IAudioBeamFrame* pAudioBeamFrame = nullptr;
	hr = pKinectSensor->get_AudioSource(&pAudioSource);
	if (FAILED(hr))
	{
		printf("Audio Source get failed!\n");
		goto endclose;
	}
	// �ٻ�ȡ��Ƶ֡��ȡ��
	if (SUCCEEDED(hr)){
		hr = pAudioSource->OpenReader(&pAudioBeamFrameReader);
	}
	
endclose:
	pKinectSensor->Close();
endstop:
	system("pause");
	return 0;
}