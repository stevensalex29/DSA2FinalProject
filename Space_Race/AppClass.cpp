#include "AppClass.h"
using namespace Simplex;
void Application::InitVariables(void)
{
	//Set the position and target of the camera
	m_pCameraMngr->SetPositionTargetAndUpward(
		vector3(0.0f, 2.0f, -5.0f), //Position
		vector3(0.0f, 0.0f, 0.0f),	//Target
		AXIS_Y);					//Up



	m_pLightMngr->SetPosition(vector3(0.0f, 3.0f, 13.0f), 1); //set the position of first light (0 is reserved for ambient light)

#ifdef DEBUG
	uint uInstances = 1;
#else
	uint uInstances = 1;
#endif
	//int nSquare = static_cast<int>(std::sqrt(uInstances));
	//m_uObjects = nSquare * nSquare;
	//for (int i = 0; i < nSquare; i++)
	//{
		//for (int j = 0; j < nSquare; j++)
		//{
			// Add objects to entity manager here
			//uIndex++;
			
			//moved all below
		//}
	//}

	m_pEntityMngr->AddEntity("AndyIsTheTeamArtist\\Spaceship.obj");
	v3Position = vector3(0.0f, 0.0f, 0.0f);
	matrix4 m4Position = glm::translate(v3Position);
	m_pEntityMngr->SetModelMatrix(m4Position);
	m_eSpaceship = m_pEntityMngr->GetEntity(uIndex);
	uIndex++;

	m_eTrafficConesList = new MyEntity*[100];

	CreateTrafficConeRowAt(vector3(5.0f, 0.0f, 1.0f), 3.0f, 0.0f);
	CreateTrafficConeRowAt(vector3(5.0f, 0.0f, 2.0f), 3.0f, 0.0f);
	CreateTrafficConeRowAt(vector3(5.0f, 0.0f, 3.0f), 3.0f, 0.0f);
	CreateTrafficConeRowAt(vector3(5.0f, 0.0f, 4.0f), 3.0f, 0.0f);
	CreateTrafficConeRowAt(vector3(5.0f, 0.0f, 5.0f), 3.0f, 0.0f);

	m_uOctantLevels = 1;
	m_pRoot = new MyOctant(m_uOctantLevels, 5);
	m_pEntityMngr->Update();
}

void Application::CreateTrafficConeRowAt(vector3 startPos, float span, float xPosDegreeAngle) {
	CreateTrafficConeAt(vector3(startPos.x - span, startPos.y, startPos.z), vector3(0.2f));
	CreateTrafficConeAt(vector3(startPos.x + span, startPos.y, startPos.z), vector3(0.2f));
}

void Application::CreateTrafficConeAt(vector3 position, vector3 size) {
	m_pEntityMngr->AddEntity("AndyIsTheTeamArtist\\TrafficCone.obj");
	vector3 trafficConePos = position;
	vector3 trafficConeScale = size;
	matrix4 trafficConeScaleMatrix = glm::translate(trafficConeScale);
	matrix4 trafficConeMatrix = glm::translate(v3Position);
	trafficConeMatrix = glm::scale(glm::translate(IDENTITY_M4, trafficConePos), trafficConeScale);
	m_pEntityMngr->SetModelMatrix(trafficConeMatrix);
	m_eTrafficConesList[uIndex] = m_pEntityMngr->GetEntity(uIndex);
	uIndex++;
}

void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the ArcBall active?
	ArcBall();

	// move forward
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
	{
		v3Position += vector3(0.5f, 0.0f, 0.5f) * vector3(sin(rot), 0, cos(rot));
	}

	// move backward
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
	{
		v3Position += vector3(-0.5f, 0.0f, -0.5f) * vector3(sin(rot), 0, cos(rot));
	}

	// steer left
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		rot += 0.05f;
	}

	// steer right
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		rot -= 0.05f;
	}

	// create the model matrix with position, rotation, and scale
	matrix4 m4ModelMatrix = IDENTITY_M4;
	m4ModelMatrix = glm::translate(m4ModelMatrix, v3Position);
	m4ModelMatrix = glm::rotate(m4ModelMatrix, rot, glm::vec3(0, 1, 0));
	m4ModelMatrix = glm::scale(m4ModelMatrix, vector3(2.0f, 2.0f, 2.0f));
	
	//sets matrix of the ship
	m_eSpaceship->SetModelMatrix(m4ModelMatrix);

	vector3 camPos;

	// follow the ship
	camPos = vector3(
		v3Position.x - 5 * sin(rot),
		v3Position.y + 2,
		v3Position.z - 5 * cos(rot)
	);

	// stay in center of screen
	//camPos = vector3(0, 2, -5);

	m_pCameraMngr->SetPositionTargetAndUpward(
		camPos,
		v3Position,	//Target
		AXIS_Y);//Up


	//Update Entity Manager
	m_pEntityMngr->Update();

	//Add objects to render list
	m_pEntityMngr->AddEntityToRenderList(-1, true);
}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	//display octree
	if (m_uOctantID == -1)
		m_pRoot->Display();
	else
		m_pRoot->Display(m_uOctantID);
	
	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList();
	
	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();
	
	//draw gui,
	DrawGUI();
	
	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}
void Application::Release(void)
{
	// release the octree
	SafeDelete(m_pRoot);

	delete m_eTrafficConesList;

	//release GUI
	ShutdownGUI();
}