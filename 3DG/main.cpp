/*
	3D Graphics Start Point

	I have tried to provide the minimum amount of code to get you started. The KeithHelpers file
	provides some useful functions for loading and compiling shaders, loading files into strings etc.
	Feel free to replace this with your own code if you prefer and please check it over to see what it does.

	There are a number of libraries included:

	GLFW: https://www.glfw.org/ - this provides a simple API for creating windows,
	GLEW: http://glew.sourceforge.net/ - The OpenGL Extension Wrangler Library.
	GLM: https://glm.g-truc.net/0.9.9/index.html - OpenGL Mathematics, a header only C++ mathematics library for graphics software.
	FreeImage: http://freeimage.sourceforge.net/ - used to load image files
	Assimp: https://www.assimp.org/ - A 3D model loader library
	IMGUI: https://github.com/ocornut/imgui - a bloat-free graphical user interface library for C++

	Keith (September 2025)
*/

// Keith Helpers - also includes headers needed for the libraries used
#include "Keith Helpers.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <algorithm>
#include <cmath>


GLFWwindow* CreateGLFWWindow(int width, int height, const std::string& title);
void Render(GLFWwindow* window);
void CreateCube();
void CreateSkybox();
void LoadCubemap();
void CreateShaders();
void CreateSkyboxShaders();
void DefineGUI();
void InitializeLighting();
void UpdateSpotlightPosition();

//cube variables
GLuint gCubeVAO{ 0 };
GLuint gShaderProgram{ 0 };
GLuint gNumIndices = 36; //each face is 2 triangles, 6 vertices per face

//skybox variables
GLuint gSkyboxVAO{ 0 };
GLuint gSkyboxVBO{ 0 };
GLuint gSkyboxTexture{ 0 };
GLuint gSkyboxShaderProgram{ 0 };
bool gShowSkybox = true;

