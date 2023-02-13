// OnscreenKeyboardDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "OnscreenKeyboardDlg.h"
#include "afxdialogex.h"

#include <math.h>

// COnscreenKeyboardDlg 대화 상자입니다.
#define TIMER_ID 101
#define TAB 0x02
#define CAPSLOCK 0x03
#define SHIFT 0x04
#define CTRL 0x05
#define BSPACE 0x06
#define ENTER 0x07
#define ALT 0x08
#define WINDOW 0x9
#define LEFT 0x0A
#define RIGHT 0x0B
#define UP 0x0C
#define DOWN 0x0D
#define ESC 0x0E
#define PUP 0x0F
#define PDN 0x10
#define HOME 0x11
#define END 0x12
#define INS 0x13
#define DEL 0x14
#define WWW 0x15


IMPLEMENT_DYNAMIC(COnscreenKeyboardDlg, CDialogEx)

COnscreenKeyboardDlg::COnscreenKeyboardDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(COnscreenKeyboardDlg::IDD, pParent)
{
	m_bCreateKeyBoard=FALSE;
}

COnscreenKeyboardDlg::~COnscreenKeyboardDlg()
{
	POSITION pos = cKeys.GetHeadPosition();
	while(pos)
	{
		delete cKeys.GetNext(pos);
	}

	int state = DescribeKeyState();
	if(state & 0x01) keybd_event(VK_CAPITAL,0,KEYEVENTF_KEYUP,0);
	if(state & 0x02) keybd_event(VK_SHIFT,0,KEYEVENTF_KEYUP,0);
	if(state & 0x04) keybd_event(VK_CONTROL,0,KEYEVENTF_KEYUP,0);
}

void COnscreenKeyboardDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(COnscreenKeyboardDlg, CDialogEx)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEACTIVATE()
	ON_WM_TIMER()
	ON_WM_SYSCOMMAND()
	ON_WM_CTLCOLOR()
	ON_WM_MOVE()
END_MESSAGE_MAP()


// COnscreenKeyboardDlg 메시지 처리기입니다.
BOOL COnscreenKeyboardDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	InitKeyBoard();
	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void COnscreenKeyboardDlg::InitKeyBoard()
{
	if(m_bCreateKeyBoard) return;

	AddKey(0x00,0x00,0); //signal row change
	AddKey(0x00,ESC,10);
	for(int fnkey = 0x70; fnkey < 0x7C; fnkey++)	AddKey(0x00,fnkey & 0xFF,10);

	AddKey(0x00,INS,14);
	AddKey(0x00,DEL,-10);

	//first row
	AddKey(0x00,0x00,1); //signal row change
	AddKey('`','~',10);
	AddKey('1','!',10);
	AddKey('2','@',10);
	AddKey('3','#',10);
	AddKey('4','$',10);
	AddKey('5','%',10);
	AddKey('6','^',10);
	AddKey('7','&',10);
	AddKey('8','*',10);
	AddKey('9','(',10);
	AddKey('0',')',10);
	AddKey('-','_',10);
	AddKey('=','+',10);
	AddKey(0x00,BSPACE,14); //backspace
	AddKey(0x00,PUP,-12);

	//second row
	AddKey(0x00,0x00,2); //signal row change
	AddKey(0x00,TAB,14); //tab
	AddKey('q','Q',10);
	AddKey('w','W',10);
	AddKey('e','E',10);
	AddKey('r','R',10);
	AddKey('t','T',10);
	AddKey('y','Y',10);
	AddKey('u','U',10);
	AddKey('i','I',10);
	AddKey('o','O',10);
	AddKey('p','P',10);
	AddKey('[','{',10);
	AddKey(']','}',10);
	AddKey('\\','|',10);
	AddKey(0x00,PDN,-12);

	//third row
	AddKey(0x00,0x00,3); //signal row change
	AddKey(0x00,CAPSLOCK,18); //caps lock
	AddKey('a','A',10);
	AddKey('s','S',10);
	AddKey('d','D',10);
	AddKey('f','F',10);
	AddKey('g','G',10);
	AddKey('h','H',10);
	AddKey('j','J',10);
	AddKey('k','K',10);
	AddKey('l','L',10);
	AddKey(';',':',10);
	AddKey('\'','\"',10);
	AddKey(0x00,ENTER,16); //enter
	AddKey(0x00,HOME,-10);

	//fourth row
	AddKey(0x00,0x00,4); //signal row change
	AddKey(0x00,SHIFT,22); //shift
	AddKey('z','Z',10);
	AddKey('x','X',10);
	AddKey('c','C',10);
	AddKey('v','V',10);
	AddKey('b','B',10);
	AddKey('n','N',10);
	AddKey('m','M',10);
	AddKey(',','<',10);
	AddKey('.','>',10);
	AddKey('/','?',10);
	AddKey(0x00,SHIFT,22); //shift
	AddKey(0x00,END,-10);

	//fifth row
	AddKey(0x00,0x00,5); //signal row change
	AddKey(0x00,CTRL,12); //ctrl
	AddKey(0x00,WINDOW,12); //window
	AddKey(0x00,ALT,12); //alt
	AddKey(' ',' ',60);
	AddKey(0x00,LEFT,12); //left
	AddKey(0x00,UP,12); //up
	AddKey(0x00,DOWN,12); //down
	AddKey(0x00,RIGHT,12); //right
	AddKey(0x00,WWW,-10); //http://WWW

	CalcWidthHeight();
	
//	SetTimer(TIMER_ID,250,NULL);

	m_bCreateKeyBoard=TRUE;
}

