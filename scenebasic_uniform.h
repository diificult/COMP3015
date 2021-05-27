#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"

#include "helper/torus.h"
#include "helper/teapot.h"
#include "helper/plane.h"
#include "helper/objmesh.h"

#include "helper/frustum.h"

#include <glm/glm.hpp>

class SceneBasic_Uniform : public Scene
{
private:
    GLSLProgram prog, shadowProg, eprog, wireframeProg, smokeProg;
    GLuint fboHandle, renderTex, shadowFBO, pass1Index, pass2Index, fsQuad;

    int shadowMapWidth, shadowMapHeight;

    Frustum lightFrustum;

    glm::mat4 lightPV, shadowBias;

    Torus torus;
    Plane plane;
    //Teapot teapot;

    std::unique_ptr <ObjMesh> Lego;

    glm::mat4 viewport;


    GLuint posBuf[2], velBuf[2], age[2];

    glm::vec3 emitterPos, emitterDir;

    GLuint particleArray[2];

    GLuint feedback[2];

    GLuint drawBuf;

    int nParticles;
    float particleLifetime;
    float angle;
    float time, deltaT;



    void setMatrices();
    void setEMatrices();
    void setMatrices(GLSLProgram&);
    void drawScene();
    void drawWalls();
    void drawCat();
    void drawCatShadow();
    void setupFBO();
    void setupEFBO();
    void compile();

    void pass1();
    void pass2();

public:
    SceneBasic_Uniform();

    void initScene();
    void initSmokeScene();
    void update( float t );
    void render();
    void resize(int, int);
};

#endif // SCENEBASIC_UNIFORM_H
