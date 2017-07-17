#include "iostream"  
#include "kinect.h"  
#define _USE_MATH_DEFINES  
#include <math.h>  

int main()
{
	printf("Hello, Wellcome to kinect world!\n");
	IKinectSensor* bb; //����һ��Sensorָ��  
	HRESULT hr = GetDefaultKinectSensor(&bb); // ��ȡһ��Ĭ�ϵ�Sensor  
	if (FAILED(hr))
	{
		printf("No Kinect connect to your pc!\n");
		goto endstop;
	}
	BOOLEAN bIsOpen = 0;
	bb->get_IsOpen(&bIsOpen); // �鿴���Ƿ��Ѿ���  
	printf("bIsOpen�� %d\n", bIsOpen);

	if (!bIsOpen) // û�򿪣����Դ�  
	{
		hr = bb->Open();
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
	bb->get_IsOpen(&bIsOpen); // �Ƿ��Ѿ���  
	printf("bIsOpen�� %d\n", bIsOpen);
	BOOLEAN bAvaliable = 0;
	bb->get_IsAvailable(&bAvaliable); // �Ƿ����  
	printf("bAvaliable�� %d\n", bAvaliable);

	DWORD dwCapability = 0;
	bb->get_KinectCapabilities(&dwCapability); // ��ȡ����  
	printf("dwCapability�� %d\n", dwCapability);
	TCHAR bbuid[256] = { 0 };
	bb->get_UniqueKinectId(256, bbuid); // ��ȡΨһID  
	printf("UID: %s\n", bbuid);

	// ��Ƶ���ݻ�ȡ  
	IAudioSource* audios = nullptr;
	UINT nAudioCount = 0;
	hr = bb->get_AudioSource(&audios);
	if (FAILED(hr))
	{
		printf("Audio Source get failed!\n");
		goto endclose;
	}
	IAudioBeam* audiobm = nullptr;
	IAudioBeamList* audiobml = nullptr;
	audios->get_AudioBeams(&audiobml);
	audiobml->OpenAudioBeam(0, &audiobm); // Ŀǰֻ֧�ֵ�һ��  
	IStream* stm = nullptr;
	audiobm->OpenInputStream(&stm);
	audios->Release();
	audiobm->Release();

	float fAngle = 0.0f;
	float fAngleConfidence = 0.0f;
	ULONG lRead = 0;
	const ULONG lBufferSize = 3200;
	float* fDataArr = new float[lBufferSize];
	while (true)
	{
		fAngle = 0.0f;
		fAngleConfidence = 0.0f;
		audiobm->get_BeamAngle(&fAngle); // ��ȡ��Ƶ�ĽǶȣ�[ -0.872665f, 0.8726665f ]  
		audiobm->get_BeamAngleConfidence(&fAngleConfidence); // ��ȡ��Ƶ�Ŀ��Ŷȣ�0 - 1��  
		if (fAngleConfidence > 0.5f)
			printf("Angle: %3.2f (%1.2f)\n", (fAngle)*180.0f / static_cast<float>(M_PI), fAngleConfidence);
		// audio  data  
		lRead = 0;
		memset(fDataArr, 0, lBufferSize);
		stm->Read(fDataArr, lBufferSize, &lRead);
		if (lRead > 0)
		{
			printf("Audio Buffer: %d\n", lRead);
		}
		Sleep(200);
	}

endclose:
	bb->Close();
endstop:
	system("pause");
	return 0;
}