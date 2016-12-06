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
#include "myCone.h"

using namespace std;
using namespace glm;

/**
* VAO ids
*/
enum object {SUN, PLANET, SKY, CONE};

/**
* VBO ids
*/
enum buffer {SUN_VERTICES, SUN_INDICES, PLANET_VERTICES, PLANET_INDICES, SKY_VERTICES, CONE_VERTICES};

static float sunAngleX = 0.0, sunAngleY = 0.0, sunAngleZ = 0.0;
static float earthOrbitX = 0.0, earthOrbitY = 0.0, earthOrbitZ = 0.0;
static float earthRotateX = 0.0, earthRotateY = 0.0, earthRotateZ = 0.0;

/**
* Light properties matrix for our light
*/
static const Light light =
{
    vec4(0.2, 0.2, 0.2, 1.0),
    vec4(1.0, 1.0, 1.0, 1.0),
    vec4(1.0, 1.0, 1.0, 1.0),
    vec4(0.0, 0.0, -4.0, 0.0)
};

/**
* Material properties matrix for the sun
*/
static const Material sunMaterial =
{
    vec4(1.0, 1.0, 1.0, 1.0),
    vec4(1.0, 1.0, 1.0, 1.0),
    vec4(0.0, 0.0, 0.0, 1.0),
    vec4(1.0, 1.0, 1.0, 1.0),
    1000.0f
};

/**
* Material properties matrix for planets
*/
static const Material planetMaterial =
{
    vec4(1.0, 1.0, 1.0, 1.0),
    vec4(1.0, 1.0, 1.0, 1.0),
    vec4(1.0, 1.0, 1.0, 1.0),
    vec4(0.0, 0.0, 0.0, 1.0),
    20.0f
};

/**
* Vertices, normals, and texture mapping for the sky
*/
static Vertex skyVertices[4] =
{
    {vec4(15.0, -15.0, -5.0, 1.0), vec3(1.0), vec2(1.0, 0.0)},
    {vec4(15.0, 15.0, -5.0, 1.0), vec3(1.0), vec2(1.0, 1.0)},
    {vec4(-15.0, -15.0, -5.0, 1.0), vec3(1.0), vec2(0.0, 0.0)},
    {vec4(-15.0, 15.0, -5.0, 1.0), vec3(1.0), vec2(0.0, 1.0)}
};

/**
* Initialize modelView, projection, and normal matrices
*/
static mat4 modelViewMat = mat4(1.0);
static mat4 projMat = mat4(1.0);
static mat4 normalMat = mat4(1.0);

/**
* Setup configuration for sun
*/
static Vertex sunVertices[(SPHERE_LONGS + 1) * (SPHERE_LATS + 1)];
static unsigned int sunIndices[SPHERE_LATS][2 * (SPHERE_LONGS + 1)];
static int sunCounts[SPHERE_LATS];
static void* sunOffsets[SPHERE_LATS];

/**
* Initial configuration for planet
*/
static Vertex planetVertices[(SPHERE_LONGS + 1) * (SPHERE_LATS + 1)];
static unsigned int planetIndices[SPHERE_LATS][2 * (SPHERE_LONGS + 1)];
static int planetCounts[SPHERE_LATS];
static void* planetOffsets[SPHERE_LATS];

/**
* Constructing and initializing a hat
*/
MyCone planetHat = MyCone(0.2, 1.0);

/**
* time between frames in milliseconds
*/
static int animationPeriod = 30;
bool isAnimate = false;

/**
 * Initial configuration for the shader
 */
static unsigned int
programId,
vertexShaderId,
fragmentShaderId,
modelViewMatLoc,
normalMatLoc,
projMatLoc,
sunTexLoc,
earthTexLoc,
skyTexLoc,
hatTexLoc,
objectLoc,
buffer[6],
vao[4],
texture[4],
width,
height;

/**
* Texture bitmaps
*/
static BitMapFile *image[4];

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

