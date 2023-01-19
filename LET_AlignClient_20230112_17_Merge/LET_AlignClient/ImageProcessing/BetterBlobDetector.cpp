#include "StdAfx.h"
#include "BetterBlobDetector.h"
#include <iterator>
//#include "UserDefineHeader.h"

using namespace cv;

BetterBlobDetector::BetterBlobDetector(const SimpleBlobDetector::Params &parameters)
{
	params = parameters;
}

void BetterBlobDetector::findBlobs(const cv::Mat &image, const cv::Mat &binaryImage,
	vector<Center> &centers, std::vector < std::vector<cv::Point> >&curContours,
	std::vector<_stDefectInfo> &curDefectInfo ) const
{
	(void)image;
	centers.clear();

	curContours.clear();

	std::vector < std::vector<cv::Point> >contours;
	Mat tmpBinaryImage = binaryImage.clone();

	findContours(tmpBinaryImage, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);


	for (size_t contourIdx = 0; contourIdx < contours.size(); contourIdx++)
	{
		_stDefectInfo defectInfo = {0, };
		Center center;
		center.confidence = 1;
		Moments moms = moments(Mat(contours[contourIdx]));
		if (params.filterByArea)
		{
			double area = moms.m00;
			defectInfo.s_dArea = moms.m00;
			if (area < params.minArea || area >= params.maxArea)
				continue;
		}
		
		if (params.filterByCircularity)
		{
			double area = moms.m00;
			double perimeter = arcLength(Mat(contours[contourIdx]), true);
			double ratio = 4 * CV_PI * area / (perimeter * perimeter);
			defectInfo.s_dCircularity = ratio;
			if (ratio < params.minCircularity || ratio >= params.maxCircularity)
				continue;
		}

		if (params.filterByInertia)
		{
			double denominator = sqrt(pow(2 * moms.mu11, 2) + pow(moms.mu20 - moms.mu02, 2));
			const double eps = 1e-2;
			double ratio;
			if (denominator > eps)
			{
				double cosmin = (moms.mu20 - moms.mu02) / denominator;
				double sinmin = 2 * moms.mu11 / denominator;
				double cosmax = -cosmin;
				double sinmax = -sinmin;

				double imin = 0.5 * (moms.mu20 + moms.mu02) - 0.5 * (moms.mu20 - moms.mu02) * cosmin - moms.mu11 * sinmin;
				double imax = 0.5 * (moms.mu20 + moms.mu02) - 0.5 * (moms.mu20 - moms.mu02) * cosmax - moms.mu11 * sinmax;
				ratio = imin / imax;
			}
			else
			{
				ratio = 1;
			}

			if (ratio < params.minInertiaRatio || ratio >= params.maxInertiaRatio)
				continue;

			center.confidence = ratio * ratio;
		}

		if (params.filterByConvexity)
		{
			vector < cv::Point > hull;
			convexHull(Mat(contours[contourIdx]), hull);
			double area = contourArea(Mat(contours[contourIdx]));
			double hullArea = contourArea(Mat(hull));
			double ratio = area / hullArea;
			if (ratio < params.minConvexity || ratio >= params.maxConvexity)
				continue;
		}
		
		center.location = Point2d(moms.m10 / moms.m00, moms.m01 / moms.m00);

		if (params.filterByColor)
		{
			if (binaryImage.at<uchar> (cvRound(center.location.y), cvRound(center.location.x)) != params.blobColor)
				continue;
		}

		//compute blob radius
		{
			vector<double> dists;
			for (size_t pointIdx = 0; pointIdx < contours[contourIdx].size(); pointIdx++)
			{
				Point2d pt = contours[contourIdx][pointIdx];
				dists.push_back(norm(center.location - pt));
			}
			std::sort(dists.begin(), dists.end());
			center.radius = (dists[(dists.size() - 1) / 2] + dists[dists.size() / 2]) / 2.;
		}

		centers.push_back(center);
		curContours.push_back(contours[contourIdx]);
		curDefectInfo.push_back( defectInfo );
	}
}

static std::vector < std::vector<cv::Point> > _contours;
static std::vector <_stDefectInfo> _defectInfos;

const std::vector < std::vector<cv::Point> > BetterBlobDetector::getContours()
{
	return _contours;
}

const std::vector < _stDefectInfo > BetterBlobDetector::getDefectInfo()
{
	return _defectInfos;
}

