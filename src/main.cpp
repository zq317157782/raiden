/*
 * main.cpp
 *
 *  Created on: 2016年11月9日
 *      Author: zhuqian
 */
#include "raiden.h"
#include "geometry.h"
int main(){
	Vector3<Float> v1(1,1,1);
	Vector3<Float> v2(2,2,2);
	Vector3<Float> v3=v1+v2;
	std::cout<<v3.x<<std::endl;
	std::cout<<"hello raiden!!!"<<std::endl;
}



