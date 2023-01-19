/************************************************************************
  			BlobResult.cpp
  			
FUNCIONALITAT: Implementaciï¿?de la classe CBlobResult
AUTOR: Inspecta S.L.
MODIFICACIONS (Modificaciï¿? Autor, Data):
 
**************************************************************************/
#include "StdAfx.h"
#include "BlobResult.h"
//! Show errors functions: only works for windows releases
#ifdef _SHOW_ERRORS
	#include <afx.h>			//suport per a CStrings
	#include <afxwin.h>			//suport per a AfxMessageBox
#endif

using namespace std;
using namespace cv;

/**************************************************************************
		Constructors / Destructors
**************************************************************************/


/**
- FUNCIï¿? CBlobResult
- FUNCIONALITAT: Constructor estandard.
- PARï¿½METRES:
- RESULTAT:
- Crea un CBlobResult sense cap blob
- RESTRICCIONS:
- AUTOR: Ricard Borrï¿½s
- DATA DE CREACIï¿? 20-07-2004.
- MODIFICACIï¿? Data. Autor. Descripciï¿?
*/
/**
- FUNCTION: CBlobResult
- FUNCTIONALITY: Standard constructor
- PARAMETERS:
- RESULT:
	- creates an empty set of blobs
- RESTRICTIONS:
- AUTHOR: Ricard Borrï¿½s
- CREATION DATE: 25-05-2005.
- MODIFICATION: Date. Author. Description.
*/
CBlobResult::CBlobResult()
{
	m_blobs = Blob_vector();
}

/**
- FUNCIï¿? CBlobResult
- FUNCIONALITAT: Constructor a partir d'una imatge. Inicialitza la seqï¿½ï¿½ncia de blobs 
			   amb els blobs resultants de l'anï¿½lisi de blobs de la imatge.
- PARï¿½METRES:
	- source: imatge d'on s'extreuran els blobs
	- mask: mï¿½scara a aplicar. Nomï¿½s es calcularan els blobs on la mï¿½scara sigui 
			diferent de 0.
- RESULTAT:
	- objecte CBlobResult amb els blobs de la imatge source
- RESTRICCIONS:
- AUTOR: Ricard Borrï¿½s
- DATA DE CREACIï¿? 25-05-2005.
- MODIFICACIï¿? Data. Autor. Descripciï¿?
*/
/**
- FUNCTION: CBlob
- FUNCTIONALITY: Constructor from an image. Fills an object with all the blobs in
	the image
- PARAMETERS:
	- source: image to extract the blobs from
	- mask: optional mask to apply. The blobs will be extracted where the mask is
			not 0.
	- numThreads: number of labelling threads.		
- RESULT:
	- object with all the blobs in the image.
- RESTRICTIONS:
- AUTHOR: Ricard Borrï¿½s
- CREATION DATE: 25-05-2005.
- MODIFICATION:
	Oct-2013. Luca Nardelli and Saverio Murgia. Changed to comply with reimplemented labelling algorithm
*/
CBlobResult::CBlobResult(IplImage *source, IplImage *mask,int numThreads)
{
	if(mask!=NULL){
		Mat temp = Mat::zeros(cv::Size(source->width,source->height),CV_8UC1);
		cvarrToMat(source).copyTo(temp,cvarrToMat(mask));
		compLabeler.set(numThreads,temp);
		compLabeler.doLabeling(m_blobs);
	}
	else{
		Mat temp = Mat::zeros(cv::Size(source->width,source->height),CV_8UC1);
		compLabeler.set(numThreads,temp);
		compLabeler.doLabeling(m_blobs);
	}
}
/**
- FUNCTION: CBlobResult
- FUNCTIONALITY: Constructor from an image. Fills an object with all the blobs in
	the image, OPENCV 2 interface
- PARAMETERS:
	- source: Mat to extract the blobs from, CV_8UC1
	- mask: optional mask to apply. The blobs will be extracted where the mask is
			not 0. All the neighbouring blobs where the mask is 0 will be extern blobs
	- numThreads: number of labelling threads. 
- RESULT:
	- object with all the blobs in the image.
- RESTRICTIONS:
- AUTHOR: Saverio Murgia & Luca Nardelli
- CREATION DATE: 06-04-2013.
- MODIFICATION: Date. Author. Description.
*/
CBlobResult::CBlobResult(Mat &source, const Mat &mask,int numThreads){
	if(mask.data){
		Mat temp=Mat::zeros(source.size(),source.type());
		source.copyTo(temp,mask);
		compLabeler.set(numThreads,temp);
		compLabeler.doLabeling(m_blobs);
	}
	else{
		compLabeler.set(numThreads,source);
		compLabeler.doLabeling(m_blobs);
	}
}

