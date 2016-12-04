///////////////////////////////////////////////////////////////////////
// litTexturedSphereShaderized.cpp
//
// Forward-compatible core GL 4.3 version of litTexturedSphere.cpp.
//
// Interaction:
// Press x, X, y, Y, z, Z to turn the hemisphere.
//
// Sumanta Guha
//
// Texture Credits: See ExperimenterSource/Textures/TEXTURE_CREDITS.txt
///////////////////////////////////////////////////////////////////////

#include <cmath>
#include <iostream>
#include <fstream>

#ifdef __APPLE__
#  include <GL/glew.h>
#  include <GL/freeglut.h>
#  include <OpenGL/glext.h>
#else
#  include <GL/glew.h>
#  include <GL/freeglut.h>
#  include <GL/glext.h>
#pragma comment(lib, "glew32.lib")
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "shader.h"
#include "sphere.h"
#include "disc.h"
#include "light.h"
#include "material.h"
#include "getbmp.h"

using namespace std;
using namespace glm;

enum object {SPHERE, DISC}; // VAO ids.
enum buffer {SPHERE_VERTICES, SPHERE_INDICES, DISC_VERTICES}; // VBO ids.

// Globals.
static float Xangle = 0.0, Yangle = 0.0, Zangle = 0.0; // Angles to rotate the sphere.

// Light properties.
static const Light light0 =
{
	vec4(0.0, 0.0, 0.0, 1.0),
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(0.0, 0.0, 0.0, 0.0)
};

// Global ambient.
static const vec4 globAmb = vec4(0.2, 0.2, 0.2, 1.0);

// Front and back material properties.
static const Material sunMaterial =
{
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(1.0, 1.0, 1.0, 1.0),
	50.0f
};

static const Material planetMaterial =
{
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(0.0, 0.0, 0.0, 1.0),
	50.0f
};

// sphere data.
static Vertex sphereVertices[(SPHERE_LONGS + 1) * (SPHERE_LATS + 1)];
static unsigned int sphereIndices[SPHERE_LATS][2 * (SPHERE_LONGS + 1)];
static int sphereCounts[SPHERE_LATS];
static void* sphereOffsets[SPHERE_LATS];

// Disc data.
static Vertex discVertices[DISC_SEGS];

static mat4 modelViewMat = mat4(1.0);
static mat4 projMat = mat4(1.0);
static mat3 normalMat = mat3(1.0);

static unsigned int
   programId,
   vertexShaderId,
   fragmentShaderId,
   modelViewMatLoc,
   normalMatLoc,
   projMatLoc,
   canLabelTexLoc,
   canTopTexLoc,
   objectLoc,
   buffer[3],
   vao[2],
   texture[2],
   width,
   height;

static BitMapFile *image[2]; // Local storage for bmp image data.

// Initialization routine.
void setup(void)
{
   glClearColor(1.0, 1.0, 1.0, 0.0);
   glEnable(GL_DEPTH_TEST);

   // Create shader program executable.
   vertexShaderId = setShader("vertex", "vertexShader.glsl");
   fragmentShaderId = setShader("fragment", "fragmentShader.glsl");
   programId = glCreateProgram();
   glAttachShader(programId, vertexShaderId);
   glAttachShader(programId, fragmentShaderId);
   glLinkProgram(programId);
   glUseProgram(programId);

   // Initialize sphere and disc.
   fillSphere(sphereVertices, sphereIndices, sphereCounts, sphereOffsets);
   fillDiscVertexArray(discVertices);

   // Create VAOs and VBOs...
   glGenVertexArrays(2, vao);
   glGenBuffers(3, buffer);

   // ...and associate data with vertex shader.
   glBindVertexArray(vao[SPHERE]);
   glBindBuffer(GL_ARRAY_BUFFER, buffer[SPHERE_VERTICES]);
   glBufferData(GL_ARRAY_BUFFER, sizeof(sphereVertices), sphereVertices, GL_STATIC_DRAW);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[SPHERE_INDICES]);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphereIndices), sphereIndices, GL_STATIC_DRAW);
   glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(sphereVertices[0]), 0);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(sphereVertices[0]), (void*)sizeof(sphereVertices[0].coords));
   glEnableVertexAttribArray(1);
   glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(sphereVertices[0]),
	                                               (void*)(sizeof(sphereVertices[0].coords)+sizeof(sphereVertices[0].normal)));
   glEnableVertexAttribArray(2);

   // ...and associate data with vertex shader.
   glBindVertexArray(vao[DISC]);
   glBindBuffer(GL_ARRAY_BUFFER, buffer[DISC_VERTICES]);
   glBufferData(GL_ARRAY_BUFFER, sizeof(discVertices), discVertices, GL_STATIC_DRAW);
   glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(discVertices[0]), 0);
   glEnableVertexAttribArray(3);
   glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(discVertices[0]), (void*)sizeof(discVertices[0].coords));
   glEnableVertexAttribArray(4);
   glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(discVertices[0]),
	                                               (void*)(sizeof(discVertices[0].coords)+sizeof(discVertices[0].normal)));
   glEnableVertexAttribArray(5);

   // Obtain modelview matrix, projection matrix, normal matrix and object uniform locations.
   modelViewMatLoc = glGetUniformLocation(programId,"modelViewMat");
   projMatLoc = glGetUniformLocation(programId,"projMat");
   normalMatLoc = glGetUniformLocation(programId,"normalMat");
   objectLoc = glGetUniformLocation(programId, "object");

   // Obtain light property uniform locations and set values.
   glUniform4fv(glGetUniformLocation(programId, "light0.ambCols"), 1, &light0.ambCols[0]);
   glUniform4fv(glGetUniformLocation(programId, "light0.difCols"), 1, &light0.difCols[0]);
   glUniform4fv(glGetUniformLocation(programId, "light0.specCols"), 1, &light0.specCols[0]);
   glUniform4fv(glGetUniformLocation(programId, "light0.coords"), 1, &light0.coords[0]);

   // Obtain global ambient uniform location and set value.
   glUniform4fv(glGetUniformLocation(programId, "globAmb"), 1, &globAmb[0]);

   // Obtain material property uniform locations and set values.
   glUniform4fv(glGetUniformLocation(programId, "sunMaterial.ambRefl"), 1, &sunMaterial.ambRefl[0]);
   glUniform4fv(glGetUniformLocation(programId, "sunMaterial.difRefl"), 1, &sunMaterial.difRefl[0]);
   glUniform4fv(glGetUniformLocation(programId, "sunMaterial.specRefl"), 1, &sunMaterial.specRefl[0]);
   glUniform4fv(glGetUniformLocation(programId, "sunMaterial.emitCols"), 1, &sunMaterial.emitCols[0]);
   glUniform1f(glGetUniformLocation(programId, "sunMaterial.shininess"), sunMaterial.shininess);

   // Load the images.
   image[0] = getbmp("sun_texture.bmp");
   image[1] = getbmp("canTop.bmp");

   // Create texture ids.
   glGenTextures(2, texture);

   // Bind can label image.
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, texture[0]);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[0]->sizeX, image[0]->sizeY, 0,
	            GL_RGBA, GL_UNSIGNED_BYTE, image[0]->data);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   canLabelTexLoc = glGetUniformLocation(programId, "canLabelTex");
   glUniform1i(canLabelTexLoc, 0);

   // Bind can top image.
   glActiveTexture(GL_TEXTURE1);
   glBindTexture(GL_TEXTURE_2D, texture[1]);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[1]->sizeX, image[1]->sizeY, 0,
	            GL_RGBA, GL_UNSIGNED_BYTE, image[1]->data);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   canTopTexLoc = glGetUniformLocation(programId, "canTopTex");
   glUniform1i(canTopTexLoc, 1);

}

