#include <glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include "LoadShaders.h"
#include "linmath.h"
//#include <vgl.h>
#include <map>
#include <vector>
using namespace std;

#define BUFFER_OFFSET(x)  ((const void*) (x))

GLuint programID;
/*
* Arrays to store the indices/names of the Vertex Array Objects and
* Buffers.  Rather than using the books enum approach I've just
* gone out and made a bunch of them and will use them as needed.
*
* Not the best choice I'm sure.
*/

GLuint vertexBuffers[10], arrayBuffers[10], elementBuffers[10];
/*
* Global variables
*   The location for the transformation and the current rotation
*   angle are set up as globals since multiple methods need to
*   access them.
*/
float rotationAngle;
float currentT = 0.0f;
bool elements;
int nbrTriangles, materialToUse = 0;
int startTriangle = 0, endTriangle = 12;
bool rotationOn = false;
mat4x4 rotation,viewMatrix, projectionMatrix;
map<string, GLuint> locationMap;
GLuint textureID[4];
GLuint currentTextureMap = 0;
GLuint teapotVAO, teapotBAO, sphereVAO, sphereBAO, sphere2VAO, sphere2BAO;
int teapotTriangles, sphereTriangles, sphere2Triangles;
// Prototypes
GLuint buildProgram(string vertexShaderName, string fragmentShaderName);
GLFWwindow * glfwStartUp(int& argCount, char* argValues[],
	string windowTitle = "Texture Mapping - Creps", int width = 800, int height = 800);
void setAttributes(float lineWidth = 1.0, GLenum face = GL_FRONT_AND_BACK,
	GLenum fill = GL_FILL);
void buildObjects();
void getLocations();
void init(string vertexShader, string fragmentShader);
void buildAndSetupTextures();
float* readOBJFile(string filename, int& nbrTriangles, float*& normalArray, float*& textureCoordArray);
/*
 * Error callback routine for glfw -- uses cstdio 
 */
static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

/*
 * keypress callback for glfw -- Escape exits...
 */
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	static float currentLimit = 1.0f;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	/*else if (key == GLFW_KEY_A && action == GLFW_PRESS) {
		mat4x4_rotate_Y(rotation, rotation, 0.31419);
	}
	else if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		mat4x4_rotate_Y(rotation, rotation, -0.31419);
	}*/
	else if (key == GLFW_KEY_X && action == GLFW_PRESS) {
		mat4x4_look_at(rotation, vec3{ 1.0f, 0.0f, 0.0f }, vec3{ 0.0f, 0.0f, 0.0f }, vec3{ 0.0f, 1.0f, 0.0f });
	}
	else if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
		mat4x4_look_at(rotation, vec3{ 0.0f, 1.0f, 0.0f }, vec3{ 0.0f, 0.0f, 0.0f }, vec3{ 0.0f, 0.0f, -1.0f });
	}
	else if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
		mat4x4_look_at(rotation, vec3{ 0.0f, 0.0f, -1.0f }, vec3{ 0.0f, 0.0f, 0.0f }, vec3{ 0.0f, 1.0f, 0.0f });
	}
	else if (key == GLFW_KEY_O && action == GLFW_PRESS) {
		mat4x4_look_at(rotation, vec3{ 1.0f, 1.0f, -1.0f }, vec3{ 0.0f, 1.0f, 0.0f }, vec3{ 0.0f, 1.0f, 0.0f });
	}
	else if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		currentTextureMap = 0;
	}
	else if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		currentTextureMap = 1;
	}
	else if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
		currentTextureMap = 2;
	} 
	else if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
		currentTextureMap = 3;
	}
}

/*
 * Routine to encapsulate some of the startup routines for GLFW.  It returns the window ID of the
 * single window that is created.
 */
GLFWwindow* glfwStartUp(int& argCount, char* argValues[], string title, int width, int height) {
	GLFWwindow* window;

	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, key_callback);

	glfwMakeContextCurrent(window);
	gladLoadGL();
	glfwSwapInterval(1);

	return window;
}


/*
 * Use the author's routines to build the program and return the program ID.
 */
GLuint buildProgram(string vertexShaderName, string fragmentShaderName) {

	/*
	*  Use the Books code to load in the shaders.
	*/
	ShaderInfo shaders[] = {
		{ GL_VERTEX_SHADER, vertexShaderName.c_str() },
		{ GL_FRAGMENT_SHADER, fragmentShaderName.c_str() },
		{ GL_NONE, NULL }
	};
	GLuint program = LoadShaders(shaders);
	if (program == 0) {
		cerr << "GLSL Program didn't load.  Error \n" << endl
			<< "Vertex Shader = " << vertexShaderName << endl
			<< "Fragment Shader = " << fragmentShaderName << endl;
	}
	glUseProgram(program);
	return program;
}

