//------------------------------------------------------------------------------
// <copyright file="BodyBasics.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

#include "resource.h"
#include "mmsystem.h" //��������ͷ�ļ�
#include <queue>
#include <string>
#include <sapi.h> //��������ͷ�ļ�
#include <sphelper.h>//��������ʶ��ͷ�ļ�
#include <atlstr.h>


using namespace std;


#pragma comment(lib,"winmm.lib") //��������ͷ�ļ���
#pragma comment(lib,"sapi.lib") //��������ͷ�ļ���
#pragma once
const int WM_RECORD = WM_USER + 100;


class CBodyBasics
{


public:
    /// <summary>
    /// Constructor
    /// </summary>
    CBodyBasics();

    /// <summary>
    /// Destructor
    /// </summary>
    ~CBodyBasics();

	
	
    /// <summary>
    /// Handles window messages, passes most to the class instance to handle
    /// </summary>
    /// <param name="hWnd">window message is for</param>
    /// <param name="uMsg">message</param>
    /// <param name="wParam">message data</param>
    /// <param name="lParam">additional message data</param>
    /// <returns>result of message processing</returns>
    static LRESULT CALLBACK MessageRouter(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    /// <summary>
    /// Handle windows messages for a class instance
    /// </summary>
    /// <param name="hWnd">window message is for</param>
    /// <param name="uMsg">message</param>
    /// <param name="wParam">message data</param>
    /// <param name="lParam">additional message data</param>
    /// <returns>result of message processing</returns>
    LRESULT CALLBACK        DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    /// <summary>
    /// Creates the main window and begins processing
    /// </summary>
    /// <param name="hInstance"></param>
    /// <param name="nCmdShow"></param>
    int                     Run(HINSTANCE hInstance, int nCmdShow);

	
	CComPtr<ISpRecognizer>m_cpRecoEngine;// ����ʶ������(recognition)�Ľӿڡ�
	CComPtr<ISpRecoContext>m_cpRecoCtxt;// ʶ������������(context)�Ľӿڡ�
	CComPtr<ISpRecoGrammar>m_cpCmdGrammar;// ʶ���ķ�(grammar)�Ľӿڡ�
	CComPtr<ISpStream>m_cpInputStream;// ��()�Ľӿڡ�
	CComPtr<ISpObjectToken>m_cpToken;// ����������(token)�ӿڡ�
	CComPtr<ISpAudio>m_cpAudio;// ��Ƶ(Audio)�Ľӿڡ�(��������ԭ��Ĭ�ϵ�������)
	ULONGLONG  ullGrammerID;
	

	

private:
	HWND                    m_hWnd;


    bool                    m_bGotReco;//����ʶ�����
	void                    MSSSpeak(LPCTSTR speakContent);//����ת����
	void                    MSSListen();//����ת����

};

