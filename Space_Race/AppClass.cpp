#include "AppClass.h"
#include <chrono>

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


	// create traffic cones
	racetrackList = new Racetrack*[20];
	racetrackList[0] = new Racetrack(&m_pEntityMngr, &uIndex);
	Racetrack* firsttrack = racetrackList[0];
	firsttrack->configuration->basespan = 6.0f;
	firsttrack->CreateRaceTrackOf(*(firsttrack->configuration));
	m_fConeSpan = firsttrack->configuration->basespan;

	//firsttrack->CreateRaceTrackOf();
	//vector3(0.0f, 0.0f, 0.0f), 100, 2.0f, 2.5f, glm::radians(3.6f), 0.0f, 0.0f

	// set initial reset position, next reset position, and finish position

	if (firsttrack->m_uNumConePositions > 0) {
		m_vResetPosition = firsttrack->m_vConeSetPositions[m_uCurrentConeIndex];
		m_uCurrentConeIndex++;
		if (m_uCurrentConeIndex < firsttrack->m_uNumConePositions) m_vNextResetPosition = firsttrack->m_vConeSetPositions[m_uCurrentConeIndex];
		m_vFinishPosition = firsttrack->m_vConeSetPositions[firsttrack->m_uNumConePositions - 1];
	}
	
	// create spaceship
	v3Position = m_vResetPosition;
	m_pEntityMngr->AddEntity("AndyIsTheTeamArtist\\Spaceship.obj");
	matrix4 m4Position = glm::translate(v3Position);
	m_pEntityMngr->SetModelMatrix(m4Position);
	m_eSpaceship = m_pEntityMngr->GetEntity(uIndex);
	uIndex++;

	// create octree
	m_uOctantLevels = 1;
	m_pRoot = new MyOctant(m_uOctantLevels, 5);
	m_pEntityMngr->Update();

	// start timer
	start = std::chrono::steady_clock::now();
}

MyEntityManager * Simplex::Application::getEntityManager()
{
	return m_pEntityMngr;
}
int Simplex::Application::getuIndex()
{
	return uIndex;
}
void Simplex::Application::incrementuIndex()
{
	uIndex += 1;
}

void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the ArcBall active?
	ArcBall();


	Racetrack * firsttrack = racetrackList[0];

	// ANDY - CAMERA GETS MESSED UP WHEN GO BACKWARDS AT START AND STOPS RESETTING AFTER FIRST LAP

	// check for next race position
	vector3 halfWidth = firsttrack->m_eTrafficConesList[0]->GetRigidBody()->GetHalfWidth();
	float offset = 1.5f;
	float nextRaceOffset = m_fConeSpan + halfWidth.x + offset;
	if (glm::distance(v3Position, m_vNextResetPosition) < nextRaceOffset) {
		m_vResetPosition = firsttrack->m_vConeSetPositions[m_uCurrentConeIndex];
		m_uCurrentConeIndex++;
		if (m_uCurrentConeIndex < firsttrack->m_uNumConePositions) m_vNextResetPosition = firsttrack->m_vConeSetPositions[m_uCurrentConeIndex];
		if (m_bCircularTrackReset) m_bCircularTrackReset == false;
	}

	// check bounds
	float maxDistance = m_fConeSpan + halfWidth.x + offset;
	if (glm::distance(v3Position, m_vResetPosition) > maxDistance) {
		m_uCurrentConeIndex -= 5;
		if (m_uCurrentConeIndex < 0) {
			m_uCurrentConeIndex = 0;
		}
		m_vResetPosition = firsttrack->m_vConeSetPositions[m_uCurrentConeIndex];
		m_uCurrentConeIndex++;
		if (m_uCurrentConeIndex < firsttrack->m_uNumConePositions) m_vNextResetPosition = firsttrack->m_vConeSetPositions[m_uCurrentConeIndex];
		if(!m_bCircularTrackReset)v3Position = m_vResetPosition;
	}

	// check reached finish (resets back to start for now)
	if (m_vResetPosition == m_vFinishPosition) {
		m_uCurrentConeIndex = 0;
		if (!firsttrack->configuration->circularTrack)v3Position = firsttrack->m_vConeSetPositions[m_uCurrentConeIndex];
		else m_bCircularTrackReset = true;
		m_vResetPosition = firsttrack->m_vConeSetPositions[m_uCurrentConeIndex];
		m_uCurrentConeIndex++;
		m_vNextResetPosition = firsttrack->m_vConeSetPositions[m_uCurrentConeIndex];
		// end timer, update best time
		end = std::chrono::steady_clock::now();
		m_dLastTime = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
		if (m_dLastTime < m_dBestTime)m_dBestTime = m_dLastTime;
		// reset timer and change track
		start = std::chrono::steady_clock::now();
	}

	// update current time
	auto curr = std::chrono::steady_clock::now();
	m_dCurrentTime = std::chrono::duration_cast<std::chrono::seconds>(curr-start).count();

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
	m4ModelMatrix = glm::scale(m4ModelMatrix, vector3(1.0f, 1.0f, 1.0f));
	
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


	// Needs to reconstruct octree (player might move as well as objects)
	m_pEntityMngr->ClearDimensionSetAll();
	SafeDelete(m_pRoot);
	m_pRoot = new MyOctant(m_uOctantLevels, 5);

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

	delete racetrackList;

	//release GUI
	ShutdownGUI();
}