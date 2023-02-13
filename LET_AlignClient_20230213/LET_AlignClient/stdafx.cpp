
// stdafx.cpp : 표준 포함 파일만 들어 있는 소스 파일입니다.
// LET_AlignClient.pch는 미리 컴파일된 헤더가 됩니다.
// stdafx.obj에는 미리 컴파일된 형식 정보가 포함됩니다.

#include "stdafx.h"
#include "UI_Control/Graphics/ImageViewer.h"
#include "resource.h"
std::vector<ImageViewer*> g_ImageWindows;

int ProcessMessages(void)
{
	MSG msg;
	while (::PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
	return 0;
}

int Delay( int ms )
{
	int nStick = GetTickCount();
	while (GetTickCount() - nStick < DWORD(ms)) {
		ProcessMessages();
		Sleep(1);
	}
	return 0;
}


std::wstring towstring(const char *src)
{
    std::wstring output;
    int src_len = (int)strlen(src);
    if (src_len > 0)
    {
        int out_len = MultiByteToWideChar(CP_ACP, 0, src, src_len, NULL, 0);
        if (out_len > 0)
        {
            output.resize(out_len * 2);
            MultiByteToWideChar(CP_ACP, 0, src, src_len, &output[0], out_len);
        }
    }
    return output;
}


DWORD WaitForMultiFlag(BOOL *pWaitFlag, float waitTime, int nFlagCount)
{
	BOOL bWaitComplete = TRUE;
	clock_t start_time, end_time;
	float elapsed_time;
	waitTime /= 1000.0;
	MSG message;
	DWORD ret = WAIT_OBJECT_0;

	start_time = clock();
	do	{
		end_time = clock();
		elapsed_time = (end_time - start_time) / (float)CLOCKS_PER_SEC;

		bWaitComplete = TRUE;

		for(int i = 0; i < nFlagCount; i++)
		{
			if( pWaitFlag[i] != TRUE )
				bWaitComplete = FALSE;
		}

		if( bWaitComplete )	break;

		if (elapsed_time > waitTime)
		{
			ret = WAIT_TIMEOUT;
			break;
		}

		Sleep(1);

		if(::PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
		{	
			::TranslateMessage(&message);
			::DispatchMessage(&message);
		}
	} while( 1 );

	ret = WAIT_TIMEOUT;
	return ret;
}

int TW_SendDataMessage(HWND parm_wnd, HWND parm_this_wnd, int parm_message_type,char* parm_data, int parm_size)
{
	int return_value = 0;
	char* p_buffer = NULL;

	if (parm_wnd != NULL) {
		if (parm_size) {
			p_buffer = (char*)::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, parm_size);
			if (p_buffer != NULL) memcpy(p_buffer, parm_data, parm_size);
		}

		COPYDATASTRUCT tip;
		tip.dwData = parm_message_type;
		tip.cbData = parm_size;
		tip.lpData = p_buffer;
		if (IsWindow(parm_wnd)) return_value = ::SendMessage(parm_wnd, WM_COPYDATA,	(WPARAM)parm_this_wnd, (LPARAM)&tip);
		if (p_buffer != NULL) ::HeapFree(::GetProcessHeap(), 0, p_buffer);
	}
	return return_value;
}

//#define MM_TEST_DATA 200
//char test_data[64] = { 0 };
//strcpy(test_data, "SendMessage");
//
//
//HWND dest_wnd = ::FindWindow(NULL, "LET_AlignClient");
//if(dest_wnd!=NULL) TW_SendDataMessage(dest_wnd, this->m_hWnd, MM_TEST_DATA, test_data,	sizeof(test_data));

void ShowImage(CString strTitle, const cv::Mat& src)
{
	CString str;
	bool existed = false;
	for (int i = 0; i < g_ImageWindows.size(); i++)
	{
		str = g_ImageWindows[i]->GetTitle();
		if (str == strTitle)
		{
			g_ImageWindows[i]->SetImage(src);
			g_ImageWindows[i]->ShowWindow(SW_SHOW);
			existed = true;
			break;
		}
	}

	if (!existed)
	{
		auto view = new ImageViewer(NULL, true);
		view->Create(IDD_IMAGE_VIEWER);
		view->SetTitle(strTitle);
		view->SetImage(src);
		view->OnClosing = [&](CDialog* sender)
		{
			int id = -1;
			CString title, title1;
			auto view = (ImageViewer*)sender;
			title = view->GetTitle();

			for (int i = 0; i < g_ImageWindows.size(); i++)
			{
				title1 = g_ImageWindows[i]->GetTitle();
				if (title == title1)
				{
					id = i;
					break;
				}
			}
			if (id > -1)g_ImageWindows.erase(g_ImageWindows.begin() + id);
		};
		g_ImageWindows.push_back(view);
		view->ShowWindow(SW_SHOW);
	}
}

void HideImage(CString strTitle)
{
	CString str;
	bool existed = false;
	int id = 0;
	for (int i = 0; i < g_ImageWindows.size(); i++)
	{
		str = g_ImageWindows[i]->GetTitle();
		if (str == strTitle)
		{
			g_ImageWindows[i]->ShowWindow(SW_HIDE);
			existed = true;
			break;
		}
	}
}

int LastIndexOf(const CString& s1, const CString& s2)
{
	int start = s1.Find(s2, 0);

	if (start >= 0)
	{
		while (start < s1.GetLength())
		{
			int idx = s1.Find(s2, start + 1);
			if (idx >= 0)
				start = idx;
			else
				break;
		}
	}

	return start;
}