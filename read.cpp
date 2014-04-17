#include <GL/glew.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <cmath>

#define GLM_FORCE_RADIANS
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#define PI 3.1415926
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform2.hpp"
//#include "glm/gtc/matrix_projection.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/vec3.hpp"
#include "glm/gtc/noise.hpp"
using namespace std;
using glm::vec3;

//glm::mat4 MVP=glm::mat4(1.0f);

int numfile=1;
int count;
int numparticles;
vec3 position;
double counter =0;

int filenumber;
int filenumbermax= 10;

GLuint vbo,particles;
GLuint shaderProgramID;
GLuint LocationTime,LocationMVP, countID, positionID;

float angle = (float)(PI / 2.0f);

GLuint initBuffers(int numfile);

//----------------------------------------------------------------------------------------------------------

GLuint	 texBufferID;	// We have to create a buffer to hold the image. However, it WON'T go in the vertex buffer
GLuint	 texCoordID;	// The ID of the "texCoord" variable in the shader
GLuint	 texID;			// The ID of the "texture" variable in the shader
GLubyte* imageData;		// This will contain the raw color information from the file

// Function which is used to compute image data and create a texture map
//=============================================================================================================
void loadBitmapFromFile(const char* filename, int* width, int* height, int* size, unsigned char** pixel_data) {
	FILE* fp = fopen(filename, "r");
	if (fp == NULL) {
		printf ("Couldn't open file... aborting\n");
	}
	short identifier = -1;
	fread(&identifier, 1, sizeof(short), fp); printf ("Identifer is: %c\n", identifier);
	int filesize = -1;
	fread(&filesize, 1, sizeof(int), fp); printf ("filesize is: %d\n", filesize);
	int reserved = -1;
	fread(&reserved, 1, sizeof(int), fp); printf ("reserved is: %d\n", reserved);
	int bitmap_offset = -1;
	fread(&bitmap_offset, 1, sizeof(int), fp); printf ("bitmap_offset is: %d\n", bitmap_offset);
	int bitmap_header_size = -1;
	fread(&bitmap_header_size, 1, sizeof(int), fp); printf ("bitmap_header_size is: %d\n", bitmap_header_size);
	int bitmap_width = -1;
	fread(&bitmap_width, 1, sizeof(int), fp); printf ("bitmap_width is: %d\n", bitmap_width);
	int bitmap_height = -1;
	fread(&bitmap_height, 1, sizeof(int), fp); printf ("bitmap_height is: %d\n", bitmap_height);
	short bitmap_planes = -1;
	fread(&bitmap_planes, 1, sizeof(short), fp); printf ("bitmap_planes is: %d\n", bitmap_planes);
	short bits_per_pixel= -1;
	fread(&bits_per_pixel, 1, sizeof(short), fp); printf ("bits_per_pixel is: %d\n", bits_per_pixel);
	int compression = -1;
	fread(&compression, 1, sizeof(int), fp); printf ("compression is: %d\n", compression);
	int bitmap_data_size = -1;
	fread(&bitmap_data_size, 1, sizeof(int), fp); printf ("bitmap_data_size is: %d\n", bitmap_data_size);
	int hresolution = -1;
	fread(&hresolution, 1, sizeof(int), fp); printf ("hresolution is: %d\n", hresolution);
	int vresolution = -1;
	fread(&vresolution, 1, sizeof(int), fp); printf ("vresolution is: %d\n", vresolution);
	int num_colors = -1;
	fread(&num_colors, 1, sizeof(int), fp); printf ("num_colors is: %d\n", num_colors);
	int num_important_colors = -1;
	fread(&num_important_colors, 1, sizeof(int), fp); printf ("num_important_colors is: %d\n", num_important_colors);
	
	// Jump to the data already!
	fseek (fp, bitmap_offset, SEEK_SET);
	unsigned char* data = new unsigned char[bitmap_data_size];
	// Read data in BGR format
	fread (data, sizeof(unsigned char), bitmap_data_size, fp);
	
	// Make pixel_data point to the pixels
	*pixel_data = data;
	*size = bitmap_data_size;
	*width = bitmap_width;
	*height = bitmap_height;
	fclose(fp);
}