/**
- FUNCIï¿? CBlobResult
- FUNCIONALITAT: Constructor de cï¿½pia. Inicialitza la seqï¿½ï¿½ncia de blobs 
			   amb els blobs del parï¿½metre.
- PARï¿½METRES:
	- source: objecte que es copiarï¿?- RESULTAT:
	- objecte CBlobResult amb els blobs de l'objecte source
- RESTRICCIONS:
- AUTOR: Ricard Borrï¿½s
- DATA DE CREACIï¿? 25-05-2005.
- MODIFICACIï¿? Data. Autor. Descripciï¿?
*/
/**
- FUNCTION: CBlobResult
- FUNCTIONALITY: Copy constructor
- PARAMETERS:
	- source: object to copy
- RESULT:
- RESTRICTIONS:
- AUTHOR: Ricard Borrï¿½s
- CREATION DATE: 25-05-2005.
- MODIFICATION: Date. Author. Description.
*/
CBlobResult::CBlobResult( const CBlobResult &source )
{	
	// creem el nou a partir del passat com a parï¿½metre
	//m_blobs = Blob_vector( source.GetNumBlobs() );
	m_blobs.reserve(source.GetNumBlobs());
	// copiem els blobs de l'origen a l'actual
	Blob_vector::const_iterator pBlobsSrc = source.m_blobs.begin();
	Blob_vector::iterator pBlobsDst = m_blobs.begin();
	while( pBlobsSrc != source.m_blobs.end() )
	{
		// no podem cridar a l'operador = ja que Blob_vector ï¿½s un 
		// vector de CBlob*. Per tant, creem un blob nou a partir del
		// blob original
		m_blobs.push_back(new CBlob(**pBlobsSrc));
		pBlobsSrc++;
	}
}



/**
- FUNCIï¿? ~CBlobResult
- FUNCIONALITAT: Destructor estandard.
- PARï¿½METRES:
- RESULTAT:
	- Allibera la memï¿½ria reservada de cadascun dels blobs de la classe
- RESTRICCIONS:
- AUTOR: Ricard Borrï¿½s
- DATA DE CREACIï¿? 25-05-2005.
- MODIFICACIï¿? Data. Autor. Descripciï¿?
*/
/**
- FUNCTION: ~CBlobResult
- FUNCTIONALITY: Destructor
- PARAMETERS:
- RESULT:
- RESTRICTIONS:
- AUTHOR: Ricard Borrï¿½s
- CREATION DATE: 25-05-2005.
- MODIFICATION: Date. Author. Description.
*/
CBlobResult::~CBlobResult()
{
	ClearBlobs();
}

/**************************************************************************
		Operadors / Operators
**************************************************************************/


/**
- FUNCIï¿? operador =
- FUNCIONALITAT: Assigna un objecte source a l'actual
- PARï¿½METRES:
	- source: objecte a assignar
- RESULTAT:
	- Substitueix els blobs actuals per els de l'objecte source
- RESTRICCIONS:
- AUTOR: Ricard Borrï¿½s
- DATA DE CREACIï¿? 25-05-2005.
- MODIFICACIï¿? Data. Autor. Descripciï¿?
*/
/**
- FUNCTION: Assigment operator
- FUNCTIONALITY: 
- PARAMETERS:
- RESULT:
- RESTRICTIONS:
- AUTHOR: Ricard Borrï¿½s
- CREATION DATE: 25-05-2005.
- MODIFICATION: Date. Author. Description.
*/
CBlobResult& CBlobResult::operator=(const CBlobResult& source)
{
	// si ja sï¿½n el mateix, no cal fer res
	if (this != &source)
	{
		// alliberem el conjunt de blobs antic
		for( int i = 0; i < GetNumBlobs(); i++ )
		{
			delete m_blobs[i];
		}
		m_blobs.clear();
		// creem el nou a partir del passat com a parï¿½metre
		m_blobs = Blob_vector( source.GetNumBlobs() );
		// copiem els blobs de l'origen a l'actual
		Blob_vector::const_iterator pBlobsSrc = source.m_blobs.begin();
		Blob_vector::iterator pBlobsDst = m_blobs.begin();

		while( pBlobsSrc != source.m_blobs.end() )
		{
			// no podem cridar a l'operador = ja que Blob_vector ï¿½s un 
			// vector de CBlob*. Per tant, creem un blob nou a partir del
			// blob original
			*pBlobsDst = new CBlob(**pBlobsSrc);
			pBlobsSrc++;
			pBlobsDst++;
		}
	}
	return *this;
}


