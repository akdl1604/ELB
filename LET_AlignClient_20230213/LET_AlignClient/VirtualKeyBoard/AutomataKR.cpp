#include "StdAfx.h"
#include "AutomataKR.h"

const int BASE_CODE = 0xac00;		// 奄段製失(亜)
const int LIMIT_MIN = 0xac00;		// 製失骨是 MIN(亜)
const int LIMIT_MAX = 0xd7a3;		// 製失骨是 MAX

// 製失 砺戚鷺
wchar_t SOUND_TABLE[68] =
{
	/* 段失 19切 0 ~ 18 */
	_T('ぁ'), _T('あ'), _T('い'), _T('ぇ'), _T('え'),
	_T('ぉ'), _T('け'), _T('げ'), _T('こ'), _T('さ'),
	_T('ざ'), _T('し'), _T('じ'), _T('す'), _T('ず'),
	_T('せ'), _T('ぜ'), _T('そ'), _T('ぞ'),
	/* 掻失 21切 19 ~ 39 */
	_T('た'), _T('だ'), _T('ち'), _T('ぢ'), _T('っ'),
	_T('つ'), _T('づ'), _T('て'), _T('で'), _T('と'),
	_T('ど'), _T('な'), _T('に'), _T('ぬ'), _T('ね'),
	_T('の'), _T('は'), _T('ば'), _T('ぱ'), _T('ひ'),
	_T('び'),
	/* 曽失 28切 40 ~ 67 */
	_T(' ') , _T('ぁ'), _T('あ'), _T('ぃ'), _T('い'),
	_T('ぅ'), _T('う'), _T('ぇ'), _T('ぉ'), _T('お'), 
	_T('か'), _T('が'), _T('き'), _T('ぎ'), _T('く'),
	_T('ぐ'), _T('け'), _T('げ'), _T('ご'), _T('さ'),
	_T('ざ'), _T('し'), _T('じ'), _T('ず'), _T('せ'),
	_T('ぜ'), _T('そ'), _T('ぞ')
};

// 段失 杯失 砺戚鷺
int MIXED_CHO_CONSON[14][3] =
{
	{ 0, 0,15}, // ぁ,ぁ,せ
	{15, 0, 1}, // せ,ぁ,あ
	{ 1, 0, 0}, // あ,ぁ,ぁ

	{ 3, 3,16}, // ぇ,ぇ,ぜ
	{16, 3, 4}, // ぜ,ぇ,え
	{ 4, 3, 3}, // え,ぇ,ぇ

	{ 7, 7,17}, // げ,げ,そ
	{17, 7, 8}, // そ,げ,こ
	{ 8, 7, 7}, // こ,げ,げ

	{ 9, 9,10}, // さ,さ,ざ
	{10, 9, 9}, // ざ,さ,さ

	{12,12,14}, // じ,じ,ず
	{14,12,13}, // ず,じ,す
	{13,12,12}  // す,じ,じ
};

// 段失,掻失 乞製 杯失 砺戚鷺
int MIXED_VOWEL[21][3] = 
{
	{19,19,21},	// た,た,ち
	{21,19,19},	// ち,た,た

	{19,39,20},	// た,び,だ
	{21,39,22},	// ち,び,ぢ

	{23,23,25},	// っ,っ,づ
	{25,23,23},	// づ,っ,っ

	{23,39,24},	// っ,び,つ
	{25,39,26},	// づ,び,て

	{27,27,31},	// で,で,に
	{31,27,27},	// に,で,で

	{27,19,28},	// で,た,と
	{28,39,29},	// と,び,ど

	{27,39,30},	// で,び,な

	{32,32,36},	// ぬ,ぬ,ば
	{36,32,32},	// ば,ぬ,ぬ

	{32,23,33},	// ぬ,っ,ね
	{33,39,34},	// ね,び,の

	{32,39,35},	// ぬ,び,は

	{39,39,37},	// び,び,ぱ
	{37,39,38},	// ぱ,び,ひ
	{38,39,39}	// ひ,び,び
};

