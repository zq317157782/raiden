#include "interpolation.h"

bool CatmullRomWeights(int size, const Float *nodes, Float x,int *offset, Float *weights){
    //先判断x是否在有效的值域内
    //PBRT在这里还解释道，使用这种条件是为了同时判断NaN的情况
    //一旦NaN参与比较操作,肯定返回false,于是整体条件为true,因此函数返回false
    if( !(x>=nodes[0]&&x<=nodes[size-1]) ){
        return false;
    }
    //使用二分法寻找最大的小于x的值的索引
    int idx=FindInterval(size,[&](int i){return nodes[i]<=x;});
    //计算offset
    (*offset)=idx-1;

    //获得x0和x1,然后映射到[0-1]之间
    Float x0=nodes[idx];
    Float x1=nodes[idx+1];

    Float t=(x-x0)/(x1-x0);
    Float t2=t*t;
    Float t3=t2*t;

    //使用中心有限差分时候的公式
    //w0=(x^3-2x^2+x)/(x_{-1}-1)
    //w1=(2x^3-3x^2+1) - w3
    //w2=(-2x^3+3x^2) - w0
    //w3=(x^3-x^2)/(x_2)

    //使用向前向后有限差分时候的公式
    //左:
    //w0=0
    //w1=(2x^3-3x^2+1)-(x^3-2x^2+x)-w3
    //w2=(-2x^3+3x^2) +(x^3-2x^2+x)
    //w3=(x^3-x^2)/(x_2)

    //右:
    //w0=(x^3-2x^2+x)/(x_{-1}-1)
    //w1=(2x^3-3x^2+1)-(x^3-x^2) 
    //w2=(-2x^3+3x^2)+(x^3-x^2) - w0
    //w3=0

    //首先计算w1和w2的前面的括号内的内容
    Float w0=0;
    Float w1=2*t3-3*t2+1;
    Float w2=-2*t3+3*t2;
    Float w3=0;
    //计算w0和w3的时候因为涉及到x_{-1}和x_{2},因此可能会越界,所以到分条件处理
    //先处理w0
    if(idx>0){
        //不越界的情况
        //这里没有像pbrt那样做一些优化操作:
        //w0=(t3-2*t2+t)*(x1-x0)/(nodes[idx-1]-x1);
        //w2-=w0;
        //优化:
        w0=(t3-2*t2+t)*(x1-x0)/(x1-nodes[idx-1]);
        w2+=w0;
    }
    else{
        //左越界
        Float v=t3-2*t2+t;
        w1-=v;
        w2+=v;
    }

    //再处理w3
    if((idx+1)<(size-1)){
        //w3=(x^3-x^2)/(x_2)
        w3=(t3-t2)*(x1-x0)/(nodes[idx+2]-x0);
        //w1=(2x^3-3x^2+1) - w3
        w1-=w3;
    }
    else{
        //右越界
         Float v=t3-t2;
         w1-=v;
         w2+=v;
    }

    //真正的赋值的地方
    weights[0]=w0;
    weights[1]=w1;
    weights[2]=w2;
    weights[3]=w3;

    return true;
}