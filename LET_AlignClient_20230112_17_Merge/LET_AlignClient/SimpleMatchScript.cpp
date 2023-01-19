#include "stdafx.h"
#include "SimpleMatchScript.h"
#include "LET_AlignClient.h"

namespace
{
	static const char* g_numStr = "1234567890.";
	static const int g_numLen = 11;
	static const char* g_strStr = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@$_/.";
	static const int g_strLen = 57;
	static const char* g_strStr2 = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@$_/.1234567890";
	static const int g_strLen2 = 67;
	static const char* g_strOp = "+-*/=:()";
	static const int g_opLen = 7;
}

enum Direction {
	ShiftUp = 1, ShiftRight, ShiftDown, ShiftLeft
};

// elements를 회전해서 제거한다.
template <class Seq>
static void rotatepopvector(Seq& seq, const unsigned int idx)
{
	if ((seq.size() - 1) > idx)
		std::rotate(seq.begin() + idx, seq.begin() + idx + 1, seq.end());
	seq.pop_back();
}

//------------------------------------------------------------------------
// 유니코드를 멀티바이트 문자로 변환
//------------------------------------------------------------------------
static std::string wstr2str(const std::wstring& wstr)
{
	const int slength = (int)wstr.length() + 1;
	const int len = ::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), slength, 0, 0, NULL, FALSE);
	char* buf = new char[len];
	::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), slength, buf, len, NULL, FALSE);
	std::string r(buf);
	delete[] buf;
	return r;
}

