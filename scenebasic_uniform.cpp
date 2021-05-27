#include "scenebasic_uniform.h"

#include <iostream>
using std::cerr;
using std::endl;

#include <glm/gtc/matrix_transform.hpp>
using glm::vec3;
using glm::mat4;
using glm::vec4;

#include "helper/noisetex.h"
#include "helper/texture.h"
#include "helper/particleutils.h"

//constructor for torus
SceneBasic_Uniform::SceneBasic_Uniform() : torus(0.7f * 2.0f, 0.3f * 2.0f, 50, 50), plane(40.0f, 40.0f, 2, 2), shadowMapWidth(512), shadowMapHeight(512), angle(0.0f), drawBuf(1), time(0), deltaT(0), particleLifetime(6.0f), nParticles(1000),
emitterPos(1, 0, 1), emitterDir(-1, 2, 0) {
     Lego = ObjMesh::load("media/cat.obj");
}

//constructor for teapot
//SceneBasic_Uniform::SceneBasic_Uniform() : teapot(13, glm::translate(mat4(1.0f), vec3(0.0f, 1.5f, 0.25f))) {}

void SceneBasic_Uniform::initScene()
{
    compile();
	glEnable(GL_DEPTH_TEST);

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    setupFBO();
    initSmokeScene();
   
    GLuint programHandle = shadowProg.getHandle();
    pass1Index = glGetSubroutineIndex(programHandle, GL_FRAGMENT_SHADER, "recordDepth");
    pass2Index = glGetSubroutineIndex(programHandle, GL_FRAGMENT_SHADER, "shadeWithShadow");

    shadowBias = mat4(vec4(0.5f, 0.0f, 0.0f, 0.0f),
        vec4(0.0f, 0.5f, 0.0f, 0.0f),
        vec4(0.0f, 0.0f, 0.5f, 0.0f),
        vec4(0.5f, 0.5f, 0.5f, 1.0f)
    );

    float c = 1.65f;
    vec3 lightPos = vec3(0.0f, c * 5.25f, c * 7.5f);
    lightFrustum.orient(lightPos, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
    lightFrustum.setPerspective(50.0f, 1.0f, 1.0f, 25.0f);
    lightPV = shadowBias * lightFrustum.getProjectionMatrix() * lightFrustum.getViewMatrix();
    
    shadowProg.setUniform("Light.Intensity", 1.0f, 1.0f, 1.0f);     //setting the Ld uniform
    shadowProg.setUniform("ShadowMap", 0);

    shadowProg.setUniform("LowThreshold", 0.30f);
    shadowProg.setUniform("HighThreshold", 0.70f);
    shadowProg.setUniform("Light.Intensity", vec3(1.0f, 1.0f, 1.0f));
    shadowProg.setUniform("NoiseTex", 2);

   

    glActiveTexture(GL_TEXTURE2);
    GLuint noiseTex = NoiseTex::generate2DTex();
    glBindTexture(GL_TEXTURE_2D, noiseTex);

#
    glActiveTexture(GL_TEXTURE3);
    Texture::loadTexture("media/smoke.png");

    glActiveTexture(GL_TEXTURE4);
    ParticleUtils::createRandomTex1D(nParticles * 3);
}

void SceneBasic_Uniform::initSmokeScene() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Generate the buffers for initial velocity and start (birth) time
    glGenBuffers(2, posBuf); // Initial velocity buffer
    glGenBuffers(2, velBuf); // Start time buffer
    glGenBuffers(2, age); // Start time buffer
    // Allocate space for all buffers
    int size = nParticles * 3 * sizeof(GLfloat);
   
    glBindBuffer(GL_ARRAY_BUFFER, posBuf[0]);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf[1]);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, velBuf[0]);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, velBuf[1]);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, age[0]);
    glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(float), 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, age[1]);
    glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(float), 0, GL_DYNAMIC_COPY);
 
    std::vector<GLfloat> tempData(nParticles);
    float rate = particleLifetime / nParticles;
    for (int i = 0; i < nParticles; i++) {
        tempData[i] = rate * (i - nParticles);
    }
    glBindBuffer(GL_ARRAY_BUFFER, age[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, nParticles * sizeof(float), tempData.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenVertexArrays(2, particleArray);

    glBindVertexArray(particleArray[0]);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, velBuf[0]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, age[0]);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    // Set up particle array 1
    glBindVertexArray(particleArray[1]);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf[1]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, velBuf[1]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, age[1]);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    // Setup the feedback objects
    glGenTransformFeedbacks(2, feedback);
    // Transform feedback a
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, posBuf[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, velBuf[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, age[0]);
    // Transform feedback 1
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, posBuf[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, velBuf[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, age[1]);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

}


void SceneBasic_Uniform::setupFBO()
{
    GLfloat border[] = { 1.0f, 0.0f, 0.0f, 0.0f };
    // The depth buffer texture
    GLuint depthTex;
    glGenTextures(1, &depthTex);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, shadowMapWidth, shadowMapHeight);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

    // Assign the depth buffer texture to texture channel
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    // Create and set up the FBO

    glGenFramebuffers(1, &shadowFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D, depthTex, 0);

    GLenum drawBuffers[] = { GL_NONE };
    glDrawBuffers(1, drawBuffers);

    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (result == GL_FRAMEBUFFER_COMPLETE) {
        printf("Framebuffer is complete. \n");
    }
    else {
        printf("Framebuffer is not complete. \n");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void SceneBasic_Uniform::compile()
{
	try {
		prog.compileShader("shader/basic_uniform.vert");
		prog.compileShader("shader/basic_uniform.frag");
		prog.link();
		//prog.use();

        eprog.compileShader("shader/basic_uniform.vert");
        eprog.compileShader("shader/edge.frag");
        eprog.link();

        wireframeProg.compileShader("shader/Wireframe.vert");
        wireframeProg.compileShader("shader/Wireframe.frag");
        wireframeProg.compileShader("shader/Wireframe.gs");
        
        wireframeProg.link();

        smokeProg.compileShader("shader/smoke.vert");
        smokeProg.compileShader("shader/smoke.frag");
        GLuint progHandle = smokeProg.getHandle();
        const char* outputNames[] = { "Position", "Velocity", "Age" };
        glTransformFeedbackVaryings(progHandle, 3, outputNames, GL_SEPARATE_ATTRIBS);

        smokeProg.link();

        shadowProg.compileShader("shader/Shadow.vert");
        shadowProg.compileShader("shader/Shadow.frag");
        shadowProg.link();
        shadowProg.use();
	} catch (GLSLProgramException &e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
}

void SceneBasic_Uniform::update( float t )
{
    deltaT = t - time;
    time = t;
}

void SceneBasic_Uniform::render()
{
    shadowProg.use();
    // Pass 1 (shadow map generation)
    view = lightFrustum.getViewMatrix();
    projection = lightFrustum.getProjectionMatrix();
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, shadowMapWidth, shadowMapHeight);
    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &pass1Index);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.5f, 10.0f);
    drawScene();
    drawWalls();
    drawCatShadow();
    glCullFace(GL_BACK);
    glFlush();
    // spitoutDepthBuffer(); // This is just used to get an image of the depth buffer
     // Pass 2 (render)
    float c = 2.0f;
    vec3 cameraPos(c * 11.5f * cos(0.5), c * 7.0f, c * 11.5f * sin(0.5));
    view = glm::lookAt(cameraPos, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
    shadowProg.setUniform("Light.Position", view * vec4(lightFrustum.getOrigin(), 1.0f));
    projection = glm::perspective(glm::radians(50.0f), (float)width / height, 0.1f, 100.0f);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width, height);
    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &pass2Index);
    drawScene();
    drawWalls();
    // Draw the light's frustum
    prog.use();
    prog.setUniform("Color", vec4(1.0f, 0.0f, 0.0f, 1.0f));
    mat4 mv = view * lightFrustum.getInverseViewMatrix();
    prog.setUniform("MVP", projection * mv);
    lightFrustum.render();
    wireframeProg.use();
   
    wireframeProg.setUniform("Light.Intensity", 1.0f, 1.0f, 1.0f);
    wireframeProg.setUniform("Line.Width", 0.75f);
    wireframeProg.setUniform("Line.Color", vec4(0.05f, 0.0f, 0.05f, 1.0f));
    wireframeProg.setUniform("Light.Position", vec4(10.0f, 15.0f, 2.0f, 1.0f));
    drawCat();
    glFinish();

    
    smokeProg.use();
    glActiveTexture(GL_TEXTURE4);
    glEnable(GL_BLEND);

    smokeProg.setUniform("RandomTex", 4);
    smokeProg.setUniform("ParticleTex", 3);
    smokeProg.setUniform("ParticleLifetime", particleLifetime);
    smokeProg.setUniform("Accel", vec3(0.0f, 0.3f, 0.0f));
    smokeProg.setUniform("ParticleSize", 0.3f);
    smokeProg.setUniform("EmitterPos", emitterPos);
    smokeProg.setUniform("EmitterBasis", ParticleUtils::makeArbitraryBasis(emitterDir));
    smokeProg.setUniform("Time", time);
    smokeProg.setUniform("DeltaT", deltaT);
    // Update pass
    smokeProg.setUniform("Pass", 1);
    glEnable(GL_RASTERIZER_DISCARD);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[drawBuf]);
    glBeginTransformFeedback(GL_POINTS);
    glBindVertexArray(particleArray[1 - drawBuf]);
    glVertexAttribDivisor(0, 0);
    glVertexAttribDivisor(1, 0);
    glVertexAttribDivisor(2, 0);
    glDrawArrays(GL_POINTS, 0, nParticles);
    glBindVertexArray(0);
    glEndTransformFeedback();
    glDisable(GL_RASTERIZER_DISCARD);
    // Render pass
    smokeProg.setUniform("Pass", 2);
    model = mat4(1.0f);
    model = glm::translate(model, vec3(9, 0, 9    ));
    setMatrices(smokeProg);
    glDepthMask(GL_FALSE);
    glBindVertexArray(particleArray[drawBuf]);
    glVertexAttribDivisor(0, 1);
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, nParticles);
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);
    // Swap buffers
    drawBuf = 1 - drawBuf;



}


