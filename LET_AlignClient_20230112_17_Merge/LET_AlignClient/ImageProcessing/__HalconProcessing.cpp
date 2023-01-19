/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "../LET_AlignClient.h"
#include "HalconProcessing.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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

	m_nFatOffsetWidth=0;
	m_nFatOffsetHeight=0;


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

//	create_shape_model(ho_TemplateImage, 2, HTuple(-15).Rad(), HTuple(15).Rad(), HTuple(4.0297).Rad(), 
	//create_shape_model(ho_TemplateImage, 2, HTuple(0).Rad(), HTuple(360).Rad(), HTuple(4.0297).Rad(), 

	if( nMinContrast == -1 || nHighContrast == -1 )
	{
		create_shape_model(ho_TemplateImage, 5, HTuple(m_nSearchAngleRange*-1).Rad(), HTuple(m_nSearchAngleRange).Rad(), HTuple(m_nSearchAngleStep).Rad(),
			(HTuple("point_reduction_high").Append("pregeneration")), "use_polarity", "auto", "auto", &hv_ModelId);
	}
	else
	{
		create_shape_model(ho_TemplateImage, 5, HTuple(m_nSearchAngleRange*-1).Rad(), HTuple(m_nSearchAngleRange).Rad(), HTuple(m_nSearchAngleStep).Rad(),
			(HTuple("point_reduction_high").Append("no_pregeneration")), "use_polarity", (HTuple(nMinContrast).Append(nHighContrast).Append(4)), 4, &hv_ModelId);
	}

	/*create_shape_model(ho_TemplateImage, 2, HTuple(0).Rad(), HTuple(360).Rad(), HTuple(4.0297).Rad(),
		(HTuple("none").Append("pregeneration")), "use_polarity", ((HTuple(25).Append(35)).Append(9)), "auto", &hv_ModelId);*/

	get_shape_model_contours(&ho_ModelContours, hv_ModelId, 1);
#endif

	m_bModelRead = true;
	m_bNccSelect = false;

	return bStatus;
}


BOOL CHalconProcessing::halcon_ReadModel(BYTE *psrc, int w,int h, int nMinContrast, int nHighContrast)
{
	BOOL bStatus=FALSE;

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

	/*create_shape_model(ho_TemplateImage, 2, HTuple(-0.5).Rad(), HTuple(0.5).Rad(), HTuple(0.1).Rad(), 
		(HTuple("none").Append("pregeneration")), "use_polarity", "auto", "auto", &hv_ModelId);*/

//	create_shape_model(ho_TemplateImage, 2, HTuple(-15).Rad(), HTuple(15).Rad(), HTuple(4.0297).Rad(), 
	//create_shape_model(ho_TemplateImage, 2, HTuple(0).Rad(), HTuple(360).Rad(), HTuple(4.0297).Rad(), 

	if( nMinContrast == -1 || nHighContrast == -1 )
	{
		create_shape_model(ho_TemplateImage, 5, HTuple(m_nSearchAngleRange*-1).Rad(), HTuple(m_nSearchAngleRange).Rad(), HTuple(m_nSearchAngleStep).Rad(),
			(HTuple("point_reduction_high").Append("pregeneration")), "use_polarity", "auto", "auto", &hv_ModelId);
	}
	else
	{
		create_shape_model(ho_TemplateImage, 5, HTuple(m_nSearchAngleRange*-1).Rad(), HTuple(m_nSearchAngleRange).Rad(), HTuple(m_nSearchAngleStep).Rad(),
			(HTuple("point_reduction_high").Append("no_pregeneration")), "use_polarity", (HTuple(nMinContrast).Append(nHighContrast).Append(4)), 4, &hv_ModelId);
	}
/*
	
	create_shape_model(ho_TemplateImage, 5, HTuple(-15).Rad(), HTuple(15).Rad(), HTuple(0.7297).Rad(), 
		(HTuple("point_reduction_high").Append("pregeneration")), "use_polarity", "auto", "auto", &hv_ModelId);
*/
	
	get_shape_model_contours(&ho_ModelContours, hv_ModelId, 1);
//	HContourToIplImage(ho_ModelContours,ho_TemplateImage);

#endif	

	m_bModelRead = true;
	m_bNccSelect = false;

	return bStatus;
}


