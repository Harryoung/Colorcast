#include <opencv2/opencv.hpp>
#include<math.h>
#include<vector>
#include"custom_basic.h"
using namespace std;
using namespace cv;

//全局变量
float cast_NNO = 0.0, M_cr = 0.0, D_cr = 0.0;

/********************************************************************************************
*函数描述：  meanValue    计算均值
*函数参数：  image    BGR图像
*函数返回值： 浮点型的均值
*********************************************************************************************/
float meanValue(Mat image){
	float mean = 0.0;
	for(int i=0;i<image.rows;i++)
		for(int j=0;j<image.cols;j++)
			mean += image.at<uchar>(i,j);
	mean /= float(image.rows*image.cols);
	return mean;
}
/********************************************************************************************
*函数描述：  stdev    计算标准差
*函数参数：  iamge    BGR图像
*函数参数：  mean		均值
*函数返回值： 浮点型的标准差值
*********************************************************************************************/
float stdev(Mat image, float mean){
	float stdev = 0.0;
	for(int i=0;i<image.rows;i++)
			for(int j=0;j<image.cols;j++)
				stdev += pow(image.at<uchar>(i,j) - mean, 2);
	stdev = sqrt(stdev / float(image.rows*image.cols));
	return stdev;
}

/********************************************************************************************
*函数描述：  computeCCI    计算色彩度CCI
*函数参数：  image    BGR图像
*函数返回值： 浮点型的色彩度值
*********************************************************************************************/
float computeCCI(Mat image){
	Mat BGR[3];
	split(image, BGR);
	Mat rg = BGR[2] - BGR[1];
	Mat yb = (BGR[2] + BGR[1]) / 2 - BGR[0];
	float mu_rg = meanValue(rg);
	float mu_yb = meanValue(yb);
	float stdev_rg = stdev(rg, mu_rg);
	float stdev_yb = stdev(yb, mu_yb);
	float mu_rgyb = sqrt(mu_rg*mu_rg + mu_yb*mu_yb);
	float stdev_rgyb = sqrt(stdev_rg*stdev_rg + stdev_yb* stdev_yb);
	float CCI = stdev_rgyb + 0.3 * mu_rgyb;
	return CCI;
}

/********************************************************************************************
*函数描述：  computeNNO    计算NNO(near neutral objects)区域
*函数参数：  LABimg   Lab色彩空间的图像
*函数返回值： bool类型的二维数组
*********************************************************************************************/
vector<vector<bool> > computeNNO(Mat LABimg)
{
	vector<vector<bool> > NNO;
	float a = 0.0, b = 0.0, maxd2 = 0.0, d = 0.0;
	for(int i=0;i<LABimg.rows;i++)
	{
		for(int j=0;j<LABimg.cols;j++)
		{
			a = float(LABimg.at<cv::Vec3b>(i,j)[1]-128);
			b = float(LABimg.at<cv::Vec3b>(i,j)[2]-128);
			float thisd2 = a*a + b*b;
			if(maxd2 < thisd2){
				maxd2 = thisd2;
			}
		}
	}
	d = sqrt(maxd2);
	for(int i=0;i<LABimg.rows;i++){
		vector<bool> thisLine;
		for(int j=0;j<LABimg.cols;j++){
			float L = float(LABimg.at<cv::Vec3b>(i,j)[0] * 100 / 255);
			a = float(LABimg.at<cv::Vec3b>(i,j)[1]-128);
			b = float(LABimg.at<cv::Vec3b>(i,j)[2]-128);
			if(L >=35 && L <=95 && sqrt(a*a + b*b) <= d/4)
				thisLine.push_back(true);
			else
				thisLine.push_back(false);
		}
		NNO.push_back(thisLine);
	}
    //处理孤立点
    for(int i=0;i<LABimg.rows;i++)
    	for(int j=0;j<LABimg.cols;j++){
    		if(NNO[i][j] == true){
    			int LeftTopX = max(0, i-1);
    			int LeftTopY = max(0, j-1);
    			int RightBottomX = min(LABimg.rows-1, i+1);
    			int RightBottomY = min(LABimg.cols-1, j+1);
    			bool Alone = true;
    			for(int m=LeftTopX;m<=RightBottomX;m++)
    				for(int n=LeftTopY;n<=RightBottomY;n++){
    					if(m==i && n==j)
    						continue;
    					if(NNO[m][n] == true){
    						Alone = false;
    						break;
    					}
    				}
    			if(Alone == true)
    				NNO[i][j] = false;
    		}
    	}
	return NNO;
}

