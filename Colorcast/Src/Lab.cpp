#include <opencv2/opencv.hpp>
#include <fstream>
#include <math.h>
#include <vector>
#include "custom_basic.h"
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>

using namespace std;
using namespace cv;

/********************************************************************************************
*函数描述：  meanValue    计算均值
*函数参数：  image    BGR图像
*函数参数：  NNO_all	黑边掩膜
*函数返回值： 浮点型的均值
*********************************************************************************************/
float meanValue(Mat image, vector<vector<bool> >& NNO_all){
	float mean = 0.0;
	int count = 0;
	for(int i=0;i<image.rows;i++)
		for(int j=0;j<image.cols;j++){
			if(image.at<cv::Vec3b>(i,j)[0] == 0 && image.at<cv::Vec3b>(i,j)[1] == 0 && image.at<cv::Vec3b>(i,j)[2] == 0){
				continue;
			}
			mean += image.at<uchar>(i,j);
			count++;
		}
	mean /= float(count);
	return mean;
}
/********************************************************************************************
*函数描述：  stdev    计算标准差
*函数参数：  iamge    BGR图像
*函数参数：  mean		均值
*函数参数：  NNO_all	黑边掩膜
*函数返回值： 浮点型的标准差值
*********************************************************************************************/
float stdev(Mat image, float mean, vector<vector<bool> >& NNO_all){
	float stdev = 0.0;
	int count = 0;
	for(int i=0;i<image.rows;i++)
		for(int j=0;j<image.cols;j++){
			if(image.at<cv::Vec3b>(i,j)[0] == 0 && image.at<cv::Vec3b>(i,j)[1] == 0 && image.at<cv::Vec3b>(i,j)[2] == 0){
				continue;
			}
			stdev += pow(image.at<uchar>(i,j) - mean, 2);
			count++;
		}
	stdev = sqrt(stdev / float(count));
	return stdev;
}

/********************************************************************************************
*函数描述：  computeCCI    计算色彩度CCI
*函数参数：  image    BGR图像
*函数参数：  NNO_all	黑边掩膜
*函数返回值： 浮点型的色彩度值
*********************************************************************************************/
float computeCCI(Mat image, vector<vector<bool> >& NNO_all){
	Mat BGR[3];
	split(image, BGR);
	Mat rg = BGR[2] - BGR[1];
	Mat yb = (BGR[2] + BGR[1]) / 2 - BGR[0];
	float mu_rg = meanValue(rg, NNO_all);
	float mu_yb = meanValue(yb, NNO_all);
	float stdev_rg = stdev(rg, mu_rg, NNO_all);
	float stdev_yb = stdev(yb, mu_yb, NNO_all);
	float mu_rgyb = sqrt(mu_rg*mu_rg + mu_yb*mu_yb);
	float stdev_rgyb = sqrt(stdev_rg*stdev_rg + stdev_yb* stdev_yb);
	float CCI = stdev_rgyb + 0.3 * mu_rgyb;
	return CCI;
}

