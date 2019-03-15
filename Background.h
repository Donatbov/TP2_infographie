#ifndef BACKGROUND_H
#define BACKGROUND_H
#include "Color.h"
#include "Ray.h"
#include "Image2D.h"

namespace rt {
    struct Background {
        virtual Color backgroundColor( const Ray& ray ) = 0;
    };
    struct MyBackground : public Background {
        Color backgroundColor( const Ray& ray ) override
        {
            Color result = Color();
            if(ray.direction[2] >= 0 && ray.direction[2] < 1 ){
                result += Color(1.0f, 1.0f, 1.0f) + ray.direction[2] * (Color(0.0f, 0.0f, 1.0f) - Color(1.0f, 1.0f, 1.0f));
            }
            else{
                Real x = -0.5f * ray.direction[ 0 ] / ray.direction[ 2 ];
                Real y = -0.5f * ray.direction[ 1 ] / ray.direction[ 2 ];
                Real d = sqrt( x*x + y*y );
                Real t = std::min( d, 30.0f ) / 30.0f;
                x -= floor( x );
                y -= floor( y );
                if ( ( ( x >= 0.5f ) && ( y >= 0.5f ) ) || ( ( x < 0.5f ) && ( y < 0.5f ) ) )
                  result += (1.0f - t)*Color( 0.2f, 0.2f, 0.2f ) + t * Color( 1.0f, 1.0f, 1.0f );
                else
                  result += (1.0f - t)*Color( 0.4f, 0.4f, 0.4f ) + t * Color( 1.0f, 1.0f, 1.0f );
            }

            return result;
        }
    };
}

#endif // BACKGROUND_H
