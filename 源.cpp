#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <fstream>
using namespace std;
using namespace cv;


int main()
{
	/* Í¼Ïñ¾ØÕó¶ÁÈ¡ */
	int imageMatrix[255 * 255];
	ifstream mapFile("map.txt", ios::in);
	for(int i = 0;i < 255 * 255;i++)
    { 
		char ch;
		mapFile >> imageMatrix[i];
	    mapFile.get(ch);
    }
	mapFile.close();

	Mat image(255,255,CV_32F);
	for(int i = 0;i < image.rows;i++)
		for(int j = 0;j < image.cols;j++)
		{
			int temp = *(imageMatrix + i*image.rows + j);
			float input;
			if(temp == 0)
				input = 1;
			else if(temp == 1)
				input = 0.5;
			else
				input = 0;
			image.at<float>(i,j) = input;
		}

		cvSaveImage("123.jpg", image);
	namedWindow("MyPicture");
    imshow("MyPicture",image);
    waitKey(0);
    return 0;
}
