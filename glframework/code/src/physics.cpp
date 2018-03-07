#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <glm\glm.hpp>


//Functions:
void PhysicsInit();
void positionVerletSolver(float dt);
void velocityVerletSolver(float dt);

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

//Other variables:
float resetTime;
//Sphere:
glm::vec3 spherePosition;
float sphereRadius;
//Cloth:
glm::vec3 clothArray[18][14];
glm::vec3 lastClothArray[18][14];
glm::vec3 auxPos;
const float mass = 1.0f;
//Forces:
glm::vec3 sumF;


bool show_test_window = false;
void GUI()
{
	bool show = true;
	ImGui::Begin("Physics Parameters", &show, 0);

	{
		//PREGUNTA: COM FER QUE COMENCIN ELS TREES OBERTS?
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
	sumF = { 0.0f, 0.0f, 0.0f };

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
			auxPos.x += particleLinkDistance;
			clothArray[i][j] = { auxPos.x , 9.5f , auxPos.z };
			lastClothArray[i][j] = { auxPos.x , 9.5f , auxPos.z };
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
				//update Forces:
				sumF = gravityAccel*mass;
				//update Positions:
				positionVerletSolver(dt);
				//update Velocities:
				velocityVerletSolver(dt);
			}
			ClothMesh::updateClothMesh((float*)clothArray);
		}
	}
}

void PhysicsCleanup() {

}

void positionVerletSolver(float dt)
{
	for (int i = 0; i<18; i++)
	{
		for (int j = 0; j<14; j++)
		{
			if (i == 0 && (j == 0 || j == 13))
			{
				break;
			}
			auxPos = clothArray[i][j];
			clothArray[i][j] = clothArray[i][j] + (clothArray[i][j] - lastClothArray[i][j]) + (sumF / mass)*pow(dt, 2);
			lastClothArray[i][j] = auxPos;
		}
	}
}

void velocityVerletSolver(float dt)
{

}