#include "AppClass.h"
#include <chrono>

using namespace std;
fstream readFile;
fstream writeFile;
int numberFramesWritten = 0;
int numberFramesRead = 0;
bool writing = false;
bool reading = false;
int lapNumber = 0;

struct
{
	float posX;
	float posY;
	float posZ;
	float rot;

} shipWriteData[9999], shipReadData[9999];

using namespace Simplex;
void Application::InitVariables(void)
{
	//Set the position and target of the camera
	m_pCameraMngr->SetPositionTargetAndUpward(
		vector3(0.0f, 2.0f, -5.0f), //Position
		vector3(0.0f, 0.0f, 0.0f),	//Target
		AXIS_Y);					//Up


	//initialize sound
	String sRoute = m_pSystem->m_pFolder->GetFolderData();
	sRoute += m_pSystem->m_pFolder->GetFolderAudio();

	//Background music
	m_soundBGM.openFromFile(sRoute + "space.wav");
	m_soundBGM.play();
	m_soundBGM.setLoop(true);

	//sound effects
	m_soundBuffer.loadFromFile(sRoute + "lapProgression.wav");
	m_sound.setBuffer(m_soundBuffer);

	

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
	
	// in a 3-lap race, the laps are 1, 2, 3
	lapNumber = 0;

	// create spaceship
	v3Position = vector3(0,0,-10); // start before starting line, like Mario Kart
	m_pEntityMngr->AddEntity("AndyIsTheTeamArtist\\Spaceship.obj");
	matrix4 m4Position = glm::translate(v3Position);
	m_pEntityMngr->SetModelMatrix(m4Position);
	m_eSpaceship = m_pEntityMngr->GetEntity(uIndex);
	uIndex++;

	// create another spaceship for viewing recordings of other laps
	v3PositionGhost = vector3(0, -99, 0); // start before starting line, like Mario Kart
	m_pEntityMngr->AddEntity("AndyIsTheTeamArtist\\Spaceship.obj");
	matrix4 m4PositionGhost = glm::translate(v3PositionGhost);
	m_pEntityMngr->SetModelMatrix(m4PositionGhost);
	m_eSpaceshipGhost = m_pEntityMngr->GetEntity(uIndex);
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

void Application::doCollisionStuffs() {
	Racetrack * firsttrack = racetrackList[0];
	for (int i = 0; i < firsttrack->configuration->numcones; i++) {
		if (firsttrack->m_eTrafficConesList[i]->IsColliding(m_eSpaceship)) {
			firsttrack->m_eTrafficConesList[i]->curvel += vector3(2.5f, 0.0f, 2.5f) * vector3(sin(shipRot), 0, cos(shipRot));
		}
		if (glm::length(firsttrack->m_eTrafficConesList[i]->curvel) > 0.0f) {
			for (int j = i; j < firsttrack->configuration->numcones; j++) {
				//j = i: so that we don't recheck cones and infinitely multiply the distance
				if (firsttrack->m_eTrafficConesList[j]->IsColliding(firsttrack->m_eTrafficConesList[i])) {
					firsttrack->m_eTrafficConesList[j]->curvel += vector3(1.5f, 0.0f, 1.5f) * firsttrack->m_eTrafficConesList[i]->curvel;
					firsttrack->m_eTrafficConesList[i]->curvel *= .4f;
				}
			}
		}
	}
	for (int i = 0; i < firsttrack->configuration->numcones; i++) {
		firsttrack->m_eTrafficConesList[i]->SetModelMatrix(glm::translate(firsttrack->m_eTrafficConesList[i]->GetModelMatrix(), firsttrack->m_eTrafficConesList[i]->curvel));
		firsttrack->m_eTrafficConesList[i]->curvel = firsttrack->m_eTrafficConesList[i]->curvel * 0.98f;
	}
}

void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the ArcBall active?
	ArcBall();


	Racetrack * firsttrack = racetrackList[0];

	doCollisionStuffs();

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
		
		// if race has not started yet
		if (lapNumber != 0)
		{
			// play finished lap sound
			m_sound.play();

			m_dLastTime = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
			if (m_dLastTime < m_dBestTime)m_dBestTime = m_dLastTime;
		}

		//reset timer and change track
		start = std::chrono::steady_clock::now();

		lapNumber++;
		numberFramesWritten = 0;
		numberFramesRead = 0;

		if (lapNumber >= 1)
		{
			writing = true;
		}

		/*
		
		READ ME

		This system of managing read and write assumes
		that this particular race is the first race you've ever done

		If you want it to read data from a previous race,
		that code comes later
		
		*/

		// start reading file if you are on lap 2 or more
		if (lapNumber >= 2)
		{
			reading = true;

			// assuming this is your first race ever
			// write the lap you just finished (previous lap)
			if (writing)
			{
				char filename[100];
				sprintf_s(filename, 99, "shipData%i.dat", lapNumber-1);
				writeFile.open(filename, ios::out | ios::binary);
				//source\repos\DSA2FinalProject\_Binary\shipData.dat

				writeFile.write(reinterpret_cast<char*>(&shipWriteData[0]), sizeof(shipWriteData[0]) * 9999);
				writeFile.close();
			}

			// assuming this is your first race ever
			// read the lap you just finished (previous lap)
			char filename[100];
			sprintf_s(filename, 99, "shipData%i.dat", lapNumber-1);
			readFile.open(filename, ios::in | ios::binary);
			//source\repos\DSA2FinalProject\_Binary\shipData.dat

			readFile.read(reinterpret_cast<char*>(&shipReadData[0]), sizeof(shipReadData[0]) * 9999);
			readFile.close();
		}

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

	// if race has not started
	if (lapNumber != 0)
	{
		m_dCurrentTime = std::chrono::duration_cast<std::chrono::seconds>(curr - start).count();
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
		for (int i = 0; i < firsttrack->configuration->numcones; i++) {
			firsttrack->m_eTrafficConesList[i]->curvel = vector3((rand() % 20 - rand() % 10 - 5) / 4.0f, 0.0f, (rand() % 20 - rand() % 10 - 5) / 4.0f);
		}
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::RShift)) {
		firsttrack->ResetPositions();
	}

	// add/subtract cones
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
	{
		for (int i = 0; i < 4; i++) {
			m_pEntityMngr->AddEntity("AndyIsTheTeamArtist\\TrafficCone.obj");
			matrix4 trafficConeMatrix = glm::translate(vector3((rand() % 200) - (rand() % 100), 0.0f, (rand() % 200) - (rand() % 100) - 50));
			m_pEntityMngr->SetModelMatrix(trafficConeMatrix * glm::scale(vector3((rand() % 10) * 0.05f)));
			firsttrack->m_eTrafficConesList[firsttrack->configuration->numcones] = m_pEntityMngr->GetEntity(uIndex);
			firsttrack->configuration->numcones += 1;
			(uIndex)++;
		}
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::RControl))
	{
		// let players subtract cones to a point
		if (firsttrack->configuration->numcones > firsttrack->configuration->conelength * 2) {
			for (int i = 0; i < 4; i++) {
				(uIndex)--;
				firsttrack->configuration->numcones -= 1;
				m_pEntityMngr->RemoveEntity((uIndex));
			}
		}
	}

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

	if (writing)
	{
		shipWriteData[numberFramesWritten].posX = v3Position.x;
		shipWriteData[numberFramesWritten].posY = v3Position.y;
		shipWriteData[numberFramesWritten].posZ = v3Position.z;
		shipWriteData[numberFramesWritten].rot = shipRot;

		numberFramesWritten++;

		printf("recording\n");
	}

	if (reading)
	{
		v3PositionGhost.x = shipReadData[numberFramesRead].posX;
		v3PositionGhost.y = shipReadData[numberFramesRead].posY;
		v3PositionGhost.z = shipReadData[numberFramesRead].posZ;
		shipRotGhost = shipReadData[numberFramesRead].rot;

		numberFramesRead++;
	}

	// create the model matrix with position, rotation, and scale
	matrix4 m4ModelMatrix = IDENTITY_M4;
	m4ModelMatrix = glm::translate(m4ModelMatrix, v3Position);
	m4ModelMatrix = glm::rotate(m4ModelMatrix, shipRot, glm::vec3(0, 1, 0));
	m4ModelMatrix = glm::rotate(m4ModelMatrix, cameraRot - shipRot, glm::vec3(0, 0, 1));
	m4ModelMatrix = glm::scale(m4ModelMatrix, vector3(1.0f, 1.0f, 1.0f));
	m_eSpaceship->SetModelMatrix(m4ModelMatrix);

	// set matrix of the ghost ship
	m4ModelMatrix = IDENTITY_M4;
	m4ModelMatrix = glm::translate(m4ModelMatrix, v3PositionGhost);
	m4ModelMatrix = glm::rotate(m4ModelMatrix, shipRotGhost, glm::vec3(0, 1, 0));
	m4ModelMatrix = glm::scale(m4ModelMatrix, vector3(1.0f, 1.0f, 1.0f));
	m_eSpaceshipGhost->SetModelMatrix(m4ModelMatrix);

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
			cameraRot -= 0.045f; // a little slower than ship's angular velocity
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
			cameraRot += 0.045f; // a little slower than ship's angular velocity
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

	// Toggle displays
	m_pEntityMngr->showCollisionBoxes(m_toggleCollisionDisplay);

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
		if(m_toggleOctreeDisplay)m_pRoot->Display();
	else
		if(m_toggleOctreeDisplay)m_pRoot->Display(m_uOctantID);
	
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