/*
 * Set up the clear color, lineWidth, and the fill type for the display.
 */
void setAttributes(float lineWidth, GLenum face, GLenum fill) {
	/*
	* I'm using wide lines so that they are easier to see on the screen.
	* In addition, this version fills in the polygons rather than leaving it
	* as lines.
	*/
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glLineWidth(lineWidth);
	glPolygonMode(face, fill);
	glEnable(GL_DEPTH_TEST);

}

/*
 * read and/or build the objects to be displayed.  Also sets up attributes that are
 * vertex related.
 */

void buildObjects() {

	//manual build of cube
	GLfloat vertices[] = {
					   -0.5f, -0.5f, -0.5f, 1.0f, -0.5f,  0.5f,  0.5f, 1.0f, -0.5f, -0.5f,  0.5f, 1.0f,
					   -0.5f, -0.5f, -0.5f, 1.0f, -0.5f,  0.5f,  0.5f, 1.0f, -0.5f,  0.5f, -0.5f, 1.0f,
					   -0.5f, -0.5f, -0.5f, 1.0f, -0.5f,  0.5f, -0.5f, 1.0f,  0.5f,  0.5f, -0.5f, 1.0f,
					   -0.5f, -0.5f, -0.5f, 1.0f,  0.5f,  0.5f, -0.5f, 1.0f,  0.5f, -0.5f, -0.5f, 1.0f,
					   -0.5f, -0.5f, -0.5f, 1.0f,  0.5f, -0.5f, -0.5f, 1.0f,  0.5f, -0.5f,  0.5f, 1.0f,
					   -0.5f, -0.5f, -0.5f, 1.0f,  0.5f, -0.5f,  0.5f, 1.0f, -0.5f, -0.5f,  0.5f, 1.0f,
					   -0.5f, -0.5f,  0.5f, 1.0f, -0.5f,  0.5f,  0.5f, 1.0f,  0.5f,  0.5f,  0.5f, 1.0f,
					   -0.5f, -0.5f,  0.5f, 1.0f,  0.5f,  0.5f,  0.5f, 1.0f,  0.5f, -0.5f,  0.5f, 1.0f,
					   -0.5f,  0.5f,  0.5f, 1.0f, -0.5f,  0.5f, -0.5f, 1.0f,  0.5f,  0.5f,  0.5f, 1.0f,
						0.5f,  0.5f,  0.5f, 1.0f,  0.5f,  0.5f, -0.5f, 1.0f, -0.5f,  0.5f, -0.5f, 1.0f,
						0.5f, -0.5f,  0.5f, 1.0f,  0.5f, -0.5f, -0.5f, 1.0f,  0.5f,  0.5f, -0.5f, 1.0f,
						0.5f, -0.5f,  0.5f, 1.0f,  0.5f,  0.5f, -0.5f, 1.0f,  0.5f,  0.5f,  0.5f, 1.0f
	};

	GLfloat colors[] = {
							0.0f, 0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 1.0f, 1.0f,
							0.0f, 0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f, 1.0f,
							0.0f, 0.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f, 1.0f,  1.0f, 1.0f, 0.0f, 1.0f,
							0.0f, 0.0f, 0.0f, 1.0f,  1.0f, 1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
							0.0f, 0.0f, 0.0f, 1.0f,  1.0f, 0.0f, 0.0f, 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
							0.0f, 0.0f, 0.0f, 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,  0.0f, 0.0f, 1.0f, 1.0f,
							0.0f, 0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 1.0f, 1.0f,  1.0f, 1.0f, 1.0f, 1.0f,
							0.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
							0.0f, 1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f, 1.0f,  1.0f, 1.0f, 1.0f, 1.0f,
							1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 1.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f, 1.0f,
							1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f, 0.0f, 1.0f,  1.0f, 1.0f, 0.0f, 1.0f,
							1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f, 0.0f, 1.0f,  1.0f, 1.0f, 1.0f, 1.0f
	};
	GLfloat cubeTextureCoordinates[] = { 0.0f, 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
										 0.0f, 1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 
		                                 3.0f, 3.0f, 3.0f, 0.0f, 0.0f, 0.0f,  // back
		                                 3.0f, 3.0f, 0.0f, 0.0f, 0.0f, 3.0f,
	                                     0.0f, 0.0f,  0.0f, 1.0f, 1.0f, 1.0f, // bottom
	                                     0.0f, 0.0f,  0.0f, 1.0f, 1.0f, 1.0f, 
		                                 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // Front
		                                 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
										 0.0f, 0.0f,  0.0f, 2.0f, 2.0f, 2.0f, 
										 0.0f, 0.0f,  0.0f, 2.0f, 2.0f, 2.0f,   // top -- pt1
										 0.0f, 2.0f, 2.0f, 2.0f, 2.0f, 0.0f,
										 0.0f, 2.0f, 2.0f, 0.0f, 0.0f, 0.0f    // top -- pt2
	};

	glGenVertexArrays(1, vertexBuffers);
	glBindVertexArray(vertexBuffers[0]);

	// Alternately...
	// GLuint   vaoID;
	// glGenVertexArrays(1, &vaoID);
	// glBindVertexArray(vaoID);
	//

/*
 * Test code for internal object.
 */
	nbrTriangles = 12;
	glGenBuffers(1, &(arrayBuffers[0]));
	glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[0]);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(vertices) + sizeof(colors) + sizeof(cubeTextureCoordinates),
		NULL, GL_STATIC_DRAW);
	//                               offset in bytes   size in bytes     ptr to data    
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(colors), colors);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(colors), sizeof(cubeTextureCoordinates), cubeTextureCoordinates);
	/*
	 * Set up variables into the shader programs (Note:  We need the
	 * shaders loaded and built into a program before we do this)
	 */
	GLuint vPosition = glGetAttribLocation(programID, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	GLuint vColor = glGetAttribLocation(programID, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 
		BUFFER_OFFSET(sizeof(vertices)));

	GLuint vTexture = glGetAttribLocation(programID, "vTexture");
	glEnableVertexAttribArray(vTexture);
	glVertexAttribPointer(vTexture, 2, GL_FLOAT, GL_FALSE, 0, 
		BUFFER_OFFSET(sizeof(vertices) + sizeof(colors)));
	
	
	//Begin the build of teapot.obj
	float* teapotNormals, *teapotTextures;
	float *teapotVertices = readOBJFile("teapot.obj", teapotTriangles, teapotNormals, teapotTextures);
	// Print info
	cout << teapotTriangles << endl;
	glGenVertexArrays(1, &teapotVAO);
	glBindVertexArray(teapotVAO);
	glGenBuffers(1, &teapotBAO);
	glBindBuffer(GL_ARRAY_BUFFER, teapotBAO);
	const int VERTICES_PER_TRIANGLE = 3;
	const int FLOATS_PER_VERTEX   = 4;
	const int FLOATS_PER_NORMAL   = 3;
	const int FLOATS_PER_TEXCOORD = 2;
	const int BYTES_PER_FLOAT     = 4;
	glBufferData(GL_ARRAY_BUFFER,
		teapotTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_VERTEX * BYTES_PER_FLOAT + 
		teapotTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_NORMAL * BYTES_PER_FLOAT + 
		teapotTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_TEXCOORD * BYTES_PER_FLOAT,
		NULL, GL_STATIC_DRAW);
	//                               offset in bytes   size in bytes     ptr to data    
	glBufferSubData(GL_ARRAY_BUFFER, 0, teapotTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_VERTEX * BYTES_PER_FLOAT, teapotVertices);
	glBufferSubData(GL_ARRAY_BUFFER, teapotTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_VERTEX * BYTES_PER_FLOAT, teapotTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_NORMAL * BYTES_PER_FLOAT, teapotNormals);
	glBufferSubData(GL_ARRAY_BUFFER, teapotTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_VERTEX * BYTES_PER_FLOAT + teapotTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_NORMAL * BYTES_PER_FLOAT, teapotTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_TEXCOORD * BYTES_PER_FLOAT, teapotTextures);

	vPosition = glGetAttribLocation(programID, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, FLOATS_PER_VERTEX, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	GLuint vNormal = glGetAttribLocation(programID, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, FLOATS_PER_NORMAL, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(teapotTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_VERTEX * BYTES_PER_FLOAT));

	vTexture = glGetAttribLocation(programID, "vTexture");
	glEnableVertexAttribArray(vTexture);
	glVertexAttribPointer(vTexture, FLOATS_PER_TEXCOORD, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(teapotTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_VERTEX * BYTES_PER_FLOAT+teapotTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_NORMAL * BYTES_PER_FLOAT));

	//This is the building of sphere.obj
	float* sphereNormals, * sphereTextures;
	float* sphereVertices = readOBJFile("sphere001.obj", sphereTriangles, sphereNormals, sphereTextures);
	// Print info
	cout << sphereTriangles << endl;
	glGenVertexArrays(1, &sphereVAO);
	glBindVertexArray(sphereVAO);
	glGenBuffers(1, &sphereBAO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereBAO);
	glBufferData(GL_ARRAY_BUFFER,
		sphereTriangles* VERTICES_PER_TRIANGLE* FLOATS_PER_VERTEX* BYTES_PER_FLOAT +
		sphereTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_NORMAL * BYTES_PER_FLOAT +
		sphereTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_TEXCOORD * BYTES_PER_FLOAT,
		NULL, GL_STATIC_DRAW);
	//                               offset in bytes   size in bytes     ptr to data    
	glBufferSubData(GL_ARRAY_BUFFER, 0, sphereTriangles* VERTICES_PER_TRIANGLE* FLOATS_PER_VERTEX* BYTES_PER_FLOAT, sphereVertices);
	glBufferSubData(GL_ARRAY_BUFFER, sphereTriangles* VERTICES_PER_TRIANGLE* FLOATS_PER_VERTEX* BYTES_PER_FLOAT, sphereTriangles* VERTICES_PER_TRIANGLE* FLOATS_PER_NORMAL* BYTES_PER_FLOAT, sphereNormals);
	glBufferSubData(GL_ARRAY_BUFFER, sphereTriangles* VERTICES_PER_TRIANGLE* FLOATS_PER_VERTEX* BYTES_PER_FLOAT + sphereTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_NORMAL * BYTES_PER_FLOAT, sphereTriangles* VERTICES_PER_TRIANGLE* FLOATS_PER_TEXCOORD* BYTES_PER_FLOAT, sphereTextures);

	vPosition = glGetAttribLocation(programID, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, FLOATS_PER_VERTEX, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	vNormal = glGetAttribLocation(programID, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, FLOATS_PER_NORMAL, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sphereTriangles* VERTICES_PER_TRIANGLE* FLOATS_PER_VERTEX* BYTES_PER_FLOAT));

	vTexture = glGetAttribLocation(programID, "vTexture");
	glEnableVertexAttribArray(vTexture);
	glVertexAttribPointer(vTexture, FLOATS_PER_TEXCOORD, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sphereTriangles* VERTICES_PER_TRIANGLE* FLOATS_PER_VERTEX* BYTES_PER_FLOAT + sphereTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_NORMAL * BYTES_PER_FLOAT));

	//This is the building of sphere2
	float* sphere2Normals, * sphere2Textures;
	float* sphere2Vertices = readOBJFile("sphere001.obj", sphere2Triangles, sphere2Normals, sphere2Textures);
	// Print info
	cout << sphere2Triangles << endl;
	glGenVertexArrays(1, &sphere2VAO);
	glBindVertexArray(sphere2VAO);
	glGenBuffers(1, &sphere2BAO);
	glBindBuffer(GL_ARRAY_BUFFER, sphere2BAO);
	glBufferData(GL_ARRAY_BUFFER,
		sphere2Triangles* VERTICES_PER_TRIANGLE* FLOATS_PER_VERTEX* BYTES_PER_FLOAT +
		sphere2Triangles * VERTICES_PER_TRIANGLE * FLOATS_PER_NORMAL * BYTES_PER_FLOAT +
		sphere2Triangles * VERTICES_PER_TRIANGLE * FLOATS_PER_TEXCOORD * BYTES_PER_FLOAT,
		NULL, GL_STATIC_DRAW);
	//                               offset in bytes   size in bytes     ptr to data    
	glBufferSubData(GL_ARRAY_BUFFER, 0, sphere2Triangles* VERTICES_PER_TRIANGLE* FLOATS_PER_VERTEX* BYTES_PER_FLOAT, sphere2Vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sphere2Triangles* VERTICES_PER_TRIANGLE* FLOATS_PER_VERTEX* BYTES_PER_FLOAT, sphere2Triangles* VERTICES_PER_TRIANGLE* FLOATS_PER_NORMAL* BYTES_PER_FLOAT, sphere2Normals);
	glBufferSubData(GL_ARRAY_BUFFER, sphere2Triangles* VERTICES_PER_TRIANGLE* FLOATS_PER_VERTEX* BYTES_PER_FLOAT + sphere2Triangles * VERTICES_PER_TRIANGLE * FLOATS_PER_NORMAL * BYTES_PER_FLOAT, sphere2Triangles* VERTICES_PER_TRIANGLE* FLOATS_PER_TEXCOORD* BYTES_PER_FLOAT, sphere2Textures);

	vPosition = glGetAttribLocation(programID, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, FLOATS_PER_VERTEX, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	vNormal = glGetAttribLocation(programID, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, FLOATS_PER_NORMAL, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sphereTriangles* VERTICES_PER_TRIANGLE* FLOATS_PER_VERTEX* BYTES_PER_FLOAT));

	vTexture = glGetAttribLocation(programID, "vTexture");
	glEnableVertexAttribArray(vTexture);
	glVertexAttribPointer(vTexture, FLOATS_PER_TEXCOORD, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sphereTriangles* VERTICES_PER_TRIANGLE* FLOATS_PER_VERTEX* BYTES_PER_FLOAT + sphereTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_NORMAL * BYTES_PER_FLOAT));
}

