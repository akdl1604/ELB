/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "../LET_AlignClient.h"
#include "HalconProcessing.h"
#include "INIReader.h"
#include "JXINIFile.h"
#include <sstream>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

template <typename T>
  std::string NumberToString ( T Number )
  {
     std::ostringstream ss;
     ss << Number;
     return ss.str();
  }


static void MyHalconExceptionHandler(const Halcon::HException& except)
{
  throw except;	
}


Hobject CHalconProcessing::IplImageToHImage(IplImage *pImage)
{
	Hobject Hobj;
	if (3 == pImage->nChannels)
	{
		IplImage *pImageRed, *pImageGreen, *pImageBlue;
		pImageRed = cvCreateImage(cvGetSize(pImage), IPL_DEPTH_8U, 1);
		pImageGreen = cvCreateImage(cvGetSize(pImage), IPL_DEPTH_8U, 1);
		pImageBlue = cvCreateImage(cvGetSize(pImage), IPL_DEPTH_8U, 1);
		cvSplit(pImage, pImageBlue, pImageGreen, pImageRed, NULL);
		gen_image3(&Hobj, "byte", pImage->width, pImage->height, (Hlong)(pImageRed->imageData), (Hlong)(pImageGreen->imageData), (Hlong)(pImageBlue->imageData));
		cvReleaseImage(&pImageRed);
		cvReleaseImage(&pImageGreen);
		cvReleaseImage(&pImageBlue);
	}
	if (1 == pImage->nChannels)
	{
		gen_image1(&Hobj, "byte", pImage->width, pImage->height, (Hlong)(pImage->imageData));
	}
	return Hobj;
}

IplImage* CHalconProcessing::HImageToIplImage(Hobject &Hobj)
{
	IplImage *pImage=NULL;
	HTuple htChannels;
	char cType[MAX_STRING];
	Hlong width,height;

	width = height = 0;

	convert_image_type(Hobj, &Hobj, "byte");
	count_channels(Hobj, &htChannels);

	if (htChannels[0].I() == 1)
	{
		unsigned char *ptr;
		get_image_pointer1(Hobj, (Hlong *)&ptr, cType, &width, &height);
		pImage = cvCreateImage(cvSize(int(width),int( height)), IPL_DEPTH_8U, 1);
		memcpy(pImage->imageData, ptr, pImage->imageSize);
	}

	if (htChannels[0].I() == 3)
	{
		unsigned char *ptrRed, *ptrGreen, *ptrBlue;
		ptrRed = ptrGreen = ptrBlue = NULL;

		get_image_pointer3(Hobj, (Hlong *)&ptrRed, (Hlong *)&ptrGreen, (Hlong *)&ptrBlue, cType, &width, &height);

		IplImage *pImageRed, *pImageGreen, *pImageBlue;
		pImage = cvCreateImage(cvSize(int(width),int( height)), IPL_DEPTH_8U, 3);
		pImageRed = cvCreateImage(cvSize(int(width),int( height)), IPL_DEPTH_8U, 1);
		pImageGreen = cvCreateImage(cvSize(int(width),int( height)), IPL_DEPTH_8U, 1);
		pImageBlue = cvCreateImage(cvSize(int(width),int( height)), IPL_DEPTH_8U, 1);
		memcpy(pImageRed->imageData, ptrRed, pImageRed->imageSize);
		memcpy(pImageGreen->imageData, ptrGreen, pImageGreen->imageSize);
		memcpy(pImageBlue->imageData, ptrBlue, pImageBlue->imageSize);
		cvMerge(pImageBlue, pImageGreen, pImageRed, NULL, pImage);
		cvReleaseImage(&pImageRed);
		cvReleaseImage(&pImageGreen);
		cvReleaseImage(&pImageBlue);
	}
	return pImage;
}

void CHalconProcessing::HContourToIplImage(Hobject &Hobj,Hobject &Himage)
{
	Hobject Region;

	gen_region_contour_xld(Hobj, &Region, "margin");
    paint_region(Region, Himage, &m_ContourImageResult, 255, "margin");
}

CHalconProcessing::CHalconProcessing()
{
#ifdef _CPP_USE_HALCON
	SetSystem("border_shape_models", "false");
#else
	set_system("border_shape_models", "false");
#endif
	

	m_ptMarkPos.x = -1;
	m_ptMarkPos.y = -1;
	m_dModelAngle = 0;
	m_dModelAngleRadian=0;
	m_dModelScore = 0;
	m_bModelRead = false;
	m_bCalibration = false;
	m_bLinearCalibraion = false;
	m_bNccSelect=false;
	m_ptMarkOffset.x = 0;
	m_ptMarkOffset.y = 0;
	m_dResolutionX=0.0;
	m_dResolutionY=0.0;
	m_nSearchAngleRange = 30;
	m_nSearchAngleStep = 1.7297;

	m_ptRotateCenter=PointF(0, 0);
	memset( m_dbPrealignResult, 0, sizeof(m_dbPrealignResult) );

	hv_ModelId = NULL;
	m_hHalHwndView = NULL;

	m_nFatOffsetWidth=0;
	m_nFatOffsetHeight=0;

	memset(m_dNa,0,sizeof(double)*2);
	memset(m_dNb,0,sizeof(double)*2);

	m_ReadNumLevel = 0;
	m_ReadStartAngle = -0.39;
	m_ReadEndAngle = 0.79;
	m_ReadStepAngle = -1;//0.0982;
	m_ReadOptimize = "none";
	m_ReadMetric = "use_polarity";
	m_ReadContrast = -1;//60;
	m_ReadMinContrast = -1;//10;
	//HTK 2022-03-30 Halcon Scale Mark Search 기능 추가
	m_ReadSmallScale = 0.2;

	m_SearchNumMatch = 1;
	m_SearchStartAngle = -0.39;
	m_SearchEndAngle = 0.79;
	m_SearchMinscore = 0.5;
	m_SearchGreedness = 0.9;
	m_SearchSubPixel ="false";
	m_SearchMethod = 0;
	m_bPreProcess = FALSE;


#ifdef _CPP_USE_HALCON
	SetCheck("~give_error");
#else
	set_check("~give_error");
#endif
	
	m_lWindowID = -1;
	Halcon::HException::InstallHHandler(&MyHalconExceptionHandler);
}

CHalconProcessing::~CHalconProcessing()
{
	if(m_bModelRead)
	{
#ifdef _CPP_USE_HALCON
		if(m_bNccSelect) ClearNccModel(hv_ModelId);
		else ClearShapeModel(hv_ModelId);
#else
		if(m_bNccSelect) clear_ncc_model(hv_ModelId);
		else clear_shape_model(hv_ModelId);	
#endif

		if(m_bLinearCalibraion)
		{
			for(int i=0;i<m_cvLinearCalib.size();i++)
				m_cvLinearCalib.at(i).release();

			m_cvLinearCalib.clear();
		}	
	}

	m_pFLinePtr.clear();
	m_pSLinePtr.clear();
	m_pARLinePtr.clear();
	m_arrContourPoint.clear();
	m_pQRPosPtr.clear();

	if(m_lWindowID != -1)
		close_window(m_lWindowID);
}

