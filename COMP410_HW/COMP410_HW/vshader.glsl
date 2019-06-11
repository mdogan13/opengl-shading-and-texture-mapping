#version 410

in   vec4 vPosition;
in   vec3 vNormal;
in  vec2 vTexCoord;
// output values that will be interpretated per-fragment
out  vec3 fN;
out  vec3 fV;
out  vec3 fL;
out vec2 texCoord;
uniform mat4 ModelView;
uniform vec4 LightPosition;
uniform mat4 Projection;
uniform int LightFlag;

void main()
{
  
    // Transform vertex position into camera (eye) coordinates
    vec3 pos = (ModelView *vPosition).xyz;
    
    fN = (ModelView *vec4(vNormal, 0.0)).xyz; // normal direction in camera coordinates

    fV = -pos; //viewer direction in camera coordinates

    if(LightFlag==1){
        //moving light source
        fL = (ModelView*LightPosition).xyz; // light direction
    }else{
        //fixed light source
        fL = LightPosition.xyz;
    }
    
    
    
    if( LightPosition.w != 0.0 ) {
        fL = LightPosition.xyz - pos;  //directional light source
    }
    
    texCoord    = vTexCoord;
    gl_Position = Projection * ModelView * vPosition;
}
