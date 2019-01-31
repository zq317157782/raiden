#pragma once
#include "raiden.h"

/*
这里主要涉及到插值相关的函数
*/

//CatmullRom曲线是一种Hermite曲线，和其它的Hermite曲线的区别在于导数的计算方式

//Catmull–Rom 样条插值
//size    : 定义域样本点个数
//nodes   : 定义域样本
//x       : 当前的x
//offset  : 指向第一个控制点
//weights : 返回的4个控制点的权重
bool CatmullRomWeights(int size, const Float *nodes, Float x,int *offset, Float *weights);

//求CatmullRom曲线的定积分
//n 控制点个数
//x 每个控制点的定义域值
//values 每个控制点的值域值
//cdf 相应的CDF
Float IntegrateCatmullRom(int n, const Float *x, const Float *values,
                          Float *cdf);

//采样一维的CatmullRom曲线
//返回的是定义域 x
//size    : 定义域样本点个数
//xval   : 定义域样本
//fval    : 值域样本
Float SampleCatmullRom(Float u,int size,const Float *xVal,const Float *fVal,const Float *FVal);