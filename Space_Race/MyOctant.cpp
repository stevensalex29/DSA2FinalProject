#include "MyOctant.h"
using namespace Simplex;
uint MyOctant::m_uOctantCount = 0;
uint MyOctant::m_uMaxLevel = 10;
uint MyOctant::m_uIdealEntityCount = 5;
uint MyOctant::GetOctantCount(void) { return m_uOctantCount; }
void MyOctant::Init(void){
	m_uChildren = 0;

	// initialized member variables
	m_fSize = 0.0f;
	m_uID = m_uOctantCount;
	m_uLevel = 0;

	m_v3Center = vector3(0.0f);
	m_v3Min = vector3(0.0f);
	m_v3Max = vector3(0.0f);

	// get instances of mesh manager and entity manager
	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	// set nodes as null pointers
	m_pRoot = nullptr;
	m_pParent = nullptr;
	for (uint n = 0; n < 8; n++) {
		m_pChild[n] = nullptr;
	}
}
void MyOctant::Swap(MyOctant& other) {
	std::swap(m_uChildren, other.m_uChildren);

	// swap the member variables of this object with the other object
	std::swap(m_fSize, other.m_fSize);
	std::swap(m_uID, other.m_uID);
	std::swap(m_pRoot, other.m_pRoot);
	std::swap(m_lChild, other.m_lChild);

	std::swap(m_v3Center, other.m_v3Center);
	std::swap(m_v3Min, other.m_v3Min);
	std::swap(m_v3Max, other.m_v3Max);

	// set the mesh manager and entity manager
	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	// swap the current nodes with the other object's nodes
	std::swap(m_uLevel, other.m_uLevel);
	std::swap(m_pParent, other.m_pParent);
	for (uint i = 0; i < 8; i++) {
		std::swap(m_pChild[i], other.m_pChild[i]);
	}
}
MyOctant * MyOctant::GetParent(void) { return m_pParent; }
void MyOctant::Release(void) {
	if (m_uLevel == 0) { 
		KillBranches();
	}
	// kill all branches and reset lists/member variables
	m_uChildren = 0;
	m_fSize = 0.0f;
	m_EntityList.clear();
	m_lChild.clear();
}
// The big 3
MyOctant::MyOctant(uint a_nMaxLevel, uint a_nIdealEntityCount) {
	// initialize member variables 
	Init(); 
	m_uOctantCount = 0;
	m_uMaxLevel = a_nMaxLevel;
	m_uIdealEntityCount = a_nIdealEntityCount;
	m_uID = m_uOctantCount;
	m_pRoot = this;
	m_lChild.clear();
	std::vector<vector3> pointList; 
	uint objects = m_pEntityMngr->GetEntityCount();

	// add global min and max of entities to list
	for (uint i = 0; i < objects; i++) {
		MyEntity* entity = m_pEntityMngr->GetEntity(i);
		MyRigidBody* rigidBody = entity->GetRigidBody();
		pointList.push_back(rigidBody->GetMinGlobal());
		pointList.push_back(rigidBody->GetMaxGlobal());
	}

	// create a new rigidbody for the octant out of the mins and maxs of the entities
	MyRigidBody* rigidBody = new MyRigidBody(pointList);
	vector3 halfWidth = rigidBody->GetHalfWidth();
	float max = halfWidth.x;

	// set the maximum length of a side from the new rigidbody
	for (int i = 1; i < 3; i++) {
		if (max < halfWidth[i])
			max = halfWidth[i];
	}

	// set the center of the rigidbody and delete the rigidbody (have all info we need)
	vector3 center = rigidBody->GetCenterLocal();
	pointList.clear();
	SafeDelete(rigidBody);

	// set the member variables using the max and center
	m_fSize = max * 2.0f;
	m_v3Center = center;
	m_v3Min = m_v3Center - (vector3(max));
	m_v3Max = m_v3Center + (vector3(max));

	// increment the octant count (just created a new one) and construct the tree structure with a max size
	m_uOctantCount++;
	ConstructTree(m_uMaxLevel);
}
MyOctant::MyOctant(vector3 a_v3Center, float a_fSize) {
	// initialize member variables
	Init();
	m_v3Center = a_v3Center;
	m_fSize = a_fSize;

	// set member variables using info passed in
	m_v3Min = m_v3Center - (vector3(m_fSize) / 2.0f);
	m_v3Max = m_v3Center + (vector3(m_fSize) / 2.0f);

	// increment count of octants (just created one)
	m_uOctantCount++;
}
MyOctant::MyOctant(MyOctant const& other) {
	// set member variables as other's member variables
	// set vector center, min, and max
	m_v3Center = other.m_v3Center;
	m_v3Min = other.m_v3Min;
	m_v3Max = other.m_v3Max;

	// set size, id, and level
	m_fSize = other.m_fSize;
	m_uID = other.m_uID;
	m_uLevel = other.m_uLevel;

	// set children, parent, root, and child list
	m_uChildren = other.m_uChildren;
	m_pParent = other.m_pParent;
	m_pRoot, other.m_pRoot;
	m_lChild, other.m_lChild;

	// set Mesh manager and entity manager
	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	// set current children as other children
	for (uint i = 0; i < 8; i++) {
		m_pChild[i] = other.m_pChild[i];
	}
}
MyOctant& MyOctant::operator=(MyOctant const& other) {
	// not equal to other, perform operation
	if (this != &other) {
		// release current object, initialize member variables, swap with other octant
		Release();
		Init();
		MyOctant temp(other);
		Swap(temp);
	}
	return *this;
}
MyOctant::~MyOctant() { Release(); };
//Accessors
float MyOctant::GetSize(void) { return m_fSize; }
vector3 MyOctant::GetCenterGlobal(void) { return m_v3Center; }
vector3 MyOctant::GetMinGlobal(void) { return m_v3Min; }
vector3 MyOctant::GetMaxGlobal(void) { return m_v3Max; }
void MyOctant::Display(uint a_nIndex, vector3 a_v3Color) {
	// display the octant if equal to the current index
	if (m_uID == a_nIndex) {
		m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) *
			glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
		return;
	}
	// recursively find the octant to display and display what's underneath as well
	for (uint i = 0; i < m_uChildren; i++) {
		m_pChild[i]->Display(a_nIndex);
	}
}
void MyOctant::Display(vector3 a_v3Color) {
	// recursively display octant volume
	for (uint i = 0; i < m_uChildren; i++) {
		m_pChild[i]->Display(a_v3Color);
	}
	// display with passed in color
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) *
		glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
}
void MyOctant::Subdivide(void) {
	// if reached maximum level, return
	if (m_uLevel >= m_uMaxLevel)
		return;

	// if already subdivided, return
	if (m_uChildren != 0)
		return;

	// create children for octant
	m_uChildren = 8;

	// set half size and full size of new octant
	float sizeH = m_fSize / 4.0f;
	float sideF = sizeH * 2.0f;
	vector3 center;

	// create new octants (subdivide)
	// bottom left back octant
	center = m_v3Center;
	center.x -= sizeH;
	center.y -= sizeH;
	center.z -= sizeH;
	m_pChild[0] = new MyOctant(center, sideF);

	// bottom right back octant
	center.x += sideF;
	m_pChild[1] = new MyOctant(center, sideF);

	// bottom right front octant
	center.z += sideF;
	m_pChild[2] = new MyOctant(center, sideF);

	// bottom left front octant
	center.x -= sideF;
	m_pChild[3] = new MyOctant(center, sideF);

	// top left front octant
	center.y += sideF;
	m_pChild[4] = new MyOctant(center, sideF);

	// top left back octant
	center.z -= sideF;
	m_pChild[5] = new MyOctant(center, sideF);

	// top right back octant
	center.x += sideF;
	m_pChild[6] = new MyOctant(center, sideF);

	// top right front octant
	center.z += sideF;
	m_pChild[7] = new MyOctant(center, sideF);

	// set new root, parent, and level of each new octant
	// if more than ideal entities in an octant, recursively subdivide
	for (uint i = 0; i < 8; i++) {
		m_pChild[i]->m_pRoot = m_pRoot;
		m_pChild[i]->m_pParent = this;
		m_pChild[i]->m_uLevel = m_uLevel + 1;
		if (m_pChild[i]->ContainsMoreThan(m_uIdealEntityCount)) {
			m_pChild[i]->Subdivide();
		}
	}
}
MyOctant * MyOctant::GetChild(uint a_nChild) {
	// get the child from the list of children only if index is valid
	if (a_nChild > 7) return nullptr;
	return m_pChild[a_nChild];
}
bool MyOctant::IsColliding(uint a_uRBIndex) {
	uint objects = m_pEntityMngr->GetEntityCount();
	// if index is not valid, return false
	if (a_uRBIndex >= objects)
		return false;
	
	// get global max and min of entity
	MyEntity* entity = m_pEntityMngr->GetEntity(a_uRBIndex);
	MyRigidBody* rigidBody = entity->GetRigidBody();
	vector3 min = rigidBody->GetMinGlobal();
	vector3 max = rigidBody->GetMaxGlobal();

	// check current max.x/min.x with other max.x/min.x, return false if not within x bounds
	if (m_v3Max.x < min.x)
		return false;
	if (m_v3Min.x > max.x)
		return false;

	// check current max.y/min.y with other max.y/min.y, return false if not within y bounds
	if (m_v3Max.y < min.y)
		return false;
	if (m_v3Min.y > max.y)
		return false;

	// check current max.z/min.z with other max.z/min.z, return false if not within z bounds
	if (m_v3Max.z < min.z)
		return false;
	if (m_v3Min.z > max.z)
		return false;

	// return true, colliding
	return true;
}
bool MyOctant::IsLeaf(void) { return m_uChildren == 0; }
bool MyOctant::ContainsMoreThan(uint a_nEntities) {
	uint count = 0;
	uint objects = m_pEntityMngr->GetEntityCount();
	// checks if objects are colliding with octant, if more than max entities in octant, return true, else, false
	for (uint n = 0; n < objects; n++) {
		if (IsColliding(n))
			count++;
		if (count > a_nEntities)
			return true;
	}
	return false;
}
void MyOctant::KillBranches(void) {
	// recursively kill nodes and set children as 0
	for (uint i = 0; i < m_uChildren; i++) {
		m_pChild[i]->KillBranches();
		delete m_pChild[i];
		m_pChild[i] = nullptr;
	}
	m_uChildren = 0;
}
void MyOctant::DisplayLeafs(vector3 a_v3Color) {
	uint leafs = m_lChild.size();
	// recursively display leafs of octree
	for (uint i = 0; i < leafs; i++) {
		m_lChild[i]->DisplayLeafs(a_v3Color);
	}
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) *
		glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
}
void MyOctant::ClearEntityList(void) {
	// recursively clear entity list of octree
	for (uint i = 0; i < m_uChildren; i++) {
		m_pChild[i]->ClearEntityList();
	}
	m_EntityList.clear();
}
void MyOctant::ConstructTree(uint a_nMaxLevel) {
	// if not the root, dont do this method
	if (m_uLevel != 0)
		return;

	// set octant count and max level count
	m_uMaxLevel = a_nMaxLevel;
	m_uOctantCount = 1;

	// clear the entity list
	m_EntityList.clear();

	// clear the tree
	KillBranches();
	m_lChild.clear();

	// subdivide if more than ideal count
	if (ContainsMoreThan(m_uIdealEntityCount)) {
		Subdivide();
	}

	// add the id of the octant to entities
	AssignIDtoEntity();

	// construct the list of objects
	ConstructList();
}
void MyOctant::AssignIDtoEntity(void) {
	// recursively assign ids to entities
	for (uint i = 0; i < m_uChildren; i++) {
		m_pChild[i]->AssignIDtoEntity();
	}
	// if entity is a leaf, find its objects, add to entity list and add dimension with id
	if (m_uChildren == 0) { 
		uint objects = m_pEntityMngr->GetEntityCount();
		for (uint i = 0; i < objects; i++) {
			if (IsColliding(i)) {
				m_EntityList.push_back(i);
				m_pEntityMngr->AddDimension(i, m_uID);
			}
		}
	}
}
void MyOctant::ConstructList(void) {
	// recursively construct list for each child of octree
	for (uint i = 0; i < m_uChildren; i++) {
		m_pChild[i]->ConstructList();
	}

	// add to root if has entities in list
	if (m_EntityList.size() > 0) {
		m_pRoot->m_lChild.push_back(this);
	}
}
