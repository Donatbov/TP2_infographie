/**
@file Renderer.h
@author JOL
*/
#pragma once
#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "Color.h"
#include "Image2D.h"
#include "Ray.h"
#include "Background.h"
#include "Scene.h"

/// Namespace RayTracer
namespace rt {

  inline void progressBar( std::ostream& output,
                           const double currentValue, const double maximumValue)
  {
    static const int PROGRESSBARWIDTH = 60;
    static int myProgressBarRotation = 0;
    static int myProgressBarCurrent = 0;
    // how wide you want the progress meter to be
    double fraction = currentValue /maximumValue;
    
    // part of the progressmeter that's already "full"
    int dotz = static_cast<int>(floor(fraction * PROGRESSBARWIDTH));
    if (dotz > PROGRESSBARWIDTH) dotz = PROGRESSBARWIDTH;
    
    // if the fullness hasn't changed skip display
    if (dotz == myProgressBarCurrent) return;
    myProgressBarCurrent = dotz;
    myProgressBarRotation++;
    
    // create the "meter"
    int ii=0;
    output << "[";
    // part  that's full already
    for ( ; ii < dotz;ii++) output<< "#";
    // remaining part (spaces)
    for ( ; ii < PROGRESSBARWIDTH;ii++) output<< " ";
    static const char* rotation_string = "|\\-/";
    myProgressBarRotation %= 4;
    output << "] " << rotation_string[myProgressBarRotation]
           << " " << (int)(fraction*100)<<"/100\r";
    output.flush();
  }

  /// This structure takes care of rendering a scene.
  struct Renderer {

    /// The scene to render
    Scene* ptrScene;
    /// The origin of the camera in space.
    Point3 myOrigin;
    /// (myOrigin, myOrigin+myDirUL) forms a ray going through the upper-left
    /// corner pixel of the viewport, i.e. pixel (0,0)
    Vector3 myDirUL;
    /// (myOrigin, myOrigin+myDirUR) forms a ray going through the upper-right
    /// corner pixel of the viewport, i.e. pixel (width,0)
    Vector3 myDirUR;
    /// (myOrigin, myOrigin+myDirLL) forms a ray going through the lower-left
    /// corner pixel of the viewport, i.e. pixel (0,height)
    Vector3 myDirLL;
    /// (myOrigin, myOrigin+myDirLR) forms a ray going through the lower-right
    /// corner pixel of the viewport, i.e. pixel (width,height)
    Vector3 myDirLR;

    int myWidth;
    int myHeight;

    Background* ptrBackground;

    Renderer() : ptrScene( 0 ) {}
    Renderer( Scene& scene ) : ptrScene( &scene ) {}
    void setScene( rt::Scene& aScene ) { ptrScene = &aScene; }

    void setViewBox( Point3 origin,
                     Vector3 dirUL, Vector3 dirUR, Vector3 dirLL, Vector3 dirLR )
    {
      myOrigin = origin;
      myDirUL = dirUL;
      myDirUR = dirUR;
      myDirLL = dirLL;
      myDirLR = dirLR;
    }

    void setResolution( int width, int height )
    {
      myWidth  = width;
      myHeight = height;
    }


    /// The main rendering routine
    void render( Image2D<Color>& image, int max_depth )
    {
      std::cout << "Rendering into image ... might take a while." << std::endl;
      image = Image2D<Color>( myWidth, myHeight );
      for ( int y = 0; y < myHeight; ++y )
        {
          Real    ty   = (Real) y / (Real)(myHeight-1);
          progressBar( std::cout, ty, 1.0 );
          Vector3 dirL = (1.0f - ty) * myDirUL + ty * myDirLL;
          Vector3 dirR = (1.0f - ty) * myDirUR + ty * myDirLR;
          dirL        /= dirL.norm();
          dirR        /= dirR.norm();
          for ( int x = 0; x < myWidth; ++x )
            {
              Real    tx   = (Real) x / (Real)(myWidth-1);
              Vector3 dir  = (1.0f - tx) * dirL + tx * dirR;
              Ray eye_ray  = Ray( myOrigin, dir, max_depth );
              Color result = trace( eye_ray );
              image.at( x, y ) = result.clamp();
            }
        }
      std::cout << "Done." << std::endl;
    }

