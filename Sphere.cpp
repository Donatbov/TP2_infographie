/**
@file Sphere.cpp
*/
#include <cmath>
#include "Sphere.h"

void
rt::Sphere::draw( Viewer& /* viewer */ )
{
  Material m = material;
  // Taking care of south pole
  glBegin( GL_TRIANGLE_FAN );
  glColor4fv( m.ambient );
  glMaterialfv(GL_FRONT, GL_DIFFUSE, m.diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, m.specular);
  glMaterialf(GL_FRONT, GL_SHININESS, m.shinyness );
  Point3 south_pole = localize( -90, 0 );
  glNormal3fv( getNormal( south_pole ) );
  glVertex3fv( south_pole );
  for ( int x = 0; x <= NLON; ++x )
    {
      Point3 p = localize( -90 + 180/NLAT, x * 360 / NLON );
      glNormal3fv( getNormal( p ) );
      glVertex3fv( p );
    }
  glEnd();
  // Taking care of in-between poles
  for ( int y = 1; y < NLAT - 1; ++y )
    {
      glBegin( GL_QUAD_STRIP);
      glColor4fv( m.ambient );
      glMaterialfv(GL_FRONT, GL_DIFFUSE, m.diffuse);
      glMaterialfv(GL_FRONT, GL_SPECULAR, m.specular);
      glMaterialf(GL_FRONT, GL_SHININESS, m.shinyness );
      for ( int x = 0; x <= NLON; ++x )
        {
          Point3 p = localize( -90 + y*180/NLAT,     x * 360 / NLON );
          Point3 q = localize( -90 + (y+1)*180/NLAT, x * 360 / NLON );
          glNormal3fv( getNormal( p ) );
          glVertex3fv( p );
          glNormal3fv( getNormal( q ) );
          glVertex3fv( q );
        }
      glEnd();
    }
  // Taking care of north pole
  glBegin( GL_TRIANGLE_FAN );
  glColor4fv( m.ambient );
  glMaterialfv(GL_FRONT, GL_DIFFUSE, m.diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, m.specular);
  glMaterialf(GL_FRONT, GL_SHININESS, m.shinyness );
  Point3 north_pole = localize( 90, 0 );
  glNormal3fv( getNormal( north_pole ) );
  glVertex3fv( north_pole );
  for ( int x = NLON; x >= 0; --x )
    {
      Point3 p = localize( -90 + (NLAT-1)*180/NLAT, x * 360 / NLON );
      glNormal3fv( getNormal( p ) );
      glVertex3fv( p );
    }
  glEnd();
}

rt::Point3
rt::Sphere::localize( Real latitude, Real longitude ) const
{
  static const Real conv_deg_rad = 2.0 * M_PI / 360.0;
  latitude  *= conv_deg_rad;
  longitude *= conv_deg_rad;
  return center 
    + radius * Point3( cos( longitude ) * cos( latitude ),
                       sin( longitude ) * cos( latitude ),
                       sin( latitude ) );
}

rt::Vector3
rt::Sphere::getNormal( Point3 p )
{
  Vector3 u = p - center;
  Real   l2 = u.dot( u );
  if ( l2 != 0.0 ) u /= sqrt( l2 );
  return u;
}

rt::Material
rt::Sphere::getMaterial( Point3 /* p */ )
{
  return material; // the material is constant along the sphere.
}

rt::Real
rt::Sphere::rayIntersection( const Ray& ray, Point3& p )
{
    // On teste si le rayon lumineux intersecte la sphere (aka distance centre <= rayon)
    Vector3 CenterClosestPoint = (ray.origin-this->center) - (ray.origin-this->center).dot(ray.direction)*ray.direction;
    Real distanceCentreAuCarre = CenterClosestPoint.dot(CenterClosestPoint);
    Real distanceBoule = distanceCentreAuCarre - this->radius * this->radius;

    if (distanceBoule <= 0){    // si on est dans la sphere, on calcule les points d'intersections
        Real delta  = 4*(ray.direction.dot(ray.origin-this->center) * ray.direction.dot(ray.origin-this->center)) - 4*((ray.origin-this->center).dot(ray.origin-this->center) - this->radius * this->radius);
        Real sol1 = (-2*ray.direction.dot(ray.origin - this->center) - (float)sqrt(delta))/2;
        Real sol2 = (-2*ray.direction.dot(ray.origin - this->center) + (float)sqrt(delta))/2;

        // On ne veut garder qu'un seul point d'intersection
        Real sol;
        if (sol1 >= 0 && sol2 >= 0){    // cas 1 : l'objet est devant le rayon
            // on garde la solution la plus proche
            sol = std::min(sol1, sol2);
        } else if(sol1 < 0 && sol2 <0){ // cas 2 : l'objet est derriere le rayon
            // il n'y a pas d'intersections
            distanceBoule = -distanceBoule;
            sol = 0; // le point le plus proche devient alors l'origine du rayon
        } else{                         // cas 3 : les 2 sol sont de signe différents, le rayon est dans la boule
            // on garde la solution positive
            sol = std::max(sol1, sol2);
        }
        // On construit le point d'intersection entre le rayon et l'objet
        p = ray.origin + sol * ray.direction;

    } else {    // si il 'y a pas d'intersection, on renvoie le point le plus proche de l'objet
        p = this->center - CenterClosestPoint;
    }


  return distanceBoule;
}
