#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <glm\glm.hpp>


//Functions:
void PhysicsInit();
void verletSolver(float dt);
void updateForces();

//Render prims:
namespace Sphere
{
	extern void updateSphere(glm::vec3 pos, float radius = 1.f);
}
namespace ClothMesh
{
	extern void setupClothMesh();
	extern void cleanupClothMesh();
	extern void updateClothMesh(float* array_data);
	extern void drawClothMesh();
}

#pragma region GUI Variables
static bool playSimulation = true;
int clicked = 0;
float totalResetTime = 3.0f;
glm::vec3 gravityAccel = { 0.0f,-9.81,0.0f };
glm::vec2 k_stretch = { 1000.0f,50.0f };
glm::vec2 k_shear = { 1000.0f,50.0f };
glm::vec2 k_bend = { 1000.0f,50.0f };
float particleLinkDistance = 0.5f;
bool useCollisions = true;
extern bool renderSphere = true;
extern bool renderCloth = true;
float elasticCoefficient = 0.5f;
float frictionCoefficient = 0.1f;
#pragma endregion

const float k_damping = 0.5f;

//Other variables:
float resetTime;
//Sphere:
glm::vec3 spherePosition;
float sphereRadius;

//Cloth:
//Position:
glm::vec3 posCloth[18][14];
glm::vec3 lastPosCloth[18][14];
glm::vec3 auxPos;
//Velocity:
glm::vec3 velCloth[18][14];
//Forces:
glm::vec3 sumFCloth[18][14];

const float mass = 1.0f;


bool show_test_window = false;
void GUI()
{
	bool show = true;
	ImGui::Begin("Physics Parameters", &show, 0);

	{
		//ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);//FrameRate
		ImGui::Checkbox("Play simulation", &playSimulation);
		if (ImGui::Button("Reset Simulation"))
		{
			clicked++;
		}
		if (clicked & 1)
		{
			PhysicsInit();
			clicked--;
		}
		ImGui::DragFloat("Reset Time", &totalResetTime, 0.05f);
		ImGui::InputFloat3("Gravity Accel", (float*)&gravityAccel);

		if (ImGui::TreeNode("Spring parameters"))
		{
			ImGui::InputFloat2("k_stretch", (float*)&k_stretch);
			ImGui::InputFloat2("k_shear", (float*)&k_shear);
			ImGui::InputFloat2("k_bend", (float*)&k_bend);
			ImGui::DragFloat("Particle Link", &particleLinkDistance, 0.05f);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Collisions"))
		{
			ImGui::Checkbox("Use Collisions", &useCollisions);
			ImGui::Checkbox("Use Sphere Collider", &renderSphere);
			ImGui::DragFloat("Elastic Coefficient", &elasticCoefficient, 0.005f);
			ImGui::DragFloat("Elastic Coefficient", &frictionCoefficient, 0.005f);

			ImGui::TreePop();
		}
	}
	// .........................

	ImGui::End();

	if (show_test_window)
	{
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}

void PhysicsInit() {

	resetTime = 0.0f;

	//Initialize Sphere at random position
	if (renderSphere)
	{
		spherePosition = { -5.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (10.0f))), static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (10.0f))), -5+ static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (10.0f))) };
		sphereRadius = 0.5f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (2.0f)));
		Sphere::updateSphere(spherePosition, sphereRadius);
	}

	//Initialize Cloth:
	auxPos.z = -4.5f;
	for (int i=0; i<18 ;i++)
	{
		auxPos.z += particleLinkDistance;
		auxPos.x = -4.0f;
		for (int j=0; j<14 ;j++)
		{
			//Position:
			auxPos.x += particleLinkDistance;
			posCloth[i][j] = { auxPos.x , 9.5f , auxPos.z };
			lastPosCloth[i][j] = { auxPos.x , 9.5f , auxPos.z };
			//Velocity:
			velCloth[i][j] = { 0.f,0.f,0.f };
			//Forces:
			sumFCloth[i][j] = { 0.f,0.f,0.f };
		}
	}
}

void PhysicsUpdate(float dt) {
	if (playSimulation)
	{
		if (resetTime >= totalResetTime)
		{
			clicked++;
		}
		else
		{
			resetTime += dt;
			if (useCollisions)
			{
				//Forces:
				updateForces();
				//Position and Velocity:
				verletSolver(dt);
			}
			ClothMesh::updateClothMesh((float*)posCloth);
		}
	}
}

void PhysicsCleanup() {

}

void updateForces()
{
	for (int i = 0; i<18; i++)
	{
		for (int j = 0; j<14; j++)
		{
			if (i == 0 && (j == 0 || j == 13))
			{
				//do nothing
			}
			else
			{
				//top
				if (i == 0)
				{
					//sumFCloth[i][j] = -(k_stretch*(glm::distance())
				}
				//bottom
				else if(i==17)
				{
					//left:
					if (j == 0)
					{

					}
					else if (j == 13)
					{

					}
					else
					{

					}
				}
				//middle
				else
				{
					//left:
					if (j == 0)
					{

					}
					//right
					else if (j == 13)
					{

					}
					//middle
					else
					{

					}
				}

			}
		}
	}
}


void verletSolver(float dt)
{
	for (int i = 0; i<18; i++)
	{
		for (int j = 0; j<14; j++)
		{
			if (i == 0 && (j == 0 || j == 13))
			{
				//do nothing
			}
			else
			{
				//Position:
				auxPos = posCloth[i][j];
				posCloth[i][j] = posCloth[i][j] + (posCloth[i][j] - lastPosCloth[i][j]) + (sumFCloth[i][j] / mass)*pow(dt, 2);
				lastPosCloth[i][j] = auxPos;
				//Velocity:
				velCloth[i][j] = (posCloth[i][j] - lastPosCloth[i][j]) / dt;
			}
		}
	}
}