#include "MapObject.h"
#include "ObjectManager.h"

CMapObject::CMapObject(SMapData *data)
{
	this->ObjectType = EGameObject::E_OBJ_MAP;
	this->data = data;
	ObjectMap = new int[this->data->Height * this->data->Width];
	memset(ObjectMap, 0, sizeof(int) * this->data->Height * this->data->Width);
	DirectionMap = new int[this->data->Height * this->data->Width];
	memset(DirectionMap, 0, sizeof(int) * this->data->Height * this->data->Width);
	RemainingLife = data->GivenLife;
	Money = data->GivenMoney;
	memset(Enemy, 0, sizeof(Enemy));
	isSpawnTime = false;
	NumberOfEnemyInMap = 0;
	CalculateEnemyPath(ObjectMap, DirectionMap);
	status = ESTATUS_PLAY;

	//CImageManager::GetInstance()->AddImage<CFileWin32Driver>("..\\..\\resource\\grid_cell.tga",true);
	//MapGridTexture = CImageManager::GetInstance()->Get("..\\..\\resource\\grid_cell.tga");
	Init();
}

CMapObject::~CMapObject(void)
{
	SAFE_DEL_ARRAY(ObjectMap);
}

void CMapObject::Init()
{

	sceneNode = smgr->addTerrainSceneNode(
                "./resource/terrain-heightmap.bmp",
                0,											// parent node
                2,											// node id
				irr::core::vector3df(-120.f+(float)data->Width/2, 0.f, -120.f+(float)data->Height/2),         // position
                irr::core::vector3df(0.f, 0.f, 0.f),         // rotation
                irr::core::vector3df(1.f, 1.f, 1.f),		  // scale
                irr::video::SColor ( 255, 255, 255, 255 ),   // vertexColor
                5,											 // maxLOD
                irr::scene::ETPS_17,                         // patchSize
                4											 // smoothFactor
                );

        //sceneNode->setMaterialFlag(irr::video::EMF_LIGHTING, false);

        sceneNode->setMaterialTexture(0,
                        driver->getTexture("./resource/terrain-texture.jpg"));
        sceneNode->setMaterialTexture(1,
                        driver->getTexture("./resource/detailmap3.jpg"));
        
        sceneNode->setMaterialType(irr::video::EMT_DETAIL_MAP);

        sceneNode->scaleTexture(1.0f, 20.0f);

		irr::scene::ITriangleSelector* selector = smgr->createTerrainTriangleSelector(sceneNode, 0);
		sceneNode->setTriangleSelector(selector);
		selector->drop();

		skydome =smgr->addSkyDomeSceneNode(driver->getTexture("./resource/skydome.jpg"),16,8,0.95f,2.0f);
		irr::scene::IMesh* m = sceneNode->getMesh();
		int a=1;
}

void CMapObject::Update(int delta_time)
{
	if (status == ESTATUS_PLAY)
	{
		static int CurrentWave = 0;
		static int NextSpawnTime;
		static int EnemyIndexInWave;
		if ((!isSpawnTime) && (NumberOfEnemyInMap==0))
		{
			if (CObjectManager::CurrentObjectManager->BulletList.GetCount()==0)
			{
				if (CurrentWave < this->data->NumberOfWaves)
				{
					CObjectManager::CurrentObjectManager->ClearEnemy();
					isSpawnTime = true;
					EnemyIndexInWave = 0;
					NextSpawnTime = this->data->Wave[CurrentWave]->SpawnTime[EnemyIndexInWave];
				}
				else Win();
			}
		}

		if (isSpawnTime)
		{
			if (NextSpawnTime>delta_time)
			{
				NextSpawnTime -= delta_time;
			}
			else
			{
				CEnemyObject* Enemy = new CEnemyObject(this->data->Wave[CurrentWave]->Enemy[EnemyIndexInWave], this->data->SourcePosition);
				Enemy->Spawn();
				CObjectManager::CurrentObjectManager->AddObject(Enemy);
				EnemyIndexInWave++;
				if (EnemyIndexInWave >= this->data->Wave[CurrentWave]->Quantity)
				{
					NextSpawnTime = 0;
					isSpawnTime = false;				
					CurrentWave++;
				}
				else
				{
					NextSpawnTime = this->data->Wave[CurrentWave]->SpawnTime[EnemyIndexInWave] - (delta_time-NextSpawnTime);
				}
			}
		}
	} ///if status == ESTATUS_PLAY
}

void CMapObject::Render()
{
}

void CMapObject::Destroy()
{
	sceneNode->remove();
	skydome->remove();
	CObjectManager::CurrentObjectManager->SetMapObject(NULL);
	delete this;
}

