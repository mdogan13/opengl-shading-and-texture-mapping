//
//  sphere.h
//  COMP410_HW
//
//  Created by Murat Doğan on 9.05.2019.
//  Copyright © 2019 Murat Doğan. All rights reserved.
//

#ifndef sphere_h

#define sphere_h
typedef Angel::vec4  color4;
typedef Angel::vec4  point4;
//Variables for the sphere------------------------------------------------------------------------
const int NumTriangles        = 1024;
const int NumVerticesSphere   = 3 * NumTriangles;
point4 pointsSp[NumVerticesSphere];
vec3   normals[NumVerticesSphere];
color4 colorSp[NumVerticesSphere];
vec2    tex_coords[NumVerticesSphere];
int IndexSphere = 0;
double sphereScale = 1;

// RGBA olors
color4 vertex_colors[8] = {
    color4( 0.0, 0.0, 0.0, 1.0 ),  // black
    color4( 1.0, 0.0, 0.0, 1.0 ),  // red
    color4( 1.0, 1.0, 0.0, 1.0 ),  // yellow
    color4( 0.0, 1.0, 0.0, 1.0 ),  // green
    color4( 0.0, 0.0, 1.0, 1.0 ),  // blue
    color4( 1.0, 0.0, 1.0, 1.0 ),  // magenta
    color4( 1.0, 1.0, 1.0, 1.0 ),  // white
    color4( 0.0, 1.0, 1.0, 1.0 )   // cyan
};

//------------------------------------------------------------------------------------------------
//Functions for the sphere-------------------------------------------------------------------------

void triangle( const point4& a, const point4& b, const point4& c ,int colorSelection){
    double u, v;//texture coordinates
    
    //populates points and colors for 1 triangle on the sphere
    vec3  normal = normalize( cross(b - a, c - b) );

    
    normals[IndexSphere] = normal;
    pointsSp[IndexSphere] = a;
    //calculating texture coordinates
    v = (0.5 - asin(a.y)/M_PI) ;
    u = (0.5*(atan2(a.z,a.x)/M_PI + 1));
    tex_coords[IndexSphere] = vec2(u, v);
    IndexSphere++;
    
 
    normals[IndexSphere] = normal;
    pointsSp[IndexSphere] = b;
    v = (0.5 - asin(b.y)/M_PI) ;
    u = (0.5*(atan2(b.z,b.x)/M_PI + 1));
    tex_coords[IndexSphere] = vec2(u, v);
    IndexSphere++;
   
    normals[IndexSphere] = normal;
    pointsSp[IndexSphere] = c;
    v = (0.5 - asin(c.y)/M_PI) ;
    u = (0.5*(atan2(c.z,c.x)/M_PI + 1));
    tex_coords[IndexSphere] = vec2(u, v);
    IndexSphere++;
}

point4 unit( const point4& p ){
    
    float len = p.x*p.x + p.y*p.y + p.z*p.z;
    point4 t;
    
    if ( len > DivideByZeroTolerance ) {
        t = p / sqrt(len);
        t.w = sphereScale;
    }
    
    return t;
}

void divide_triangle( const point4& a, const point4& b, const point4& c, int count ,int colorSelection){
    //recursive algorithm for vertex generation, runs -count- times.
    
    if ( count > 0 ) {
        //keeps dividing triangles into smaller triangles
        point4 v1x = unit( a + b );
        point4 v2x = unit( a + c );
        point4 v3x = unit( b + c );
        divide_triangle(  a, v1x, v2x, count - 1 , colorSelection);
        divide_triangle(  c, v2x, v3x, count - 1 ,colorSelection);
        divide_triangle(  b, v3x, v1x, count - 1 ,colorSelection);
        divide_triangle( v1x, v3x, v2x, count - 1 ,colorSelection);
        
    }else{
        triangle( a, b, c, colorSelection );
    }
}

void drawSphere(int count, int colorSelection){
    //draws sphere using divide_triangle function
    
    point4 v[4] = {
        
        vec4( 0.0, 0.0, 1.0, 100.0 ),
        vec4( 0.0, 0.942809, -0.333333, 100.0 ),
        vec4( -0.816497, -0.471405, -0.333333, 100.0 ),
        vec4( 0.816497, -0.471405, -0.333333, 100.0 )
    };
    
    divide_triangle( v[0], v[1], v[2], count ,colorSelection);
    divide_triangle( v[3], v[2], v[1], count ,colorSelection);
    divide_triangle( v[0], v[3], v[1], count ,colorSelection);
    divide_triangle( v[0], v[2], v[3], count ,colorSelection);
}
//------------------------------------------------------------------------------------------------

#endif /* sphere_h */