void BetterBlobDetector::detectImpl(const cv::Mat& image, std::vector<cv::KeyPoint>& keypoints, const cv::Mat&) const
{
	//TODO: support mask
	_contours.clear();
	_defectInfos.clear();
	keypoints.clear();

	Mat grayscaleImage;
	if (image.channels() == 3)
		cvtColor(image, grayscaleImage, CV_BGR2GRAY);
	else
		grayscaleImage = image;

	std::vector < std::vector<Center> > centers;
	std::vector < std::vector<cv::Point> >contours;
	std::vector < _stDefectInfo > defectInfos;
	
	//for (double thresh = params.minThreshold; thresh < params.maxThreshold; thresh += params.thresholdStep)
	// 구조상 최소 2번 이상 loop가 돌아야 함..ㅠㅠ 수정하기 귀찮아서..
	for(int i = 0; i < 2; i++)
	{
		Mat binarizedImage;
		//threshold(grayscaleImage, binarizedImage, thresh, 255, THRESH_BINARY);
		threshold(grayscaleImage, binarizedImage, params.minThreshold, 255, THRESH_BINARY);

		std::vector < Center > curCenters;
		std::vector < std::vector<cv::Point> > curContours, newContours;
		std::vector <_stDefectInfo> curDefectInfos, newDefectInfos;

		findBlobs(grayscaleImage, binarizedImage, curCenters, curContours, curDefectInfos);
		
		std::vector < std::vector<Center> > newCenters;
		for (size_t i = 0; i < curCenters.size(); i++)
		{
			bool isNew = true;
			for (size_t j = 0; j < centers.size(); j++)
			{
				double dist = norm(centers[j][ centers[j].size() / 2 ].location - curCenters[i].location);
				isNew = dist >= params.minDistBetweenBlobs && dist >= centers[j][ centers[j].size() / 2 ].radius && dist >= curCenters[i].radius;

				if (!isNew)
				{
					centers[j].push_back(curCenters[i]);

					size_t k = centers[j].size() - 1;
					while( k > 0 && centers[j][k].radius < centers[j][k - 1].radius )
					{
						centers[j][k] = centers[j][k - 1];
						k--;
					}

					centers[j][k] = curCenters[i];
					break;
				}
			}

			if (isNew)
			{
				newCenters.push_back(vector<Center> (1, curCenters[i]));
				newContours.push_back(curContours[i]);
				newDefectInfos.push_back( curDefectInfos[i] );
				//centers.push_back(vector<Center> (1, curCenters[i]));
			}
		}

		std::copy(newCenters.begin(), newCenters.end(), std::back_inserter(centers));
		std::copy(newContours.begin(), newContours.end(), std::back_inserter(contours));
		std::copy(newDefectInfos.begin(), newDefectInfos.end(), std::back_inserter(defectInfos));
	}

	for (size_t i = 0; i < centers.size(); i++)
	{
		if (centers[i].size() < params.minRepeatability)
			continue;

		Point2d sumPoint(0, 0);
		double normalizer = 0;
		for (size_t j = 0; j < centers[i].size(); j++)
		{
			sumPoint += centers[i][j].confidence * centers[i][j].location;
			normalizer += centers[i][j].confidence;
		}
		sumPoint *= (1. / normalizer);
		KeyPoint kpt(sumPoint, (float)(centers[i][centers[i].size() / 2].radius));
		keypoints.push_back(kpt);
		_contours.push_back(contours[i]);
		_defectInfos.push_back( defectInfos[i] );
	}
}

CMBlob::CMBlob()
{
	Initialize();
}

CMBlob::~CMBlob()
{
	Free();
}

CMBlob::CMBlob(int maxline, int maxblob, int maxindex)
{
	Create(maxline, maxblob, maxindex);
}

void CMBlob::Initialize()
{
	m_ImgWidth = 0;
	m_ImgHeight = 0;
	m_RoiLeft = 0;
	m_RoiTop = 0;
	m_RoiRight = 0;
	m_RoiBottom = 0;
	m_MaxLine = 0;
	m_NumLine = 0;
	m_aPtrLine = NULL;
	m_MaxIndex = 0;
	m_NumIndex = 0;
	m_aPtrIndex = NULL;
	m_MaxBlob = 0;
	m_NumBlob = 0;
	m_aPtrBlob = 0;
	m_aMap = NULL;

	memset(m_pPtRect, 0, sizeof(POINT) * 4);
	m_bCheckDone = FALSE;
}