/**
- FUNCIï¿? operador +
- FUNCIONALITAT: Concatena els blobs de dos CBlobResult
- PARï¿½METRES:
	- source: d'on s'agafaran els blobs afegits a l'actual
- RESULTAT:
	- retorna un nou CBlobResult amb els dos CBlobResult concatenats
- RESTRICCIONS:
- AUTOR: Ricard Borrï¿½s
- DATA DE CREACIï¿? 25-05-2005.
- NOTA: per la implementaciï¿? els blobs del parï¿½metre es posen en ordre invers
- MODIFICACIï¿? Data. Autor. Descripciï¿?
*/
/**
- FUNCTION: + operator
- FUNCTIONALITY: Joins the blobs in source with the current ones
- PARAMETERS:
	- source: object to copy the blobs
- RESULT:
	- object with the actual blobs and the source blobs
- RESTRICTIONS:
- AUTHOR: Ricard Borrï¿½s
- CREATION DATE: 25-05-2005.
- MODIFICATION: Date. Author. Description.
*/
CBlobResult CBlobResult::operator+( const CBlobResult& source ) const
{	
	//creem el resultat a partir dels blobs actuals
	CBlobResult resultat( *this );

	// reservem memï¿½ria per als nous blobs
	resultat.m_blobs.resize( resultat.GetNumBlobs() + source.GetNumBlobs() );

	// declarem els iterador per recï¿½rrer els blobs d'origen i desti
	Blob_vector::const_iterator pBlobsSrc = source.m_blobs.begin();
	Blob_vector::iterator pBlobsDst = resultat.m_blobs.end();

	// insertem els blobs de l'origen a l'actual
	while( pBlobsSrc != source.m_blobs.end() )
	{
		pBlobsDst--;
		*pBlobsDst = new CBlob(**pBlobsSrc);
		pBlobsSrc++;
	}
	
	return resultat;
}

/**************************************************************************
		Operacions / Operations
**************************************************************************/

/**
- FUNCIï¿? AddBlob
- FUNCIONALITAT: Afegeix un blob al conjunt
- PARï¿½METRES:
	- blob: blob a afegir
- RESULTAT:
	- modifica el conjunt de blobs actual
- RESTRICCIONS:
- AUTOR: Ricard Borrï¿½s
- DATA DE CREACIï¿? 2006/03/01
- MODIFICACIï¿? Data. Autor. Descripciï¿?
*/
void CBlobResult::AddBlob( CBlob *blob )
{
	if( blob != NULL ){
		CBlob* tp = new CBlob(blob);
		m_blobs.push_back(tp);
	}
}


#ifdef MATRIXCV_ACTIU

/**
- FUNCIï¿? GetResult
- FUNCIONALITAT: Calcula el resultat especificat sobre tots els blobs de la classe
- PARï¿½METRES:
	- evaluador: Qualsevol objecte derivat de COperadorBlob
- RESULTAT:
	- Retorna un array de double's amb el resultat per cada blob
- RESTRICCIONS:
- AUTOR: Ricard Borrï¿½s
- DATA DE CREACIï¿? 25-05-2005.
- MODIFICACIï¿? Data. Autor. Descripciï¿?
*/
/**
- FUNCTION: GetResult
- FUNCTIONALITY: Computes the function evaluador on all the blobs of the class
				 and returns a vector with the result
- PARAMETERS:
	- evaluador: function to apply to each blob (any object derived from the 
				 COperadorBlob class )
- RESULT:
	- vector with all the results in the same order as the blobs
- RESTRICTIONS:
- AUTHOR: Ricard Borrï¿½s
- CREATION DATE: 25-05-2005.
- MODIFICATION: Date. Author. Description.
*/
double_vector CBlobResult::GetResult( blobOperator *evaluador ) const
{
	if( GetNumBlobs() <= 0 )
	{
		return double_vector();
	}

	// definim el resultat
	double_vector result = double_vector( GetNumBlobs() );
	// i iteradors sobre els blobs i el resultat
	double_vector::iterator itResult = result.GetIterator();
	Blob_vector::const_iterator itBlobs = m_blobs.begin();

	// avaluem la funciï¿?en tots els blobs
	while( itBlobs != m_blobs.end() )
	{
		*itResult = (*evaluador)(**itBlobs);
		itBlobs++;
		itResult++;
	}
	return result;
}
#endif

/**
- FUNCIï¿? GetSTLResult
- FUNCIONALITAT: Calcula el resultat especificat sobre tots els blobs de la classe
- PARï¿½METRES:
	- evaluador: Qualsevol objecte derivat de COperadorBlob
- RESULTAT:
	- Retorna un array de double's STL amb el resultat per cada blob
- RESTRICCIONS:
- AUTOR: Ricard Borrï¿½s
- DATA DE CREACIï¿? 25-05-2005.
- MODIFICACIï¿? Data. Autor. Descripciï¿?
*/
/**
- FUNCTION: GetResult
- FUNCTIONALITY: Computes the function evaluator on all the blobs of the class
				 and returns a vector with the result
- PARAMETERS:
	- evaluador: function to apply to each blob (any object derived from the 
				 COperadorBlob class )
- RESULT:
	- vector with all the results in the same order as the blobs
- RESTRICTIONS:
- AUTHOR: Ricard Borrï¿½s
- CREATION DATE: 25-05-2005.
- MODIFICATION: Date. Author. Description.
*/
double_stl_vector CBlobResult::GetSTLResult( blobOperator *evaluador ) const
{
	if( GetNumBlobs() <= 0 )
	{
		return double_stl_vector();
	}

	// definim el resultat
	double_stl_vector result = double_stl_vector( GetNumBlobs() );
	// i iteradors sobre els blobs i el resultat
	double_stl_vector::iterator itResult = result.begin();
	Blob_vector::const_iterator itBlobs = m_blobs.begin();

	// avaluem la funciï¿?en tots els blobs
	while( itBlobs != m_blobs.end() )
	{
		*itResult = (*evaluador)(**itBlobs);
		itBlobs++;
		itResult++;
	}
	return result;
}