//===========================================================================================================================

//==========================================================================================================================
#pragma region SHADER_FUNCTIONS
static char* readFile(const char* filename) {
	// Open the file
	FILE* fp = fopen (filename, "r");
	// Move the file pointer to the end of the file and determing the length
	fseek(fp, 0, SEEK_END);
	long file_length = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* contents = new char[file_length+1];
	// zero out memory
	for (int i = 0; i < file_length+1; i++) {
		contents[i] = 0;
	}
	// Here's the actual read
	fread (contents, 1, file_length, fp);
	// This is how you denote the end of a string in C
	contents[file_length+1] = '\0';
	fclose(fp);
	return contents;
}

bool compiledStatus(GLint shaderID){
	GLint compiled = 0;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compiled);
	if (compiled) {
		return true;
	}
	else {
		GLint logLength;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logLength);
		char* msgBuffer = new char[logLength];
		glGetShaderInfoLog(shaderID, logLength, NULL, msgBuffer);
		printf ("%s\n", msgBuffer);
		delete (msgBuffer);
		return false;
	}
}

GLuint makeVertexShader(const char* shaderSource) {
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource (vertexShaderID, 1, (const GLchar**)&shaderSource, NULL);
	glCompileShader(vertexShaderID);
	bool compiledCorrectly = compiledStatus(vertexShaderID);
	if (compiledCorrectly) {
		return vertexShaderID;
	}
	return -1;
}

GLuint makeFragmentShader(const char* shaderSource) {
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderID, 1, (const GLchar**)&shaderSource, NULL);
	glCompileShader(fragmentShaderID);
	bool compiledCorrectly = compiledStatus(fragmentShaderID);
	if (compiledCorrectly) {
		return fragmentShaderID;
	}
	return -1;
}

GLuint makeShaderProgram (GLuint vertexShaderID, GLuint fragmentShaderID) {
	GLuint shaderID = glCreateProgram();
	glAttachShader(shaderID, vertexShaderID);
	glAttachShader(shaderID, fragmentShaderID);
	glLinkProgram(shaderID);
	return shaderID;
}
#pragma endregion SHADER_FUNCTIONS

//=========================================================================================================================

/*void matrices(){

	glm::mat4 view = glm::lookAt(vec3(0.0f,0.0f,1.0f), vec3(0.0f,0.0f,-1.0f), vec3(0.0f,1.0f,0.0f));
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::rotate( model,90.0f,vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 projection = glm::perspective(45.0f, (float)800/600, 1.0f, 100.0f);
    glm::mat4 MVP = projection*view*model;

}*/