void CMBlob::InitValue()
{
	m_NumBlob = 0;
	memset(m_aPtrLine, 0, sizeof(STLINE) * m_MaxLine);
	memset(m_aPtrBlob, 0, sizeof(STBLOB) * m_MaxBlob);
	memset(m_aPtrIndex, 0, sizeof(STINDEX) * m_MaxIndex);
	memset(m_aMap, 0, sizeof(WORD) * m_MaxIndex);
	memset(m_bRegBlobSeed, 0, sizeof(BOOL) * m_MaxBlob);
	memset(m_pPtRect, 0, sizeof(POINT) * 4);
	memset(m_pVertx, 0, sizeof(float) * 4);
	memset(m_pVerty, 0, sizeof(float) * 4);
}

BOOL CMBlob::Create(int maxline, int maxblob, int maxindex)
{
	Initialize();

	m_MaxLine = maxline;
	m_MaxBlob = maxblob;
	m_MaxIndex = maxindex;

	m_aPtrLine = new STLINE[m_MaxLine];
	if (m_aPtrLine == NULL)
	{
		AfxMessageBox((LPCTSTR)"Blob : Insufficient Memory");
		return FALSE;
	}
	m_aPtrBlob = new STBLOB[m_MaxBlob];
	if (m_aPtrBlob == NULL)
	{
		AfxMessageBox((LPCTSTR)"Blob : Insufficient Memory");
		return FALSE;
	}
	m_aPtrIndex = new STINDEX[m_MaxIndex];
	if (m_aPtrIndex == NULL)
	{
		AfxMessageBox((LPCTSTR)"Blob : Insufficient Memory");
		return FALSE;
	}

	m_aMap = new WORD[m_MaxIndex];
	if (m_aMap == NULL)
	{
		AfxMessageBox((LPCTSTR)"Blob : Insufficient Memory");
		return FALSE;
	}

	// 2010.09.13
	m_bRegBlobSeed = new BOOL[m_MaxBlob];



	return TRUE;
}

void CMBlob::Free()
{
	delete[] m_bRegBlobSeed;
	m_bRegBlobSeed = NULL;

	delete[] m_aPtrLine;
	m_aPtrLine = NULL;

	delete[] m_aPtrBlob;
	m_aPtrBlob = NULL;

	delete[] m_aPtrIndex;
	m_aPtrIndex = NULL;

	delete[] m_aMap;
	m_aMap = NULL;

	m_MaxLine = 0;
	m_MaxBlob = 0;
	m_MaxIndex = 0;
}

void CMBlob::SetImgInfo(int width, int height, int byteDepth, int stepwidth)
{
	m_ImgWidth = width;
	m_ImgHeight = height;
	m_StepWidth = stepwidth;
	m_ByteDepth = byteDepth;
}

int CMBlob::BlobScan(LPBYTE pData)
{
	int result;

	result = ExtractLine(pData);
	//	if (result!=BLOBERR_NONE) return result;
	result += IndexLine();
	//	if (result!=BLOBERR_NONE) return result;
	result += LabelBlobObj();
	//	if (result!=BLOBERR_NONE) return result;

	return result;
}

void CMBlob::SetRoi(LPRECT pRect)
{
	m_RoiLeft = pRect->left;
	m_RoiTop = pRect->top;
	m_RoiRight = pRect->right;
	m_RoiBottom = pRect->bottom;

	m_RoiLeft = ((m_RoiLeft<0 || m_RoiLeft>m_ImgWidth) ? 0 : m_RoiLeft);
	m_RoiTop = ((m_RoiTop<0 || m_RoiTop>m_ImgHeight) ? m_ImgHeight : m_RoiTop);
	m_RoiRight = ((m_RoiRight<0 || m_RoiRight>m_ImgWidth) ? m_ImgWidth : m_RoiRight);
	m_RoiBottom = ((m_RoiBottom<0 || m_RoiBottom>m_ImgHeight) ? 0 : m_RoiBottom);
}

int CMBlob::ExtractLine(LPBYTE pData)
{
	int x, y;
	LPBYTE pBuf;

	m_NumLine = 0;

	for (y = m_RoiTop; y < m_RoiBottom; y++)
	{
		x = m_RoiLeft;
		pBuf = pData + (m_ImgHeight - y - 1) * m_StepWidth + x;
		while (x < m_RoiRight)
		{
			if (*pBuf && PointInPolygon(float(x), float(y)))
			{
				m_aPtrLine[m_NumLine].y = y;
				m_aPtrLine[m_NumLine].x1 = x;
				while (*pBuf && x < m_RoiRight)
				{
					if (!PointInPolygon(float(x), float(y))) break;
					pBuf++;
					x++;
				}
				m_aPtrLine[m_NumLine].x2 = x;
				m_aPtrLine[m_NumLine].index = 0;   // 2008.09.04

				m_NumLine++;
				if (m_NumLine >= m_MaxLine) return BLOBERR_MAXLINE;
			}
			else
			{
				pBuf++;
				x++;
			}
		}
	}
	return BLOBERR_NONE;
}

