#include <iostream>
#include <unistd.h>
#include <string>
#include <sstream>
#include <fstream>

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "stb_image.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "gameItem.h"

#define X glm::vec3(1.f,.0f,.0f)
#define Y glm::vec3(0.f,1.f,.0f)
#define Z glm::vec3(0.f,.0f,1.0f)
#define X1 glm::vec4(1.f,.0f,.0f,1.0f)
#define Y1 glm::vec4(0.f,1.f,.0f,1.0f)
#define Z1 glm::vec4(0.f,.0f,1.0f,1.0f)

#define TARGET_UPS 60.
#define SECOND_PER_UPDATE 1./TARGET_UPS

using namespace std;

void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

string readFile(const char* filename) {

    // 1. retrieve the shader source code from filePath
    std::string shaderCode;
    std::string fragmentCode;
    std::ifstream shaderFile;
    // ensure ifstream objects can throw exceptions:
    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        // open files
        shaderFile.open(filename);
        std::stringstream shaderStream;
        // read file's buffer contents into streams
        shaderStream << shaderFile.rdbuf();
        // close file handlers
        shaderFile.close();
        // convert stream into string
        shaderCode = shaderStream.str();
    }
    catch (std::ifstream::failure e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    return shaderCode;
}

glm::vec3 rotate3(glm::vec3 v, float angle, glm::vec3 axis) {

    glm::vec4 v4 = glm::vec4(v.x, v.y, v.z, 1.0f);
    v4 = glm::rotate(glm::mat4(1.0f), angle, axis) * v4;

    return glm::vec3(v4.x, v4.y, v4.z);
}
glm::vec3 normalize(glm::vec3 v) {
    float n = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (n < 0.000001) {
        return glm::vec3(0., 0., 0.);
    }
    return v / n;
}

