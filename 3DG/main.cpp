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
void CreateShaders();
void DefineGUI();

// Since this is a simple demo we will use some globals
// A real program should use a class to encapsulate state
GLuint gCubeVAO{ 0 };
GLuint gShaderProgram{ 0 };
GLuint gNumIndices = 36; //each face is 2 triangles, 6 vertices per face

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

	CreateCube();
	CreateShaders();

	// Enter main loop until the user closes the window
	while (!glfwWindowShouldClose(window))
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GLFW_TRUE);

		Render(window);

		// GLFW updating - internally swaps the front and back buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// You should delete any OpenGL resources here
	glDeleteProgram(gShaderProgram);
	glDeleteVertexArrays(1, &gCubeVAO);

	// Clean up and exit
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
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
		// Front 
		-0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,

		 // Back
		  0.5f, -0.5f, -0.5f,
		 -0.5f, -0.5f, -0.5f,
		  0.5f,  0.5f, -0.5f,
		 -0.5f,  0.5f, -0.5f,

		 // Top
		 -0.5f,  0.5f,  0.5f,
		  0.5f,  0.5f,  0.5f,
		 -0.5f,  0.5f, -0.5f,
		  0.5f,  0.5f, -0.5f,

		  // Bottom
		  -0.5f, -0.5f, -0.5f,
		   0.5f, -0.5f, -0.5f,
		  -0.5f, -0.5f,  0.5f,
		   0.5f, -0.5f,  0.5f,

		   // Right
			0.5f, -0.5f,  0.5f,
			0.5f, -0.5f, -0.5f,
			0.5f,  0.5f,  0.5f,
			0.5f,  0.5f, -0.5f,

			// Left
			-0.5f, -0.5f, -0.5f,
			-0.5f, 0.5f,  0.5f,
			-0.5f, -0.5f, -0.5f,
			-0.5f,  0.5f,  0.5f
	};


	std::vector<unsigned int> indices = {
		// Front 
		0, 1, 2,  2, 1, 3,

		// Back 
		4, 5, 6,  6, 5, 7,

		// Top 
		8, 9, 10,  10, 9, 11,

		// Bottom
		12, 13, 14,  14, 13, 15,

		// Right
		16, 17, 18,  18, 17, 19,

		// Left
		20, 21, 22,  22, 21, 23
	};

	std::vector<float> colours = {
		// Front - red
		1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 1.0f,

		// Back - green
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,

		// Top - blue
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,

		// Bottom - yellow
		1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f,

		// Right - magenta
		1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 1.0f,

		// Left - cyan
		0.0f, 1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f, 1.0f,
	};

	GLuint VBO, EBO, VBOcolour;

	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);


	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0);
	glEnableVertexAttribArray(0);


	glGenBuffers(1, &VBOcolour);
	glBindBuffer(GL_ARRAY_BUFFER, VBOcolour);
	glBufferData(GL_ARRAY_BUFFER, colours.size() * sizeof(float), colours.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)0);
	glEnableVertexAttribArray(1);


	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);

	gCubeVAO = VAO;
	gNumIndices = indices.size();
}

void CreateShaders()
{
	GLuint vertexShader = KeithHelpers::LoadAndCompileShader(GL_VERTEX_SHADER, "Data/Shaders/vertex_shader.vert");
	GLuint fragmentShader = KeithHelpers::LoadAndCompileShader(GL_FRAGMENT_SHADER, "Data/Shaders/fragment_shader.frag");
	gShaderProgram = glCreateProgram();

	// Attach the vertex shader to this program (copies it)
	glAttachShader(gShaderProgram, vertexShader);

	// The attibute 0 maps to the input stream "vertex_position" in the vertex shader
	// Not needed if you use (location=0) in the vertex shader itself
	//glBindAttribLocation(m_program, 0, "vertex_position");

	// Attach the fragment shader (copies it)
	glAttachShader(gShaderProgram, fragmentShader);

	// Done with the originals of these as we have made copies
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	KeithHelpers::LinkProgramShaders(gShaderProgram);
}

void Render(GLFWwindow* window)
{
	// Clear the screen each time
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Use the shader program
	glUseProgram(gShaderProgram);

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
		glm::vec3(0.0f, 0.0f, 3.0f), //Camera pos
		glm::vec3(0.0f, 0.0f, 0.0f), //look at pos
		glm::vec3(0.0f, 1.0f, 0.0f)  //up vector
	);

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float aspect = static_cast<float>(width) / static_cast<float>(height);
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

	//get locations and set matrices
	GLuint modelLoc = glGetUniformLocation(gShaderProgram, "model");
	GLuint viewLoc = glGetUniformLocation(gShaderProgram, "view");
	GLuint projLoc = glGetUniformLocation(gShaderProgram, "projection");

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// Draw Triangle
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
	// Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	{
		ImGui::Begin("3DG");                    // Create a window called "3GP" and append into it.

		ImGui::Text("Visibility.");             // Display some text (you can use a format strings too)	

		//	ImGui::Checkbox("Wireframe", &m_wireframe);	// A checkbox linked to a member variable

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