void CHalconProcessing::HalconOpenWindow(HWND m_hWnd)
{
	//Hlong lWWindowID;
	//set_window_attr("border_width", 0);
	//lWWindowID=(Hlong)m_hWnd;
	//set_check("~father");
	//open_window(16, 16, 600, 600,lWWindowID, "visible", "", &m_lWindowID);
	//set_check("father");

	if (HDevWindowStack::IsOpen())    close_window(HDevWindowStack::Pop());
	set_window_attr("background_color","black");
	open_window(0,0,600,600,0,"","",&m_lWindowID);
	HDevWindowStack::Push(m_lWindowID);
}
UPoint<double> CHalconProcessing::halcon_GetResultPos()
{
	return m_ptMarkPos;
}
double CHalconProcessing::halcon_GetResultAngle()
{
	
	return m_dModelAngle;
}
double CHalconProcessing::halcon_GetResultRadianAngle()
{

	return m_dModelAngleRadian;
}
double CHalconProcessing::halcon_GetResultScore()
{
	return m_dModelScore;
}
void CHalconProcessing::halcon_SetShapeModelOrigin(double xoffset,double yoffset)
{

	if(m_bModelRead)
	{
#ifdef _CPP_USE_HALCON
		if( m_bNccSelect )	SetShapeModelOrigin( hv_ModelId,yoffset, xoffset);
		else				SetNccModelOrigin( hv_ModelId,yoffset, xoffset);
#else
		if( m_bNccSelect )	set_ncc_model_origin(hv_ModelId, yoffset, xoffset);
		else				set_shape_model_origin(hv_ModelId, yoffset, xoffset);
#endif
	} 
}
void CHalconProcessing::halcon_SetMarkOffset(int xoffset,int yoffset)
{
	m_ptMarkOffset.x = xoffset;
	m_ptMarkOffset.y = yoffset;
}
BOOL CHalconProcessing::halcon_ReadModel(CString modelname, int nMinContrast, int nHighContrast)
{
	BOOL bStatus=FALSE;

	if(m_SearchMethod==1)
	{
		return halcon_ReadNccModel(modelname);
	}
	//HTK 2022-03-30 Halcon Scale Mark Search 기능 추가
	else if (m_ReadSmallScale > 0.1) return halcon_ReadScaleModel(modelname,"NONE");

	try 
	{
		CPoint ptCenterOffset;
		CString name = modelname.Left(modelname.GetLength() - 3) + "dat";
		CFile file ;

		if ( file.Open( _T(name),  CFile::modeRead  ) )
		{
			file.Read( &ptCenterOffset, sizeof(CPoint) ) ;
			int i = 0;
			for(i = 0; i < 4; i++)
			{
				if(	!file.Read( &m_cvTrans_mat[i], UINT( m_cvTrans_mat[i].total() * m_cvTrans_mat[i].elemSize())))
					break;
			}	

			if(i == 4) m_bCalibration = true;

			file.Read( &m_ptRotateCenter, sizeof(PointF) );
			file.Read( &m_dResolutionX, sizeof(double) );
			file.Read( &m_dResolutionY, sizeof(double) );

			m_ptMarkOffset.x = ptCenterOffset.x;
			m_ptMarkOffset.y = ptCenterOffset.y;

			file.Close() ;
		}
	}
	catch(...) 
	{
	}

#ifdef _CPP_USE_HALCON
	ReadImage(&ho_TemplateImage, (LPCTSTR)modelname);
	CreateShapeModel(ho_TemplateImage, 2, HTuple(-15).TupleRad(), HTuple(15).TupleRad(),	HTuple(0.7297).TupleRad(),//
		(HTuple("none").Append("pregeneration")), "use_polarity",((HTuple(25).Append(35)).Append(9)),"auto" , &hv_ModelId);//
	GetShapeModelContours(&ho_ModelContours, hv_ModelId, 1);
#else

	if(m_bModelRead)
	{
#ifdef _CPP_USE_HALCON
		if(m_bNccSelect) ClearNccModel(hv_ModelId);
		else ClearShapeModel(hv_ModelId);
#else
		if(m_bNccSelect)	clear_ncc_model(hv_ModelId);
		else				clear_shape_model(hv_ModelId);	
#endif
	}

	read_image(&ho_TemplateImage, modelname);
	
	HTuple width, height;
	get_image_size( ho_TemplateImage, &width, &height);

	m_nFatWidth = int(width[0].D());
	m_nFatHeight = int(height[0].D());	

	//HTK 2022-04-13 Mark Angle Search
	create_shape_model(ho_TemplateImage,m_ReadNumLevel, HTuple(m_ReadStartAngle).Rad(), HTuple(m_ReadEndAngle- m_ReadStartAngle).Rad(),m_ReadStepAngle!=-1?HTuple(m_ReadStepAngle).Rad():"auto" ,
		m_ReadOptimize=="none"?"auto":m_ReadOptimize,(LPCTSTR) m_ReadMetric, (Hlong)m_ReadContrast!=-1?(Hlong)m_ReadContrast:HTuple("auto"), (Hlong)m_ReadMinContrast!=-1?(Hlong)m_ReadMinContrast:HTuple("auto"), &hv_ModelId);


	get_shape_model_contours(&ho_ModelContours, hv_ModelId, 1);

	m_nFatOffsetWidth = 0;
	m_nFatOffsetHeight = 0;

#endif

	m_bModelRead = true;
	m_bNccSelect = false;

	return bStatus;
}
BOOL CHalconProcessing::halcon_ReadModel(BYTE *psrc, int w,int h, int nMinContrast, int nHighContrast)
{
	BOOL bStatus=FALSE;

	if (m_SearchMethod == 1)
	{
		return halcon_ReadNccModel(psrc, w, h);
	}
	//HTK 2022-03-30 Halcon Scale Mark Search 기능 추가
	else if (m_ReadSmallScale > 0.1) return halcon_ReadScaleModel(psrc, 0, w, h);

#ifdef _CPP_USE_HALCON
	ReadImage(&ho_TemplateImage, (LPCTSTR)modelname);
	CreateShapeModel(ho_TemplateImage, 2, HTuple(0).TupleRad(), HTuple(360).TupleRad(),	HTuple(4.0297).TupleRad(),//
		(HTuple("none").Append("pregeneration")), "use_polarity",((HTuple(25).Append(35)).Append(9)),"auto" , &hv_ModelId);//
	GetShapeModelContours(&ho_ModelContours, hv_ModelId, 1);
#else

	if(m_bModelRead)
	{
#ifdef _CPP_USE_HALCON
		if(m_bNccSelect) ClearNccModel(hv_ModelId);
		else ClearShapeModel(hv_ModelId);
#else
		if(m_bNccSelect)	clear_ncc_model(hv_ModelId);
		else				clear_shape_model(hv_ModelId);	
#endif
	}

	gen_image1(&ho_TemplateImage,"byte",w,h,(Hlong)psrc);		//Image Load

	m_nFatWidth = w;
	m_nFatHeight = h;
	
	//HTK 2022-04-13 Mark Angle Search
	create_shape_model(ho_TemplateImage,m_ReadNumLevel, HTuple(m_ReadStartAngle).Rad(), HTuple(m_ReadEndAngle-m_ReadStartAngle).Rad(),m_ReadStepAngle!=-1?HTuple(m_ReadStepAngle).Rad():"auto" ,
			m_ReadOptimize=="none"?"auto":m_ReadOptimize,(LPCTSTR) m_ReadMetric, (Hlong)m_ReadContrast!=-1?(Hlong)m_ReadContrast:HTuple("auto"), (Hlong)m_ReadMinContrast!=-1?(Hlong)m_ReadMinContrast:HTuple("auto"), &hv_ModelId);
		
	get_shape_model_contours(&ho_ModelContours, hv_ModelId, 1);
//	HContourToIplImage(ho_ModelContours,ho_TemplateImage);

	m_nFatOffsetWidth = 0;
	m_nFatOffsetHeight = 0;

#endif	

	m_bModelRead = true;
	m_bNccSelect = false;

	return bStatus;
}
//HTK 2022-03-30 Halcon Scale Mark Search 기능 추가
BOOL CHalconProcessing::halcon_ReadScaleModel(BYTE* psrc, BYTE* pMask, int w, int h)
{

	BOOL bStatus = FALSE;

	//HTK 2022-04-02 Mask처리시 이미지 사이즈 변경되는 문제 디버깅
	HTuple SelectNum, hv_Row1, hv_Column1, hv_Area, hv_Row2, hv_Column2, hv_Area2;
	Hobject hoMaskImage, hoGrayImage, hoImageReduced, ho_ModelRegion, BrightRegions;
	Hobject Region, ConnectedRegions, FilledRegions, InspRegions, ModelRegion;
	Hlong nNum;


	if (m_bModelRead)
	{
		if (m_bNccSelect)	clear_ncc_model(hv_ModelId);
		else				clear_shape_model(hv_ModelId);
	}

	gen_image1(&ho_TemplateImage, "byte", w, h, (Hlong)psrc);		//Image Load
	if (pMask == 0)
	{
		gen_image_const(&hoMaskImage, "byte", w, h);
	}
	else gen_image1(&hoMaskImage, "byte", w, h, (Hlong)pMask);		//Image Load
	gen_rectangle1(&ho_ModelRegion, 0, 0, double(h), double(w));

	threshold(hoMaskImage, &BrightRegions, 200, 255);
	connection(BrightRegions, &ConnectedRegions);
	//fill_up (ConnectedRegions, &FilledRegions); //외곽 전체를 마스킹 할경우 내부를 채워 모델 생성이 안됨
	//select_shape (FilledRegions, &InspRegions, "area", "and", 10, 99999999999);	
	select_shape(ConnectedRegions, &InspRegions, "area", "and", 10, 99999999999);

	difference(ho_ModelRegion, InspRegions, &ModelRegion);

	count_obj(ModelRegion, &SelectNum);
	tuple_int(SelectNum, &nNum);

	if (nNum > 0)	reduce_domain(ho_TemplateImage, ModelRegion, &hoImageReduced);
	else reduce_domain(ho_TemplateImage, ho_ModelRegion, &hoImageReduced);

	m_nFatWidth = w;
	m_nFatHeight = h;

	area_center(hoImageReduced, &hv_Area, &hv_Row1, &hv_Column1);
	//HTK 2022-04-02 Mask처리시 이미지 사이즈 변경되는 문제 디버깅
	area_center(ho_TemplateImage, &hv_Area2, &hv_Row2, &hv_Column2);

	create_scaled_shape_model(hoImageReduced, m_ReadNumLevel, HTuple(m_ReadStartAngle).Rad(), HTuple(m_ReadEndAngle).Rad(), m_ReadStepAngle != -1 ? HTuple(m_ReadStepAngle).Rad() : "auto",
		1 - m_ReadSmallScale, 1 + m_ReadSmallScale, 0.0135, m_ReadOptimize == "none" ? "auto" : m_ReadOptimize, (LPCTSTR)m_ReadMetric, (Hlong)m_ReadContrast != -1 ? (Hlong)m_ReadContrast : HTuple("auto"), (Hlong)m_ReadMinContrast != -1 ? (Hlong)m_ReadMinContrast : HTuple("auto"), &hv_ModelId);

	get_shape_model_contours(&ho_ModelContours, hv_ModelId, 1);

	//HTK 2022-04-02 Mask처리시 이미지 사이즈 변경되는 문제 디버깅
	m_nFatOffsetWidth = hv_Column2[0].D() - hv_Column1[0].D();
	m_nFatOffsetHeight = hv_Row2[0].D() - hv_Row1[0].D();

	m_bModelRead = true;
	m_bNccSelect = false;

	if (getHalconModelID().Num() == 0)
	{
		// 2022-08-23 KBJ
		//AfxMessageBox("Please create the model again and reset the masking area!");
		bStatus = FALSE;
	}

	return bStatus;
}
//HTK 2022-03-30 Halcon Scale Mark Search 기능 추가
BOOL CHalconProcessing::halcon_ReadScaleModel(CString modelname, CString modelMaskname)
{
	BOOL bStatus = FALSE;

	HTuple SelectNum, hv_Row1, hv_Column1, hv_Area, hv_Row2, hv_Column2, hv_Area2;
	Hobject hoMaskImage, hoGrayImage, hoImageReduced, ho_ModelRegion, BrightRegions;
	Hobject Region, ConnectedRegions, FilledRegions, InspRegions, ModelRegion;
	Hlong nNum;

	if (m_bModelRead)
	{
#ifdef _CPP_USE_HALCON
		if (m_bNccSelect) ClearNccModel(hv_ModelId);
		else ClearShapeModel(hv_ModelId);
#else
		if (m_bNccSelect)	clear_ncc_model(hv_ModelId);
		else				clear_shape_model(hv_ModelId);
#endif
	}

	HTuple width, height;
	read_image(&ho_TemplateImage, modelname);
	get_image_size(ho_TemplateImage, &width, &height);

	if (modelMaskname == "NONE")	gen_image_const(&hoMaskImage, "byte", width, height);
	else	read_image(&hoMaskImage, modelMaskname);	

	m_nFatWidth = int(width[0].D());
	m_nFatHeight = int(height[0].D());

	gen_rectangle1(&ho_ModelRegion, 0, 0, double(m_nFatHeight), double(m_nFatWidth));

	threshold(hoMaskImage, &BrightRegions, 200, 255);
	connection(BrightRegions, &ConnectedRegions);
	//fill_up (ConnectedRegions, &FilledRegions); //외곽 전체를 마스킹 할경우 내부를 채워 모델 생성이 안됨
	//select_shape (FilledRegions, &InspRegions, "area", "and", 10, 99999999999);	
	select_shape(ConnectedRegions, &InspRegions, "area", "and", 10, 99999999999);
	difference(ho_ModelRegion, InspRegions, &ModelRegion);

	count_obj(ModelRegion, &SelectNum);
	tuple_int(SelectNum, &nNum);

	if (nNum > 0)	reduce_domain(ho_TemplateImage, ModelRegion, &hoImageReduced);
	else reduce_domain(ho_TemplateImage, ho_ModelRegion, &hoImageReduced);


	area_center(hoImageReduced, &hv_Area, &hv_Row1, &hv_Column1);
	//HTK 2022-04-02 Mask처리시 이미지 사이즈 변경되는 문제 디버깅
	area_center(ho_TemplateImage, &hv_Area2, &hv_Row2, &hv_Column2);

	create_scaled_shape_model(hoImageReduced, m_ReadNumLevel, HTuple(m_ReadStartAngle).Rad(), HTuple(m_ReadEndAngle).Rad(), m_ReadStepAngle != -1 ? HTuple(m_ReadStepAngle).Rad() : "auto",
		1 - m_ReadSmallScale, 1 + m_ReadSmallScale, 0.0135, m_ReadOptimize == "none" ? "auto" : m_ReadOptimize, (LPCTSTR)m_ReadMetric, (Hlong)m_ReadContrast != -1 ? (Hlong)m_ReadContrast : HTuple("auto"), (Hlong)m_ReadMinContrast != -1 ? (Hlong)m_ReadMinContrast : HTuple("auto"), &hv_ModelId);

	get_shape_model_contours(&ho_ModelContours, hv_ModelId, 1);

	//HTK 2022-04-02 Mask처리시 이미지 사이즈 변경되는 문제 디버깅
	m_nFatOffsetWidth = hv_Column2[0].D() - hv_Column1[0].D();
	m_nFatOffsetHeight = hv_Row2[0].D() - hv_Row1[0].D();

	m_bModelRead = true;
	m_bNccSelect = false;

	if (getHalconModelID().Num() == 0)
	{
		// 2022-08-23 KBJ
		// AfxMessageBox("Please create the model again and reset the masking area!");
		bStatus = FALSE;
	}

	return bStatus;
}
//HTK 2022-03-30 Halcon Scale Mark Search 기능 추가
BOOL CHalconProcessing::halcon_ReadModelNew(BYTE* psrc, BYTE* pMask, int w, int h)
{
	if (m_SearchMethod == 1)
	{
		return halcon_ReadNccModel(psrc, w, h);
	}
	else if (m_ReadSmallScale > 0.1) return halcon_ReadScaleModel(psrc, pMask, w, h);

	BOOL bStatus = FALSE;

	HTuple SelectNum, hv_Row1, hv_Column1, hv_Area, hv_Row2, hv_Column2, hv_Area2;
	Hobject hoMaskImage, hoGrayImage, hoImageReduced, ho_ModelRegion, BrightRegions;
	Hobject Region, ConnectedRegions, FilledRegions, InspRegions, ModelRegion;
	Hlong nNum;

	if (m_bModelRead)
	{
		if (m_bNccSelect)	clear_ncc_model(hv_ModelId);
		else				clear_shape_model(hv_ModelId);
	}

	gen_image1(&ho_TemplateImage, "byte", w, h, (Hlong)psrc);		//Image Load
	gen_image1(&hoMaskImage, "byte", w, h, (Hlong)pMask);			//Image Load
	gen_rectangle1(&ho_ModelRegion, 0, 0, double(h), double(w));

	threshold(hoMaskImage, &BrightRegions, 200, 255);
	connection(BrightRegions, &ConnectedRegions);
	//fill_up (ConnectedRegions, &FilledRegions); //외곽 전체를 마스킹 할경우 내부를 채워 모델 생성이 안됨
	//select_shape (FilledRegions, &InspRegions, "area", "and", 10, 99999999999);	
	select_shape(ConnectedRegions, &InspRegions, "area", "and", 10, 99999999999);

	difference(ho_ModelRegion, InspRegions, &ModelRegion);

	count_obj(ModelRegion, &SelectNum);
	tuple_int(SelectNum, &nNum);

	if (nNum > 0)	reduce_domain(ho_TemplateImage, ModelRegion, &hoImageReduced);
	else reduce_domain(ho_TemplateImage, ho_ModelRegion, &hoImageReduced);

	m_nFatWidth = w;
	m_nFatHeight = h;

	area_center(hoImageReduced, &hv_Area, &hv_Row1, &hv_Column1);
	//HTK 2022-04-02 Mask처리시 이미지 사이즈 변경되는 문제 디버깅
	area_center(ho_TemplateImage, &hv_Area2, &hv_Row2, &hv_Column2);

	//HTK 2022-04-13 Mark Angle Search
	create_shape_model(hoImageReduced, m_ReadNumLevel, HTuple(m_ReadStartAngle).Rad(), HTuple(m_ReadEndAngle- m_ReadStartAngle).Rad()  - HTuple(m_ReadStartAngle).Rad(), m_ReadStepAngle != -1 ? HTuple(m_ReadStepAngle).Rad() : "auto",
		m_ReadOptimize == "none" ? "auto" : m_ReadOptimize, (LPCTSTR)m_ReadMetric, (Hlong)m_ReadContrast != -1 ? (Hlong)m_ReadContrast : HTuple("auto"), (Hlong)m_ReadMinContrast != -1 ? (Hlong)m_ReadMinContrast : HTuple("auto"), &hv_ModelId);

	get_shape_model_contours(&ho_ModelContours, hv_ModelId, 1);

	//HTK 2022-04-02 Mask처리시 이미지 사이즈 변경되는 문제 디버깅
	m_nFatOffsetWidth =  hv_Column2[0].D() - hv_Column1[0].D();
	m_nFatOffsetHeight =  hv_Row2[0].D() - hv_Row1[0].D();

	m_bModelRead = true;
	m_bNccSelect = false;

	if (getHalconModelID().Num() == 0)
	{
		// 2022-08-23 KBJ
		// AfxMessageBox("Please create the model again and reset the masking area!");
		bStatus = FALSE;
	}


	return bStatus;
}
//HTK 2022-03-30 Halcon Scale Mark Search 기능 추가
BOOL CHalconProcessing::halcon_ReadModelNew(CString modelname, CString modelMaskname)
{
	if (m_SearchMethod == 1)
	{
		return halcon_ReadNccModel(modelname);
	}
	else if (m_ReadSmallScale > 0.1) return halcon_ReadScaleModel(modelname, modelMaskname);

	BOOL bStatus = FALSE;

	HTuple SelectNum, hv_Row1, hv_Column1, hv_Area, hv_Row2, hv_Column2, hv_Area2;
	Hobject hoMaskImage, hoGrayImage, hoImageReduced, ho_ModelRegion, BrightRegions;
	Hobject Region, ConnectedRegions, FilledRegions, InspRegions, ModelRegion;
	Hlong nNum;	

	if (m_bModelRead)
	{
#ifdef _CPP_USE_HALCON
		if (m_bNccSelect) ClearNccModel(hv_ModelId);
		else ClearShapeModel(hv_ModelId);
#else
		if (m_bNccSelect)	clear_ncc_model(hv_ModelId);
		else				clear_shape_model(hv_ModelId);
#endif
}

	read_image(&ho_TemplateImage, modelname);

	HTuple width, height;
	get_image_size(ho_TemplateImage, &width, &height);

	m_nFatWidth = int(width[0].D());
	m_nFatHeight = int(height[0].D());

	if (modelMaskname == "NONE")	gen_image_const(&hoMaskImage, "byte", width, height);
	else 							read_image(&hoMaskImage, modelMaskname);	

	gen_rectangle1(&ho_ModelRegion, 0, 0, double(m_nFatHeight), double(m_nFatWidth));

	threshold(hoMaskImage, &BrightRegions, 200, 255);
	connection(BrightRegions, &ConnectedRegions);
	//fill_up (ConnectedRegions, &FilledRegions); //외곽 전체를 마스킹 할경우 내부를 채워 모델 생성이 안됨
	//select_shape (FilledRegions, &InspRegions, "area", "and", 10, 99999999999);	
	select_shape(ConnectedRegions, &InspRegions, "area", "and", 10, 99999999999);
	difference(ho_ModelRegion, InspRegions, &ModelRegion);

	count_obj(ModelRegion, &SelectNum);
	tuple_int(SelectNum, &nNum);

	if (nNum > 0)	reduce_domain(ho_TemplateImage, ModelRegion, &hoImageReduced);
	else reduce_domain(ho_TemplateImage, ho_ModelRegion, &hoImageReduced);


	area_center(hoImageReduced, &hv_Area, &hv_Row1, &hv_Column1);
	area_center(ho_TemplateImage, &hv_Area2, &hv_Row2, &hv_Column2);

	//HTK 2022-04-13 Mark Angle Search
	create_shape_model(hoImageReduced, m_ReadNumLevel, HTuple(m_ReadStartAngle).Rad(), HTuple(m_ReadEndAngle- m_ReadStartAngle).Rad(), m_ReadStepAngle != -1 ? HTuple(m_ReadStepAngle).Rad() : "auto",
		m_ReadOptimize == "none" ? "auto" : m_ReadOptimize, (LPCTSTR)m_ReadMetric, (Hlong)m_ReadContrast != -1 ? (Hlong)m_ReadContrast : HTuple("auto"), (Hlong)m_ReadMinContrast != -1 ? (Hlong)m_ReadMinContrast : HTuple("auto"), &hv_ModelId);

	get_shape_model_contours(&ho_ModelContours, hv_ModelId, 1);


	m_nFatOffsetWidth = hv_Column2[0].D() - hv_Column1[0].D();
	m_nFatOffsetHeight = hv_Row2[0].D() - hv_Row1[0].D();

	m_bModelRead = true;
	m_bNccSelect = false;

	if (getHalconModelID().Num() == 0)
	{
		// 2022-08-23 KBJ
		// AfxMessageBox("Please create the model again and reset the masking area!");
		bStatus = FALSE;
	}

	return bStatus;
}
BOOL CHalconProcessing::halcon_ReadNccModel(BYTE *ptemplateSrc,int w,int h)
{
	BOOL bStatus=FALSE;	

	if(m_SearchMethod==0)
	{
		return halcon_ReadModel(ptemplateSrc,  w, h);
	}

	if(m_bModelRead)
	{
#ifdef _CPP_USE_HALCON
		if(m_bNccSelect) ClearNccModel(hv_ModelId);
		else ClearShapeModel(hv_ModelId);
#else
		if(m_bNccSelect)	clear_ncc_model(hv_ModelId);
		else				clear_shape_model(hv_ModelId);	
#endif
	}

	gen_image1(&ho_TemplateImage,"byte",w,h,(Hlong)ptemplateSrc);		//Image Load


#ifdef _CPP_USE_HALCON
	CreateNccModel(ho_TemplateImage, "auto", HTuple(0).TupleRad(), HTuple(360).TupleRad(), "auto", "use_polarity", &hv_ModelId);
#else

	try
	{
		m_nFatWidth = w;
		m_nFatHeight = h;

		create_ncc_model(ho_TemplateImage, m_ReadNumLevel, HTuple(m_ReadStartAngle).Rad(), HTuple(m_ReadEndAngle).Rad(), m_ReadStepAngle!=-1?HTuple(m_ReadStepAngle).Rad():"auto" , (LPCTSTR) m_ReadMetric, &hv_ModelId);
	}
	catch (HException &except) 
	{
		int  error_num = except.err; 
	}
#endif	

	m_bModelRead = true;
	m_bNccSelect = true;

	return bStatus;
}
BOOL CHalconProcessing::halcon_ReadNccModel(CString modelname)
{
	BOOL bStatus=FALSE;

	if(m_SearchMethod==0)
	{
		return halcon_ReadModel(modelname);
	}

	try 
	{
		CPoint ptCenterOffset;
		CString name=modelname.Left(modelname.GetLength()-3)+"dat";
		CFile file ;

		if ( file.Open( _T(name),  CFile::modeRead  ) )
		{
			file.Read( &ptCenterOffset, sizeof(CPoint) ) ;
			int i=0;
			for(i=0;i<4;i++)
			{
				if(	!file.Read( &m_cvTrans_mat[i], UINT(m_cvTrans_mat[i].total()*m_cvTrans_mat[i].elemSize())))
					break;
			}	

			if(i==4) m_bCalibration=true;	

			file.Read( &m_ptRotateCenter, sizeof(PointF) ) ;
			file.Read( &m_dResolutionX, sizeof(double) ) ;
			file.Read( &m_dResolutionY, sizeof(double) ) ;

			m_ptMarkOffset.x = ptCenterOffset.x;
			m_ptMarkOffset.y = ptCenterOffset.y;

			file.Close() ;
		}
	}
	catch(...) 
	{
	}

#ifdef _CPP_USE_HALCON
	ReadImage(&ho_TemplateImage, (LPCTSTR)modelname);
	CreateShapeModel(ho_TemplateImage, 2, HTuple(0).TupleRad(), HTuple(360).TupleRad(),	HTuple(4.0297).TupleRad(),//
		(HTuple("none").Append("pregeneration")), "use_polarity",((HTuple(25).Append(35)).Append(9)),"auto" , &hv_ModelId);//
	GetShapeModelContours(&ho_ModelContours, hv_ModelId, 1);
#else

	if(m_bModelRead)
	{
#ifdef _CPP_USE_HALCON
		if(m_bNccSelect) ClearNccModel(hv_ModelId);
		else ClearShapeModel(hv_ModelId);
#else
		if(m_bNccSelect)
			clear_ncc_model(hv_ModelId);
		else
			clear_shape_model(hv_ModelId);
#endif
	}

	read_image(&ho_TemplateImage, modelname);

	
	HTuple width, height;
	get_image_size( ho_TemplateImage, &width, &height);

	m_nFatWidth = int(width[0].D());
	m_nFatHeight = int(height[0].D());

	create_ncc_model(ho_TemplateImage, m_ReadNumLevel, HTuple(m_ReadStartAngle).Rad(), HTuple(m_ReadEndAngle).Rad(), m_ReadStepAngle!=-1?HTuple(m_ReadStepAngle).Rad():"auto" , (LPCTSTR) m_ReadMetric, &hv_ModelId);

	m_nFatOffsetWidth = 0;
	m_nFatOffsetHeight = 0;

#endif
	m_bNccSelect = TRUE;
	m_bModelRead = true;

	return bStatus;
}
BOOL CHalconProcessing::halcon_ReadMaskModel(CString modelname,BOOL bTemplateShape,BOOL m_bMaskRoiUse[4],BOOL m_bMaskRoiShape[4],CRect m_crMaskRect[4], int nMinContrast, int nHighContrast)
{
	if(m_SearchMethod==1)
	{
		return halcon_ReadMaskNCCModel(modelname, bTemplateShape, m_bMaskRoiUse,m_bMaskRoiShape,m_crMaskRect);
	}
	//HTK 2022-03-30 Halcon Scale Mark Search 기능 추가
	else if (m_ReadSmallScale > 0.1) return halcon_ReadScaleModel(modelname, "NONE");

	BOOL bStatus=TRUE;
	Hlong w;
	Hlong h;

	HTuple SelectNum,hv_Row1,hv_Column1,hv_Area, hv_Row2, hv_Column2, hv_Area2;

	if(m_bModelRead)
	{
#ifdef _CPP_USE_HALCON
		if(m_bNccSelect) ClearNccModel(hv_ModelId);
		else ClearShapeModel(hv_ModelId);
#else
		if(m_bNccSelect)	clear_ncc_model(hv_ModelId);
		else				clear_shape_model(hv_ModelId);
#endif
	}

#ifdef _CPP_USE_HALCON
	HObject  ho_Image, ho_ModelRegion, ho__TmpRegion;
#else
	Hobject  ho_Image, ho_ModelRegion, ho__TmpRegion;

	read_image(&ho_Image, modelname);
	get_image_size(ho_Image,&w,&h);

	if(bTemplateShape) 
	{
		double dir=w>h?0.:90.;
		gen_ellipse(&ho_ModelRegion, (h)/2., (w)/2. ,HTuple(dir).Rad(),	MAX(w/2.,h/2.),MIN(w/2.,h/2.));	
	}
	else 
	{		
		gen_rectangle1(&ho_ModelRegion, 0, 0, double(h), double(w));
	}	

	
	for(int nItem=0;nItem<4;nItem++)
	{
		if(m_bMaskRoiUse[nItem])
		{
			if(m_bMaskRoiShape[nItem]) 
			{
				double dir=m_crMaskRect[nItem].Width()>m_crMaskRect[nItem].Height()?0.:90.;
				gen_ellipse(&ho__TmpRegion, (m_crMaskRect[nItem].top+m_crMaskRect[nItem].bottom)/2., (m_crMaskRect[nItem].left+m_crMaskRect[nItem].right)/2. ,HTuple(dir).Rad(),
					MAX(m_crMaskRect[nItem].Width()/2.,m_crMaskRect[nItem].Height()/2.),MIN(m_crMaskRect[nItem].Width()/2.,m_crMaskRect[nItem].Height()/2.));		
			}
			else 
			{
				gen_rectangle1(&ho__TmpRegion, (double)m_crMaskRect[nItem].top, (double)m_crMaskRect[nItem].left, (double)m_crMaskRect[nItem].bottom, (double)m_crMaskRect[nItem].right);		
			}

			difference(ho_ModelRegion, ho__TmpRegion, &ho_ModelRegion);
		}
	}	

	area_center(ho_ModelRegion,&hv_Area,&hv_Row1,&hv_Column1);
	//HTK 2022-04-02 Mask처리시 이미지 사이즈 변경되는 문제 디버깅
	area_center(ho_Image, &hv_Area2, &hv_Row2, &hv_Column2);

	reduce_domain(ho_Image, ho_ModelRegion, &ho_TemplateImage);
	
	//HTK 2022-04-13 Mark Angle Search
	create_shape_model(ho_TemplateImage,m_ReadNumLevel, HTuple(m_ReadStartAngle).Rad(), HTuple(m_ReadEndAngle- m_ReadStartAngle).Rad(),m_ReadStepAngle!=-1?HTuple(m_ReadStepAngle).Rad():"auto" ,
		m_ReadOptimize=="none"?"auto":m_ReadOptimize,(LPCTSTR) m_ReadMetric, (Hlong)m_ReadContrast!=-1?(Hlong)m_ReadContrast:HTuple("auto"), (Hlong)m_ReadMinContrast!=-1?(Hlong)m_ReadMinContrast:HTuple("auto"), &hv_ModelId);

	get_shape_model_contours(&ho_ModelContours, hv_ModelId, 1);

	//HTK 2022-04-02 Mask처리시 이미지 사이즈 변경되는 문제 디버깅
	m_nFatOffsetWidth = hv_Column2[0].D() - hv_Column1[0].D();
	m_nFatOffsetHeight = hv_Row2[0].D() - hv_Row1[0].D();

#endif

	m_bNccSelect = FALSE;
	m_bModelRead = true;

	return bStatus;
}
BOOL CHalconProcessing::halcon_ReadMaskModel(BYTE *ptemplateSrc,int w,int h,BOOL bTemplateShape,BOOL m_bMaskRoiUse[4],BOOL m_bMaskRoiShape[4],CRect m_crMaskRect[4],int nMinContrast, int nHighContrast)
{
	BOOL bStatus=TRUE;

	HTuple SelectNum,hv_Row1,hv_Column1,hv_Area, hv_Row2, hv_Column2, hv_Area2;

	if(m_SearchMethod==1)
	{
		return halcon_ReadMaskNCCModel(ptemplateSrc, w, h, bTemplateShape, m_bMaskRoiUse,m_bMaskRoiShape,m_crMaskRect);
	}
	//HTK 2022-03-30 Halcon Scale Mark Search 기능 추가
	else if (m_ReadSmallScale > 0.1) return halcon_ReadScaleModel(ptemplateSrc, 0, w, h);

	if(m_bModelRead)
	{
#ifdef _CPP_USE_HALCON
		if(m_bNccSelect) ClearNccModel(hv_ModelId);
		else ClearShapeModel(hv_ModelId);
#else
		if(m_bNccSelect)			clear_ncc_model(hv_ModelId);
		else			clear_shape_model(hv_ModelId);
#endif
	}

#ifdef _CPP_USE_HALCON
	HObject  ho_Image, ho_ModelRegion, ho__TmpRegion;
#else
	Hobject  ho_Image, ho_ModelRegion, ho__TmpRegion;

	gen_image1(&ho_Image,"byte",w,h,(Hlong)ptemplateSrc);

	if(bTemplateShape) 
	{
		double dir=w>h?0.:90.;
		gen_ellipse(&ho_ModelRegion, (h)/2., (w)/2. ,HTuple(dir).Rad(),	MAX(w/2.,h/2.),MIN(w/2.,h/2.));	
	}
	else 
	{		
		gen_rectangle1(&ho_ModelRegion, 0, 0, h, w);
	}	

	
	for(int nItem=0;nItem<4;nItem++)
	{
		if(m_bMaskRoiUse[nItem])
		{
			if(m_bMaskRoiShape[nItem]) 
			{
				double dir=m_crMaskRect[nItem].Width()>m_crMaskRect[nItem].Height()?0.:90.;
				gen_ellipse(&ho__TmpRegion, (m_crMaskRect[nItem].top+m_crMaskRect[nItem].bottom)/2., (m_crMaskRect[nItem].left+m_crMaskRect[nItem].right)/2. ,HTuple(dir).Rad(),
					MAX(m_crMaskRect[nItem].Width()/2.,m_crMaskRect[nItem].Height()/2.),MIN(m_crMaskRect[nItem].Width()/2.,m_crMaskRect[nItem].Height()/2.));		
			}
			else 
			{				
				gen_rectangle1(&ho__TmpRegion, (double)m_crMaskRect[nItem].top, (double)m_crMaskRect[nItem].left, (double)m_crMaskRect[nItem].bottom, (double)m_crMaskRect[nItem].right);		
			}

			difference(ho_ModelRegion, ho__TmpRegion, &ho_ModelRegion);
		}
	}	

	area_center(ho_ModelRegion,&hv_Area,&hv_Row1,&hv_Column1);
	//HTK 2022-04-02 Mask처리시 이미지 사이즈 변경되는 문제 디버깅
	area_center(ho_Image, &hv_Area2, &hv_Row2, &hv_Column2);

	reduce_domain(ho_Image, ho_ModelRegion, &ho_TemplateImage);

//	write_image(ho_TemplateImage,"bmp",0,"d:\\sdf.bmp");
	if (m_lWindowID != -1)
	{
		if (HDevWindowStack::IsOpen())     disp_obj(ho_TemplateImage, HDevWindowStack::GetActive());
	}

	//HTK 2022-04-13 Mark Angle Search
	create_shape_model(ho_TemplateImage,m_ReadNumLevel, HTuple(m_ReadStartAngle).Rad(), HTuple(m_ReadEndAngle- m_ReadStartAngle).Rad(),m_ReadStepAngle!=-1?HTuple(m_ReadStepAngle).Rad():"auto" ,
		m_ReadOptimize=="none"?"auto":m_ReadOptimize,(LPCTSTR) m_ReadMetric, (Hlong)m_ReadContrast!=-1?(Hlong)m_ReadContrast:HTuple("auto"), (Hlong)m_ReadMinContrast!=-1?(Hlong)m_ReadMinContrast:HTuple("auto"), &hv_ModelId);


	get_shape_model_contours(&ho_ModelContours, hv_ModelId, 1);

	//HTK 2022-04-02 Mask처리시 이미지 사이즈 변경되는 문제 디버깅
	m_nFatOffsetWidth = hv_Column2[0].D() - hv_Column1[0].D();
	m_nFatOffsetHeight = hv_Row2[0].D() - hv_Row1[0].D();

	if (m_lWindowID != -1)
	{
		if (HDevWindowStack::IsOpen())     disp_obj(ho_ModelContours, HDevWindowStack::GetActive());
	}
#endif

	m_bNccSelect = FALSE;
	m_bModelRead = true;

	return bStatus;
}
BOOL CHalconProcessing::halcon_ReadMaskNCCModel(CString modelname,BOOL bTemplateShape,BOOL m_bMaskRoiUse[4],BOOL m_bMaskRoiShape[4],CRect m_crMaskRect[4])
{
	HTuple hv_Row1,hv_Column1,hv_Area, hv_Row2, hv_Column2, hv_Area2;

	if(m_SearchMethod==0)
	{
		return halcon_ReadMaskModel(modelname, bTemplateShape, m_bMaskRoiUse,m_bMaskRoiShape,m_crMaskRect);
	}

	BOOL bStatus=TRUE;
	Hlong w;
	Hlong h;

	if(m_bModelRead)
	{
#ifdef _CPP_USE_HALCON
		if(m_bNccSelect) ClearNccModel(hv_ModelId);
		else ClearShapeModel(hv_ModelId);
#else
		if(m_bNccSelect)	clear_ncc_model(hv_ModelId);
		else				clear_shape_model(hv_ModelId);
#endif
	}

#ifdef _CPP_USE_HALCON
	HObject  ho_Image, ho_ModelRegion, ho__TmpRegion;
#else
	Hobject  ho_Image, ho_ModelRegion, ho__TmpRegion;

	read_image(&ho_Image, modelname);
	get_image_size(ho_Image,&w,&h);

	if(bTemplateShape) 
	{
		double dir=w>h?0.:90.;
		gen_ellipse(&ho_ModelRegion, (h)/2., (w)/2. ,HTuple(dir).Rad(),	MAX(w/2.,h/2.),MIN(w/2.,h/2.));	
	}
	else 
	{		
		gen_rectangle1(&ho_ModelRegion, 0, 0, double(h), double(w));
	}	

	
	for(int nItem=0;nItem<4;nItem++)
	{
		if(m_bMaskRoiUse[nItem])
		{
			if(m_bMaskRoiShape[nItem]) 
			{
				double dir=m_crMaskRect[nItem].Width()>m_crMaskRect[nItem].Height()?0.:90.;
				gen_ellipse(&ho__TmpRegion, (m_crMaskRect[nItem].top+m_crMaskRect[nItem].bottom)/2., (m_crMaskRect[nItem].left+m_crMaskRect[nItem].right)/2. ,HTuple(dir).Rad(),
					MAX(m_crMaskRect[nItem].Width()/2.,m_crMaskRect[nItem].Height()/2.),MIN(m_crMaskRect[nItem].Width()/2.,m_crMaskRect[nItem].Height()/2.));		
			}
			else 
			{
				gen_rectangle1(&ho__TmpRegion, (double)m_crMaskRect[nItem].top, (double)m_crMaskRect[nItem].left, (double)m_crMaskRect[nItem].bottom, (double)m_crMaskRect[nItem].right);		
			}

			difference(ho_ModelRegion, ho__TmpRegion, &ho_ModelRegion);
		}
	}	

	area_center(ho_ModelRegion,&hv_Area,&hv_Row1,&hv_Column1);
	area_center(ho_Image, &hv_Area2, &hv_Row2, &hv_Column2);
	reduce_domain(ho_Image, ho_ModelRegion, &ho_TemplateImage);
	
	create_ncc_model(ho_TemplateImage, m_ReadNumLevel, HTuple(m_ReadStartAngle).Rad(), HTuple(m_ReadEndAngle).Rad(), m_ReadStepAngle!=-1?HTuple(m_ReadStepAngle).Rad():"auto" , (LPCTSTR) m_ReadMetric, &hv_ModelId);

	m_nFatOffsetWidth = hv_Column2[0].D() - hv_Column1[0].D();
	m_nFatOffsetHeight = hv_Row2[0].D() - hv_Row1[0].D();

#endif

	m_bNccSelect = FALSE;
	m_bModelRead = true;

	return bStatus;
}
BOOL CHalconProcessing::halcon_ReadMaskNCCModel(BYTE *ptemplateSrc,int w,int h,BOOL bTemplateShape,BOOL m_bMaskRoiUse[4],BOOL m_bMaskRoiShape[4],CRect m_crMaskRect[4])
{
	BOOL bStatus=TRUE;

	HTuple hv_Row1,hv_Column1,hv_Area, hv_Row2, hv_Column2, hv_Area2;

	if(m_SearchMethod==0)
	{
		return halcon_ReadMaskModel(ptemplateSrc, w, h, bTemplateShape, m_bMaskRoiUse,m_bMaskRoiShape,m_crMaskRect);
	}

	if(m_bModelRead)
	{
#ifdef _CPP_USE_HALCON
		if(m_bNccSelect) ClearNccModel(hv_ModelId);
		else ClearShapeModel(hv_ModelId);
#else
		if(m_bNccSelect)			clear_ncc_model(hv_ModelId);
		else			clear_shape_model(hv_ModelId);
#endif
	}

#ifdef _CPP_USE_HALCON
	HObject  ho_Image, ho_ModelRegion, ho__TmpRegion;
#else
	Hobject  ho_Image, ho_ModelRegion, ho__TmpRegion;

	gen_image1(&ho_Image,"byte",w,h,(Hlong)ptemplateSrc);

	if(bTemplateShape) 
	{
		double dir=w>h?0.:90.;
		gen_ellipse(&ho_ModelRegion, (h)/2., (w)/2. ,HTuple(dir).Rad(),	MAX(w/2.,h/2.),MIN(w/2.,h/2.));	
	}
	else 
	{		
		gen_rectangle1(&ho_ModelRegion, 0, 0, h, w);
	}	

	
	for(int nItem=0;nItem<4;nItem++)
	{
		if(m_bMaskRoiUse[nItem])
		{
			if(m_bMaskRoiShape[nItem]) 
			{
				double dir=m_crMaskRect[nItem].Width()>m_crMaskRect[nItem].Height()?0.:90.;
				gen_ellipse(&ho__TmpRegion, (m_crMaskRect[nItem].top+m_crMaskRect[nItem].bottom)/2., (m_crMaskRect[nItem].left+m_crMaskRect[nItem].right)/2. ,HTuple(dir).Rad(),
					MAX(m_crMaskRect[nItem].Width()/2.,m_crMaskRect[nItem].Height()/2.),MIN(m_crMaskRect[nItem].Width()/2.,m_crMaskRect[nItem].Height()/2.));		
			}
			else 
			{				
				gen_rectangle1(&ho__TmpRegion, (double)m_crMaskRect[nItem].top, (double)m_crMaskRect[nItem].left, (double)m_crMaskRect[nItem].bottom, (double)m_crMaskRect[nItem].right);		
			}

			difference(ho_ModelRegion, ho__TmpRegion, &ho_ModelRegion);
		}
	}	

	area_center(ho_ModelRegion,&hv_Area,&hv_Row1,&hv_Column1);
	//HTK 2022-04-02 Mask처리시 이미지 사이즈 변경되는 문제 디버깅
	area_center(ho_Image, &hv_Area2, &hv_Row2, &hv_Column2);
	reduce_domain(ho_Image, ho_ModelRegion, &ho_TemplateImage);

//	write_image(ho_TemplateImage,"bmp",0,"d:\\sdf.bmp");
	if (m_lWindowID != -1)
	{
		if (HDevWindowStack::IsOpen())     disp_obj(ho_TemplateImage, HDevWindowStack::GetActive());
	}

	create_ncc_model(ho_TemplateImage, m_ReadNumLevel, HTuple(m_ReadStartAngle).Rad(), HTuple(m_ReadEndAngle).Rad(), m_ReadStepAngle!=-1?HTuple(m_ReadStepAngle).Rad():"auto" , (LPCTSTR) m_ReadMetric, &hv_ModelId);
	
	//HTK 2022-04-02 Mask처리시 이미지 사이즈 변경되는 문제 디버깅
	m_nFatOffsetWidth = hv_Column2[0].D() - hv_Column1[0].D();
	m_nFatOffsetHeight = hv_Row2[0].D() - hv_Row1[0].D();

	if (m_lWindowID != -1)
	{
		if (HDevWindowStack::IsOpen())     disp_obj(ho_ModelContours, HDevWindowStack::GetActive());
	}
#endif

	m_bNccSelect = FALSE;
	m_bModelRead = true;

	return bStatus;
}
BOOL CHalconProcessing::halcon_SearchMark(BYTE *pbyImage, int w,int h,CRect InspROI ,double score,char * charMethod)
{
	BOOL bStatus=FALSE;

	if(!m_bModelRead) return bStatus;
	
	//HTK 2022-03-30 Halcon Scale Mark Search 기능 추가
	if (m_SearchMethod == 1)
	{
		return halcon_SearchNccMark(pbyImage, w, h, InspROI, score);
	}
	else if (m_ReadSmallScale>0.1) return halcon_SearchScaleMark(pbyImage, w, h, InspROI, score, charMethod);

	// Local iconic variables

	score = score-0.2;

	if( score < 0.3 )
		score = 0.3;
	
#ifdef _CPP_USE_HALCON
	HObject  ho_Image;
	HObject  ho_TransContours,ho_Center;
	HObject  ho_ROI_0,ho_Lens,ho_ImagePart;
#else
	Hobject  ho_Image;
	Hobject  ho_TransContours,ho_Center;
	Hobject  ho_ROI_0,ho_Lens,ho_ImagePart;
#endif
	// Local control variables
	HTuple  hv_MatchingObjIdx;
	HTuple  hv_HomMat,ho_max;
	HTuple  hv_Row1, hv_Column1, hv_Row2, hv_Column2;

	hv_Row1 = InspROI.top;
	hv_Column1 = InspROI.left;
	hv_Row2 = InspROI.bottom;
	hv_Column2 = InspROI.right;

#ifdef _CPP_USE_HALCON
	GenImage1(&ho_Image,"byte",w,h,(Hlong)pbyImage);		//Image Load

	GenRectangle1(&ho_ROI_0, hv_Row1, hv_Column1, hv_Row2, hv_Column2);
	ReduceDomain(ho_Image, ho_ROI_0, &ho_Lens);
	CropDomain(ho_Lens, &ho_ImagePart);
	try {
		FindShapeModel(ho_ImagePart, hv_ModelId, HTuple(0).TupleRad(), HTuple(360).TupleRad(), 	score, 1, 0.5,
			charMethod, (HTuple(2).Append(1)), 0.75, &hv_ModelRow, &hv_ModelColumn, &hv_ModelAngle, &hv_ModelScore);
		TupleMax(hv_ModelScore,&ho_max);

		HTuple end_val19 = (hv_ModelScore.TupleLength())-1;
		HTuple step_val19 = 1;

		for (hv_MatchingObjIdx=0; hv_MatchingObjIdx.Continue(end_val19, step_val19); hv_MatchingObjIdx += step_val19)
		{
			HomMat2dIdentity(&hv_HomMat);
			HomMat2dRotate(hv_HomMat, HTuple(hv_ModelAngle[hv_MatchingObjIdx]), 0, 0, &hv_HomMat);
			HomMat2dTranslate(hv_HomMat, HTuple(hv_ModelRow[hv_MatchingObjIdx]), HTuple(hv_ModelColumn[hv_MatchingObjIdx]),	&hv_HomMat);
			AffineTransContourXld(ho_ModelContours, &ho_TransContours, hv_HomMat);

			if(hv_ModelScore[hv_MatchingObjIdx]==ho_max)
			{
				m_ptMarkPos.y = hv_ModelRow[hv_MatchingObjIdx].D() + m_ptMarkOffset.x;
				m_ptMarkPos.x = hv_ModelColumn[hv_MatchingObjIdx].D() + m_ptMarkOffset.y;

				m_dModelAngleRadian =hv_ModelAngle[hv_MatchingObjIdx].D();
				m_dModelScore = hv_ModelScore[hv_MatchingObjIdx].D();
				m_dModelAngle = m_dModelAngleRadian* (180. / PI);
				
				bStatus = TRUE;
			}
		}
	}
	catch(...) { }
#else
	
	gen_image1(&ho_Image, "byte", w, h, (Hlong)pbyImage);		//Image Load
	gen_rectangle1(&ho_ROI_0, hv_Row1, hv_Column1, hv_Row2, hv_Column2);
	reduce_domain(ho_Image, ho_ROI_0, &ho_Lens);
	crop_domain(ho_Lens, &ho_ImagePart);

	if (m_lWindowID != -1)
	{
		if (HDevWindowStack::IsOpen())     disp_obj(ho_ImagePart, HDevWindowStack::GetActive());
	}

	try{

		//HTK 2022-04-13 Mark Angle Search , Search Level : m_ReadNumLevel-> 0으로 변경
		find_shape_model(ho_ImagePart, hv_ModelId, HTuple(m_SearchStartAngle).Rad(), HTuple(m_SearchEndAngle- m_SearchStartAngle).Rad(), score, m_SearchNumMatch, 0.5,
			(LPCTSTR)m_SearchSubPixel, 0, m_SearchGreedness, &hv_ModelRow, &hv_ModelColumn, &hv_ModelAngle, &hv_ModelScore);
		
//		find_shape_model(ho_ImagePart, hv_ModelId, HTuple(m_nSearchAngleRange*-1).Rad(), HTuple(m_nSearchAngleRange).Rad(), score, 4, 0.5, 
//			charMethod,5, 0.75, &hv_ModelRow, &hv_ModelColumn, &hv_ModelAngle, &hv_ModelScore); //(HTuple(m_ReadNumLevel).Append(-2)) 블러링 된경우 사용
		
		if( hv_ModelScore.Num() == 0)
			return FALSE;
		
		tuple_max(hv_ModelScore,&ho_max);
		HTuple end_val19 = (hv_ModelScore.Num()) - 1;
		HTuple step_val19 = 1;		

		for (hv_MatchingObjIdx=0; hv_MatchingObjIdx.Continue(end_val19, step_val19); hv_MatchingObjIdx += step_val19)
		{
			if(hv_ModelScore[hv_MatchingObjIdx] == ho_max)
			{
				//HTK 2022-04-02 Mask처리시 이미지 사이즈 변경되는 문제 디버깅
				double offX, offY;

				m_dModelAngleRadian = hv_ModelAngle[hv_MatchingObjIdx].D();
				m_dModelScore = hv_ModelScore[hv_MatchingObjIdx].D();
				m_dModelAngle = m_dModelAngleRadian * (180. / PI);

				offX = cos(-m_dModelAngleRadian) * m_nFatOffsetWidth - sin(-m_dModelAngleRadian) * m_nFatOffsetHeight;
				offY = sin(-m_dModelAngleRadian) * m_nFatOffsetWidth + cos(-m_dModelAngleRadian) * m_nFatOffsetHeight;

				m_ptMarkPos.y = hv_ModelRow[hv_MatchingObjIdx].D() + offY;  //
				m_ptMarkPos.x = hv_ModelColumn[hv_MatchingObjIdx].D() + offX; //				

				bStatus = TRUE;
			}

			if (m_lWindowID != -1)
			{
				if (HDevWindowStack::IsOpen()) 
				{
					disp_obj(ho_TransContours, HDevWindowStack::GetActive());
				}
			}
		}
  }
  catch (Halcon::HException& except)
  {
	  int  error_num = except.err; 
	  return -1;
  }
#endif	


	return bStatus;
}
BOOL CHalconProcessing::halcon_SearchNccMark(BYTE *pbyImage, int w,int h,CRect InspROI ,double score)
{
	BOOL bStatus=FALSE;

	if(!m_bModelRead) return bStatus;

	if(m_SearchMethod==0)
	{
		return halcon_SearchMark(pbyImage, w, h,InspROI , score,(LPTSTR)(LPCTSTR)m_SearchSubPixel);
	}

	// Local iconic variables
	
#ifdef _CPP_USE_HALCON
	HObject  ho_Image;
	HObject  ho_ROI_0,ho_Lens,ho_ImagePart;
#else
	Hobject  ho_Image;
	Hobject  ho_ROI_0,ho_Lens,ho_ImagePart;
#endif
	// Local control variables
	HTuple  hv_MatchingObjIdx;
	HTuple  hv_HomMat,ho_max;	
	HTuple  hv_Row1, hv_Column1, hv_Row2, hv_Column2;

	hv_Row1 = InspROI.top;
	hv_Column1 = InspROI.left;
	hv_Row2 = InspROI.bottom;
	hv_Column2 = InspROI.right;

#ifdef _CPP_USE_HALCON
	GenImage1(&ho_Image,"byte",w,h,(Hlong)pbyImage);		//Image Load

	GenRectangle1(&ho_ROI_0, hv_Row1, hv_Column1, hv_Row2, hv_Column2);
	ReduceDomain(ho_Image, ho_ROI_0, &ho_Lens);
	CropDomain(ho_Lens, &ho_ImagePart);
	try{		
		FindNccModel(ho_Image, hv_ModelId, HTuple(0).TupleRad(), HTuple(360).TupleRad(),  score, 34, 0.5, "true", 0, &hv_ModelRow, &hv_ModelColumn, &hv_ModelAngle, &hv_ModelScore);

		if(hv_ModelScore.TupleLength()==0) return FALSE;
		TupleMax(hv_ModelScore,&ho_max);

		HTuple end_val19 = (hv_ModelScore.TupleLength())-1;
		HTuple step_val19 = 1;

		for (hv_MatchingObjIdx=0; hv_MatchingObjIdx.Continue(end_val19, step_val19); hv_MatchingObjIdx += step_val19)
		{
			if(hv_ModelScore[hv_MatchingObjIdx]==ho_max)
			{
				m_ptMarkPos.y = hv_ModelRow[hv_MatchingObjIdx].D() + m_ptMarkOffset.x;
				m_ptMarkPos.x = hv_ModelColumn[hv_MatchingObjIdx].D() + m_ptMarkOffset.y;

				m_dModelAngleRadian =hv_ModelAngle[hv_MatchingObjIdx].D();
				m_dModelScore = hv_ModelScore[hv_MatchingObjIdx].D();
				m_dModelAngle = m_dModelAngleRadian* (180. / PI);
				
				bStatus=TRUE;
			}
		}
	}
	catch(...){}
#else

	try
	{
		gen_image1(&ho_Image,"byte",w,h,(Hlong)pbyImage);		//Image Load
		gen_rectangle1(&ho_ROI_0, hv_Row1, hv_Column1, hv_Row2, hv_Column2);
		reduce_domain(ho_Image, ho_ROI_0, &ho_Lens);
		crop_domain(ho_Lens, &ho_ImagePart);
				
		find_ncc_model(ho_ImagePart, hv_ModelId, HTuple(m_SearchStartAngle).Rad(), HTuple(m_SearchEndAngle).Rad(),  m_SearchMinscore, m_SearchNumMatch, 0.5, "true", m_ReadNumLevel, &hv_ModelRow, &hv_ModelColumn, &hv_ModelAngle, &hv_ModelScore);
	//	find_ncc_model(ho_ImagePart, hv_ModelId, HTuple(m_nSearchAngleRange*-1).Rad(), HTuple(m_nSearchAngleRange).Rad(),  score, 1, 0.5, "true", 0, &hv_ModelRow, &hv_ModelColumn, &hv_ModelAngle, &hv_ModelScore);

		if(hv_ModelScore.Num()==0) return FALSE;
		tuple_max(hv_ModelScore,&ho_max);
		HTuple end_val19 = (hv_ModelScore.Num())-1;
		HTuple step_val19 = 1;
		for (hv_MatchingObjIdx=0; hv_MatchingObjIdx.Continue(end_val19, step_val19); hv_MatchingObjIdx += step_val19)
		{
			if(hv_ModelScore[hv_MatchingObjIdx]==ho_max)
			{
				//HTK 2022-04-02 Mask처리시 이미지 사이즈 변경되는 문제 디버깅
				double offX, offY;

				m_dModelAngleRadian = hv_ModelAngle[hv_MatchingObjIdx].D();
				m_dModelScore = hv_ModelScore[hv_MatchingObjIdx].D();
				m_dModelAngle = m_dModelAngleRadian * (180. / PI);

				offX = cos(-m_dModelAngleRadian) * m_nFatOffsetWidth - sin(-m_dModelAngleRadian) * m_nFatOffsetHeight;
				offY = sin(-m_dModelAngleRadian) * m_nFatOffsetWidth + cos(-m_dModelAngleRadian) * m_nFatOffsetHeight;

				m_ptMarkPos.y = hv_ModelRow[hv_MatchingObjIdx].D() + offY;  //
				m_ptMarkPos.x = hv_ModelColumn[hv_MatchingObjIdx].D() + offX; //

				bStatus=TRUE;
			}
		}
	}

	catch (HException &except) 
	{
		int  error_num = except.err; 
	}
#endif	


	return bStatus;
}
//HTK 2022-03-30 Halcon Scale Mark Search 기능 추가
BOOL CHalconProcessing::halcon_SearchScaleMark(BYTE* pbyImage, int w, int h, CRect InspROI, double score, char* charMethod)
{
	BOOL bStatus = FALSE;

	if (!m_bModelRead) return bStatus;

	if (m_SearchMethod == 1)
	{
		return halcon_SearchNccMark(pbyImage, w, h, InspROI, score);
	}

	// Local iconic variables

	score = score - 0.2;

	if (score < 0.3)
		score = 0.3;

#ifdef _CPP_USE_HALCON
	HObject  ho_Image;
	HObject  ho_TransContours, ho_Center;
	HObject  ho_ROI_0, ho_Lens, ho_ImagePart;
#else
	Hobject  ho_Image;
	Hobject  ho_TransContours, ho_Center;
	Hobject  ho_ROI_0, ho_Lens, ho_ImagePart;
#endif
	// Local control variables
	HTuple  hv_MatchingObjIdx;
	HTuple  hv_HomMat, ho_max;
	HTuple  hv_Row1, hv_Column1, hv_Row2, hv_Column2;

	hv_Row1 = InspROI.top;
	hv_Column1 = InspROI.left;
	hv_Row2 = InspROI.bottom;
	hv_Column2 = InspROI.right;

#ifdef _CPP_USE_HALCON
	GenImage1(&ho_Image, "byte", w, h, (Hlong)pbyImage);		//Image Load

	GenRectangle1(&ho_ROI_0, hv_Row1, hv_Column1, hv_Row2, hv_Column2);
	ReduceDomain(ho_Image, ho_ROI_0, &ho_Lens);
	CropDomain(ho_Lens, &ho_ImagePart);
	try {
		FindShapeModel(ho_ImagePart, hv_ModelId, HTuple(0).TupleRad(), HTuple(360).TupleRad(), score, 1, 0.5,
			charMethod, (HTuple(2).Append(1)), 0.75, &hv_ModelRow, &hv_ModelColumn, &hv_ModelAngle, &hv_ModelScore);
		TupleMax(hv_ModelScore, &ho_max);

		HTuple end_val19 = (hv_ModelScore.TupleLength()) - 1;
		HTuple step_val19 = 1;

		for (hv_MatchingObjIdx = 0; hv_MatchingObjIdx.Continue(end_val19, step_val19); hv_MatchingObjIdx += step_val19)
		{
			HomMat2dIdentity(&hv_HomMat);
			HomMat2dRotate(hv_HomMat, HTuple(hv_ModelAngle[hv_MatchingObjIdx]), 0, 0, &hv_HomMat);
			HomMat2dTranslate(hv_HomMat, HTuple(hv_ModelRow[hv_MatchingObjIdx]), HTuple(hv_ModelColumn[hv_MatchingObjIdx]), &hv_HomMat);
			AffineTransContourXld(ho_ModelContours, &ho_TransContours, hv_HomMat);

			if (hv_ModelScore[hv_MatchingObjIdx] == ho_max)
			{
				m_ptMarkPos.y = hv_ModelRow[hv_MatchingObjIdx].D() + m_ptMarkOffset.x;
				m_ptMarkPos.x = hv_ModelColumn[hv_MatchingObjIdx].D() + m_ptMarkOffset.y;

				m_dModelAngleRadian = hv_ModelAngle[hv_MatchingObjIdx].D();
				m_dModelScore = hv_ModelScore[hv_MatchingObjIdx].D();
				m_dModelAngle = m_dModelAngleRadian * (180. / PI);

				bStatus = TRUE;
			}
		}
	}
	catch (...) {}
#else

	gen_image1(&ho_Image, "byte", w, h, (Hlong)pbyImage);		//Image Load
	gen_rectangle1(&ho_ROI_0, hv_Row1, hv_Column1, hv_Row2, hv_Column2);
	reduce_domain(ho_Image, ho_ROI_0, &ho_Lens);
	crop_domain(ho_Lens, &ho_ImagePart);

	if (m_lWindowID != -1)
	{
		if (HDevWindowStack::IsOpen())     disp_obj(ho_ImagePart, HDevWindowStack::GetActive());
	}

	try {

		find_scaled_shape_model(ho_ImagePart, hv_ModelId, HTuple(m_SearchStartAngle).Rad(), HTuple(m_SearchEndAngle).Rad(), 1 - m_ReadSmallScale, 1 + m_ReadSmallScale, 
			0.5, 1, 0.5, (LPCTSTR)m_SearchSubPixel, 0, m_SearchGreedness, &hv_ModelRow, &hv_ModelColumn, &hv_ModelAngle, &hv_ModelScale, &hv_ModelScore);
		
		if (hv_ModelScore.Num() == 0)
			return FALSE;

		tuple_max(hv_ModelScore, &ho_max);
		HTuple end_val19 = (hv_ModelScore.Num()) - 1;
		HTuple step_val19 = 1;

		for (hv_MatchingObjIdx = 0; hv_MatchingObjIdx.Continue(end_val19, step_val19); hv_MatchingObjIdx += step_val19)
		{
			if (hv_ModelScore[hv_MatchingObjIdx] == ho_max)
			{
				double offX, offY;

				m_dModelAngleRadian = hv_ModelAngle[hv_MatchingObjIdx].D();
				m_dModelScore = hv_ModelScore[hv_MatchingObjIdx].D();
				m_dModelAngle = m_dModelAngleRadian * (180. / PI);

				offX = cos(-m_dModelAngleRadian) * m_nFatOffsetWidth - sin(-m_dModelAngleRadian) * m_nFatOffsetHeight;
				offY = sin(-m_dModelAngleRadian) * m_nFatOffsetWidth + cos(-m_dModelAngleRadian) * m_nFatOffsetHeight;

				m_ptMarkPos.y = hv_ModelRow[hv_MatchingObjIdx].D() + offY;  //
				m_ptMarkPos.x = hv_ModelColumn[hv_MatchingObjIdx].D() + offX; //

				bStatus = TRUE;
			}

			if (m_lWindowID != -1)
			{
				if (HDevWindowStack::IsOpen())
				{
					disp_obj(ho_TransContours, HDevWindowStack::GetActive());
				}
			}
		}
	}
	catch (Halcon::HException& except)
	{
		int  error_num = except.err;
		return -1;
	}
#endif	


	return bStatus;
}
BOOL CHalconProcessing::halcon_ReadInspectModel(CString modelname)
{
	BOOL bStatus=FALSE;
	
	HTuple  hv_NumLevels,hv_AreaModelRegions,hv_RowModelRegions,hv_ColumnModelRegions,hv_HeightPyramid,hv_i;
#ifdef _CPP_USE_HALCON
	HObject  ho_ShapeModelImages,  ho_ShapeModelRegions;
#else
	Hobject  ho_ShapeModelImages,  ho_ShapeModelRegions;
#endif
	
	try 
	{
		CPoint ptCenterOffset;
		CString name=modelname.Left(modelname.GetLength()-3)+"dat";
		CFile file ;

		if ( file.Open( _T(name),  CFile::modeRead  ) )
		{
			file.Read( &ptCenterOffset, sizeof(CPoint) ) ;
			int i=0;
			for(i=0;i<4;i++)
			{
				if(	!file.Read( &m_cvTrans_mat[i], UINT(m_cvTrans_mat[i].total()*m_cvTrans_mat[i].elemSize())))
					break;
			}	

			if(i==4) m_bCalibration=true;	

			file.Read( &m_ptRotateCenter, sizeof(PointF) ) ;
			file.Read( &m_dResolutionX, sizeof(double) ) ;
			file.Read( &m_dResolutionY, sizeof(double) ) ;

			m_ptMarkOffset.x = ptCenterOffset.x;
			m_ptMarkOffset.y = ptCenterOffset.y;

			file.Close() ;
		}


	}
	catch(...) 
	{
	}

#ifdef _CPP_USE_HALCON	
	ReadImage(&ho_TemplateImage, (LPCTSTR)modelname);
	InspectShapeModel(ho_TemplateImage, &ho_ShapeModelImages, &ho_ShapeModelRegions, 8, 30);

	AreaCenter(ho_ShapeModelRegions, &hv_AreaModelRegions, &hv_RowModelRegions, &hv_ColumnModelRegions);
	CountObj(ho_ShapeModelRegions, &hv_HeightPyramid);
	HTuple end_val38 = hv_HeightPyramid;
	HTuple step_val38 = 1;
	for (hv_i=1; hv_i.Continue(end_val38, step_val38); hv_i += step_val38)
	{
		if (0 != (HTuple(hv_AreaModelRegions[hv_i-1])>=15))			hv_NumLevels = hv_i;
	}	

	CreateShapeModel(ho_TemplateImage, hv_NumLevels, HTuple(0).TupleRad(), HTuple(360).TupleRad(),	"auto",//
		(HTuple("none").Append("no_pregeneration")), "use_polarity",((HTuple(11).Append(12)).Append(4)),"auto" , &hv_ModelId);//
	GetShapeModelContours(&ho_ModelContours, hv_ModelId, 1);
#else
	read_image(&ho_TemplateImage, modelname);	
	inspect_shape_model(ho_TemplateImage, &ho_ShapeModelImages, &ho_ShapeModelRegions, 8, 60);

	area_center(ho_ShapeModelRegions, &hv_AreaModelRegions, &hv_RowModelRegions, &hv_ColumnModelRegions);
	count_obj(ho_ShapeModelRegions, &hv_HeightPyramid);
	HTuple end_val38 = hv_HeightPyramid;
	HTuple step_val38 = 1;
	for (hv_i=1; hv_i.Continue(end_val38, step_val38); hv_i += step_val38)
	{
		if (0 != (HTuple(hv_AreaModelRegions[hv_i-1])>=15))			hv_NumLevels = hv_i;
	}	
	create_shape_model(ho_TemplateImage, hv_NumLevels, HTuple(0).Rad(), HTuple(360).Rad(), "auto", 
		(HTuple("none").Append("no_pregeneration")), "use_polarity", ((HTuple(11).Append(12)).Append(4)), "auto", &hv_ModelId);
	get_shape_model_contours(&ho_ModelContours, hv_ModelId, 1);

#endif	

	m_bModelRead = true;

	return bStatus;
}
BOOL CHalconProcessing::halcon_WriteModel(CString modelname,BYTE *pbyImage,int w,int h,CRect roiRect,CPoint ptCenterOffset)
{
	cv::Rect myROI(roiRect.left,roiRect.top,roiRect.Width(),roiRect.Height());
	cv::Mat img(h, w, CV_8UC1, pbyImage);
	cv::Mat croppedImage = img(myROI);

	cv::imwrite(cv::String(modelname),croppedImage);

	CString name=modelname.Left(modelname.GetLength()-3)+"dat";
	CFile file ;

	if (file.Open( _T(name), CFile::modeCreate | CFile::modeWrite ))
	{
		file.Write( &ptCenterOffset, sizeof(CPoint) ) ;
		if(m_bCalibration)
		{
			for(int i=0;i<4;i++)
			{
				file.Write( &m_cvTrans_mat[i], UINT(m_cvTrans_mat[i].total()*m_cvTrans_mat[i].elemSize())) ;
			}	

			file.Write( &m_ptRotateCenter, sizeof(PointF) ) ;
			file.Write( &m_dResolutionX, sizeof(double) ) ;
			file.Write( &m_dResolutionY, sizeof(double) ) ;
			
		}
		file.Close() ;
	}	

	return TRUE;
}
BOOL CHalconProcessing::halcon_CameraToRobotData(std::vector<UPoint<double>> vecSrcPts,std::vector<UPoint<double>> vecDstPts,double stage_cx,double stage_cy)
{
	if(vecSrcPts.size()<9 || vecDstPts.size()<9) return FALSE;

	cv::Point2f src_p[4];
	cv::Point2f dst_p[4];

	int j=0;

	for(int i=0;i<4;i++)
	{
		switch (i)
		{
			case 0: j=0; break;
			case 1: j=1; break;
			case 2: j=3; break;
			case 3: j=4; break;
		}

		// from points 
		src_p[0] = cv::Point2f(float(vecSrcPts[0+j].x), float(vecSrcPts[0+j].y)); 
		src_p[1] = cv::Point2f(float(vecSrcPts[1+j].x), float(vecSrcPts[1+j].y)); 
		src_p[2] = cv::Point2f(float(vecSrcPts[4+j].x), float(vecSrcPts[4+j].y)); 
		src_p[3] = cv::Point2f(float(vecSrcPts[3+j].x), float(vecSrcPts[3+j].y)); 

		// to points 
		dst_p[0] = cv::Point2f(float(vecDstPts[0+j].x-stage_cx), float(vecDstPts[0+j].y-stage_cy)); 
		dst_p[1] = cv::Point2f(float(vecDstPts[1+j].x-stage_cx), float(vecDstPts[1+j].y-stage_cy)); 
		dst_p[2] = cv::Point2f(float(vecDstPts[4+j].x-stage_cx), float(vecDstPts[4+j].y-stage_cy));  
		dst_p[3] = cv::Point2f(float(vecDstPts[3+j].x-stage_cx), float(vecDstPts[3+j].y-stage_cy)); 

		m_cvTrans_mat[i] = cv::getPerspectiveTransform(src_p, dst_p);
	}
	
	//  0	1	2
	//	3	4	5
	//	6	7	8

	double res_x = 0.0, res_y = 0.0, res_x2, res_y2;

	if( (vecSrcPts[2].x - vecSrcPts[0].x) == 0.0 || (vecSrcPts[8].x - vecSrcPts[6].x) == 0.0 ||
		(vecSrcPts[6].y - vecSrcPts[0].y) == 0.0 || (vecSrcPts[8].y - vecSrcPts[2].y) == 0.0)
	{
		m_dResolutionX = -1.0;
		m_dResolutionY = -1.0;
	}
	else
	{
		res_x = (vecDstPts[2].x - vecDstPts[0].x) / (vecSrcPts[2].x - vecSrcPts[0].x);
		res_x2 = (vecDstPts[8].x - vecDstPts[6].x) / (vecSrcPts[8].x - vecSrcPts[6].x);
		res_y = (vecDstPts[6].y - vecDstPts[0].y) / (vecSrcPts[6].y - vecSrcPts[0].y);
		res_y2 = (vecDstPts[8].y - vecDstPts[2].y) / (vecSrcPts[8].y - vecSrcPts[2].y);

		m_dResolutionX = ( res_x + res_x2 ) / 2.0;
		m_dResolutionY = ( res_y + res_y2 ) / 2.0;
	}

	m_bCalibration = true;

	return TRUE;
}
UPoint<double> CHalconProcessing::halcon_GetRobotData(UPoint<double> vecSrcPts, int w,int h)
{
	UPoint<double> vecPts={-1,-1};

	if(!m_bCalibration) return vecPts;		

	int index=0;
	if(vecSrcPts.x<=w/2 && vecSrcPts.y<=h/2) index=0;
	else if(vecSrcPts.x>=w/2 && vecSrcPts.y<=h/2) index=1;
	else if(vecSrcPts.x<=w/2 && vecSrcPts.y>=h/2) index=2;
	else if(vecSrcPts.x>=w/2 && vecSrcPts.y>=h/2) index=3;	

	std::vector<cv::Point2f>  srcPoints,dstPoints;
	srcPoints.push_back(cv::Point2f(float(vecSrcPts.x), float(vecSrcPts.y)));
	cv::perspectiveTransform(srcPoints, dstPoints, m_cvTrans_mat[index]);

	vecPts.x = dstPoints[0].x;
	vecPts.y = dstPoints[0].y;

	return vecPts;
}
UPoint<double> CHalconProcessing::findAutoCenterPos(unsigned char *pImage,int w,int h,CRect rect)
{
	UPoint<double> vecPts={-1,-1};

	cv::Mat  SrcImg(h, w, CV_8UC1, pImage);//메모리 공유
	cv::Rect myROI(rect.left-4,rect.top-4,rect.Width()+8,rect.Height()+8);
	
	cv::Mat  m_bmpSrcImg = SrcImg(myROI).clone();

	cv::threshold(m_bmpSrcImg,m_bmpSrcImg,0,255,cv::THRESH_BINARY+cv::THRESH_OTSU);
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Point> largecontours;
	cv::findContours(m_bmpSrcImg, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

	double largestArea = 0;

	for( int i = 0; i < contours.size(); i++ )
	{
		if (contours[i].size()  >largestArea)
		{
			largecontours=contours[i];
			largestArea = double(contours[i].size());
		}
	}
	if(largestArea>0)
	{
		cv::Moments mu = moments( largecontours, false ); 
		if(mu.m00!=0)
		{
			cv::Point2f center = cv::Point2f( static_cast<float>(mu.m10/mu.m00) , static_cast<float>(mu.m01/mu.m00) ); 
			vecPts.x = myROI.x + center.x;		vecPts.y = myROI.y + center.y;
		}
		
	}	

	return vecPts;
}
//수학적좌표계 X:좌->우(+), Y:하->상(+), Theta:시계CCW(+)

/**
@brief 세점을 지나는 원의 중심점과 반지름을 구한다.
@param * r:  원의 반지름
@param * psRelMatchPos:  원의 중심점 좌표
@param * psCenterPoint:  세점들의 좌표
*/
void CHalconProcessing ::halcon_GetThreePoint_Circle(float* r, UPoint<double> &psRelMatchPos, std::vector<UPoint<double>> psCenterPoint)
{
	float d1=float(1000000000000),d2=float(1000000000000);
	float cx=0,cy=0;
	if(psCenterPoint[1].y - psCenterPoint[0].y!=0)	 d1 = float((psCenterPoint[1].x - psCenterPoint[0].x)/(psCenterPoint[1].y - psCenterPoint[0].y)); 
	if(psCenterPoint[2].y - psCenterPoint[1].y!=0)	 d2 = float((psCenterPoint[2].x - psCenterPoint[1].x)/(psCenterPoint[2].y - psCenterPoint[1].y)); 

	if(d2 - d1!=0) cx = float(((psCenterPoint[2].y - psCenterPoint[0].y) + (psCenterPoint[1].x + psCenterPoint[2].x) * d2 - (psCenterPoint[0].x + psCenterPoint[1].x) * d1)/(2 * (d2 - d1))); 
	cy = float((-d1 * (cx-(psCenterPoint[0].x + psCenterPoint[1].x)/2) + (psCenterPoint[0].y + psCenterPoint[1].y)/2 ));

	m_ptRotateCenter.X = Gdiplus::REAL(psRelMatchPos.x = cx);
	m_ptRotateCenter.Y = Gdiplus::REAL(psRelMatchPos.y = cy);

	
	*r = sqrt(pow((float)(psCenterPoint[0].x - cx), 2) + pow((float)(psCenterPoint[0].y - cy), 2));
}
PointF CHalconProcessing ::halcon_CenterPointFrom3Points(PointF startPoint, PointF middlePoint, PointF endPoint) 
{
	double centerX = 0, centerY = 0;
#ifdef _ORGCALC
	double ma= 1000000000000,mb= 1000000000000;	

	if ((middlePoint.X - startPoint.X)!=0) 	ma = (middlePoint.Y - startPoint.Y) / (middlePoint.X - startPoint.X); 
	if ((endPoint.X - middlePoint.X)!=0)   mb = (endPoint.Y - middlePoint.Y) / (endPoint.X - middlePoint.X); 
	
	if (ma == 0.f) 		ma = 0.000000000001; 
	if (mb == 0.f) 		mb = 0.000000000001; 

	if(mb-ma!=0)
		centerX = (ma * mb * (startPoint.Y - endPoint.Y) + mb * (startPoint.X + middlePoint.X) -	ma * (middlePoint.X + endPoint.X)) / (2 * (mb - ma)); 
	 centerY = (-1 * (centerX - (startPoint.X + middlePoint.X) / 2) / ma) + ((startPoint.Y + middlePoint.Y) / 2); 
	 
#else
	cv::Point2f  CenPoint[2];

	CenPoint[0].x = 0.5 * (startPoint.X + middlePoint.X);
	CenPoint[0].y = 0.5 * (startPoint.Y + middlePoint.Y);
	CenPoint[1].x = 0.5 * (startPoint.X + endPoint.X);
	CenPoint[1].y = 0.5 * (startPoint.Y + endPoint.Y);

	double a1 = (startPoint.Y - middlePoint.Y != 0) ? (-1 * (startPoint.X - middlePoint.X) / (startPoint.Y - middlePoint.Y)): 0.000000000001;
	double b1 = CenPoint[0].y - a1 * CenPoint[0].x;
	double a2 = (startPoint.Y - endPoint.Y != 0) ? (-1 * (startPoint.X - endPoint.X) / (startPoint.Y - endPoint.Y)) : 0.000000000001;
	double b2 = CenPoint[1].y - a2 * CenPoint[1].x;

	if (a1 != a2)
	{
		centerX = (b2 - b1) / (a1 - a2);
		centerY = a1 * centerX + b1;
	}
#endif
	m_ptRotateCenter.X = Gdiplus::REAL(centerX);
	m_ptRotateCenter.Y = Gdiplus::REAL(centerY);

	return  PointF((float)centerX, (float)centerY); 
}
bool CHalconProcessing ::halcon_GetTwoPointTheta_Circle(UPoint<double> &psRelMatchPos, std::vector<UPoint<double>> psCenterPoint,double angle)
{
	double rangle = -angle * (PI / 180.0);

	double dx = (psCenterPoint[1].x - psCenterPoint[0].x);
	double dy = (psCenterPoint[1].y - psCenterPoint[0].y);

	if ((dx * dx + dy * dy) <= 0.0000001 && (rangle * rangle) <= 0)
	{
		return false;
	}

	psRelMatchPos.x = ((dx * (cos(rangle) - 1) - dy * sin(rangle)) / (2 * (cos(rangle) - 1)) - dx);
	psRelMatchPos.y = ((dy - psRelMatchPos.x * sin(rangle)) / (cos(rangle) - 1));
	psRelMatchPos.x *= -1;
	psRelMatchPos.y *= -1;
	psRelMatchPos.x += psCenterPoint[0].x;
	psRelMatchPos.y += psCenterPoint[0].y;

	m_ptRotateCenter.X = Gdiplus::REAL(psRelMatchPos.x);
	m_ptRotateCenter.Y = Gdiplus::REAL(psRelMatchPos.y);

	return true;
}
/*
 @brief 두점을 지나는 원의 중심점을 구한다.
 @param * psRelMatchPos 원의 중심점 좌표
 @param fR 반지름
 @param * psCenterPoint 첫번째 점과 두번째 점의 좌표
 @param Direction 좌측 또는 우측방향
 */
void CHalconProcessing ::halcon_GetTwoPoint_Circle(UPoint<double> &psRelMatchPos, const float fR, std::vector<UPoint<double>> psCenterPoint, const BOOL Direction)
 {
	float fdx = float((psCenterPoint[1].x - psCenterPoint[0].x));
	float fdy = float((psCenterPoint[1].y - psCenterPoint[0].y));
	float fxc = float((psCenterPoint[1].x + psCenterPoint[0].x) / 2.0);
	float fyc = float((psCenterPoint[1].y + psCenterPoint[0].y) / 2.0);
 
	float fd = sqrt(fdx * fdx + fdy * fdy); //두점사이의 거리
	float fd2 = fd/2; //거리의 중간
	float fOffset = sqrt(fR * fR - fd2 * fd2);
 
	float fplusx=0,fplusy=0;
	if(fd!=0.f)
	{
		fplusx = fOffset * fdy/fd;
		fplusy = fOffset * fdx/fd;
	}	
 
	if(Direction == 1)
	{
		psRelMatchPos.x = fxc + fplusx;
		psRelMatchPos.y = fyc - fplusy;
	}
	else// if(Direction == 0)
	{
		psRelMatchPos.x = fxc - fplusx;
		psRelMatchPos.y = fyc + fplusy;
	}

	m_ptRotateCenter.X =Gdiplus::REAL(psRelMatchPos.x);
	m_ptRotateCenter.Y =Gdiplus::REAL(psRelMatchPos.y);
}
void CHalconProcessing ::halcon_CalcRotationCenter2(UPoint<double> &psRelMatchPos, std::vector<UPoint<double>> psCenterPoint,double angle)
{
    double rangle = -angle * (PI / 180.0);

    double dataX2[2] = { psCenterPoint[1].x, psCenterPoint[1].y };
    cv::Mat A =  cv::Mat(2, 2, CV_64FC1);

    A.at<double>(0, 0) =  -1 + cos(rangle);
    A.at<double>(0, 1) =  sin(rangle) * -1;
    A.at<double>(1, 0) =  sin(rangle);
    A.at<double>(1, 1) =  -1 + cos(rangle);

    cv::Mat B = cv::Mat(2, 2, CV_64FC1);

    B.at<double>(0, 0) =  cos(rangle);
    B.at<double>(0, 1) =  sin(rangle) * -1;
    B.at<double>(1, 0) =  sin(rangle);
    B.at<double>(1, 1) =  cos(rangle);

    cv::Mat x1 = cv::Mat(2, 1, CV_64F);
	x1.at<double>(0, 0) =  psCenterPoint[0].x;
    x1.at<double>(0, 1) =  psCenterPoint[0].y;

    cv:: Mat x2 = cv::Mat(2, 1, CV_64F);
	x2.at<double>(0, 0) = psCenterPoint[1].x;
    x2.at<double>(0, 1) = psCenterPoint[1].y;

    try
    {
       cv::Mat x = A.inv() * B * x1 - A.inv() * x2;

        m_ptRotateCenter.X = Gdiplus::REAL(x.at<double>(0, 0));
        m_ptRotateCenter.Y = Gdiplus::REAL(x.at<double>(1, 0));
    }
    catch(...)
    {
    }

	return ;
}
void CHalconProcessing ::halcon_CalcRotationCenter(UPoint<double> &psRelMatchPos, std::vector<UPoint<double>> psCenterPoint,double angle)
{
    double rangle = -angle * (PI / 180.0);

    double sinT = sin(rangle);
    double cosT = cos(rangle);
    double x1 = psCenterPoint[0].x;
    double y1 =  psCenterPoint[0].y;
    double x2 =  psCenterPoint[1].x;
    double y2 = psCenterPoint[1].y;

	cv::Mat A =  cv::Mat(2, 2, CV_64FC1);

    A.at<double>(0, 0) = 1 - cosT;
    A.at<double>(0, 1) = sinT;
    A.at<double>(1, 0) = sinT;
    A.at<double>(1, 1) = -1 + cosT;

    cv::Mat B = cv::Mat(2, 1, CV_64FC1);
    B.at<double>(0, 0)= y1 * sinT - x1 * cosT + x2;
    B.at<double>(1, 0)= x1 * sinT + y1 * cosT - y2;

    try
    {
        cv::Mat x = A.inv() * B;
        m_ptRotateCenter.X = Gdiplus::REAL(x.at<double>(0, 0));
        m_ptRotateCenter.Y = Gdiplus::REAL(x.at<double>(1, 0));
    }
    catch(...)
    {  
    }
            return ;
}
double CHalconProcessing::GetAngleFromPoint(Point point, Point centerPoint)
{
	double dy = (point.Y - centerPoint.Y);
	double dx = (point.X - centerPoint.X);

	//double theta = atan2(dx,dy);  //// double theta = Math.Atan2(dy,dx);   ///
	//double angle = double(int(90 - ((theta * 180) / PI)) % 360);

	double theta = atan2(dx, -dy);
	double angle = int((theta * 180 / PI) + 360) % 360;	

	return  angle * 180 / PI;
}
bool  CHalconProcessing::halcon_GetSlopeResolutionFromPoint(Point first, Point second,double &slope,double &resolution,int distance,int dir)
{
	if(second.X - first.X==0) slope=0;
	else slope = double (second.Y - first.Y) / (second.X - first.X);

	resolution= 0;

	if(dir==1 && (second.Y - first.Y)!=0)	resolution = distance / double (second.Y - first.Y);
	else if(dir==0 && (second.X - first.X)!=0) resolution = distance  / double (second.X - first.X);

	return true;
}
void CHalconProcessing::writeMatToFile(cv::Mat& m, const char* filename)
{
	cv::FileStorage file(filename, cv::FileStorage::WRITE);

	file << "Matrix" << m;
	file.release();  

}
void CHalconProcessing::readMatToFile(cv::Mat& m, const char* filename)
{
	cv::FileStorage storage(filename, cv::FileStorage::READ);
	storage["Matrix"] >> m;
	storage.release();

	m_bCalibration = true;
}
void CHalconProcessing::halcon_CalcRotate(double x1, double y1, double rangle, double *x2, double *y2)
{
	double tmpx,tmpy;
	tmpx = x1 - m_ptRotateCenter.X;
	tmpy = y1 - m_ptRotateCenter.Y;

	*x2 = (tmpx * cos(rangle) - tmpy * sin(rangle) + m_ptRotateCenter.X);
	*y2 = (tmpx * sin(rangle) + tmpy * cos(rangle) + m_ptRotateCenter.Y);
}
BOOL CHalconProcessing::halcon_CalcPrealign(UPoint<double> ptLeftPos, UPoint<double>ptRightPos,UPoint<double> ptMarkPitch)
{
	try{

		memset( m_dbPrealignResult, 0, sizeof(m_dbPrealignResult) );

		double dbAlignTH, dbAlignX, dbAlignY;
		double dbDistY = ptLeftPos.y - ptRightPos.y;
		dbAlignTH = asin( dbDistY / ptMarkPitch.x ) / PI * 180.0;

		halcon_CalcRotate(ptLeftPos.x, ptLeftPos.y, dbAlignTH * PI / 180.0, &dbAlignX, &dbAlignY);

		m_dbPrealignResult[0] = -dbAlignX;
		m_dbPrealignResult[1] = -dbAlignY;
		m_dbPrealignResult[2] = -dbAlignTH;
	}
	catch(...)
	{
		return TRUE;
	}

	return FALSE;
}
void CHalconProcessing::halcon_releaseModel()
{
	if(m_bModelRead)
	{
#ifdef _CPP_USE_HALCON
		if(m_bNccSelect) ClearNccModel(hv_ModelId);
		else ClearShapeModel(hv_ModelId);
#else
		if(m_bNccSelect) clear_ncc_model(hv_ModelId);
		else clear_shape_model(hv_ModelId);	
#endif
		m_bModelRead = FALSE;
	}
}
double CHalconProcessing::halcon_GetLinearData(double vecSrcPts, std::vector<double> vtThetaReal)
{
	double vecPts = -1;

	if(!m_bLinearCalibraion) return vecPts;	
	int index=0;
	BOOL bFind = FALSE;

	for(int i = 0; i < vtThetaReal.size() - 1; i++ )
	{
		if( vecSrcPts > vtThetaReal[i] && vecSrcPts <= vtThetaReal[i + 1] )
		{
			bFind = TRUE;
			break;
		}
		
		index++;
	}
/*
	do{
		if(startAngle+(step*(index+1))>vecSrcPts)
			break;
		index++;
	}while(index<m_cvLinearCalib.size());
*/
	if( !bFind )
	{
		vecPts = vecSrcPts;
		return vecPts;
	}

	std::vector<cv::Point2f> srcPoints, dstPoints;
	srcPoints.push_back(cv::Point2f(float(vecSrcPts), float(vecSrcPts)));
	cv::perspectiveTransform(srcPoints, dstPoints, m_cvLinearCalib[index]);

	vecPts = dstPoints[0].y;

	return vecPts;
}
double CHalconProcessing::halcon_GetLinearData(double vecSrcPts,double startAngle,double step)
{
	double vecPts = -1;

	if(!m_bLinearCalibraion) return vecPts;		

	int index=0;
	do{
		if(startAngle+(step*(index+1))>vecSrcPts)
			break;
		index++;
	}while(index<m_cvLinearCalib.size());

	std::vector<cv::Point2f>  srcPoints,dstPoints;
	srcPoints.push_back(cv::Point2f(float(vecSrcPts), float(vecSrcPts)));
	cv::perspectiveTransform(srcPoints, dstPoints, m_cvLinearCalib[index]);

	vecPts = dstPoints[0].y;

	return vecPts;
}
bool CHalconProcessing::halcon_LinearCalibraion(std::vector<double> src,std::vector<double> dst)
{
	int nCount = int(src.size());

	if(nCount>=2)
	{
		cv::Point2f src_p[4];
		cv::Point2f dst_p[4];

		for(int i=0;i<m_cvLinearCalib.size();i++)
			m_cvLinearCalib.at(i).release();

		m_cvLinearCalib.clear();

		int loopC = nCount-1;
		for(int i=0;i<loopC;i++)
		{
			// from points 
			src_p[0] = cv::Point2f(float(src[i]), float(src[i])); 
			src_p[1] = cv::Point2f(float(src[i+1]), float(src[i])); 
			src_p[2] = cv::Point2f(float(src[i+1]), float(src[i+1])); 
			src_p[3] = cv::Point2f(float(src[i]), float(src[i+1])); 

			// to points 
			dst_p[0] = cv::Point2f(float(src[i]), float(dst[i])); 
			dst_p[1] = cv::Point2f(float(src[i+1]), float(dst[i])); 
			dst_p[2] = cv::Point2f(float(src[i+1]), float(dst[i+1])); 
			dst_p[3] = cv::Point2f(float(src[i]), float(dst[i+1])); 

			m_cvLinearCalib.push_back(cv::getPerspectiveTransform(src_p, dst_p));
		}

		m_bLinearCalibraion = true;
	}
	else return false;

	return true;
}
//HTK 2022-03-30 Halcon Read Para Job별로 분기
void CHalconProcessing::saveParameter(CString fname,int ncam,int npos,int nindex, int njob)
{
	//CFileStatus fs;
	CString str;

	//if( CFile::GetStatus(fname, fs) )
	{
		CString section= "MARK_MODEL";
		str.Format("_%d_%d_%d_%d",ncam, npos, nindex,njob);

		::JXWritePrivateProfileString(section,"MARK_NUMLEVEL"+str,NumberToString(m_ReadNumLevel).c_str(),fname);
		::JXWritePrivateProfileString(section,"MARK_START_ANGLE"+str, NumberToString(m_ReadStartAngle).c_str(),fname);
		::JXWritePrivateProfileString(section,"MARK_END_ANGLE"+str, NumberToString(m_ReadEndAngle).c_str(),fname);
		::JXWritePrivateProfileString(section,"MARK_STEP_ANGLE"+str, NumberToString(m_ReadStepAngle).c_str(),fname);
		::JXWritePrivateProfileString(section,"MARK_OPTIMIZE"+str,m_ReadOptimize,fname);
		::JXWritePrivateProfileString(section,"MARK_METRIC"+str, m_ReadMetric,fname); 
		::JXWritePrivateProfileString(section,"MARK_CONTRAST"+str, NumberToString(m_ReadContrast).c_str(),fname);
		::JXWritePrivateProfileString(section,"MARK_MINCONTRAST"+str, NumberToString(m_ReadMinContrast).c_str(),fname);
		::JXWritePrivateProfileString(section, "MARK_SMALL_SCALE" + str, NumberToString(m_ReadSmallScale).c_str(), fname);

		::JXWritePrivateProfileString(section, "MARK_FIND_NUMMATCH"+str, NumberToString(m_SearchNumMatch).c_str(),fname) ;
		::JXWritePrivateProfileString(section, "MARK_FIND_STARTANGLE"+str,NumberToString(m_SearchStartAngle).c_str(),fname);
		::JXWritePrivateProfileString(section, "MARK_FIND_ENDANGLE"+str, NumberToString(m_SearchEndAngle).c_str(),fname);
		::JXWritePrivateProfileString(section, "MARK_FIND_MINSCORE"+str, NumberToString(m_SearchMinscore).c_str(),fname);
		::JXWritePrivateProfileString(section, "MARK_FIND_GREEDNESS"+str,NumberToString(m_SearchGreedness).c_str(),fname);
		::JXWritePrivateProfileString(section, "MARK_FIND_SUBPIXEL"+str, m_SearchSubPixel,fname);
		::JXWritePrivateProfileString(section, "MARK_FIND_METHOD"+str, NumberToString(m_SearchMethod).c_str(),fname) ;

		::JXWritePrivateProfileString(section, "MARK_PRE_PROCESSING" + str, NumberToString(m_bPreProcess).c_str(), fname);

		::JXUnloadPrivateProfile();
	}
}
//HTK 2022-03-30 Halcon Read Para Job별로 분기
void CHalconProcessing::readParameter(CString fname,int ncam,int npos,int nindex,int njob)
{
	CFileStatus fs;
	CString str;

	if( CFile::GetStatus(fname, fs) )
	{
		INIReader reader(fname.GetBuffer(0));
		if (reader.ParseError() != 0)      return ;

		std::string section= "MARK_MODEL";
		str.Format("_%d_%d_%d_%d",ncam, npos, nindex, njob);

		if(!reader.HasValue(section, LPCTSTR("MARK_NUMLEVEL" + str)))
			str.Format("_%d_%d_%d", ncam, npos, nindex);

		m_ReadNumLevel = (int)reader.GetInteger(section,LPCTSTR("MARK_NUMLEVEL"+str), 0);
		m_ReadStartAngle = reader.GetReal(section, LPCTSTR("MARK_START_ANGLE"+str), 0);
		m_ReadEndAngle = reader.GetReal(section, LPCTSTR("MARK_END_ANGLE"+str), 360);
		m_ReadStepAngle = reader.GetReal(section, LPCTSTR("MARK_STEP_ANGLE"+str), -1);
		m_ReadOptimize = reader.Get(section, LPCTSTR("MARK_OPTIMIZE"+str),"Point_reduction_low").c_str();
		m_ReadMetric = reader.Get(section, LPCTSTR("MARK_METRIC"+str), "use_polarity").c_str(); 
		m_ReadContrast = reader.GetReal(section, LPCTSTR("MARK_CONTRAST"+str), -1);
		m_ReadMinContrast = reader.GetReal(section, LPCTSTR("MARK_MINCONTRAST"+str), -1);
		m_ReadSmallScale = reader.GetReal(section, LPCTSTR("MARK_SMALL_SCALE" + str), 0.2);

		m_SearchNumMatch = (int)reader.GetInteger(section, LPCTSTR("MARK_FIND_NUMMATCH"+str), 1) ;
		m_SearchStartAngle = reader.GetReal(section,LPCTSTR("MARK_FIND_STARTANGLE"+str), 0);
		m_SearchEndAngle = reader.GetReal(section, LPCTSTR("MARK_FIND_ENDANGLE"+str), 360);
		m_SearchMinscore = reader.GetReal(section, LPCTSTR("MARK_FIND_MINSCORE"+str), 0.5);
		m_SearchGreedness = reader.GetReal(section, LPCTSTR("MARK_FIND_GREEDNESS"+str), 0.9);
		m_SearchSubPixel = (LPCTSTR)reader.Get(section, LPCTSTR("MARK_FIND_SUBPIXEL"+str), "least_squares").c_str() ;
		m_SearchMethod = (int)reader.GetInteger(section, LPCTSTR("MARK_FIND_METHOD"+str), 0) ;

		m_bPreProcess = (int)reader.GetInteger(section, LPCTSTR("MARK_PRE_PROCESSING" + str), 0);
	}

}
void CHalconProcessing::halconWriteModel(CString modelName)
{
	if(m_SearchMethod==1)
	{
		write_ncc_model(hv_ModelId,(LPCTSTR)modelName);
	}
	else
	{
		write_shape_model(hv_ModelId, (LPCTSTR)modelName);
	}
}
void CHalconProcessing::halconReadModel(CString modelName)
{
	if(m_SearchMethod==1)
	{
		clear_ncc_model(hv_ModelId);
		read_ncc_model((LPCTSTR)modelName, &hv_ModelId);
	}
	else
	{
		clear_shape_model(hv_ModelId);
		read_shape_model((LPCTSTR)modelName, &hv_ModelId);
	}
}

bool CHalconProcessing::halcon_ShowModel(BYTE *pImg,int w,int h,CRect rect,HWND HwndView,bool bshowimg)
{
	if(hv_ModelId.Num() == 0) return false;

	Hobject  Model,Image;
	Hobject ContoursAffinTrans1,ROI_0;
	HTuple HomMat2D2, Row2, Column2,tmpWidth,tmpHeight,Area1;
	HTuple yoffset,xoffset;

	if( m_bNccSelect )	get_ncc_model_origin(hv_ModelId, &yoffset, &xoffset);
	else				get_shape_model_origin(hv_ModelId, &yoffset, &xoffset);

	double y = yoffset[0].D();
	double x = xoffset[0].D();

	gen_image1(&Image,"byte",w,h,(Hlong)pImg);	
	get_image_size(Image,&tmpWidth,&tmpHeight);
	set_check("~father"); 
	open_window(0,0,rect.Width(),rect.Height(),Hlong(HwndView),"transparent","",&m_hHalHwndView);
	set_check("father");
	set_part(m_hHalHwndView, 0, 0, tmpHeight - 1, tmpWidth - 1);// 

	if(bshowimg)
	{
		clear_window(m_hHalHwndView);	
		disp_obj(Image,m_hHalHwndView);  
	}

	gen_rectangle1 (&ROI_0, 0, 0, tmpHeight,tmpWidth);
	area_center(ROI_0, &Area1, &Row2, &Column2);
	vector_angle_to_rigid (-yoffset+m_nFatOffsetHeight, -xoffset+m_nFatOffsetWidth, 0, Row2, Column2, 0, &HomMat2D2);

	get_shape_model_contours(&Model, hv_ModelId, 1);
	affine_trans_contour_xld(Model, &ContoursAffinTrans1, HomMat2D2) ;

	set_color(m_hHalHwndView,"red");
	set_line_width(m_hHalHwndView,2);
    set_draw(m_hHalHwndView,"margin");
	disp_obj(ContoursAffinTrans1,m_hHalHwndView);
   
   return true;
}
void CHalconProcessing::halcon_ShowModelClose()
{
	if(m_hHalHwndView!=NULL)
		close_window(m_hHalHwndView);
}
bool CHalconProcessing::halcon_DisplayShapeModel(BYTE *m_workImg,int low,int high)
{
	CString msg;

	Hobject  Image,ModelTrans,Model;
	Hobject  ROI_0;
	HTuple  Area, RowRef, ColumnRef, HomMat2D, Row, Column, ModelID;
	HTuple  tmpWidth,tmpHeight,yoffset,xoffset;
	
	gen_image1(&Image,"byte",m_nFatWidth,m_nFatHeight,(Hlong)m_workImg);
	get_image_size(Image,&tmpWidth,&tmpHeight);

	clear_window(m_hHalHwndView);
	disp_obj(Image, m_hHalHwndView);

	set_color(m_hHalHwndView,"red");
    set_draw(m_hHalHwndView,"margin");

	msg.Format("Mincontrast : %d / Contrast : %d",low,high);
	write_string(m_hHalHwndView, (LPCTSTR)msg);

	//HTK 2022-04-13 Mark Angle Search
	create_shape_model(Image,m_ReadNumLevel, HTuple(m_ReadStartAngle).Rad(), HTuple(m_ReadEndAngle- m_ReadStartAngle).Rad(),m_ReadStepAngle!=-1?HTuple(m_ReadStepAngle).Rad():"auto" ,
			m_ReadOptimize=="none"?"auto":m_ReadOptimize,(LPCTSTR) m_ReadMetric, high, low, &ModelID);

	if(ModelID.Num()==0) return false;

	get_shape_model_contours(&Model, ModelID, 1);
	
	get_shape_model_origin(hv_ModelId, &yoffset, &xoffset);
	gen_rectangle1 (&ROI_0, 0, 0, tmpHeight,tmpWidth);
    area_center(ROI_0, &Area, &RowRef, &ColumnRef);
	vector_angle_to_rigid(0, 0, 0, RowRef, ColumnRef, 0, &HomMat2D);
    affine_trans_contour_xld(Model, &ModelTrans, HomMat2D);

    disp_obj(ModelTrans,m_hHalHwndView);

	return true;
}
HTuple CHalconProcessing::halcon_DispImage(IplImage *m_workImg,CRect rect,HWND HwndView )
{
	Hobject  Image;

	HTuple tmpWidth;
	HTuple tmpHeight;
	HTuple HalHwndView=NULL;

	Image=IplImageToHImage(m_workImg);

	open_window(0,0,rect.Width(),rect.Height(),Hlong(HwndView),"visible","",&HalHwndView);

	get_image_size(Image,&tmpWidth,&tmpHeight);
	set_part(HalHwndView, 0, 0, tmpHeight - 1, tmpWidth - 1);// 
	clear_window(HalHwndView);
	disp_obj(Image, HalHwndView);

	return HalHwndView;
}

void CHalconProcessing::halcon_DispImageToModelContour(cv::Mat *m_workImg,int col,int row ,CRect Inspr,CRect r,HWND hWnd)
{
	HTuple  hv_WindowHandle;
	Hobject  ModelRegion,TransContours,Image;
	HTuple  ModelRegionArea, RefRow, RefColumn, HomMat2D, HomMatSearch2D;
	Hobject ModelContours = getModelContour();
	HTuple ModelRow,ModelColumn;

//	Hlong MainWndID=(Hlong) hWnd;
//	gen_image1(&Image,"byte",col,row,(Hlong)m_workImg->data);
	
	//  모델 Contour 그리는 경우 사용
	//gen_rectangle1 (&ModelRegion, Inspr.left, Inspr.top, Inspr.right, Inspr.bottom);
	//area_center (ModelRegion, &ModelRegionArea, &RefRow, &RefColumn);
	//vector_angle_to_rigid (0, 0, 0, RefRow, RefColumn, 0, &HomMat2D);
	//affine_trans_contour_xld (ModelContours, &TransContours, HomMat2D);

	if(r.left==0 && r.top==0) 
	{
		ModelRow=hv_ModelRow;
		ModelColumn=hv_ModelColumn;
	}
	else 
	{
		ModelRow=r.top;
		ModelColumn=r.left;
	}


	get_shape_model_origin(hv_ModelId, &RefRow, &RefColumn);
	hom_mat2d_identity (&HomMatSearch2D);
	hom_mat2d_translate (HomMatSearch2D, ModelRow[0], ModelColumn[0], &HomMatSearch2D);
    hom_mat2d_rotate (HomMatSearch2D, hv_ModelAngle[0], ModelRow[0], ModelColumn[0], &HomMatSearch2D);
    
	hom_mat2d_translate (HomMatSearch2D,  -RefRow,-RefColumn, &HomMatSearch2D);
	hom_mat2d_translate (HomMatSearch2D, Inspr.top, Inspr.left,&HomMatSearch2D);
    affine_trans_contour_xld (ModelContours, &TransContours, HomMatSearch2D);

	//if (HDevWindowStack::IsOpen())    close_window(HDevWindowStack::Pop());
	//set_check("~father");
    //open_window(r.left,r.top,r.Width(),r.Height(),MainWndID,"transparent","",&hv_WindowHandle);
	//set_check("father"); 
	//set_draw(hv_WindowHandle,"margin");
	//HDevWindowStack::Push(hv_WindowHandle);

	//if (HDevWindowStack::IsOpen())
	//{
	//	disp_obj(Image, HDevWindowStack::GetActive());
	//	set_color(HDevWindowStack::GetActive(),"blue");
	//	disp_obj(TransContours, HDevWindowStack::GetActive());
	//}
	 
	 HTuple  hv_I, hv_Row, hv_Col,hv_Number;

	 count_obj(TransContours, &hv_Number);
	 if(hv_Number[0].I()<=0) return;

	 HTuple end_val13 = hv_Number,end_val18;
	 HTuple step_val13 = 1,step_val18=1,hv_J;
	 Hobject  ho_Line;

	 m_arrContourPoint.clear();

	 POINT pt;
	 std::vector<POINT> arrPoint; 

	 for (hv_I=1; hv_I.Continue(end_val13, step_val13); hv_I += step_val13)
	 {
		select_obj(TransContours, &ho_Line, hv_I);
		get_contour_xld(ho_Line, &hv_Row, &hv_Col);		
		end_val18 = (hv_Row.Num()) - 1;

		for (hv_J=1; hv_J.Continue(end_val18, step_val18); hv_J += step_val18)
		{
			pt.y = int( (hv_Row[hv_J].D()));
			pt.x = int( (hv_Col[hv_J].D()));			
			arrPoint.push_back(pt);		
		}

		 m_arrContourPoint.push_back(arrPoint);
		 arrPoint.clear();
	 }
}

///////////////////////////////////////////
//
//  Processing 관련 함수 
///////////////////////////////////////////

void estimate_background_illumination (Halcon::Hobject ho_Image, Halcon::Hobject *ho_IlluminationImage)
{
  Hobject  ho_ImageFFT, ho_ImageGauss, ho_ImageConvol;
  HTuple  hv_ExpDefaultCtrlDummyVar, hv_Width, hv_Height;

  get_image_pointer1(ho_Image, &hv_ExpDefaultCtrlDummyVar, &hv_ExpDefaultCtrlDummyVar, &hv_Width, &hv_Height);
  rft_generic(ho_Image, &ho_ImageFFT, "to_freq", "none", "complex", hv_Width);
  gen_gauss_filter(&ho_ImageGauss, 50, 50, 0, "n", "rft", hv_Width, hv_Height);
  convol_fft(ho_ImageFFT, ho_ImageGauss, &ho_ImageConvol);
  rft_generic(ho_ImageConvol, &(*ho_IlluminationImage), "from_freq", "none", "byte",  hv_Width);

  return;
}
void calculate_lines_gauss_parameters (Halcon::HTuple hv_MaxLineWidth, Halcon::HTuple hv_Contrast,Halcon::HTuple *hv_Sigma, Halcon::HTuple *hv_Low, Halcon::HTuple *hv_High)
{

  // Local control variables 
  HTuple  hv_ContrastHigh, hv_ContrastLow, hv_HalfWidth;
  HTuple  hv_Help;

  if (0 != ((hv_MaxLineWidth.Num())!=1)) return ;
  if (0 != ((hv_MaxLineWidth.IsNumber()).Not())) return ;
  if (0 != (hv_MaxLineWidth<=0))return ;
  if (0 != (HTuple((hv_Contrast.Num())!=1).And((hv_Contrast.Num())!=2)))return ;
  if (0 != (((hv_Contrast.IsNumber()).Min())==0))return ;

  hv_ContrastHigh = hv_Contrast[0];
  if (0 != (hv_ContrastHigh<0))return ;

  if (0 != ((hv_Contrast.Num())==2))    hv_ContrastLow = hv_Contrast[1];
  else   hv_ContrastLow = hv_ContrastHigh/3.0;


  if (0 != (hv_ContrastLow<0)) return ;
  if (0 != (hv_ContrastLow>hv_ContrastHigh))return ;

  if (0 != (hv_MaxLineWidth<(HTuple(3.0).Sqrt())))
  {
    hv_ContrastLow = (hv_ContrastLow*hv_MaxLineWidth)/(HTuple(3.0).Sqrt());
    hv_ContrastHigh = (hv_ContrastHigh*hv_MaxLineWidth)/(HTuple(3.0).Sqrt());
    hv_MaxLineWidth = HTuple(3.0).Sqrt();
  }

  hv_HalfWidth = hv_MaxLineWidth/2.0;
  (*hv_Sigma) = hv_HalfWidth/(HTuple(3.0).Sqrt());
  hv_Help = ((-2.0*hv_HalfWidth)/((HTuple(6.283185307178).Sqrt())*((*hv_Sigma).Pow(3.0))))*((-0.5*((hv_HalfWidth/(*hv_Sigma)).Pow(2.0))).Exp());
  (*hv_High) = (hv_ContrastHigh*hv_Help).Abs();
  (*hv_Low) = (hv_ContrastLow*hv_Help).Abs();
}
IplImage* CHalconProcessing::halcon_linePreProcTest(BYTE *psrc, int w,int h,int bProc)
{
  Hobject  ho_Angio;
  HTuple  hv_Width, hv_Height;

  gen_image1(&ho_Angio,"byte",w,h,(Hlong)psrc);		//Image Load
  get_image_size(ho_Angio, &hv_Width, &hv_Height);

  Hobject  ho_Seg, ho_EdgeAmplitude, ho_SelectedRegions1;
  Hobject  ho_BinImage;

  if(bProc==1)
  {
	  auto_threshold(ho_Angio, &ho_Seg, 3.0);
	  connection(ho_Seg, &ho_EdgeAmplitude);
	  select_shape(ho_EdgeAmplitude, &ho_SelectedRegions1, "area", "and", 10, 99999);
	  region_to_bin(ho_SelectedRegions1, &ho_BinImage, 255, 0, hv_Width, hv_Height);
  }
  else if(bProc==2)
  {
	edges_image(ho_Angio,&ho_EdgeAmplitude,&ho_SelectedRegions1,"lanser2",0.5,"none",-1,-1);
	hysteresis_threshold(ho_EdgeAmplitude,&ho_SelectedRegions1,10,15,30);
	region_to_bin(ho_SelectedRegions1, &ho_BinImage, 255, 0, hv_Width, hv_Height);
  }

  return HImageToIplImage(ho_BinImage);
}
bool CHalconProcessing::halcon_lines_gauss(BYTE *psrc, int w,int h,CRect InspROI, int MinLineLength,int MaxLineWidth, int Contrast,bool bwhite, bool bShow,int id,int bProc,bool reverse)
{
	Hobject  ho_ROI_0,ho_Lens,ho_ImagePart;

  Hobject  ho_Angio, ho_Lines, ho_Line,ho_SelectedContours;
  Hobject  ho_ContoursSplit,ho_SortedContours;

  HTuple  hv_Width, hv_Height, hv_WindowID, hv_MaxLineWidth;
  HTuple  hv_Contrast, hv_Sigma, hv_Low, hv_High, hv_Number;
  HTuple  hv_I, hv_Row, hv_Col, hv_Angle, hv_WidthL, hv_WidthR;
  HTuple  hv_RowR, hv_ColR, hv_RowL, hv_ColL;
  HTuple  hv_Max, hv_Max2,hv_J,hv_Width2, hv_Height2;

  HTuple  hv_RowBegin, hv_ColBegin, hv_RowEnd, hv_ColEnd, hv_Nr, hv_Nc,hv_Dist;

  if(bShow)
  {
	if (HDevWindowStack::IsOpen())    close_window(HDevWindowStack::Pop());
  }
  gen_image1(&ho_Angio,"byte",w,h,(Hlong)psrc);		//Image Load
  get_image_size(ho_Angio, &hv_Width, &hv_Height);

  Hobject  ho_Seg, ho_EdgeAmplitude, ho_SelectedRegions1;
  Hobject  ho_BinImage,SE;

  if(bProc==1)
  {
	  if(reverse==true || id==0)
	  {
		  gen_disc_se(&SE, (const char *)"byte", Hlong(3), Hlong(3), 0.0);
		  gray_dilation (ho_Angio, SE, &ho_Angio);
	  }
	  auto_threshold(ho_Angio, &ho_Seg, 3.0);
	  connection(ho_Seg, &ho_EdgeAmplitude);
	  select_shape(ho_EdgeAmplitude, &ho_SelectedRegions1, "area", "and", 10, 99999);
	  region_to_bin(ho_SelectedRegions1, &ho_BinImage, 255, 0, hv_Width, hv_Height);
  }
  else if(bProc==2)
  {
	edges_image(ho_Angio,&ho_EdgeAmplitude,&ho_SelectedRegions1,"lanser2",0.5,"none",-1,-1);
	hysteresis_threshold(ho_EdgeAmplitude,&ho_SelectedRegions1,10,15,30);
	region_to_bin(ho_SelectedRegions1, &ho_BinImage, 255, 0, hv_Width, hv_Height);
  }

  if(InspROI.right>w) InspROI.right=w-1;
  if(InspROI.bottom>w) InspROI.bottom=h-1;
  if(InspROI.left<0) InspROI.left=0;
  if(InspROI.top<0) InspROI.top=0;

  gen_rectangle1(&ho_ROI_0,InspROI.top, InspROI.left, InspROI.bottom, InspROI.right);
  
  if(bProc>0)	reduce_domain(ho_BinImage, ho_ROI_0, &ho_Lens);
  else reduce_domain(ho_Angio, ho_ROI_0, &ho_Lens);
  
  crop_domain(ho_Lens, &ho_ImagePart);

  get_image_size(ho_ImagePart, &hv_Width2, &hv_Height2);

  if(bShow)
  {
	  set_window_attr("background_color","black");
	  open_window(0,0,hv_Width2/1.0,hv_Height2/1,0,"","",&hv_WindowID);
	  HDevWindowStack::Push(hv_WindowID);
	  if (HDevWindowStack::IsOpen())    disp_obj(ho_ImagePart, HDevWindowStack::GetActive());
	  if (HDevWindowStack::IsOpen())    set_color(HDevWindowStack::GetActive(),"blue");
  }
  hv_MaxLineWidth = MaxLineWidth; //20
  hv_Contrast = Contrast; //40
  calculate_lines_gauss_parameters(hv_MaxLineWidth, hv_Contrast.Concat(0), &hv_Sigma,  &hv_Low, &hv_High);

  lines_gauss(ho_ImagePart, &ho_Lines, hv_Sigma, hv_Low, hv_High, bwhite?"light":"dark", "true", "parabolic", "true");

  if(bShow && bProc==1)
	{
		if (HDevWindowStack::IsOpen())      set_color(HDevWindowStack::GetActive(),"red");
		if (HDevWindowStack::IsOpen())      disp_obj(ho_Lines, HDevWindowStack::GetActive()); // 센타선
	}

//  lines_gauss(ho_ImagePart, &ho_Lines, 2.3, 0.0, 0.7, "dark", "true", "parabolic", "true");

  CString str;
  
  select_contours_xld(ho_Lines, &ho_SelectedContours, "contour_length", MinLineLength, 100000, -0.5, 0.5);
  segment_contours_xld(ho_SelectedContours, &ho_ContoursSplit, "lines", 5, 10, 2);
  sort_contours_xld(ho_ContoursSplit,&ho_SortedContours,"upper_left", "true","row");
  count_obj(ho_SortedContours, &hv_Number);

  HTuple end_val13 = hv_Number;
  HTuple step_val13 = 1;

  
  if(id==0)	m_pFLinePtr.clear();
  else m_pSLinePtr.clear();

  if(reverse==true) m_pARLinePtr.clear();

  for (hv_I=1; hv_I.Continue(end_val13, step_val13); hv_I += step_val13)
  {
    select_obj(ho_SortedContours, &ho_Line, hv_I);
    get_contour_xld(ho_Line, &hv_Row, &hv_Col);

    get_contour_attrib_xld(ho_Line, "angle", &hv_Angle);
    get_contour_attrib_xld(ho_Line, "width_left", &hv_WidthL);
    get_contour_attrib_xld(ho_Line, "width_right", &hv_WidthR);

    hv_RowR = hv_Row+(((hv_Angle.Cos())*hv_WidthR)*(HTuple(0.75).Sqrt()));
    hv_ColR = hv_Col+(((hv_Angle.Sin())*hv_WidthR)*(HTuple(0.75).Sqrt()));
    hv_RowL = hv_Row-(((hv_Angle.Cos())*hv_WidthL)*(HTuple(0.75).Sqrt()));
    hv_ColL = hv_Col-(((hv_Angle.Sin())*hv_WidthL)*(HTuple(0.75).Sqrt()));

	tuple_max(hv_RowR.Concat(hv_RowL), &hv_Max);
    tuple_max(hv_ColR.Concat(hv_ColL), &hv_Max2);
    if (0 != (HTuple(hv_Max<hv_Height).And(hv_Max2<hv_Width)))
    {
		if(bShow)
		{
			if (HDevWindowStack::IsOpen())      set_color(HDevWindowStack::GetActive(),"red");
			if (HDevWindowStack::IsOpen())      disp_obj(ho_Line, HDevWindowStack::GetActive()); // 센타선
			if (HDevWindowStack::IsOpen())      set_color(HDevWindowStack::GetActive(),"green");
			disp_polygon(hv_WindowID, hv_RowL, hv_ColL); //아래쪽선
			disp_polygon(hv_WindowID, hv_RowR, hv_ColR); //윗쪽선
		}

		fit_line_contour_xld(ho_Line, "tukey", -1, 0, 5, 2, &hv_RowBegin, &hv_ColBegin,  &hv_RowEnd, &hv_ColEnd, &hv_Nr, &hv_Nc, &hv_Dist);
		if(reverse==true)
		{
			if(hv_I==hv_Number.Int())
			{
				HTuple end_val19 = (hv_RowL.Num()) - 1;
				HTuple step_val19 = 1;
				for (hv_J=1; hv_J.Continue(end_val19, step_val19); hv_J += step_val19)
				{
					PointF ptrPf;
					ptrPf.X =Gdiplus::REAL( (hv_ColL[hv_J].D()+InspROI.left));
					ptrPf.Y = Gdiplus::REAL( (hv_RowL[hv_J].D()+InspROI.top));

					m_pARLinePtr.push_back(ptrPf);			
				}
			}
		}
		else if(hv_I==1)
		{
			HTuple end_val18 = (hv_RowL.Num()) - 1;
			HTuple step_val18 = 1;
			for (hv_J=1; hv_J.Continue(end_val18, step_val18); hv_J += step_val18)
			{
				PointF ptrPf;
				ptrPf.X = Gdiplus::REAL( (hv_ColR[hv_J].D()+InspROI.left));
				ptrPf.Y = Gdiplus::REAL( (hv_RowR[hv_J].D()+InspROI.top));

				if(id==0)	m_pFLinePtr.push_back(ptrPf);
				else m_pSLinePtr.push_back(ptrPf);				
			}

			m_dNa[id]=-hv_Nc[0].D()/hv_Nr[0].D();
			m_dNb[id]=hv_Dist[0].D()/hv_Nr[0].D()+InspROI.top;
			/*str.Format("기울기 %f, y절편 %f",-hv_Nc[0].D()/hv_Nr[0].D(),hv_Dist[0].D()/hv_Nr[0].D());
			AfxMessageBox(str);*/
		}
	 }
  }
    
  return true;
}
bool CHalconProcessing::halcon_laser_lines_detect(BYTE *psrc, int w,int h,CRect InspROI,bool bShow)
{
  Hobject  ho_Angio,ho_ROI_0,ho_Lens,ho_ImagePart;
  Hobject  ho_EdgeAmplitude, ho_ImageMax,ho_Line;
  Hobject  ho_Lines, ho_UnionContours,ho_SortedContours;
  HTuple  hv_Width, hv_Height, hv_WindowID,hv_Row, hv_Col;
  HTuple  hv_J,hv_Number;
  if(bShow)
  {
	if (HDevWindowStack::IsOpen())    close_window(HDevWindowStack::Pop());
  }
  gen_image1(&ho_Angio,"byte",w,h,(Hlong)psrc);		//Image Load
  get_image_size(ho_Angio, &hv_Width, &hv_Height);


  if(InspROI.right>w) InspROI.right=w-1;
  if(InspROI.bottom>w) InspROI.bottom=h-1;
  if(InspROI.left<0) InspROI.left=0;
  if(InspROI.top<0) InspROI.top=0;

  gen_rectangle1(&ho_ROI_0,InspROI.top, InspROI.left, InspROI.bottom, InspROI.right);  
  reduce_domain(ho_Angio, ho_ROI_0, &ho_Lens);  
  crop_domain(ho_Lens, &ho_ImagePart);

  get_image_size(ho_ImagePart, &hv_Width, &hv_Height);

  if(bShow)
  {
	  set_window_attr("background_color","black");
	  open_window(0,0,hv_Width/1.0,hv_Height/1,0,"","",&hv_WindowID);
	  HDevWindowStack::Push(hv_WindowID);
	  if (HDevWindowStack::IsOpen())    disp_obj(ho_ImagePart, HDevWindowStack::GetActive());
	  if (HDevWindowStack::IsOpen())    set_color(HDevWindowStack::GetActive(),"blue");
  }
  
  gray_dilation_rect(ho_ImagePart, &ho_EdgeAmplitude, 7, 5);
  gray_erosion_rect(ho_EdgeAmplitude, &ho_ImageMax, 13, 3);
  lines_gauss(ho_ImageMax, &ho_Lines, 0.9, 1, 18, "light", "true", "gaussian", "true");
  union_adjacent_contours_xld(ho_Lines, &ho_UnionContours, 100, 1, "attr_keep");

  sort_contours_xld(ho_UnionContours,&ho_SortedContours,"upper_left", "true","column");
  count_obj(ho_SortedContours, &hv_Number);
  if(hv_Number[0].I()<=0) return false;

  select_obj(ho_SortedContours, &ho_Line, 1);
  get_contour_xld(ho_Line, &hv_Row, &hv_Col);
  HTuple end_val18 = (hv_Row.Num()) - 1;
	HTuple step_val18 = 1;
	m_pFLinePtr.clear();

	for (hv_J=1; hv_J.Continue(end_val18, step_val18); hv_J += step_val18)
	{
		PointF ptrPf;
		ptrPf.X = Gdiplus::REAL( (hv_Row[hv_J].D()+InspROI.left));
		ptrPf.Y = Gdiplus::REAL( (hv_Col[hv_J].D()+InspROI.top));

		m_pFLinePtr.push_back(ptrPf);			
	}
  if(bShow)
	{
		if (HDevWindowStack::IsOpen())      set_color(HDevWindowStack::GetActive(),"green");
		disp_polygon(hv_WindowID, hv_Row, hv_Col); //아래쪽선
	}
  return true;
}
int CHalconProcessing::halcon_muraDetection(BYTE *psrc, int w,int h,CRect InspROI,bool bwhite)
{
	int defect_count=0;

	Hobject  ho_ROI_0,ho_Lens,ho_ImagePart;

	Hobject  ho_Image, ho_R, ho_G, ho_B, ho_ImageFFT1;
	Hobject  ho_ImageSub, ho_ImageMedian, ho_Basins, ho_Defects;

	HTuple  hv_Path, hv_ExpDefaultCtrlDummyVar, hv_Width;
	HTuple  hv_Height, hv_WindowHandle, hv_f, hv_Energy, hv_Correlation;
	HTuple  hv_Homogeneity, hv_Contrast, hv_Indices, hv_NDefects;

	gen_image1(&ho_Image,"byte",w,h,(Hlong)psrc);		//Image Load

	if(InspROI.right>w) InspROI.right=w-1;
	if(InspROI.bottom>w) InspROI.bottom=h-1;
	if(InspROI.left<0) InspROI.left=0;
	if(InspROI.top<0) InspROI.top=0;

	gen_rectangle1(&ho_ROI_0,InspROI.top, InspROI.left, InspROI.bottom, InspROI.right);
  	reduce_domain(ho_Image, ho_ROI_0, &ho_Lens);
  	crop_domain(ho_Lens, &ho_ImagePart);
	get_image_pointer1(ho_ImagePart, &hv_ExpDefaultCtrlDummyVar, &hv_ExpDefaultCtrlDummyVar,  &hv_Width, &hv_Height);

	estimate_background_illumination(ho_ImagePart, &ho_ImageFFT1);
	
	if(bwhite)sub_image(ho_ImageFFT1,ho_ImagePart, &ho_ImageSub, 2, 100);
	else sub_image(ho_ImagePart, ho_ImageFFT1, &ho_ImageSub, 2, 100);

	median_image(ho_ImageSub, &ho_ImageMedian, "circle", 9, "mirrored");
	watersheds_threshold(ho_ImageMedian, &ho_Basins, 20);
	cooc_feature_image(ho_Basins, ho_ImageMedian, 6, 0, &hv_Energy, &hv_Correlation, &hv_Homogeneity, &hv_Contrast);
	tuple_find((hv_Energy-0.05).Sgn(), -1, &hv_Indices);
	select_obj(ho_Basins, &ho_Defects, hv_Indices+1);
	count_obj(ho_Defects, &hv_NDefects);

	defect_count=int(hv_NDefects.Num());

	if(defect_count > 0)
	{
		HTuple  hv_Row1, hv_Column1, hv_Row2, hv_Column2;
		HTuple  hv_Circularity, hv_Rectangularity,hv_Area, hv_Row, hv_Column;
		HTuple  hv_Min1, hv_Max1, hv_Range1, hv_Mean1, hv_Deviation1;

		area_center(ho_Defects, &hv_Area, &hv_Row, &hv_Column);
		smallest_rectangle1(ho_ImageMedian, &hv_Row1, &hv_Column1, &hv_Row2, &hv_Column2);

		circularity(ho_Defects, &hv_Circularity);
		rectangularity(ho_Defects, &hv_Rectangularity);
		min_max_gray(ho_Defects, ho_ImageMedian, 0, &hv_Min1, &hv_Max1, &hv_Range1);
		intensity(ho_Defects, ho_ImageMedian, &hv_Mean1, &hv_Deviation1);

		cooc_feature_image(ho_Defects, ho_ImageMedian, 6, 0, &hv_Energy, &hv_Correlation, &hv_Homogeneity, &hv_Contrast); 
	}
		

	return defect_count;
}
int CHalconProcessing::halcon_DefectDetection(BYTE *psrc, int w,int h,CRect InspROI,int nWhiteThreshold, int nBlackThreshold,bool bShow)
{
	int defect_count=0;

	Hobject  ho_ROI_0,ho_Lens,ho_ImagePart,RegionDynThresh2,RegionDynThresh1;
	Hobject ho_RegionConcat,ho_RegionUnion,ho_ImageGauss;
	Hobject  ho_Image, ho_R, ho_G, ho_B, ho_ImageFFT1;
	Hobject  ho_ImageSub, ho_ImageMedian, ho_Basins, ho_Defects,ho_ImageMean2;
	Hobject  RegionClosing1,RegionFillUp,RegionOpening,ConnectedRegions;

	HTuple  hv_Path, hv_ExpDefaultCtrlDummyVar, hv_Width,hv_WindowID;
	HTuple  hv_Height, hv_WindowHandle, hv_f, hv_Energy, hv_Correlation;
	HTuple  hv_Homogeneity, hv_Contrast, hv_Indices, hv_NDefects;

	gen_image1(&ho_Image,"byte",w,h,(Hlong)psrc);		//Image Load

	if(InspROI.right>w) InspROI.right=w-1;
	if(InspROI.bottom>w) InspROI.bottom=h-1;
	if(InspROI.left<0) InspROI.left=0;
	if(InspROI.top<0) InspROI.top=0;

	
	gen_rectangle1(&ho_ROI_0,InspROI.top, InspROI.left, InspROI.bottom, InspROI.right);
  	reduce_domain(ho_Image, ho_ROI_0, &ho_Lens);
  	crop_domain(ho_Lens, &ho_ImagePart);

	gauss_image(ho_ImagePart,&ho_ImageGauss,7);
	mean_image (ho_ImageGauss, &ho_ImageMedian, 23, 23) ;
	
	if(bShow)
	{
		if (HDevWindowStack::IsOpen())    close_window(HDevWindowStack::Pop());

		HTuple  hv_Height2,hv_Width2;
		 get_image_size(ho_ImagePart, &hv_Width2, &hv_Height2);

		set_window_attr("background_color","black");
		open_window(0,0,hv_Width2/1.0,hv_Height2/1,0,"","",&hv_WindowID);
		HDevWindowStack::Push(hv_WindowID);
		if (HDevWindowStack::IsOpen())    disp_obj(ho_ImagePart, HDevWindowStack::GetActive());
		if (HDevWindowStack::IsOpen())    set_color(HDevWindowStack::GetActive(),"blue");

	}

	if(nWhiteThreshold!=255)			dyn_threshold (ho_ImagePart, ho_ImageMedian, &RegionDynThresh1, nWhiteThreshold, "light") ;
	if(nBlackThreshold!=0)				dyn_threshold (ho_ImagePart, ho_ImageMedian, &RegionDynThresh2, nBlackThreshold, "dark") ;

	concat_obj(RegionDynThresh1, RegionDynThresh2, &ho_RegionConcat);
	union1(ho_RegionConcat, &ho_RegionUnion);
	closing_circle (ho_RegionUnion, &RegionClosing1, 7.5);
	fill_up (RegionClosing1, &RegionFillUp);
	opening_circle (RegionFillUp, &RegionOpening, 2.5) ;
	connection (RegionOpening, &ConnectedRegions);
	select_shape (ConnectedRegions, &ho_Defects, "area", "and", 20, 9999999) ;

	if(bShow)
	{
		//Hobject  ho_BinImage;
		//HTuple  hv_Height2,hv_Width2;
		// get_image_size(ho_ImagePart, &hv_Width2, &hv_Height2);
		//region_to_bin(ConnectedRegions, &ho_BinImage, 255, 0, hv_Width2, hv_Height2);
		//if (HDevWindowStack::IsOpen())    disp_obj(ho_BinImage, HDevWindowStack::GetActive());
	}
	
	count_obj(ho_Defects, &hv_NDefects);
	defect_count=int(hv_NDefects.Num());

	if(defect_count > 0)
	{
		HTuple  hv_Row1, hv_Column1, hv_Row2, hv_Column2;
		HTuple  hv_Circularity, hv_Rectangularity,hv_Area, hv_Row, hv_Column;
		HTuple  hv_Min1, hv_Max1, hv_Range1, hv_Mean1, hv_Deviation1;

		area_center(ho_Defects, &hv_Area, &hv_Row, &hv_Column);
		smallest_rectangle1(ho_ImageMedian, &hv_Row1, &hv_Column1, &hv_Row2, &hv_Column2);

		circularity(ho_Defects, &hv_Circularity);
		rectangularity(ho_Defects, &hv_Rectangularity);
		min_max_gray(ho_Defects, ho_ImageMedian, 0, &hv_Min1, &hv_Max1, &hv_Range1);
		intensity(ho_Defects, ho_ImageMedian, &hv_Mean1, &hv_Deviation1);

		cooc_feature_image(ho_Defects, ho_ImageMedian, 6, 0, &hv_Energy, &hv_Correlation, &hv_Homogeneity, &hv_Contrast); 

		if(bShow)
		{
			if (HDevWindowStack::IsOpen())  
		    {
				set_color(HDevWindowStack::GetActive(),"red");
			    disp_obj(ho_Defects, HDevWindowStack::GetActive()); // 센타선
			}
		}
	}

	return defect_count;
}
int CHalconProcessing::halcon_AkonDetection(BYTE *psrc, int w,int h,CRect InspROI,int nWhiteThreshold, int nBlackThreshold,std::vector<CRect> &rectInfo,bool bShow)
{
	int defect_count=0;
	
	HTuple  hv_WindowID;
	HTuple TempNumL;
	Hobject  ho_ROI_0,ho_Lens,ho_ImagePart;
	Hobject  ho_Image,ImgInvert,Circle_top,RegionTopHat,ImgSub_top,ImgInvert_top,ImgSub_topResult;

	gen_image1(&ho_Image,"byte",w,h,(Hlong)psrc);		//Image Load
	cv::Mat sgrayMat(h, w, CV_8UC1, psrc);

	int tboffset = 0;
	int lroffset = 0;
	if(InspROI.Width()>InspROI.Height() && InspROI.Height()>300) tboffset = 20;
	else if(InspROI.Width()<InspROI.Height()) lroffset = 20;

	InspROI.DeflateRect(lroffset,tboffset,lroffset,tboffset);

	if(InspROI.right>w) InspROI.right=w-1;
	if(InspROI.bottom>w) InspROI.bottom=h-1;
	if(InspROI.left<0) InspROI.left=0;
	if(InspROI.top<0) InspROI.top=0;
	
	gen_rectangle1(&ho_ROI_0,InspROI.top, InspROI.left, InspROI.bottom, InspROI.right);
  	reduce_domain(ho_Image, ho_ROI_0, &ho_Lens);
  	crop_domain(ho_Lens, &ho_ImagePart);

	if(bShow)
	{
		if (HDevWindowStack::IsOpen())    close_window(HDevWindowStack::Pop());

		HTuple  hv_Height2,hv_Width2;
		 get_image_size(ho_ImagePart, &hv_Width2, &hv_Height2);

		set_window_attr("background_color","black");
		open_window(0,0,(Hlong)-1.0,-1,0,"","",&hv_WindowID);
		HDevWindowStack::Push(hv_WindowID);
		if (HDevWindowStack::IsOpen())    disp_image(ho_ImagePart, HDevWindowStack::GetActive());
		if (HDevWindowStack::IsOpen())    set_color(HDevWindowStack::GetActive(),"blue");
	}

	Hobject ImageMeanL,RegionDynThreshL,RegionFillUp,RegionFillUpL,ConnectedRegionsL;
	Hobject ConnectedRegions,SelectedRegionsL1,SelectedRegionsL2,SelectedRegionsL3,SortedRegions;

	mean_image(ho_ImagePart, &ImageMeanL, 100, 100);
	dyn_threshold(ho_ImagePart, ImageMeanL, &RegionDynThreshL, 2, "light");
	connection(RegionDynThreshL, &ConnectedRegions);

	fill_up(ConnectedRegions, &RegionFillUp);
	erosion_rectangle1(RegionFillUp,&RegionFillUpL,7,3);
	connection(RegionFillUpL, &ConnectedRegionsL);
	select_shape(ConnectedRegionsL, &SelectedRegionsL1, "area", "and", 500, 135000000);
	dilation_rectangle1 (SelectedRegionsL1,&SelectedRegionsL3,2,10);
	select_shape(SelectedRegionsL3, &SelectedRegionsL2, "rectangularity", "and", 0.4, 1);
	count_obj(SelectedRegionsL2, &TempNumL);

	HTuple end_val6 = TempNumL;
	HTuple step_val6 = 1,hv_I;
	HTuple Row, Column,	Phi, Length1, Length2;
	HTuple Area, CenterRow, CenterColumn;
	HTuple Row1, Column1, Row2, Column2;
	Hobject  ObjectSelected;

	CRect inforoi,chkroi;
	bool bisCheck;
	int topAvg=0;
	bool bVertical = InspROI.Height()>InspROI.Width() ? true:false;

	if(bVertical)
		sort_region(SelectedRegionsL2, &SortedRegions, "upper_left", "true", "row");
	else 
		sort_region(SelectedRegionsL2, &SortedRegions, "upper_left", "true", "column");

	int lsum=0,rsum=0,l=-1,r=-1,t=-1,b=-1;
	int nMax = -9999,j=0;

	for (hv_I=1; hv_I.Continue(end_val6, step_val6); hv_I += step_val6)
	{
		try
		{
			select_obj(SortedRegions, &ObjectSelected, hv_I);
			smallest_rectangle2(ObjectSelected, &Row, &Column,	&Phi, &Length1, &Length2);
			area_center(ObjectSelected, &Area, &CenterRow, &CenterColumn);
			smallest_rectangle1(ObjectSelected, &Row1, &Column1, &Row2, &Column2);

			inforoi.top =  LONG(InspROI.top+Row1[0].D()-tboffset);
			inforoi.left = LONG(InspROI.left+Column1[0].D()-lroffset);
			inforoi.bottom = LONG(InspROI.top+Row2[0].D()+tboffset);
			inforoi.right = LONG(InspROI.left+Column2[0].D()+lroffset);

			if(bVertical)
			{
				if(hv_I==1) topAvg=inforoi.left;
				else
				{
					if(abs(topAvg-inforoi.left)<5)
						topAvg = (topAvg+inforoi.left)/2;
				}

				bisCheck = true;
				for(int i=1;i<rectInfo.size();i++)
				{
					chkroi = rectInfo.at(i);
					if((chkroi.top-5<=inforoi.top && chkroi.bottom+5>=inforoi.top) || 
						(chkroi.top-5<=inforoi.top && chkroi.top+5>=inforoi.top) ||
						(chkroi.bottom-5<=inforoi.bottom && chkroi.bottom+5>=inforoi.bottom)) // x축 중복 제거
					{
						bisCheck = false;
						break;
					}
				}

				if(bisCheck)
				{
					if(topAvg!=0 && abs(topAvg-inforoi.left)>5)
					{
						inforoi.left = topAvg;
						inforoi.right = inforoi.left + inforoi.Width();
					}

					if(InspROI.Width()*0.8 >inforoi.Width())   // 세로축 길이가 ROI의 80% 이하 짧은 경우 보상
						inforoi.right = inforoi.left+int(InspROI.Width()*0.9);					
				}
			}
			else
			{
				if(hv_I==1) topAvg=inforoi.top;
				else
				{
					if(abs(topAvg-inforoi.top)<10)
						topAvg = (topAvg+inforoi.top)/2;
				}

				bisCheck = true;
				for(int i=1;i<rectInfo.size();i++)
				{
					chkroi = rectInfo.at(i);
					if((chkroi.left-5<=inforoi.left && chkroi.right+5>=inforoi.left) || 
						(chkroi.left-5<=inforoi.left && chkroi.left+5>=inforoi.left) ||
						(chkroi.right-5<=inforoi.right && chkroi.right+5>=inforoi.right)) // x축 중복 제거
					{
						bisCheck = false;
						break;
					}
				}

				if(bisCheck)
				{
					if(topAvg!=0 && abs(topAvg-inforoi.top)>5)
					{
						inforoi.top = topAvg;
						inforoi.bottom = inforoi.top + inforoi.Height();
					}

					if(InspROI.Height()*0.8 >inforoi.Height())   // 세로축 길이가 ROI의 80% 이하 짧은 경우 보상
						inforoi.bottom = inforoi.top+int(InspROI.Height()*0.9);
				}
			}

			if(bisCheck)
			{				
				rectInfo.push_back(inforoi);
			}
		}
		catch (...)
		{
			continue;
		}
	}	

	if(bShow)
	{
		if (HDevWindowStack::IsOpen())    disp_obj(SelectedRegionsL2, HDevWindowStack::GetActive());
	}

	sgrayMat.release();

	return defect_count;
}

CPoint CHalconProcessing::halcon_AkonMarkDetection(BYTE *psrc, int w,int h,CRect InspROI,bool bShow)
{
	CPoint rPt=CPoint(0,0);
	
	HTuple  hv_WindowID,TempNumL;
	Hobject  ho_ROI_0,ho_Lens,ho_ImagePart,ho_Image;

	gen_image1(&ho_Image,"byte",w,h,(Hlong)psrc);		//Image Load

	if(InspROI.right>w) InspROI.right=w-1;
	if(InspROI.bottom>w) InspROI.bottom=h-1;
	if(InspROI.left<0) InspROI.left=0;
	if(InspROI.top<0) InspROI.top=0;
	
	gen_rectangle1(&ho_ROI_0,InspROI.top, InspROI.left, InspROI.bottom, InspROI.right);
  	reduce_domain(ho_Image, ho_ROI_0, &ho_Lens);
  	crop_domain(ho_Lens, &ho_ImagePart);

	if(bShow)
	{
		if (HDevWindowStack::IsOpen())    close_window(HDevWindowStack::Pop());

		HTuple  hv_Height2,hv_Width2;
		 get_image_size(ho_ImagePart, &hv_Width2, &hv_Height2);

		set_window_attr("background_color","black");
		open_window(0,0,(Hlong)-1.0,-1,0,"","",&hv_WindowID);
		HDevWindowStack::Push(hv_WindowID);
		if (HDevWindowStack::IsOpen())    disp_image(ho_ImagePart, HDevWindowStack::GetActive());
		if (HDevWindowStack::IsOpen())    set_color(HDevWindowStack::GetActive(),"blue");
	}

	Hobject ImageMeanL,RegionDynThreshL,RegionFillUp,RegionFillUpL,ConnectedRegionsL;
	HTuple Area, CenterRow, CenterColumn;
	Hobject  ObjectSelected,SelectedRegionsL1,SelectedRegions;
	HTuple Row1, Column1, Row2, Column2;

	mean_image(ho_ImagePart, &ImageMeanL, 200, 200);  //배경 Search
	dyn_threshold(ho_ImagePart, ImageMeanL, &RegionDynThreshL, 5, "light"); // 이진화
	fill_up(RegionDynThreshL, &RegionFillUp);  // 영역 채움
	erosion_rectangle1(RegionFillUp,&RegionFillUpL,7,7);
	connection(RegionFillUpL, &ConnectedRegionsL);  // 연결
	select_shape(ConnectedRegionsL, &SelectedRegionsL1, "area", "and", 10000, 1350000); // 일정 크기만 선택
	count_obj(SelectedRegionsL1, &TempNumL);

	select_shape_std (SelectedRegionsL1, &SelectedRegions, "max_area", 70) ;  //Similarity measure

	if(TempNumL>=1)
	{
		select_obj(SelectedRegions, &ObjectSelected, 1);
		area_center(ObjectSelected, &Area, &CenterRow, &CenterColumn);
		smallest_rectangle1(ObjectSelected, &Row1, &Column1, &Row2, &Column2);

		//rPt.x=InspROI.left+CenterColumn[0].D(); // 무게 중심
		//rPt.y=InspROI.top+CenterRow[0].D();     // 무게 중심

		rPt.x=LONG(InspROI.left+(Column1[0].D()+Column2[0].D())/2);  // 영역 중심
		rPt.y=LONG(InspROI.top+(Row1[0].D()+Row2[0].D())/2);  // 영역 중심
	}

	if(bShow)
	{
		if (HDevWindowStack::IsOpen())    disp_obj(SelectedRegionsL1, HDevWindowStack::GetActive());
	}

	return rPt;
}

int CHalconProcessing::halcon_QRcodeDetection(BYTE *psrc, int w,int h,CRect InspROI)
{
	int rVal=0;

	HTuple  DataCodeHandle, ResultHandles;
	Hobject  ho_ROI_0,ho_Lens,ho_ImagePart,ho_Image, ho_ImageEmphasize,SymbolXLDs,SymbolXLD,Region;
	HTuple  DecodedDataStrings,i, Row, Col;
	HTuple Row1, Column1, Row2, Column2;

	gen_image1(&ho_Image,"byte",w,h,(Hlong)psrc);		//Image Load

	if(InspROI.right>w) InspROI.right=w-1;
	if(InspROI.bottom>w) InspROI.bottom=h-1;
	if(InspROI.left<0) InspROI.left=0;
	if(InspROI.top<0) InspROI.top=0;
	
	gen_rectangle1(&ho_ROI_0,InspROI.top, InspROI.left, InspROI.bottom, InspROI.right);
  	reduce_domain(ho_Image, ho_ROI_0, &ho_Lens);
  	crop_domain(ho_Lens, &ho_ImagePart);

	create_data_code_2d_model("QR Code", "default_parameters", "maximum_recognition",  &DataCodeHandle);
	emphasize(ho_ImagePart, &ho_ImageEmphasize, 7, 7, 1);
	find_data_code_2d(ho_ImageEmphasize, &SymbolXLDs, DataCodeHandle, "stop_after_result_num",  9, &ResultHandles, &DecodedDataStrings);

	rVal =int( ResultHandles.Num()-1);
	CRect r;
	m_pQRPosPtr.clear();

	if(rVal >= 0)
	{
		for (i=0; i<=(ResultHandles.Num())-1; i+=1)
		{
			select_obj(SymbolXLDs, &SymbolXLD, i+1);
			gen_region_contour_xld (SymbolXLD, &Region, "filled");
			smallest_rectangle1(Region, &Row1, &Column1, &Row2, &Column2);
			r = CRect(Column1[0].I(),Row1[0].I(),Column2[0].I(),Row2[0].I());
			m_pQRPosPtr.push_back(r);
		}
	}

	return rVal;
}

//bool  CHalconProcessing::CalcCalibrationMat(int nCam, cv::Point2d[] visionPt, cv::Point2d[] robotPt)
//{
//	if (visionPt.Length != robotPt.Length)           throw new Exception("imgPt.Length != robotPt.Length");
//	if (robotPt.Length < 3) return false;
//
//	Point2f[] v2f = new Point2f[visionPt.Length];
//	Point2f[] r2f = new Point2f[visionPt.Length];
//
//	for (int i = 0; i < visionPt.Length; i++)
//	{
//		v2f[i] = new Point2f((float)visionPt[i].X, (float)visionPt[i].Y);
//		r2f[i] = new Point2f((float)robotPt[i].X, (float)robotPt[i].Y);
//	}
//	Mat affine = Cv2.GetAffineTransform(v2f, r2f);
//
//	Mat affine9 = new Mat(3, 3, MatType.CV_64FC1, new double[] {
//		affine.At<double>(0, 0), affine.At<double>(0, 1), affine.At<double>(0, 2),
//			affine.At<double>(1, 0), affine.At<double>(1, 1), affine.At<double>(1, 2),
//			0, 0, 1 });
//
//	double detAffine = affine9.Determinant();
//	if (Math.Abs(detAffine) > 0.0000000001)
//	{
//		m_bValidMatCalibXY[nCam] = true;
//		m_matVtoR[nCam] = affine9;
//		m_matRtoV[nCam] = m_matVtoR[nCam].Inv();
//
//		CalcCalibrationResult(nCam);
//		return true;
//	}
//
//	return false;
//}
//
//bool  CHalconProcessing::CalCalibrationMat1D(int nCam, Point2d[] visionPt, Point2d[] robotPt)
//{
//	if (nCam < 0 || nCam >= CAM_MAX)       throw new Exception("Camera No is out of range!");
//	if (m_listCalibXY_ptVision.Count != m_listCalibXY_ptMotor.Count)    throw new Exception("No. of vision points and robot points are not same!");
//
//	m_bValidMatCalibXY[nCam] = false;
//
//	Mat A = new Mat(robotPt.Length * 2, 5, MatType.CV_64FC1);
//	Mat B = new Mat(robotPt.Length * 2, 1, MatType.CV_64FC1);
//	Point2d K = new Point2d(robotPt[robotPt.Length - 1].X - robotPt[0].X, robotPt[robotPt.Length - 1].Y - robotPt[0].Y);
//	K *= (1 / K.DistanceTo(new Point2d()));
//
//	A.SetTo(0); B.SetTo(0);
//
//	for (int i = 0; i < robotPt.Length; i++)
//	{
//		A.Set<double>(2 * i, 0, robotPt[i].X);
//		A.Set<double>(2 * i, 1, robotPt[i].Y);
//		A.Set<double>(2 * i, 2, 1);
//		A.Set<double>(2 * i, 4, -visionPt[i].X * (K.X * robotPt[i].X + K.Y * robotPt[i].Y));
//		B.Set<double>(2 * i, visionPt[i].X);
//
//		A.Set<double>(2 * i + 1, 0, -robotPt[i].Y);
//		A.Set<double>(2 * i + 1, 1, robotPt[i].X);
//		A.Set<double>(2 * i + 1, 3, 1);
//		A.Set<double>(2 * i + 1, 4, -visionPt[i].Y * (K.X * robotPt[i].X + K.Y * robotPt[i].Y));
//		B.Set<double>(2 * i + 1, visionPt[i].Y);
//	}
//
//	try
//	{
//		Mat X = (A.Transpose() * A).Inv() * A.Transpose() * B;
//
//		double[] Aelements = new double[6];
//		X.GetArray(0, 0, Aelements);
//
//		Mat invMat = new Mat(3, 3, MatType.CV_64FC1, new double[] {
//			Aelements[0], Aelements[1], Aelements[2],
//				Aelements[1], -Aelements[0], Aelements[3],
//				K.X* Aelements[4], K.Y* Aelements[4], 1 });
//
//		double det = invMat.Determinant();
//		if (Math.Abs(det) < 0.0000000001) return false;
//		Mat calMat = invMat.Inv();
//		calMat *= (1 / calMat.At<double>(2, 2));
//		invMat.Dispose();
//
//		m_bValidMatCalibXY[nCam] = true;
//		m_matVtoR[nCam] = calMat;
//		m_matRtoV[nCam] = m_matVtoR[nCam].Inv();
//
//		CalcCalibrationResult(nCam);
//
//		NotifyIconObserver(CalibrationAssistants.UPDATE_XLD);
//	}
//	catch
//	{
//		return false;
//	}
//	finally
//	{
//		A.Dispose();
//		B.Dispose();
//	}
//
//	return true;
//}
//
//// 영상의 pixel 위치로부터 robot 좌표를 얻는다.
//bool  CHalconProcessing::GetMotorPos(int nCam, double xV, double yV, out double xR, out double yR)
//{
//	xR = yR = 0;
//	try
//	{
//		Mat ptVision = new Mat(3, 1, MatType.CV_64FC1, new double[] { xV, yV, 1.0 });
//		//Mat ptRobot = m_matVtoR.Cross(ptVision);
//		Mat ptRobot = m_matVtoR[nCam] * ptVision;
//		xR = ptRobot.At<double>(0, 0) / ptRobot.At<double>(2, 0);
//		yR = ptRobot.At<double>(1, 0) / ptRobot.At<double>(2, 0);
//	}
//	catch(...)
//	{
//		return false;
//	}
//	return true;
//}
//
//// robot 좌표에서 영상의 pixel 좌표를 얻는다
//bool  CHalconProcessing::GetVisionPos(double xR, double yR, double &xV, double &yV)
//{
//	xV = yV = 0;
//	try
//	{
//		cv::Mat ptRobot = cv::Mat::zeros(3, 1, CV_64FC1);
//		ptRobot.at<double>(0, 0) = xR;
//		ptRobot.at<double>(1, 0) = yR;
//		ptRobot.at<double>(2, 0) = 1.0;
//
//		cv::Mat ptVision = m_matRtoV * ptRobot;
//		xV = ptVision.at<double>(0, 0) / ptVision.at<double>(2, 0);
//		yV = ptVision.at<double>(1, 0) / ptVision.at<double>(2, 0);
//	}
//	catch(...)
//	{
//		return false;
//	}
//	return true;
//}