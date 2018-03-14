#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <glm\glm.hpp>
#include <iostream>


//Functions:
void PhysicsInit();
void verletSolver(float dt);
void updateForces();
glm::vec3 springForce(glm::vec3 p1, glm::vec3 p2, glm::vec3 v1, glm::vec3 v2, glm::vec2 k, float particleLinkDistance);
void allParticlePlaneCollisions();
void particlePlaneCollision(glm::vec3 normal, float d, int i, int j);
void checkParticlePlaneCollision(glm::vec3 normal, float d, int i, int j);
void sphereCollisions();

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
float stretchDistance = 0.5f;
float shearDistance, bendDistance;
bool useCollisions = true;
extern bool renderSphere = true;
extern bool renderCloth = true;
float elasticCoefficient = 0.5f;
float frictionCoefficient = 0.1f;
#pragma endregion

//Time:
float resetTime;
float deltaTime;
const float reducer = 10.0f;
//Sphere:
glm::vec3 spherePosition;
float sphereRadius;
//Planes:
glm::vec3 XplaneNormal = { 1,0,0 };
glm::vec3 YplaneNormal = { 0,1,0 };
glm::vec3 ZplaneNormal = { 0,0,1 };

//Cloth:
//Position:
glm::vec3 posCloth[18][14];
glm::vec3 lastPosCloth[18][14];
glm::vec3 auxPos;
//Velocity:
glm::vec3 velCloth[18][14];
glm::vec3 lastVelCloth[18][14];
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
			ImGui::DragFloat("Particle Link Distance", &stretchDistance, 0.05f);

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
	shearDistance = sqrt(2)*stretchDistance;
	bendDistance = 2 * stretchDistance;

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
		auxPos.z += stretchDistance;
		auxPos.x = -4.0f;
		for (int j=0; j<14 ;j++)
		{
			//Position:
			auxPos.x += stretchDistance;
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
	deltaTime = dt / reducer;
	if (playSimulation)
	{
		if (resetTime >= totalResetTime)
		{
			clicked++;
		}
		else
		{
			resetTime += dt;

			//for best performance, we reduce the given time and operate 
			for (int i = 0; i <= reducer; i++)
			{
				//Forces:
				updateForces();
				//Position and Velocity:
				verletSolver(deltaTime);

				//Collisions:
				if (useCollisions)
				{
					allParticlePlaneCollisions();
					//sphereCollisions();
				}
			}
			ClothMesh::updateClothMesh((float*)posCloth);
		}
	}
}

void PhysicsCleanup() {

}
void updateForces()
{
	for (int i = 0; i < 18; i++)
	{
		for (int j = 0; j < 14; j++)
		{
			if (i == 0 && (j == 0 || j == 13))
			{
				//do nothing
			}
			else
			{
				//gravity Force
				sumFCloth[i][j] = gravityAccel * mass;

				if (i - 1 >= 0)
				{
					//stretch up
					sumFCloth[i][j]+= springForce(posCloth[i][j], posCloth[i-1][j], velCloth[i][j], velCloth[i-1][j], k_stretch, stretchDistance);
					if (i - 2 >= 0)
					{
						//bending up
						sumFCloth[i][j] += springForce(posCloth[i][j], posCloth[i - 2][j], velCloth[i][j], velCloth[i - 2][j], k_bend, bendDistance);
					}
					if (j - 1 >= 0)
					{
						//shear left
						sumFCloth[i][j] += springForce(posCloth[i][j], posCloth[i - 1][j - 1], velCloth[i][j], velCloth[i - 1][j - 1], k_shear, shearDistance);
					}
					if (j + 1 < 14)
					{
						//shear right
						sumFCloth[i][j] += springForce(posCloth[i][j], posCloth[i - 1][j + 1], velCloth[i][j], velCloth[i - 1][j + 1], k_shear, shearDistance);
					}
				}
				//vertical stretch down
				if (i + 1 < 18)
				{
					sumFCloth[i][j] += springForce(posCloth[i][j], posCloth[i + 1][j], velCloth[i][j], velCloth[i + 1][j], k_stretch, stretchDistance);
					if (i + 2 < 18)//bending down
					{
						sumFCloth[i][j] += springForce(posCloth[i][j], posCloth[i + 2][j], velCloth[i][j], velCloth[i + 2][j], k_bend, bendDistance);
					}
					if (j - 1 >= 0)
					{
						//shear left
						sumFCloth[i][j] += springForce(posCloth[i][j], posCloth[i + 1][j - 1], velCloth[i][j], velCloth[i + 1][j - 1], k_shear, shearDistance);
					}
					if (j + 1 < 14)
					{
						//shear right
						sumFCloth[i][j] += springForce(posCloth[i][j], posCloth[i + 1][j + 1], velCloth[i][j], velCloth[i + 1][j + 1], k_shear, shearDistance);
					}
				}
				//horizontal stretch left
				if (j - 1 >= 0)
				{
					sumFCloth[i][j] += springForce(posCloth[i][j], posCloth[i][j - 1], velCloth[i][j], velCloth[i][j - 1], k_stretch, stretchDistance);
					if (j - 2 >= 0)//bending left
					{
						sumFCloth[i][j] += springForce(posCloth[i][j], posCloth[i][j - 2], velCloth[i][j], velCloth[i][j - 2], k_bend, bendDistance);
					}
				}
				//horizontal stretch right
				if (j + 1 < 14)
				{
					sumFCloth[i][j] += springForce(posCloth[i][j], posCloth[i][j + 1], velCloth[i][j], velCloth[i][j + 1], k_stretch, stretchDistance);
					if (j + 2 < 14)//bending right
					{
						sumFCloth[i][j] += springForce(posCloth[i][j], posCloth[i][j + 2], velCloth[i][j], velCloth[i][j + 2], k_bend, bendDistance);
					}
				}
			}
		}
	}
}