int CMBlob::IndexLine()
{
	int i, j, k, indi, indj, pti, ptj;
	BOOL bNew, bMaxLink;

	ZeroMemory((void*)m_aPtrIndex, sizeof(STINDEX) * m_MaxIndex);
	////////////////////////////////
	m_NumIndex = 1;
	////////////////////////////////
	bMaxLink = FALSE;

	if (m_NumLine < 1)
	{
		return BLOBERR_NONE;
	}

	m_aPtrLine[0].index = m_NumIndex++;
	for (i = 1; i < m_NumLine; i++)
	{
		for (k = i - 1; k >= 0 && (m_aPtrLine[k].y > m_aPtrLine[i].y - 2); k--);
		bNew = TRUE;

		for (j = k; j < i; j++)
		{
			if (j < 0) continue;   // 2008.09.04  k=-1인 경우가 발생하기 때문에 m_aPtrLine[-1].index처럼 배열을 음수로 인덱싱 하는 경우가 발생하고, 
								   // 아래의 while loop에서 무한 loop에 빠질 가능성이 있어서 추가한 코드임 

			if (m_aPtrLine[i].y - 1 != m_aPtrLine[j].y) continue;
			if (m_aPtrLine[j].x2 >= m_aPtrLine[i].x1 && m_aPtrLine[j].x1 <= m_aPtrLine[i].x2)
			{
				indi = m_aPtrLine[i].index;
				indj = m_aPtrLine[j].index;
				if (bNew)
				{
					m_aPtrLine[i].index = indj;
					bNew = FALSE;
				}
				else if (indi < indj)
				{
					if (m_aPtrIndex[indj].LinkPtr == 0) m_aPtrIndex[indj].LinkPtr = indi;
					else
					{
						ptj = m_aPtrIndex[indj].LinkPtr;
						while (ptj < m_MaxIndex && m_aPtrIndex[ptj].LinkPtr>0 && ptj != m_aPtrIndex[ptj].LinkPtr) ptj = m_aPtrIndex[ptj].LinkPtr;

						if (m_aPtrIndex[indi].LinkPtr == 0) pti = indi;
						else
						{
							pti = m_aPtrIndex[indi].LinkPtr;
							while (pti < m_MaxIndex && m_aPtrIndex[pti].LinkPtr>0 && pti != m_aPtrIndex[pti].LinkPtr) pti = m_aPtrIndex[pti].LinkPtr;
						}
						if (pti < ptj) m_aPtrIndex[ptj].LinkPtr = indi;
						else if (pti > ptj) m_aPtrIndex[pti].LinkPtr = indj;
					}
				}
				else if (indi > indj)
				{
					if (m_aPtrIndex[indi].LinkPtr == 0) m_aPtrIndex[indi].LinkPtr = indj;
					else
					{
						pti = m_aPtrIndex[indi].LinkPtr;
						while (pti < m_MaxIndex && m_aPtrIndex[pti].LinkPtr>0 && pti != m_aPtrIndex[pti].LinkPtr) pti = m_aPtrIndex[pti].LinkPtr;

						if (m_aPtrIndex[indj].LinkPtr == 0) ptj = indj;
						else
						{
							ptj = m_aPtrIndex[indj].LinkPtr;
							while (ptj < m_MaxIndex && m_aPtrIndex[ptj].LinkPtr>0 && ptj != m_aPtrIndex[ptj].LinkPtr) ptj = m_aPtrIndex[ptj].LinkPtr;
						}
						if (pti < ptj) m_aPtrIndex[ptj].LinkPtr = indi;
						else if (pti > ptj) m_aPtrIndex[pti].LinkPtr = indj;
					}
				}
			}
		}
		if (bNew) m_aPtrLine[i].index = m_NumIndex++;
		if (m_NumIndex >= m_MaxIndex) return BLOBERR_MAXINDEX;
	}
	if (bMaxLink) return BLOBERR_MAXLINK;
	return BLOBERR_NONE;
}

