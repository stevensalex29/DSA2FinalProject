#include "AppClass.h"
#include <chrono>

using namespace std;
fstream file;
int numberFramesWritten = 0;
int numberFramesRead = 0;
bool writing = false;
bool reading = false;

struct
{
	float posX;
	float posY;
	float posZ;
	float rot;
} shipRecordData[9999];

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

int Simplex::Application::ClosestPositionIndex()
{
	int curshortest = -1;
	float curshortestdistance = 1000.0f;
	for (int i = 0; i < racetrackList[0]->configuration->conelength; i++) {
		if (curshortestdistance > glm::distance(racetrackList[0]->m_vConeSetPositions[i], v3Position)) {
			curshortestdistance = glm::distance(racetrackList[0]->m_vConeSetPositions[i], v3Position);
			curshortest = i;
		}
	}
	return curshortest;
}

void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the ArcBall active?
	ArcBall();


	Racetrack * firsttrack = racetrackList[0];

	vector3 halfWidth = firsttrack->m_eTrafficConesList[0]->GetRigidBody()->GetHalfWidth();
	float offset = 1.5f;
	float maxDistance = m_fConeSpan + halfWidth.x + offset;

	//find reset position
	if (ClosestPositionIndex() > m_uCurrentConeIndex) {
		m_vResetPosition = firsttrack->m_vConeSetPositions[ClosestPositionIndex()];
	}

	//check bounds
	if (glm::distance(v3Position, m_vResetPosition) > maxDistance) {
		m_uCurrentConeIndex -= 3;
		if (m_uCurrentConeIndex < 0) {
			m_uCurrentConeIndex = 0;
		}
		if (firsttrack->m_vConeSetPositions[0] == m_vResetPosition) {
			v3Position = firsttrack->m_vConeSetPositions[1];
		}
		else {
			v3Position = m_vResetPosition;
		}
		m_vResetPosition = firsttrack->m_vConeSetPositions[ClosestPositionIndex()];
	}
	m_uCurrentConeIndex = ClosestPositionIndex();

	// check reached finish

	if (oldIndex > 90 && m_uCurrentConeIndex < 10) {
		end = std::chrono::steady_clock::now();
		m_dLastTime = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
		if (m_dLastTime < m_dBestTime)m_dBestTime = m_dLastTime;
		//reset timer and change track
		start = std::chrono::steady_clock::now();
	}
	oldIndex = m_uCurrentConeIndex;

	if (m_uCurrentConeIndex == firsttrack->configuration->conelength && !lap) {
		lap = true;
		// end timer, update best time
		
	}
	if (m_uCurrentConeIndex != firsttrack->configuration->conelength && lap) {
		lap = false;
	}

	// update current time
	auto curr = std::chrono::steady_clock::now();
	m_dCurrentTime = std::chrono::duration_cast<std::chrono::seconds>(curr-start).count();

	// move forward
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
	{
		v3Position += vector3(0.5f, 0.0f, 0.5f) * vector3(sin(shipRot), 0, cos(shipRot));
	}

	// move backward
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
	{
		v3Position += vector3(-0.5f, 0.0f, -0.5f) * vector3(sin(shipRot), 0, cos(shipRot));
	}

	// steer left
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		shipRot += 0.05f;
	}

	// steer right
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		shipRot -= 0.05f;
	}

	// start writing
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::O))
	{
		numberFramesWritten = 0;
		numberFramesRead = 0;
		writing = true;
		reading = false;
	}

	// start reading
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::P))
	{
		numberFramesWritten = 0;
		numberFramesRead = 0;
		writing = false;
		reading = true;
		file.open("shipData.dat", ios::in | ios::binary);

		//source\repos\DSA2FinalProject\_Binary\shipData.dat

		file.read(reinterpret_cast<char*>(&shipRecordData[0]), sizeof(shipRecordData[0]) * 9999);
		file.close();
	}

	// stop reading and writing
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::I))
	{
		if (writing)
		{
			printf("Writing...\n");
			file.open("shipData.dat", ios::out | ios::binary);

			//source\repos\DSA2FinalProject\_Binary\shipData.dat
			file.write(reinterpret_cast<char*>(&shipRecordData[0]), sizeof(shipRecordData[0]) * 9999);
			file.close();
		}

		numberFramesWritten = 0;
		numberFramesRead = 0;
		writing = false;
		reading = false;
	}

	if (writing)
	{
		shipRecordData[numberFramesWritten].posX = v3Position.x;
		shipRecordData[numberFramesWritten].posY = v3Position.y;
		shipRecordData[numberFramesWritten].posZ = v3Position.z;
		shipRecordData[numberFramesWritten].rot = shipRot;

		numberFramesWritten++;

		printf("recording\n");
	}

	if (reading)
	{
		v3Position.x = shipRecordData[numberFramesRead].posX;
		v3Position.y = shipRecordData[numberFramesRead].posY;
		v3Position.z = shipRecordData[numberFramesRead].posZ;
		shipRot = shipRecordData[numberFramesRead].rot;

		printf("%f %f %f\n", v3Position.x, v3Position.y, v3Position.z);

		numberFramesRead++;

		printf("Hello WOrld\n");
	}

	// create the model matrix with position, rotation, and scale
	matrix4 m4ModelMatrix = IDENTITY_M4;
	m4ModelMatrix = glm::translate(m4ModelMatrix, v3Position);
	m4ModelMatrix = glm::rotate(m4ModelMatrix, shipRot, glm::vec3(0, 1, 0));
	m4ModelMatrix = glm::rotate(m4ModelMatrix, shipRot, glm::vec3(0, 0, -1));
	m4ModelMatrix = glm::scale(m4ModelMatrix, vector3(1.0f, 1.0f, 1.0f));
	
	//sets matrix of the ship
	m_eSpaceship->SetModelMatrix(m4ModelMatrix);

	vector3 camPos;

	desiredCameraRot = shipRot;

	// Reverse Camera
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
	{
		desiredCameraRot += 3.14159f;
	}

	if (cameraRot > desiredCameraRot)
	{
		if (abs(desiredCameraRot - cameraRot) >= 1.0f)
		{
			cameraRot -= 0.10f; // spin fast if you have a big difference
		}

		else
		{
			// spin slow if it is a small difference
			cameraRot -= 0.04f; // a little slower than ship's angular velocity
		}
	}

	if (cameraRot < desiredCameraRot)
	{
		if (abs(desiredCameraRot - cameraRot) >= 1.0f)
		{
			cameraRot += 0.10f; // spin fast if you have a big difference
		}

		else
		{
			// spin slow if it is a small difference
			cameraRot += 0.04f; // a little slower than ship's angular velocity
		}
	}

	// follow the ship
	camPos = vector3(
		v3Position.x - 5 * sin(cameraRot),
		v3Position.y + 2,
		v3Position.z - 5 * cos(cameraRot)
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