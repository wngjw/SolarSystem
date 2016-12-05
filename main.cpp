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
#include "light.h"
#include "material.h"
#include "getbmp.h"
#include "vertex.h"

using namespace std;
using namespace glm;

enum object {SPHERE, PLANET, SKY}; // VAO ids.
enum buffer {SPHERE_VERTICES, SPHERE_INDICES, PLANET_VERTICES, PLANET_INDICES, SKY_VERTICES}; // VBO ids.

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
    vec4(0.0, 0.0, 0.0, 1.0),
    vec4(1.0, 1.0, 1.0, 1.0),
    1000.0f
};

static const Material planetMaterial =
{
    vec4(1.0, 1.0, 1.0, 1.0),
    vec4(1.0, 1.0, 1.0, 1.0),
    vec4(1.0, 1.0, 1.0, 1.0),
    vec4(0.0, 0.0, 0.0, 1.0),
    20.0f
};

static Vertex skyVertices[4] =
{
    {vec4(15.0, -15.0, -5.0, 1.0), vec3(1.0), vec2(1.0, 0.0)},
    {vec4(15.0, 15.0, -5.0, 1.0), vec3(1.0), vec2(1.0, 1.0)},
    {vec4(-15.0, -15.0, -5.0, 1.0), vec3(1.0), vec2(0.0, 0.0)},
    {vec4(-15.0, 15.0, -5.0, 1.0), vec3(1.0), vec2(0.0, 1.0)}
};

static mat4 modelViewMat = mat4(1.0);
static mat4 projMat = mat4(1.0);
static mat4 normalMat = mat4(1.0);

/**
* Setup configuration for sun
*/
static Vertex sphereVertices[(SPHERE_LONGS + 1) * (SPHERE_LATS + 1)];
static unsigned int sphereIndices[SPHERE_LATS][2 * (SPHERE_LONGS + 1)];
static int sphereCounts[SPHERE_LATS];
static void* sphereOffsets[SPHERE_LATS];

/**
* Setup configuration for planet
*/
static Vertex planetVertices[(SPHERE_LONGS + 1) * (SPHERE_LATS + 1)];
static unsigned int planetIndices[SPHERE_LATS][2 * (SPHERE_LONGS + 1)];
static int planetCounts[SPHERE_LATS];
static void* planetOffsets[SPHERE_LATS];

/**
 * Setup configuration for shader
 */
static unsigned int
programId,
vertexShaderId,
fragmentShaderId,
modelViewMatLoc,
normalMatLoc,
projMatLoc,
sunTexLoc,
canTopTexLoc,
skyTexLoc,
objectLoc,
buffer[5],
vao[3],
texture[3],
width,
height;

static BitMapFile *image[3]; // Bitmap files used as textures

/**
 * Setup configuration for view rotation
 */
vec4 eyeStart = vec4(0.0, 0.0, 5.0, 1.0);
vec4 eye = eyeStart; // camera location
mat4 viewRotation;  // rotational part of matrix that transforms between World and Camera coordinates
vec4 VPN(0,.5,1,0);  // used as starting value for setting uvn
vec4 VUP(0,1,0,1);  // used as starting value for setting uvn

void calcUVN(vec4 VPN, vec4 VUP)
{
    vec4 n = normalize(VPN);
    vec4 u = vec4(cross(vec3(VUP),vec3(n)),0);
    u = normalize(u);
    vec4 v = vec4(cross(vec3(n),vec3(u)),0);
    viewRotation = mat4(u,v,n,vec4(0,0,0,1));
}