BOOL CMBlob::LabelBlobObj()
{
	int i, pt;

	////////////////////////////////////////
	m_NumBlob = 1;
	////////////////////////////////////////
	int numIndex = 0;
	BOOL bMaxBlob = FALSE;

	for (i = 1; i < m_NumIndex; i++)
	{
		if (m_aPtrIndex[i].LinkPtr == 0)
		{
			m_aPtrIndex[i].label = m_NumBlob++;
			if (m_NumBlob >= m_MaxBlob)
			{
				numIndex = i;
				bMaxBlob = TRUE;
				break;
			}

		}
	}

	//    if(bMaxBlob)
	//		m_NumIndex = numIndex;

	for (i = 1; i < m_NumIndex; i++)
	{
		if (m_aPtrIndex[i].LinkPtr == 0) continue;
		pt = m_aPtrIndex[i].LinkPtr;
		while (pt < m_MaxIndex && m_aPtrIndex[pt].LinkPtr>0 && pt != m_aPtrIndex[pt].LinkPtr) pt = m_aPtrIndex[pt].LinkPtr;
		m_aPtrIndex[i].label = m_aPtrIndex[pt].label;
	}

	if (bMaxBlob)
		return BLOBERR_MAXBLOB;
	else
		return BLOBERR_NONE;
}

void CMBlob::DrawLabel(LPBYTE pData)
{
	int i, j;
	LPBYTE pBuf;

	for (i = 0; i < m_NumLine; i++)
	{
		pBuf = pData + (m_ImgHeight - m_aPtrLine[i].y - 1) * m_StepWidth + m_aPtrLine[i].x1;
		for (j = m_aPtrLine[i].x1; j < m_aPtrLine[i].x2; j++)
		{
			int label = m_aPtrIndex[m_aPtrLine[i].index].label;
			if (label < 256)
				*pBuf++ = (BYTE)(label);   // input buffer가 BYTE형이므로 256미만의 값만으로 라벨링한다. 
		}
	}
}

void CMBlob::ExtractBlob()
{
	int i, lbl;

	for (i = 0; i < m_NumBlob; i++)
	{
		m_aPtrBlob[i].top = 65000;
		m_aPtrBlob[i].bottom = 0;
		m_aPtrBlob[i].left = 65000;
		m_aPtrBlob[i].right = 0;
		m_aPtrBlob[i].pxlSize = 0;
		m_aPtrBlob[i].Sx = 0;
		m_aPtrBlob[i].Sy = 0;
		m_aPtrBlob[i].Sxx = 0;
		m_aPtrBlob[i].Syy = 0;
		m_aPtrBlob[i].Sxy = 0;
		m_aPtrBlob[i].centerOfMass.x = 0;
		m_aPtrBlob[i].centerOfMass.y = 0;
		m_aPtrBlob[i].orientation = 0;
		m_aPtrBlob[i].state = 0;
	}

	memset(m_bRegBlobSeed, 0, sizeof(BOOL) * m_MaxBlob);

	double Sx = 0, Sy = 0, Sxx = 0, Syy = 0, Sxy = 0;

	for (i = 0; i < m_NumLine; i++)
	{
		lbl = m_aPtrIndex[m_aPtrLine[i].index].label;
		if (!m_bRegBlobSeed[lbl])
		{
			m_aPtrBlob[lbl].seed.x = m_aPtrLine[i].x1;
			m_aPtrBlob[lbl].seed.y = m_aPtrLine[i].y;
			m_bRegBlobSeed[lbl] = TRUE;
		}

		if (m_aPtrBlob[lbl].top > m_aPtrLine[i].y)
		{
			m_aPtrBlob[lbl].top = m_aPtrLine[i].y;
			m_aPtrBlob[lbl].rectpoint[1].x = m_aPtrLine[i].x1;
			m_aPtrBlob[lbl].rectpoint[1].y = m_aPtrLine[i].y;
		}
		if (m_aPtrBlob[lbl].bottom < m_aPtrLine[i].y)
		{
			m_aPtrBlob[lbl].bottom = m_aPtrLine[i].y;
			m_aPtrBlob[lbl].rectpoint[3].x = m_aPtrLine[i].x1;
			m_aPtrBlob[lbl].rectpoint[3].y = m_aPtrLine[i].y;
		}
		if (m_aPtrBlob[lbl].left > m_aPtrLine[i].x1)
		{
			m_aPtrBlob[lbl].left = m_aPtrLine[i].x1;
			m_aPtrBlob[lbl].rectpoint[0].x = m_aPtrLine[i].x1;
			m_aPtrBlob[lbl].rectpoint[0].y = m_aPtrLine[i].y;
		}
		if (m_aPtrBlob[lbl].right < m_aPtrLine[i].x2)
		{
			m_aPtrBlob[lbl].right = m_aPtrLine[i].x2;
			m_aPtrBlob[lbl].rectpoint[2].x = m_aPtrLine[i].x2;
			m_aPtrBlob[lbl].rectpoint[2].y = m_aPtrLine[i].y;
		}
		m_aPtrBlob[lbl].pxlSize += m_aPtrLine[i].x2 - m_aPtrLine[i].x1;

		for (int x = m_aPtrLine[i].x1; x < m_aPtrLine[i].x2; x++)
		{
			m_aPtrBlob[lbl].Sx += x;
			m_aPtrBlob[lbl].Sy += m_aPtrLine[i].y;
			m_aPtrBlob[lbl].Sxx += x * x;
			m_aPtrBlob[lbl].Syy += m_aPtrLine[i].y * m_aPtrLine[i].y;
			m_aPtrBlob[lbl].Sxy += x * m_aPtrLine[i].y;
		}

		m_aPtrBlob[lbl].label = lbl;
	}
}

