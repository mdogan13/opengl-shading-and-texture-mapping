//COMP410 Assignment 3
//
//Murat Doğan
//27624

#include <unistd.h>
#include "Angel.h"
#include "sphere.h"

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

//Function signatures----------------------------------------------------------------------------
void resetAnimation();
void reshape( int width, int height );
void paintShape(int colorSelection);
//-----------------------------------------------------------------------------------------------
//Variables for the program-----------------------------------------------------------------------
GLuint program;
vec3 displacement(-1,0.4,0.0);
vec3 velocity(-0.01,0.000001,0.0);
vec3 accelaration(-0.00000001,0.0001,0.0);


enum { Gouraud = 0, Phong = 1};
enum { FixedLight = 0,MovingLight = 1};
enum { Metal = 0,Plastic = 1};

int ambient=1; //flag for toggling
int diffuse=1; //flag for toggling
int specular=1; //flag for toggling
color4 light_diffuse(1.0,1.0, 1.0, 1.0 );
point4 light_position(0.0, 1.0, 1.0, 0.0 );
int shadingType=1; // default value is phong
int lightsrc=0; // default value is fixed
int materialType=0; // default value is metal
int firstLoad=1;
int textureSelection =0; //0 for earth, 1 for basketball
GLuint textures[2];
 
GLubyte image[2048][1024][3]; //earth.ppm
GLubyte image2[512][256][3]; //basketball.ppm

bool textureFlag = false; //enable/disable texture mapping
GLuint  TextureFlag; // texture flag uniform location

bool lightFlag = false; //enable/disable movement of light source
GLuint  LightFlag; // light flag uniform location

// Array of rotation angles (in degrees) for each coordinate axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int      Axis = Xaxis;
GLfloat  Theta[NumAxes] = { 0.0, 0.0, 0.0 };

// Model-view and projection matrices uniform location
GLuint  ModelView, Projection;

//----------------------------------------------------------------------------
void resetAnimation(){
    //this is called when user presses i,
    //it initializes the object's position, velocity and accelaration vectors.
    displacement.x=-1.0;
    displacement.y=0.4;
    
    velocity.x=-0.01;
    velocity.y=0.0000001;
    
    accelaration.x=-0.00000001;
    accelaration.y=0.0001;
    
}