// 曽失 杯失 砺戚鷺
int MIXED_JONG_CONSON[22][3] = 
{
	{41,41,64}, // ぁ,ぁ,せ
	{64,41,42}, // せ,ぁ,あ
	{42,41,41}, // あ,ぁ,ぁ

	{41,59,43}, // ぁ,さ,ぃ

	{44,62,45}, // い,じ,ぅ
	{44,67,46}, // い,ぞ,う

	{47,47,65}, // ぇ,ぇ,ぜ
	{65,47,47}, // ぜ,ぇ,ぇ

	{48,41,49}, // ぉ,ぁ,お
	{48,56,50}, // ぉ,け,か

	{48,57,51}, // ぉ,げ,が
	{51,57,54}, // が,げ,く

	{48,59,52}, // ぉ,さ,き
	{48,47,53}, // ぉ,ぇ,ぎ	
	{48,67,55}, // ぉ,ぞ,ぐ

	{57,57,66}, // げ,げ,そ
	{66,57,57}, // そ,げ,げ

	{57,59,58}, // げ,さ,ご

	{59,59,60}, // さ,さ,ざ
	{60,59,59}, // ざ,さ,さ

	{62,62,63}, // じ,じ,ず
	{63,62,62}  // ず,じ,じ
};

// 曽失 歳背 砺戚鷺
int DIVIDE_JONG_CONSON[13][3] = 
{
	{41,41,42}, // ぁ,ぁ,あ
	{41,59,43}, // ぁ,さ,ぃ
	{44,62,45}, // い,じ,ぅ
	{44,67,46}, // い,ぞ,う
	{48,41,49}, // ぉ,ぁ,お
	{48,56,50}, // ぉ,け,か
	{48,57,51}, // ぉ,げ,が
	{48,66,54}, // ぉ,そ,く
	{48,59,52}, // ぉ,さ,き
	{48,65,53}, // ぉ,ぜ,ぎ	
	{48,67,55}, // ぉ,ぞ,ぐ
	{57,59,58}, // げ,さ,ご
	{59,59,60}  // さ,さ,ざ
};

CAutomataKR::CAutomataKR(void)
{
	Clear();
}

CAutomataKR::~CAutomataKR(void)
{
}

// 獄遁 段奄鉢
void CAutomataKR::Clear()
{
	m_nStatus		= HS_FIRST;
	completeText	= _T("");
	ingWord			= NULL;
	m_completeWord	= NULL;
}