//------------------------------------------------------------------------
// 멀티바이트 문자를 유니코드로 변환
//------------------------------------------------------------------------
static std::wstring str2wstr(const std::string& str)
{
	int len;
	int slength = (int)str.length() + 1;
	len = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	::MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

// "  skdfjskdjf  "
// "$$$skdfjskdjf$$$"
// "skdfjskdjf"
static std::string& trim(std::string& str)
{
	// 앞에서부터 검색
	for (int i = 0; i < (int)str.length(); ++i)
	{
		if ((str[i] == '\n') || (str[i] == '\t') || (str[i] == '\r') || (str[i] == ' '))
		{
			rotatepopvector(str, i);
			//			str[ i] = '$';
			--i;
		}
		else
			break;
	}

	// 뒤에서부터 검색
	for (int i = (int)str.length() - 1; i >= 0; --i)
	{
		if ((str[i] == '\n') || (str[i] == '\t') || (str[i] == '\r') || (str[i] == ' '))
		{
			rotatepopvector(str, i);
			//			str[ i] = '$';
		}
		else
			break;
	}

	//	replaceAll(str, "$", "");
	return str;
}


// "  skdfjskdjf  "
// "$$$skdfjskdjf$$$"
// "skdfjskdjf"
static void trimw(std::wstring& str)
{
	str = str2wstr(trim(wstr2str(str)));
}

//------------------------------------------------------------------------
// 스트링포맷
//------------------------------------------------------------------------
static std::string format(const char* fmt, ...)
{
	char textString[256] = { '\0' };
	va_list args;
	va_start(args, fmt);
	vsnprintf_s(textString, sizeof(textString), _TRUNCATE, fmt, args);
	va_end(args);
	return textString;
}


//------------------------------------------------------------------------
// 스트링포맷 wstring 용
//------------------------------------------------------------------------
static std::wstring formatw(const char* fmt, ...)
{
	char textString[256] = { '\0' };
	va_list args;
	va_start(args, fmt);
	vsnprintf_s(textString, sizeof(textString), _TRUNCATE, fmt, args);
	va_end(args);
	return str2wstr(textString);
}

CSimpleMatchScript::CSimpleMatchScript()
	: m_state(WAIT)
	, m_curIdx(0)
{
}

CSimpleMatchScript::~CSimpleMatchScript()
{
	m_commands.clear();
}


// parse id : pid
// id -> alphabet + {alphabet | number}
std::string CSimpleMatchScript::pid(const std::string& str)
{
	if (str.empty())
		return "";

	std::string out;
	out.reserve(64);
	int i = 0;

	const char* n = strchr(g_strStr, str[i]);
	if (!n)
		return out;

	out += str[i++];

	while (1)
	{
		const char* n = strchr(g_strStr2, str[i]);
		if (!n || !*n)
			break;
		out += str[i++];
	}

	return out;
}


// aaa, bb, cc, "dd ee"  ff -> aaa, bb, cc, dd ee
int CSimpleMatchScript::attrs(const std::string & str, OUT std::string & out)
{
	int i = 0;

	out.reserve(64);

	bool isLoop = true;
	bool isComma = false;
	bool isString = false;
	bool isFirst = true;
	while (isLoop && str[i])
	{
		switch (str[i])
		{
		case '"':
			isFirst = false;
			isString = !isString;
			break;
		case ',': // comma
			if (isString)
			{
				out += ',';
			}
			else
			{
				isComma = true;
				out += ',';
			}
			break;

		case '\r':
		case '\n':
		case ' ': // space
			if (isFirst)
			{
				// nothing~
			}
			else if (isString)
			{
				out += ',';
			}
			else
			{
				if (!isComma)
					isLoop = false;
			}
			break;
		default:
			isFirst = false;
			isComma = false;
			out += str[i];
			break;
		}
		++i;
	}

	return i;
}


// attr - list ->  { id [ = value ] }
void CSimpleMatchScript::attr_list(const std::string & str)
{
	std::string parseStr = str;
	while (1)
	{
		trim(parseStr);

		std::string id = pid(parseStr);
		trim(id);
		if (id.empty())
			break;
		parseStr = parseStr.substr(id.size());
		trim(parseStr);

		unsigned int pos1 = (unsigned int)parseStr.find("=");
		unsigned int pos2 = (unsigned int)MIN(parseStr.find(" "), parseStr.find("\n"));

		if (pos1==0 && pos1 < pos2) // id = data
		{
			parseStr = parseStr.substr(pos1 + 1);
			trim(parseStr);

			// aaa, bb, cc  dd -> aaa, bb, cc 
			std::string data;
			const int offset = attrs(parseStr, data);
			parseStr = parseStr.substr(offset);

			m_commands.push_back(std::pair<std::string, std::string>(id, data));
		}
		else // id or eof
		{
			m_commands.push_back(std::pair<std::string, std::string>(id, ""));

			if (parseStr.empty() && (pos2 == std::string::npos))
				break;
		}
	}
}


// parse grammar
// var = parameter1, parameter2, parameter3
void CSimpleMatchScript::Parse(const std::string & script)
{
	m_commands.clear();
	attr_list(script);
}


std::string CSimpleMatchScript::Match(INOUT cv::Mat & src, OUT cv::Mat & dst, const std::string & script
	, const bool showMsg)
	//showMsg=true
{
	m_src = src.clone(); // 결과 정보를 출력할 때, 쓰임
	dst = src.clone();
	m_tessImg = cv::Mat();

	vector<cv::Mat> accMat;
	accMat.reserve(8);

	Parse(script);

	unsigned long t1 = GetTickCount64();

	std::string errMsg;
	char value[128];
	for each (auto & item in m_commands)
	{
		const std::string cmd = item.first;
		const std::string param = item.second;

		//----------------------------------------------------------------------
		// roi=x,y,width,height
		if (cmd == "roi")
		{
			cv::Rect roi = { 0,0,0,0 };
			sscanf(param.c_str(), "%d,%d,%d,%d", &roi.x, &roi.y, &roi.width, &roi.height);
			if (roi.area() > 0)
			{
				if (dst.data)
					dst = dst(roi);
				continue;
			}
		}
		if (cmd == "roiCopy")
		{
			cv::Rect roi = { 0,0,0,0 };
			sscanf(param.c_str(), "%d,%d,%d,%d", &roi.x, &roi.y, &roi.width, &roi.height);
			if (roi.area() > 0)
			{
				if (dst.data)
				{
					dst.copyTo(m_src(roi));
				}
				dst=m_src;
				continue;
			}
		}
		//----------------------------------------------------------------------
		// rect=x,y,width,height
		if (cmd == "rect")
		{
			cv::Rect rect = { 0,0,0,0 };
			sscanf(param.c_str(), "%d,%d,%d,%d", &rect.x, &rect.y, &rect.width, &rect.height);
			if (rect.area() > 0)
			{
				cRectContour r(rect);
				r.Draw(dst, cv::Scalar(0, 0, 255), 2);
				continue;
			}
		}

		//----------------------------------------------------------------------
		//     - bgr=num1,num2,num3
		//         - Mat &= Scalar(num1,num2,num3)
		if (cmd == "bgr")
		{
			int bgr[3] = { 0,0,0 };
			sscanf(param.c_str(), "%d,%d,%d", bgr, bgr + 1, bgr + 2);
			if ((bgr[0] != 0) || (bgr[1] != 0) || (bgr[2] != 0))
			{
				if (dst.data)
					dst &= cv::Scalar(bgr[0], bgr[1], bgr[2]);
				continue;
			}
		}


		//----------------------------------------------------------------------
		//     - scale=num
		//         - Mat *= num
		if (cmd == "scale")
		{
			float scale = 0;
			sscanf(param.c_str(), "%f", &scale);
			if (scale != 0)
			{
				if (dst.data)
					dst *= scale;
				continue;
			}
		}

		//----------------------------------------------------------------------
		//	- gray convert
		if (cmd == "gray")
		{
			if (dst.data && (dst.channels() >= 3))
				cvtColor(dst, dst, CV_BGR2GRAY);
			continue;
		}

		//----------------------------------------------------------------------
		if (cmd == "threshold")
		{
			int  thresh1 = 0;
			sscanf(param.c_str(), "%d", &thresh1);
			if (thresh1 > 0)
			{
				if (dst.data && (dst.channels() >= 3))
					cvtColor(dst, dst, CV_BGR2GRAY);
				threshold(dst, dst, thresh1, 255, CV_THRESH_BINARY);
				continue;
			}
		}

		//----------------------------------------------------------------------
		if (cmd == "otsu")
		{
			cv::threshold(dst, dst, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

			continue;
		}

		//----------------------------------------------------------------------
		if (cmd == "adapthreshold")
		{
			double thresh_c = 0;
			int block_size = 0;
			sscanf(param.c_str(), "%d,%lf", &block_size, &thresh_c);
			if (thresh_c > 0)
			{
				if (dst.data && (dst.channels() >= 3))
					cvtColor(dst, dst, CV_BGR2GRAY);
				adaptiveThreshold(dst, dst, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, block_size, thresh_c);
				continue;
			}
		}

		//----------------------------------------------------------------------
		if (cmd == "invert")
		{
			int  thresh = 0;
			sscanf(param.c_str(), "%d", &thresh);
			if (thresh > 0)
			{
				if (dst.data && (dst.channels() >= 3))
					cvtColor(dst, dst, CV_BGR2GRAY);
				threshold(dst, dst, thresh, 255, CV_THRESH_BINARY_INV);
				continue;
			}
		}


		//----------------------------------------------------------------------
		// cvt = hsv/hls/bgr
		if (cmd == "cvt")
		{
			if (param == "bgr-hsv")
			{
				cvtColor(dst, dst, CV_BGR2HSV);
			}
			else if (param == "bgr-hls")
			{
				cvtColor(dst, dst, CV_BGR2HLS);
			}
			else if (param == "bgr-gray")
			{
				cvtColor(dst, dst, CV_BGR2GRAY);
			}
			else if (param == "hsv-bgr")
			{
				cvtColor(dst, dst, CV_HSV2BGR);
			}
			else if (param == "hls-bgr")
			{
				cvtColor(dst, dst, CV_HLS2BGR);
			}
			else if (param == "gray-bgr")
			{
				cvtColor(dst, dst, CV_GRAY2BGR);
			}

			continue;
		}


		//----------------------------------------------------------------------
		//     - hsv=num1,num2,num3,num4,num5,num6
		//         - hsv converting, inRange( Scalar(num1,num2,num3), Scalar(num4,num5,num6) )
		if (cmd == "hsv")
		{
			int hsv[6] = { 0,0,0, 0,0,0 }; // inrage
			sscanf(param.c_str(), "%d,%d,%d,%d,%d,%d", hsv, hsv + 1, hsv + 2, hsv + 3, hsv + 4, hsv + 5);
			if ((hsv[0] != 0) || (hsv[1] != 0) || (hsv[2] != 0) || (hsv[3] != 0) || (hsv[4] != 0) || (hsv[5] != 0))
			{
				if (dst.data)
				{
					cvtColor(dst, dst, CV_BGR2HSV);
					inRange(dst, cv::Scalar(hsv[0], hsv[1], hsv[2]), cv::Scalar(hsv[3], hsv[4], hsv[5]), dst);
					cvtColor(dst, dst, CV_GRAY2BGR);
					accMat.push_back(dst.clone());
				}
				continue;
			}
		}

		//----------------------------------------------------------------------
		//     - hls=num1,num2,num3,num4,num5,num6
		//         - hsv converting, inRange( Scalar(num1,num2,num3), Scalar(num4,num5,num6) )
		if (cmd == "hls")
		{
			int hsl[6] = { 0,0,0, 0,0,0 }; // inrage
			sscanf(param.c_str(), "%d,%d,%d,%d,%d,%d", hsl, hsl + 1, hsl + 2, hsl + 3, hsl + 4, hsl + 5);
			if ((hsl[0] != 0) || (hsl[1] != 0) || (hsl[2] != 0) || (hsl[3] != 0) || (hsl[4] != 0) || (hsl[5] != 0))
			{
				if (dst.data)
				{
					cvtColor(dst, dst, CV_BGR2HLS);
					inRange(dst, cv::Scalar(hsl[0], hsl[1], hsl[2]), cv::Scalar(hsl[3], hsl[4], hsl[5]), dst);
					cvtColor(dst, dst, CV_GRAY2BGR);
					accMat.push_back(dst.clone());
				}
				continue;
			}
		}

		if (cmd == "acc")
		{
			if (!accMat.empty())
			{
				cv::Mat tmp(accMat.front().rows, accMat.front().cols, accMat.front().flags);
				for each (auto & m in accMat)
					tmp += m;
				dst = tmp;
			}
			continue;
		}

		if (cmd == "shiftAdd")
		{
			int pixels=0, direction=0;
			sscanf(param.c_str(), "%d,%d", &pixels, &direction);

			if (dst.data)
			{
				cv::Mat tmpSrc(dst.size(), CV_8UC1, cv::Scalar(0));
				dst.copyTo(tmpSrc);
				tmpSrc = shiftFrame(tmpSrc, pixels, direction);
				dst += tmpSrc; 
			}
			continue;
		}
		if (cmd == "shiftSub")
		{
			int pixels = 0, direction = 0;
			sscanf(param.c_str(), "%d,%d", &pixels, &direction);

			if (dst.data)
			{
				cv::Mat tmpSrc(dst.size(), CV_8UC1, cv::Scalar(0));
				dst.copyTo(tmpSrc);
				tmpSrc = shiftFrame(tmpSrc, pixels, direction);
				dst -= tmpSrc;
			}
			continue;
		}
		//----------------------------------------------------------------------
		// SOBEL MAGNITUDE
		if (cmd == "soble_magnitude")
		{
			if (dst.data)
			{
				cv::Mat x, y;
				cv::Sobel(dst, x, CV_32FC1, 1, 0);
				cv::Sobel(dst, y, CV_32FC1, 0, 1);

				cv::Mat float_mag, mag;
				cv::magnitude(x, y, float_mag);
				float_mag.convertTo(dst, CV_8UC1);
			}
			continue;
		}

		//----------------------------------------------------------------------
		//Remove Spot Noise
		if (cmd == "removenoise")
		{
			int  sz_min = 0,sz_color=0;
			sscanf(param.c_str(), "%d,%d",&sz_color,&sz_min);
			if (sz_min > 0)
			{
				cv::Mat tmpSrc(dst.size(), CV_8UC1, cv::Scalar(0));
				dst.copyTo(tmpSrc);
				if (sz_color == 1) cv::bitwise_not(tmpSrc, tmpSrc);

				std::vector<std::vector<cv::Point>> contours;
				std::vector<cv::Vec4i> hierarchy;
				cv::findContours(tmpSrc, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
			
				for (int i = 0; i < contours.size(); i++)
				{
					cv::Moments moms = moments(cv::Mat(contours[i]));
					double area = moms.m00;

					if (area< sz_min)
					{
						if (sz_color == 1) drawContours(dst, contours, i, cv::Scalar(255), CV_FILLED, 8, hierarchy, 0, cv::Point());
						else drawContours(dst, contours, i, cv::Scalar(0), CV_FILLED, 8, hierarchy, 0, cv::Point());
					}
				}

			}
			continue;
		}
		//----------------------------------------------------------------------
		// Re Load
		if (cmd == "reload")
		{
			if (!dst.empty() && (src.channels() != dst.channels()))
			{
				dst.release();
			}
			dst = src.clone();
			continue;
		}

		if (cmd == "save")
		{
			ZeroMemory(value, sizeof(value));
			sscanf(param.c_str(), "%s", value);
			if (value[0] != NULL)
			{
				cv::imwrite(value,dst);
			}
			continue;
		}
		//----------------------------------------------------------------------
		// hough
		if (cmd == "hough")
		{
			ZeroMemory(value, sizeof(value));
			sscanf(param.c_str(), "%s", value);
			if (value[0] != NULL)
			{
				std::vector<cv::Vec2f> lines;
				cv::HoughLines(dst, lines, 1, 0.1f, 80);

				// ~~~~ programming
			}
			continue;
		}


		//----------------------------------------------------------------------
		// canny=threshold
		if (cmd == "canny")
		{
			ZeroMemory(value, sizeof(value));
			sscanf(param.c_str(), "%s", value);
			if (value[0] != NULL)
			{
				const int threshold1 = atoi(value);
				cv::Canny(dst, dst, 0, threshold1, 5);
			}
			continue;
		}


		//----------------------------------------------------------------------
		// findcontours=arcAlpha
		if (cmd == "findcontours")
		{
			int  sz_min = 0;
			sscanf(param.c_str(), "%d", &sz_min);
			if (sz_min > 0)
			{
				cv::Mat  displayImage;
				cv::cvtColor(dst, displayImage, cv::COLOR_GRAY2BGR);				
				
				vector<cv::Point> approx;
				vector<vector<cv::Point>> contours;
				cv::findContours(dst, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);				

				for (int i = 0; i < contours.size(); i++)
				{
					int area = contourArea(contours[i]);

					if (area > sz_min)
					{
						double epsilon = cv::arcLength(cv::Mat(contours[i]), true)*0.0005;
						cv::approxPolyDP(contours[i], approx, epsilon, true);
						drawContours(displayImage, vector<vector<cv::Point>>(1, approx), -1, cv::Scalar(255, 0, 0), 1);
						approx.clear();
					}
				}	
				
				contours.clear();

				dst = displayImage.clone();
			}
			continue;
		}
		//----------------------------------------------------------------------
		//     - Pseudo coloring
		if (cmd == "pseudo")
		{
			/* 0	COLORMAP_AUTUMN	colorscale_autumn
			 1	COLORMAP_BONE	colorscale_bone
			 2	COLORMAP_JET	colorscale_jet
			 3	COLORMAP_WINTER	colorscale_winter
			 4	COLORMAP_RAINBOW	colorscale_rainbow
			 5	COLORMAP_OCEAN	colorscale_ocean
			 6	COLORMAP_SUMMER	colorscale_summer
			 7	COLORMAP_SPRING	colorscale_spring
			 8	COLORMAP_COOL	colorscale_cool
			 9	COLORMAP_HSV	colorscale_hsv
			 10	COLORMAP_PINK	colorscale_pink
			 11	COLORMAP_HOT*/
			int mode = 2;
			sscanf(param.c_str(), "%d", &mode);
			if (mode >=0 && mode<=11)
			{
				cv::applyColorMap(dst, dst, mode);
		    }
			continue;
		}

		//----------------------------------------------------------------------
		//     - dilate
		if (cmd == "dilate")
		{
			int X = 3, Y = 3;
			sscanf(param.c_str(), "%d,%d", &X, &Y);
			dilate(dst, dst, cv::Mat::ones(cv::Size(X, Y), CV_8UC1));
			continue;
		}

		//----------------------------------------------------------------------
		//     - erode
		if (cmd == "erode")
		{
			int X = 0, Y = 0;
			sscanf(param.c_str(), "%d,%d", &X, &Y);
			erode(dst, dst, cv::Mat::ones(cv::Size(X, Y), CV_8UC1));
			continue;
		}
			
		//----------------------------------------------------------------------
		//     - morphologyEx
		if (cmd == "morphologyEx")
		{
			int filterSize = 0,method=0;
			sscanf(param.c_str(), "%d,%d", &filterSize, &method);
			if (method >= 0 && method <= 6)
			{
				cv::Mat element = cv::getStructuringElement(CV_SHAPE_RECT, cv::Size(filterSize, filterSize));

				switch (method)
				{
				case 0: morphologyEx(dst, dst, CV_MOP_ERODE, element); break;
				case 1: morphologyEx(dst, dst, CV_MOP_DILATE, element); break;
				case 2: morphologyEx(src, dst, CV_MOP_OPEN, element); break;
				case 3: morphologyEx(dst, dst, CV_MOP_CLOSE, element); break;
				case 4: morphologyEx(dst, dst, CV_MOP_GRADIENT, element); break;
				case 5: morphologyEx(dst, dst, CV_MOP_TOPHAT, element); break;
				case 6: morphologyEx(dst, dst, CV_MOP_BLACKHAT, element); break;
				}
			}
			continue;
		}	
		//Gaussian
		if (cmd == "gaussian")
		{
			cv::GaussianBlur(dst, dst, cv::Size(0, 0), 8.);
			continue;
		}
		//MeanFilter
		if (cmd == "medianblur")
		{
			cv::medianBlur(dst, dst,3);
			continue;
		}
			//Sobel Up=0,Down=1,Left=2,Right=3,45=4,135=5,225=6,315=7
		if (cmd == "sobel")
		{
			ZeroMemory(value, sizeof(value));
			sscanf(param.c_str(), "%s", value);
			if (value[0] != NULL)
			{
				int nDirection = atoi(value);
				sobelDirection(3, nDirection, dst, dst);
			}
			continue;
		}

		//Enhance=0, EnhanceY=1,EnhanceX=2,
		if (cmd == "enhance")
		{
			ZeroMemory(value, sizeof(value));
			sscanf(param.c_str(), "%s", value);
			if (value[0] != NULL)
			{
				int proc = atoi(value);
				InspectionEnhance(&dst, proc, false);
			}
			continue;
		}

		//Flip
		if (cmd == "flip")
		{
			ZeroMemory(value, sizeof(value));
			sscanf(param.c_str(), "%s", value);
			if (value[0] != NULL)
			{
				int proc = atoi(value);
				cv::flip(dst, dst, proc);
			}
			continue;
		}
		//  bliateralFilter
		if (cmd == "bliateralfilter")
		{
			cv::bilateralFilter(dst, dst, -1, 5, 5 / 2);
			continue;
		}

		//resize
		if (cmd == "resize")
		{
			ZeroMemory(value, sizeof(value));
			sscanf(param.c_str(), "%s", value);
			if (value[0] != NULL)
			{
				double proc = atof(value);
				cv::resize(dst, dst, cv::Size(), proc, proc, CV_INTER_CUBIC);
			}
			continue;
		}

		if (cmd == "cartToPolar")
		{
			int aperture = 3;

			cv::Mat sobelX, sobelY;
			cv::Mat sobelMagnitude, sobelOrientation;
			
			cv::Sobel(dst, sobelX, CV_32F, 1, 0, aperture);
			cv::Sobel(dst, sobelY, CV_32F, 0, 1, aperture);

			cv::cartToPolar(sobelX, sobelY, sobelMagnitude, sobelOrientation, true);
			
			normalize(sobelMagnitude, dst, 0, 255, 32, CV_8UC1);
			
			sobelX.release();
			sobelY.release();
			sobelMagnitude.release();
			sobelOrientation.release();

			continue;
		}
		

	}

	unsigned long t2 = GetTickCount64();
	if (dst.data && showMsg)
		putText(dst, ::format("time=%d", t2 - t1).c_str(), cv::Point(0, 60), 1, 2.f, cv::Scalar(255, 255, 255), 2);

	return errMsg;
}

void CSimpleMatchScript::sobelDirection(int nMaskSize, int nDirection, cv::Mat& srcImage, cv::Mat& dstImage)
{
	cv::Mat mask = cv::Mat::ones(nMaskSize, nMaskSize, CV_32F) / 25;

	float* pBuffer = (float*)mask.data;

	if (nMaskSize == 3)
	{
		switch (nDirection) {
		case SOBEL_UP:
			pBuffer[0] = 1;	    pBuffer[1] = 2;	    pBuffer[2] = 1;
			pBuffer[3] = 0;	    pBuffer[4] = 0;	    pBuffer[5] = 0;
			pBuffer[6] = -1;	pBuffer[7] = -2;	pBuffer[8] = -1;
			break;
		case SOBEL_DN:
			pBuffer[0] = -1;	pBuffer[1] = -2;	pBuffer[2] = -1;
			pBuffer[3] = 0;	    pBuffer[4] = 0;	    pBuffer[5] = 0;
			pBuffer[6] = 1;	    pBuffer[7] = 2;	    pBuffer[8] = 1;
			break;
		case SOBEL_LEFT:
			pBuffer[0] = 1;	pBuffer[1] = 0;	pBuffer[2] = -1;
			pBuffer[3] = 2;	pBuffer[4] = 0;	pBuffer[5] = -2;
			pBuffer[6] = 1;	pBuffer[7] = 0;	pBuffer[8] = -1;
			break;
		case SOBEL_RIGHT:
			pBuffer[0] = -1;	pBuffer[1] = 0;	pBuffer[2] = 1;
			pBuffer[3] = -2;	pBuffer[4] = 0;	pBuffer[5] = 2;
			pBuffer[6] = -1;	pBuffer[7] = 0;	pBuffer[8] = 1;
			break;
		case SOBEL_45: // 왼쪽 방향쪽
			pBuffer[0] = -2;	pBuffer[1] = -1;	pBuffer[2] = 0;
			pBuffer[3] = -1;	pBuffer[4] = 0;	    pBuffer[5] = 1;
			pBuffer[6] = 0;	    pBuffer[7] = 1;	    pBuffer[8] = 2;
			break;
		case SOBEL_135: // 왼쪽 방향쪽
			pBuffer[0] = 2;	pBuffer[1] = 1;	  pBuffer[2] = 0;
			pBuffer[3] = 1;	pBuffer[4] = 0;	  pBuffer[5] = -1;
			pBuffer[6] = 0;	pBuffer[7] = -1;  pBuffer[8] = -2;
			break;
		case SOBEL_225: // 오른쪽 방향쪽
			pBuffer[0] = 0;	 pBuffer[1] = 1;	pBuffer[2] = 2;
			pBuffer[3] = -1; pBuffer[4] = 0;	pBuffer[5] = 1;
			pBuffer[6] = -2; pBuffer[7] = -1;	pBuffer[8] = 0;
			break;
		case SOBEL_315: // 오른쪽 방향쪽
			pBuffer[0] = 0;	 pBuffer[1] = -1;	pBuffer[2] = -2;
			pBuffer[3] = 1;  pBuffer[4] = 0;	pBuffer[5] = -1;
			pBuffer[6] = 2;  pBuffer[7] = 1;	pBuffer[8] = 0;
			break;
		};
	}

	cv::filter2D(srcImage, dstImage, -1, mask, cv::Point(-1, -1), (0, 0), 2);
	mask.release();
}

void CSimpleMatchScript::InspectionEnhance(cv::Mat * src, int id, bool disp)		// 0 : All, 1 : Y, 2 : X
{
	cv::Mat m_KrnlGusX[3];
	cv::Mat m_KrnlLOGX[3];
	cv::Mat m_KrnlGusY[3];
	cv::Mat m_KrnlLOGY[3];

	cv::Mat m_Proc1Buf;
	cv::Mat m_Proc2Buf;
	cv::Mat m_Proc3Buf;
	cv::Mat m_TempoBuf;

	float nKernelGUS[12] = { 10, 18, 29, 43, 57, 67, 71, 67, 57, 43, 29, 18 }; //509
	float nKernelGUS2[12] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }; // 12
	float nKernelLOG[24] = { -1, -2, -4, -9, -15, -20, -22, -16, 0, 22, 42, 50, 42, 22, 0, -16, -22, -20, -15, -9, -4, -2, -1, 0 }; //32

	m_KrnlGusX[0] = cv::Mat(12, 1, CV_32FC1, nKernelGUS) / 509.f;
	m_KrnlLOGX[0] = cv::Mat(24, 1, CV_32FC1, nKernelLOG) / 32.f;
	m_KrnlGusY[0] = cv::Mat(1, 12, CV_32FC1, nKernelGUS) / 509.f;
	m_KrnlLOGY[0] = cv::Mat(1, 24, CV_32FC1, nKernelLOG) / 32.f;

	m_KrnlLOGX[1] = cv::Mat(24, 1, CV_32FC1, nKernelLOG) / 32.f;
	m_KrnlGusY[1] = cv::Mat(1, 12, CV_32FC1, nKernelGUS2) / 12.f;

	m_KrnlGusX[2] = cv::Mat(12, 1, CV_32FC1, nKernelGUS2) / 12.f;
	m_KrnlLOGY[2] = cv::Mat(1, 24, CV_32FC1, nKernelLOG) / 32.f;

	int w = src->cols;
	int h = src->rows;

	m_Proc1Buf = cv::Mat::zeros(h, w, CV_32FC1);
	m_Proc2Buf = cv::Mat::zeros(h, w, CV_32FC1);
	m_Proc3Buf = cv::Mat::zeros(h, w, CV_32FC1);
	m_TempoBuf = cv::Mat::zeros(h, w, CV_32FC1);

	cv::Mat dst;
	cv::Mat element(3, 3, CV_8U, cv::Scalar(1));
	cv::Mat srcImage;

	src->copyTo(srcImage);
	src->convertTo(m_TempoBuf, CV_32F, 1., -128.);

	if (id == 0)	// 양방향 Enhance
	{
		cv::sepFilter2D(m_TempoBuf, m_Proc1Buf, CV_32F, m_KrnlGusX[0], m_KrnlLOGY[0], cv::Point(-1, -1), 0, cv::BORDER_REPLICATE);
		cv::sepFilter2D(m_TempoBuf, m_Proc2Buf, CV_32F, m_KrnlLOGX[0], m_KrnlGusY[0], cv::Point(-1, -1), 0, cv::BORDER_REPLICATE);
		cv::add(m_Proc1Buf, m_Proc2Buf, m_Proc3Buf);
	}
	else if (id == 1)
	{
		// 단방향 Enhance
		cv::sepFilter2D(m_TempoBuf, m_Proc3Buf, CV_32F, m_KrnlGusX[0], m_KrnlLOGY[0], cv::Point(-1, -1), 0, cv::BORDER_REPLICATE);
	}
	else
	{
		// 단방향 Enhance
		cv::sepFilter2D(m_TempoBuf, m_Proc3Buf, CV_32F, m_KrnlLOGX[1], m_KrnlGusY[1], cv::Point(-1, -1), 0, cv::BORDER_REPLICATE);
	}

	m_Proc3Buf.convertTo(*src, CV_8U, 1., 128.);

	for (int i = 0; i < 3; i++)
	{
		m_KrnlGusX[i].release();
		m_KrnlLOGX[i].release();
		m_KrnlGusY[i].release();
		m_KrnlLOGY[i].release();
	}

	m_Proc1Buf.release();
	m_Proc2Buf.release();
	m_Proc3Buf.release();
	m_TempoBuf.release();
	dst.release();
	element.release();
	srcImage.release();
}

cv::Mat CSimpleMatchScript::shiftFrame(cv::Mat frame, int pixels, int direction)
{
	cv::Mat temp = cv::Mat::zeros(frame.size(), frame.type());

	switch (direction)
	{
	case(ShiftUp):
		frame(cv::Rect(0, pixels, frame.cols, frame.rows - pixels)).copyTo(temp(cv::Rect(0, 0, temp.cols, temp.rows - pixels)));
		break;
	case(ShiftRight):
		frame(cv::Rect(0, 0, frame.cols - pixels, frame.rows)).copyTo(temp(cv::Rect(pixels, 0, frame.cols - pixels, frame.rows)));
		break;
	case(ShiftDown):
		frame(cv::Rect(0, 0, frame.cols, frame.rows - pixels)).copyTo(temp(cv::Rect(0, pixels, frame.cols, frame.rows - pixels)));
		break;
	case(ShiftLeft):
		frame(cv::Rect(pixels, 0, frame.cols - pixels, frame.rows)).copyTo(temp(cv::Rect(0, 0, frame.cols - pixels, frame.rows)));
		break;
	}

	return temp;
}


cRectContour::cRectContour() :
	m_contours(4)
	, m_xIdx(0)
{
}

cRectContour::cRectContour(const cv::Rect& rect) :
	m_contours(4)
	, m_xIdx(0)
{
	Init(rect);
}

cRectContour::~cRectContour()
{
}

bool cRectContour::Init(const cv::Rect& rect)
{
	m_contours[0] = rect.tl();
	m_contours[1] = cv::Point(rect.x + rect.width, rect.y);
	m_contours[2] = cv::Point(rect.x + rect.width, rect.y + rect.height);
	m_contours[3] = cv::Point(rect.x, rect.y + rect.height);
	return true;
}


// 선을 그린다.
void DrawLines(cv::Mat & dst, const vector<cv::Point> & lines, const cv::Scalar & color, const int thickness,
	const bool isLoop = true)
{
	if (lines.size() < 2)
		return;

	for (u_int i = 0; i < lines.size() - 1; ++i)
		line(dst, lines[i], lines[i + 1], color, thickness);

	if (isLoop)
		line(dst, lines[lines.size() - 1], lines[0], color, thickness);
}


// 박스 출력.
void cRectContour::Draw(cv::Mat & dst, const cv::Scalar & color, const int thickness) const
// color = cv::Scalar(0, 0, 0), thickness = 1
{
	DrawLines(dst, m_contours, color, thickness);
}


// 사각형의 중점을 리턴한다.
cv::Point cRectContour::Center() const
{
	cv::Point center;
	for each (auto & pt in m_contours)
		center += pt;

	center = cv::Point(int(center.x / m_contours.size()), int(center.y / m_contours.size()));
	return center;
}


// 사각형의 중점을 중심으로 스케일한다.
void cRectContour::ScaleCenter(const float scale)
{
	const cv::Point center = Center();

	for (u_int i = 0; i < m_contours.size(); ++i)
	{
		m_contours[i] = center + ((m_contours[i] - center) * scale);
	}
}


// 가로 세로 각각 스케일링 한다.
// 0 -------- 1
// |          |
// |    +     |
// |          |
// 3 -------- 2
void cRectContour::Scale(const float vscale, const float hscale)
{

	// 가로 스케일링
	vector<cv::Point> tmp1(4);
	tmp1[0] = m_contours[1] + (m_contours[0] - m_contours[1]) * hscale;
	tmp1[1] = m_contours[0] + (m_contours[1] - m_contours[0]) * hscale;
	tmp1[2] = m_contours[3] + (m_contours[2] - m_contours[3]) * hscale;
	tmp1[3] = m_contours[2] + (m_contours[3] - m_contours[2]) * hscale;

	// 세로 스케일링
	vector<cv::Point> tmp2(4);
	tmp2[0] = m_contours[3] + (m_contours[0] - m_contours[3]) * vscale;
	tmp2[3] = m_contours[0] + (m_contours[3] - m_contours[0]) * vscale;
	tmp2[1] = m_contours[2] + (m_contours[1] - m_contours[2]) * vscale;
	tmp2[2] = m_contours[1] + (m_contours[2] - m_contours[1]) * vscale;

	m_contours[0] = (tmp1[0] + tmp2[0]) * 0.5f;
	m_contours[1] = (tmp1[1] + tmp2[1]) * 0.5f;
	m_contours[2] = (tmp1[2] + tmp2[2]) * 0.5f;
	m_contours[3] = (tmp1[3] + tmp2[3]) * 0.5f;
}


// index 번째 포인터를 리턴한다.
cv::Point cRectContour::At(const int index) const
{
	return m_contours[index];
}


int cRectContour::Width() const
{
	const int idx1 = m_xIdx;
	const int idx2 = m_xIdx + 1;
	const int idx3 = m_xIdx + 2;
	const int idx4 = (m_xIdx + 3) % 4;

	return (int)abs(((m_contours[idx2].x - m_contours[idx1].x) +
		(m_contours[idx3].x - m_contours[idx4].x)) * 0.5f);
}


int cRectContour::Height() const
{
	const int idx1 = m_xIdx + 1;
	const int idx2 = m_xIdx + 2;
	const int idx3 = (m_xIdx + 3) % 4;
	const int idx4 = m_xIdx;

	return (int)abs(((m_contours[idx2].y - m_contours[idx1].y) +
		(m_contours[idx3].y - m_contours[idx4].y)) * 0.5f);
}


cRectContour & cRectContour::operator= (const cRectContour & rhs)
{
	if (this != &rhs)
	{
		m_contours = rhs.m_contours;
		m_xIdx = rhs.m_xIdx;
	}

	return *this;
}