/*
 * This fills in the locations of most of the uniform variables for the program.
 * there are better ways of handling this but this is good in going directly from
 * what we had.
 *
 * Revised to get the locations and names of the uniforms from OpenGL.  These
 * are then stored in a map so that we can look up a uniform by name when we
 * need to use it.  The map is still global but it is a little neater than the
 * version that used all the locations.  The locations are still there right now
 * in case that is more useful for you.
 *
 */

void getLocations() {
	/*
	 * Find out how many uniforms there are and go out there and get them from the
	 * shader program.  The locations for each uniform are stored in a global -- locationMap --
	 * for later retrieval.
	 */
	GLint numberBlocks;
	char uniformName[1024];
	int nameLength;
	GLint size;
	GLenum type;
	glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &numberBlocks);
	for (int blockIndex = 0; blockIndex < numberBlocks; blockIndex++) {
		glGetActiveUniform(programID, blockIndex, 1024, &nameLength, &size, &type, uniformName);
		cout << uniformName << endl;
		locationMap[string(uniformName)] = blockIndex;
	}
	
}

/*
 * read in raw data file for image. 
 */
unsigned char* getImageFromFile(string filename, int width, int height) {
	ifstream infile;
	unsigned char* image;
	
	infile.open(filename.c_str(), ios_base::binary);
	if (!infile.fail()) {
		image = new unsigned char[width * height * 3];
		//infile.get(); // get rid of the newline
		memset(image, 0, width * height * 3); // clear out the memory area
		infile.read((char*)image, width * height * 3);
		infile.close();
		return image;
	}
	else {
		return nullptr;
	}
}