void createVaoSun()
{
    glBindVertexArray(vao[SUN]);
    glBindBuffer(GL_ARRAY_BUFFER, buffer[SUN_VERTICES]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sunVertices), sunVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[SUN_INDICES]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sunIndices), sunIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(sunVertices[0]), 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(sunVertices[0]), (void*)sizeof(sunVertices[0].coords));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(sunVertices[0]),
                          (void*)(sizeof(sunVertices[0].coords) + sizeof(sunVertices[0].normal)));
    glEnableVertexAttribArray(2);
}

void createVaoPlanet()
{
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
}

void createVaoSky()
{
    glBindVertexArray(vao[SKY]);
    glBindBuffer(GL_ARRAY_BUFFER, buffer[SKY_VERTICES]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyVertices), skyVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(skyVertices[0]), 0);
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(skyVertices[0]), (void*)sizeof(skyVertices[0].coords));
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(8, 2, GL_FLOAT, GL_FALSE, sizeof(skyVertices[0]), (void*)(sizeof(skyVertices[0].coords) + sizeof(skyVertices[0].normal)));
    glEnableVertexAttribArray(8);
}

void createVaoCone()
{
    glBindVertexArray(vao[CONE]);
    glBindBuffer(GL_ARRAY_BUFFER, buffer[CONE_VERTICES]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planetHat.vertices), planetHat.vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(planetHat.vertices[0]), 0);
    glEnableVertexAttribArray(9);
    glVertexAttribPointer(10, 3, GL_FLOAT, GL_FALSE, sizeof(planetHat.vertices[0]), (void*)sizeof(planetHat.vertices[0].coords));
    glEnableVertexAttribArray(10);
    glVertexAttribPointer(11, 2, GL_FLOAT, GL_FALSE, sizeof(planetHat.vertices[0]), (void*)(sizeof(planetHat.vertices[0].coords) + sizeof(planetHat.vertices[0].normal)));
    glEnableVertexAttribArray(11);
}

void createAndLinkShader()
{
    vertexShaderId = setShader("vertex", "vertexShader.glsl");
    fragmentShaderId = setShader("fragment", "fragmentShader.glsl");
    programId = glCreateProgram();
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);
    glLinkProgram(programId);
    glUseProgram(programId);
}

void setShaderConfiguration()
{
    /// Obtain modelView matrix, projection matrix, normal matrix and object uniform locations.
    modelViewMatLoc = glGetUniformLocation(programId, "modelViewMat");
    projMatLoc = glGetUniformLocation(programId, "projMat");
    normalMatLoc = glGetUniformLocation(programId, "normalMat");
    objectLoc = glGetUniformLocation(programId, "object");

    /// Obtain light properties matrix.
    glUniform4fv(glGetUniformLocation(programId, "light.lightAmbience"), 1, &light.ambCols[0]);
    glUniform4fv(glGetUniformLocation(programId, "light.lightDiffuse"), 1, &light.difCols[0]);
    glUniform4fv(glGetUniformLocation(programId, "light.lightSpecular"), 1, &light.specCols[0]);
    glUniform4fv(glGetUniformLocation(programId, "light.coords"), 1, &light.coords[0]);

    /// Obtain material properties matrix for sun.
    glUniform4fv(glGetUniformLocation(programId, "sunMaterial.matAmbience"), 1, &sunMaterial.ambRefl[0]);
    glUniform4fv(glGetUniformLocation(programId, "sunMaterial.matDiffuse"), 1, &sunMaterial.difRefl[0]);
    glUniform4fv(glGetUniformLocation(programId, "sunMaterial.matSpecular"), 1, &sunMaterial.specRefl[0]);
    glUniform4fv(glGetUniformLocation(programId, "sunMaterial.matEmittance"), 1, &sunMaterial.emitCols[0]);
    glUniform1f(glGetUniformLocation(programId, "sunMaterial.matShininess"), sunMaterial.shininess);

    /// Obtain material properties matrix for planets.
    glUniform4fv(glGetUniformLocation(programId, "planetMaterial.matAmbience"), 1, &planetMaterial.ambRefl[0]);
    glUniform4fv(glGetUniformLocation(programId, "planetMaterial.matDiffuse"), 1, &planetMaterial.difRefl[0]);
    glUniform4fv(glGetUniformLocation(programId, "planetMaterial.matSpecular"), 1, &planetMaterial.specRefl[0]);
    glUniform4fv(glGetUniformLocation(programId, "planetMaterial.matEmittance"), 1, &planetMaterial.emitCols[0]);
    glUniform1f(glGetUniformLocation(programId, "planetMaterial.matShininess"), planetMaterial.shininess);
}

