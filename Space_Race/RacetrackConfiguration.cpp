#include "RacetrackConfiguration.h"

using namespace Simplex;

RacetrackConfiguration::RacetrackConfiguration()
{
	//vector3(0.0f, 0.0f, 0.0f), 100, 2.0f, 2.5f, glm::radians(3.6f), 0.0f, 0.0f
	startPos = vector3(0.0f, 0.0f, 0.0f);
	conelength = 100;
	conespacing = 2.0f;
	basespan = 2.5f;
	variance = glm::radians(3.6f);
	boundX = 0.0f;
	boundZ = 0.0f;
}


RacetrackConfiguration::~RacetrackConfiguration()
{
}
