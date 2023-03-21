#pragma comment(lib, "winmm.lib")
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"

#include <iostream>
#include <array>
#include <Windows.h>
#include <mmsystem.h>
#include <thread>

// window
gps::Window myWindow;

// matrices
glm::mat4 view;
glm::mat4 model;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;
glm::mat4 lightRotation;

// shader uniform locations
GLint viewLoc;
GLint modelLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;

// for fragment discarding
GLint modelLocFragmentDiscarding;
GLint viewLocFragmentDiscarding;
GLint projectionLocFragmentDiscarding;

/////////// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 2.0f, 20.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.3f;
GLfloat cameraSensivity = 0.1f;
GLfloat pitch, yaw;
double previousXpos, previousYpos;
//////////

//models
gps::Model3D terrain;
gps::Model3D terrain_trees;
gps::Model3D eliceMoara;
gps::Model3D bmwBody;
gps::Model3D bmwTire;
gps::Model3D tree1;
gps::Model3D tree2;

/////////// for shadows
const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;
gps::Model3D screenQuad;
GLuint shadowMapFBO;
GLuint depthMapTexture;
///////////

// shaders
gps::Shader basicShader;
gps::Shader screenQuadShader;
gps::Shader fragmentDiscardingShader;
gps::Shader depthMapShader;

//skybox
gps::SkyBox mySkyBox;
gps::Shader skyboxShader;
std::vector<const GLchar*> faces;

GLboolean pressedKeys[1024];

// car constants needed for car movement
#define BODY_X_POS 58.5f
#define TIRE_Y_PHASE (-0.95f)
#define FRONT_WHEEL_PHASE (1.8f)
#define REAR_WHEEL_PHASE (-3.3f)

#define DELAY_TIME 0.05


volatile bool shouldRun; // for the thread function
float delta;
bool pauseAnimation;
bool scenePresentation;
bool showDepthMap;
int retina_width, retina_height;
int enableFog;

GLfloat lightAngle;

uint8_t currentBezierCurve;
float scenePresentationIndex;
double currentTime, nextUpdateCameraMovement;


#define TREE_ARRAY_SIZE 15

std::array<glm::vec2, TREE_ARRAY_SIZE> tree1Positions = {
    glm::vec2(-58.0f,33.0f),
    glm::vec2(-40.0f,-3.0f),
    glm::vec2(4.0f,46.0f),
    glm::vec2(52.0f,-59.0f),
    glm::vec2(79.0f,-59.0f),
    glm::vec2(80.0f,-13.0f),
    glm::vec2(78.0f,38.0f),
    glm::vec2(-62.0f,-36.0f),
    glm::vec2(16.0f,-63.0f),
    glm::vec2(53.0f,-78.0f),
    glm::vec2(72.0f,-28.0f),
    glm::vec2(71.0f,32.0f),
    glm::vec2(3.0f,1.0f),
    glm::vec2(36.0f,11.0f),
    glm::vec2(52.0f,0.2f)
};
std::array<glm::vec2, TREE_ARRAY_SIZE> tree2Positions = {
    glm::vec2(-34.0f,35.0f),
    glm::vec2(4.0f,59.0f),
    glm::vec2(28.0f,-24.0f),
    glm::vec2(50.0f,-37.0f),
    glm::vec2(79.0f,-39.0f),
    glm::vec2(81.0f,23.0f),
    glm::vec2(71.0f,67.0f),
    glm::vec2(10.0f,-36.0f),
    glm::vec2(34.0f,-65.0f),
    glm::vec2(71.0f,-84.0f),
    glm::vec2(71.0f,-6.0f),
    glm::vec2(1.0f,31.0f),
    glm::vec2(16.0f,10.0f),
    glm::vec2(32.0f,1.2f),
    glm::vec2(52.0f,12.0f)
};

double lastTimeStamp = glfwGetTime();
float movementSpeed = 7; // units per second
void updateDelta(double elapsedSeconds) {    
    if (!pauseAnimation)
        delta = delta + movementSpeed * elapsedSeconds;
    if (delta > MAX_Z)
        delta = MIN_X;
}