void init(string vertexShader, string fragmentShader) {

	setAttributes(1.0f, GL_FRONT_AND_BACK, GL_FILL);

	programID = buildProgram(vertexShader, fragmentShader);
	mat4x4_identity(rotation);
	mat4x4_identity(viewMatrix);
	mat4x4_ortho(projectionMatrix, -5.0f, 5.0f, -5.0f, 5.0f, -100.0f, 100.0f);
	buildObjects();
	getLocations();

	buildAndSetupTextures();
}
/*
 * This function loads and sets up 4 textures for use by the program.  The 
 * textures are in a raw format -- just a list of all the pixel values.
 */
void buildAndSetupTextures()
{
	/*
	*  Create and load the textures 1 of each should go on each of the 4 images that are loaded onto the screen
	*/
	string filenames[] = { "image1.raw", "image2.raw", "image3.raw", "image4.raw" };

	unsigned char* imageData;
	glGenTextures(4, textureID);
	for (int i = 0; i < 4; i++) {
		imageData = getImageFromFile(filenames[i], 256, 256);

		glBindTexture(GL_TEXTURE_2D, textureID[i]);
		glTexStorage2D(GL_TEXTURE_2D, 9, GL_RGB8, 256, 256);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 256, GL_RGB, GL_UNSIGNED_BYTE, imageData);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // No padding between pixels/rows
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);
		//		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glEnable(GL_TEXTURE_2D);
		free(imageData);
	}
}

