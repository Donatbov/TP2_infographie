#ifndef PERIODICPLANE_H
#define PERIODICPLANE_H

#include "GraphicalObject.h"

namespace rt {

    struct PeriodicPlane : public GraphicalObject // Vous devez dériver de GraphicalObject
    {
        Point3 c;
        Vector3 u,v;
        Material main_m, band_m;
        Real w;

        /// Creates a periodic infinite plane passing through \a c and
        /// tangent to \a u and \a v. Then \a w defines the width of the
        /// band around (0,0) and its period to put material \a band_m,
        /// otherwise \a main_m is used.
        PeriodicPlane( Point3 c, Vector3 u, Vector3 v,
                       Material main_m, Material band_m, Real w):
                       c(c), u(u), v(v), main_m(main_m), band_m(band_m), w(w)
        {}

        void coordinates( Point3 p, Real& x, Real& y ){
            //p = x*u + y*v;

        }

        void init(Viewer& v){}

        void draw(Viewer& viewer){
            glBegin(GL_TRIANGLES);



            glEnd();
        }

        Vector3 getNormal(Point3 p){
            return u.cross(v);
        }

        Material getMaterial(Point3 p){

        }

        Real rayIntersection(const Ray& ray, Point3& p){

        }
    };

}
#endif // PERIODICPLANE_H