#define NR_OF_BEZIER_CURVES 10
std::array<glm::vec2,NR_OF_BEZIER_CURVES * 2 + 1> pointsToFollow = {
    glm::vec2(2.0f,18.0f),    // 1
    glm::vec2(-54.0f,31.0f),
    glm::vec2(-35.0f,-4.0f),  // 2
    glm::vec2(-54.0f,32.0f),
    glm::vec2(1.0f,18.0f),    // 3
    glm::vec2(28.0f,18.0f),
    glm::vec2(54.0f,18.0f),   // 4
    glm::vec2(62.0f,36.0f),
    glm::vec2(62.0f,67.0f),   // 5
    glm::vec2(62.0f,-9.0f),
    glm::vec2(62.0f,-74.0f),  // 6
    glm::vec2(62.0f,-19.0f),
    glm::vec2(62.0f,-6.0f),   // 7
    glm::vec2(31.0f,-6.0f),    
    glm::vec2(5.0f,-6.0f),    // 8
    glm::vec2(31.0f,-6.0f),  
    glm::vec2(62.0f,-6.0f),   // 9
    glm::vec2(76.0f,6.0f),
    glm::vec2(56.0f,18.0f),   // 10
    glm::vec2(26.0f,18.0f),  
    glm::vec2(0.0f,18.0f)   // 11
};
std::array<glm::vec2, NR_OF_BEZIER_CURVES + 1> cameraRotationToFollow = {
    glm::vec2(0.0f,190),
    glm::vec2(0.0f,0.0f),
    glm::vec2(-8.0f, 315.0f),
    glm::vec2(.0f,160.0f),
    glm::vec2(.0f,305.0f),
    glm::vec2(.0f,150.0f),
    glm::vec2(.0f,270.0f),
    glm::vec2(.0f,90.0f),
    glm::vec2(.0f,180.0f),
    glm::vec2(.0f,180.0f),
    glm::vec2(.0f,.0f)
};

float getParametricPoint(float n1, float n2, float parametricIndex)
{
    float delta = n2 - n1;
    return n1 + (delta * parametricIndex);
}

void updateCameraMovement()
{
    float xa, xb, ya, yb, x, y, x1, x2, x3, y1, y2, y3;
    x1 = pointsToFollow.at(currentBezierCurve).x;
    y1 = pointsToFollow.at(currentBezierCurve).y;
    x2 = pointsToFollow.at(currentBezierCurve + 1).x;
    y2 = pointsToFollow.at(currentBezierCurve + 1).y;
    x3 = pointsToFollow.at(currentBezierCurve + 2).x;
    y3 = pointsToFollow.at(currentBezierCurve + 2).y;

    scenePresentationIndex += 0.01;
    xa = getParametricPoint(x1, x2, scenePresentationIndex);
    ya = getParametricPoint(y1, y2, scenePresentationIndex);
    xb = getParametricPoint(x2, x3, scenePresentationIndex);
    yb = getParametricPoint(y2, y3, scenePresentationIndex);

    x = getParametricPoint(xa, xb, scenePresentationIndex);
    y = getParametricPoint(ya, yb, scenePresentationIndex);

    if (scenePresentationIndex >= 1.0f) { // daca s-a parcurs o curba bezier
        scenePresentationIndex = 0.0f;    // resetam indexul de parcurgere
        currentBezierCurve += 2;          // avansam catre urmatoarele 3 puncte pe care aplicam curba bezier

        myCamera.rotate(cameraRotationToFollow.at(currentBezierCurve/2).x, cameraRotationToFollow.at(currentBezierCurve/2).y);
    }

    if (currentBezierCurve >= NR_OF_BEZIER_CURVES * 2) // indexarea incepe de la 0, merge si cu ==
        scenePresentation = false;
    
    myCamera.setPosition(glm::vec3(x, 3.0f, y));
}

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
    glViewport(0, 0, width, height);

    // create projection matrix
    projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);
    // send projection matrix to shader
    basicShader.useShaderProgram();
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {

    if (action == GLFW_PRESS) {
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        case GLFW_KEY_F:
            enableFog ^= 1;
            fragmentDiscardingShader.useShaderProgram();
            glUniform1i(glGetUniformLocation(fragmentDiscardingShader.shaderProgram, "enableFog"), enableFog);
            basicShader.useShaderProgram();
            glUniform1i(glGetUniformLocation(basicShader.shaderProgram, "enableFog"), enableFog);
            break;
        case GLFW_KEY_P:
            pauseAnimation = !pauseAnimation;
            break;
        case GLFW_KEY_M:
            showDepthMap = !showDepthMap;
            break;
        case GLFW_KEY_Z:
            myCamera.printPosition();
        case GLFW_KEY_1: // solid
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
        case GLFW_KEY_2: // wireframe
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            break;
        case GLFW_KEY_3: // poligonal
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            break;
        case GLFW_KEY_4: // scene presentation
            scenePresentation = !scenePresentation;
            myCamera.resetPosition();
            myCamera.rotate(cameraRotationToFollow.at(0).x, cameraRotationToFollow.at(0).y);
            currentBezierCurve = 0;
            scenePresentationIndex = 0.0f;
            break;
        default:
            break;
        }
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    float deltaX = (float)(xpos - previousXpos);
    float deltaY = (float)(ypos - previousYpos);

    previousXpos = xpos;
    previousYpos = ypos;

    yaw += deltaX * cameraSensivity;
    pitch -= deltaY * cameraSensivity;

    if (pitch > 89.0f)
        pitch = 89.0f;
    else if (pitch < -89.0f)
        pitch = -89.0f;

    myCamera.rotate(pitch, yaw);
}

