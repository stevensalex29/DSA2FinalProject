#pragma once

#include "Definitions.h"
namespace Simplex {
	class RacetrackConfiguration
	{
		/*
	USAGE: Creates a track of cones depending on what values the user puts into the function.
	ARGUMENTS:
	startPos: starting position of the racetrack.
	conelength: how many sets of cones in total for the racetrack.
	conespacing: how far apart these cones are from each other.
	basespan: the radius of how far apart traffic cones are from each other, to use as a starter (average-ish) value for the track.
	variance: how quickly the racetrack changes.
	boundX: how big on the X component the track should try to fit into.
	boundZ: how big on the Z component the track should try to fit into.
	*/
	public:
		RacetrackConfiguration();
		~RacetrackConfiguration();
		vector3 startPos;
		int conelength;
		float conespacing;
		float basespan;
		float variance;
		float boundX;
		float boundZ;
	};
}