BOOL CHalconProcessing::halcon_ReadNccModel(BYTE *ptemplateSrc,int w,int h)
{
	BOOL bStatus=FALSE;	


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
		create_ncc_model(ho_TemplateImage, "auto", HTuple(m_nSearchAngleRange*-1).Rad(), HTuple(m_nSearchAngleRange).Rad(), "auto", "use_polarity", &hv_ModelId);
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

	create_ncc_model(ho_TemplateImage, "auto", HTuple(m_nSearchAngleRange*-1).Rad(), HTuple(m_nSearchAngleRange).Rad(), "auto", "use_polarity", &hv_ModelId);

	//create_shape_model(ho_TemplateImage, 2, HTuple(-15).Rad(), HTuple(15).Rad(), HTuple(4.0297).Rad(), 
	//	(HTuple("none").Append("pregeneration")), "use_polarity", ((HTuple(25).Append(35)).Append(9)), "auto", &hv_ModelId);
	///*create_shape_model(ho_TemplateImage, 2, HTuple(0).Rad(), HTuple(360).Rad(), HTuple(4.0297).Rad(), 
	//	(HTuple("none").Append("pregeneration")), "use_polarity", ((HTuple(25).Append(35)).Append(9)), "auto", &hv_ModelId);*/
	//get_shape_model_contours(&ho_ModelContours, hv_ModelId, 1);

#endif
	m_bNccSelect = TRUE;
	m_bModelRead = true;

	return bStatus;
}

BOOL CHalconProcessing::halcon_ReadMaskModel(CString modelname,BOOL bTemplateShape,BOOL m_bMaskRoiUse[4],BOOL m_bMaskRoiShape[4],CRect m_crMaskRect[4], int nMinContrast, int nHighContrast)
{
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

	reduce_domain(ho_Image, ho_ModelRegion, &ho_TemplateImage);
	

	if( nMinContrast == -1 || nHighContrast == -1 )
	{
		create_shape_model(ho_TemplateImage, 5, HTuple(m_nSearchAngleRange*-1).Rad(), HTuple(m_nSearchAngleRange).Rad(), HTuple(m_nSearchAngleStep).Rad(),
				(HTuple("point_reduction_high").Append("pregeneration")), "use_polarity", "auto", "auto", &hv_ModelId);
	}
	else
	{
		create_shape_model(ho_TemplateImage, 5,HTuple(m_nSearchAngleRange*-1).Rad(), HTuple(m_nSearchAngleRange).Rad(), HTuple(m_nSearchAngleStep).Rad(),
			(HTuple("point_reduction_high").Append("no_pregeneration")), "use_polarity", (HTuple(nMinContrast).Append(nHighContrast).Append(4)), 4, &hv_ModelId);
	}

	get_shape_model_contours(&ho_ModelContours, hv_ModelId, 1);

	//m_nFatOffsetWidth=w-Col[0].D();
	//m_nFatOffsetHeight=h-Row[0].D();

#endif

	m_bNccSelect = FALSE;
	m_bModelRead = true;

	return bStatus;
}