bool CMapObject::CalculateEnemyPath(int* ObjectMap, int* DirectionMap)
{
	int* TraceMap = new int[data->Width*data->Height];
	memset(TraceMap, 0, (data->Width)*(data->Height)*sizeof(int));
	TraceMap[data->DestinationPosition.y*data->Width+data->DestinationPosition.x] = 1;

	
	

	LogicPosition* Queue = new LogicPosition[(data->Width)*(data->Height)];	//allocate queue
	int cur = 0;	//current index of queue
	int last = 0;	//last index of queue
	Queue[0] = data->DestinationPosition;	//add destination to queue
	while (cur<=last)
	{
		LogicPosition curpos = Queue[cur];
		for (int i=1; i<=4; i++)
		{
			LogicPosition nextpos = curpos + OppositeMove[i];
			if (	(nextpos.x>=0) && (nextpos.x<data->Width) && (nextpos.y>=0) && (nextpos.y<data->Height) 
					&&(ObjectMap[nextpos.y*data->Width+nextpos.x]!=E_OBJ_TOWER)
					&& (ObjectMap[nextpos.y*data->Width+nextpos.x]!=E_OBJ_OBSTACLE)
					&& (TraceMap[nextpos.y*data->Width+nextpos.x]== 0))
			{
				TraceMap[nextpos.y*data->Width+nextpos.x] = TraceMap[curpos.y*data->Width+curpos.x] + 1;
				DirectionMap[nextpos.y*data->Width+nextpos.x] = i;
				Queue[++last] = nextpos;
			}
		}
		cur++;
	}

	bool isValid = true;

	for (int i=0; i<data->Width; i++)
	{
		for (int j=0; j<data->Height; j++)
		{
			LogicPosition curpos(i,j);
			if (	(curpos.x>=0) && (curpos.x<data->Width) && (curpos.y>=0) && (curpos.y<data->Height) 
					&&(ObjectMap[curpos.y*data->Width+curpos.x]!=E_OBJ_TOWER)
					&& (ObjectMap[curpos.y*data->Width+curpos.x]!=E_OBJ_OBSTACLE)
					&& (TraceMap[curpos.y*data->Width+curpos.x]==0))
			{
				isValid = false;
				break;
			}
		}
		if (!isValid) break;
	}
	SAFE_DEL(TraceMap);
	return isValid;
}


void CMapObject::BuildTower(STowerData* data, LogicPosition position)
{
	CTowerObject* tower = new CTowerObject(data);
	int* NewObjectMap = new int[this->data->Width* this->data->Height];
	memcpy(NewObjectMap, this->ObjectMap, sizeof(int)*this->data->Width* this->data->Height);

	for (int i=0;i<2;i++)
		for (int j=0;j<2;j++)
			if (NewObjectMap[(position.y+i) *this->data->Width + (position.x+j)] == E_OBJ_NONE)
			{
				NewObjectMap[(position.y+i) *this->data->Width + (position.x+j)] = E_OBJ_TOWER;
			}
			else
			{
				delete NewObjectMap;
				return;
			}

	/*NewObjectMap[position.y *this->data->Width + position.x] = E_OBJ_TOWER;
	NewObjectMap[(position.y+1) *this->data->Width + position.x] = E_OBJ_TOWER;
	NewObjectMap[position.y *this->data->Width + (position.x+1)] = E_OBJ_TOWER;
	NewObjectMap[(position.y+1) *this->data->Width + (position.x+1)] = E_OBJ_TOWER;*/
	int* NewDirectionMap = new int[this->data->Width* this->data->Height];
	memset(NewDirectionMap, 0, sizeof(int)*this->data->Width* this->data->Height);
	if (CalculateEnemyPath(NewObjectMap, NewDirectionMap))
	{
		tower->logicposition = position;
		tower->position = Position(position.x+1, position.y+1, 0.8);
		tower->sceneNode->setPosition(irr::core::vector3df(tower->position.x, tower->position.z, tower->position.y));
		CObjectManager::CurrentObjectManager->AddObject(tower);
		SAFE_DEL(ObjectMap);
		this->ObjectMap = NewObjectMap;
		SAFE_DEL(DirectionMap);
		this->DirectionMap = NewDirectionMap;
	}	
}

void CMapObject::AddObstacle(SObstacleData* data, LogicPosition position, int size)
{
	CObstacleObject* obstacle = new CObstacleObject(data,size);
	int* NewObjectMap = new int[this->data->Width* this->data->Height];
	memcpy(NewObjectMap, this->ObjectMap, sizeof(int)*this->data->Width* this->data->Height);

	for (int i=0;i<obstacle->size;i++)
		for (int j=0;j<obstacle->size;j++)
			if (NewObjectMap[(position.y+i) *this->data->Width + (position.x+j)] == E_OBJ_NONE)
			{
				NewObjectMap[(position.y+i) *this->data->Width + (position.x+j)] = E_OBJ_OBSTACLE;
			}
			else
			{
				delete NewObjectMap;
				return;
			}

	int* NewDirectionMap = new int[this->data->Width* this->data->Height];
	memset(NewDirectionMap, 0, sizeof(int)*this->data->Width* this->data->Height);
	if (CalculateEnemyPath(NewObjectMap, NewDirectionMap))
	{
		obstacle->logicposition = position;
		obstacle->position = Position(position.x+(float)size/2.0, position.y+(float)size/2.0, 0);
		obstacle->sceneNode->setPosition(irr::core::vector3df(obstacle->position.x, obstacle->position.z, obstacle->position.y));
		CObjectManager::CurrentObjectManager->AddObject(obstacle);
		SAFE_DEL(ObjectMap);
		this->ObjectMap = NewObjectMap;
		SAFE_DEL(DirectionMap);
		this->DirectionMap = NewDirectionMap;
	}	
}

void CMapObject::Win()
{
	status = ESTATUS_WIN;
	printf("WIN\n");
}

void CMapObject::Lose()
{
	status = ESTATUS_LOSE;
	printf("LOSE\n");
}