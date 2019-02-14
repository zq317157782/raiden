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

    weights[1]=2*t3-3*t2+1;
    weights[2]=-2*t3+3*t2;
  
    //计算w0和w3的时候因为涉及到x_{-1}和x_{2},因此可能会越界,所以到分条件处理
    //先处理w0
    if(idx>0){
        //不越界的情况
        //这里没有像pbrt那样做一些优化操作:
        Float w0=(t3-2*t2+t)*(x1-x0)/(nodes[idx-1]-x1);
        weights[0]=w0;
        weights[2]-=w0;
    }
    else{
        //左越界
        Float w0=t3-2*t2+t;
        weights[0]=0;
        weights[1]-=w0;
        weights[2]+=w0;
    }

    //再处理w3
    if((idx+1)<(size-1)){
        //w3=(x^3-x^2)/(x_2)
        Float w3=(t3-t2)*(x1-x0)/(nodes[idx+2]-x0);
        //w1=(2x^3-3x^2+1) - w3
        weights[1]-=w3;
        weights[3]=w3;
    }
    else{
        //右越界
         Float w3=t3-t2;
         weights[1]-=w3;
         weights[2]+=w3;
         weights[3]=0;
    }


    return true;
}


Float IntegrateCatmullRom(int n, const Float *x, const Float *values,Float *cdf){
    Float sum=0;//定积分值
    cdf[0]=0;
    for(int i=0;i<n-1;++i){
        Float x0=x[i];
        Float x1=x[i+1];
        Float w=x1-x0;// dx

        Float f0=values[i];
        Float f1=values[i+1];

        Float d0,d1;//相应端点的一阶导数
        //可以使用中心差分的使用中心差分
        //不然使用右差分
        if(i>0){
            d0=w*(f1-values[i-1])/(x1-x[i-1]);//中心差分
        }else{
            d0=f1-f0;//右差分
        }

        if(i<(n-2)){
            d1=w*(values[i+2]-f0)/(x[i+2]-x0);//中心差分
        }else{
            d1=f1-f0;//右差分
        }

        //累计
        //用的是CatmullRom的解析式
        sum+=w*((f1+f0)*0.5f + (d0-d1)*(1.0f/12.0f));
        cdf[i+1]=sum;
    }
    return sum;
}


Float SampleCatmullRom(int n,const Float *x,const Float *f,const Float *F,Float u,Float* fval,Float* pdf){
    //首先找到u所在的区间
    //PDF=函数值/函数的积分
    //CDF=函数的部分积分/函数的积分
    
    //这里使用二分法找到u所在的区间
    u=u*F[n-1];
    int i=FindInterval(n,[&](int i){return F[i]<=u;});
    
    //获得当前区间的相应的值
    Float x0=x[i];
    Float x1=x[i+1];

    Float w=x1-x0;

    Float f0=f[i];
    Float f1=f[i+1];

    Float d0,d1;//相应端点的一阶导数
    //可以使用中心差分的使用中心差分
    //不然使用右差分
    if(i>0){
        d0=w*(f1-f[i-1])/(x1-x[i-1]);//中心差分
    }else{
        d0=f1-f0;//右差分
    }

    if(i<(n-2)){
        d1=w*(f[i+2]-f0)/(x[i+2]-x0);//中心差分
    }else{
        d1=f1-f0;//右差分
    }
    
    //计算给样条线段的积分的逆函数用的样本
    //非标准化的CDF
    //(这是提醒我自己的，这里为啥要除以w? 请记住dt=(x-x0)/w*dx,然后从dt的积分变换到dx的积分，相信我自己能够回忆起来！！！！)
    u=(u-F[i])/w;
    
    //开始使用牛顿-二分法来求定义域值
    //首先先要选择一个合适的初始值,这里假设样条线段是线性的 f(t)=(1-t)f(0)+tf(1)
    //然后求积分 F(t)=tf(0)-t^2f(0)+t^2f(1)
    //然后求逆函数  f(0)-srqt(f(0)^2+2(f(1)-f(0))*x)/(f(0)-f(1))
    Float t;
    if(f1!=f0){
        t=(f0-std::sqrt(std::max(f0*f0+2*(f1-f0)*u,(Float)0)))/(f0-f1);
    }else{
        //这种情况为f0=f1,因此f(x)=f0,然后积分，求逆函数巴拉巴拉，就能得到这个了
        t=u/f0;
    }
    

    int a =0;
    int b =1;
    Float Fhat;
    Float fhat;
    //进入牛顿-二分法
    while(true){
        //首先判断t是否在二分法允许的范围内
        //不在的话，更新t值为当前二分区间的中心点
        if(t<=a||t>=b){
            t=(a+b)*0.5f;
        }
        //计算相应的t的函数值
        //这里是相应的CatmullRom样条的Horner格式
        //这里直接复制于PBRT
        //其实就是换了下函数的排列
        Fhat = t * (f0 + t * (0.5f * d0 + t * ((1.0f/3.0f) * (-2 * d0 - d1) +f1 - f0 + t * (0.25f * (d0 + d1) + 0.5f * (f0 - f1)))));
        fhat = f0 + t * (d0 + t * (-2 * d0 - d1 + 3 * (f1 - f0) + t * (d0 + d1 + 2 * (f0 - f1))));

        //判断是否接近实际的root值
        //牛顿法的话要判断函数值是否接近0
        //二分法的话，比较二分边界是否足够接近
        if( std::abs(Fhat-u)<1e-6f||(b-a)<1e-6f){
            break;
        }
        //到这里的话，说明还需要迭代，提高进度
        
        if(Fhat<u){
            //说明[a,t]不可能包含解,更新a
            a=t;
        }else {
            //说明[t,b]不可能包含解,更新b
            b=t;
        }
        //运用牛顿法
        t=t-(Fhat-u)/fhat;
    }

    if(fval){
        (*fval)=fhat;
    }

    if(pdf){
        (*pdf)=fhat/F[n-1];
    }

    //从[0-1]映射回原来的空间
    return x0+t*w;
}