void bindSunTexture()
{
    image[0] = getbmp("sun_texture.bmp");
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
}

void bindEarthTexture()
{
    image[1] = getbmp("earth_texture.bmp");
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[1]->sizeX, image[1]->sizeY, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, image[1]->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    earthTexLoc = glGetUniformLocation(programId, "planetTex");
    glUniform1i(earthTexLoc, 1);
}

void bindSkyTexture()
{
    image[2] = getbmp("sky_texture.bmp");
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

void bindEarthHat()
{
    image[3] = getbmp("hat_texture.bmp");
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, texture[3]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[3]->sizeX, image[3]->sizeY, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, image[3]->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    hatTexLoc = glGetUniformLocation(programId, "hatTex");
    glUniform1i(hatTexLoc, 3);
}

// Initialization routine.
void setup(void)
{
    calcUVN(VPN, VUP);

    glClearColor(1.0, 1.0, 1.0, 0.0);
    glEnable(GL_DEPTH_TEST);

    createAndLinkShader();
    setShaderConfiguration();

    /// Fill vertices, normals, and textures of VAOs
    fillSphere(sunVertices, sunIndices, sunCounts, sunOffsets);
    fillSphere(planetVertices, planetIndices, planetCounts, planetOffsets);

    /// Create VAO's
    glGenVertexArrays(4, vao);
    glGenBuffers(6, buffer);
    createVaoSun();
    createVaoPlanet();
    createVaoSky();
    createVaoCone();

    /// Create texture ids.
    glGenTextures(4, texture);

    /// Bind textures
    bindSunTexture();
    bindEarthTexture();
    bindSkyTexture();
    bindEarthHat();
}

// Drawing routine.
void drawScene(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /// Calculate and update projection matrix.
    projMat = frustum(-1.0, 1.0, -1.0, 1.0, 1.0, 15.0);
    glUniformMatrix4fv(projMatLoc, 1, GL_FALSE, value_ptr(projMat));

    /// Calculate and update modelview matrix.
    modelViewMat = lookAt(vec3(eye), vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0));
    mat4 mvmsave = modelViewMat;
    glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, value_ptr(modelViewMat));

    /// Draw the sky
    glUniform1ui(objectLoc, SKY);
    glBindVertexArray(vao[SKY]);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    /// Handle view rotations
    modelViewMat = rotate(modelViewMat, sunAngleZ, vec3(0.0, 0.0, 1.0));
    modelViewMat = rotate(modelViewMat, sunAngleY, vec3(0.0, 1.0, 0.0));
    modelViewMat = rotate(modelViewMat, sunAngleX, vec3(1.0, 0.0, 0.0));
    glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, value_ptr(modelViewMat));

    /// Calculate and update normal matrix.
    normalMat = transpose(inverse(mat3(modelViewMat)));
    glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, value_ptr(normalMat));
    modelViewMat = mvmsave;

    /// Draw sun.
    glUniform1ui(objectLoc, SUN);
    glBindVertexArray(vao[SUN]);
    glMultiDrawElements(GL_TRIANGLE_STRIP, sunCounts, GL_UNSIGNED_INT, (const void **)sunOffsets, SPHERE_LATS);

    modelViewMat = rotate(modelViewMat, earthOrbitX, vec3(0.0, 0.0, 1.0));
    modelViewMat = rotate(modelViewMat, earthOrbitY, vec3(0.0, 1.0, 0.0));
    modelViewMat = rotate(modelViewMat, earthOrbitZ, vec3(1.0, 0.0, 0.0));
    modelViewMat = translate(modelViewMat, vec3(-2.0, 0.0, 0.0));
    modelViewMat = scale(modelViewMat, vec3(0.2, 0.2, 0.2));
    modelViewMat = rotate(modelViewMat, earthRotateX, vec3(0.0, 0.0, 1.0));
    modelViewMat = rotate(modelViewMat, earthRotateY, vec3(0.0, 1.0, 0.0));
    modelViewMat = rotate(modelViewMat, earthRotateZ, vec3(1.0, 0.0, 0.0));
    glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, value_ptr(modelViewMat));

    /// Calculate and update normal matrix.
    normalMat = transpose(inverse(mat3(modelViewMat)));
    glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, value_ptr(normalMat));

    /// Draw planet.
    glUniform1ui(objectLoc, PLANET);
    glBindVertexArray(vao[PLANET]);
    glMultiDrawElements(GL_TRIANGLE_STRIP, planetCounts, GL_UNSIGNED_INT, (const void **)planetOffsets, SPHERE_LATS);

    modelViewMat = mvmsave;
    modelViewMat = rotate(modelViewMat, earthOrbitX, vec3(0.0, 0.0, 1.0));
    modelViewMat = rotate(modelViewMat, earthOrbitY, vec3(0.0, 1.0, 0.0));
    modelViewMat = rotate(modelViewMat, earthOrbitZ, vec3(1.0, 0.0, 0.0));
    modelViewMat = translate(modelViewMat, vec3(-2.0, 0.0, 0.0));
    modelViewMat = rotate(modelViewMat, earthRotateX, vec3(0.0, 0.0, 1.0));
    modelViewMat = rotate(modelViewMat, earthRotateY, vec3(0.0, 1.0, 0.0));
    modelViewMat = rotate(modelViewMat, earthRotateZ, vec3(1.0, 0.0, 0.0));
    glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, value_ptr(modelViewMat));

    /// Calculate and update normal matrix.
    normalMat = transpose(inverse(mat3(modelViewMat)));
    glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, value_ptr(normalMat));

    glUniform1ui(objectLoc, CONE);
    glBindVertexArray(vao[CONE]);
    glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices + 2);
    modelViewMat = mvmsave;

    glutSwapBuffers();
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
    glViewport(0, 0, w, h);
    width = w;
    height = h;
}

