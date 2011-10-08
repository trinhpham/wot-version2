#pragma once
#include "gameobject.h"


class CBulletObject :
	public CGameObject
{
public:
	CBulletObject(SBulletData* data, Position position);
	~CBulletObject(void);

	void Init();
	void Update();
	void Render();
	void Destroy();

	Position position;
	SBulletData *data;

};
