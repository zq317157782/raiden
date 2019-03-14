/*
 * texture.cpp
 *
 *  Created on: 2016年12月29日
 *      Author: zhuqian
 */

#include "texture.h"
#include "mmath.h"
Float Lanczos(Float x, Float tau) {
	//当x非常接近0的时候，取1
	//加快运算么？
    //还是为了防止误差
    x = std::abs(x);

	if(x<1e-5){
		return 1.0;
	}
	if(x>1.0){
		return 0.0;
	}
	x=x*Pi;

	//在x为1的时候，整个sinc走了tau个cycle
	Float sinc=std::sin(x*tau)/(x*tau);
	//lanczos就取sinc的一个周期
	Float lanczos=std::sin(x)/x;
	return sinc*lanczos;
}

