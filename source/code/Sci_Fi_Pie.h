#if !defined(SCIFIPIE_H)

#include "Game_Math.h"

#define MAX_CONTROLLERS 5
#define KEYBOARD_CONTROLLER 4

struct GameBackBuffer
{
	void* Memory;
	int Pitch;
	int BytesPerPixel;
	int Width;
	int Height;
	int Size;
};

struct Color
{
	u8 A;
	u8 R;
	u8 G;
	u8 B;
};

struct GameButton
{	
	bool EndedDown;
};

struct GameMouse
{
	u16 PosX;
	u16 PosY;

	GameButton LMB;
	GameButton RMB;
};

struct GameController
{
	GameButton Up;
	GameButton Down;
	GameButton Left;
	GameButton Right;
	GameButton Action1;
	GameButton Action2;
	GameButton Action3;
	GameButton Action4;

	f32 StickX;
	f32 StickY;

	bool isAnalogue;
};

struct GameInput
{
	GameController GameControllers[MAX_CONTROLLERS]; 
	GameMouse Mouse;
};

struct GameMemory
{
	bool IsInitialized;

	u64 MainMemorySize;
	u64 TempMemorySize;

	void* MainMemory;
	void* TempMemory;
};

enum EntityType
{
	EntityType_Null,

	EntityType_Wall,
	EntityType_Player,
	EntityType_Projectile,
};

struct Entity
{
	u32 Index;

	EntityType Type;

	V2 Direction;
	V2 Position;
	V2 Velocity;
	V2 Acceleration;

	f32 Damping;
	f32 AccelerationMag;

	f32 Bounciness;

	f32 Width;
	f32 Height;
	f32 CollisionRadius;
};

struct GameState
{
	Entity Player1;
	Entity Projectile;
	Entity Walls[256];
	f32 dT;
};



//Services Platform -> Game
//IO - Memory?

//Services Game -> Platform
//input - soundbuffer - videobuffer
internal void GameLoop(GameBackBuffer* buffer, GameInput* gameInput, GameMemory* gameMemory);

#define SCIFIPIE_H


#endif 