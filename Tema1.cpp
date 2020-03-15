#include "Tema1.h"

#include <vector>
#include <iostream>
#include <fstream>

#include <Core/Engine.h>
#include "Transform2D.h"

#include <time.h>
#include <string> 

using namespace std;

Tema1::Tema1()
{
}

Tema1::~Tema1()
{
}

void Tema1::Init()
{
	/* initialize random seed: */
	srand(time(NULL));

	auto camera = GetSceneCamera();
	camera->SetPosition(glm::vec3(0, 0, 50));
	camera->SetRotation(glm::vec3(0, 0, 0));
	camera->Update();
	GetCameraInput()->SetActive(false);

	logicSpace.x = 0;		// logic x
	logicSpace.y = 0;		// logic y
	logicSpace.width = 16;	// logic width
	logicSpace.height = 9;	// logic height

	glm::vec3 corner = glm::vec3(0.001, 0.001, 0);

	backgroundColor = glm::vec3(0.3, 0.75, 0.79);
	birdHeadColor = glm::vec3(0.9, 0.9, 0.9);
	birdWingsColor = glm::vec3(0.9, 0.9, 0.9);
	birdBodyColor = glm::vec3(0.95, 0.71, 0.2);
	obstacleColor = glm::vec3(0.79, 0.9, 0.49);
	
	bird.x = 5;
	bird.y = 7;
	bird.scaleX = 0.3;
	bird.scaleY = 0.3;

	bird.rotate = 0;
	bird.speedY = 0;

	highScore = ReadHighScore();
	//------------------------------------------------------------------------------------------

	cullFace = GL_BACK;
	polygonMode = GL_FILL;

	// Create a mesh box using custom data
	{
		vector<VertexFormat> verticesBird
		{
			VertexFormat
			(
				glm::vec3(0.5f, 0.5f, 0),
				birdBodyColor
			),
			VertexFormat
			(
				glm::vec3(1.5f, 0.5f, 0),
				birdBodyColor
			),
			VertexFormat
			(
				glm::vec3(0.5f, 1, 0),
				birdBodyColor
			),
			VertexFormat
			(
				glm::vec3(1.5f, 1, 0),
				birdBodyColor
			),
			VertexFormat
			(
				glm::vec3(0.5f, 2, 0),
				birdBodyColor
			),
			VertexFormat
			(
				glm::vec3(1.5f, 2, 0),
				birdBodyColor
			),
			VertexFormat
			(
				glm::vec3(0.5f, 2.5f, 0),
				birdBodyColor
			),
			VertexFormat
			(
				glm::vec3(1.5f, 2.5f, 0),
				birdBodyColor
			),
			VertexFormat
			(
				glm::vec3(2.5f, 2, 0),
				birdHeadColor
			),
			VertexFormat
			(
				glm::vec3(2.5f, 1, 0),
				birdHeadColor
			),
			VertexFormat
			(
				glm::vec3(3, 1.5f, 0),
				birdBodyColor
			),
			VertexFormat
			(
				glm::vec3(1, 3, 0),
				birdWingsColor
			),
			VertexFormat
			(
				glm::vec3(1, 0, 0),
				birdWingsColor
			),
			VertexFormat
			(
				glm::vec3(0, 2.5f, 0),
				birdWingsColor
			),
			VertexFormat
			(
				glm::vec3(0, 0.5f, 0),
				birdWingsColor
			),
		};

		vector<unsigned short> indicesBird =
		{
			0, 1, 2,
			1, 3, 2,
			3, 4, 2,
			3, 5, 4,
			5, 6, 4,
			5, 7, 6,
			9, 5, 3,
			9, 8, 5,
			9, 10, 8,
			7, 11, 6,
			12, 1, 0,
			6, 13, 4,
			2, 14, 0
		};
	
		Mesh* bird = CreateMesh("bird", verticesBird, indicesBird);

		for (int i = 17; i <= 32; i+= 5) 
		{
			GenerateObstacle(i, rand() % 3 + 1, false);
			GenerateObstacle(i, rand() % 3 + 1, true);
		}
	}
}

// 2D visualization matrix
glm::mat3 Tema1::VisualizationTransf2D(const LogicSpace & logicSpace, const ViewportSpace & viewSpace)
{
	float sx, sy, tx, ty;
	sx = viewSpace.width / logicSpace.width;
	sy = viewSpace.height / logicSpace.height;
	tx = viewSpace.x - sx * logicSpace.x;
	ty = viewSpace.y - sy * logicSpace.y;

	return glm::transpose(glm::mat3(
		sx, 0.0f, tx,
		0.0f, sy, ty,
		0.0f, 0.0f, 1.0f));
}

// uniform 2D visualization matrix (same scale factor on x and y axes)
glm::mat3 Tema1::VisualizationTransf2DUnif(const LogicSpace & logicSpace, const ViewportSpace & viewSpace)
{
	float sx, sy, tx, ty, smin;
	sx = viewSpace.width / logicSpace.width;
	sy = viewSpace.height / logicSpace.height;
	if (sx < sy)
		smin = sx;
	else
		smin = sy;
	tx = viewSpace.x - smin * logicSpace.x + (viewSpace.width - smin * logicSpace.width) / 2;
	ty = viewSpace.y - smin * logicSpace.y + (viewSpace.height - smin * logicSpace.height) / 2;

	return glm::transpose(glm::mat3(
		smin, 0.0f, tx,
		0.0f, smin, ty,
		0.0f, 0.0f, 1.0f));
}

