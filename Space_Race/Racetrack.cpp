#include "Racetrack.h"

using namespace Simplex;

Racetrack::Racetrack(MyEntityManager ** m_ent, uint* uIndex)
{
	m_eTrafficConesList = new MyEntity*[10000];
	m_vConeSetPositions = new vector3[10000];
	m_uNumConePositions = 0;
	configuration = new RacetrackConfiguration();
	this->uIndex = uIndex;
	this->m_ent = *m_ent;
}

Racetrack::~Racetrack()
{
	delete m_eTrafficConesList;
	delete m_vConeSetPositions;
	delete configuration;
}

void Simplex::Racetrack::CreateTrafficConeAt(vector3 position, vector3 size) {
	m_ent->AddEntity("AndyIsTheTeamArtist\\TrafficCone.obj");
	matrix4 trafficConeMatrix = glm::translate(position);
	m_ent->SetModelMatrix(trafficConeMatrix * glm::scale(size));
	m_eTrafficConesList[configuration->numcones] = m_ent->GetEntity(*uIndex);
	(*uIndex)++;
	configuration->numcones++;
}

void Simplex::Racetrack::CreateTrafficConeRowAt(vector3 startPos, float span, float xPosDegreeAngle) {
	CreateTrafficConeAt(vector3(startPos.x - cos(glm::radians(90.0f) + xPosDegreeAngle) * span, startPos.y, startPos.z - sin(glm::radians(90.0f) + xPosDegreeAngle) * span), vector3(0.2f));
	CreateTrafficConeAt(vector3(startPos.x + cos(glm::radians(90.0f) + xPosDegreeAngle) * span, startPos.y, startPos.z + sin(glm::radians(90.0f) + xPosDegreeAngle) * span), vector3(0.2f));
	m_vConeSetPositions[m_uNumConePositions] = startPos;
	m_uNumConePositions++;
}

void Simplex::Racetrack::CreateRaceTrackOf(RacetrackConfiguration config)
{
	float curangle = glm::radians(90.0f);
	vector3 curPos = config.startPos;
	for (int i = 0; i < config.conelength; i++) {
		CreateTrafficConeRowAt(curPos, config.basespan, curangle);
		curangle += config.variance;
		curPos += vector3(cos(curangle) * config.conespacing, 0.0f, sin(curangle) * config.conespacing);
	}
}