// Drawing routine.
void drawScene(void)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   // Calculate and update projection matrix.
   projMat = frustum(-1.0, 1.0, -1.0, 1.0, 1.0, 10.0);
   glUniformMatrix4fv(projMatLoc, 1, GL_FALSE, value_ptr(projMat));

   // Calculate and update modelview matrix.
   modelViewMat = mat4(1.0);
   modelViewMat = lookAt(vec3(0.0, 0.0, 3.0), vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0));
   modelViewMat = rotate(modelViewMat, Zangle, vec3(0.0, 0.0, 1.0));
   modelViewMat = rotate(modelViewMat, Yangle, vec3(0.0, 1.0, 0.0));
   modelViewMat = rotate(modelViewMat, Xangle, vec3(1.0, 0.0, 0.0));
   glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, value_ptr(modelViewMat));

   // Calculate and update normal matrix.
   normalMat = transpose(inverse(mat3(modelViewMat)));
   glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, value_ptr(normalMat));

   // Draw sphere.
   glUniform1ui(objectLoc, SPHERE);
   glBindVertexArray(vao[SPHERE]);

   glMultiDrawElements(GL_TRIANGLE_STRIP, sphereCounts, GL_UNSIGNED_INT, (const void **)sphereOffsets, SPHERE_LATS);

   // Draw disc.
//   glUniform1ui(objectLoc, DISC);
//   glBindVertexArray(vao[DISC]);
//   glDrawArrays(GL_TRIANGLE_FAN, 0, DISC_SEGS);

   glutSwapBuffers();
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
   glViewport(0, 0, w, h);
   width = w; height = h;
}

// Keyboard input processing routine.
void keyInput(unsigned char key, int x, int y)
{
   switch(key)
   {
      case 27:
         exit(0);
         break;
      case 'x':
         Xangle += 0.05;
		 if (Xangle > 360.0) Xangle -= 360.0;
         glutPostRedisplay();
         break;
      case 'X':
         Xangle -= 0.05;
		 if (Xangle < 0.0) Xangle += 360.0;
         glutPostRedisplay();
         break;
      case 'y':
         Yangle += 0.05;
		 if (Yangle > 360.0) Yangle -= 360.0;
         glutPostRedisplay();
         break;
      case 'Y':
         Yangle -= 0.05;
		 if (Yangle < 0.0) Yangle += 360.0;
         glutPostRedisplay();
         break;
      case 'z':
         Zangle += 0.05;
		 if (Zangle > 360.0) Zangle -= 360.0;
         glutPostRedisplay();
         break;
      case 'Z':
         Zangle -= 0.05;
		 if (Zangle < 0.0) Zangle += 360.0;
         glutPostRedisplay();
         break;
      default:
         break;
   }
}

// Routine to output interaction instructions to the C++ window.
void printInteraction(void)
{
   cout << "Interaction:" << endl;
   cout << "Press x, X, y, Y, z, Z to turn the sphere." << endl;
}

// Main routine.
int main(int argc, char **argv)
{
   printInteraction();
   glutInit(&argc, argv);

   glutInitContextVersion(4, 3);
   glutInitContextProfile(GLUT_CORE_PROFILE);
   glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);

   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
   glutInitWindowSize(500, 500);
   glutInitWindowPosition(100, 100);
   glutCreateWindow("The Sun");
   glutDisplayFunc(drawScene);
   glutReshapeFunc(resize);
   glutKeyboardFunc(keyInput);

   glewExperimental = GL_TRUE;
   glewInit();

   setup();

   glutMainLoop();
}