    // Affiche les sources de lumières avant d'appeler la fonction qui
    // donne la couleur de fond.
    Color background( const Ray& ray )
    {
      Color result = Color( 0.0, 0.0, 0.0 );
      for ( auto* light : ptrScene->myLights )
        {
          Real cos_a = light->direction( ray.origin ).dot( ray.direction );
          if ( cos_a > 0.99f )
            {
              Real a = acos( cos_a ) * 360.0 / M_PI / 8.0;
              a = std::max( 1.0f - a, 0.0f );
              result += light->color( ray.origin ) * a * a;
            }
        }
      if ( ptrBackground != 0 ) result += ptrBackground->backgroundColor( ray );
      return result;
    }


    /// The rendering routine for one ray.
    /// @return the color for the given ray.
    Color trace( const Ray& ray )
    {
      assert( ptrScene != 0 );
      Color result;
      GraphicalObject* obj_i = 0; // pointer to intersected object
      Point3           p_i;       // point of intersection
      ptrBackground = new MyBackground();
      // Look for intersection in this direction.
      Real ri = ptrScene->rayIntersection( ray, obj_i, p_i );
      // Nothing was intersected
      if ( ri >= 0.0f ) return background( ray ); // background
      //if ( ri >= 0.0f ) return Color( 0.0, 0.0, 0.0 ); // some background color
      // Something was intersected
      //result = obj_i->getMaterial(p_i).ambient + obj_i->getMaterial(p_i).diffuse;   // rendu grosssier : on somme la couleur directe et la couleur diffuse de l'objet
      result = illumination(ray, obj_i, p_i);
      return result;
    }

    /// Calcule l'illumination de l'objet obj au point p, sachant que l'observateur est le rayon ray.
    Color illumination( const Ray& ray, GraphicalObject* obj, Point3 p ){
        Color result = Color( 0.0, 0.0, 0.0 );

        for(auto& l : ptrScene->myLights){    // Pour chaque source de lumiere

            // On calcule le coefficient de diffusion associé
            Real coeffDiff = l->direction(p).dot(obj->getNormal(p));
            if (coeffDiff < 0)
                coeffDiff = 0;
            result += coeffDiff*obj->getMaterial(p).diffuse*l->color(p);

            // ainsi que la couleur spéculaire associée
            Vector3 W = reflect(ray.direction, obj->getNormal(p));
            Real cosBeta = l->direction(p).dot(W);
            if(cosBeta >= 0){
                Real coeffSpec = powf(cosBeta, obj->getMaterial(p).shinyness);
                result += coeffSpec*obj->getMaterial(p).specular*l->color(p);
            }

            // et enfin les ombres
            result += shadow(Ray(p,l->direction(p)),l->color(p))*l->color(p);
        }
        result += obj->getMaterial(p).ambient;    // on ajoute la couleur ambiante

        return result;
    }

    /// Calcule le vecteur réfléchi à W selon la normale N.
    Vector3 reflect( const Vector3& W, Vector3 N ) const{
        return W -2* W.dot(N) * N;
    }

    /// Calcule la couleur de la lumière (donnée par light_color) dans la
    /// direction donnée par le rayon. Si aucun objet n'est traversé,
    /// retourne light_color, sinon si un des objets traversés est opaque,
    /// retourne du noir, et enfin si les objets traversés sont
    /// transparents, attenue la couleur.
    Color shadow( const Ray& ray, Color light_color ){
        Point3 p = ray.origin;
        Vector3 l = ray.direction;
        while(light_color.max() > 0.003f){
            //on déplace légèrement p vers l
            p += 0.001f * l;
            GraphicalObject* object = 0;
            Point3 p2;
            //Si intersection
            if (ptrScene->rayIntersection(ray, object, p2) <= 0.0f){
                Material m = object->getMaterial(p2);
                light_color = light_color * m.diffuse * m.coef_refraction;
                p = p2;
            }
            else{
                break;
            }
        }
        return light_color;
    }

  };



} // namespace rt

#endif // #define _RENDERER_H_