/**
- FUNCIï¿? GetNumber
- FUNCIONALITAT: Calcula el resultat especificat sobre un ï¿½nic blob de la classe
- PARï¿½METRES:
	- evaluador: Qualsevol objecte derivat de COperadorBlob
	- indexblob: nï¿½mero de blob del que volem calcular el resultat.
- RESULTAT:
	- Retorna un double amb el resultat
- RESTRICCIONS:
- AUTOR: Ricard Borrï¿½s
- DATA DE CREACIï¿? 25-05-2005.
- MODIFICACIï¿? Data. Autor. Descripciï¿?
*/
/**
- FUNCTION: GetNumber
- FUNCTIONALITY: Computes the function evaluador on a blob of the class
- PARAMETERS:
	- indexBlob: index of the blob to compute the function
	- evaluador: function to apply to each blob (any object derived from the 
				 COperadorBlob class )
- RESULT:
- RESTRICTIONS:
- AUTHOR: Ricard Borrï¿½s
- CREATION DATE: 25-05-2005.
- MODIFICATION: Date. Author. Description.
*/
double CBlobResult::GetNumber( int indexBlob, blobOperator *evaluador ) const
{
	if( indexBlob < 0 || indexBlob >= GetNumBlobs() )
		RaiseError( EXCEPTION_BLOB_OUT_OF_BOUNDS );
	return (*evaluador)( *m_blobs[indexBlob] );
}

/**
- FUNCIï¿? Filter (const version)
- FUNCIONALITAT: Filtra els blobs de la classe i deixa el resultat amb nomï¿½s 
			   els blobs que han passat el filtre.
			   El filtrat es basa en especificar condicions sobre un resultat dels blobs
			   i seleccionar (o excloure) aquells blobs que no compleixen una determinada
			   condicio
- PARï¿½METRES:
	- dst: variable per deixar els blobs filtrats
	- filterAction:	acciï¿?de filtrat. Incloure els blobs trobats (B_INCLUDE),
				    o excloure els blobs trobats (B_EXCLUDE)
	- evaluador: Funciï¿?per evaluar els blobs (qualsevol objecte derivat de COperadorBlob
	- Condition: tipus de condiciï¿?que ha de superar la mesura (FilterType) 
				 sobre cada blob per a ser considerat.
				    B_EQUAL,B_NOT_EQUAL,B_GREATER,B_LESS,B_GREATER_OR_EQUAL,
				    B_LESS_OR_EQUAL,B_INSIDE,B_OUTSIDE
	- LowLimit:  valor numï¿½ric per a la comparaciï¿?(Condition) de la mesura (FilterType)
	- HighLimit: valor numï¿½ric per a la comparaciï¿?(Condition) de la mesura (FilterType)
				 (nomï¿½s tï¿?sentit per a aquelles condicions que tenen dos valors 
				 (B_INSIDE, per exemple).
- RESULTAT:
	- Deixa els blobs resultants del filtrat a destination
- RESTRICCIONS:
- AUTOR: Ricard Borrï¿½s
- DATA DE CREACIï¿? 25-05-2005.
- MODIFICACIï¿? Data. Autor. Descripciï¿?
*/
/**
- FUNCTION: Filter (const version)
- FUNCTIONALITY: Get some blobs from the class based on conditions on measures
				 of the blobs. 
- PARAMETERS:
	- dst: where to store the selected blobs
	- filterAction:	B_INCLUDE: include the blobs which pass the filter in the result 
				    B_EXCLUDE: exclude the blobs which pass the filter in the result 
	- evaluador: Object to evaluate the blob
	- Condition: How to decide if  the result returned by evaluador on each blob
				 is included or not. It can be:
				    B_EQUAL,B_NOT_EQUAL,B_GREATER,B_LESS,B_GREATER_OR_EQUAL,
				    B_LESS_OR_EQUAL,B_INSIDE,B_OUTSIDE
	- LowLimit:  numerical value to evaluate the Condition on evaluador(blob)
	- HighLimit: numerical value to evaluate the Condition on evaluador(blob).
				 Only useful for B_INSIDE and B_OUTSIDE
- RESULT:
	- It returns on dst the blobs that accomplish (B_INCLUDE) or discards (B_EXCLUDE)
	  the Condition on the result returned by evaluador on each blob
- RESTRICTIONS:
- AUTHOR: Ricard Borrï¿½s
- CREATION DATE: 25-05-2005.
- MODIFICATION: Date. Author. Description.
*/
/////////////////////////// FILTRAT DE BLOBS ////////////////////////////////////
void CBlobResult::Filter(CBlobResult &dst, 
						 int filterAction, 
						 blobOperator *evaluador, 
						 int condition, 
						 double lowLimit, double highLimit /*=0*/) const
							