//lighting variables
struct DirectionalLight {
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

struct Spotlight {
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
};

DirectionalLight gDirLight;
PointLight gPointLight;
Spotlight gSpotlight;
bool gShowDirLight = true;
bool gShowPointLight = false;
bool gShowSpotlight = false;

//material properties
glm::vec3 gMaterialAmbient = glm::vec3(0.2f, 0.2f, 0.2f);
glm::vec3 gMaterialDiffuse = glm::vec3(0.8f, 0.8f, 0.8f);
glm::vec3 gMaterialSpecular = glm::vec3(1.0f, 1.0f, 1.0f);
float gMaterialShininess = 32.0f;

//cube coloring options
bool gUsePerFaceColors = false;  //toggle between colour selection or basic 6 colours for cube
glm::vec3 gCubeColor = glm::vec3(1.0f, 1.0f, 1.0f); //default white

//camera for spotlight
glm::vec3 gCameraPos = glm::vec3(0.0f, 0.0f, 3.0f);

//rotation
float gRotationX = 0.0f;
float gRotationY = 0.0f;
float gRotationSpeedX = 0.5f;
float gRotationSpeedY = 0.5f;
bool gAutoRotate = true;

int main()
{
    GLFWwindow* window{ CreateGLFWWindow(1280, 720, "3DG") };
    if (!window)
        return -1;

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Initialize lighting
    InitializeLighting();

    CreateCube();
    CreateSkybox();
    LoadCubemap();
    CreateShaders();
    CreateSkyboxShaders();

    // Enter main loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        // Handle spotlight following camera
        UpdateSpotlightPosition();

        Render(window);

        // GLFW updating - internally swaps the front and back buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // You should delete any OpenGL resources here
    glDeleteProgram(gShaderProgram);
    glDeleteProgram(gSkyboxShaderProgram);
    glDeleteVertexArrays(1, &gCubeVAO);
    glDeleteVertexArrays(1, &gSkyboxVAO);
    glDeleteBuffers(1, &gSkyboxVBO);
    glDeleteTextures(1, &gSkyboxTexture);

    // Clean up and exit
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void InitializeLighting()
{
    //directional ligh
    gDirLight.direction = glm::vec3(-0.2f, -1.0f, -0.3f);
    gDirLight.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
    gDirLight.diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
    gDirLight.specular = glm::vec3(1.0f, 1.0f, 1.0f);

    //point light
    gPointLight.position = glm::vec3(1.5f, 0.0f, 0.0f);
    gPointLight.ambient = glm::vec3(0.1f, 0.1f, 0.1f);
    gPointLight.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
    gPointLight.specular = glm::vec3(1.0f, 1.0f, 1.0f);
    gPointLight.constant = 1.0f;
    gPointLight.linear = 0.09f;
    gPointLight.quadratic = 0.032f;

    //spotlight
    gSpotlight.position = gCameraPos;
    gSpotlight.direction = glm::vec3(0.0f, 0.0f, -1.0f);
    gSpotlight.ambient = glm::vec3(0.0f, 0.0f, 0.0f);
    gSpotlight.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
    gSpotlight.specular = glm::vec3(1.0f, 1.0f, 1.0f);
    gSpotlight.cutOff = glm::cos(glm::radians(12.5f));
    gSpotlight.outerCutOff = glm::cos(glm::radians(17.5f));
    gSpotlight.constant = 1.0f;
    gSpotlight.linear = 0.09f;
    gSpotlight.quadratic = 0.032f;
}

void UpdateSpotlightPosition()
{
    //update spotlight to follow camera
    gSpotlight.position = gCameraPos;
}

// Uses GLFW to set up a window via GLFW. Also initialises GLEW and OpenGL.
GLFWwindow* CreateGLFWWindow(int width, int height, const std::string& title)
{
    if (!glfwInit())
    {
        std::cout << "Failed to initialise GLFW" << std::endl;
        return nullptr;
    }

    std::cout << "GLFW initialised" << std::endl;

    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 0);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // We want OpenGL 4.6 minimum
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL 

    glEnable(GL_MULTISAMPLE);

#ifdef _DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

    GLFWwindow* window{ glfwCreateWindow(width, height, title.c_str(), NULL, NULL) };
    if (!window)
    {
        std::cout << "Failed to create window" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);

    glEnable(GL_MULTISAMPLE);

    glewExperimental = true; // Needed in core profile

    std::cout << "GLFW window initialised" << std::endl;

    GLenum err{ glewInit() };
    if (GLEW_OK != err)
    {

        auto errs = glewGetErrorString(err);

        std::cout << "Failed to initialise GLEW, Error: " << errs << std::endl;

        return nullptr;
    }

    std::cout << "GLEW initialised" << std::endl;

    // Enable trapping of OpenGL errors via a callback
    // the callback is in KeithHelpers.cpp
#if defined(_DEBUG)			
    int flags{ 0 };
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        // initialize debug output 
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(KeithHelpers::glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
#endif

    // Disable V-Sync for speed
    glfwSwapInterval(0);

    // Setup Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style and Platform/Renderer bindings
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    std::cout << "IMGUI initialised" << std::endl;

    return window;
}

void CreateCube()
{
    std::vector<float> verts = {
        //front 
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,

         //back
          0.5f, -0.5f, -0.5f,
         -0.5f, -0.5f, -0.5f,
          0.5f,  0.5f, -0.5f,
         -0.5f,  0.5f, -0.5f,

         //top
         -0.5f,  0.5f,  0.5f,
          0.5f,  0.5f,  0.5f,
         -0.5f,  0.5f, -0.5f,
          0.5f,  0.5f, -0.5f,

          //bottom
          -0.5f, -0.5f, -0.5f,
           0.5f, -0.5f, -0.5f,
          -0.5f, -0.5f,  0.5f,
           0.5f, -0.5f,  0.5f,

           //right
            0.5f, -0.5f,  0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f,  0.5f,  0.5f,
            0.5f,  0.5f, -0.5f,

            //left
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f,  0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f,  0.5f,  0.5f
    };

    //normals for Phong shading
    std::vector<float> normals = {
        //front
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,

        //back
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,

        //top
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,

        //bottom
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,

        //right
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,

        //left
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f
    };

    std::vector<unsigned int> indices = {
        //front 
        0, 1, 2,  2, 1, 3,

        //back 
        4, 5, 6,  6, 5, 7,
        
        //top 
        8, 9, 10,  10, 9, 11,

        //bottom
        12, 13, 14,  14, 13, 15,

        //right
        16, 17, 18,  18, 17, 19,

        //left
        20, 21, 22,  22, 21, 23
    };

    // Original per-face colors (from the original code)
    std::vector<float> perFaceColours = {
        //front - red
        1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,

        //back - green
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,

        //top - blue
        0.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,

        //bottom - yellow
        1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f,

        //right - magenta
        1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 1.0f,

        //left - cyan
        0.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f, 1.0f,
    };

    std::vector<float> singleColour;
    for (int i = 0; i < 24; i++) 
    { 
        singleColour.push_back(gCubeColor.r);
        singleColour.push_back(gCubeColor.g);
        singleColour.push_back(gCubeColor.b);
        singleColour.push_back(1.0f);
    }

    GLuint VBO, EBO, VBOcolour, VBOnormal;

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    //vertex positions
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    //normals
    glGenBuffers(1, &VBOnormal);
    glBindBuffer(GL_ARRAY_BUFFER, VBOnormal);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0);
    glEnableVertexAttribArray(1);

    //colors 
    glGenBuffers(1, &VBOcolour);
    glBindBuffer(GL_ARRAY_BUFFER, VBOcolour);
    glBufferData(GL_ARRAY_BUFFER, perFaceColours.size() * sizeof(float), perFaceColours.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)0);
    glEnableVertexAttribArray(2);

