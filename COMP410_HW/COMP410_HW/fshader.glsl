// Per-fragment interpolated values from the vertex shader

//Since normals are fixed for a given face of the cube in this example, fN per-fragment interpolation yields fixed values. Per-fragment interpolation of fL and fV however gives smoothly varying values through faces.
#version 410

in  vec3 fN;
in  vec3 fL;
in  vec3 fV;
in  vec2 texCoord;
uniform int TextureFlag;
uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform mat4 ModelView;
uniform vec4 LightPosition;
uniform float Shininess;
out vec4 fcolor;
uniform sampler2D tex;

void main() 
{
    if (TextureFlag == 1){
        //sample a texture color from texture object
        fcolor = texture(tex, texCoord );
        
    }else{
        // Normalize the input lighting vectors
        vec3 N = normalize(fN);
        vec3 V = normalize(fV);
        vec3 L = normalize(fL);

        vec3 H = normalize( L + V );
        
        vec4 ambient = AmbientProduct;

        float Kd = max(dot(L, N), 0.0);
        vec4 diffuse = Kd*DiffuseProduct;
        
        float Ks = pow(max(dot(N, H), 0.0), Shininess);
        vec4 specular = Ks*SpecularProduct;

        // discard the specular highlight if the light's behind the vertex
        if( dot(L, N) < 0.0 ) {
            specular = vec4(0.0, 0.0, 0.0, 1.0);
        }

    fcolor = ambient + diffuse + specular;
        fcolor.a = 1.0;
    }
} 