/********************************************************************************************
*函数描述：  computeEC		计算并返回一幅图像的Lab色彩空间等价圆信息
*函数参数：  LABimg		Lab色彩空间的图像
*函数参数：  NNO		NNO区域二值图
*函数参数：
*函数返回值： 返回值通过五个引用返回，无显式返回值
*********************************************************************************************/
void computeEC(Mat LABimg, vector<vector<bool> > NNO, float& cast, float& da, float& db, float& D, float& M)
{
    float a=0,b=0;
    int HistA[256],HistB[256], NNO_count=0;
    for(int i=0;i<256;i++)
    {
        HistA[i]=0;
        HistB[i]=0;
    }
    for(int i=0;i<LABimg.rows;i++)
    {
        for(int j=0;j<LABimg.cols;j++)
        {
        	if(NNO[i][j] == true){
				a+=float(LABimg.at<cv::Vec3b>(i,j)[1]-128);//在计算过程中，要考虑将CIE L*a*b*空间还原 后同
				b+=float(LABimg.at<cv::Vec3b>(i,j)[2]-128);
				int x=LABimg.at<cv::Vec3b>(i,j)[1];
				int y=LABimg.at<cv::Vec3b>(i,j)[2];
				HistA[x]++;
				HistB[y]++;
				NNO_count++;
        	}
        }
    }
    da=a/float(NNO_count);
    db=b/float(NNO_count);
    D =sqrt(da*da+db*db);
    float Ma=0,Mb=0;
    for(int i=0;i<256;i++)
    {
        Ma+=pow(i-128-da, 2)*HistA[i];//计算范围-128～127
        Mb+=pow(i-128-db, 2)*HistB[i];
    }
    Ma/=float((NNO_count));
    Mb/=float((NNO_count));
    M=sqrt(Ma+Mb);
    float K=(D - M) / M;
    cast = K;
    return;
}

/********************************************************************************************
*函数描述：  castClassification    对初步判断为色偏的图像进行分类，判别真实色偏与本质色偏
*函数参数：  LABimg    Lab色彩空间图像
*函数参数：  D 				Lab色彩空间等价圆圆心距中性轴距离
*函数参数：  M				Lab色彩空间等价圆半径
*函数返回值： 无返回值，直接打印判别结果
*********************************************************************************************/
void castClassification(Mat LABimg, float D, float M)
{
	//统计L分量直方图并初步筛选本质色偏图像
	int HistL[101];
	for(int i=0;i<101;i++)
		HistL[i] = 0;
	for(int i=0;i<LABimg.rows;i++)
		for(int j=0;j<LABimg.cols;j++){
			int L = int(LABimg.at<cv::Vec3b>(i,j)[0] * 100 / 255);
			HistL[L] ++;
		}
	int maxLIndex = 0;
	int maxL = HistL[maxLIndex];
	for(int i=0;i<101;i++){
		if(maxL < HistL[i]){
			maxL = HistL[i];
			maxLIndex = i;
		}
	}
	for(int i=0;i<101;i++){
		if(HistL[i] < maxL / 100)
			HistL[i] = 0;
	}
	int firstNoZeroIndex = 0;
	int lastNoZeroIndex = 0;
	for(int i=0;i<101;i++){
		if(HistL[i] != 0){
			firstNoZeroIndex = i;
			break;
		}
	}
	for(int j=100;j>=0;j--){
		if(HistL[j] != 0){
			lastNoZeroIndex = j;
			break;
		}
	}
	int LRange = lastNoZeroIndex - firstNoZeroIndex;
	if(maxLIndex - firstNoZeroIndex <= LRange * 0.8 ){
		printf("本质色偏！！！\n");
		return;
	}
	if(FLT_EQUALS(M_cr, 0.0)){
		vector<vector<bool> > NNO = computeNNO(LABimg);
		float  da_NNO = 0.0, db_NNO = 0.0, D_NNO = 0.0, M_NNO = 0.0;
		computeEC(LABimg, NNO, cast_NNO, da_NNO, db_NNO, D_NNO, M_NNO);
		M_cr = (M - M_NNO) / M;
		D_cr = (D - D_NNO) / D;
	}
	if(cast_NNO < -0.3 && M_cr > 0.7 && D_cr > 0.6){
		printf("本质色偏！！！\n");
	}
	else{
		printf("真实色偏！！！\n");
	}
	return;
}