    //indices
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    gCubeVAO = VAO;
    gNumIndices = indices.size();
}

void UpdateCubeColors()
{
    glBindVertexArray(gCubeVAO);

    GLuint VBOcolour;
    glGenBuffers(1, &VBOcolour);

    if (gUsePerFaceColors) 
    {
        std::vector<float> perFaceColours = {
            //front - red
            1.0f, 0.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 0.0f, 1.0f,

            //back - green
            0.0f, 1.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f,

            //top - blue
            0.0f, 0.0f, 1.0f, 1.0f,
            0.0f, 0.0f, 1.0f, 1.0f,
            0.0f, 0.0f, 1.0f, 1.0f,
            0.0f, 0.0f, 1.0f, 1.0f,

            //bottom - yellow
            1.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 0.0f, 1.0f,

            //right - magenta
            1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 1.0f,

            //left - cyan
            0.0f, 1.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 1.0f, 1.0f,
        };

        glBindBuffer(GL_ARRAY_BUFFER, VBOcolour);
        glBufferData(GL_ARRAY_BUFFER, perFaceColours.size() * sizeof(float), perFaceColours.data(), GL_STATIC_DRAW);
    }
    else 
    {
        std::vector<float> singleColour;
        for (int i = 0; i < 24; i++) 
        {
            singleColour.push_back(gCubeColor.r);
            singleColour.push_back(gCubeColor.g);
            singleColour.push_back(gCubeColor.b);
            singleColour.push_back(1.0f);
        }

        glBindBuffer(GL_ARRAY_BUFFER, VBOcolour);
        glBufferData(GL_ARRAY_BUFFER, singleColour.size() * sizeof(float), singleColour.data(), GL_STATIC_DRAW);
    }

    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)0);
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    glDeleteBuffers(1, &VBOcolour);
}

