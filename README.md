# Raiden (雷電[ライデン])
一个基于物理的渲染器,起始于学习PBRT_V3.
*项目的三个重要目标:*
* 重新实现PBRT_V3以此学习PBRT新的架构以及算法，并且完成PBRT的练习。
* 在此框架下继续渲染领域的研究。
* 尝试修改框架，以达到更加完善的设计。

-  基础几何,交点信息
    - [x] 基础的向量,法线,空间点,包围盒和射线结构和相关函数(Vector2,Vector3,Point2,Point3,Normal3,Bound2,Bound3,Ray,RayDifferential)
    - [x] 基础矩阵和Transform(Matrix4x4,Transform)
    - [ ] 插值的Trasform(AnimateTrasform)
      - [x] 四元数(Quaternion)
        - [x] Slerp 
    - [x] 交点信息(Interaction)
      - [x] 表面交点信息结构(SurfaceInteraction)
      - [x] 体素交点信息结构(MediumInteraction)
-  形状,曲面细分和浮点数误差分析
      - [x] Shape抽象类
      - [x] 球体的基础结构(Sphere)
      - [ ] 圆柱体的基础结构
      - [x] 三角面片的基础结构(Triangle,TriangleMesh)
      - [x] Disk的基础结构(Disk)
      - [x] 曲线的基础结构(Curve)
      - [ ] 曲面细分
    - [x] 浮点数误差
-  图元和加速结构
    - [x] 基础的图元抽象类(Primitive)
      - [x] 几何图元(GeomPrimitive)
      - [x] 加速结构抽象类(Aggregate)
        - [x] BVH(Mean,Middle,SAH)
          - [ ] 进阶的BVH
        - [ ] KDTree
        - [x] 单纯的迭代(Iteration)
        - [x] Grid
 - 频谱,SPD
   - [x] Spectrum(RGBSpectrum,SampleSpectrum)