/********************************************************************************************
*函数描述：  secondTest    对初步判断为非色偏的图像进行二次检测
*函数参数：  LABimg    Lab色彩空间的图像
*函数返回值： 无返回值，直接打印检测结果
*********************************************************************************************/
void secondTest(Mat LABimg, float D, float M)
{
	vector<vector<bool> > NNO = computeNNO(LABimg);
	float da_NNO = 0.0, db_NNO = 0.0, D_NNO = 0.0, M_NNO = 0.0;
	computeEC(LABimg, NNO, cast_NNO, da_NNO, db_NNO, D_NNO, M_NNO);
	M_cr = (M - M_NNO) / M;
	D_cr = (D - D_NNO) / D;
	if(cast_NNO < -0.5 || (M_cr > 0.7 && D_cr > 0.6)){
		printf("非色偏图像！！！\n");
	}
	else if(cast_NNO <= 0.5 || (M_cr <= 0.4 && D_cr <= 0.3)){
		printf("色偏图像！！！接下来进行色偏分类...\n");
		castClassification(LABimg, D, M);
	}
	else{
		printf("无法识别！！！\n");
	}
	return;
}

//主函数
int main(){
	Mat image = imread("/Users/bean/Colorcast/image6.jpeg");
	float CCI = computeCCI(image);
	printf("CCI: %f \n", CCI);
//	printf("\n 色偏检测\n\n");
//	Mat LABimg;
//	cvtColor(image,LABimg,CV_BGR2Lab);//参考http://blog.csdn.net/laviewpbt/article/details/9335767
//	                                       //由于OpenCV定义的格式是uint8，这里输出的LABimg从标准的0～100，-127～127，-127～127，被映射到了0～255，0～255，0～255空间
//
//	vector<vector<bool> > NNO_all;
//	for(int i=0;i<LABimg.rows;i++){
//		vector<bool> thisLine;
//		for(int j=0;j<LABimg.cols;j++)
//			thisLine.push_back(true);
//		NNO_all.push_back(thisLine);
//	}
//	float cast = 0.0, da = 0.0, db = 0.0, D = 0.0, M = 0.0;
//	computeEC(LABimg, NNO_all, cast, da, db, D, M);
//	printf("cast=%f; da=%f; db=%f; D=%f; M=%f\n", cast, da, db, D, M);
//	if((D > 10 && cast > 0.6) || cast > 1.5){
//		printf("初步判断为色偏，接下来进行色偏分类...\n");
//		castClassification(LABimg, D, M);
//	}
//	else{
//		printf("初步判断为非色偏，接下来进行二次检测...\n");
//		secondTest(LABimg, D, M);
//	}
//	printf("\n");
//	cvWaitKey(0);
	return 0;
}