void animate(int value)
{
    if (isAnimate)
    {
        sunAngleY += 0.05;
        if (sunAngleY > 360.0) sunAngleY -= 360.0;
        earthOrbitY += 0.12;
        if (earthOrbitY > 360.0) earthOrbitY -= 360.0;
        earthRotateY += 0.25;
        if (earthRotateY > 360.0) earthRotateY -= 360.0;
        glutPostRedisplay();
        glutTimerFunc(animationPeriod, animate, 1);
    }
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
        sunAngleX += 0.05;
        if (sunAngleX > 360.0) sunAngleX -= 360.0;
        glutPostRedisplay();
        break;
    case 'X':
        sunAngleX -= 0.05;
        if (sunAngleX < 0.0) sunAngleX += 360.0;
        glutPostRedisplay();
        break;
    case 'y':
        sunAngleY += 0.05;
        if (sunAngleY > 360.0) sunAngleY -= 360.0;
        glutPostRedisplay();
        break;
    case 'Y':
        sunAngleY -= 0.05;
        if (sunAngleY < 0.0) sunAngleY += 360.0;
        glutPostRedisplay();
        break;
    case 'z':
        sunAngleZ += 0.05;
        if (sunAngleZ > 360.0) sunAngleZ -= 360.0;
        glutPostRedisplay();
        break;
    case 'Z':
        sunAngleZ -= 0.05;
        if (sunAngleZ < 0.0) sunAngleZ += 360.0;
        glutPostRedisplay();
        break;
    case ' ':
        if (isAnimate) isAnimate = 0;
        else
        {
            isAnimate = 1;
            animate(1);
        }
        break;
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
    glutInitWindowSize(800, 800);
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