void CreateSkybox()
{
    //skybox vertices (position only)
    float skyboxVertices[] = {
        //positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    glGenVertexArrays(1, &gSkyboxVAO);
    glGenBuffers(1, &gSkyboxVBO);
    glBindVertexArray(gSkyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gSkyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void LoadCubemap()
{
    std::vector<std::string> faces = {
        "Data/Models/Sky/Clouds/SkyBox_Right.tga",   
        "Data/Models/Sky/Clouds/SkyBox_Left.tga",   
        "Data/Models/Sky/Clouds/SkyBox_Top.tga",     
        "Data/Models/Sky/Clouds/SkyBox_Bottom.tga",  
        "Data/Models/Sky/Clouds/SkyBox_Front.tga",   
        "Data/Models/Sky/Clouds/SkyBox_Back.tga"     
    };

    glGenTextures(1, &gSkyboxTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, gSkyboxTexture);

    // Load each face
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        std::cout << "Loading skybox texture: " << faces[i] << std::endl;

        unsigned char placeholder[4] = { 128, 128, 255, 255 }; 
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA,
            1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, placeholder);
    }

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void CreateShaders()
{
    //create Phong shader
    const char* phongVertexShaderSource = R"(
        #version 460 core
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 normal;
        layout(location = 2) in vec4 color;
        
        out vec3 FragPos;
        out vec3 Normal;
        out vec4 VertexColor;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        
        void main()
        {
            FragPos = vec3(model * vec4(position, 1.0));
            Normal = mat3(transpose(inverse(model))) * normal;
            VertexColor = color;
            gl_Position = projection * view * vec4(FragPos, 1.0);
        }
    )";

    const char* phongFragmentShaderSource = R"(
        #version 460 core
        in vec3 FragPos;
        in vec3 Normal;
        in vec4 VertexColor;
        
        out vec4 fragment_colour;
        
        // Material properties
        uniform vec3 materialAmbient;
        uniform vec3 materialDiffuse;
        uniform vec3 materialSpecular;
        uniform float materialShininess;
        
        // Color mode toggle
        uniform bool useVertexColors;
        
        // Directional light
        struct DirLight {
            vec3 direction;
            vec3 ambient;
            vec3 diffuse;
            vec3 specular;
        };
        uniform DirLight dirLight;
        uniform bool dirLightEnabled;
        
        // Point light
        struct PointLight {
            vec3 position;
            vec3 ambient;
            vec3 diffuse;
            vec3 specular;
            float constant;
            float linear;
            float quadratic;
        };
        uniform PointLight pointLight;
        uniform bool pointLightEnabled;
        
        // Spotlight
        struct SpotLight {
            vec3 position;
            vec3 direction;
            vec3 ambient;
            vec3 diffuse;
            vec3 specular;
            float cutOff;
            float outerCutOff;
            float constant;
            float linear;
            float quadratic;
        };
        uniform SpotLight spotLight;
        uniform bool spotLightEnabled;
        
        // Camera position
        uniform vec3 viewPos;
        
        // Function prototypes
        vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 baseColor);
        vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor);
        vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor);
        
        void main()
        {
            // Properties
            vec3 norm = normalize(Normal);
            vec3 viewDir = normalize(viewPos - FragPos);
            
            // Determine base color based on mode
            vec3 baseColor;
            if (useVertexColors) {
                baseColor = VertexColor.rgb;
            } else {
                baseColor = materialDiffuse;
            }
            
            vec3 result = vec3(0.0);
            
            // Calculate directional lighting
            if (dirLightEnabled) {
                result += CalcDirLight(dirLight, norm, viewDir, baseColor);
            }
            
            // Calculate point lighting
            if (pointLightEnabled) {
                result += CalcPointLight(pointLight, norm, FragPos, viewDir, baseColor);
            }
            
            // Calculate spotlight lighting
            if (spotLightEnabled) {
                result += CalcSpotLight(spotLight, norm, FragPos, viewDir, baseColor);
            }
            
            fragment_colour = vec4(result, 1.0);
        }
        
        vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 baseColor)
        {
            vec3 lightDir = normalize(-light.direction);
            
            // Diffuse shading
            float diff = max(dot(normal, lightDir), 0.0);
            
            // Specular shading
            vec3 reflectDir = reflect(-lightDir, normal);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);
            
            // Combine results
            vec3 ambient = light.ambient * materialAmbient;
            vec3 diffuse = light.diffuse * diff * baseColor;
            vec3 specular = light.specular * spec * materialSpecular;
            
            return (ambient + diffuse + specular);
        }
        
        vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor)
        {
            vec3 lightDir = normalize(light.position - fragPos);
            
            // Diffuse shading
            float diff = max(dot(normal, lightDir), 0.0);
            
            // Specular shading
            vec3 reflectDir = reflect(-lightDir, normal);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);
            
            // Attenuation
            float distance = length(light.position - fragPos);
            float attenuation = 1.0 / (light.constant + light.linear * distance + 
                                     light.quadratic * (distance * distance));
            
            // Combine results
            vec3 ambient = light.ambient * materialAmbient;
            vec3 diffuse = light.diffuse * diff * baseColor;
            vec3 specular = light.specular * spec * materialSpecular;
            
            ambient *= attenuation;
            diffuse *= attenuation;
            specular *= attenuation;
            
            return (ambient + diffuse + specular);
        }
        
        vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor)
        {
            vec3 lightDir = normalize(light.position - fragPos);
            
            // Diffuse shading
            float diff = max(dot(normal, lightDir), 0.0);
            
            // Specular shading
            vec3 reflectDir = reflect(-lightDir, normal);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);
            
            // Attenuation
            float distance = length(light.position - fragPos);
            float attenuation = 1.0 / (light.constant + light.linear * distance + 
                                     light.quadratic * (distance * distance));
            
            // Spotlight intensity
            float theta = dot(lightDir, normalize(-light.direction));
            float epsilon = light.cutOff - light.outerCutOff;
            float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
            
            // Combine results
            vec3 ambient = light.ambient * materialAmbient;
            vec3 diffuse = light.diffuse * diff * baseColor;
            vec3 specular = light.specular * spec * materialSpecular;
            
            ambient *= attenuation * intensity;
            diffuse *= attenuation * intensity;
            specular *= attenuation * intensity;
            
            return (ambient + diffuse + specular);
        }
    )";

    //compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &phongVertexShaderSource, NULL);
    glCompileShader(vertexShader);

    //check for compile errors
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    //compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &phongFragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    //check for compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    //create shader program
    gShaderProgram = glCreateProgram();
    glAttachShader(gShaderProgram, vertexShader);
    glAttachShader(gShaderProgram, fragmentShader);
    glLinkProgram(gShaderProgram);

    //check for linking errors
    glGetProgramiv(gShaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(gShaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    //clean up shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void CreateSkyboxShaders()
{
    //create skybox vertex shader
    const char* skyboxVertexShaderSource = R"(
        #version 460 core
        layout(location = 0) in vec3 position;
        out vec3 TexCoords;
        uniform mat4 projection;
        uniform mat4 view;
        void main()
        {
            TexCoords = position;
            vec4 pos = projection * mat4(mat3(view)) * vec4(position, 1.0);
            gl_Position = pos.xyww;
        }
    )";

    //create skybox fragment shader
    const char* skyboxFragmentShaderSource = R"(
        #version 460 core
        in vec3 TexCoords;
        out vec4 fragment_colour;
        uniform samplerCube skybox;
        void main()
        {
            fragment_colour = texture(skybox, TexCoords);
        }
    )";

    //compile skybox shaders
    GLuint skyboxVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(skyboxVertexShader, 1, &skyboxVertexShaderSource, NULL);
    glCompileShader(skyboxVertexShader);

    GLuint skyboxFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(skyboxFragmentShader, 1, &skyboxFragmentShaderSource, NULL);
    glCompileShader(skyboxFragmentShader);

    //create and link skybox shader program
    gSkyboxShaderProgram = glCreateProgram();
    glAttachShader(gSkyboxShaderProgram, skyboxVertexShader);
    glAttachShader(gSkyboxShaderProgram, skyboxFragmentShader);
    glLinkProgram(gSkyboxShaderProgram);

    //clean up shaders
    glDeleteShader(skyboxVertexShader);
    glDeleteShader(skyboxFragmentShader);
}

