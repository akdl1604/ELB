#if !defined LINEF
#define LINEF
 
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#define PI 3.1415926
 
class LineFinder {
 
  private:
 
      // original image
      cv::Mat img;
 
      // vector containing the end points 
      // of the detected lines
      std::vector<cv::Vec4i> lines;
 
      // accumulator resolution parameters
      double deltaRho;
      double deltaTheta;
 
      // minimum number of votes that a line 
      // 선으로 간주되기 전에 받아야 하는 투표 최소 개수
      int minVote;
 
      // 선의 최소 길이
      double minLength;
 
      // 선을 따라가는 최대 허용 간격(gap)
      double maxGap;
 
  public:
 
      // Default accumulator resolution is 1 pixel by 1 degree
      // no gap, no mimimum length
      LineFinder() : deltaRho(1), deltaTheta(PI/180), minVote(10), minLength(0.), maxGap(0.) {}
 
      // Set the minimum number of votes
      void setMinVote(int minv) {
 
          minVote= minv;
      }
 
      // Set line length and gap
      void setLineLengthAndGap(double length, double gap) {
 
          minLength= length;
          maxGap= gap;
      }
 
      // Apply probabilistic Hough Transform
      std::vector<cv::Vec4i> findLines(cv::Mat& binary) {
 
          lines.clear();
          cv::HoughLinesP(binary,lines,deltaRho,deltaTheta,minVote, minLength, maxGap);
 
          return lines;
      }
	  
	  void findLines(cv::Mat& nputimg, std::vector<cv::Vec4f> &lines, cv::Mat &outputimg) {
	   
	       cv::Mat reduced_noise,tmp_inputimg;
		   cv::equalizeHist(inputimg, tmp_inputimg);
		   
		   cv::bilateralFilter(tmp_inputimg, reduced_noise, 5, 75, 75, 4);

		   cv::Ptr<cv::LineSegmentDetector> Line_detector = cv::createLineSegmentDetector();
		   Line_detector->detect(reduced_noise, lines);
		   Line_detector->drawSegments(outputimg, lines);
		   
		   tmp_inputimg.release();
		   reduced_noise.release();
	  }
 
      // Draw the detected lines on an image
      int drawDetectedLines(cv::Mat &image, cv::Scalar color=cv::Scalar(0,0,255)) {
    
          // Draw the lines
          std::vector<cv::Vec4i>::const_iterator it2= lines.begin();
    
          while (it2!=lines.end()) {
        
              cv::Point pt1((*it2)[0],(*it2)[1]);        
              cv::Point pt2((*it2)[2],(*it2)[3]);
              
              cv::line( image, pt1, pt2, color, 5); 
        
              ++it2;    
          }
      }
 
      // Eliminates lines that do not have an orientation equals to
      // the ones specified in the input matrix of orientations
      // At least the given percentage of pixels on the line must 
      // be within plus or minus delta of the corresponding orientation
      std::vector<cv::Vec4i> removeLinesOfInconsistentOrientations(
          const cv::Mat &orientations, double percentage, double delta) {
 
              std::vector<cv::Vec4i>::iterator it= lines.begin();
    
              // check all lines
              while (it!=lines.end()) {
 
                  // end points
                  int x1= (*it)[0];
                  int y1= (*it)[1];
                  int x2= (*it)[2];
                  int y2= (*it)[3];
           
                  // line orientation + 90o to get the parallel line
                  double ori1= atan2(static_cast<double>(y1-y2),static_cast<double>(x1-x2))+PI/2;
                  if (ori1>PI) ori1= ori1-2*PI;
 
                  double ori2= atan2(static_cast<double>(y2-y1),static_cast<double>(x2-x1))+PI/2;
                  if (ori2>PI) ori2= ori2-2*PI;
    
                  // for all points on the line
                  cv::LineIterator lit(orientations,cv::Point(x1,y1),cv::Point(x2,y2));
                  int i,count=0;
                  for(i = 0, count=0; i < lit.count; i++, ++lit) { 
        
                      float ori= *(reinterpret_cast<float *>(*lit));
 
                      // is line orientation similar to gradient orientation ?
                      if (std::min(fabs(ori-ori1),fabs(ori-ori2))<delta)
                          count++;
        
                  }
 
                  double consistency= count/static_cast<double>(i);
 
                  // set to zero lines of inconsistent orientation
                  if (consistency < percentage) {
 
                      (*it)[0]=(*it)[1]=(*it)[2]=(*it)[3]=0;
 
                  }
 
                  ++it;
              }
 
              return lines;
      }
	  
	  void computeStrengthOfEdges(const cv::Mat &magnitude, vector<Vec4f> lines) 
	  {
		vector<float> magnitude_of_line;
		vector<float> strength;

		for (size_t i = 0; i < lines.size(); i++) {

			// Generate a LineIterator for every line
			LineIterator line_iterator(magnitude, Point2f(lines[i][0], lines[i][1]), Point2f(lines[i][2], lines[i][3]), 8, true);

			for (int j = 0; j < line_iterator.count; j++, ++line_iterator) {
			// Get all values the line hits (8 way-connection) in the magnitude image
				float magnitude_value = magnitude.at<float>(line_iterator.pos()); 
				magnitude_of_line.push_back(magnitude_value);
			}
			// Calculate the sum all values
			float sum = accumulate(magnitude_of_line.begin(), magnitude_of_line.end(), 0.0);    
			strength.push_back(sum);
			magnitude_of_line.clear();      
		}   
	}
};
 
 
#endif