void processMovement() {
    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_SPACE]) {
        myCamera.move(gps::MOVE_UP, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_LEFT_SHIFT]) {
        myCamera.move(gps::MOVE_DOWN, cameraSpeed);
    }
}

void initOpenGLWindow() {
    myWindow.Create(1920, 1080, "Proiect OpenGL");
    //for RETINA display
    glfwGetFramebufferSize(myWindow.getWindow(), &retina_width, &retina_height);
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    //glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED); /// dissable the cursor
}

void initModels() {
    terrain.LoadModel("objects\\terrain\\terrain.obj", "objects\\terrain\\");
    terrain_trees.LoadModel("objects\\terrain_trees\\terrain_trees.obj", "objects\\terrain_trees\\");
    eliceMoara.LoadModel("objects\\elice\\elice_moara.obj", "objects\\elice\\");
    bmwBody.LoadModel("objects\\bmw\\bmw_body.obj", "objects\\bmw\\");
    bmwTire.LoadModel("objects\\bmw\\bmw_tire.obj", "objects\\bmw\\");
    tree1.LoadModel("objects\\tree1\\Tree.obj", "objects\\tree1\\");
    tree2.LoadModel("objects\\tree2\\Tree.obj", "objects\\tree2\\");

    // skybox
    faces.push_back("textures/skybox/posx.jpg");
    faces.push_back("textures/skybox/negx.jpg");
    faces.push_back("textures/skybox/posy.jpg");
    faces.push_back("textures/skybox/negy.jpg");
    faces.push_back("textures/skybox/posz.jpg");
    faces.push_back("textures/skybox/negz.jpg");
    mySkyBox.Load(faces);
}

void initShaders() {
    basicShader.loadShader("shaders/basicShader.vert", "shaders/basicShader.frag");
    fragmentDiscardingShader.loadShader("shaders/fragmentDiscardingShader.vert", "shaders/fragmentDiscardingShader.frag");
    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    depthMapShader.loadShader("shaders/depthMap.vert", "shaders/depthMap.frag");
    screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
    screenQuad.LoadModel("objects/quad/quad.obj");
}

