#pragma once

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef INOUT
#define INOUT
#endif

class cRectContour
{
public:
	cRectContour();
	cRectContour(const cv::Rect& rect);
	virtual ~cRectContour();

	bool Init(const cv::Rect& rect);
	void Draw(cv::Mat& dst, const cv::Scalar& color = cv::Scalar(0, 0, 0), const int thickness = 1) const;

	int Width() const;
	int Height() const;
	cv::Point At(const int index) const;
	cv::Point Center() const;
	void ScaleCenter(const float scale);
	void Scale(const float vscale, const float hscale);

	cRectContour& operator = (const cRectContour& rhs);


public:
	vector<cv::Point> m_contours;
	int m_xIdx; // x axis index
};

class CSimpleMatchScript
{
public:
	CSimpleMatchScript();
	~CSimpleMatchScript();

	std::string Match(INOUT cv::Mat& src, OUT cv::Mat& dst, const std::string& script, const bool showMsg = true);
	void sobelDirection(int nMaskSize, int nDirection, cv::Mat& srcImage, cv::Mat& dstImage);
	void InspectionEnhance(cv::Mat* src, int id, bool disp);
	cv::Mat shiftFrame(cv::Mat frame, int pixels, int direction);
	
protected:
	void Parse(const std::string& script);
	std::string pid(const std::string& str);
	int attrs(const std::string& str, OUT std::string& out);
	void attr_list(const std::string& str);

public:
	enum STATE { WAIT, BEGIN_MATCH };
	STATE m_state;
	vector<std::pair<std::string, std::string>> m_commands;
	cv::Mat m_src;
	cv::Mat m_tessImg;
	int m_curIdx;
	int m_beginMatchTime;
};