// 徹坪球 脊径 貢 繕杯 (舛背遭 int 坪球葵聖 脊径 閤焼 廃越繕杯)
wchar_t CAutomataKR::SetKeyCode(int nKeyCode)
{
	m_completeWord = NULL;

	// 働呪徹 脊径
	if(nKeyCode < 0)
	{
		m_nStatus = HS_FIRST;

		if(nKeyCode == KEY_CODE_SPACE) // 句嬢床奄
		{
			if(ingWord != NULL)
				completeText += ingWord;
			else
				completeText += _T(' ');

			ingWord = NULL;
		}
		else if(nKeyCode == KEY_CODE_ENTER) // 鎧形床奄
		{
			if(ingWord != NULL)
				completeText += ingWord;

			completeText += _T("\r\n");

			ingWord = NULL;
		}
		else if(nKeyCode == KEY_CODE_BACKSPACE) // 走酔奄
		{
			if(ingWord == NULL)
			{
				if(completeText.GetLength() > 0)
				{
					if(completeText.Right(1) == _T("\n"))
						completeText = completeText.Left(completeText.GetLength() -2);
					else
						completeText = completeText.Left(completeText.GetLength() -1);
				}
			}
			else
			{
				m_nStatus = DownGradeIngWordStatus(ingWord);
			}
		}

		return m_completeWord;
	}



	switch(m_nStatus)
	{
	case HS_FIRST:
		// 段失
		m_nPhonemez[0]	= nKeyCode;
		ingWord			= SOUND_TABLE[m_nPhonemez[0]];
		m_nStatus		= nKeyCode > 18 ? HS_FIRST_C : HS_FIRST_V;
		break;

	case HS_FIRST_C:
		// 乞製 + 乞製
		if(nKeyCode > 18)	// 乞製
		{
			if(MixVowel(&m_nPhonemez[0], nKeyCode) == FALSE)
			{
				m_completeWord = SOUND_TABLE[m_nPhonemez[0]];
				m_nPhonemez[0] = nKeyCode;
			}
		}
		else				// 切製
		{
			m_completeWord	= SOUND_TABLE[m_nPhonemez[0]];
			m_nPhonemez[0]	= nKeyCode;
			m_nStatus		= HS_FIRST_V;
		}
		break;

	case HS_FIRST_V:
		// 切製 + 切製
		if(nKeyCode > 18)	// 乞製
		{
			m_nPhonemez[1]	= nKeyCode;
			m_nStatus		= HS_MIDDLE_STATE;
		}
		else				// 切製
		{
			if(!MixInitial(nKeyCode))
			{
				m_completeWord	= SOUND_TABLE[m_nPhonemez[0]];
				m_nPhonemez[0]	= nKeyCode;
				m_nStatus		= HS_FIRST_V;
			}
		}
		break;

	case HS_MIDDLE_STATE:
		// 段失 + 乞製 + 乞製
		if(nKeyCode > 18)
		{
			if(MixVowel(&m_nPhonemez[1], nKeyCode) == FALSE)
			{
				m_completeWord	= CombineHangle(1);
				m_nPhonemez[0]	= nKeyCode;
				m_nStatus		= HS_FIRST_C;
			}
		}
		else
		{
			int jungCode = ToFinal(nKeyCode);

			if(jungCode > 0)
			{
				m_nPhonemez[2]	= jungCode;
				m_nStatus		= HS_END_STATE;
			}
			else
			{
				m_completeWord	= CombineHangle(1);
				m_nPhonemez[0]	= nKeyCode;
				m_nStatus		= HS_FIRST;
			}
		}
		break;

	case HS_END:
		// 段失 + 掻失 + 曽失
		if(nKeyCode > 18)  
		{
			m_completeWord	= CombineHangle(1);
			m_nPhonemez[0]	= nKeyCode;
			m_nStatus		= HS_FIRST;
		}
		else
		{
			int jungCode = ToFinal(nKeyCode);
			if(jungCode > 0)
			{
				m_nPhonemez[2]	= jungCode;
				m_nStatus		= HS_END_STATE;
			}
			else
			{
				m_completeWord	= CombineHangle(1);
				m_nPhonemez[0]	= nKeyCode;
				m_nStatus		= HS_FIRST;
			}  
		}
		break;

	case HS_END_STATE:
		// 段失 + 掻失 + 切製(曽) + 切製(曽)
		if(nKeyCode > 18)
		{
			m_completeWord = CombineHangle(1);

			m_nPhonemez[0]	= ToInitial(m_nPhonemez[2]);
			m_nPhonemez[1]	= nKeyCode;
			m_nStatus		= HS_MIDDLE_STATE;
		}
		else
		{
			int jungCode = ToFinal(nKeyCode);
			if(jungCode > 0)
			{
				m_nStatus = HS_END_EXCEPTION;

				if(!MixFinal(jungCode))
				{
					m_completeWord	= CombineHangle(2);
					m_nPhonemez[0]	= nKeyCode;
					m_nStatus		= HS_FIRST_V;
				}
			}
			else
			{
				m_completeWord	= CombineHangle(2);
				m_nPhonemez[0]	= nKeyCode;
				m_nStatus		= HS_FIRST_V;
			}  
		}
		break;

	case HS_END_EXCEPTION:
		// 段失 + 掻失 + 曽失(亥切製)
		if(nKeyCode > 18)  
		{
			DecomposeConsonant();
			m_nPhonemez[1]	= nKeyCode;
			m_nStatus		= HS_MIDDLE_STATE;
		}
		else
		{
			int jungCode = ToFinal(nKeyCode);
			if(jungCode > 0)
			{
				m_nStatus = HS_END_EXCEPTION;

				if(!MixFinal(jungCode))
				{
					m_completeWord	= CombineHangle(2);
					m_nPhonemez[0]	= nKeyCode;
					m_nStatus		= HS_FIRST_V;
				}
			}
			else
			{
				m_completeWord	= CombineHangle(2);
				m_nPhonemez[0]	= nKeyCode;
				m_nStatus		= HS_FIRST_V; 
			}
		}
		break;
	}

	// 薄仙 左戚澗 越切雌殿
	CombineIngWord(m_nStatus);

	// 刃失 庚切伸 幻級奄
	if(m_completeWord != NULL) 
		completeText += TCHAR(m_completeWord);

	return m_completeWord;
}