void Tema1::SetViewportArea(const ViewportSpace & viewSpace, glm::vec3 colorColor, bool clear)
{
	glViewport(viewSpace.x, viewSpace.y, viewSpace.width, viewSpace.height);

	glEnable(GL_SCISSOR_TEST);
	glScissor(viewSpace.x, viewSpace.y, viewSpace.width, viewSpace.height);

	// Clears the color buffer (using the previously set color) and depth buffer
	glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);

	GetSceneCamera()->SetOrthographic((float)viewSpace.x, (float)(viewSpace.x + viewSpace.width), (float)viewSpace.y, (float)(viewSpace.y + viewSpace.height), 0.1f, 400);
	GetSceneCamera()->Update();
}

Mesh* Tema1::CreateObstacle(const char* name, float height, bool isTop)
{
	vector<VertexFormat> verticesObstacle
	{
		VertexFormat
		(
			isTop ? glm::vec3(0, 9 - height, 0) : glm::vec3(0, 0, 0),
			obstacleColor
		),
		VertexFormat
		(
			isTop ? glm::vec3(2, 9 - height,  0) : glm::vec3(2, 0, 0),
			obstacleColor
		),
		VertexFormat
		(
			isTop ? glm::vec3(0, 9,  0) : glm::vec3(0, height, 0),
			obstacleColor
		),
		VertexFormat
		(
			isTop ? glm::vec3(2, 9,  0) : glm::vec3(2, height, 0),
			obstacleColor
		)
	};

	vector<unsigned short> indicesObstacle =
	{
		0, 1, 2,
		1, 3, 2
	};

	Mesh* obstacle = CreateMesh(name, verticesObstacle, indicesObstacle);
	return obstacle;
}

Mesh* Tema1::CreateMesh(const char* name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned short>& indices)
{
	unsigned int VAO = 0;
	// Create the VAO and bind it
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Create the VBO and bind it
	unsigned int VBO = 0;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// Send vertices data into the VBO buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	// Crete the IBO and bind it
	unsigned int IBO = 0;
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	// Send indices data into the IBO buffer
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);

	// ========================================================================
	// This section describes how the GPU Shader Vertex Shader program receives data
	// It will be learned later, when GLSL shaders will be introduced
	// For the moment just think that each property value from our vertex format needs to be send to a certain channel
	// in order to know how to receive it in the GLSL vertex shader

	// set vertex position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), 0);

	// set vertex normal attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(sizeof(glm::vec3)));

	// set texture coordinate attribute
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3)));

	// set vertex color attribute
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3) + sizeof(glm::vec2)));
	// ========================================================================

	// Unbind the VAO
	glBindVertexArray(0);

	// Check for OpenGL errors
	CheckOpenGLError();

	// Mesh information is saved into a Mesh object
	meshes[name] = new Mesh(name);
	meshes[name]->InitFromBuffer(VAO, static_cast<unsigned short>(indices.size()));
	return meshes[name];
}

