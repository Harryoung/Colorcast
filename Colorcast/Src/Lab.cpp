#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;
/********************************************************************************************
*函数描述：  calcCast    计算并返回一幅图像的色偏度以及，色偏方向
*函数参数：  InputImg    需要计算的图片，BGR存放格式，彩色（3通道），灰度图无效
*           cast        计算出的偏差值，小于1表示比较正常，大于1表示存在色偏
*           da          红/绿色偏估计值，da大于0，表示偏红；da小于0表示偏绿
*           db          黄/蓝色偏估计值，db大于0，表示偏黄；db小于0表示偏蓝
*函数返回值： 返回值通过cast、da、db三个应用返回，无显式返回值
*********************************************************************************************/
void colorException(Mat InputImg,float& cast,float& da,float& db)
{
    Mat LABimg;
    cvtColor(InputImg,LABimg,CV_BGR2Lab);//参考http://blog.csdn.net/laviewpbt/article/details/9335767
                                       //由于OpenCV定义的格式是uint8，这里输出的LABimg从标准的0～100，-127～127，-127～127，被映射到了0～255，0～255，0～255空间
    float a=0,b=0;
    int HistA[256],HistB[256];
    for(int i=0;i<256;i++)
    {
        HistA[i]=0;
        HistB[i]=0;
    }
    for(int i=0;i<LABimg.rows;i++)
    {
        for(int j=0;j<LABimg.cols;j++)
        {
            a+=float(LABimg.at<cv::Vec3b>(i,j)[1]-128);//在计算过程中，要考虑将CIE L*a*b*空间还原 后同
            b+=float(LABimg.at<cv::Vec3b>(i,j)[2]-128);
            int x=LABimg.at<cv::Vec3b>(i,j)[1];
            int y=LABimg.at<cv::Vec3b>(i,j)[2];
            HistA[x]++;
            HistB[y]++;
        }
    }
    da=a/float(LABimg.rows*LABimg.cols);
    db=b/float(LABimg.rows*LABimg.cols);
    float D =sqrt(da*da+db*db);
    float Ma=0,Mb=0;
    for(int i=0;i<256;i++)
    {
        Ma+=pow(abs(i-128-da),2)*HistA[i];//计算范围-128～127
        Mb+=pow(abs(i-128-db),2)*HistB[i];
    }
    Ma/=float((LABimg.rows*LABimg.cols));
    Mb/=float((LABimg.rows*LABimg.cols));
    float M=sqrt(Ma+Mb);
    float K=D/M;
    cast = K;
    return;
}

int main(){
	Mat image = imread("/Users/bean/Colorcast/image1.jpeg");
	imshow("源图像",image);
	printf("\n 色偏检测\n\n");
    float x = 0.0, y = 0.0, z = 0.0;
	colorException(image, x, y, z);
	printf("x=%f; y=%f; z=%f", x, y, z);
	printf("\n");
	cvWaitKey(0);
	return 0;
}