// 段失生稽 痕発
int CAutomataKR::ToInitial(int nKeyCode)
{
	switch(nKeyCode)
	{
	case 41: return 0;	// ぁ
	case 42: return 1;	// あ
	case 44: return 2;	// い
	case 47: return 3;	// ぇ
	case 48: return 5;	// ぉ
	case 56: return 6;	// け
	case 57: return 7;	// げ
	case 59: return 9;	// さ
	case 60: return 10;	// ざ
	case 61: return 11;	// し
	case 62: return 12;	// じ
	case 63: return 14;	// ず
	case 64: return 15;	// せ
	case 65: return 16;	// ぜ
	case 66: return 17;	// そ
	case 67: return 18;	// ぞ
	}
	return -1;
}

// 曽失生稽 痕発
int CAutomataKR::ToFinal(int nKeyCode)
{
	switch(nKeyCode)
	{
	case 0: return 41;	// ぁ
	case 1: return 42;	// あ
	case 2: return 44;	// い
	case 3: return 47;	// ぇ
	case 5: return 48;	// ぉ
	case 6: return 56;	// け
	case 7: return 57;	// げ
	case 9: return 59;	// さ
	case 10: return 60;	// ざ
	case 11: return 61;	// し
	case 12: return 62;	// じ
	case 14: return 63;	// ず
	case 15: return 64;	// せ
	case 16: return 65;	// ぜ
	case 17: return 66;	// そ
	case 18: return 67;	// ぞ
	}
	return -1;
}

// 切製歳背
void CAutomataKR::DecomposeConsonant()
{
	int i = 0;
	if(m_nPhonemez[3] > 40 || m_nPhonemez[4] > 40)
	{
		do
		{
			if(DIVIDE_JONG_CONSON[i][2] == m_nPhonemez[2])
			{
				m_nPhonemez[3] = DIVIDE_JONG_CONSON[i][0];
				m_nPhonemez[4] = DIVIDE_JONG_CONSON[i][1];

				m_completeWord = CombineHangle(3);
				m_nPhonemez[0]	 = ToInitial(m_nPhonemez[4]);
				return;
			}
		}
		while(++i< 13);
	}

	m_completeWord = CombineHangle(1);
	m_nPhonemez[0]	 = ToInitial(m_nPhonemez[2]);
}

// 段失杯失
BOOL CAutomataKR::MixInitial(int nKeyCode)
{
	if(nKeyCode >= 19)
		return FALSE;

	int i = 0;
	do
	{
		if(MIXED_CHO_CONSON[i][0] == m_nPhonemez[0] && MIXED_CHO_CONSON[i][1] == nKeyCode)
		{
			m_nPhonemez[0] = MIXED_CHO_CONSON[i][2];
			return TRUE;
		}
	}
	while(++i < 14);

	return FALSE;
}

