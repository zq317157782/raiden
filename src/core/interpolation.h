#pragma once
#include "raiden.h"

/*
这里主要涉及到插值相关的函数
*/

//Catmull–Rom 样条插值
//size    : 值域样本点个数
//nodes   : 值域样本
//x       : 当前的x
//offset  : 指向第一个控制点
//weights : 返回的4个控制点的权重
bool CatmullRomWeights(int size, const Float *nodes, Float x,int *offset, Float *weights);