GLFWwindow* initWindow(GLFWwindow* window, int width, int height, const char* name) {
    if (!glfwInit()) {
        cout << "The glfw window intialisation failed !" << endl;
        exit(-1);
    }
    glfwSetErrorCallback(error_callback);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    window = glfwCreateWindow(width, height, name, NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    return window;
}
void initIMGUI(GLFWwindow* window) {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls


    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chagsto existing ones.
    ImGui_ImplOpenGL3_Init();
}

struct windowParams {
    int width;
    int height;
    float ratio;
    windowParams() : width(1920), height(1080), ratio((float)this->width / (float)this->height) {}
}typedef windowParams;

struct mouseParams {
    glm::vec2 mouseSensivity;
    mouseParams() : mouseSensivity(glm::vec2(1.f, 1.f)) {}
}typedef mouseParams;

struct gameState {
    glm::vec2 mousePos;
    glm::vec2 lastMousePos;
    float forward;
    float sideways;
    float upwards;
    float running;
    bool key_ESCAPE;
    //hud parameters
    bool showFaces;
    bool showEdges;
    bool showBackSideEdges;
    bool showVertexIndices;
    bool showVertices;
    bool showNormals;
    bool backFaceCulling;
    float normalSize;
    bool debugMode;
    float speedOfTime;
    long int tick;
    bool isGamePaused;
    bool nextStep;
    gameItem* gameItems;
    int gameItemCount;
    unsigned int numberTexture;
    unsigned int shaderProgram;
    float fov;
    glm::vec4 clearColor;
    float getIngameTime() {
        return (float)this->tick * SECOND_PER_UPDATE;
    }
    gameState(gameItem* gameItems, int gameItemCount, unsigned int shaderProgram) :
        mousePos(glm::vec2(0.f)),
        lastMousePos(glm::vec2(0.)),
        forward(0.f),
        sideways(0.f),
        upwards(0.f),
        running(0.f),
        key_ESCAPE(false),
        //hud parameters
        showFaces(1),
        showEdges(0),
        showBackSideEdges(1),
        showVertexIndices(0),
        showVertices(0),
        showNormals(0),
        backFaceCulling(0),
        normalSize(1.),
        debugMode(0),
        //time control params
        speedOfTime(1.),
        tick(0),
        isGamePaused(false),
        nextStep(false),
        gameItems(gameItems),
        gameItemCount(gameItemCount),
        shaderProgram(shaderProgram),
        fov(60.),
        clearColor(glm::vec4(135. / 255., 209. / 255., 235 / 255., 1.)) {
        this->numberTexture = gameItem::loadTexture("numbers.png");
        glUseProgram(this->shaderProgram);
        glUniform1i(glGetUniformLocation(this->shaderProgram, "numbersTexture"), 0);
        glUniform1i(glGetUniformLocation(this->shaderProgram, "materialTexture"), 1);
    }
} typedef gameState;

struct camera {
    glm::vec3 position;
    glm::vec2 angleRotation;
    glm::vec3 relativeXAxis;
    glm::vec3 relativeZAxis;
    float speed;
    float runningSpeedFactor;
    void reset() {
        position = glm::vec3(0.f, 1.0f, 2.0f);
        angleRotation = glm::vec2(0., 0.);
        relativeZAxis = Z;
        relativeXAxis = X;
    }
    camera() :
        position(glm::vec3(0.f, 1.0f, 2.0f)),
        angleRotation(glm::vec2(0., 0.)),
        relativeZAxis(Z),
        speed(0.02f),
        runningSpeedFactor(5.0) {}

}typedef camera;

struct renderData {
    unsigned int VAO;
    unsigned int VAO2;
    unsigned int shaderProgram;
    unsigned int texture;
    renderData(unsigned int VAO, unsigned int VAO2, unsigned int shaderProgram, unsigned int texture) :
        VAO(VAO),
        VAO2(VAO2),
        shaderProgram(shaderProgram),
        texture(texture) {}
}typedef renderData;

unsigned int compileShader(const char* fileName, unsigned int shaderType) {

    unsigned int shader;
    shader = glCreateShader(shaderType);
    if (fileName == NULL) return shader;
    string sourceString = readFile(fileName);
    const char* shaderSource = sourceString.c_str();
    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);

    //check for compilation errors
    int  success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::COMPILATION_FAILED\n" << fileName << " : " << infoLog << endl;
    }
    return shader;
}
unsigned int buildShaderProgram(const char* vertexShaderFileName, const char* fragmentShaderFileName, const char* geometryShaderFileName = NULL) {
    unsigned int vertexShader = compileShader(vertexShaderFileName, GL_VERTEX_SHADER);
    unsigned int geometryShader = compileShader(geometryShaderFileName, GL_GEOMETRY_SHADER);
    unsigned int fragmentShader = compileShader(fragmentShaderFileName, GL_FRAGMENT_SHADER);

    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    if (geometryShaderFileName != NULL)
        glAttachShader(shaderProgram, geometryShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    //check for errors
    int  success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        cout << "ERROR LINKING SHADER PROGRAM : " << infoLog << endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(geometryShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}


bool onePressToggle(GLFWwindow* window, int key, bool* was_pressed, bool* toggle) {
    bool toggled = false;
    if (glfwGetKey(window, key) == GLFW_PRESS) {
        *was_pressed = true;
    }
    else if (*was_pressed) {
        *toggle = !*toggle;
        *was_pressed = false;
        toggled = true;
    }
    return toggled;
}
void processInputs(GLFWwindow* window, windowParams* wp, gameState* gs, mouseParams* mp, camera* cam) {
    glfwPollEvents();
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (glfwGetKey(window, GLFW_KEY_END) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (onePressToggle(window, GLFW_KEY_ESCAPE, &(gs->key_ESCAPE), &(gs->debugMode))) {
        if (gs->debugMode) {
            gs->lastMousePos = gs->mousePos;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwSetCursorPos(window, gs->lastMousePos.x, gs->lastMousePos.y);

        }
    }
    double mx, my;
    glfwGetCursorPos(window, &mx, &my);
    gs->mousePos = glm::vec2((float)mx * mp->mouseSensivity.x, (float)my * mp->mouseSensivity.x);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        gs->forward = 1;
    }
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        gs->forward = -1;
    }
    else {
        gs->forward = 0;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        gs->sideways = -1;
    }
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        gs->sideways = 1;
    }
    else {
        gs->sideways = 0;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        gs->upwards = 1;
    }
    else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        gs->upwards = -1;
    }
    else {
        gs->upwards = 0;
    }
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        gs->running = 1;
    }
    else {
        gs->running = 0;
    }

    //IMGUI inputs
    ImGui::Text("Use ESCAPE to enter debug mode");

    if (ImGui::TreeNodeEx("Hud")) {
        ImGui::Checkbox("Show faces", &(gs->showFaces));
        ImGui::Checkbox("Show vertex indices", &(gs->showVertexIndices));
        ImGui::Checkbox("Show vertices", &(gs->showVertices));
        ImGui::Checkbox("Show edges", &(gs->showEdges));
        ImGui::Checkbox("Show back side edges", &(gs->showBackSideEdges));
        ImGui::Checkbox("Show normals", &(gs->showNormals));
        if (ImGui::Checkbox("Back Face Culling", &(gs->backFaceCulling))) {
            if (gs->backFaceCulling) {
                glEnable(GL_CULL_FACE);
            }
            else {
                glDisable(GL_CULL_FACE);
            }
        }
        ImGui::SliderFloat("Normal size", &(gs->normalSize), 0.5, 10.);
        if (ImGui::ColorEdit4("Clear color", &(gs->clearColor.x))) {
            glClearColor(gs->clearColor.x, gs->clearColor.y, gs->clearColor.z, gs->clearColor.w);
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Camera")) {
        ImGui::SliderFloat("Camera speed", &(cam->speed), 0.001, 0.1);
        ImGui::SliderFloat("Camera running speed factor", &(cam->runningSpeedFactor), 0.1, 10.);
        ImGui::SliderFloat("FOV", &(gs->fov), 0, 180);
        if (ImGui::Button("Reset camera")) {
            cam->reset();
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Time")) {
        ImGui::SliderFloat("Speed of time", &(gs->speedOfTime), 0.1, 10);
        if (!gs->isGamePaused && ImGui::Button("Pause")) {
            gs->isGamePaused = true;
        }
        if (gs->isGamePaused) {
            if (ImGui::Button("Play")) {
                gs->isGamePaused = false;
            }
            if (ImGui::Button("Next")) {
                gs->nextStep = true;
            }
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Game Items")) {
        for (int i = 0;i < gs->gameItemCount;i++) {
            if (ImGui::TreeNodeEx(gs->gameItems[i].name)) {

                if (ImGui::TreeNodeEx("Scale")) {
                    ImGui::SliderFloat("Cube scale x", &(gs->gameItems[i].scale.x), 0.1, 10);
                    ImGui::SliderFloat("Cube scale y", &(gs->gameItems[i].scale.y), 0.1, 10);
                    ImGui::SliderFloat("Cube scale z", &(gs->gameItems[i].scale.z), 0.1, 10);
                    ImGui::TreePop();
                }

                if (ImGui::TreeNodeEx("Position")) {
                    ImGui::SliderFloat("Cube position x", &(gs->gameItems[i].position.x), -10, 10);
                    ImGui::SliderFloat("Cube position y", &(gs->gameItems[i].position.y), -10, 10);
                    ImGui::SliderFloat("Cube position z", &(gs->gameItems[i].position.z), -10, 10);
                    ImGui::TreePop();
                }

                if (ImGui::TreeNodeEx("Rotation")) {
                    ImGui::SliderFloat("Cube rotation axis x", &(gs->gameItems[i].rotationAxis.x), -1., 1.);
                    ImGui::SliderFloat("Cube rotation axis y", &(gs->gameItems[i].rotationAxis.y), -1., 1.);
                    ImGui::SliderFloat("Cube rotation axis z", &(gs->gameItems[i].rotationAxis.z), -1., 1.);
                    ImGui::SliderFloat("Cube rotation angle", &(gs->gameItems[i].rotationAngle), -3. * M_PI, 3. * M_PI);
                    ImGui::TreePop();
                }
                ImGui::ColorEdit4("Edges Color", &(gs->gameItems[i].edgesColor.x));

                ImGui::TreePop();
            }
        }
        ImGui::TreePop();

    }





}

void update(gameState* gs, windowParams* wp, camera* cam) {

    //Camera mouvement
    float camSpeed = cam->speed * (1.f + gs->running * (cam->runningSpeedFactor - 1.f));
    cam->position += normalize(cam->relativeZAxis * gs->forward + cam->relativeXAxis * gs->sideways + Y * gs->upwards) * camSpeed;

}


void render(GLFWwindow* window, windowParams* wp, camera* cam, gameState* gs) {

    //Camera orientation
    if (!gs->debugMode) {
        cam->angleRotation = glm::vec2(-(-gs->mousePos.x + 0.5f * wp->width) / wp->width, -(-gs->mousePos.y + 0.5f * wp->height) / wp->height);
        cam->relativeXAxis = rotate3(X, -cam->angleRotation.x, Y);
        cam->relativeZAxis = rotate3(cam->relativeXAxis, glm::radians(90.0), Y);
        cam->relativeZAxis = rotate3(cam->relativeZAxis, -cam->angleRotation.y, cam->relativeXAxis);
    }
    //camera setup
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    viewMatrix = glm::rotate(viewMatrix, cam->angleRotation.x, Y);
    viewMatrix = glm::rotate(viewMatrix, cam->angleRotation.y, cam->relativeXAxis);
    viewMatrix = glm::translate(viewMatrix, -cam->position);
    glm::mat4 projMatrix = glm::perspective(glm::radians(gs->fov), wp->ratio, 0.0001f, 100.0f);
    //update uniform variables
    glUseProgram(gs->shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(gs->shaderProgram, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(gs->shaderProgram, "projMatrix"), 1, GL_FALSE, glm::value_ptr(projMatrix));
    glUniform1f(glGetUniformLocation(gs->shaderProgram, "ratio"), wp->ratio);
    glUniform1i(glGetUniformLocation(gs->shaderProgram, "showBackSideEdges"), gs->showBackSideEdges);
    glUniform1i(glGetUniformLocation(gs->shaderProgram, "showNormals"), gs->showNormals);
    glUniform1f(glGetUniformLocation(gs->shaderProgram, "normalSize"), gs->normalSize);
    glUniform1i(glGetUniformLocation(gs->shaderProgram, "showVertexIndices"), gs->showVertexIndices);
    glUniform1i(glGetUniformLocation(gs->shaderProgram, "showVertices"), gs->showVertices);
    glUniform3fv(glGetUniformLocation(gs->shaderProgram, "camPos"), 1, glm::value_ptr(cam->position));
    glUniform1f(glGetUniformLocation(gs->shaderProgram, "time"), gs->getIngameTime());

    //Draw
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gs->numberTexture);
    glActiveTexture(GL_TEXTURE1);

    for (int i = 0; i < gs->gameItemCount; i++) {
        glm::mat4 modelMatrix = glm::mat4(1.0);
        modelMatrix = glm::translate(modelMatrix, -gs->gameItems[i].position);
        modelMatrix = glm::rotate(modelMatrix, gs->gameItems[i].rotationAngle, gs->gameItems[i].rotationAxis);
        modelMatrix = glm::scale(modelMatrix, gs->gameItems[i].scale);

        glUniform1i(glGetUniformLocation(gs->shaderProgram, "isEdge"), false);
        glUniform4fv(glGetUniformLocation(gs->shaderProgram, "edgesColor"), 1, glm::value_ptr(gs->gameItems[i].edgesColor));
        glUniformMatrix4fv(glGetUniformLocation(gs->shaderProgram, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glBindVertexArray(gs->gameItems[i].VAO);
        if (gs->showFaces) {
            glBindTexture(GL_TEXTURE_2D, gs->gameItems[i].texture);
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(1.0f, 1.0f);
            glDrawElements(GL_TRIANGLES, gs->gameItems[i].indexCount, GL_UNSIGNED_INT, 0);
            glDisable(GL_POLYGON_OFFSET_FILL);
        }
        if (gs->showEdges) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glUniform1i(glGetUniformLocation(gs->shaderProgram, "isEdge"), true);
            glDrawElements(GL_TRIANGLES, gs->gameItems[i].indexCount, GL_UNSIGNED_INT, 0);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        }
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
}

int main() {
    GLFWwindow* window;
    int width = 1850, height = 1080;
    window = initWindow(window, width, height, "OpenGL-Base-Project");

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    initIMGUI(window);

    //----------------------------------------------------------------------------------------- MESH DATA
    float vertices[] = {
        //positions             texCoords
         0.5f,  0.5f, 0.5f,     1.f,0.f,         // top right 
         0.5f, -0.5f, 0.5f,     1.f,1.f,        // bottom right
        -0.5f, -0.5f, 0.5f,     0.f,1.f,         // bottom left
        -0.5f,  0.5f, 0.5f,     0.f,0.f,        // top left 
        //Back
         0.5f,  0.5f, -0.5f,     1.,0.,        // top right
         0.5f, -0.5f, -0.5f,     1.,1.,       // bottom right
        -0.5f, -0.5f, -0.5f,     0.,1.,       // bottom left
        -0.5f,  0.5f, -0.5f,      0.,0.,       // top left */
    };
    unsigned int indices[] = {  // cube faces
        0, 3, 1,    //front
        1, 3, 2,
        1, 2, 6,    //botom 
        1, 6, 5,
        5, 6, 7,    //back
        4, 5, 7,
        0, 5, 4,    //right  
        0, 1, 5,
        2, 3, 6,    //left
        7, 6, 3,
        0, 7, 3,    //top
        0, 4, 7,//*/

    };
    float vertices2[] = {
        10, 0, 10,    1, 1,
        10, 0, -10,   1, 0,
        -10, 0, 10,   0, 1,
        -10, 0, -10,  0, 0
    };
    unsigned int indices2[] = {
        0, 1, 2,
        1, 3, 2,
    };
    //-----------------------------------------------------------------------------------------

    unsigned int shaderProgram = buildShaderProgram("./vertexShader.glsl", "./fragmentShader.glsl", "./geometryShader.glsl");

    glClearColor(135. / 255., 209. / 255., 235 / 255., 1.);
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);

    gameItem cube("Cube", vertices, sizeof(vertices) / sizeof(float), indices, sizeof(indices) / sizeof(int), "Carre.png");
    gameItem floor("Floor", vertices2, sizeof(vertices2) / sizeof(float), indices2, sizeof(indices2) / sizeof(int), "damier.png");
    int gameItemCount = 2;
    gameItem gameItems[] = { cube , floor };

    gameState gs = gameState(gameItems, gameItemCount, shaderProgram);
    mouseParams mp = mouseParams();
    windowParams wp = windowParams();
    camera cam = camera();


    double previous = glfwGetTime();
    double lag = 0.;
    while (!glfwWindowShouldClose(window)) { //------------------------------------------------------------------------------------------LOOP

        double current = glfwGetTime();
        double elapsed = current - previous;
        previous = current;
        lag += elapsed;

        processInputs(window, &wp, &gs, &mp, &cam);

        int counter = 0;
        double t1 = glfwGetTime();   // NEED average update time < SECOND_PER_UPDATE 
        while ((lag >= SECOND_PER_UPDATE && gs.speedOfTime > 0 && !gs.isGamePaused) || (gs.isGamePaused && gs.nextStep)) {
            counter++;
            gs.tick++;
            update(&gs, &wp, &cam);
            lag -= SECOND_PER_UPDATE / gs.speedOfTime;
            gs.nextStep = false;
        }
        double t2 = glfwGetTime();

        ImGui::Text("FPS : %f \nupdates per frame : %d\naverage update time : %f\nSECOND_PER_UPDATE : %f",
            1. / elapsed, counter, (counter == 0 ? 0 : (t2 - t1) / (float)counter), SECOND_PER_UPDATE);

        render(window, &wp, &cam, &gs);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteTextures(1, &(gs.numberTexture));
    for (int i = 0; i < gameItemCount;i++) {
        glDeleteTextures(1, &(gs.gameItems[i].texture));
        glDeleteBuffers(1, &(gs.gameItems[i].EBO));
        glDeleteBuffers(1, &(gs.gameItems[i].VBO));
        glDeleteVertexArrays(1, &(gs.gameItems[i].VAO));
    }

    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}

