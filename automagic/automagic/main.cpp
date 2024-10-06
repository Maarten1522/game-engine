#include <cmath>
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include <chrono>
#include <thread>
#include <string>
#include <iomanip>
#include <cstdint>
#include <cstdlib>
#include <sstream>
#include "camera.h"
#include <iostream>
#include "Texture.h"
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shaderClass.h"
#include <stb/stb_image.h>
#include <glm/gtc/type_ptr.hpp>
#include "vendor/imgui/imgui.h"
#include <glm/gtc/matrix_transform.hpp>
#include "vendor/imgui/imgui_impl_glfw.h"
#include "vendor/imgui/imgui_impl_opengl3.h"

GLfloat vertices[] = {
    //cords                 // colors                   //texture
    0.5f,  0.5f, 0.0f,      1.0f, 0.0f, 0.0f, 1.0f,     1.0f,  1.0f,       /*  achter rechts boven  0 */
    0.5f, -0.5f, 0.0f,      0.0f, 1.0f, 0.0f, 1.0f,     1.0f,  0.0f,       /*  achter rechts onder  1 */
   -0.5f, -0.5f, 0.0f,      0.0f, 0.0f, 1.0f, 1.0f,     0.0f,  0.0f,       /*  achter links onder   2 */
   -0.5f,  0.5f, 0.0f,      1.0f, 1.0f, 1.0f, 1.0f,     0.0f,  1.0f,       /*  achter links boven   3 */
   -0.5f,  0.5f, 1.0f,      0.3f, 0.7f, 0.6f, 1.0f,    -1.0f,  1.0f,       /*  links links boven    4 */
   -0.5f, -0.5f, 1.0f,      0.0f, 0.0f, 1.0f, 1.0f,    -1.0f,  0.0f,       /*  links links onder    5 */
    0.5f,  0.5f, 1.0f,      0.0f, 0.0f, 0.0f, 1.0f,     1.0f,  1.0f,       /*  rechts rechts boven  6 */
    0.5f, -0.5f, 1.0f,      0.0f, 0.0f, 0.0f, 0.0f,     1.0f,  0.0f        /*  rechts rechts onder  7 */
};

GLuint indices[] = {
    0, 1, 3,        /*  first  back   Triangle  */
    1, 2, 3,        /*  second back   Triangle  */
    5, 6, 7,        /*  first  front  Triangle  */
    5, 4, 6,        /*  second front  Triangle  */
    4, 2, 3,        /*  first   left  Triangle  */
    4, 5, 2,        /*  second  left  Triangle  */
    6, 0, 1,        /*  first   right Triangle  */
    7, 6, 1,        /*  second  right Triangle  */
    5, 7, 1,        /*  first  bottom Triangle  */
    5, 2, 1,        /*  second bottom Triangle  */
    3, 6, 0,        /*  first  top    Triangle  */
    3, 6, 4         /*  second top    Triangle  */
};

namespace constants
{
    static bool GUI = false;
    static bool polymode = false;
    int16_t fpsCount = 0;
    int16_t window_x = 1100;
    int16_t window_y = 800;
    float fpsTime = glfwGetTime();
    int16_t frames;
    int16_t fps;
}
static void print(std::string printy)
{
    std::cout << printy << "\n";
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) 
{
    glViewport(0, 0, width, height);
}
static void processInput(GLFWwindow* window)
{
    using namespace constants;

    if (polymode) 
        {glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);}
    else 
        {glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);}

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        GUI = !GUI;
        while (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwPollEvents();
        }
    }

    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
        while (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
            glfwPollEvents();
        }
    }
}
static GLFWwindow* initialize() {
    using namespace constants;
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(window_x, window_y, "Automation Panel", NULL, NULL);
    if (window == NULL) 
    {
        std::cout << "LOL u even failed to create a window" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    gladLoadGL();
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) 
    {
        std::cout << "how do you evn failed to initialize GLAD" << std::endl;
        return nullptr;
    }

    GLFWimage images[1];
    images[0].pixels = stbi_load("automagic/assets/images/logo.png", &images[0].width, &images[0].height, 0, 4); //rgba channels 
    glfwSetWindowIcon(window, 1, images);
    stbi_image_free(images[0].pixels);
    return window;
}

int main() {
    using std::string;
    using std::cin;
    using std::cout;
    using namespace constants;

    GLFWwindow* window = initialize();
    if (window == nullptr) 
    {
        std::cout << "LOL u stupid\n u can`t even initialize a window. Exiting..." << std::endl;
        return -1;
    }

    Shader shaderProgram("automagic/assets/shaders/default.vert", "automagic/assets/shaders/default.frag");

    VAO VAO1;
    VAO1.Bind();
    VBO VBO1(vertices, sizeof(vertices));
    EBO EBO1(indices, sizeof(indices));
    VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 9 * sizeof(float), (void*)0);
    VAO1.LinkAttrib(VBO1, 1, 4, GL_FLOAT, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    VAO1.LinkAttrib(VBO1, 2, 2, GL_FLOAT, 9 * sizeof(float), (void*)(7 * sizeof(float)));
    VAO1.Unbind();
    VBO1.Unbind();
    EBO1.Unbind();

    /*  incase jpg its GL_RGB in case png its GL_RGBA   */
    Texture WALL("automagic/assets/images/wall.jpg", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_UNSIGNED_BYTE);
    WALL.texUnit(shaderProgram, "tex0", 0);

    glEnable(GL_DEPTH_TEST);
    Camera camera(window_x, window_y, glm::vec3(0.0f, 0.0f, 2.0f));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    while (!glfwWindowShouldClose(window)) 
    {
        frames++;
        if (glfwGetTime() - 1 >= fpsTime) 
        {
            fps = frames;
            frames = 0;
            printf("FPS: %d\n", fps);
            fpsTime = glfwGetTime();
        }
        processInput(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        shaderProgram.use();

        if (!GUI) {camera.Inputs(window);}
        camera.Matrix(45.0f, 0.1f, 100.0f, shaderProgram, "camMatrix");
        WALL.Bind();
        VAO1.Bind();
        glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(int), GL_UNSIGNED_INT, 0);

        if (GUI)
        {
            ImGui::Begin("Hello!, Maarten");
            ImGui::Text("jij bent geweldig, je hebt gwn geen errors");
            ImGui::Text("you have %d",fps);
            ImGui::Text("------------------------------------------");
            ImGui::Checkbox("wireframe", &polymode);
            ImGui::End();
        }
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    VAO1.Delete();
    VBO1.Delete();
    EBO1.Delete();
    WALL.Delete();
    shaderProgram.Delete();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}