/*
 * texture.h
 *
 *  Created on: 2016年12月29日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_TEXTURE_H_
#define SRC_CORE_TEXTURE_H_
#include "raiden.h"
#include "interaction.h"
//二维纹理映射
class TextureMapping2D {
  public:
    virtual ~TextureMapping2D();
    //返回相应的纹理坐标以及Texture(s,t)相对于Screen(x,y)的偏导
    virtual Point2f Map(const SurfaceInteraction &si, Vector2f *dstdx,
                        Vector2f *dstdy) const = 0;
};


#endif /* SRC_CORE_TEXTURE_H_ */