// Initialization routine.
void setup(void)
{
    calcUVN(VPN, VUP);
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
    fillSphere(planetVertices, planetIndices, planetCounts, planetOffsets);

    // Create VAOs and VBOs...
    glGenVertexArrays(3, vao);
    glGenBuffers(5, buffer);
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
                          (void*)(sizeof(sphereVertices[0].coords) + sizeof(sphereVertices[0].normal)));
    glEnableVertexAttribArray(2);
    // ...and associate data with vertex shader.
    glBindVertexArray(vao[PLANET]);
    glBindBuffer(GL_ARRAY_BUFFER, buffer[PLANET_VERTICES]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planetVertices), planetVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[PLANET_INDICES]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planetIndices), planetIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(planetVertices[0]), 0);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(planetVertices[0]), (void*)sizeof(planetVertices[0].coords));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(planetVertices[0]),
                          (void*)(sizeof(planetVertices[0].coords) + sizeof(planetVertices[0].normal)));
    glEnableVertexAttribArray(5);

    glBindVertexArray(vao[SKY]);
    glBindBuffer(GL_ARRAY_BUFFER, buffer[SKY_VERTICES]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyVertices), skyVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(skyVertices[0]), 0);
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(skyVertices[0]), (void*)sizeof(skyVertices[0].coords));
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(8, 2, GL_FLOAT, GL_FALSE, sizeof(skyVertices[0]),
                          (void*)(sizeof(skyVertices[0].coords) + sizeof(skyVertices[0].normal)));
    glEnableVertexAttribArray(8);

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

    glUniform4fv(glGetUniformLocation(programId, "planetMaterial.ambRefl"), 1, &planetMaterial.ambRefl[0]);
    glUniform4fv(glGetUniformLocation(programId, "planetMaterial.difRefl"), 1, &planetMaterial.difRefl[0]);
    glUniform4fv(glGetUniformLocation(programId, "planetMaterial.specRefl"), 1, &planetMaterial.specRefl[0]);
    glUniform4fv(glGetUniformLocation(programId, "planetMaterial.emitCols"), 1, &planetMaterial.emitCols[0]);
    glUniform1f(glGetUniformLocation(programId, "planetMaterial.shininess"), planetMaterial.shininess);

    // Load the images.
    image[0] = getbmp("sun_texture.bmp");
    image[1] = getbmp("earth_texture.bmp");
    image[2] = getbmp("sky_texture.bmp");
    // Create texture ids.
    glGenTextures(3, texture);
    // Bind can label image.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[0]->sizeX, image[0]->sizeY, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, image[0]->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    sunTexLoc = glGetUniformLocation(programId, "sunTex");
    glUniform1i(sunTexLoc, 0);

    // Bind can top image.
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[1]->sizeX, image[1]->sizeY, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, image[1]->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    canTopTexLoc = glGetUniformLocation(programId, "planetTex");
    glUniform1i(canTopTexLoc, 1);

    // Bind sky image.
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[2]->sizeX, image[2]->sizeY, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, image[2]->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    skyTexLoc = glGetUniformLocation(programId, "skyTex");
    glUniform1i(skyTexLoc, 2);
}

// Drawing routine.
void drawScene(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Calculate and update projection matrix.
    projMat = frustum(-1.0, 1.0, -1.0, 1.0, 1.0, 15.0);
    glUniformMatrix4fv(projMatLoc, 1, GL_FALSE, value_ptr(projMat));

    // Calculate and update modelview matrix.
    modelViewMat = mat4(1.0);
    modelViewMat = lookAt(vec3(eye), vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0));

    glUniform1ui(objectLoc, SKY);
    glBindVertexArray(vao[SKY]);
    glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, value_ptr(modelViewMat));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


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
    glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, value_ptr(modelViewMat));
    glMultiDrawElements(GL_TRIANGLE_STRIP, sphereCounts, GL_UNSIGNED_INT, (const void **)sphereOffsets, SPHERE_LATS);

    modelViewMat = translate(modelViewMat, vec3(-2.0, 0.0, 0.0));
    modelViewMat = scale(modelViewMat, vec3(0.2, 0.2, 0.2));
    glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, value_ptr(modelViewMat));
    // Draw planet.
    glUniform1ui(objectLoc, PLANET);
    glBindVertexArray(vao[PLANET]);
    glMultiDrawElements(GL_TRIANGLE_STRIP, planetCounts, GL_UNSIGNED_INT, (const void **)planetOffsets, SPHERE_LATS);

    glutSwapBuffers();
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
    glViewport(0, 0, w, h);
    width = w;
    height = h;
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