void Render(GLFWwindow* window)
{
    //clear the screen each time
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //update rotation
    static float lastTime = 0.0f;
    float currentTime = static_cast<float>(glfwGetTime());
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    if (gAutoRotate)
    {
        gRotationX += gRotationSpeedX * deltaTime;
        gRotationY += gRotationSpeedY * deltaTime;
    }

    //set up matrices
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, gRotationX, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, gRotationY, glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 view = glm::lookAt(
        gCameraPos, 
        glm::vec3(0.0f, 0.0f, 0.0f), //look at pos
        glm::vec3(0.0f, 1.0f, 0.0f)  //up vector
    );

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    float aspect = static_cast<float>(width) / static_cast<float>(height);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

    // Render skybox
    if (gShowSkybox)
    {
        // Change depth function so depth test passes when values are equal to depth buffer's content
        glDepthFunc(GL_LEQUAL);

        // Use skybox shader
        glUseProgram(gSkyboxShaderProgram);

        // Set matrices (remove translation from view matrix for skybox)
        glm::mat4 viewWithoutTranslation = glm::mat4(glm::mat3(view));
        GLuint viewLoc = glGetUniformLocation(gSkyboxShaderProgram, "view");
        GLuint projLoc = glGetUniformLocation(gSkyboxShaderProgram, "projection");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewWithoutTranslation));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Bind skybox texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, gSkyboxTexture);
        GLuint skyboxLoc = glGetUniformLocation(gSkyboxShaderProgram, "skybox");
        glUniform1i(skyboxLoc, 0);

        // Draw skybox
        glBindVertexArray(gSkyboxVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        // Reset depth function
        glDepthFunc(GL_LESS);
    }

    // Use the shader program
    glUseProgram(gShaderProgram);

    //get locations and set matrices
    GLuint modelLoc = glGetUniformLocation(gShaderProgram, "model");
    GLuint viewLoc = glGetUniformLocation(gShaderProgram, "view");
    GLuint projLoc = glGetUniformLocation(gShaderProgram, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //set color mode
    GLuint useVertexColorsLoc = glGetUniformLocation(gShaderProgram, "useVertexColors");
    glUniform1i(useVertexColorsLoc, gUsePerFaceColors);

    //set material properties
    GLuint matAmbientLoc = glGetUniformLocation(gShaderProgram, "materialAmbient");
    GLuint matDiffuseLoc = glGetUniformLocation(gShaderProgram, "materialDiffuse");
    GLuint matSpecularLoc = glGetUniformLocation(gShaderProgram, "materialSpecular");
    GLuint matShininessLoc = glGetUniformLocation(gShaderProgram, "materialShininess");

    glUniform3fv(matAmbientLoc, 1, glm::value_ptr(gMaterialAmbient));
    glUniform3fv(matDiffuseLoc, 1, glm::value_ptr(gMaterialDiffuse));
    glUniform3fv(matSpecularLoc, 1, glm::value_ptr(gMaterialSpecular));
    glUniform1f(matShininessLoc, gMaterialShininess);

    //set camera position
    GLuint viewPosLoc = glGetUniformLocation(gShaderProgram, "viewPos");
    glUniform3fv(viewPosLoc, 1, glm::value_ptr(gCameraPos));

    //set directional light
    GLuint dirLightEnabledLoc = glGetUniformLocation(gShaderProgram, "dirLightEnabled");
    glUniform1i(dirLightEnabledLoc, gShowDirLight);

    if (gShowDirLight)
    {
        GLuint dirLightDirLoc = glGetUniformLocation(gShaderProgram, "dirLight.direction");
        GLuint dirLightAmbientLoc = glGetUniformLocation(gShaderProgram, "dirLight.ambient");
        GLuint dirLightDiffuseLoc = glGetUniformLocation(gShaderProgram, "dirLight.diffuse");
        GLuint dirLightSpecularLoc = glGetUniformLocation(gShaderProgram, "dirLight.specular");

        glUniform3fv(dirLightDirLoc, 1, glm::value_ptr(gDirLight.direction));
        glUniform3fv(dirLightAmbientLoc, 1, glm::value_ptr(gDirLight.ambient));
        glUniform3fv(dirLightDiffuseLoc, 1, glm::value_ptr(gDirLight.diffuse));
        glUniform3fv(dirLightSpecularLoc, 1, glm::value_ptr(gDirLight.specular));
    }

    //set point light
    GLuint pointLightEnabledLoc = glGetUniformLocation(gShaderProgram, "pointLightEnabled");
    glUniform1i(pointLightEnabledLoc, gShowPointLight);

    if (gShowPointLight)
    {
        GLuint pointLightPosLoc = glGetUniformLocation(gShaderProgram, "pointLight.position");
        GLuint pointLightAmbientLoc = glGetUniformLocation(gShaderProgram, "pointLight.ambient");
        GLuint pointLightDiffuseLoc = glGetUniformLocation(gShaderProgram, "pointLight.diffuse");
        GLuint pointLightSpecularLoc = glGetUniformLocation(gShaderProgram, "pointLight.specular");
        GLuint pointLightConstLoc = glGetUniformLocation(gShaderProgram, "pointLight.constant");
        GLuint pointLightLinearLoc = glGetUniformLocation(gShaderProgram, "pointLight.linear");
        GLuint pointLightQuadLoc = glGetUniformLocation(gShaderProgram, "pointLight.quadratic");

        glUniform3fv(pointLightPosLoc, 1, glm::value_ptr(gPointLight.position));
        glUniform3fv(pointLightAmbientLoc, 1, glm::value_ptr(gPointLight.ambient));
        glUniform3fv(pointLightDiffuseLoc, 1, glm::value_ptr(gPointLight.diffuse));
        glUniform3fv(pointLightSpecularLoc, 1, glm::value_ptr(gPointLight.specular));
        glUniform1f(pointLightConstLoc, gPointLight.constant);
        glUniform1f(pointLightLinearLoc, gPointLight.linear);
        glUniform1f(pointLightQuadLoc, gPointLight.quadratic);
    }

    //set spotlight
    GLuint spotLightEnabledLoc = glGetUniformLocation(gShaderProgram, "spotLightEnabled");
    glUniform1i(spotLightEnabledLoc, gShowSpotlight);

    if (gShowSpotlight)
    {
        GLuint spotLightPosLoc = glGetUniformLocation(gShaderProgram, "spotLight.position");
        GLuint spotLightDirLoc = glGetUniformLocation(gShaderProgram, "spotLight.direction");
        GLuint spotLightAmbientLoc = glGetUniformLocation(gShaderProgram, "spotLight.ambient");
        GLuint spotLightDiffuseLoc = glGetUniformLocation(gShaderProgram, "spotLight.diffuse");
        GLuint spotLightSpecularLoc = glGetUniformLocation(gShaderProgram, "spotLight.specular");
        GLuint spotLightCutOffLoc = glGetUniformLocation(gShaderProgram, "spotLight.cutOff");
        GLuint spotLightOuterCutOffLoc = glGetUniformLocation(gShaderProgram, "spotLight.outerCutOff");
        GLuint spotLightConstLoc = glGetUniformLocation(gShaderProgram, "spotLight.constant");
        GLuint spotLightLinearLoc = glGetUniformLocation(gShaderProgram, "spotLight.linear");
        GLuint spotLightQuadLoc = glGetUniformLocation(gShaderProgram, "spotLight.quadratic");

        glUniform3fv(spotLightPosLoc, 1, glm::value_ptr(gSpotlight.position));
        glUniform3fv(spotLightDirLoc, 1, glm::value_ptr(gSpotlight.direction));
        glUniform3fv(spotLightAmbientLoc, 1, glm::value_ptr(gSpotlight.ambient));
        glUniform3fv(spotLightDiffuseLoc, 1, glm::value_ptr(gSpotlight.diffuse));
        glUniform3fv(spotLightSpecularLoc, 1, glm::value_ptr(gSpotlight.specular));
        glUniform1f(spotLightCutOffLoc, gSpotlight.cutOff);
        glUniform1f(spotLightOuterCutOffLoc, gSpotlight.outerCutOff);
        glUniform1f(spotLightConstLoc, gSpotlight.constant);
        glUniform1f(spotLightLinearLoc, gSpotlight.linear);
        glUniform1f(spotLightQuadLoc, gSpotlight.quadratic);
    }

    //draw cube
    glBindVertexArray(gCubeVAO);
    glDrawElements(GL_TRIANGLES, gNumIndices, GL_UNSIGNED_INT, 0);


    // IMGUI    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    DefineGUI();

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void DefineGUI()
{
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    {
        ImGui::Begin("3DG - ICA");    // Create a window

        ImGui::Text("Visibility Controls");
        ImGui::Checkbox("Show Skybox", &gShowSkybox);

        ImGui::Separator();
        ImGui::Text("Cube Coloring");

        // Color mode toggle
        bool colorModeChanged = false;
        if (ImGui::Checkbox("Use Per-Face Colors", &gUsePerFaceColors)) //red, yellow, green, magenta, blue, cyan
        {
            colorModeChanged = true;
            UpdateCubeColors();
        }

        if (!gUsePerFaceColors)
        {
            if (ImGui::ColorEdit3("Cube Color", &gCubeColor[0])) 
            {
                colorModeChanged = true;
            }

            if (colorModeChanged) 
            {
                UpdateCubeColors();
            }
        }

        ImGui::Separator();
        ImGui::Text("Lighting Controls");

        // Light type toggles
        ImGui::Checkbox("Directional Light (Sun)", &gShowDirLight);
        ImGui::Checkbox("Point Light (Lamp)", &gShowPointLight);
        ImGui::Checkbox("Spotlight (Flashlight)", &gShowSpotlight);

        ImGui::Separator();

        // Directional light controls
        if (ImGui::CollapsingHeader("Directional Light Settings"))
        {
            ImGui::SliderFloat3("Direction", &gDirLight.direction[0], -1.0f, 1.0f);
            ImGui::ColorEdit3("Ambient", &gDirLight.ambient[0]);
            ImGui::ColorEdit3("Diffuse", &gDirLight.diffuse[0]);
            ImGui::ColorEdit3("Specular", &gDirLight.specular[0]);

            if (ImGui::Button("Reset Directional Light"))
            {
                gDirLight.direction = glm::vec3(-0.2f, -1.0f, -0.3f);
                gDirLight.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
                gDirLight.diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
                gDirLight.specular = glm::vec3(1.0f, 1.0f, 1.0f);
            }
        }

        // Point light controls
        if (ImGui::CollapsingHeader("Point Light Settings"))
        {
            ImGui::SliderFloat3("Position", &gPointLight.position[0], -5.0f, 5.0f);
            ImGui::ColorEdit3("Ambient##2", &gPointLight.ambient[0]);
            ImGui::ColorEdit3("Diffuse##2", &gPointLight.diffuse[0]);
            ImGui::ColorEdit3("Specular##2", &gPointLight.specular[0]);
            ImGui::SliderFloat("Constant", &gPointLight.constant, 0.0f, 2.0f);
            ImGui::SliderFloat("Linear", &gPointLight.linear, 0.0f, 0.2f);
            ImGui::SliderFloat("Quadratic", &gPointLight.quadratic, 0.0f, 0.1f);

            if (ImGui::Button("Reset Point Light"))
            {
                gPointLight.position = glm::vec3(1.5f, 0.0f, 0.0f);
                gPointLight.ambient = glm::vec3(0.1f, 0.1f, 0.1f);
                gPointLight.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
                gPointLight.specular = glm::vec3(1.0f, 1.0f, 1.0f);
                gPointLight.constant = 1.0f;
                gPointLight.linear = 0.09f;
                gPointLight.quadratic = 0.032f;
            }
        }

        // Spotlight controls
        if (ImGui::CollapsingHeader("Spotlight Settings"))
        {
            ImGui::SliderFloat3("Direction##2", &gSpotlight.direction[0], -1.0f, 1.0f);
            ImGui::ColorEdit3("Ambient##3", &gSpotlight.ambient[0]);
            ImGui::ColorEdit3("Diffuse##3", &gSpotlight.diffuse[0]);
            ImGui::ColorEdit3("Specular##3", &gSpotlight.specular[0]);
            float cutOffDegrees = glm::degrees(glm::acos(gSpotlight.cutOff));
            float outerCutOffDegrees = glm::degrees(glm::acos(gSpotlight.outerCutOff));
            ImGui::SliderFloat("CutOff (degrees)", &cutOffDegrees, 0.0f, 45.0f);
            ImGui::SliderFloat("Outer CutOff (degrees)", &outerCutOffDegrees, 0.0f, 45.0f);
            gSpotlight.cutOff = glm::cos(glm::radians(cutOffDegrees));
            gSpotlight.outerCutOff = glm::cos(glm::radians(outerCutOffDegrees));
            ImGui::SliderFloat("Constant##2", &gSpotlight.constant, 0.0f, 2.0f);
            ImGui::SliderFloat("Linear##2", &gSpotlight.linear, 0.0f, 0.2f);
            ImGui::SliderFloat("Quadratic##2", &gSpotlight.quadratic, 0.0f, 0.1f);

            if (ImGui::Button("Reset Spotlight"))
            {
                gSpotlight.direction = glm::vec3(0.0f, 0.0f, -1.0f);
                gSpotlight.ambient = glm::vec3(0.0f, 0.0f, 0.0f);
                gSpotlight.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
                gSpotlight.specular = glm::vec3(1.0f, 1.0f, 1.0f);
                gSpotlight.cutOff = glm::cos(glm::radians(12.5f));
                gSpotlight.outerCutOff = glm::cos(glm::radians(17.5f));
                gSpotlight.constant = 1.0f;
                gSpotlight.linear = 0.09f;
                gSpotlight.quadratic = 0.032f;
            }
        }

        ImGui::Separator();
        ImGui::Text("Material Properties");

        ImGui::ColorEdit3("Ambient Color", &gMaterialAmbient[0]);
        ImGui::ColorEdit3("Diffuse Color", &gMaterialDiffuse[0]);
        ImGui::ColorEdit3("Specular Color", &gMaterialSpecular[0]);
        ImGui::SliderFloat("Shininess", &gMaterialShininess, 1.0f, 256.0f);

        if (ImGui::Button("Reset Material"))
        {
            gMaterialAmbient = glm::vec3(0.2f, 0.2f, 0.2f);
            gMaterialDiffuse = glm::vec3(0.8f, 0.8f, 0.8f);
            gMaterialSpecular = glm::vec3(1.0f, 1.0f, 1.0f);
            gMaterialShininess = 32.0f;
        }

        ImGui::Separator();
        ImGui::Text("Cube Rotation Controls");
        ImGui::Separator();

        //auto-rotate toggle
        ImGui::Checkbox("Auto Rotate", &gAutoRotate);

        if (gAutoRotate)
        {
            ImGui::Text("Rotation speed:");
            ImGui::SliderFloat("X Speed", &gRotationSpeedX, 0.0f, 5.0f, "%.2f rad/s");
            ImGui::SliderFloat("Y Speed", &gRotationSpeedY, 0.0f, 5.0f, "%.2f rad/s");

            //reset speed
            if (ImGui::Button("Reset Speeds"))
            {
                gRotationSpeedX = 0.5f;
                gRotationSpeedY = 0.3f;
            }
        }
        else
        {
            ImGui::Text("Auto rotation is OFF");
            ImGui::Text("Use sliders to manually adjust rotation");
        }

        ImGui::Separator();

        //manual controls 
        ImGui::Text("Manual rotation angles:");
        ImGui::SliderFloat("X Rotation", &gRotationX, 0.0f, 6.28318f, "%.3f rad");
        ImGui::SliderFloat("Y Rotation", &gRotationY, 0.0f, 6.28318f, "%.3f rad");

        //rest rotation
        if (ImGui::Button("Reset Rotation"))
        {
            gRotationX = 0.0f;
            gRotationY = 0.0f;
        }

        ImGui::SameLine();

        //Random rotation
        if (ImGui::Button("RandomRotation"))
        {
            gRotationX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 6.28318f;
            gRotationY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 6.28318f;
        }

        ImGui::Separator();

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        ImGui::End();
    }
}