{
	// do the job
	DoFilter(dst, filterAction, evaluador, condition, lowLimit, highLimit );
}


/**
- FUNCIï¿? Filter
- FUNCIONALITAT: Filtra els blobs de la classe i deixa el resultat amb nomï¿½s 
			   els blobs que han passat el filtre.
			   El filtrat es basa en especificar condicions sobre un resultat dels blobs
			   i seleccionar (o excloure) aquells blobs que no compleixen una determinada
			   condicio
- PARï¿½METRES:
	- dst: variable per deixar els blobs filtrats
	- filterAction:	acciï¿?de filtrat. Incloure els blobs trobats (B_INCLUDE),
				    o excloure els blobs trobats (B_EXCLUDE)
	- evaluador: Funciï¿?per evaluar els blobs (qualsevol objecte derivat de COperadorBlob
	- Condition: tipus de condiciï¿?que ha de superar la mesura (FilterType) 
				 sobre cada blob per a ser considerat.
				    B_EQUAL,B_NOT_EQUAL,B_GREATER,B_LESS,B_GREATER_OR_EQUAL,
				    B_LESS_OR_EQUAL,B_INSIDE,B_OUTSIDE
	- LowLimit:  valor numï¿½ric per a la comparaciï¿?(Condition) de la mesura (FilterType)
	- HighLimit: valor numï¿½ric per a la comparaciï¿?(Condition) de la mesura (FilterType)
				 (nomï¿½s tï¿?sentit per a aquelles condicions que tenen dos valors 
				 (B_INSIDE, per exemple).
- RESULTAT:
	- Deixa els blobs resultants del filtrat a destination
- RESTRICCIONS:
- AUTOR: Ricard Borrï¿½s
- DATA DE CREACIï¿? 25-05-2005.
- MODIFICACIï¿? Data. Autor. Descripciï¿?
*/
/**
- FUNCTION: Filter
- FUNCTIONALITY: Get some blobs from the class based on conditions on measures
				 of the blobs. 
- PARAMETERS:
	- dst: where to store the selected blobs
	- filterAction:	B_INCLUDE: include the blobs which pass the filter in the result 
				    B_EXCLUDE: exclude the blobs which pass the filter in the result 
	- evaluador: Object to evaluate the blob
	- Condition: How to decide if  the result returned by evaluador on each blob
				 is included or not. It can be:
				    B_EQUAL,B_NOT_EQUAL,B_GREATER,B_LESS,B_GREATER_OR_EQUAL,
				    B_LESS_OR_EQUAL,B_INSIDE,B_OUTSIDE
	- LowLimit:  numerical value to evaluate the Condition on evaluador(blob)
	- HighLimit: numerical value to evaluate the Condition on evaluador(blob).
				 Only useful for B_INSIDE and B_OUTSIDE
- RESULT:
	- It returns on dst the blobs that accomplish (B_INCLUDE) or discards (B_EXCLUDE)
	  the Condition on the result returned by evaluador on each blob
- RESTRICTIONS:
- AUTHOR: Ricard Borrï¿½s
- CREATION DATE: 25-05-2005.
- MODIFICATION: Date. Author. Description.
*/
void CBlobResult::Filter(CBlobResult &dst, 
						 int filterAction, 
						 blobOperator *evaluador, 
						 int condition, 
						 double lowLimit, double highLimit /*=0*/)
							
{
	int numBlobs = GetNumBlobs();

	// do the job
	DoFilter(dst, filterAction, evaluador, condition, lowLimit, highLimit );

	// inline operation: remove previous blobs
	if( &dst == this ) 
	{
		// esborrem els primers blobs ( que sï¿½n els originals )
		// ja que els tindrem replicats al final si passen el filtre
		Blob_vector::iterator itBlobs = m_blobs.begin();
		for( int i = 0; i < numBlobs; i++ )
		{
			delete *itBlobs;
			itBlobs++;
		}
		m_blobs.erase( m_blobs.begin(), itBlobs );
	}
}

void CBlobResult::Filter( CBlobResult &dst, FilterAction filterAction, blobOperator *evaluador, FilterCondition condition, double lowLimit, double highLimit /*= 0 */ )
{
	Filter(dst,(int)filterAction,evaluador,(int)condition,lowLimit,highLimit);
}


