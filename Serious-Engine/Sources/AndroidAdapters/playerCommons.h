#ifndef PLAYERCONTROLS_H
#define PLAYERCONTROLS_H

#include <Engine/StdH.h>
#include <Engine/Math/Vector.h>

#define MAX_WEAPONS 30

struct PlayerControls {
	FLOAT3D aRotation;
	FLOAT3D aViewRotation;
	FLOAT3D vTranslation;

	BOOL bMoveForward;
	BOOL bMoveBackward;
	BOOL bMoveLeft;
	BOOL bMoveRight;
	BOOL bMoveUp;
	BOOL bMoveDown;

	BOOL bTurnLeft;
	BOOL bTurnRight;
	BOOL bTurnUp;
	BOOL bTurnDown;
	BOOL bTurnBankingLeft;
	BOOL bTurnBankingRight;
	BOOL bCenterView;

	BOOL bLookLeft;
	BOOL bLookRight;
	BOOL bLookUp;
	BOOL bLookDown;
	BOOL bLookBankingLeft;
	BOOL bLookBankingRight;

	BOOL bSelectWeapon[MAX_WEAPONS+1];
	BOOL bWeaponNext;
	BOOL bWeaponPrev;
	BOOL bWeaponFlip;

	BOOL bWalk;
	BOOL bStrafe;
	BOOL bFire;
	BOOL bReload;
	BOOL bUse;
	BOOL bShowTabInfo;
	BOOL bDropMoney;
	BOOL bComputer;
	BOOL bUseOrComputer;
	BOOL bUseOrComputerLast;  // for internal use
	BOOL b3rdPersonView;

	BOOL bSniperZoomIn;
	BOOL bSniperZoomOut;
	BOOL bFireBomb;

	BOOL bBack;
	BOOL bStart;

	float axisValue[10];
	float shiftAxisValue[10];
};

#endif // PLAYERCONTROLS_H