//-------------------------------------------------------------------------------------------------------------------
GLuint initBuffers(int numfile) {

	double** myarray=0;

	string st1="plume";
	string st2=".dat";
	string temp,str1;
	string line1,line2,line;

	stringstream fnum;
	fnum<<numfile;
	temp = fnum.str();
	str1 = st1+temp+st2;

	cout<<str1<<endl;
	ifstream pfile;
	pfile.open(str1.c_str());
	if(pfile.is_open())
	{
		getline(pfile,line1);
		getline(pfile,line2);
		count=0;
		while(!pfile.eof()) 
		{
		getline(pfile,line);
		
		count++;
		}
	}
	numparticles=count;
	pfile.close();

  

    myarray=new double*[numparticles];
	for(int i=0;i<numparticles;i++)
	{
		myarray[i]=new double[10];
	}


pfile.open(str1.c_str());
	if(pfile.is_open())
	{
		getline(pfile,line1);
		getline(pfile,line2);
		for(int i=0;i<numparticles;i++)
		{
			for(int j=0;j<10;j++)
			{
				pfile>>myarray[i][j];
				
			}
		}
	}
	pfile.close();

	float *posarray = new float[numparticles*3];
	float *text_coord = new float[numparticles*2];

for(int i=0;i<numparticles;i++)
    {
    
    position.x = myarray[i][0];
	position.y = myarray[i][1];
	position.z = myarray[i][2];

	vec3 normvec = glm::normalize(position);
	/*glm::mat4 myMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::vec4 myVector(normvec1, 1.0f);
	glm::vec4 normvec = myMatrix * myVector;*/
	
	posarray[3*i]=200*normvec.x;
	posarray[3*i+1]=200*normvec.y;
	posarray[3*i+2]=normvec.z;

	cout<<position.z<<endl;

	//text_coord[2*i]=0.3 + 10*abs(normvec.x);
	//text_coord[2*i+1]=0.3 + 10*abs(normvec.y);

	text_coord[2*i]=0.3 + glm::perlin(glm::vec2(0.3, abs(normvec.x)));
	text_coord[2*i+1]=0.3 + glm::perlin(glm::vec2(0.3, abs(normvec.y)));


    }

   //cout<<posarray[2]<<endl;
   glGenBuffers(1, &vbo); 
   int size1 = numparticles * 3 * sizeof(float);
   int size2 = numparticles * 2 * sizeof(float);
   glBindBuffer(GL_ARRAY_BUFFER, vbo);
   glBufferData(GL_ARRAY_BUFFER, (size1 + size2), NULL, GL_STATIC_DRAW);



   glBindBuffer(GL_ARRAY_BUFFER,vbo);
   glBufferSubData(GL_ARRAY_BUFFER, 0, size1, posarray);
   glBufferSubData(GL_ARRAY_BUFFER, size1, size2, text_coord);

    delete[] myarray;
    delete[] posarray;
    delete[] text_coord;
    glGenVertexArrays(1, &particles);
	glBindVertexArray(particles);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
   // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    return particles;
}

//-----------------------------------------------------------------------------------------------------------

void idler(){
	counter =counter+0.01;
	//counter= fmod(counter,100000);
	
	if ((counter>10000.0) && (filenumber<filenumbermax)){
		
		glDeleteVertexArrays(1,&particles);
        filenumber++;
		particles=initBuffers(filenumber);
		glutPostRedisplay();
		counter =0;
		cout<<filenumber<<endl;	
	}
if (filenumber == filenumbermax)
	{
		filenumber=1;
	}

}



//--------------------------------------------------------------------------------------------------------------
void render (){

	int PARTICLE_COUNT = numparticles;

   glUseProgram(shaderProgramID);
    glEnable (GL_PROGRAM_POINT_SIZE);
    glEnable( GL_POINT_SPRITE );
    glEnable( GL_POINT_SMOOTH );
   glEnable (GL_BLEND);

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    GLfloat fogColor[] = {0.2f, 0.5f, 0.2f, 1};
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, 10.0f);
    glFogf(GL_FOG_END, 20.0f);

   glm::mat4 view = glm::lookAt(vec3(0.0f,0.0f,2.0f), vec3(0.0f,0.0f,0.0f), vec3(0.0f,1.0f,0.0f));
	glm::mat4 model = glm::mat4(1.0f);
	//model = glm::translate(vec3(0.0,0.0,1.0));
	//model = glm::rotate( model,(float)PI/2,vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 projection = glm::perspective(45.0f, (float)800/600, 1.0f, 1000.0f);
    glm::mat4 MVP = projection*view*model;
    glUniform1f (countID, counter);
   glUniformMatrix4fv(LocationMVP, 1, GL_FALSE, glm::value_ptr(MVP)); 
   //glUniform3fv(LocationGravity, 1, Gravity);
   //glUniform1f (LocationLifetime, (GLfloat)Lifetime);