void COnscreenKeyboardDlg::CalcWidthHeight()
{
	pKeyWidth = 0;
	pKeyHeight = 0;
	KEYDEF * key;
	POSITION pos = cKeys.GetHeadPosition();
	long totalWidth = 0;
	long longest = 0;
	long rows = 0;
	while(pos)
	{
		key = cKeys.GetNext(pos);
		if((key->cNormal == 0x00) && (key->cShifted == 0x00))
		{
			rows++;
			if(totalWidth > longest)
			{
				longest = totalWidth;
			}
			totalWidth = 0;
		}
		else
		{
			totalWidth += abs(key->cWidth);
		}
	}
	CRect rect;
	GetClientRect(rect);
	pKeyHeight = rect.Height() / rows;
	pKeyWidth = (int)(((double)rect.Width()) / ((((double)(longest)) / 10)));
}

void COnscreenKeyboardDlg::AddKey(	char pNormal,	char pShifted,	int pWidth)
{
	KEYDEF * key = new KEYDEF;
	key->cNormal = pNormal;
	key->cShifted = pShifted;
	key->cWidth = pWidth;
	cKeys.AddTail(key);
}

void COnscreenKeyboardDlg::DrawKey(CDC * dc, CRect & rc, KEYDEF * key, BOOL cHilight)
{
	if(!cHilight)
	{
		rc.DeflateRect(1,1);
		dc->FillSolidRect(rc,COLOR_UI_BODY);
		dc->Draw3dRect(rc,GetSysColor(COLOR_3DSHADOW),GetSysColor(COLOR_3DDKSHADOW));
		rc.DeflateRect(1,1);
		dc->Draw3dRect(rc,GetSysColor(COLOR_3DLIGHT),GetSysColor(COLOR_3DSHADOW));
	}
	else
	{
		rc.DeflateRect(1,1);
		dc->FillSolidRect(rc,COLOR_BTN_BODY);
		dc->Draw3dRect(rc,GetSysColor(COLOR_3DLIGHT),GetSysColor(COLOR_3DSHADOW));
		rc.DeflateRect(1,1);
		dc->Draw3dRect(rc,GetSysColor(COLOR_3DSHADOW),GetSysColor(COLOR_3DDKSHADOW));
	}

	CFont font;  
   font.CreateFont(20, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, "궁서체");
   CFont *pOldFont = dc->SelectObject(&font);

   CPen pen;
   pen.CreatePen( PS_SOLID, 2, RGB(255,255,255) );   
    CPen* oldPen = dc->SelectObject( &pen );  
	dc->SetTextColor(RGB(255,255,255));

	rc.DeflateRect(3,1);
	if(key->cNormal == 0x00)
	{
		CString label = " ";
		BOOL special = FALSE;
		if(key->cShifted > 0x6F) //is a function
		{
			int fkeynum = key->cShifted - 0x6F;
			label.Format("F%d",fkeynum);
		}
		else
		{
			switch(key->cShifted)
			{
			case TAB:    // 0x02
				label = "Tab->";
				break;
			case CAPSLOCK:    // 0x03
				label = "Caps";
				break;
			case SHIFT:    // 0x04
				label = "Shift";
				break;
			case CTRL:    // 0x05
				label = "Ctrl";
				break;
			case BSPACE:    // 0x06
				label = "<-Bspc";
				break;
			case ENTER:    // 0x07
				label = "Enter";
				break;
			case ALT:    // 0x08
				label = "Alt";
				break;
			case WINDOW:    // 0x9
				label = "Wnd";
				break;
			case ESC:
				label = "Esc";
				break;
			case LEFT:    // 0x0A
				special = TRUE;
				{
					int cx = (rc.left + rc.right)/2;
					int cy = (rc.top + rc.bottom)/2;
					int siz = rc.Width()/6;
					dc->MoveTo(cx+siz, cy);
					dc->LineTo(cx-siz, cy);
					dc->LineTo(cx-siz + (siz*4)/5, cy - siz/2);
					dc->MoveTo(cx-siz, cy);
					dc->LineTo(cx-siz + (siz*4)/5, cy + siz/2);
				}
				break;
			case RIGHT:    // 0x0B
				special = TRUE;
				{
					int cx = (rc.left + rc.right)/2;
					int cy = (rc.top + rc.bottom)/2;
					int siz = rc.Width()/6;
					dc->MoveTo(cx-siz, cy);
					dc->LineTo(cx+siz, cy);
					dc->LineTo(cx+siz - (siz*4)/5, cy - siz/2);
					dc->MoveTo(cx+siz, cy);
					dc->LineTo(cx+siz - (siz*4)/5, cy + siz/2);
				}
				break;
			case UP:    // 0x0C
				special = TRUE;
				{
					int cx = (rc.left + rc.right)/2;
					int cy = (rc.top + rc.bottom)/2;
					int siz = rc.Width()/6;
					dc->MoveTo(cx, cy+siz);
					dc->LineTo(cx, cy-siz);
					dc->LineTo(cx - siz/2, cy-siz + (siz*4)/5);
					dc->LineTo(cx, cy-siz);
					dc->LineTo(cx + siz/2, cy-siz + (siz*4)/5);
				}
				break;
			case DOWN:    // 0x0D
				special = TRUE;
				{
					int cx = (rc.left + rc.right)/2;
					int cy = (rc.top + rc.bottom)/2;
					int siz = rc.Width()/6;
					dc->MoveTo(cx, cy-siz);
					dc->LineTo(cx, cy+siz);
					dc->LineTo(cx - siz/2, cy+siz - (siz*4)/5);
					dc->LineTo(cx, cy+siz);
					dc->LineTo(cx + siz/2, cy+siz - (siz*4)/5);
				}
				break;
			case PUP: //0x0F
				label = "PgUp";
				break;
			case PDN: //0x10
				label = "PgDn";
				break;
			case HOME: //0x11
				label = "Home";
				break;
			case END: //0x12
				label = "End";
				break;
			case INS: //0x13
				label = "Ins";
				break;
			case DEL: //0x14
				label = "Del";
				break;
			case WWW:
				label = "WWW";
				break;
			default:
				ASSERT(FALSE);
				label = "#ERR#";
				break;
			}
		}
		if(!special)
		{
			CSize tsize;
			tsize = dc->GetTextExtent(label);
			dc->TextOut(
				((rc.left + rc.right)/2) - (tsize.cx/2), 
				((rc.top + rc.bottom)/2) - (tsize.cy/2), 
				label);
		}
	}
	else
	{
		if((key->cShifted >= 'A') && (key->cShifted <= 'Z'))
		{
			CSize tsize;
			tsize = dc->GetTextExtent((CString)key->cShifted);
			dc->TextOut(((rc.left + rc.right)/2) - (tsize.cx/2), ((rc.top + rc.bottom)/2) - (tsize.cy/2),(CString) key->cShifted);
		}
		else
		{
			CSize tsize;
			tsize = dc->GetTextExtent((CString)'M');
			dc->TextOut(rc.left, rc.top,(CString)key->cShifted);
			dc->TextOut(rc.right - tsize.cx, rc.bottom - tsize.cy,	(CString)key->cNormal);
		}
	}
	dc->SelectObject( oldPen );
	 dc->SelectObject(pOldFont);
   font.DeleteObject();
    pen.DeleteObject(); 
}


