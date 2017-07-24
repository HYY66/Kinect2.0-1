#include <Windows.h>
#include <atlstr.h>
#include <sphelper.h>
#include <sapi.h>
#include <comutil.h>
#include <string.h>

#pragma comment(lib,"sapi.lib")
#ifdef _UNICODE
#pragma   comment(lib,   "comsuppw.lib")  //_com_util::ConvertBSTRToString
#else
#pragma   comment(lib,   "comsupp.lib")  //_com_util::ConvertBSTRToString
#endif

#define GID_CMD_GR 333333
#define WM_RECOEVENT WM_USER+1

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

TCHAR szAppName[] = TEXT("��������Demo");
BOOL b_initSR;
BOOL b_Cmd_Grammar;
CComPtr<ISpRecoContext> m_cpRecoCtxt;//����ʶ�����ӿ�
CComPtr<ISpRecoGrammar> m_cpCmdGramma;//ʶ���﷨
CComPtr<ISpRecognizer> m_cpRecoEngine; //����ʶ������
int speak(wchar_t *str);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	HWND hwnd;
	MSG msg;
	WNDCLASS wndclass;

	//������ṹ���ʼ��ֵ
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hInstance = hInstance;
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;

	//ע�ᴰ����
	if (!RegisterClass(&wndclass))
	{
		//ʧ�ܺ���ʾ������
		MessageBox(NULL, TEXT("This program requires Windows NT!"), szAppName, MB_ICONERROR);
		return 0;
	}

	//��������
	hwnd = CreateWindow(szAppName,
		TEXT("����ʶ��"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL);

	//��ʾ����
	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);

	//������Ϣѭ��
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);//������Ϣ
		DispatchMessage(&msg);//�ַ���Ϣ
	}
	return msg.wParam;
}

/*
*��Ϣ�ص�����,�ɲ���ϵͳ����
*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;

	switch (message)
	{
	case WM_CREATE:
	{
					  //��ʼ��COM�˿�
					  ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
					  //����ʶ�������Ľӿ�
					  HRESULT hr = m_cpRecoEngine.CoCreateInstance(CLSID_SpSharedRecognizer);
					  if (SUCCEEDED(hr))
					  {
						  hr = m_cpRecoEngine->CreateRecoContext(&m_cpRecoCtxt);
					  }
					  else
					  {
						  MessageBox(hwnd, TEXT("����ʵ��������"), TEXT("��ʾ"), S_OK);
					  }
					  //����ʶ����Ϣ��ʹ�����ʱ�̼���������Ϣ
					  if (SUCCEEDED(hr))
					  {
						  hr = m_cpRecoCtxt->SetNotifyWindowMessage(hwnd, WM_RECOEVENT, 0, 0);
					  }
					  else
					  {
						  MessageBox(hwnd, TEXT("���������Ľӿڳ���"), TEXT("��ʾ"), S_OK);
					  }
					  //�������Ǹ���Ȥ���¼�
					  if (SUCCEEDED(hr))
					  {
						  ULONGLONG ullMyEvents = SPFEI(SPEI_SOUND_START) | SPFEI(SPEI_RECOGNITION) | SPFEI(SPEI_SOUND_END);
						  hr = m_cpRecoCtxt->SetInterest(ullMyEvents, ullMyEvents);
					  }
					  else
					  {
						  MessageBox(hwnd, TEXT("����ʶ����Ϣ����"), TEXT("��ʾ"), S_OK);
					  }
					  //�����﷨����
					  b_Cmd_Grammar = TRUE;
					  if (FAILED(hr))
					  {
						  MessageBox(hwnd, TEXT("�����﷨�������"), TEXT("��ʾ"), S_OK);
					  }
					  hr = m_cpRecoCtxt->CreateGrammar(GID_CMD_GR, &m_cpCmdGramma);
					  hr = m_cpCmdGramma->LoadCmdFromFile(L"cmd.xml", SPLO_DYNAMIC);
					  if (FAILED(hr))
					  {
						  MessageBox(hwnd, TEXT("�����ļ��򿪳���"), TEXT("��ʾ"), S_OK);
					  }
					  b_initSR = TRUE;
					  //�ڿ�ʼʶ��ʱ�������﷨����ʶ��
					  hr = m_cpCmdGramma->SetRuleState(NULL, NULL, SPRS_ACTIVE);
					  break;
	}
	case WM_RECOEVENT:
	{
						 RECT rect;
						 GetClientRect(hwnd, &rect);
						 hdc = GetDC(hwnd);
						 USES_CONVERSION;
						 CSpEvent event;
						 while (event.GetFrom(m_cpRecoCtxt) == S_OK)
						 {
							 switch (event.eEventId)
							 {
							 case SPEI_RECOGNITION:
							 {
													  static const WCHAR wszUnrecognized[] = L"<Unrecognized>";
													  CSpDynamicString dstrText;
													  //ȡ����Ϣ���
													  if (FAILED(event.RecoResult()->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, TRUE, &dstrText, NULL)))
													  {
														  dstrText = wszUnrecognized;
													  }
													  BSTR SRout;
													  dstrText.CopyToBSTR(&SRout);
													  char * lpszText2 = _com_util::ConvertBSTRToString(SRout);
													  if (b_Cmd_Grammar)
													  {
														  //MessageBoxA(0,lpszText2,"����",0);
														  if (strcmp("��ѶQQ", lpszText2) == 0)
														  {
															  //MessageBox(0,TEXT("�����"),TEXT("����"),0);
															  speak(L"�õ�");
															  //��QQ.exe
															  ShellExecuteA(NULL, "open", "D:\\QQ\\QQProtect\\Bin\\QQProtect.exe", 0, 0, 1);
														  }
														  if (strcmp("ȷ��", lpszText2) == 0)
														  {
															  //���»س���
															  keybd_event(VK_RETURN, 0, 0, 0);
															  keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);
														  }
														  if (strcmp("����", lpszText2) == 0)
														  {
															  speak(L"�õ�");
															  //����ϵͳ����wmplayer.exe��������
															  ShellExecuteA(NULL, "open", "\"C:\\Program Files (x86)\\Windows Media Player\\wmplayer.exe\"", "C:\\Users\\KYT\\Desktop\\123.mp3", 0, 0);
														  }
													  }
							 }
							 }
						 }
						 break;
	}
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

#pragma comment(lib,"ole32.lib")//CoInitialize CoCreateInstance ��Ҫ����ole32.dll

/*
*�����ϳɺ������ʶ��ַ���str
*/
int speak(wchar_t *str)
{
	ISpVoice * pVoice = NULL;
	::CoInitialize(NULL);
	//���ISpVoice�ӿ�
	long hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice);
	hr = pVoice->Speak(str, 0, NULL);
	pVoice->Release();
	pVoice = NULL;
	//ǧ��Ҫ����
	::CoUninitialize();
	return TRUE;
}