/*
 * The display routine is basically unchanged at this point.
 */
void display() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// needed
	glUseProgram(programID);
	/*glBindTexture(GL_TEXTURE_2D, textureID[currentTextureMap]);
	GLuint texLocation = glGetUniformLocation(programID, "tex");*/

	GLuint modelMatrixLocation = glGetUniformLocation(programID, "modelingMatrix");
	GLuint projectionMatrixLocation = glGetUniformLocation(programID, "projectionMatrix");
	glUniformMatrix4fv(projectionMatrixLocation, 1, false, (const GLfloat*)projectionMatrix);

	//cube model
	glBindTexture(GL_TEXTURE_2D, textureID[0]);
	GLuint texLocation = glGetUniformLocation(programID, "tex");
	mat4x4 cubeModelingMatrix;
	mat4x4_identity(cubeModelingMatrix);
	mat4x4_translate(cubeModelingMatrix, 2.0, 2.0, 0.0);
	mat4x4_mul(cubeModelingMatrix, rotation, cubeModelingMatrix);
	glUniformMatrix4fv(modelMatrixLocation, 1, false, (const GLfloat *)cubeModelingMatrix);
	glBindVertexArray(vertexBuffers[0]);
	glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[0]);
	glDrawArrays(GL_TRIANGLES, 0, nbrTriangles * 3);
	
	//teapot model
	glBindTexture(GL_TEXTURE_2D, textureID[1]);
	texLocation = glGetUniformLocation(programID, "tex");
	mat4x4 teapotModelingMatrix;
	mat4x4_identity(teapotModelingMatrix);
	mat4x4_translate(teapotModelingMatrix, -2.0, -2.0, 0.0);
	mat4x4_scale_aniso(teapotModelingMatrix, teapotModelingMatrix, 1.0f / 12.0f, 1.0f / 12.0f, 1.0f / 12.0f);
	mat4x4_mul(teapotModelingMatrix, rotation, teapotModelingMatrix);
	glUniformMatrix4fv(modelMatrixLocation, 1, false, (const GLfloat*)teapotModelingMatrix);
	glBindVertexArray(teapotVAO);
	glBindBuffer(GL_ARRAY_BUFFER, teapotBAO);
	glDrawArrays(GL_TRIANGLES, 0, teapotTriangles*3);

	//sphere model
	glBindTexture(GL_TEXTURE_2D, textureID[2]);
	texLocation = glGetUniformLocation(programID, "tex");
	mat4x4 sphereModelingMatrix;
	mat4x4_identity(sphereModelingMatrix);
	mat4x4_translate(sphereModelingMatrix, 2.0, -2.0, 0.0);
	mat4x4_mul(sphereModelingMatrix, rotation, sphereModelingMatrix);
	glUniformMatrix4fv(modelMatrixLocation, 1, false, (const GLfloat*)sphereModelingMatrix);
	glBindVertexArray(sphereVAO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereBAO);
	glDrawArrays(GL_TRIANGLES, 0, sphereTriangles * 3);

	//Adds a copy of the sphere for a fourth object a texture can be mapped to
	glBindTexture(GL_TEXTURE_2D, textureID[3]);
	texLocation = glGetUniformLocation(programID, "tex");
	mat4x4 sphere2ModelingMatrix;
	mat4x4_identity(sphere2ModelingMatrix);
	mat4x4_translate(sphere2ModelingMatrix, -2.0, 2.0, 0.0);
	mat4x4_mul(sphere2ModelingMatrix, rotation, sphere2ModelingMatrix);
	glUniformMatrix4fv(modelMatrixLocation, 1, false, (const GLfloat*)sphere2ModelingMatrix);
	glBindVertexArray(sphere2VAO);
	glBindBuffer(GL_ARRAY_BUFFER, sphere2BAO);
	glDrawArrays(GL_TRIANGLES, 0, sphere2Triangles * 3);
}

/*
* Handle window resizes -- adjust size of the viewport -- more on this later
*/

void reshapeWindow(GLFWwindow* window, int width, int height)
{
	float ratio;
	ratio = width / (float)height;

	glViewport(0, 0, width, height);

}
/*
* Main program with calls for many of the helper routines.
*/
int main(int argCount, char* argValues[]) {
	GLFWwindow* window = nullptr;
	window = glfwStartUp(argCount, argValues, "Texture Mapping - Creps");
	init("texture.vert", "texture.frag");
	glfwSetWindowSizeCallback(window, reshapeWindow);

	while (!glfwWindowShouldClose(window))
	{
		display();
		glfwSwapBuffers(window);
		glfwPollEvents();
	};

	glfwDestroyWindow(window);

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
