/*
 * custom_basic.h
 *
 *  Created on: Jul 6, 2018
 *      Author: Jusbin
 */

#ifndef CUSTOM_BASIC_H_
#define CUSTOM_BASIC_H_


#endif /* CUSTOM_BASIC_H_ */

//常用返回类型值
/*!成功执行*/
const int RE_SUCCESS = 0;
/*!文件不存在*/
const int RE_FILENOTEXIST = 1;
/*!文件格式不被支持*/
const int RE_FILENOTSUPPORT = 2;
/*!图像数据类型不正确*/
const int RE_FILETYPEERROR = 3;
/*!创建图像失败*/
const int RE_CREATEFAILED = 4;
/*!输入参数错误*/
const int RE_PARAMERROR = 5;
/*!其它错误*/
const int RE_FAILED = 6;
/*!图像不存在公共区域*/
const int RE_NOSAMEEXTENT = 7;
/*!用户取消操作*/
const int RE_USERCANCEL = 8;
/*!文件已经被使用*/
const int RE_FILEISUSED = 9;
/*!不支持的像素深度*/
const int RE_DEPTHNOTSUPPORT = 10;
/*!波段数量不符合要求*/
const int RE_BANDCOUNTERROR = 11;
/*!文件不存在投影*/
const int RE_NOPROJECTION = 12;
/*!投影不一致*/
const int RE_PROJECTIONDIFF = 13;

const int RE_IOERROR = 14;

//常用数据类型重定义
/*!byte*/
typedef unsigned char	byte;
/*!8U*/
typedef unsigned char	DT_8U;
/*!16U*/
typedef unsigned short	DT_16U;
/*!16S*/
typedef short			DT_16S;
/*!32U*/
typedef unsigned int	DT_32U;
/*!32S*/
typedef int				DT_32S;
/*!32F*/
typedef float			DT_32F;
/*!64F*/
typedef double			DT_64F;
/*!32CF*/
//typedef complex<float>	DT_32CF;
/*!64CF*/
//typedef complex<double>	DT_64CF;

//其它常用宏定义
/**
 * @brief 释放数组
 */
#define RELEASE(x)	if(x!=NULL){delete[]x;x=NULL;}

/**
 * @brief 定义园周率及度和弧度转换
 */
#ifndef M_PI
/*!定义园周率PI*/
#define M_PI	3.1415926535897932384626433832795
/*!定义园周率2*PI*/
#define M_PI2	6.283185307179586476925286766559
/*!弧度转度*/
#define DEG_PER_RAD	((double)(180.0/M_PI))
/*!度转弧度*/
#define RAD_PER_DEG	((double)(M_PI/180.0))
#endif

/**
 * @brief 定义平方
 */
#ifndef M_SQUARE
#define M_SQUARE(x)	(x)*(x)
#endif

/**
 * brief 定义立方
 */
#ifndef M_CUBE
#define M_CUBE(x)	(x)*(x)*(x)
#endif

/*!判断浮点数是否NaN值*/
//inline bool isnan(const float &v)	{return _isnan(v)?true:false;}
/*!判断double数是否NaN值*/
//inline bool isnan(const double &v)	{return _isnan(v)?true:false;}
/*!获取double的NaN值*/
//inline double nan()	{return numeric_limits<double>::quiet_NaN();}

/**
 * @brief float类型的极值
 */
#ifndef FLT_EPSILON
/*!浮点数比较EPSILON值*/
#define FLT_EPSILON	0.0000000000000000000000001
#endif

#ifndef FLT_EQUALS
/*!浮点数是否相等*/
#define FLT_EQUALS(x, y)	(fabs((double)x-y)<FLT_EPSILON)
/*!浮点数是否相等（指定比较阈值）*/
#define FLT_EQUALS_N(x, y, z)	(fabs((double)x-y)<z)
#endif

#ifndef FLT_ZERO
/*!浮点数是否为零*/
#define FLT_ZERO(x)	(fabs(x)<FLT_EPSILON)
#endif
