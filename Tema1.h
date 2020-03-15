#pragma once

#include <Component/SimpleScene.h>
#include <Core/Engine.h>
#include <vector>

class Tema1 : public SimpleScene
{
	public:
		struct ViewportSpace
		{
			ViewportSpace() : x(0), y(0), width(1), height(1) {}
			ViewportSpace(int x, int y, int width, int height)
				: x(x), y(y), width(width), height(height) {}
			int x;
			int y;
			int width;
			int height;
		};

		struct LogicSpace
		{
			LogicSpace() : x(0), y(0), width(1), height(1) {}
			LogicSpace(float x, float y, float width, float height)
				: x(x), y(y), width(width), height(height) {}
			float x;
			float y;
			float width;
			float height;
		};

		struct Bird
		{
			float x, y;
			float speedY;
			float scaleX, scaleY;
			float rotate;
		};

		struct Obstacle
		{
			Mesh* mesh;
			float x, width, height;
			bool birdPassed, isTop;
		};

	public:
		Tema1();
		~Tema1();

		void Init() override;

		std::vector<Obstacle> obstacles;

	private:
		Mesh* CreateMesh(const char* name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned short>& indices);
		Mesh* CreateObstacle(const char* name, float height, bool isTop);

		void FrameStart() override;
		void Update(float deltaTimeSeconds) override;
		void FrameEnd() override;

		void DrawObstacle(glm::mat3 visMatrix, float deltaTimeSeconds);
		void DrawBird(glm::mat3 visMatrix, float deltaTimeSeconds);
		
		unsigned long long Tema1::ReadHighScore();
		void Tema1::WriteHighScore();
		void Tema1::LogScore();
		void Tema1::GenerateObstacle(float obstacleX, float obstacleSize, bool isTop);

		bool Tema1::IsCollision(Obstacle currentObstacle);

		void OnInputUpdate(float deltaTime, int mods) override;
		void OnKeyPress(int key, int mods) override;
		void OnKeyRelease(int key, int mods) override;
		void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
		void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
		void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
		void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;

		// Sets the logic space and view space
		// logicSpace: { x, y, width, height }
		// viewSpace: { x, y, width, height }
		glm::mat3 VisualizationTransf2D(const LogicSpace &logicSpace, const ViewportSpace &viewSpace);
		glm::mat3 VisualizationTransf2DUnif(const LogicSpace &logicSpace, const ViewportSpace &viewSpace);

		void SetViewportArea(const ViewportSpace &viewSpace, glm::vec3 colorColor, bool clear = true);

	protected:
		ViewportSpace viewSpace;
		LogicSpace logicSpace;
		glm::mat3 modelMatrix, visMatrix;
		GLenum cullFace;
		GLenum polygonMode;
		Bird bird;
		glm::vec3 backgroundColor, birdBodyColor, birdWingsColor, birdHeadColor, obstacleColor;

		float obstacleSpeed = 0.08f;
		unsigned long long numberOfObstacles = 0;

		unsigned long long highScore;
		unsigned long long score = 0;
};