Float SampleCatmullRom2D(int n1,int n2,const Float *xv1,const Float *xv2,const Float *f,const Float *cdf,Float alpha,Float u,Float* fval,Float* pdf){
    //首先计算alpha的插值权重
    int offset;
    Float weights[4];
    if(!CatmullRomWeights(n1,xv1,alpha,&offset,weights)){
        return 0;
    }

    //通过第一维度的权重，计算第二维度的值得lambda
    auto interpolate=[&](const Float *f,int idx){
        Float value=0;
        //插值操作
        for(int i=0;i<4;++i){
            if(weights[i]!=0){
                value+=f[n2*(offset+i)+idx]*weights[i];
            }
        }
        return value;
    };
    
    //首先找到u所在的区间
    //PDF=函数值/函数的积分
    //CDF=函数的部分积分/函数的积分
    
    //这里使用二分法找到u所在的区间
    Float maximum=interpolate(cdf,n2-1);
    u=u*maximum;
    int i=FindInterval(n2,[&](int idx){return interpolate(cdf,idx)<=u;});

    //获得当前区间的相应的值
    Float x0=xv2[i];
    Float x1=xv2[i+1];

    Float w=x1-x0;

    Float f0= interpolate(f,i); 
    Float f1= interpolate(f,i+1);

    Float d0,d1;//相应端点的一阶导数
    //可以使用中心差分的使用中心差分
    //不然使用右差分
    if(i>0){
        d0=w*(f1-interpolate(f,i-1))/(x1-xv2[i-1]);//中心差分
    }else{
        d0=f1-f0;//右差分
    }

    if(i<(n2-2)){
        d1=w*(interpolate(f,i+2)-f0)/(xv2[i+2]-x0);//中心差分
    }else{
        d1=f1-f0;//右差分
    }

    //计算给样条线段的积分的逆函数用的样本
    //非标准化的CDF
    //(这是提醒我自己的，这里为啥要除以w? 请记住dt=(x-x0)/w*dx,然后从dt的积分变换到dx的积分，相信我自己能够回忆起来！！！！)
    u=(u-interpolate(cdf,i))/w;

    //开始使用牛顿-二分法来求定义域值
    //首先先要选择一个合适的初始值,这里假设样条线段是线性的 f(t)=(1-t)f(0)+tf(1)
    //然后求积分 F(t)=tf(0)-t^2f(0)+t^2f(1)
    //然后求逆函数  f(0)-srqt(f(0)^2+2(f(1)-f(0))*x)/(f(0)-f(1))
    Float t;
    if(f1!=f0){
        t=(f0-std::sqrt(std::max(f0*f0+2*(f1-f0)*u,(Float)0)))/(f0-f1);
    }else{
        //这种情况为f0=f1,因此f(x)=f0,然后积分，求逆函数巴拉巴拉，就能得到这个了
        t=u/f0;
    }

    int a =0;
    int b =1;
    Float Fhat;
    Float fhat;
    //进入牛顿-二分法
    while(true){
        //首先判断t是否在二分法允许的范围内
        //不在的话，更新t值为当前二分区间的中心点
        if(t<a||t>b){
            t=(a+b)*0.5f;
        }
        //计算相应的t的函数值
        //这里是相应的CatmullRom样条的Horner格式
        //这里直接复制于PBRT
        //其实就是换了下函数的排列
        Fhat = t * (f0 + t * (0.5f * d0 + t * ((1.0f/3.0f) * (-2 * d0 - d1) +f1 - f0 + t * (0.25f * (d0 + d1) + 0.5f * (f0 - f1)))));
        fhat = f0 + t * (d0 + t * (-2 * d0 - d1 + 3 * (f1 - f0) + t * (d0 + d1 + 2 * (f0 - f1))));

        //判断是否接近实际的root值
        //牛顿法的话要判断函数值是否接近0
        //二分法的话，比较二分边界是否足够接近
        if( std::abs(Fhat-u)<1e-6f||(b-a)<1e-6f){
            break;
        }
        //到这里的话，说明还需要迭代，提高进度
        
        if(Fhat<u){
            //说明[a,t]不可能包含解,更新a
            a=t;
        }else {
            //说明[t,b]不可能包含解,更新b
            b=t;
        }
        //运用牛顿法
        t=t-(Fhat-u)/fhat;
       // LInfo<<"t"<<t<<" alpha"<<alpha<<" u"<<u<<" d0"<<d0<<" d1"<<d1<<" a"<<a<<" b"<<b<<" w0"<<weights[0]<<" w1"<<weights[1]<<" w2"<<weights[2]<<" w3"<<weights[3];
    }

    if(fval){
        (*fval)=fhat;
    }

    if(pdf){
        (*pdf)=fhat/maximum;
    }

    //LInfo<<"--------------------------------------------------------------------";
    //从[0-1]映射回原来的空间
    return x0+t*w;
}