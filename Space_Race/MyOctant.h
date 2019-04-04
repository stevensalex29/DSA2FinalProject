/*----------------------------------------------
Programmer: Alex Stevens (axs9576@gmail.com)
Date: 2019/03
----------------------------------------------*/
#pragma once

#include "MyEntityManager.h"

namespace Simplex
{
	class MyOctant
	{
		static uint m_uOctantCount; // stores the number of octants instantiated
		static uint m_uMaxLevel; // stores the maximum level an octant can go to
		static uint m_uIdealEntityCount; // how many ideal entities this object will have

		uint m_uID = 0; // stores the current ID of the octant
		uint m_uLevel = 0; // stores the current level of the octant
		uint m_uChildren = 0; // number of children on the octant, (will be 0 or 8)

		float m_fSize = 0.0f; // size of the octant

		MeshManager* m_pMeshMngr = nullptr; // Mesh manager singleton
		MyEntityManager* m_pEntityMngr = nullptr; // Entity manager singleton

		vector3 m_v3Center = vector3(0.0f); // stores the center point of the octant
		vector3 m_v3Min = vector3(0.0f); // stores the minimum vector of the octant
		vector3 m_v3Max = vector3(0.0f); // stores the maximum vector of the octant

		MyOctant* m_pParent = nullptr; // stores the parent of the octant
		MyOctant* m_pChild[8]; // stores the children of the octant

		std::vector<uint> m_EntityList; // the list of entities under the octant

		MyOctant* m_pRoot = nullptr; // the root octant
		std::vector<MyOctant*> m_lChild; // the list of nodes that contain objects

	public:
		/*
		Usage: Constructor that will create an octant containing all entity instances in the mesh manager
		Arguments:
		- uint a_uMaxLevel  = 2 -> Sets the maximum level of subdivision
		- uint a_nIdealEntityCount = 5 -> Sets the ideal level of objects per octant
		Output: class object
		*/
		MyOctant(uint a_uMaxLevel = 2, uint a_nIdealEntityCount = 5);
		/*
		Usage: Constructor
		Arguments:
		- vector3 a_v3Center -> Center of the octant in global space
		- float a_fSize -> size of each side of the octant volume
		Output: class object
		*/
		MyOctant(vector3 a_v3Center, float a_fSize);
		/*
		Usage: Copy Constructor
		Arguments: class object to copy
		Output: class object instance
		*/
		MyOctant(MyOctant const& other);
		/*
		Usage: Copy Assignment Operator
		Arguments: class object to copy
		Output: ---
		*/
		MyOctant& operator=(MyOctant const& other);
		/*
		Usage: Destructor
		Arguments: ---
		Output: ---
		*/
		~MyOctant(void);
		/*
		Usage: Changes object contents for other object's
		Arguments:
		- MyOctant& other -> object to swap content from
		Output: ---
		*/
		void Swap(MyOctant& other);
		/*
		Usage: Gets this octant's size
		Arguments: ---
		Output: size of octant
		*/
		float GetSize(void);
		/*
		Usage: Gets the center of the octant in global space
		Arguments: ---
		Output: Center of the octant in global space
		*/
		vector3 GetCenterGlobal(void);
		/*
		Usage: Gets the min corner of the octant in global space
		Arguments: ---
		Output: Minimum in global space
		*/
		vector3 GetMinGlobal(void);
		/*
		Usage: Gets the max corner of the octant in global space
		Arguments: ---
		Output: Maximum in global space
		*/
		vector3 GetMaxGlobal(void);
		/*
		Usage: Asks if there is a collision with the Entity specified by index from the Bounding Object Manager
		Arguments:
		- uint a_uRBIndex -> Index of the entity in the entity manager
		Output: check of the collision
		*/
		bool IsColliding(uint a_uRBIndex);
		/*
		Usage: Displays the MyOctant volume specified by index including the objects underneath
		Arguments:
		- uint a_nIndex -> MyOctant to be displayed
		- vector3 a_v3Color = C_YELLOW -> Color of the volume to display
		Output: ---
		*/
		void Display(uint a_nIndex, vector3 a_v3Color = C_YELLOW);
		/*
		Usage: Displays the MyOctant volume in the color specified
		Arguments:
		- vector3 a_v3Color = C_YELLOW -> Color of the volume to display
		Output: ---
		*/
		void Display(vector3 a_v3Color = C_YELLOW);
		/*
		Usage: Displays the non empty leafs in the octree
		Arguments:
		- vector3 a_v3Color = C_YELLOW -> Color of the volume to display
		Output: ---
		*/
		void DisplayLeafs(vector3 a_v3Color = C_YELLOW);
		/*
		Usage: Clears the Entity List for each node
		Arguments: ---
		Output: ---
		*/
		void ClearEntityList(void);
		/*
		Usage: Allocates 8 smaller octants in the child pointers
		Arguments: ---
		Output: ---
		*/
		void Subdivide(void);
		/*
		Usage: Returns the child specified in the index
		Arguments: uint a_nChild -> index of the child from 0 to 7
		Output: MyOctant object of the child in index
		*/
		MyOctant* GetChild(uint a_nChild);
		/*
		Usage: Returns the parent of the octant
		Arguments: ---
		Output: MyOctant object parent
		*/
		MyOctant* GetParent(void);
		/*
		Usage: Asks the MyOctant if it does not contain any children (a leaf)
		Arguments: ---
		Output: If contains no children
		*/
		bool IsLeaf(void);
		/*
		Usage: Asks the MyOctant if it contains more than this many Bounding Objects
		Arguments:
		- uint a_nEntities -> Number of Entities to query
		Output: It contains at least this many Entities
		*/
		bool ContainsMoreThan(uint a_nEntities);
		/*
		Usage: Deletes all children and the children of their children
		Arguments: ---
		Output: ---
		*/
		void KillBranches(void);
		/*
		Usage: Creates a tree using subdivisions, the max number of objects and levels
		Arguments:
		- uint a_nMaxLevel = 3 -> Sets the maximum level of the tree while constructing it
		Output: ---
		*/
		void ConstructTree(uint a_nMaxLevel = 3);
		/*
		Usage: Traverse the tree up to the leafs and sets the objects in them to the index
		Arguments: ---
		Output: ---
		*/
		void AssignIDtoEntity(void);
		/*
		Usage: Gets the total number of octants in the world
		Arguments: ---
		Output: ---
		*/
		uint GetOctantCount(void);

	private:
		/*
		Usage: Deallocates member fields
		Arguments: ---
		Output: ---
		*/
		void Release(void);
		/*
		Usage: Allocates member fields
		Arguments: ---
		Output: ---
		*/
		void Init(void);
		/*
		Usage: Creates the list of all leafs that contain objects
		Arguments: ---
		Output: ---
		*/
		void ConstructList(void);
	}; // class
}// namespace Simplex