//! Does the Filter method job
void CBlobResult::DoFilter(CBlobResult &dst, int filterAction, blobOperator *evaluador, 
						   int condition, double lowLimit, double highLimit/* = 0*/) const
{
	int i, numBlobs;
	bool resultavaluacio;
	double_stl_vector avaluacioBlobs;
	double_stl_vector::iterator itavaluacioBlobs;

	if( GetNumBlobs() <= 0 ) return;
	if( !evaluador ) return;
	//avaluem els blobs amb la funciï¿?pertinent	
	avaluacioBlobs = GetSTLResult(evaluador);
	itavaluacioBlobs = avaluacioBlobs.begin();
	numBlobs = GetNumBlobs();
	switch(condition)
	{
		case B_EQUAL:
			for(i=0;i<numBlobs;i++, itavaluacioBlobs++)
			{
				resultavaluacio= *itavaluacioBlobs == lowLimit;
				if( ( resultavaluacio && filterAction == B_INCLUDE ) ||
					( !resultavaluacio && filterAction == B_EXCLUDE ))
				{
					dst.m_blobs.push_back( new CBlob( GetBlob( i ) ));
				}				
			}
			break;
		case B_NOT_EQUAL:
			for(i=0;i<numBlobs;i++, itavaluacioBlobs++)
			{
				resultavaluacio = *itavaluacioBlobs != lowLimit;
				if( ( resultavaluacio && filterAction == B_INCLUDE ) ||
					( !resultavaluacio && filterAction == B_EXCLUDE ))
				{
					dst.m_blobs.push_back( new CBlob( GetBlob( i ) ));
				}
			}
			break;
		case B_GREATER:
			for(i=0;i<numBlobs;i++, itavaluacioBlobs++)
			{
				resultavaluacio= *itavaluacioBlobs > lowLimit;
				if( ( resultavaluacio && filterAction == B_INCLUDE ) ||
					( !resultavaluacio && filterAction == B_EXCLUDE ))
				{
					dst.m_blobs.push_back( new CBlob( GetBlob( i ) ));
				}
			}
			break;
		case B_LESS:
			for(i=0;i<numBlobs;i++, itavaluacioBlobs++)
			{
				resultavaluacio= *itavaluacioBlobs < lowLimit;
				if( ( resultavaluacio && filterAction == B_INCLUDE ) ||
					( !resultavaluacio && filterAction == B_EXCLUDE ))
				{
					dst.m_blobs.push_back( new CBlob( GetBlob( i ) ));
				}
			}
			break;
		case B_GREATER_OR_EQUAL:
			for(i=0;i<numBlobs;i++, itavaluacioBlobs++)
			{
				resultavaluacio= *itavaluacioBlobs>= lowLimit;
				if( ( resultavaluacio && filterAction == B_INCLUDE ) ||
					( !resultavaluacio && filterAction == B_EXCLUDE ))
				{
					dst.m_blobs.push_back( new CBlob( GetBlob( i ) ));
				}
			}
			break;
		case B_LESS_OR_EQUAL:
			for(i=0;i<numBlobs;i++, itavaluacioBlobs++)
			{
				resultavaluacio= *itavaluacioBlobs <= lowLimit;
				if( ( resultavaluacio && filterAction == B_INCLUDE ) ||
					( !resultavaluacio && filterAction == B_EXCLUDE ))
				{
					dst.m_blobs.push_back( new CBlob( GetBlob( i ) ));
				}
			}
			break;
		case B_INSIDE:
			for(i=0;i<numBlobs;i++, itavaluacioBlobs++)
			{
				resultavaluacio=( *itavaluacioBlobs >= lowLimit) && ( *itavaluacioBlobs <= highLimit); 
				if( ( resultavaluacio && filterAction == B_INCLUDE ) ||
					( !resultavaluacio && filterAction == B_EXCLUDE ))
				{
					dst.m_blobs.push_back( new CBlob( GetBlob( i ) ));
				}
			}
			break;
		case B_OUTSIDE:
			for(i=0;i<numBlobs;i++, itavaluacioBlobs++)
			{
				resultavaluacio=( *itavaluacioBlobs < lowLimit) || ( *itavaluacioBlobs > highLimit); 
				if( ( resultavaluacio && filterAction == B_INCLUDE ) ||
					( !resultavaluacio && filterAction == B_EXCLUDE ))
				{
					dst.m_blobs.push_back( new CBlob( GetBlob( i ) ));
				}
			}
			break;
	}
}
/**
- FUNCIï¿? GetBlob
- FUNCIONALITAT: Retorna un blob si aquest existeix (index != -1)
- PARï¿½METRES:
	- indexblob: index del blob a retornar
- RESULTAT:
- RESTRICCIONS:
- AUTOR: Ricard Borrï¿½s
- DATA DE CREACIï¿? 25-05-2005.
- MODIFICACIï¿? Data. Autor. Descripciï¿?
*/
/*
- FUNCTION: GetBlob
- FUNCTIONALITY: Gets the n-th blob (without ordering the blobs)
- PARAMETERS:
	- indexblob: index in the blob array
- RESULT:
- RESTRICTIONS:
- AUTHOR: Ricard Borrï¿½s
- CREATION DATE: 25-05-2005.
- MODIFICATION: Date. Author. Description.
*/
CBlob CBlobResult::GetBlob(int indexblob) const
{	
	if( indexblob < 0 || indexblob >= GetNumBlobs() )
		RaiseError( EXCEPTION_BLOB_OUT_OF_BOUNDS );

	return *m_blobs[indexblob];
}
CBlob *CBlobResult::GetBlob(int indexblob)
{	
	if( indexblob < 0 || indexblob >= GetNumBlobs() )
		RaiseError( EXCEPTION_BLOB_OUT_OF_BOUNDS );
	return m_blobs[indexblob];
}

