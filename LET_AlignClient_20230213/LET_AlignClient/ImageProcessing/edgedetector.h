#if !defined SOBELEDGES
#define SOBELEDGES
 
#define PI 3.1415926
 
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
 
class EdgeDetector {
 
  private:
 
      // original image
      cv::Mat img;
 
      // 16-bit signed int image
      cv::Mat sobel;
 
      // Aperture size of the Sobel kernel
      int aperture;
 
      // Sobel magnitude
      cv::Mat sobelMagnitude;
 
      // Sobel orientation
      cv::Mat sobelOrientation;
 
  public:
 
      EdgeDetector() : aperture(3) {}
 
      // Compute the Sobel
      void computeSobel(const cv::Mat& image) {
 
          cv::Mat sobelX;
          cv::Mat sobelY;
 
          // Compute Sobel
          cv::Sobel(image, sobelX, CV_32F, 1, 0, aperture);
          cv::Sobel(image, sobelY, CV_32F, 0, 1, aperture);
 
          // Compute magnitude and orientation
          cv::cartToPolar(sobelX, sobelY, sobelMagnitude, sobelOrientation);
      }
 
      // Get Sobel orientation
      cv::Mat getOrientation() {
 
          return sobelOrientation;
      }
 
};
 
 
#endif