void CMBlob::DrawBlob(LPBYTE pData, int lbl, BOOL bOriginLeftBottom)
{
	// 해당 lbl을 255로 칠하기..
	LPBYTE pBuf;

	for (int i = 0; i < m_NumLine; i++)
	{
		if (lbl == m_aPtrIndex[m_aPtrLine[i].index].label)
		{
			if (bOriginLeftBottom)
				pBuf = pData + (m_ImgHeight - m_aPtrLine[i].y - 1) * m_StepWidth + m_aPtrLine[i].x1;
			else
				pBuf = pData + m_aPtrLine[i].y * m_StepWidth + m_aPtrLine[i].x1;  // origin left,top

			for (int x = m_aPtrLine[i].x1; x < m_aPtrLine[i].x2; x++)
			{
				*pBuf++ = 255;
			}
		}
	}
}

void CMBlob::SetBlobChangeValue(int ind, int l, int t, int r, int b, int s)
{
	if (ind < m_NumBlob)
	{
		m_aPtrBlob[ind].top = t;
		m_aPtrBlob[ind].bottom = b;
		m_aPtrBlob[ind].left = l;
		m_aPtrBlob[ind].right = r;
		m_aPtrBlob[ind].pxlSize = s;
		m_aPtrBlob[ind].Sx = (l + r) / 2;
		m_aPtrBlob[ind].Sy = (t + b) / 2;
		m_aPtrBlob[ind].centerOfMass.x = (l + r) / 2;
		m_aPtrBlob[ind].centerOfMass.y = (t + b) / 2;
		m_aPtrBlob[ind].state = 0;
	}
}

void CMBlob::AddBlob(int l, int t, int r, int b, int s)
{
	if (l > r)swap(l, r);
	if (t > b)swap(t, b);

	if (l == r) {
		l -= 1; r += 1;
	}
	if (t == b) {
		t -= 1; b += 1;
	}

	m_aPtrBlob[m_NumBlob].top = t;
	m_aPtrBlob[m_NumBlob].bottom = b;
	m_aPtrBlob[m_NumBlob].left = l;
	m_aPtrBlob[m_NumBlob].right = r;
	m_aPtrBlob[m_NumBlob].pxlSize = s;
	m_aPtrBlob[m_NumBlob].Sx = (l + r) / 2;
	m_aPtrBlob[m_NumBlob].Sy = (t + b) / 2;
	m_aPtrBlob[m_NumBlob].Sxx = 0;
	m_aPtrBlob[m_NumBlob].Syy = 0;
	m_aPtrBlob[m_NumBlob].Sxy = 0;
	m_aPtrBlob[m_NumBlob].centerOfMass.x = (l + r) / 2;
	m_aPtrBlob[m_NumBlob].centerOfMass.y = (t + b) / 2;
	m_aPtrBlob[m_NumBlob].orientation = 0;
	m_aPtrBlob[m_NumBlob].state = 0;


	m_NumBlob++;
}