CBlob CBlobResult::GetBlobByID(t_labelType id) const{
	for(int i = 0;i<GetNumBlobs();i++){
		if(GetBlob(i).GetID()==id){
			return m_blobs[i];
		}
	}
	RaiseError( EXCEPTION_EXECUTE_FAULT );
	return CBlob();
}

CBlob *CBlobResult::GetBlobByID(t_labelType id){
	for(int i = 0;i<GetNumBlobs();i++){
		if(GetBlob(i)->GetID()==id){
			return m_blobs[i];
		}
	}
	RaiseError( EXCEPTION_EXECUTE_FAULT );
	return (new CBlob());
}

/**
- FUNCIï¿? GetNthBlob
- FUNCIONALITAT: Retorna l'enï¿½ssim blob segons un determinat criteri
- PARï¿½METRES:
	- criteri: criteri per ordenar els blobs (objectes derivats de COperadorBlob)
	- nBlob: index del blob a retornar
	- dst: on es retorna el resultat
- RESULTAT:
	- retorna el blob nBlob a dst ordenant els blobs de la classe segons el criteri
	  en ordre DESCENDENT. Per exemple, per obtenir el blob major:
		GetNthBlob( CBlobGetArea(), 0, blobMajor );
		GetNthBlob( CBlobGetArea(), 1, blobMajor ); (segon blob mï¿½s gran)
- RESTRICCIONS:
- AUTOR: Ricard Borrï¿½s
- DATA DE CREACIï¿? 25-05-2005.
- MODIFICACIï¿? Data. Autor. Descripciï¿?
*/
/*
- FUNCTION: GetNthBlob
- FUNCTIONALITY: Gets the n-th blob ordering first the blobs with some criteria
- PARAMETERS:
	- criteri: criteria to order the blob array
	- nBlob: index of the returned blob in the ordered blob array
	- dst: where to store the result
- RESULT:
- RESTRICTIONS:
- AUTHOR: Ricard Borrï¿½s
- CREATION DATE: 25-05-2005.
- MODIFICATION: Date. Author. Description.
*/
void CBlobResult::GetNthBlob( blobOperator *criteri, int nBlob, CBlob &dst ) const
{
	// verifiquem que no estem accedint fora el vector de blobs
	if( nBlob < 0 || nBlob >= GetNumBlobs() )
	{
		//RaiseError( EXCEPTION_BLOB_OUT_OF_BOUNDS );
		dst = CBlob();
		return;
	}

	double_stl_vector avaluacioBlobs, avaluacioBlobsOrdenat;
	double valorEnessim;

	//avaluem els blobs amb la funciï¿?pertinent	
	avaluacioBlobs = GetSTLResult(criteri);

	avaluacioBlobsOrdenat = double_stl_vector( GetNumBlobs() );

	// obtenim els nBlob primers resultats (en ordre descendent)
	std::partial_sort_copy( avaluacioBlobs.begin(), 
						    avaluacioBlobs.end(),
						    avaluacioBlobsOrdenat.begin(), 
						    avaluacioBlobsOrdenat.end(),
						    std::greater<double>() );

	valorEnessim = avaluacioBlobsOrdenat[nBlob];

	// busquem el primer blob que tï¿?el valor n-ssim
	double_stl_vector::const_iterator itAvaluacio = avaluacioBlobs.begin();

	bool trobatBlob = false;
	int indexBlob = 0;
	while( itAvaluacio != avaluacioBlobs.end() && !trobatBlob )
	{
		if( *itAvaluacio == valorEnessim )
		{
			trobatBlob = true;
			dst = CBlob( GetBlob(indexBlob));
		}
		itAvaluacio++;
		indexBlob++;
	}
}

/**
- FUNCIï¿? ClearBlobs
- FUNCIONALITAT: Elimina tots els blobs de l'objecte
- PARï¿½METRES:
- RESULTAT: 
	- Allibera tota la memï¿½ria dels blobs
- RESTRICCIONS:
- AUTOR: Ricard Borrï¿½s Navarra
- DATA DE CREACIï¿? 25-05-2005.
- MODIFICACIï¿? Data. Autor. Descripciï¿?
*/
/*
- FUNCTION: ClearBlobs
- FUNCTIONALITY: Clears all the blobs from the object and releases all its memory
- PARAMETERS:
- RESULT:
- RESTRICTIONS:
- AUTHOR: Ricard Borrï¿½s
- CREATION DATE: 25-05-2005.
- MODIFICATION: Date. Author. Description.
*/
void CBlobResult::ClearBlobs()
{
	Blob_vector::iterator itBlobs = m_blobs.begin();
	while( itBlobs != m_blobs.end() )
	{
		delete *itBlobs;
		itBlobs++;
	}

	m_blobs.clear();
}