void SceneBasic_Uniform::drawScene()
{
    vec3 color = vec3(0.2f, 0.5f, 0.9f);
    shadowProg.setUniform("Material.Ka", color * 0.05f);
    shadowProg.setUniform("Material.Kd", color);
    shadowProg.setUniform("Material.Ks", 0.95f, 0.95f, 0.95f);
    shadowProg.setUniform("Material.Shininess", 150.0f);
    model = mat4(1.0f);
    model = glm::translate(model, vec3(0.0f, 2.0f, 5.0f));
    model = glm::rotate(model, glm::radians(-45.0f), vec3(1.0f, 0.0f, 0.0f));
    setMatrices(); //we set matrices 
    torus.render();     //we render the torus
    
    
}



void SceneBasic_Uniform::drawWalls()
{
    shadowProg.setUniform("Wall", true);
    shadowProg.setUniform("Material.Kd", 0.25f, 0.25f, 0.25f);
    shadowProg.setUniform("Material.Ks", 0.0f, 0.0f, 0.0f);
    shadowProg.setUniform("Material.Ka", 0.05f, 0.05f, 0.05f);
    shadowProg.setUniform("Material.Shininess", 1.0f);
    model = mat4(1.0f);
    setMatrices();
    plane.render();
    model = mat4(1.0f);
    model = glm::translate(model, vec3(-5.0f, 5.0f, 0.0f));
    model = glm::rotate(model, glm::radians(-90.0f), vec3(0.0f, 0.0f, 1.0f));
    setMatrices();
    plane.render();
    model = mat4(1.0f);
    model = glm::translate(model, vec3(0.0f, 5.0f, -5.0f));
    model = glm::rotate(model, glm::radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
    setMatrices();
    plane.render();
    model = mat4(1.0f);
    shadowProg.setUniform("Wall", false);
}

void SceneBasic_Uniform::drawCat()
{

    vec3 color = vec3(0.2f, 0.5f, 0.9f);

    wireframeProg.setUniform("Material.Ka", color * 0.05f);
    wireframeProg.setUniform("Material.Kd", color);
    wireframeProg.setUniform("Material.Ks", 0.95f, 0.95f, 0.95f);
    wireframeProg.setUniform("Material.Shininess", 150.0f);
    model = mat4(1.0f);
    model = glm::translate(model, vec3(7.0f, 0.8f, 0.0f));
    model = glm::rotate(model, glm::radians(-30.0f), vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, vec3(0.0125f, 0.0125f, 0.0125f));
    setEMatrices(); //we set matrices 
    Lego->render();
}

void SceneBasic_Uniform::drawCatShadow()
{

    vec3 color = vec3(0.2f, 0.5f, 0.9f);

    shadowProg.setUniform("Material.Ka", color * 0.05f);
    shadowProg.setUniform("Material.Kd", color);
    shadowProg.setUniform("Material.Ks", 0.95f, 0.95f, 0.95f);
    shadowProg.setUniform("Material.Shininess", 150.0f);
    model = mat4(1.0f);
    model = glm::translate(model, vec3(7.0f, 0.8f, 0.0f));
    model = glm::rotate(model, glm::radians(-30.0f), vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, vec3(0.0125f, 0.0125f, 0.0125f));
    setMatrices(); //we set matrices 
    Lego->render();
}


void SceneBasic_Uniform::setEMatrices()
{
    mat4 mv = view * model; //we create a model view matrix

    wireframeProg.setUniform("ModelViewMatrix", mv); //set the uniform for the model view matrix
    wireframeProg.setUniform("ProjectionMatrix", projection);


    wireframeProg.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2]))); //we set the uniform for normal matrix

    wireframeProg.setUniform("MVP", projection * mv); //we set the model view matrix by multiplying the mv with the projection matrix

    wireframeProg.setUniform("ViewportMatrix", viewport);
}
void SceneBasic_Uniform::setMatrices()
{
    mat4 mv = view * model; //we create a model view matrix
    
    shadowProg.setUniform("ModelViewMatrix", mv); //set the uniform for the model view matrix
    
    shadowProg.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2]))); //we set the uniform for normal matrix
    
    shadowProg.setUniform("MVP", projection * mv); //we set the model view matrix by multiplying the mv with the projection matrix

    shadowProg.setUniform("ShadowMatrix", lightPV * model);
}

void SceneBasic_Uniform::setMatrices(GLSLProgram& p)
{
    mat4 mv = view * model;
    p.setUniform("MV", mv);
    p.setUniform("Proj", projection);
}

void SceneBasic_Uniform::resize(int w, int h)
{
    glViewport(0, 0, w, h);
    width = w;
    height = h;
    projection = glm::perspective(glm::radians(70.0f), (float)w / h, 0.3f, 100.0f);
    
    float w2 = w / 2.0f;
    float h2 = h / 2.0f;
    viewport = mat4(vec4(w2, 0.0f, 0.0f, 0.0f),
        vec4(0.0f, h2, 0.0f, 0.0f),
        vec4(0.0f, 0.0f, 1.0f, 0.0f),
        vec4(w2 + 0, h2 + 0, 0.0f, 1.0f));
}