void CMBlob::BlobSizeControl(int ind, int size)
{
	int l, t, r, b,  pixel;
	int ol, ot, or , ob;

	ot = m_aPtrBlob[ind].top;
	ob = m_aPtrBlob[ind].bottom;
	ol = m_aPtrBlob[ind].left;
	or = m_aPtrBlob[ind].right;

	pixel = m_aPtrBlob[ind].pxlSize;

	l = m_aPtrBlob[ind].seed.x;
	t = m_aPtrBlob[ind].seed.y;
	r = int(l + sqrt(size));
	b = int(t + sqrt(size));

	SetBlobChangeValue(ind, l, t, r, b, size);

	AddBlob((r + or ) / 2, (b + ob) / 2, or , ob, size);
}

PSTBLOB CMBlob::GetBlob(int ind)
{
	double Mx, My, Mxy;
	if (ind < m_NumBlob)
	{
		Mx = m_aPtrBlob[ind].Sxx - (m_aPtrBlob[ind].Sx * m_aPtrBlob[ind].Sx) / m_aPtrBlob[ind].pxlSize;
		My = m_aPtrBlob[ind].Syy - (m_aPtrBlob[ind].Sy * m_aPtrBlob[ind].Sy) / m_aPtrBlob[ind].pxlSize;
		Mxy = m_aPtrBlob[ind].Sxy - (m_aPtrBlob[ind].Sx * m_aPtrBlob[ind].Sy) / m_aPtrBlob[ind].pxlSize;

		m_aPtrBlob[ind].centerOfMass.x = (int)(m_aPtrBlob[ind].Sx / m_aPtrBlob[ind].pxlSize);
		m_aPtrBlob[ind].centerOfMass.y = (int)(m_aPtrBlob[ind].Sy / m_aPtrBlob[ind].pxlSize);
		double pi = 3.14159265359;
		m_aPtrBlob[ind].orientation = atan2((Mx - My + sqrt((Mx - My) * (Mx - My) + 4 * Mxy * Mxy)), 2 * Mxy) * 180 / pi;

		return &(m_aPtrBlob[ind]);
	}
	return NULL;
}

void CMBlob::SetBlobStatus(int ind, int bstatus)
{
	if (ind < m_NumBlob)
		m_aPtrBlob[ind].state = bstatus;
}

void CMBlob::EraseLabel(LPBYTE pData)
{
	int x, y;
	LPBYTE pBuf;

	ZeroMemory(m_aMap, sizeof(WORD) * m_MaxIndex);

	for (y = m_RoiTop; y < m_RoiBottom; y++)
	{
		pBuf = pData + (m_ImgHeight - y - 1) * m_StepWidth + m_RoiLeft;
		for (x = m_RoiLeft; x < m_RoiRight; x++)
			m_aMap[*pBuf++]++;
	}
	for (x = 0; x < m_NumIndex; x++)
	{
		if (m_aMap[m_aPtrIndex[x].label] == 0) m_aPtrIndex[x].label = 0;
	}
}

int CMBlob::CheckAcceptable(LPBYTE pData)
{
	int i, x, y, count;
	LPBYTE pBuf;

	for (i = 0; i < m_NumIndex; i++) m_aPtrIndex[i].check = 0;

	for (i = 0; i < m_NumLine; i++)
	{
		y = m_aPtrLine[i].y;
		pBuf = pData + (m_ImgHeight - y - 1) * m_StepWidth + m_aPtrLine[i].x1;
		for (x = m_aPtrLine[i].x1; x <= m_aPtrLine[i].x2; x++)
		{
			if (*pBuf++ > 0) m_aPtrIndex[m_aPtrLine[i].index].check = 1;
		}
	}

	ZeroMemory(m_aMap, sizeof(WORD) * m_MaxIndex);

	for (i = 1; i < m_NumIndex; i++)
		if (m_aPtrIndex[i].check > 0) m_aMap[m_aPtrIndex[i].label]++;

	x = 1;
	for (i = 1; i < m_NumBlob; i++)
	{
		if (m_aMap[i] > 0) m_aMap[i] = x++;
	}
	count = x;

	m_aMap[0] = 0;
	for (i = 1; i < m_NumIndex; i++)
		m_aPtrIndex[i].check = m_aMap[m_aPtrIndex[i].label];


	m_bCheckDone = TRUE;

	return count;
}