/**
- FUNCIï¿? RaiseError
- FUNCIONALITAT: Funciï¿?per a notificar errors al l'usuari (en debug) i llenï¿½a
			   les excepcions
- PARï¿½METRES:
	- errorCode: codi d'error
- RESULTAT: 
	- Ensenya un missatge a l'usuari (en debug) i llenï¿½a una excepciï¿?- RESTRICCIONS:
- AUTOR: Ricard Borrï¿½s Navarra
- DATA DE CREACIï¿? 25-05-2005.
- MODIFICACIï¿? Data. Autor. Descripciï¿?
*/
/*
- FUNCTION: RaiseError
- FUNCTIONALITY: Error handling function
- PARAMETERS:
	- errorCode: reason of the error
- RESULT:
	- in _SHOW_ERRORS version, shows a message box with the error. In release is silent.
	  In both cases throws an exception with the error.
- RESTRICTIONS:
- AUTHOR: Ricard Borrï¿½s
- CREATION DATE: 25-05-2005.
- MODIFICATION: Date. Author. Description.
*/
void CBlobResult::RaiseError(const int errorCode) const
{
//! Do we need to show errors?
#ifdef _SHOW_ERRORS
	CString msg, format = "Error en CBlobResult: %s";

	switch (errorCode)
	{
	case EXCEPTION_BLOB_OUT_OF_BOUNDS:
		msg.Format(format, "Intentant accedir a un blob no existent");
		break;
	default:
		msg.Format(format, "Codi d'error desconegut");
		break;
	}

	AfxMessageBox(msg);

#endif
	throw errorCode;
}



/**************************************************************************
		Auxiliars / Auxiliary functions
**************************************************************************/


/**
- FUNCIï¿? PrintBlobs
- FUNCIONALITAT: Escriu els parï¿½metres (ï¿½rea, perï¿½metre, exterior, mitjana) 
			   de tots els blobs a un fitxer.
- PARï¿½METRES:
	- nom_fitxer: path complet del fitxer amb el resultat
- RESULTAT:
- RESTRICCIONS:
- AUTOR: Ricard Borrï¿½s
- DATA DE CREACIï¿? 25-05-2005.
- MODIFICACIï¿? Data. Autor. Descripciï¿?
*/
/*
- FUNCTION: PrintBlobs
- FUNCTIONALITY: Prints some blob features in an ASCII file
- PARAMETERS:
	- nom_fitxer: full path + filename to generate
- RESULT:
- RESTRICTIONS:
- AUTHOR: Ricard Borrï¿½s
- CREATION DATE: 25-05-2005.
- MODIFICATION: Date. Author. Description.
*/
void CBlobResult::PrintBlobs( char *nom_fitxer ) const
{
	double_stl_vector area, /*perimetre,*/ exterior, compacitat, longitud, 
					  externPerimeter, perimetreConvex, perimetre;
	int i;
	FILE *fitxer_sortida;

 	area      = GetSTLResult( CBlobGetArea());
	perimetre = GetSTLResult( CBlobGetPerimeter());
	exterior  = GetSTLResult( CBlobGetExterior());
	compacitat = GetSTLResult(CBlobGetCompactness());
	longitud  = GetSTLResult( CBlobGetLength());
	externPerimeter = GetSTLResult( CBlobGetExternPerimeter());
	perimetreConvex = GetSTLResult( CBlobGetHullPerimeter());

	fitxer_sortida = fopen(nom_fitxer, "w" );

	for(i=0; i<GetNumBlobs(); i++)
	{
		fprintf( fitxer_sortida, "blob %d ->\t a=%7.0f\t p=%8.2f (%8.2f extern)\t pconvex=%8.2f\t ext=%.0f\t c=%3.2f\t l=%8.2f\n",
				 i, area[i], perimetre[i], externPerimeter[i], perimetreConvex[i], exterior[i], compacitat[i], longitud[i] );
	}
	fclose( fitxer_sortida );

}

CBlob* CBlobResult::getBlobNearestTo( cv::Point pt )
{
	float minD = FLT_MAX,d=0;
	int numBlobs = int(m_blobs.size());
	int indNearest = -1;

	for(int i=0;i<numBlobs;i++)
	{
		cv::Point diff = m_blobs[i]->getCenter() - pt;
		d = float(diff.x*diff.x+diff.y*diff.y);
		if(minD > d){
			indNearest = i;
			minD=d;
		}
	}
	if(indNearest!=-1)
		return m_blobs[indNearest];
	else
		return NULL;
}