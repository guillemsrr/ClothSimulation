#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <glm\glm.hpp>


//Functions:
void PhysicsInit();

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
glm::vec3 gravityAccel = { 0.0f,-9.81,0.0f };
glm::vec2 k_stretch = { 1000.0f,50.0f };
glm::vec2 k_shear = { 1000.0f,50.0f };
glm::vec2 k_bend = { 1000.0f,50.0f };
float particleLink = 0.5f;
bool useCollisions = false;
bool useSphereCollider = true;
float elasticCoefficient = 0.5f;
float frictionCoefficient = 0.1f;
#pragma endregion

//Other variables:
glm::vec3 spherePosition;
float sphereRadius;

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
		ImGui::InputFloat3("Gravity Accel", (float*)&gravityAccel);

		if (ImGui::TreeNode("Spring parameters"))
		{
			ImGui::InputFloat2("k_stretch", (float*)&k_stretch);
			ImGui::InputFloat2("k_shear", (float*)&k_shear);
			ImGui::InputFloat2("k_bend", (float*)&k_bend);
			ImGui::DragFloat("Particle Link", &particleLink, 0.05f);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Collisions"))
		{
			ImGui::Checkbox("Use Collisions", &useCollisions);
			ImGui::Checkbox("Use Sphere Collider", &useSphereCollider);
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
	if (useSphereCollider)//NO PINTA L'ESFERA!?
	{
		spherePosition = { -5.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (10.0f))), -5.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (10.0f))), static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (10.0f))) };
		sphereRadius = 1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (10.0f)));
		Sphere::updateSphere(spherePosition, sphereRadius);
	}
	//Iniciar la Cloth:


	
}

void PhysicsUpdate(float dt) {
	if (playSimulation)
	{
		if (useCollisions)
		{

		}
	}
}

//PREGUNTA: QUÈ S'HAVIA DE POSAR EN EL CLEANUP ANTERIOR? I AQUEST?
void PhysicsCleanup() {

}