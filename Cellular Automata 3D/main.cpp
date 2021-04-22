#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <cmath>
#include <vector>

#include "ResourceManager.h"
#include "Camera.h"

// callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// utility functions
void Init();
void processInput(GLFWwindow* window);

// 3D cellular automata
void InitCA();
void ProcessCA();
void DrawCA();
void DrawBorder();
void Swap();
bool BeginCA;

// window settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;

// time variables
double deltaTime = 0.0f;		// time between current frame and last frame
double lastFrame = 0.0f;		// time of last frame

float Timer = 0.0f;
float DrawTimer = 1.0f;

// camera settings
Camera camera(glm::vec3(25.0f, 25.0f, 90.0f));
bool firstMouse = true;
double lastX = (double)SCR_WIDTH / 2.0f;
double lastY = (double)SCR_HEIGHT / 2.0f;

// Cellular Automata
const int GridX = 50;
const int GridY = 50;
const int GridZ = 50;
int currentZ = 1;

int main()
{
	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	// TODO
	// glfwWindowHint(GLFW_RESIZABLE, false);

	// glfw window creation
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Cellular Automata", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}


	// configure global OpenGL state
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);


	// initialize buffers
	Init();

	// initialize 3D Cellular Automata
	InitCA();


	// render loop
	while (!glfwWindowShouldClose(window))
	{
		// time
		double currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		

		// input
		processInput(window);


		// rendering commands here
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
		if (BeginCA)
		{
			Timer += (float)deltaTime;
			ProcessCA();
			currentZ = std::min(currentZ + 1, 50);

			if (Timer >= DrawTimer)
			{
				Swap();
				currentZ = 1;
				Timer = 0.0f;
			}
		}

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
		glm::mat4 view = camera.GetViewMatrix();

		ResourceManager::GetShader("cube").Use();
		ResourceManager::GetShader("cube").SetMatrix4f("projection", projection);
		ResourceManager::GetShader("cube").SetMatrix4f("view", view);

		ResourceManager::GetShader("cube_outline").Use();
		ResourceManager::GetShader("cube_outline").SetMatrix4f("projection", projection);
		ResourceManager::GetShader("cube_outline").SetMatrix4f("view", view);

		DrawBorder();
		DrawCA();


		// check and call events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources
	glfwTerminate();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//														Callback functions
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
		BeginCA = !BeginCA;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	double xoffset = xpos - lastX;
	double yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement((float)xoffset, (float)yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll((float)yoffset);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//														Utility functions
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GLuint CubeVAO;
void Init()
{
	float CubeVertices[] = {
		// back face
		-0.5f, -0.5f, -0.5f,		// bottom-left
		0.5f, 0.5f, -0.5f,			// top-right
		0.5f, -0.5f, -0.5f,			// bottom-right
		0.5f, 0.5f, -0.5f,			// top-right
		-0.5f, -0.5f, -0.5f,		// bottom-left
		-0.5f, 0.5f, -0.5f,			// top-left

		// front face
		-0.5f, -0.5f, 0.5f,			// bottom-left
		0.5f, -0.5f, 0.5f,			// bottom-right
		0.5f, 0.5f, 0.5f,			// top-right
		0.5f, 0.5f, 0.5f,			// top-right
		-0.5f, 0.5f, 0.5f,			// top-left
		- 0.5f, -0.5f, 0.5f,		// bottom-left

		// left face
		-0.5f, 0.5f, 0.5f,			// top-right
		-0.5f, 0.5f, -0.5f,			// top-left
		-0.5f, -0.5f, -0.5f,		// bottom-left
		-0.5f, -0.5f, -0.5f,		// bottom-left
		-0.5f, -0.5f, 0.5f,			// bottom-right
		-0.5f, 0.5f, 0.5f,			// top-right

		// right face
		0.5f, 0.5f, 0.5f,			// top-left
		0.5f, -0.5f, -0.5f,			// bottom-right
		0.5f, 0.5f, -0.5f,			// top-right
		0.5f, -0.5f, -0.5f,			// bottom-right
		0.5f, 0.5f, 0.5f,			// top-left
		0.5f, -0.5f, 0.5f,			// bottom-left

		// bottom face
		-0.5f, -0.5f, -0.5f,		// top-right
		0.5f, -0.5f, -0.5f,			// top-left
		0.5f, -0.5f, 0.5f,			// bottom-left
		0.5f, -0.5f, 0.5f,			// bottom-left
		-0.5f, -0.5f, 0.5f,			// bottom-right
		-0.5f, -0.5f, -0.5f,		// top-right

		// top face
		-0.5f, 0.5f, -0.5f,			// top-left
		0.5f, 0.5f, 0.5f,			// bottom-right
		0.5f, 0.5f, -0.5f,			// top-right
		0.5f, 0.5f, 0.5f,			// bottom-right
		-0.5f, 0.5f, -0.5f,			// top-left
		-0.5f, 0.5f, 0.5f			// bottom-left
	};

	GLuint CubeVBO;
	glGenVertexArrays(1, &CubeVAO);
	glGenBuffers(1, &CubeVBO);

	glBindVertexArray(CubeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, CubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CubeVertices), CubeVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// load shaders
	ResourceManager::LoadShader("shaders/cube_outline.vert", "shaders/cube_outline.frag", "shaders/cube_outline.geom", "cube_outline");

	ResourceManager::LoadShader("shaders/cube.vert", "shaders/cube.frag", nullptr, "cube");

	// configure shaders
	ResourceManager::GetShader("cube_outline").Use();
	ResourceManager::GetShader("cube_outline").SetVector4f("color", glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, (float)deltaTime);

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.MovementSpeed = 10.0f;
	else
		camera.MovementSpeed = 5.0f;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//														3D Cellular Automata
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*

	The grid is a cube 50/50/50

	At each step of the CA, empty cells (cells that have a state 0) are updated as follows:
	1. Count the cell’s neighboring faces, edges and corners (separates the 26 neighbors into 3 groups: faces, edges and corners)
	2. If the empty cell does not share at least 1 face with a neighbor stop processing the cell and go onto the next one
	3. Use the rule array to update the new cell state (like any CA you use a temp array for the new cell states so all cells are update simultaneously)
	4. Stop when the CA structure reaches the edge of the 3D grid

*/

std::vector<std::vector<std::vector<int>>> mat1, mat2;

int Rule[5][7][13][9];

void InitCA()
{
	mat1.resize(GridX + 2, std::vector<std::vector<int>>(GridY + 2, std::vector<int>(GridZ + 2, 0)));
	mat2.resize(GridX + 2, std::vector<std::vector<int>>(GridY + 2, std::vector<int>(GridZ + 2, 0)));

	// todo : make initial state
	for (int z = 24; z <= 26; z++)
	{
		for (int y = 24; y <= 26; y++)
		{
			for (int x = 24; x <= 26; x++)
			{
				mat1[x][y][z] = 4;
			}
		}
	}

	// fill Rule
	// todo : defined rules

	// Rule 4/4/5/M
	for (int face = 0; face <= 6; face++)
	{
		for (int edge = 0; edge <= 12; edge++)
		{
			for (int corner = 0; corner <= 8; corner++)
			{
				if (face + edge + corner == 4)
				{
					Rule[0][face][edge][corner] = 4;
				}
			}
		}
	}

	// todo : random seed
	if (false)
	{
		for (int state = 0; state < 5; state++)
		{
			for (int face = 0; face <= 6; face++)
			{
				for (int edge = 0; edge <= 12; edge++)
				{
					for (int corner = 0; corner <= 8; corner++)
					{
						// Rule[state][face][edge][corner] = std::rand() % 5;
					}
				}
			}
		}
	}
}

void ProcessCA()
{
	for (int z = 1; z <= GridZ; z++)
	{
		for (int y = 1; y <= GridY; y++)
		{
			for (int x = 1; x <= GridX; x++)
			{	
				int CellState = mat1[x][y][z];
				mat2[x][y][z] = CellState;

				if (CellState > 0)
				{
					mat2[x][y][z]--;
				}
				else if (CellState == 0)
				{
					// faces
					int FaceCount = 0;
					if (mat1[x][y][z - 1] > 0) FaceCount++;
					if (mat1[x][y][z + 1] > 0) FaceCount++;
					if (mat1[x][y - 1][z] > 0) FaceCount++;
					if (mat1[x][y + 1][z] > 0) FaceCount++;
					if (mat1[x - 1][y][z] > 0) FaceCount++;
					if (mat1[x + 1][y][z] > 0) FaceCount++;

					if (FaceCount > 0)
					{
						int EdgeCount = 0;
						int CornerCount = 0;

						//edges
						if (mat1[x][y - 1][z - 1] > 0) EdgeCount++;
						if (mat1[x - 1][y][z - 1] > 0) EdgeCount++;
						if (mat1[x + 1][y][z - 1] > 0) EdgeCount++;
						if (mat1[x][y + 1][z - 1] > 0) EdgeCount++;
						if (mat1[x - 1][y - 1][z] > 0) EdgeCount++;
						if (mat1[x + 1][y - 1][z] > 0) EdgeCount++;
						if (mat1[x - 1][y + 1][z] > 0) EdgeCount++;
						if (mat1[x + 1][y + 1][z] > 0) EdgeCount++;
						if (mat1[x][y - 1][z + 1] > 0) EdgeCount++;
						if (mat1[x - 1][y][z + 1] > 0) EdgeCount++;
						if (mat1[x + 1][y][z + 1] > 0) EdgeCount++;
						if (mat1[x][y + 1][z + 1] > 0) EdgeCount++;

						//corners
						if (mat1[x - 1][y - 1][z - 1] > 0) CornerCount++;
						if (mat1[x + 1][y - 1][z - 1] > 0) CornerCount++;
						if (mat1[x - 1][y + 1][z - 1] > 0) CornerCount++;
						if (mat1[x + 1][y + 1][z - 1] > 0) CornerCount++;
						if (mat1[x - 1][y - 1][z + 1] > 0) CornerCount++;
						if (mat1[x + 1][y - 1][z + 1] > 0) CornerCount++;
						if (mat1[x - 1][y + 1][z + 1] > 0) CornerCount++;
						if (mat1[x + 1][y + 1][z + 1] > 0) CornerCount++;

						// update Temp Grid
						mat2[x][y][z] = Rule[CellState][FaceCount][EdgeCount][CornerCount];
					}
				}
			}
		}
	}
}

void Swap()
{
	for (int z = 1; z <= GridZ; z++)
		for (int y = 1; y <= GridY; y++)
			for (int x = 1; x <= GridX; x++)
				mat1[x][y][z] = mat2[x][y][z];
}

void DrawCA()
{
	// todo : Instancing

	for (int z = 1; z <= GridZ; z++)
	{
		for (int y = 1; y <= GridY; y++)
		{
			for (int x = 1; x <= GridX; x++)
			{
				int CellState = mat1[x][y][z];

				if (CellState > 0)
				{
					glm::vec4 color = glm::vec4(1.0f);
					switch (CellState)
					{
						case 1: color = glm::vec4(0.589f, 0.082f, 0.0f, 1.0f); break;
						case 2: color = glm::vec4(1.0f, 0.501f, 0.0f, 1.0f); break;
						case 3: color = glm::vec4(1.0f, 0.647f, 0.0f, 1.0f); break;
						case 4: color = glm::vec4(1.0f, 0.749f, 0.0f, 1.0f); break;
					}

					glm::mat4 model = glm::mat4(1.0f);
					model = glm::translate(model, glm::vec3((float)x, (float)y, (float)z));
					model = glm::scale(model, glm::vec3(0.997f, 0.997f, 0.997f));

					ResourceManager::GetShader("cube").Use();
					ResourceManager::GetShader("cube").SetMatrix4f("model", model);
					ResourceManager::GetShader("cube").SetVector4f("color", color);
					glDrawArrays(GL_TRIANGLES, 0, 36);

					model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
					ResourceManager::GetShader("cube_outline").Use();
					ResourceManager::GetShader("cube_outline").SetMatrix4f("model", model);
					glDrawArrays(GL_TRIANGLES, 0, 36);
				}
			}
		}
	}
}

void DrawBorder()
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(GridX / 2.0f, GridY / 2.0f, GridZ / 2.0f));
	model = glm::scale(model, glm::vec3((float)GridX, (float)GridY, (float)GridZ));

	ResourceManager::GetShader("cube_outline").Use();
	ResourceManager::GetShader("cube_outline").SetMatrix4f("model", model);
	ResourceManager::GetShader("cube_outline").SetVector4f("color", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	glDrawArrays(GL_TRIANGLES, 0, 36);
	ResourceManager::GetShader("cube_outline").SetVector4f("color", glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
}