void initUniforms() {
    basicShader.useShaderProgram();

    // get view matrix for current camera
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(basicShader.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // create model matrix
    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(basicShader.shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // compute normal matrix
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(basicShader.shaderProgram, "normalMatrix");
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // create projection matrix
    projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(basicShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //set the light direction (direction towards the light)
    lightDir = glm::vec3(-80.0f, 100.0f, 0.0f);
    lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    lightDirLoc = glGetUniformLocation(basicShader.shaderProgram, "lightDir");
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

    //set light color
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
    lightColorLoc = glGetUniformLocation(basicShader.shaderProgram, "lightColor");
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));


    fragmentDiscardingShader.useShaderProgram();

    // get view matrix for current camera
    view = myCamera.getViewMatrix();
    viewLocFragmentDiscarding = glGetUniformLocation(fragmentDiscardingShader.shaderProgram, "view");
    glUniformMatrix4fv(viewLocFragmentDiscarding, 1, GL_FALSE, glm::value_ptr(view));

    // create model matrix
    model = glm::mat4(1.0f);
    modelLocFragmentDiscarding = glGetUniformLocation(fragmentDiscardingShader.shaderProgram, "model");
    glUniformMatrix4fv(modelLocFragmentDiscarding, 1, GL_FALSE, glm::value_ptr(model));

    // create projection matrix
    projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
    projectionLocFragmentDiscarding = glGetUniformLocation(fragmentDiscardingShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLocFragmentDiscarding, 1, GL_FALSE, glm::value_ptr(projection));
}

void initFBO() {
    glGenFramebuffers(1, &shadowMapFBO);
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {
    glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const GLfloat nearPlane = 0.1f, farPlane = 200.0f;
    glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, nearPlane, farPlane);
    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
    return lightSpaceTrMatrix;
}

void drawObjects(gps::Shader shader, bool depthPass) {
    shader.useShaderProgram();

    // do not send the normal matrix if we are rendering in the depth map
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    // update delta for car deplasament and rotation    
    double currentTimeStamp = glfwGetTime();
    updateDelta(currentTimeStamp - lastTimeStamp);
    lastTimeStamp = currentTimeStamp;

    // moving car
    model = glm::translate(glm::mat4(1.0f), glm::vec3(BODY_X_POS, 0.0f,delta));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    bmwBody.Draw(shader);
    model = glm::translate(glm::mat4(1.0f), glm::vec3(BODY_X_POS, TIRE_Y_PHASE, delta + FRONT_WHEEL_PHASE))*
        glm::rotate(glm::mat4(1.0f), glm::radians(delta*100), glm::vec3(1, 0, 0));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    bmwTire.Draw(shader);
    model = glm::translate(glm::mat4(1.0f), glm::vec3(BODY_X_POS, TIRE_Y_PHASE, delta + REAR_WHEEL_PHASE))*
        glm::rotate(glm::mat4(1.0f), glm::radians(delta*100), glm::vec3(1, 0, 0));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    bmwTire.Draw(shader);

    // elicea morii
    model = glm::translate(glm::mat4(1.0f), glm::vec3(2.2f, 15.0f, -14.5f))*
        glm::rotate(glm::mat4(1.0f), glm::radians(delta*15), glm::vec3(0, 0, 1));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    eliceMoara.Draw(shader);


    if (depthPass) { // copacii se deseneaza sepaarat, suntem interesati doar de umbra lor
        for (int i = 0; i < TREE_ARRAY_SIZE; i++) {
            model = glm::translate(glm::mat4(1.0f), glm::vec3(tree1Positions.at(i).x, -1.5f, tree1Positions.at(i).y)) * glm::scale(glm::vec3(3.0f));
            glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            tree1.Draw(shader);
            model = glm::translate(glm::mat4(1.0f), glm::vec3(tree2Positions.at(i).x, -1.5f, tree2Positions.at(i).y)) * glm::scale(glm::vec3(2.0f));
            glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            tree2.Draw(shader);
        }
    }
    
    // terrain
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.5f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // do not send the normal matrix if we are rendering in the depth map
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    terrain.Draw(shader);
}

void renderScene() {

    depthMapShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    drawObjects(depthMapShader, true);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (showDepthMap) {
        glViewport(0, 0, retina_width, retina_height);

        glClear(GL_COLOR_BUFFER_BIT);

        screenQuadShader.useShaderProgram();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

        glDisable(GL_DEPTH_TEST);
        screenQuad.Draw(screenQuadShader);
        glEnable(GL_DEPTH_TEST);
    }
    else {

        if (scenePresentation) {
            currentTime = glfwGetTime();
            if (currentTime > nextUpdateCameraMovement) {
                nextUpdateCameraMovement = currentTime + DELAY_TIME;
                updateCameraMovement();
            }
        }

        // final scene rendering
        glViewport(0, 0, retina_width, retina_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        basicShader.useShaderProgram();

        view = myCamera.getViewMatrix();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        //bind the shadow map
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(basicShader.shaderProgram, "shadowMap"), 3);

        glUniformMatrix4fv(glGetUniformLocation(basicShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));

        drawObjects(basicShader, false);

        fragmentDiscardingShader.useShaderProgram();
        glUniformMatrix4fv(glGetUniformLocation(fragmentDiscardingShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
        glUniformMatrix4fv(glGetUniformLocation(fragmentDiscardingShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        terrain_trees.Draw(fragmentDiscardingShader);

        for (int i = 0; i < TREE_ARRAY_SIZE; i++) {
            model = glm::translate(glm::mat4(1.0f), glm::vec3(tree1Positions.at(i).x, -1.5f, tree1Positions.at(i).y)) * glm::scale(glm::vec3(5.5f));
            glUniformMatrix4fv(glGetUniformLocation(fragmentDiscardingShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            tree1.Draw(fragmentDiscardingShader);
            model = glm::translate(glm::mat4(1.0f), glm::vec3(tree2Positions.at(i).x, -1.5f, tree2Positions.at(i).y)) * glm::scale(glm::vec3(3.5f));
            glUniformMatrix4fv(glGetUniformLocation(fragmentDiscardingShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            tree2.Draw(fragmentDiscardingShader);
        }
        
        skyboxShader.useShaderProgram();
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        projection = glm::perspective(glm::radians(45.0f), (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height, 0.1f, 1000.0f);
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        mySkyBox.Draw(skyboxShader, view, projection);
    }
}


void processSounds()
{
    shouldRun = true;
    while (shouldRun) {
        glm::vec3 pos = myCamera.getPosition();
        float distance = sqrt(pow(pos.x - BODY_X_POS, 2) + pow(pos.y, 2) + pow(pos.z - delta, 2));
        if (distance < 50.0f)
            PlaySound(TEXT("engine.wav"), NULL, SND_FILENAME);
        else
            Sleep(500);
    }
}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
    glDeleteTextures(1, &depthMapTexture);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &shadowMapFBO);
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
	initModels();
	initShaders();
	initUniforms();
    initFBO();
    setWindowCallbacks();

    std::thread t(processSounds);

	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
	    renderScene();
 
		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}
    shouldRun = false;
	cleanup();

    t.join();

    return EXIT_SUCCESS;
}