// 曽失杯失
BOOL CAutomataKR::MixFinal(int nKeyCode)
{
	if(nKeyCode <= 40) return FALSE;

	int i = 0;
	do
	{
		if(MIXED_JONG_CONSON[i][0] == m_nPhonemez[2] && MIXED_JONG_CONSON[i][1] == nKeyCode)
		{
			m_nPhonemez[3] = m_nPhonemez[2];
			m_nPhonemez[4] = nKeyCode;
			m_nPhonemez[2] = MIXED_JONG_CONSON[i][2];

			return TRUE;
		}
	}
	while(++i < 22);

	return FALSE;
}

// 乞製杯失
BOOL CAutomataKR::MixVowel(int * currentCode, int inputCode)
{
	int i = 0;
	do
	{
		if(MIXED_VOWEL[i][0] == * currentCode && MIXED_VOWEL[i][1] == inputCode)
		{
			* currentCode = MIXED_VOWEL[i][2];
			return TRUE;
		}
	}
	while(++i< 21);

	return FALSE;
}

// 廃越繕杯
wchar_t CAutomataKR::CombineHangle(int cho, int jung, int jong)
{
	// 段失 * 21 * 28 + (掻失 - 19) * 28 + (曽失 - 40) + BASE_CODE;
	return BASE_CODE - 572 + jong + cho * 588 + jung * 28;
}

wchar_t CAutomataKR::CombineHangle(int status)
{
	switch(status)
	{
	//段失 + 掻失
	case 1: return CombineHangle(m_nPhonemez[0], m_nPhonemez[1], 40);

	//段失 + 掻失 + 曽失
	case 2: return CombineHangle(m_nPhonemez[0], m_nPhonemez[1], m_nPhonemez[2]);
	
	//段失 + 掻失 + 亥切製01
	case 3: return CombineHangle(m_nPhonemez[0], m_nPhonemez[1], m_nPhonemez[3]);
	}
	return ' ';
}

void CAutomataKR::CombineIngWord(int status)
{
	switch(m_nStatus)
	{
	case HS_FIRST:
	case HS_FIRST_V:
	case HS_FIRST_C:
		ingWord = SOUND_TABLE[m_nPhonemez[0]];
		break;

	case HS_MIDDLE_STATE:
	case HS_END:
		ingWord = CombineHangle(1);
		break;

	case HS_END_STATE:
	case HS_END_EXCEPTION:
		ingWord = CombineHangle(2);
		break;
	}
}

int CAutomataKR::DownGradeIngWordStatus(wchar_t word)
{
	int iWord = word;

	//段失幻 赤澗 井酔
	if(iWord < LIMIT_MIN || iWord > LIMIT_MAX)
	{
		ingWord = NULL;

		return HS_FIRST;
	}

	//庚切坪球 端域
	//iWord = firstWord * (21*28)
	//		+ middleWord * 28
	//		+ lastWord * 27
	//		+ BASE_CODE;
	//
	int totalWord	= iWord - BASE_CODE;
	int iFirstWord	= totalWord / 28 / 21;	//段失
	int iMiddleWord = totalWord / 28 % 21;	//掻失
	int iLastWord	= totalWord % 28;		//曽失

	m_nPhonemez[0] = iFirstWord; //段失煽舌

	if(iLastWord == 0)	//曽失戚 蒸澗 井酔
	{
		ingWord = SOUND_TABLE[m_nPhonemez[0]];

		return HS_FIRST_V;
	}

	m_nPhonemez[1] = iMiddleWord + 19; //掻失煽舌

	iLastWord += 40;

	for(int i = 0; i < 13; i++)
	{
		if(iLastWord == DIVIDE_JONG_CONSON[i][2])
		{
			ingWord = CombineHangle(3);
			m_nPhonemez[2] = DIVIDE_JONG_CONSON[i][0]; // 曽失煽舌
			return HS_END_EXCEPTION;
		}
	}

	ingWord = CombineHangle(1);

	return HS_MIDDLE_STATE;
}