glm::vec3 springForce(glm::vec3 p1, glm::vec3 p2, glm::vec3 v1, glm::vec3 v2, glm::vec2 k, float particleLinkDistance)
{
	return -(k.x*(glm::distance(p1, p2) - particleLinkDistance) + k.y*glm::dot((v1 - v2), (p1 - p2) / glm::distance(p1, p2)))*(p1 - p2) / glm::distance(p1, p2);
}


void verletSolver(float dt)
{
	for (int i = 0; i < 18; i++)
	{
		for (int j = 0; j < 14; j++)
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
				lastVelCloth[i][j] = velCloth[i][j];
				velCloth[i][j] = (posCloth[i][j] - lastPosCloth[i][j]) / dt;
			}
		}
	}
}

void allParticlePlaneCollisions()
{
	for (int i = 0; i < 18; i++)
	{
		for (int j = 0; j < 14; j++)
		{
			//left plane
			checkParticlePlaneCollision(XplaneNormal, 5, i, j);
			//right plane
			checkParticlePlaneCollision(-XplaneNormal, 5, i, j);
			//back plane
			checkParticlePlaneCollision(ZplaneNormal, 5, i, j);
			//front plane
			checkParticlePlaneCollision(-ZplaneNormal, 5, i, j);
			//down plane
			checkParticlePlaneCollision(YplaneNormal, 0, i, j);
			//up plane
			checkParticlePlaneCollision(-YplaneNormal, 10, i, j);
		}
	}
}

void checkParticlePlaneCollision(glm::vec3 normal, float d, int i, int j)
{
	if ((glm::dot(normal, lastPosCloth[i][j]) + d)*(glm::dot(normal, posCloth[i][j]) + d) <= 0.f)
	{
		particlePlaneCollision(normal, d, i, j);
	}
}

void particlePlaneCollision(glm::vec3 normal, float d, int i, int j)
{

	////Position:
	//auxPos = posCloth[i][j];
	//posCloth[i][j] = posCloth[i][j] + (posCloth[i][j] - lastPosCloth[i][j]) + (sumFCloth[i][j] / mass)*pow(deltaTime, 2);
	//lastPosCloth[i][j] = auxPos;
	////Velocity:
	//lastVelCloth[i][j] = velCloth[i][j];
	//velCloth[i][j] = (posCloth[i][j] - lastPosCloth[i][j]) / deltaTime;
}

void sphereCollisions()
{
	glm::vec3  normalTangentPlane;
	float d;
	glm::vec3 normalVelocity;
	glm::vec3 tangentVelocity;
	for (int i = 0; i < 18; i++)
	{
		for (int j = 0; j < 14; j++)
		{
			if (glm::distance(posCloth[i][j], spherePosition) < sphereRadius)
			{
				auxPos = lastPosCloth[i][j];
				while (glm::distance(auxPos, spherePosition) > sphereRadius)
				{
					auxPos += lastVelCloth[i][j] * 0.1f;
				}
				normalTangentPlane = auxPos - spherePosition;
				d = -glm::dot(normalTangentPlane, auxPos);
				posCloth[i][j] = posCloth[i][j] - (1.0f + elasticCoefficient) * (glm::dot(normalTangentPlane, posCloth[i][j]) + d)*(normalTangentPlane);
				normalVelocity = glm::dot(normalTangentPlane, lastVelCloth[i][j])*normalTangentPlane;
				tangentVelocity = lastVelCloth[i][j] - normalVelocity;
				velCloth[i][j] = velCloth[i][j] - (1.0f + elasticCoefficient) * glm::dot(normalTangentPlane, velCloth[i][j])*normalTangentPlane - frictionCoefficient * tangentVelocity;
			}
		}
	}
}