void CMBlob::DrawCheck(LPBYTE pData, int numdraw)
{
	int i, j, pxl, cutLevel;
	LPBYTE pBuf;

	if (numdraw < 0) cutLevel = m_NumBlob;
	else cutLevel = numdraw;

	for (i = 0; i < m_NumLine; i++)
	{
		pBuf = pData + (m_ImgHeight - m_aPtrLine[i].y - 1) * m_StepWidth + m_aPtrLine[i].x1;
		pxl = m_aPtrIndex[m_aPtrLine[i].index].check;
		if (pxl >= cutLevel) continue;
		for (j = m_aPtrLine[i].x1; j < m_aPtrLine[i].x2; j++)
		{
			if (pxl < cutLevel)
				*pBuf++ = (BYTE)pxl;
		}
	}
}


PSTLINE CMBlob::GetLine(int ind)
{
	if (ind < m_NumLine) return &(m_aPtrLine[ind]);
	return NULL;
}


BOOL CMBlob::ExtractBlobFromCheck(int numBlob)
{
	if (!m_bCheckDone)
		return FALSE;

	int i;
	for (i = 0; i < m_NumBlob; i++)
	{
		m_aPtrBlob[i].top = 65000;
		m_aPtrBlob[i].bottom = 0;
		m_aPtrBlob[i].left = 65000;
		m_aPtrBlob[i].right = 0;
		m_aPtrBlob[i].pxlSize = 0;
	}

	for (i = 0; i < m_NumLine; i++)
	{
		WORD lbl = m_aPtrIndex[m_aPtrLine[i].index].check;
		if (lbl >= numBlob) continue;

		if (m_aPtrBlob[lbl].top > m_aPtrLine[i].y) m_aPtrBlob[lbl].top = m_aPtrLine[i].y;
		if (m_aPtrBlob[lbl].bottom < m_aPtrLine[i].y) m_aPtrBlob[lbl].bottom = m_aPtrLine[i].y;
		if (m_aPtrBlob[lbl].left > m_aPtrLine[i].x1) m_aPtrBlob[lbl].left = m_aPtrLine[i].x1;
		if (m_aPtrBlob[lbl].right < m_aPtrLine[i].x2) m_aPtrBlob[lbl].right = m_aPtrLine[i].x2;
		m_aPtrBlob[lbl].pxlSize += m_aPtrLine[i].x2 - m_aPtrLine[i].x1;
	}

	if (m_NumBlob >= numBlob)
		m_NumBlob = numBlob;


	return TRUE;
}

int __cdecl CompareByBlob(const void* elem1, const void* elem2)
{
	STBLOB* p1 = (STBLOB*)elem1;
	STBLOB* p2 = (STBLOB*)elem2;

	if (p1->pxlSize < p2->pxlSize) return 1;
	else if (p1->pxlSize == p2->pxlSize) return 0;
	else  return -1;

	return 0;
}

void CMBlob::SortingBlob()
{
	qsort(static_cast<void*>(&m_aPtrBlob[1]), m_NumBlob - 1, sizeof(STBLOB), CompareByBlob);
}

void CMBlob::SetRealPoint(LPPOINT pPoint, int pos)
{
	if (pos >= 0 && pos < 4)
	{
		pPoint->x = ((pPoint->x < 0) ? 0 : pPoint->x);
		pPoint->x = ((pPoint->x > m_ImgWidth) ? m_ImgWidth - 1 : pPoint->x);
		pPoint->y = ((pPoint->y < 0) ? 0 : pPoint->y);
		pPoint->y = ((pPoint->y > m_ImgHeight) ? m_ImgHeight - 1 : pPoint->y);

		m_pPtRect[pos] = *pPoint;
		m_pVertx[pos] = float(pPoint->x);
		m_pVerty[pos] = float(pPoint->y);
	}
}


BOOL CMBlob::PointInPolygon(float testx, float testy)
{
	if (m_pPtRect[0].x + m_pPtRect[0].y + m_pPtRect[1].x + m_pPtRect[1].y == 0) return TRUE;

	int   polySides = 4, i, j = polySides - 1;
	BOOL  oddNodes = FALSE;

	for (i = 0; i < polySides; i++)
	{
		if (m_pVerty[j] - m_pVerty[i] != 0 && (m_pVerty[i] < testy && m_pVerty[j] >= testy || m_pVerty[j] < testy && m_pVerty[i] >= testy))
		{
			if (m_pVertx[i] + ((testy - m_pVerty[i]) / (m_pVerty[j] - m_pVerty[i])) * (m_pVertx[j] - m_pVertx[i]) < testx) 	oddNodes = !oddNodes;

		}
		j = i;
	}

	return oddNodes;
}