// OpenGL initialization
void
init(){
    
    if(firstLoad)drawSphere(4, 0);
    firstLoad=0;
    
    
    // Initialize texture objects
    glGenTextures( 2, textures );
    
    //settings for earth.ppm
    glBindTexture( GL_TEXTURE_2D, textures[0] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 2048, 1024, 0,GL_RGB, GL_UNSIGNED_BYTE, image );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    
    //settings for basketball.ppm
    glBindTexture( GL_TEXTURE_2D, textures[1] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 512, 256, 0,GL_RGB, GL_UNSIGNED_BYTE, image2 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
     
    
    glBindTexture( GL_TEXTURE_2D, textures[textureSelection] ); //set current texture
    
    
    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
    
    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(pointsSp) + sizeof(normals)+sizeof(tex_coords),NULL, GL_STATIC_DRAW );
    
    
    GLintptr offset = 0;
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(pointsSp), pointsSp );
     offset += sizeof(pointsSp);
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(pointsSp),sizeof(normals), normals );
    offset += sizeof(normals);
    glBufferSubData( GL_ARRAY_BUFFER, offset, sizeof(tex_coords), tex_coords );
    
    
    GLuint program;
    // Load shaders and use the resulting shader program
    if(shadingType==Gouraud){
        program = InitShader( "vshader_gouraud.glsl", "fshader_gouraud.glsl" );
    }else{
        //phong
        program = InitShader( "vshader.glsl", "fshader.glsl" );
        
    }
    
    glUseProgram( program );
    
    // set up vertex arrays
    offset = 0;
    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(0) );
    offset += sizeof(pointsSp);
    GLuint vNormal = glGetAttribLocation( program, "vNormal" );
    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(sizeof(pointsSp)) );
     offset += sizeof(normals);
    GLuint vTexCoord = glGetAttribLocation( program, "vTexCoord" );
    glEnableVertexAttribArray( vTexCoord );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(offset) );
    
    // Initialize shader lighting parameters
    color4 light_ambient(1.0, 1.0, 1.0, 1.0 );
    color4 light_specular(1.0, 1.0, 1.0, 1.0 );
    
    color4 material_ambient;
    color4 material_diffuse;
    color4 material_specular;
    float  material_shininess;
    
    //sets material properties
    if(materialType==Metal){
        
        material_ambient.x=0.23125;
        material_ambient.y=0.23125;
        material_ambient.z=0.23125;
        material_ambient.w=1.0;
        material_diffuse.x=0.2775;
        material_diffuse.y=0.2775;
        material_diffuse.z=0.2775;
        material_diffuse.w=1.0;
        material_specular.x=0.773911;
        material_specular.y=0.773911;
        material_specular.z=0.773911;
        material_specular.w=1.0;
        
        material_shininess = 89.6;
    }else{
        //Plastic
    
        material_ambient.x=0.0;
        material_ambient.y=0.0;
        material_ambient.z=0.0;
        material_ambient.w=1.0;
        material_diffuse.x=0.55;
        material_diffuse.y=0.55;
        material_diffuse.z=0.55;
        material_diffuse.w=1.0;
        material_specular.x=0.0;
        material_specular.y=0.0;
        material_specular.z=0.0;
        material_specular.w=1.0;
        
        material_shininess = 32 ;
    }
    
    
    
    color4 ambient_product = light_ambient * material_ambient;
    color4 diffuse_product = light_diffuse * material_diffuse;
    color4 specular_product = light_specular * material_specular;
    
    if(ambient)glUniform4fv( glGetUniformLocation(program, "AmbientProduct"),1, ambient_product );
    if(diffuse)glUniform4fv( glGetUniformLocation(program, "DiffuseProduct"),1, diffuse_product );
    if(specular) glUniform4fv( glGetUniformLocation(program, "SpecularProduct"),1, specular_product );
    
    glUniform4fv( glGetUniformLocation(program, "LightPosition"),1, light_position );
    
    glUniform1f( glGetUniformLocation(program, "Shininess"),
                material_shininess );
    
   
    
    // Retrieve transformation uniform variable locations
    ModelView = glGetUniformLocation( program, "ModelView" );
    Projection = glGetUniformLocation( program, "Projection" );
    
    LightFlag = glGetUniformLocation(program, "LightFlag");
    TextureFlag = glGetUniformLocation(program, "TextureFlag");

    glUniform1i( TextureFlag, textureFlag );
    glUniform1i( LightFlag, lightFlag );
    
    
    glEnable(GL_CULL_FACE);
    glEnable( GL_DEPTH_TEST );
    
    glClearColor( 1.0, 1.0, 1.0, 1.0 );
    reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
}

//----------------------------------------------------------------------------
void paintShape(int colorSelection){
    //changes the light color so that to object appears in a different color
    light_diffuse.x=vertex_colors[colorSelection].x;
    light_diffuse.y=vertex_colors[colorSelection].y;
    light_diffuse.z=vertex_colors[colorSelection].z;
    init();
}
void
display( void )
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //  Generate tha model-view matrixn
    
    const vec3 viewer_pos( 0.0, 0.0, 3.0 );
    
    mat4  model_view = (Translate( -viewer_pos ) *Translate( displacement ) *Scale(0.2,0.2,0.2)*
                        RotateX( Theta[Xaxis] ) *
                        RotateY( Theta[Yaxis] ) *
                        RotateZ( Theta[Zaxis] ) );
    
    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view );
    
    //Handles the animation by manipulating displacement,
    //velocity and accelaration vectors of the object.
    
    if(displacement.y <= -1 ){
        // If the object hits the bottom border
        if(velocity.x<=-0.0100031) velocity.x =0;
        velocity = vec3 (velocity.x,-velocity.y + 0.01,0.0);
    }
    
    if(displacement.y >= 1 ){
        // If the object hits the bottom top border
        velocity = vec3 (velocity.x, -velocity.y,0.0);
    }
    
    if(displacement.y <-1){
        // Prevents the object from falling when vertical speed is very small
        displacement.y = -1;
    }
    
    if(displacement.x >=0.8){
        // When the object hits the right border,
        // sets its horizontal displacement to left border
        // so it looks like the object is in another scene.
        
        displacement.x=-1;
        
    }
    
    accelaration.y = 0.001;
    
    if(!(velocity.x==0)){
        // Update velocity and displacement
        // as long as the horizontal velocity is not zero
        // (Helps animation to stop and reduces the CPU usage) .
        velocity.operator+=(accelaration);
        displacement.operator-=(velocity);
    }
    
    Theta[Yaxis]+=2;
    
    
   
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    
    glutSwapBuffers();
}

//----------------------------------------------------------------------------


