#pragma once
#include "RacetrackConfiguration.h"
#include "Definitions.h"
#include "MyEntity.h"
#include "MyEntityManager.h"

namespace Simplex {

	class Racetrack
	{
	public:
		RacetrackConfiguration* configuration;
		uint m_uNumConePositions;
		MyEntity** m_eTrafficConesList;
		MyEntityManager* m_ent;
		vector3* m_vConeSetPositions;
		uint* uIndex;

		Racetrack(MyEntityManager** m_ent, uint* uIndex);
		~Racetrack();
		void CreateRaceTrackOf(RacetrackConfiguration config);
		/*
		USAGE: Creates a row of (two) traffic cones at a position. The span is the radius of how far apart the
		traffic cones are from each other. The xPosDegreeAngle is the rotation along the x axis that the span is.
		*/
		void CreateTrafficConeRowAt(vector3 startPos, float span, float xPosDegreeAngle);
		/*
		USAGE: Creates a traffic cone object and adds it to the entity manager.
		ARGUMENTS: position, size. These attributes apply directly to the traffic cone model and bounding box.
		*/
		void CreateTrafficConeAt(vector3 position, vector3 size);
	};
}