/********************************************************************************************
*函数描述：  computeNNO    计算NNO(near neutral objects)区域
*函数参数：  image   原图像
*函数参数：  LABimg   Lab色彩空间的图像
*函数返回值： bool类型的二维数组
*********************************************************************************************/
vector<vector<bool> > computeNNO(Mat image, Mat LABimg)
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
			if(image.at<cv::Vec3b>(i,j)[0] == 0 && image.at<cv::Vec3b>(i,j)[1] == 0 && image.at<cv::Vec3b>(i,j)[2] == 0){
				thisLine.push_back(false);
				continue;
			}
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
*函数描述：  secondTest    计算cast_NNO, da_NNO, db_NNO, D_NNO, M_NNO, M_cr, D_cr特征
*函数参数：  iamge    原图像
*函数参数：  LABimg    Lab色彩空间的图像
*函数参数：  D，M		原图像参数D，M
*函数参数：  out		输出文件
*函数返回值： 无返回值，直接打印检测结果
*********************************************************************************************/
void secondTest(Mat image, Mat LABimg, float D, float M, ofstream& out)
{
	vector<vector<bool> > NNO = computeNNO(image, LABimg);
	float da_NNO = 0.0, db_NNO = 0.0, D_NNO = 0.0, M_NNO = 0.0;
	float cast_NNO = 0.0, M_cr = 0.0, D_cr = 0.0;
	computeEC(LABimg, NNO, cast_NNO, da_NNO, db_NNO, D_NNO, M_NNO);
	M_cr = (M - M_NNO) / M;
	D_cr = (D - D_NNO) / D;
	if(! out.is_open())
	{
		cout << "Error opening output file!!!" << endl;
		exit(1);
	}
	out << cast_NNO << "\t" << da_NNO << "\t" << db_NNO <<"\t" << D_NNO <<"\t" << M_NNO << "\t" << M_cr << "\t" << D_cr << "\t";
	return;
}
/********************************************************************************************
*函数描述：  computeFeatures    计算特征并写入文本文件
*函数参数：  srcImg    输入图像全路径
*函数参数：  outputPath		输出文件路径
*函数参数：  lable		标签：0-正常，1-异常
*函数返回值： 无返回值，写入到文件
*********************************************************************************************/
void computeFeatures(string srcImgPath, string outputPath, int label, ofstream& out){
	Mat image = imread(srcImgPath.c_str());
	Mat LABimg;
	cvtColor(image,LABimg,CV_BGR2Lab);
	vector<vector<bool> > NNO_all;
	for(int i=0;i<LABimg.rows;i++){
		vector<bool> thisLine;
		for(int j=0;j<LABimg.cols;j++){
			if(image.at<cv::Vec3b>(i,j)[0] == 0 && image.at<cv::Vec3b>(i,j)[1] == 0 && image.at<cv::Vec3b>(i,j)[2] == 0)
				thisLine.push_back(false);
			else
				thisLine.push_back(true);
		}
		NNO_all.push_back(thisLine);
	}
	float cast = 0.0, da = 0.0, db = 0.0, D = 0.0, M = 0.0;
	computeEC(LABimg, NNO_all, cast, da, db, D, M);
	if(! out.is_open())
	{
		cout << "Error opening output file!!!" << endl;
		exit(1);
	}
	out << srcImgPath << "\t" << cast << "\t" << da <<"\t" << db <<"\t" << D << "\t" << M << "\t";
	secondTest(image, LABimg, D, M, out);
	float CCI = computeCCI(image, NNO_all);
	out << CCI << "\t" << label << endl;
	return;
}

//主函数
int main(){
	string srcDir = "/Users/bean/Colorcast";
	string abnormalDir = srcDir + "/AbnormalSamples";
	string normalDir = srcDir + "/NormalSamples";
	string dstPath = srcDir + "/train.txt";
	ofstream out(dstPath.c_str(), ios::out | ios::app);
	if (! out.is_open())
	{ cout << "Error creating output file!!!" << endl; exit (1); }
	out << "image\tcast\tda\tdb\tD\tM\tcast_NNO\tda_NNO\tdb_NNO\tD_NNO\tM_NNO\tM_cr\tD_cr\tCCI\tlabel" << endl;
	struct dirent *ptr;
	string curImg;
	DIR *abnormalDIR = opendir(abnormalDir.c_str());
	int count = 0;
	while((ptr = readdir(abnormalDIR)) != NULL) ///read the list of this dir
	    {
			curImg =  ptr->d_name;
			computeFeatures(abnormalDir + "/" + curImg, dstPath, 1, out);
			count++;
			cout << curImg << " has been processed!!!" << " Current count:" << count << endl;

	    }
	closedir(abnormalDIR);
	DIR *normalDIR = opendir(normalDir.c_str());
	while((ptr = readdir(normalDIR)) != NULL) ///read the list of this dir
		{
			curImg =  ptr->d_name;
			computeFeatures(normalDir + "/" + curImg, dstPath, 0, out);
			count++;
			cout << curImg << " has been processed!!!" << " Current count:" << count << endl;
		}
	closedir(normalDIR);
	return 0;
}
