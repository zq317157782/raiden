#include "curve.h"
#include "paramset.h"
std::vector<std::shared_ptr<Shape>> CreateCurves(const Transform *o2w, const Transform *w2o, bool reverseOrientation,
    const Point3f *c, Float w0, Float w1, CurveType type,int splitDepth){
        std::shared_ptr<CurveCommon> common=std::make_shared<CurveCommon>(c,w0,w1,type);
        int segmentNum=1<<splitDepth;

        std::vector<std::shared_ptr<Shape>> curves;
        curves.reserve(segmentNum);

        for(int i=0;i<segmentNum;++i){
            Float u0=i/(Float)segmentNum;
            Float u1=(i+1)/(Float)segmentNum;
            curves.push_back(std::make_shared<Curve>(o2w,w2o,reverseOrientation,common,u0,u1));
        }
        return curves;
    }


    std::vector<std::shared_ptr<Shape>> CreateCurveShape(const Transform *o2w,
                                                     const Transform *w2o,
                                                     bool reverseOrientation,
                                                     const ParamSet &params){
            int np=0;
            const Point3f* cp=params.FindPoint3f("P",&np);//所得所有的控制点
            int splitDepth=params.FindOneInt("splitdepth",3);//获得每个线段的分割深度 n=1<<splitDepth 
            int degree= params.FindOneInt("degree",3);
            if(((np - 1 - degree) % degree) != 0){
                LError<<"Invalid number of control points "<<np<<": for the degree"<<degree<<"Bezier basis "<<degree+1<<" n*"<<degree<<"are required, for n >= 0.";
            }

            CurveType type;
            std::string curveType=params.FindOneString("type","flat");
            if(curveType=="flat"){
                type=CurveType::FLAT;
            }

            Float width=params.FindOneFloat("width",1);
            Float width0=params.FindOneFloat("width0",width);
            Float width1=params.FindOneFloat("width1",width);



            int segmentNum=(np-1)/degree;
            const Point3f* cpBase=cp;
            std::vector<std::shared_ptr<Shape>> curves;

            for(int i=0;i<segmentNum;++i){
                Point3f segCP[4];
                if(degree==2){
                    segCP[0]=cpBase[0];
                    segCP[1]=Lerp(2.0f/3.0f,cpBase[0],cpBase[1]);
                    segCP[2]=Lerp(1.0f/3.0f,cpBase[1],cpBase[2]);
                    segCP[3]=cpBase[2];
                }
                else{
                    for(int j=0;j<4;++j){
                        segCP[j]=cpBase[j];
                    }
                }
                cpBase+=degree;

                auto c=CreateCurves(o2w,w2o,reverseOrientation,segCP,width0,width1,type,splitDepth);
                curves.insert(curves.end(), c.begin(), c.end());
            }
            return curves;
    }