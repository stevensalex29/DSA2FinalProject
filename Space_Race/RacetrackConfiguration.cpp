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
	randomsection = 10;
	circularTrack = true;
}

RacetrackConfiguration::RacetrackConfiguration(vector3 startPos, int conelength, float basespan, float variance, float boundX, float boundY, int randomsection = 10, bool circularTrack = false)
{
	this->startPos = startPos;
	this->conelength = conelength;
	this->conespacing = conespacing;
	this->basespan = basespan;
	this->variance = variance;
	this->boundX = boundX;
	this->boundX = boundY;
	this->randomsection = randomsection;
	this->circularTrack = circularTrack;
}


RacetrackConfiguration::~RacetrackConfiguration()
{
}
