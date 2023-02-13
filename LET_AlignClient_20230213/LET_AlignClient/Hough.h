#pragma once

#include <vector>

class CHough
{
	public:
		CHough();
		~CHough();

 	public: 
 		int Transform(unsigned char* img_data, int w, int h); 
 		std::vector< std::pair< std::pair<int, int>, std::pair<int, int> > > GetLines(int threshold); 
 		const unsigned int* GetAccu(int *w, int *h); 
 	private: 
 		unsigned int* _accu; 
 		int _accu_w; 
 		int _accu_h; 
 		int _img_w; 
 		int _img_h; 
};