void
idle( void )
{
    
    if ( Theta[Axis] > 360.0 ) {
        Theta[Axis] -= 360.0;
    }
    
    glutPostRedisplay();
}

//----------------------------------------------------------------------------

void
keyboard( unsigned char key, int x, int y ){
    switch( key ) {
        case 033: // Escape Key
        case 'q': case 'Q':
            exit( EXIT_SUCCESS );
            break;
        case 'i': case 'I':
            resetAnimation();
            break;
        case 't': case 'T':
            //Changes current texture
            if(textureSelection){
                textureSelection=0;
                init();
            }else{
                textureSelection=1;
                init();
            }
            break;
        case 'h':case 'H':
            std::cout << "------------------------------" << std::endl;
            std::cout << "------HELP------" << std::endl;
            std::cout << "------------------------------" << std::endl;
            std::cout << "Right click to view the menu." << std::endl;
            std::cout << "I\tInitialize(reset) the pose" << std::endl;
            std::cout << "T\tToggle texture when the drawing mode is  Texture' " << std::endl;
            std::cout << "Q\tExit the program" << std::endl;
            break;
    }
}

//----------------------------------------------------------------------------

void
reshape( int width, int height )
{
    if (( width != 700 ) || ( height != 700 ))
    {
        glutReshapeWindow(width, width);
        width = 700;
        height = 700;
    }
    glViewport( 1, 1, width, height );
    
    GLfloat aspect = GLfloat(width)/height;
    mat4  projection = Perspective( 45.0, aspect, 0.5, 6.0 );
    
    glUniformMatrix4fv( Projection, 1, GL_TRUE, projection );
}