//--------------------------------------------------------------------------------------------------------------
   glEnableVertexAttribArray(texCoordID);
	int textureCoordOffset = 2*numparticles*sizeof(GLfloat);
	glVertexAttribPointer(texCoordID, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(textureCoordOffset));

	GLfloat text_color[] = {1.0f,0.0f,0.0f,0.0f};
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// Set the preferences
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, text_color);
	glActiveTexture(GL_TEXTURE0);				// Turn on texture unit 
	glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	

	glUniform1i(texID, 0);						// Tell "s_vTexCoord" to use the 0th texture unit

//------------------------------------------------------------------------------------------------------------
  


   //glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
   //glBlendFunc ( GL_SRC_ALPHA , GL_ONE_MINUS_SRC_COLOR );
   glBlendFunc ( GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA );
   //glBlendFunc ( GL_ONE,GL_ONE );
   glUseProgram (shaderProgramID);
   //current_seconds=Timer();
   //glUniform1f (LocationTime, (GLfloat)current_seconds);

   glBindVertexArray(particles);
   glDrawArrays (GL_POINTS, 0, PARTICLE_COUNT);
   glDisable (GL_BLEND);
   glDisable (GL_PROGRAM_POINT_SIZE);
   //l=clock();
   glutSwapBuffers();
   //glutPostRedisplay();	

}


//-------------------------------------------------------------------------------------------------------------------
void changeViewport(int w, int h){
	glViewport(0, 0, w, h);
}



//--------------------------------------------------------------------------------------------------------------------------
int main (int argc, char** argv) {
	// Standard stuff...
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Particles");
	glutReshapeFunc(changeViewport);
	
	glewInit();
	//matrices();
	
	filenumber=5;
	particles = initBuffers(filenumber);


	// Make a shader
	char* vertexShaderSourceCode = readFile("vertexShader1.vsh");
	char* fragmentShaderSourceCode = readFile("fragmentShader1.fsh");
	GLuint vertShaderID = makeVertexShader(vertexShaderSourceCode);
	GLuint fragShaderID = makeFragmentShader(fragmentShaderSourceCode);
	shaderProgramID = makeShaderProgram(vertShaderID, fragShaderID);



	// ============ New! glUniformLocation is how you pull IDs for uniform variables===============
	LocationMVP = glGetUniformLocation(shaderProgramID, "MVP");
	countID = glGetUniformLocation(shaderProgramID,"LifeTime");
	positionID = glGetAttribLocation(shaderProgramID, "pos");
	/*LocationGravity = glGetUniformLocation(shaderProgramID, "Gravity");
	LocationLifetime=glGetUniformLocation(shaderProgramID, "ParticleLifetime");
	LocationTime = glGetUniformLocation(shaderProgramID, "Time");*/

	//===========================================================================================================
	int bmpWidth = -1;
	int bmpHeight = -1;
	int bmpSize = -1;
	loadBitmapFromFile("txt1.bmp", &bmpWidth, &bmpHeight, &bmpSize, (unsigned char**)&imageData);

	glEnable (GL_TEXTURE_2D);					// Turn on texturing
	glGenTextures(1, &texBufferID);				// Create an ID for a texture buffer

	glBindTexture (GL_TEXTURE_2D, texBufferID);	// Bind that buffer so we can then fill it (in next line)
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bmpWidth, bmpHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, imageData);


	texCoordID = glGetAttribLocation(shaderProgramID, "s_vTexCoord");
	/*glEnableVertexAttribArray(texCoordID);
	int textureCoordOffset = 3*NUM_VERTICES*sizeof(GLfloat);
	glVertexAttribPointer(texCoordID, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(textureCoordOffset));

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// Set the preferences
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); */

	texID = glGetUniformLocation(shaderProgramID, "texture");
	//glActiveTexture(GL_TEXTURE0);				// Turn on texture unit 0
	//glUniform1i(texID, 0);						// Tell "s_vTexCoord" to use the 0th texture unit

	//===========================================================================================================

	glUseProgram(shaderProgramID);
	//glClearColor(0.1f, 0.1f, 0.1f, 1);
	glutDisplayFunc(render);
	glutIdleFunc(idler);
	glutMainLoop();
	
	return 0;


}