void Tema1::FrameStart()
{
	// Clears the color buffer (using the previously set color) and depth buffer
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Tema1::GenerateObstacle(float obstacleX, float obstacleSize, bool isTop) 
{
	string currentObstacleID = "obstacle" + to_string(++numberOfObstacles);
	Mesh* currentMesh = CreateObstacle(currentObstacleID.c_str(), obstacleSize, isTop);
	obstacles.push_back(
	{ 
		currentMesh, 
		obstacleX, 
		2, 
		obstacleSize, 
		false,
		isTop
	});
}

unsigned long long Tema1::ReadHighScore() 
{
	ifstream highScoreFile(RESOURCE_PATH::ROOT + "/FlappyHighScore.txt");
	string highScoreAsString;

	if (highScoreFile.is_open())
	{
		getline(highScoreFile, highScoreAsString);
		highScoreFile.close();
		return atoi(highScoreAsString.c_str());
	}
	else 
	{
		cout << "Unable to open" + RESOURCE_PATH::ROOT + "/FlappyHighScore.txt";
		exit(-1);
	}
}

void Tema1::WriteHighScore()
{
	ofstream highScoreFile(RESOURCE_PATH::ROOT + "/FlappyHighScore.txt");
	if (highScoreFile.is_open())
	{
		highScoreFile << highScore;
		highScoreFile.close();
	}
	else
	{
		cout << "Unable to open" + RESOURCE_PATH::ROOT + "/FlappyHighScore.txt";
		exit(-1);
	}
}

bool Tema1::IsCollision(Obstacle currentObstacle) 
{
	float currentObstacleY = currentObstacle.isTop ? 9 - currentObstacle.height : 0;
	float circleRadius = 3 * bird.scaleX / 2;
	float auxX = bird.x;
	float auxY = bird.y;

	// Closest edges
	if (bird.x < currentObstacle.x) auxX = currentObstacle.x; // Left
	else if (bird.x > currentObstacle.x + currentObstacle.width) auxX = currentObstacle.x + currentObstacle.width; // Right

	if (bird.y < currentObstacleY) auxY = currentObstacleY; // Top
	else if (bird.y > currentObstacleY + currentObstacle.height) auxY = currentObstacleY + currentObstacle.height; // Bottom

	// Distance from the closest edges
	float distance = sqrt(pow(bird.x - auxX, 2) + pow(bird.y - auxY, 2));

	// Collision condition
	if (distance <= circleRadius) return true;
	else return false;
}

void Tema1::Update(float deltaTimeSeconds)
{
	glm::ivec2 resolution = window->GetResolution();

	// Sets the screen area where to draw - the full window
	viewSpace = ViewportSpace(0, 0, resolution.x, resolution.y);
	SetViewportArea(viewSpace, glm::vec3(0), true);
	
	// Compute the 2D visualization matrix
	visMatrix = glm::mat3(1);
	visMatrix *= VisualizationTransf2DUnif(logicSpace, viewSpace);

	// Draws the god damn bird...
	DrawBird(visMatrix, deltaTimeSeconds);

	// Draws the scene for the current frame
	DrawObstacle(visMatrix, deltaTimeSeconds);
}

void Tema1::DrawBird(glm::mat3 visMatrix, float deltaTimeSeconds)
{
	// Set polygon mode
	glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

	// Enable face culling
	glEnable(GL_CULL_FACE);

	// Set face custom culling.
	glCullFace(cullFace);

	if (bird.y < 8.5f   || bird.speedY < 0) bird.y += bird.speedY;
	if (bird.y < -1.5f) LogScore();
	bird.speedY -= 0.0125f;

	if (bird.speedY > 0 && bird.rotate < 30) bird.rotate += 2;
	else if (bird.speedY < 0 && bird.rotate > -30) bird.rotate -= 2;
	
	// Render bird
	modelMatrix = visMatrix;
	modelMatrix *= Transform2D::Translate(bird.x, bird.y);
	modelMatrix *= Transform2D::Scale(bird.scaleX, bird.scaleY);

	modelMatrix *= Transform2D::Rotate(bird.rotate);
	modelMatrix *= Transform2D::Translate(-1.5f, -1.5f);

	RenderMesh2D(meshes["bird"], shaders["VertexColor"], modelMatrix);

	// Disable face culling
	glDisable(GL_CULL_FACE);
}

void Tema1::DrawObstacle(glm::mat3 visMatrix, float deltaTimeSeconds)
{
	vector<struct Uncolored>::iterator it;

	for (int i = 0; i < obstacles.size(); i++) 
	{
		if (IsCollision(obstacles[i])) LogScore();

		if (obstacles[i].x + 2 < bird.x && !obstacles[i].birdPassed)
		{
			obstacles[i].birdPassed = true;
			score++;
		}

		obstacles[i].x -= obstacleSpeed;
		modelMatrix = visMatrix * Transform2D::Translate(obstacles[i].x, 0);
		RenderMesh2D(meshes[obstacles[i].mesh->GetMeshID()], shaders["VertexColor"], modelMatrix);
	}

	int numberOfObstaclesBeforeErase = obstacles.size();

	obstacles.erase(std::remove_if(obstacles.begin(), obstacles.end(), [](Obstacle currentObstacle) 
	{
		return currentObstacle.x < -2;
	}), obstacles.end());

	if (obstacles.size() < numberOfObstaclesBeforeErase)
	{
		float farthestObstacleX = 0;

		for (int i = 0; i < obstacles.size(); i++)
		{
			if (obstacles[i].x > farthestObstacleX) farthestObstacleX = obstacles[i].x;
		}

		GenerateObstacle(farthestObstacleX + 5, rand() % 3 + 1, false);
		GenerateObstacle(farthestObstacleX + 5, rand() % 3 + 1, true);

		if (score % 10 == 0) obstacleSpeed += 0.01f;
	}
}

void Tema1::LogScore()
{
	if (highScore < score / 2) 
	{
		highScore = score / 2;
		WriteHighScore();
	}

	cout << "You lost :(!" << '\n';
	cout << "HighScore: " << highScore << '\n';
	cout << "Score: " << score / 2 << '\n';;
	exit(0);
}

void Tema1::OnInputUpdate(float deltaTime, int mods)
{
	
}

void Tema1::OnKeyPress(int key, int mods)
{
	if (key == GLFW_KEY_SPACE)
	{
		bird.speedY = 0.2f;
	}

	if (key == GLFW_KEY_I) 
	{
		if (polygonMode == GL_LINE) polygonMode = GL_FILL;
		else polygonMode = GL_LINE;
	}
}

void Tema1::OnKeyRelease(int key, int mods)
{
	// add key release event
}

void Tema1::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	// add mouse move event
}

void Tema1::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button press event
}

void Tema1::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button release event
}

void Tema1::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}

void Tema1::FrameEnd()
{

}