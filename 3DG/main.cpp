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


GLFWwindow* CreateGLFWWindow(int width, int height, const std::string& title);
void Render(GLFWwindow* window);
void CreateSquare();
void CreateShaders();
void CreateCubemap();
void DefineGUI();

// Since this is a simple demo we will use some globals
// A real program should use a class to encapsulate state
GLuint gTriangleVAO{ 0 };
GLuint gShaderProgram{ 0 };
GLuint gCubemapShader{ 0 };
GLuint gCubemapTexture{ 0 };
GLuint gCubemapVAO{ 0 };

int main()
{
	GLFWwindow* window{ CreateGLFWWindow(1280, 720, "3DG") };
	if (!window)
		return -1;

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

	CreateSquare();
	CreateCubemap();
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
	glDeleteProgram(gCubemapShader);
	glDeleteVertexArrays(1, &gTriangleVAO);
	glDeleteVertexArrays(1, &gCubemapVAO);
	glDeleteTextures(1, &gCubemapTexture);


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

void CreateSquare()
{
	// TODO: create a triangle

	// Remember the process for OpenGL objects:
	// 1. Create the object id e.g. glGenBuffers(1, &bufferID);
	// 2. Bind the object e.g. glBindBuffer(GL_ARRAY_BUFFER, bufferID);
	// 3. Set any parameters e.g. glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	// 4. Unbind the object e.g. glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You will need a VAO (Vertex Array Object) and a VBO (Vertex Buffer Object) for this triangle

	// Store the VAO in global for use in rendering
	//gTriangleVAO = VAO;

	std::vector<float> verts = {
		//Front
	 -1.0f, -1.0f,  1.0f,
	  1.0f, -1.0f,  1.0f,
	 -1.0f,  1.0f,  1.0f,
	  1.0f,  1.0f,  1.0f,
	  //right
	  1.0f, -1.0f,  1.0f,
	  1.0f, -1.0f, -1.0f,
	  1.0f,  1.0f,  1.0f,
	  1.0f,  1.0f, -1.0f,
	  //left
	  1.0f, -1.0f, -1.0f,
	 -1.0f, -1.0f, -1.0f,
	  1.0f,  1.0f, -1.0f,
	 -1.0f,  1.0f, -1.0f,
	 //back
	 -1.0f, -1.0f, -1.0f,
	 -1.0f, -1.0f,  1.0f,
	 -1.0f,  1.0f, -1.0f,
	 -1.0f,  1.0f,  1.0f,
	 //bottom
	 -1.0f, -1.0f, -1.0f,
	  1.0f, -1.0f, -1.0f,
	 -1.0f, -1.0f,  1.0f,
	  1.0f, -1.0f,  1.0f,
	  //top
	 -1.0f,  1.0f,  1.0f,
	  1.0f,  1.0f,  1.0f,
	 -1.0f,  1.0f, -1.0f,
	  1.0f,  1.0f, -1.0f,

	};

	GLuint VBO;
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	std::vector<float> colours = {
		1,0,0,1,
		1,0,0,1,
		1,0,0,1,
		1,0,0,1,//red

		0,1,0,1,
		0,1,0,1,
		0,1,0,1,
		0,1,0,1,//green

		0,0,1,1,
		0,0,1,1,
		0,0,1,1,
		0,0,1,1,//blue

		1,1,0,1,
		1,1,0,1,
		1,1,0,1,
		1,1,0,1,//yellow

		1,0,1,1,
		1,0,1,1,
		1,0,1,1,
		1,0,1,1,//magenta

		0,1,1,1,
		0,1,1,1,
		0,1,1,1,
		0,1,1,1,//cyan

	};

	GLuint VBOcolour;
	glGenBuffers(1, &VBOcolour);

	glBindBuffer(GL_ARRAY_BUFFER, VBOcolour);

	glBufferData(GL_ARRAY_BUFFER, colours.size() * sizeof(float), colours.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//position attribute
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//colour attribute
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	gTriangleVAO = VAO;

}

void CreateShaders()
{
	//GLuint vertexShader = KeithHelpers::LoadAndCompileShader(GL_VERTEX_SHADER, "Data/Shaders/vertex_shader.vert");
	//GLuint fragmentShader = KeithHelpers::LoadAndCompileShader(GL_FRAGMENT_SHADER, "Data/Shaders/fragment_shader.frag");
	//gShaderProgram = glCreateProgram();

	//// Attach the vertex shader to this program (copies it)
	//glAttachShader(gShaderProgram, vertexShader);

	//// The attibute 0 maps to the input stream "vertex_position" in the vertex shader
	//// Not needed if you use (location=0) in the vertex shader itself
	////glBindAttribLocation(m_program, 0, "vertex_position");

	//// Attach the fragment shader (copies it)
	//glAttachShader(gShaderProgram, fragmentShader);

	//// Done with the originals of these as we have made copies
	//glDeleteShader(vertexShader);
	//glDeleteShader(fragmentShader);

	//KeithHelpers::LinkProgramShaders(gShaderProgram);

	GLuint cubemapVertexShader = KeithHelpers::LoadAndCompileShader(GL_VERTEX_SHADER, "Data/Shaders/cubemap_vertex.vert");
	GLuint cubemapFragmentShader = KeithHelpers::LoadAndCompileShader(GL_FRAGMENT_SHADER, "Data/Shaders/cubemap_fragment.frag");
	gCubemapShader = glCreateProgram();

	glAttachShader(gCubemapShader, cubemapVertexShader);
	glAttachShader(gCubemapShader, cubemapFragmentShader);
	glDeleteShader(cubemapVertexShader);
	glDeleteShader(cubemapFragmentShader);
	KeithHelpers::LinkProgramShaders(gCubemapShader);
}

void CreateCubemap()
{
	float skyboxVertices[] = {
		// positions          
		-10.0f,  10.0f, -10.0f,
		-10.0f, -10.0f, -10.0f,
		 10.0f, -10.0f, -10.0f,
		 10.0f, -10.0f, -10.0f,
		 10.0f,  10.0f, -10.0f,
		-10.0f,  10.0f, -10.0f,

		-10.0f, -10.0f,  10.0f,
		-10.0f, -10.0f, -10.0f,
		-10.0f,  10.0f, -10.0f,
		-10.0f,  10.0f, -10.0f,
		-10.0f,  10.0f,  10.0f,
		-10.0f, -10.0f,  10.0f,

		 10.0f, -10.0f, -10.0f,
		 10.0f, -10.0f,  10.0f,
		 10.0f,  10.0f,  10.0f,
		 10.0f,  10.0f,  10.0f,
		 10.0f,  10.0f, -10.0f,
		 10.0f, -10.0f, -10.0f,

		-10.0f, -10.0f,  10.0f,
		-10.0f,  10.0f,  10.0f,
		 10.0f,  10.0f,  10.0f,
		 10.0f,  10.0f,  10.0f,
		 10.0f, -10.0f,  10.0f,
		-10.0f, -10.0f,  10.0f,

		-10.0f,  10.0f, -10.0f,
		 10.0f,  10.0f, -10.0f,
		 10.0f,  10.0f,  10.0f,
		 10.0f,  10.0f,  10.0f,
		-10.0f,  10.0f,  10.0f,
		-10.0f,  10.0f, -10.0f,

		-10.0f, -10.0f, -10.0f,
		-10.0f, -10.0f,  10.0f,
		 10.0f, -10.0f, -10.0f,
		 10.0f, -10.0f, -10.0f,
		-10.0f, -10.0f,  10.0f,
		 10.0f, -10.0f,  10.0f

	};

	//create VAo and VBO for cubemap

	GLuint cubemapVBO;
	glGenVertexArrays(1, &gCubemapVAO);
	glGenBuffers(1, &cubemapVBO);

	glBindVertexArray(gCubemapVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubemapVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindVertexArray(0);
	glDeleteBuffers(1, &cubemapVBO);

	//generate and bind cubemap
	glGenTextures(1, &gCubemapTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, gCubemapTexture);

	//Define the 6 faces of cubemap
	const int size = 512;
	std::vector<unsigned char> faceData(size * size * 3);

	//define colours for each face
	unsigned char colours[6][3] = {
		{128,128,128},
		{128,128,128},
		{100,100,255},
		{100,255,100},
		{255,255,255},
		{255,255,255},
	};

	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < size * size * 3; j += 3)
		{
			faceData[j] = colours[i][0];
			faceData[j+1] = colours[i][1];
			faceData[j+2] = colours[i][2];
		}
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, faceData.data());
	}

	//set texture parameters
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Render(GLFWwindow* window)
{
	// Clear the screen each time
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	glDepthMask(GL_FALSE);
	glUseProgram(gCubemapShader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, gCubemapTexture);

	glBindVertexArray(gCubemapVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	glDepthMask(GL_TRUE);

	glUseProgram(gShaderProgram);
	glBindVertexArray(gTriangleVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);


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

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		ImGui::End();
	}
}