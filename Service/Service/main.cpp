#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "winsock2.h"
#include<cstdlib>
#pragma comment(lib,"ws2_32.lib")//���ÿ��ļ�
using namespace std;


char recvBuf[100];
SOCKET sockConn;
/**
* ��һ���µ��߳������������
*/
DWORD WINAPI Fun(LPVOID lpParamter)
{
	while (true){
		memset(recvBuf, 0, sizeof(recvBuf));
		//      //��������
		recv(sockConn, recvBuf, sizeof(recvBuf), 0);
		printf("%s\n", recvBuf);
	}
	closesocket(sockConn);
}

int main()
{
	WSADATA wsaData;
	int port = 8080;//�˿ں�
	// �����׽��ֿ�
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("��ʼ��ʧ��");
		return 0;
	}
	// �������ڼ������׽���,������˵��׽���
	SOCKET sockSrv = socket(AF_INET, SOCK_STREAM, 0);
	// ���÷�������ַ
	SOCKADDR_IN addrSrv;
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(port); //1024���ϵĶ˿ں�
	addrSrv.sin_addr.S_un.S_addr = inet_addr("192.168.137.1");
	// ��
	int retVal = bind(sockSrv, (LPSOCKADDR)&addrSrv, sizeof(SOCKADDR_IN));
	if (retVal == SOCKET_ERROR){
		printf("����ʧ��:%d\n", WSAGetLastError());
		return 0;
	}
	// ����
	if (listen(sockSrv, 10) == SOCKET_ERROR){
		printf("����ʧ��:%d", WSAGetLastError());
		return 0;
	}

	SOCKADDR_IN addrClient;
	int len = sizeof(SOCKADDR);

	while (1)
	{
		//�ȴ��ͻ�������
		sockConn = accept(sockSrv, (SOCKADDR *)&addrClient, &len);
		if (sockConn == SOCKET_ERROR){
			printf("�ȴ�����ʧ��:%d", WSAGetLastError());
			break;
		}
		printf("�ͻ��˵�IP��:[%s]\n", inet_ntoa(addrClient.sin_addr));

		//��������
		char sendbuf[] = "�����IP:192.168.137.1 Port:8080";

		int iSend = send(sockConn, sendbuf, sizeof(sendbuf), 0);
		if (iSend == SOCKET_ERROR){
			printf("����ʧ��");
			break;
		}

		HANDLE hThread = CreateThread(NULL, 0, Fun, NULL, 0, NULL);
		CloseHandle(hThread);
	}

	closesocket(sockSrv);
	WSACleanup();
	system("pause");
	return 0;
}