BOOL CHalconProcessing::halcon_ReadMaskModel(BYTE *ptemplateSrc,int w,int h,BOOL bTemplateShape,BOOL m_bMaskRoiUse[4],BOOL m_bMaskRoiShape[4],CRect m_crMaskRect[4],int nMinContrast, int nHighContrast)
{
	BOOL bStatus=TRUE;

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

	reduce_domain(ho_Image, ho_ModelRegion, &ho_TemplateImage);

//	write_image(ho_TemplateImage,"bmp",0,"d:\\sdf.bmp");
	if (m_lWindowID != -1)
	{
		if (HDevWindowStack::IsOpen())     disp_obj(ho_TemplateImage, HDevWindowStack::GetActive());
	}

	//create_shape_model(ho_TemplateImage,5, HTuple(-15).Rad(), HTuple(15).Rad(), HTuple(0.7297).Rad(),
	//	(HTuple("point_reduction_high").Append("pregeneration")), "use_polarity", "auto", "auto", &hv_ModelId);

	if( nMinContrast == -1 || nHighContrast == -1 )
	{
		create_shape_model(ho_TemplateImage, 5, HTuple(m_nSearchAngleRange*-1).Rad(), HTuple(m_nSearchAngleRange).Rad(), HTuple(m_nSearchAngleStep).Rad(),
				(HTuple("point_reduction_high").Append("pregeneration")), "use_polarity", "auto", "auto", &hv_ModelId);
	}
	else
	{
		create_shape_model(ho_TemplateImage, 5, HTuple(m_nSearchAngleRange*-1).Rad(), HTuple(m_nSearchAngleRange).Rad(), HTuple(m_nSearchAngleStep).Rad(),
			(HTuple("point_reduction_high").Append("no_pregeneration")), "use_polarity", (HTuple(nMinContrast).Append(nHighContrast).Append(4)), 4, &hv_ModelId);
	}


	get_shape_model_contours(&ho_ModelContours, hv_ModelId, 1);


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

	// Local iconic variables

	score = score-0.2;

	if( score < 0.3 )
		score = 0.3;
	
#ifdef _CPP_USE_HALCON
	HObject  ho_Image;
	HObject  ho_TransContours;
	HObject  ho_ROI_0,ho_Lens,ho_ImagePart;
#else
	Hobject  ho_Image;
	Hobject  ho_TransContours;
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
				m_ptMarkPos.y = hv_ModelRow[hv_MatchingObjIdx].D() +m_ptMarkOffset.x;
				m_ptMarkPos.x = hv_ModelColumn[hv_MatchingObjIdx].D()+m_ptMarkOffset.y;

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
	//	if( hv_ModelId == 0 )	return FALSE;

		//find_shape_model(ho_ImagePart, hv_ModelId, HTuple(-5).Rad(), HTuple(10).Rad(), 0.3, 1, 0.5, 
					//charMethod, (HTuple(3).Append(1)), 0.75, &hv_ModelRow, &hv_ModelColumn, &hv_ModelAngle, &hv_ModelScore);
		/*find_shape_model(ho_ImagePart, hv_ModelId, HTuple(-15).Rad(), HTuple(15).Rad(), score, 1, 0.5, 
			charMethod, (HTuple(2).Append(1)), 0.75, &hv_ModelRow, &hv_ModelColumn, &hv_ModelAngle, &hv_ModelScore);*/

		find_shape_model(ho_ImagePart, hv_ModelId, HTuple(m_nSearchAngleRange*-1).Rad(), HTuple(m_nSearchAngleRange).Rad(), score, 4, 0.5, 
			charMethod,5, 0.75, &hv_ModelRow, &hv_ModelColumn, &hv_ModelAngle, &hv_ModelScore);
		
		if( hv_ModelScore.Num() == 0)
			return FALSE;
		
		tuple_max(hv_ModelScore,&ho_max);
		HTuple end_val19 = (hv_ModelScore.Num()) - 1;
		HTuple step_val19 = 1;

		for (hv_MatchingObjIdx=0; hv_MatchingObjIdx.Continue(end_val19, step_val19); hv_MatchingObjIdx += step_val19)
		{
			/*hom_mat2d_identity(&hv_HomMat);
			hom_mat2d_rotate(hv_HomMat, HTuple(hv_ModelAngle[hv_MatchingObjIdx]), 0, 0, &hv_HomMat);
			hom_mat2d_translate(hv_HomMat, HTuple(hv_ModelRow[hv_MatchingObjIdx]), HTuple(hv_ModelColumn[hv_MatchingObjIdx]),	&hv_HomMat);
			affine_trans_contour_xld(ho_ModelContours, &ho_TransContours, hv_HomMat);*/

			if(hv_ModelScore[hv_MatchingObjIdx] == ho_max)
			{
				/*m_ContourImageResult.Reset();
				vector_angle_to_rigid(0, 0, 0, hv_ModelRow, hv_ModelColumn, hv_ModelAngle, &hv_HomMat);
				affine_trans_contour_xld(ho_ModelContours, &ho_TransContours, hv_HomMat);
				HContourToIplImage(ho_TransContours,ho_ImagePart);*/

				m_ptMarkPos.y = hv_ModelRow[hv_MatchingObjIdx].D() - m_nFatOffsetWidth;
				m_ptMarkPos.x = hv_ModelColumn[hv_MatchingObjIdx].D() - m_nFatOffsetHeight;
								
				m_dModelAngleRadian =hv_ModelAngle[hv_MatchingObjIdx].D();
				m_dModelScore = hv_ModelScore[hv_MatchingObjIdx].D();
				m_dModelAngle = m_dModelAngleRadian* (180. / PI);

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
				m_ptMarkPos.y = hv_ModelRow[hv_MatchingObjIdx].D() +m_ptMarkOffset.x;
				m_ptMarkPos.x = hv_ModelColumn[hv_MatchingObjIdx].D()+m_ptMarkOffset.y;

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

		//find_ncc_model(ho_Image, hv_ModelId, HTuple(0).Rad(), HTuple(360).Rad(),  score, 5, 0.5, "true", 0, &hv_ModelRow, &hv_ModelColumn, &hv_ModelAngle, &hv_ModelScore);
				
		find_ncc_model(ho_ImagePart, hv_ModelId, HTuple(m_nSearchAngleRange*-1).Rad(), HTuple(m_nSearchAngleRange).Rad(),  score, 1, 0.5, "true", 0, &hv_ModelRow, &hv_ModelColumn, &hv_ModelAngle, &hv_ModelScore);

		if(hv_ModelScore.Num()==0) return FALSE;
		tuple_max(hv_ModelScore,&ho_max);
		HTuple end_val19 = (hv_ModelScore.Num())-1;
		HTuple step_val19 = 1;
		for (hv_MatchingObjIdx=0; hv_MatchingObjIdx.Continue(end_val19, step_val19); hv_MatchingObjIdx += step_val19)
		{
			if(hv_ModelScore[hv_MatchingObjIdx]==ho_max)
			{
				m_ptMarkPos.y = hv_ModelRow[hv_MatchingObjIdx].D();
				m_ptMarkPos.x = hv_ModelColumn[hv_MatchingObjIdx].D();

				m_dModelAngleRadian =hv_ModelAngle[hv_MatchingObjIdx].D();
				m_dModelScore = hv_ModelScore[hv_MatchingObjIdx].D();
				m_dModelAngle = m_dModelAngleRadian* (180. / PI);

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
	double ma= 1000000000000,mb= 1000000000000;
	double centerX=0,centerY=0;

	if ((middlePoint.X - startPoint.X)!=0) 	ma = (middlePoint.Y - startPoint.Y) / (middlePoint.X - startPoint.X); 
	if ((endPoint.X - middlePoint.X)!=0)   mb = (endPoint.Y - middlePoint.Y) / (endPoint.X - middlePoint.X); 
	
	if (ma == 0.f) 		ma = 0.000000000001; 
	if (mb == 0.f) 		mb = 0.000000000001; 

	if(mb-ma!=0)
		centerX = (ma * mb * (startPoint.Y - endPoint.Y) + mb * (startPoint.X + middlePoint.X) -	ma * (middlePoint.X + endPoint.X)) / (2 * (mb - ma)); 
	 centerY = (-1 * (centerX - (startPoint.X + middlePoint.X) / 2) / ma) + ((startPoint.Y + middlePoint.Y) / 2); 

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