void COnscreenKeyboardDlg::SendKey(KEYDEF * keydef)
{
	if(keydef->cNormal == 0x00)
	{
		int vk;
		BOOL uptoo = TRUE;
		if(keydef->cShifted > 0x6F) //is a function key
		{
			vk = keydef->cShifted;
		}
		else
		{
			switch(keydef->cShifted)
			{
			case TAB:    // 0x02
				vk = VkKeyScan(0x09) & 0xFF;
				break;
			case CAPSLOCK:    // 0x03
				vk = VK_CAPITAL;
				break;
			case SHIFT:    // 0x04
				vk = VK_SHIFT;
				if(GetKeyState(vk) & 0xF000) return;
				uptoo = FALSE;
				break;
			case CTRL:    // 0x05
				vk = VK_CONTROL;
				if(GetKeyState(vk) & 0xF000) return;
				uptoo = FALSE;
				break;
			case BSPACE:    // 0x06
				vk = VK_BACK;
				break;
			case ENTER:    // 0x07
				vk = VkKeyScan(0x0D) & 0xFF;
				break;
			case ALT:    // 0x08
				vk = VK_MENU;
				break;
			case WINDOW:    // 0x9
				vk = VK_LWIN;
				break;
			case LEFT:    // 0x0A
				vk = VK_LEFT;
				break;
			case RIGHT:    // 0x0B
				vk = VK_RIGHT;
				break;
			case UP:    // 0x0C
				vk = VK_UP;
				break;
			case DOWN:    // 0x0D
				vk = VK_DOWN;
				break;
			case PUP: //0x0F
				vk = VK_PRIOR;
				break;
			case PDN: //0x10
				vk = VK_NEXT;
				break;
			case HOME: //0x11
				vk = VK_HOME;
				break;
			case END: //0x12
				vk = VK_END;
				break;
			case INS: //0x13
				vk = VK_INSERT;
				break;
			case DEL: //0x14
				vk = VK_DELETE;
				break;
			case WWW:
				KEYDEF key;
				key.cNormal = key.cShifted = 'H';
				SendKey(&key);
				key.cNormal = key.cShifted = 'T';
				SendKey(&key);
				SendKey(&key);
				key.cNormal = key.cShifted = 'P';
				SendKey(&key);
				key.cNormal = key.cShifted = ':';
				SendKey(&key);
				key.cNormal = key.cShifted = '/';
				SendKey(&key);
				SendKey(&key);
				key.cNormal = key.cShifted = 'W';
				SendKey(&key);
				SendKey(&key);
				SendKey(&key);
				vk = VkKeyScan('.') & 0xFF;
				break;
			case ESC:
				 vk = VK_ESCAPE;
				 break;
			default:
				ASSERT(FALSE);
				break;
			}
		}
		keybd_event(vk,0,0,0);
		if(uptoo)
			keybd_event(vk,0,KEYEVENTF_KEYUP,0);
	}
	else
	{
		char pChar = keydef->cNormal;
		SHORT ks = VkKeyScan(pChar);
		BYTE key = ks & 0xFF;

		keybd_event(key,0,0,0);
		keybd_event(key,0,KEYEVENTF_KEYUP,0);

		//turn off the control and shift if they were down
		unsigned char vk = VK_SHIFT;
		if(GetKeyState(vk) & 0xF000)
		{
			keybd_event(vk,0,KEYEVENTF_KEYUP,0);
		}
		vk = VK_CONTROL;
		if(GetKeyState(vk) & 0xF000) //allow toggle
			keybd_event(vk,0,KEYEVENTF_KEYUP,0);
	}
}

void COnscreenKeyboardDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	ReleaseFocus();

	CRect client;
	GetClientRect(client);
	int x = 0;
	int y = -pKeyHeight;
	KEYDEF * key;
	POSITION pos = cKeys.GetHeadPosition();
	while(pos)
	{
		key = cKeys.GetNext(pos);
		if((key->cNormal == 0x00) && (key->cShifted == 0x00))
		{
			x = 0;
			y += pKeyHeight;
		}
		else
		{
			int width = ((abs(key->cWidth) * pKeyWidth) / 10);
			CRect rc(x,y,x + width, y + pKeyHeight);
			if(key->cWidth < 0)		rc.right = client.right;

			x += width;
			if(rc.PtInRect(point))		
			{
				DrawKeyboard();
				CClientDC dc(this);
				rc.DeflateRect(1,1);
				dc.Draw3dRect(rc,RGB(255,0,0),RGB(255,0,0));
				SendKey(key);
			}
		}
	}
}

void COnscreenKeyboardDlg::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	OnLButtonDown(nFlags, point);
}

BOOL COnscreenKeyboardDlg::PreTranslateMessage(MSG* pMsg) 
{
	HWND wnd = ::GetForegroundWindow();
	if(IsWindow(wnd))
	{
		if(wnd != this->m_hWnd)
		{
			if(gFocus != wnd)
			{
				if(IsWindow(gFocus))
				{
					AttachThreadInput(GetWindowThreadProcessId(m_hWnd,NULL),GetWindowThreadProcessId(gFocus,NULL),	FALSE);
				}

				gFocus = wnd;
				AttachThreadInput(	GetWindowThreadProcessId(m_hWnd,NULL),	GetWindowThreadProcessId(gFocus,NULL),	TRUE);
			}
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

void COnscreenKeyboardDlg::OnRButtonDblClk(UINT nFlags, CPoint point) 
{
	OnLButtonDown(nFlags, point);
}

void COnscreenKeyboardDlg::OnRButtonDown(UINT nFlags, CPoint point) 
{
	OnLButtonDown(nFlags, point);
}

void COnscreenKeyboardDlg::OnRButtonUp(UINT nFlags, CPoint point) 
{
	ReleaseFocus();
}

BOOL COnscreenKeyboardDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	ReleaseFocus();
	return TRUE;
}

void COnscreenKeyboardDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	ReleaseFocus();
}

void COnscreenKeyboardDlg::ReleaseFocus() 
{
	if(IsWindow(gFocus))
	{
		HWND wnd = ::GetForegroundWindow();
		if(IsWindow(wnd))
		{
			if(wnd == gFocus)				return;
		}
		::SetForegroundWindow(gFocus);
		::SetFocus(gFocus);
	}
}

int COnscreenKeyboardDlg::DescribeKeyState() 
{
	int state = 0;
	short ks;
	ks = GetKeyState(VK_CAPITAL);
	if(ks & 0x000F) state += 0x01;
	ks = GetKeyState(VK_SHIFT);
	if(ks & 0xF000) state += 0x02;
	ks = GetKeyState(VK_CONTROL);
	if(ks & 0xF000) state += 0x04;
	return state;
}

void COnscreenKeyboardDlg::DrawKeyboard()
{
	CClientDC dc(this);

	CRect client;
	GetClientRect(client);
	CRgn clipRgn;
	clipRgn.CreateRectRgnIndirect(&client);
	dc.SelectClipRgn(&clipRgn);
	dc.SelectObject(GetStockObject(DEFAULT_GUI_FONT));
	dc.SetBkMode(TRANSPARENT);
	int x = 0;
	int y = -pKeyHeight;
	KEYDEF * key;
	POSITION pos = cKeys.GetHeadPosition();
	cCurrentKeyState = DescribeKeyState();
	int state = cCurrentKeyState;

	while(pos)
	{
		key = cKeys.GetNext(pos);
		if((key->cNormal == 0x00) && (key->cShifted == 0x00))
		{
			x = 0;
			y += pKeyHeight;
		}
		else
		{
			int width = ((abs(key->cWidth) * pKeyWidth) / 10);
			CRect rc(x,y,x + width, y + pKeyHeight);
			x += width;
			if(key->cWidth < 0)			rc.right = client.right; 

			BOOL hilight = FALSE;
			if(key->cNormal == 0x00)
			{
				switch(key->cShifted)
				{
				case CAPSLOCK:			if(state & 0x01)			hilight = TRUE;				break;
				case SHIFT:				if(state & 0x02)			hilight = TRUE;				break;
				case CTRL:				if(state & 0x04)			hilight = TRUE;				break;
				default:				break;
				}
			}
			DrawKey(&dc, rc, key, hilight);
		}
	}
}

void COnscreenKeyboardDlg::OnTimer(UINT_PTR nIDEvent) 
{
	if(nIDEvent == TIMER_ID)
	{
		int state = DescribeKeyState();
		DrawKeyboard();
	}
}


int COnscreenKeyboardDlg::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	return MA_NOACTIVATE;
	//return CDialog::OnMouseActivate(pDesktopWnd, nHitTest, message);
}



void COnscreenKeyboardDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if(nID==SC_CLOSE)
	{
		this->ShowWindow(SW_HIDE);
		return;
	}

	CDialogEx::OnSysCommand(nID, lParam);
}

HBRUSH COnscreenKeyboardDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{

	return m_hbrBkg;
}




void COnscreenKeyboardDlg::OnMove(int x, int y)
{
	CDialogEx::OnMove(x, y);

	DrawKeyboard();
}