//----------------------------------------------------------------------------
void drawingModeMenu(int id){
    //Handles wireframe/solid/texture drawing mode changes
    switch(id) {
        case 1:
            textureFlag=false;
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            init();
            break;
        case 2:
            textureFlag=false;
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            init();
            break;
        case 3:
            //texture mapping
            textureFlag=true;
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            init();
            break;
    }
    glutPostRedisplay();
}
void shadingMenu(int id){
    //Handles shading mode changes
    switch(id) {
        case 1:
            //guro
            shadingType=Gouraud;
            init();
            
            break;
        case 2:
            //phong
            shadingType=Phong;
            init();
            
            break;
    
            
    }
    glutPostRedisplay();
}
void componentMenu(int id){
    //Handles component toggling
    switch(id) {
        case 1:
            //turn off ambient
            if(ambient){
                ambient=0;
            }else{
                ambient=1;
            }
            init();
            
            break;
        case 2:
            //diffuse
            if(diffuse){
                diffuse=0;
            }else{
                diffuse=1;
            }
            init();
            
            break;
        case 3:
            //specular
            if(specular){
                specular=0;
            }else{
                specular=1;
            }
            init();
            break;
            
    }
    glutPostRedisplay();
}
void lightsourceMenu(int id){
    //Handles light source movement
    switch(id) {
        case 1:
            //fixed
            lightFlag=false;
            init();
            break;
        case 2:
            //moving
            lightFlag=true;
            init();
            break;
            
            
    }
    glutPostRedisplay();
}
void materialMenu(int id){
    //Handles material change
   
    switch(id) {
        case 1:
            //metal
            materialType=Metal;
            init();
            break;
        case 2:
            //plastic
            materialType=Plastic;
            init();
            break;
            
            
    }
    glutPostRedisplay();
}
void colorMenu(int id){
    //Handles color changes
    switch(id) {
        case 0:
            paintShape(0);
            break;
        case 1:
            paintShape(1);
            break;
        case 2:
            paintShape(2);
            break;
        case 3:
            paintShape(3);
            break;
        case 4:
            paintShape(4);
            break;
        case 5:
            paintShape(5);
            break;
        case 6:
            paintShape(6);
            break;
        case 7:
            paintShape(7);
            break;
            
    }
    glutPostRedisplay();
}
void createMenus(){
  
    int drawingModeSubMenu = glutCreateMenu(drawingModeMenu);
    glutAddMenuEntry("Wireframe", 1);
    glutAddMenuEntry("Shading", 2);
    glutAddMenuEntry("Texture", 3);
    int shadingSubMenu = glutCreateMenu(shadingMenu);
    glutAddMenuEntry("Gouraud", 1);
    glutAddMenuEntry("Phong", 2);
    int componentSubMenu = glutCreateMenu(componentMenu);
    glutAddMenuEntry("Toggle ambient", 1);
    glutAddMenuEntry("Toggle diffuse", 2);
    glutAddMenuEntry("Toggle specular", 3);
    int lightsourceSubMenu = glutCreateMenu(lightsourceMenu);
    glutAddMenuEntry("Fixed", 1);
    glutAddMenuEntry("Moving", 2);
    int materialSubMenu = glutCreateMenu(materialMenu);
    glutAddMenuEntry("Metal", 1);
    glutAddMenuEntry("Plastic", 2);
    int colorSubMenu = glutCreateMenu(colorMenu);
    glutAddMenuEntry("Black", 0);
    glutAddMenuEntry("Red", 1);
    glutAddMenuEntry("Yellow", 2);
    glutAddMenuEntry("Green", 3);
    glutAddMenuEntry("Blue", 4);
    glutAddMenuEntry("Magenta", 5);
    glutAddMenuEntry("White", 6);
    glutAddMenuEntry("Cyan", 7);
    glutCreateMenu(NULL);
    glutAddSubMenu("Components", componentSubMenu);
    glutAddSubMenu("Light Source", lightsourceSubMenu);
    glutAddSubMenu("Material Selection", materialSubMenu);
    glutAddSubMenu("Drawing Mode", drawingModeSubMenu);
    glutAddSubMenu("Shading", shadingSubMenu);
    glutAddSubMenu("Color", colorSubMenu);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

bool readPPMFile(char const * filename){
    //reads the ppm file and populates the corresponding image array.
    
    int nameFlag = strcmp("earth.ppm", filename);
    
    std::string workingDirectory;
    size_t size=300;
    char *path=NULL;
    path=getcwd(path,size);
    workingDirectory=path;
    std::string fileName(filename);
  
    std::string filepath = workingDirectory+ "/" + fileName;
  
    const char* pathAsPointer = filepath.c_str();
    int n,m,k, nm;
    char c;
    char b[100];
    int red ,green, blue;
    FILE * fd = fopen(pathAsPointer, "r"); // openning to read
    
    if( fd == NULL ){
        std::cout << "File cannot be opened!" << std::endl;
        return false;
    }
   
    fscanf(fd,"%[^\n] ",b);

    fscanf(fd, "%c",&c);
    while(c == '#') {
        fscanf(fd, "%[^\n] ", b);
        printf("%s\n",b);
        fscanf(fd, "%c",&c);
    }
    ungetc(c,fd);
    fscanf(fd, "%d %d %d", &n, &m, &k);
    //printf("%d rows %d columns max value= %d\n",n,m,k);
    nm = n*m;
    for(int i = 0; i < n; i++){
        for(int j = 0; j < m; j++){
            fscanf(fd,"%d %d %d",&red, &green, &blue );
            if(nameFlag==0){
                image[n-i-1][j][0] = (GLubyte)red;
                image[n-i-1][j][1] = (GLubyte)green;
                image[n-i-1][j][2] = (GLubyte)blue;
            }else{
                image2[n-i-1][j][0] = (GLubyte)red;
                image2[n-i-1][j][1] = (GLubyte)green;
                image2[n-i-1][j][2] = (GLubyte)blue;
            }
          
        }
    }
    
    fclose(fd);
    return true;
}
int main( int argc, char **argv )
{
    char const *earth = "earth.ppm";
    char const *basketball = "basketball.ppm";
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE );
    glutInitWindowSize( 512,512 );
    glutCreateWindow( "Assignment 3" );
    
    glewExperimental = GL_TRUE;
    glewInit();
    
    init();
    createMenus();
    readPPMFile(earth);
    readPPMFile(basketball);
    std::cout << "------------------------------" << std::endl;
    std::cout << "-----------CONTROLS-----------" << std::endl;
    std::cout << "------------------------------" << std::endl;
    std::cout << "Right click to view the menu." << std::endl;
    std::cout << "I\tInitialize(reset) the pose" << std::endl;
    std::cout << "T\tToggle texture when the drawing mode is  'Texture' " << std::endl;
    std::cout << "H\tPrint this menu." << std::endl;
    std::cout << "Q\tExit the program" << std::endl;
    std::cout << "Note: Please test toggling components while using metal material(default) as the plastic material's ambient and specular components are zero. In addition, if a problem while reading the ppm files occurs, try replacing the 'workingDirectory' variable with the path to the ppm files." << std::endl;
    
  
    
    glutDisplayFunc( display );
    glutKeyboardFunc( keyboard );
    glutReshapeFunc( reshape );
    glutIdleFunc( idle );
    glutMainLoop();
    return 0;
}
