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
	uint uIndex = 0;
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
	
	m_uOctantLevels = 1;
	m_pRoot = new MyOctant(m_uOctantLevels, 5);
	m_pEntityMngr->Update();
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the ArcBall active?
	ArcBall();

	//gets position matrix
	matrix4 m4Position = glm::translate(IDENTITY_M4, v3Position);

	if (rot > 45)
		rot = 45;
	if (rot < -45)
		rot = -45;


	quaternion q2 = glm::angleAxis(glm::radians(rot), AXIS_Y);

	//sets position of the ship
	m_eSpaceship->SetModelMatrix(ToMatrix4(q2));

	//Set the position and target of the camera
	m_pCameraMngr->SetPositionTargetAndUpward(
		vector3(0.0f + v3Position.x, 2.0f, -5.0f+v3Position.z), //Position
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

	//release GUI
	ShutdownGUI();
}