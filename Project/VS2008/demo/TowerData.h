#pragma once
#include <string.h>

enum ETowerType 
{
	E_TOWER_ATTACK, 
	E_TOWER_BUFF,
};

struct STowerData
{
	//logic attributes
	char TowerName[50];
	int Damage;
	int Range;
	ETowerType TowerType;

	//graphic attributes
	int ModelID;
	
	//sound attributes
	int SoundID;

	STowerData(char* TowerName, int Damage, int Range, ETowerType TowerType, int ModelID, int SoundID)
	{
		strcpy(this->TowerName, TowerName);
		this->Damage = Damage;
		this->Range = Range;
		this->TowerType = TowerType;
		this->ModelID = ModelID;
		this->SoundID = SoundID;
	}
};