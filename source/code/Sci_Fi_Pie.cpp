#include "Sci_Fi_Pie.h"
#include "Game_Math.h"
#include "Game_Intrinsics.h"
#include "Collision.h"
#include "Tile.h"

#define StickDeadZone 8000
#define StickMaxValue 32767

static Tile TileMap1[9][16] = 
{
	{{1, false, {0,0}}, {1, false, {1,0}}, {1, false, {2,0}}, {1, false ,{3,0}}, {1, false, {4,0}}, {1, false, {5,0}}, {1, false, {6,0}}, {1, false, {7,0}}, {1, false, {8,0}}, {1, false, {9,0}}, {1, false, {10,0}}, {1, false, {11,0}}, {1, false, {12,0}}, {1, false, {13,0}}, {1, false, {14,0}}, {1, false, {15,0}}},
	{{1, false, {0,1}}, {0, false, {1,1}}, {0, false, {2,1}}, {1, false ,{3,1}}, {0, false, {4,1}}, {0, false, {5,1}}, {0, false, {6,1}}, {0, false, {7,1}}, {0, false, {8,1}}, {0, false, {9,1}}, {0, false, {10,1}}, {0, false, {11,1}}, {0, false, {12,1}}, {0, false, {13,1}}, {0, false, {14,1}}, {1, false, {15,1}}},
	{{1, false, {0,2}}, {0, false, {1,2}}, {0, false, {2,2}}, {0, false, {3,2}}, {0, false, {4,2}}, {0, false, {5,2}}, {0, false, {6,2}}, {0, false, {7,2}}, {0, false, {8,2}}, {0, false, {9,2}}, {0, false, {10,2}}, {0, false, {11,2}}, {1, false, {12,2}}, {0, false, {13,2}}, {0, false, {14,2}}, {1, false, {15,2}}},
	{{1, false, {0,3}}, {0, false, {1,3}}, {1, false, {2,3}}, {0, false, {3,3}}, {0, false, {4,3}}, {0, false, {5,3}}, {0, false, {6,3}}, {1, false, {7,3}}, {0, false, {8,3}}, {0, false, {9,3}}, {1, false, {10,3}}, {0, false, {11,3}}, {1, false, {12,3}}, {0, false, {13,3}}, {0, false, {14,3}}, {1, false, {15,3}}},
	{{1, false, {0,4}}, {0, false, {1,4}}, {1, false, {2,4}}, {1, false, {3,4}}, {0, false, {4,4}}, {0, false, {5,4}}, {1, false, {6,4}}, {1, false, {7,4}}, {1, false, {8,4}}, {0, false, {9,4}}, {0, false, {10,4}}, {0, false, {11,4}}, {1, false, {12,4}}, {0, false, {13,4}}, {1, false, {14,4}}, {1, false, {15,4}}},
	{{1, false, {0,5}}, {0, false, {1,5}}, {1, false, {2,5}}, {0, false, {3,5}}, {0, false, {4,5}}, {0, false, {5,5}}, {0, false, {6,5}}, {1, false, {7,5}}, {0, false, {8,5}}, {0, false, {9,5}}, {1, false, {10,5}}, {0, false, {11,5}}, {1, false, {12,5}}, {0, false, {13,5}}, {0, false, {14,5}}, {1, false, {15,5}}},
	{{1, false, {0,6}}, {0, false, {1,6}}, {0, false, {2,6}}, {0, false, {3,6}}, {0, false, {4,6}}, {1, false, {5,6}}, {0, false, {6,6}}, {0, false, {7,6}}, {0, false, {8,6}}, {0, false, {9,6}}, {0, false, {10,6}}, {0, false, {11,6}}, {1, false, {12,6}}, {0, false, {13,6}}, {0, false, {14,6}}, {1, false, {15,6}}},
	{{1, false, {0,7}}, {0, false, {1,7}}, {0, false, {2,7}}, {1, false, {3,7}}, {0, false, {4,7}}, {0, false, {5,7}}, {0, false, {6,7}}, {0, false, {7,7}}, {0, false, {8,7}}, {0, false, {9,7}}, {0, false, {10,7}}, {0, false, {11,7}}, {0, false, {12,7}}, {0, false, {13,7}}, {0, false, {14,7}}, {1, false, {15,7}}},
	{{1, false, {0,8}}, {1, false, {1,8}}, {1, false, {2,8}}, {1, false, {3,8}}, {1, false, {4,8}}, {1, false, {5,8}}, {1, false, {6,8}}, {1, false, {7,8}}, {1, false, {8,8}}, {1, false, {9,8}}, {1, false, {10,8}}, {1, false, {11,8}}, {1, false, {12,8}}, {1, false, {13,8}}, {1, false, {14,8}}, {1, false, {15,8}}},
};

internal Color DEBUGCreateColor(u8 alpha, u8 red, u8 green, u8 blue)
{
	Color color = {};
	color.R = red;
	color.G = green;
	color.B = blue;

	return color;
}

internal void DebugDrawRect(GameBackBuffer* buffer, Color color,
							V2 center,
							s32 width, s32 height)
{
	if(width > buffer->Width)
	{
		width = buffer->Width;
	}
	else if(height > buffer->Height)
	{
		height = buffer->Height;
	}

	s32 x1 = (center.X - (0.5f * width));
	s32 x2 = (center.X + (0.5f * width));

	s32 y1 = buffer->Height - center.Y - (0.5f * height);
	s32 y2 = buffer->Height - center.Y + (0.5f * height);

	s32 yOffset = 0;
	if(y1 < 0)
	{
		yOffset = -y1;
		y1 = 0;
	}
	else if(y2 > buffer->Height)
	{
		y2 = buffer->Height;
	}

	s32 xOffset = 0;
	if(x1 < 0)
	{
		xOffset = -x1;
		x1 = 0;
	}
	else if(x2 > buffer->Width)
	{
		x2 = buffer->Width;
	}

	u8* bufferRow = (u8*)buffer->Memory + (x1 * buffer->BytesPerPixel) + (y1 * buffer->Pitch); 

	for(int y = y1; y < y2; y++)
	{
		u32* bufferPixel = (u32*)bufferRow;

		for(int x = x1; x < x2; x++)
		{
			*bufferPixel = ((color.A << 24) | (color.R << 16) | (color.G << 8) | (color.B <<0));					
	
			bufferPixel++;
		}
		bufferRow += buffer->Pitch;
	}
}

internal void DebugPutPixel(int x, int y, Color* color, GameBackBuffer* buffer)
{
	u32* bufferPixel = (u32*)((u8*)buffer->Memory + (x * buffer->BytesPerPixel) + (y * buffer->Pitch));
	*bufferPixel = ((color->A << 24) | (color->R << 16) | (color->G << 8) | (color->B <<0));					
}

internal void DebugDrawCircle(GameBackBuffer* buffer, Color* color,
								int x0, int  y0, int radius)
{
    int x = radius-1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1);

    y0 = buffer->Height - y0;

    while (x >= y)
    {
          	
        DebugPutPixel(x0 + x, y0 + y, color, buffer);
        DebugPutPixel(x0 + y, y0 + x, color, buffer);
        DebugPutPixel(x0 - y, y0 + x, color, buffer);
        DebugPutPixel(x0 - x, y0 + y, color, buffer);
        DebugPutPixel(x0 - x, y0 - y, color, buffer);
        DebugPutPixel(x0 - y, y0 - x, color, buffer);
        DebugPutPixel(x0 + y, y0 - x, color, buffer);
        DebugPutPixel(x0 + x, y0 - y, color, buffer);

        if (err <= 0)
        {
            y++;
            err += dy;
            dy += 2;
        }
        if (err > 0)
        {
            x--;
            dx += 2;
            err += (-radius << 1) + dx;
        }
    }
}

internal void UpdatePlayer(Entity* player, GameState* gameState)
{
	MoveEntity(player, gameState);
}

internal void UpdateProjectile(Entity* projectile, GameState* gameState)
{
	MoveEntity(projectile, gameState);
}

internal void GameLoop(GameBackBuffer* buffer, GameInput* gameInput, GameMemory* gameMemory)
{
	GameState* gameState = (GameState*)gameMemory->MainMemory;

	TileMap TileMap = {};
	TileMap.CountX = 16;
	TileMap.CountY = 9;
	TileMap.Tiles = *TileMap1;
	TileMap.MetersToPixels = TileMap.TilePixelLength / TileMap.TileMeterLength;
	TileMap.TileMeterLength = TileMap.TileMeterLength * TileMap.MetersToPixels;

	Color playerColor = DEBUGCreateColor(0, 32, 32, 255);
	Color playerColColor = DEBUGCreateColor(0, 255, 0, 0);

	if(!gameMemory->IsInitialized)
	{
		gameState->dT = 33.33f / 1000.0f;

		gameState->Entities[0].Index = 0;	
		gameState->Entities[0].Position = TileMap.MetersToPixels* V2{2.5f, 2.5f};
		gameState->Entities[0].Speed = 120.0f * TileMap.MetersToPixels;
		gameState->Entities[0].Width = 1.0f * TileMap.MetersToPixels;
		gameState->Entities[0].Height = 1.2f * TileMap.MetersToPixels;
		gameState->Entities[0].CollisionRadius = 0.5f * TileMap.MetersToPixels;
		gameState->Entities[0].Damping = 0.2f;
		gameState->Entities[0].Bounciness = 1.0f;
		gameState->Entities[0].Type = EntityType_Player;
		gameState->Entities[0].Collider = ColliderType_Circle;

		gameState->Entities[1].Index = 1;	
		gameState->Entities[1].Speed = 180.0f * TileMap.MetersToPixels;
		gameState->Entities[1].Width = 0.8f * TileMap.MetersToPixels;
		gameState->Entities[1].Height = 0.8f * TileMap.MetersToPixels;
		gameState->Entities[1].CollisionRadius = gameState->Entities[1].Width / 2;
		gameState->Entities[1].Damping = 0.2f;
		gameState->Entities[1].Bounciness = 2.0f;
		gameState->Entities[1].Type = EntityType_Projectile;
		gameState->Entities[1].Collider = ColliderType_Circle;
		gameState->Entities[1].Direction = V2{0.7f, 0.7f};
		gameState->Entities[1].Acceleration = gameState->Entities[1].Direction * gameState->Entities[1].Speed;
		gameState->Entities[1].IsActive = true;
		gameState->Entities[1].Position = GetTileCenter(1, 2, &TileMap);

		u32 index = 2;

		//Load entities ( just walls right now )
		//TODO(matt): Tidy this up.... Make it use the flat TileMap.Tiles pointer
		for(s32 y = 0; y < TileMap.CountY; y++)
		{
			for(s32 x = 0; x < TileMap.CountX; x++)
			{
				if(TileMap1[y][x].TileType == 1 && TileMap1[y][x].IsLoaded == false)
				{
					TileMap1[y][x].IsLoaded = true;

					Entity* wall = &gameState->Entities[index];
					wall->Position = GetTileCenter(x, y, &TileMap);
					wall->Width = TileMap.TileMeterLength;
					wall->Height = TileMap.TileMeterLength;
					wall->Type = EntityType_Wall;
					wall->Index = index;
					wall->Collider = ColliderType_Box;
					wall->CollisionWidth = TileMap.TileMeterLength;
					wall->CollisionHeight = TileMap.TileMeterLength;

					u32 wallCountY = 1;
					u32 wallCountX = 1;

					bool HasVerticalMapped = false;

					while(TileMap1[y + wallCountY][x].TileType == 1 &&
						 (y + wallCountY < TileMap.CountY))
					{
						TileMap1[y + wallCountY][x].IsLoaded = true;
						if(HasVerticalMapped == false)
						{
							HasVerticalMapped = true;
						}


                        wall->Position.Y += TileMap.TileMeterLength / 2;
						wall->CollisionHeight += TileMap.TileMeterLength;
						wall->Height += TileMap.TileMeterLength;
						wallCountY++;
					}

					if(TileMap1[y][x + wallCountX].TileType == 1 &&
				     	(x + wallCountX < TileMap.CountX))				
					{
						if(HasVerticalMapped == true)
						{
							index++;
						}
						wall = &gameState->Entities[index];
						wall->Position = GetTileCenter(x, y, &TileMap);
						wall->Width = TileMap.TileMeterLength;
						wall->Height = TileMap.TileMeterLength;
						wall->Type = EntityType_Wall;
						wall->Index = index;
						wall->Collider = ColliderType_Box;
						wall->CollisionWidth = TileMap.TileMeterLength;
						wall->CollisionHeight = TileMap.TileMeterLength;

						while(TileMap1[y][x + wallCountX].TileType == 1 &&
						     (x + wallCountX < TileMap.CountX))
						{
							TileMap1[y][x  + wallCountX].IsLoaded = true;

	                        wall->Position.X += TileMap.TileMeterLength / 2;
							wall->CollisionWidth += TileMap.TileMeterLength;
							wall->Width += TileMap.TileMeterLength;
							wallCountX++;
						}					
					}

					index++;
				}
				else if(TileMap1[y][x].TileType == 0 && TileMap1[y][x].IsLoaded == false)
				{
					TileMap1[y][x].IsLoaded = true;

					Entity* ground = &gameState->Entities[index];
					ground->Position = GetTileCenter(x, y, &TileMap);
					ground->Width = TileMap.TileMeterLength;
					ground->Height = TileMap.TileMeterLength;
					ground->Type = EntityType_Ground;
					ground->Index = index;
					ground->Collider = ColliderType_Null;
					ground->CollisionWidth = 0.0f;
					ground->CollisionHeight = 0.0f;

					index++;
				}
			}
		}

		gameMemory->IsInitialized = true;
	}

	//TODO(matt): Get Input from all controllers
	gameState->Entities[0].Direction = gameState->Entities[0].Direction;
	gameState->Entities[0].Direction = V2{ 0,0 };

	//TODO(matt): Make controllers specific to player entities 
	for(s32 controllerIndex = 0; controllerIndex < MAX_CONTROLLERS; controllerIndex++)
	{
		if(gameInput->GameControllers[controllerIndex].isAnalogue)
		{
			f32 sMag = Sqrtf32(Square(gameInput->GameControllers[controllerIndex].StickX) +
							   Square(gameInput->GameControllers[controllerIndex].StickY));

			if(sMag > StickMaxValue)
			{
				sMag = StickMaxValue;
			}

			if(sMag > StickDeadZone)
			{
				f32 nX = gameInput->GameControllers[controllerIndex].StickX / sMag;
				f32 nY = gameInput->GameControllers[controllerIndex].StickY / sMag;

				gameState->Entities[0].Direction = V2{nX, nY};
				gameState->Entities[0].Facing = V2{nX, nY};
			}
		}
		else
		{
			if(gameInput->GameControllers[controllerIndex].Up.EndedDown)
			{
				gameState->Entities[0].Direction.Y = 1.0f;
			}
			if(gameInput->GameControllers[controllerIndex].Down.EndedDown)
			{
				gameState->Entities[0].Direction.Y = -1.0f;
			}
			if(gameInput->GameControllers[controllerIndex].Left.EndedDown)
			{
				gameState->Entities[0].Direction.X = -1.0f;
			}
			if(gameInput->GameControllers[controllerIndex].Right.EndedDown)
			{
				gameState->Entities[0].Direction.X = 1.0f;
			}
			if((gameState->Entities[0].Direction.Y != 0.0f) && (gameState->Entities[0].Direction.X != 0.0f))
			{
				gameState->Entities[0].Direction *= 0.7071067811865475f;
			}
		}

		if(gameInput->GameControllers[controllerIndex].Action1.EndedDown)
		{
			gameState->Entities[1].IsActive = true;
			if(gameState->Entities[1].Direction.X == 0 && gameState->Entities[1].Direction.Y == 0)
			{
				gameState->Entities[1].Direction = gameState->Entities[0].Facing;				
			}
			else
			{
				gameState->Entities[1].Direction = gameState->Entities[0].Direction;				
			}
			gameState->Entities[1].Position = gameState->Entities[0].Position + (gameState->Entities[1].Direction	* gameState->Entities[0].CollisionRadius * 2);
			gameState->Entities[1].Acceleration = gameState->Entities[1].Direction * gameState->Entities[1].Speed;
		}

		gameState->Entities[0].Acceleration = gameState->Entities[0].Speed * gameState->Entities[0].Direction;
	}

	UpdatePlayer(&gameState->Entities[0], gameState);
	UpdateProjectile(&gameState->Entities[1], gameState);
	
	// Draw Tile map	
	// Tile* baseTile = TileMap.Tiles;
	// u32 tileCount = TileMap.CountX * TileMap.CountY;
	// u32 tileIndex = 0;

	// for(tileIndex; tileIndex < tileCount; tileIndex++)
	// {
	// 	if(baseTile->TileType == 0)
	// 	{
	// 		DebugDrawRect(buffer, 
	// 					  DEBUGCreateColor(0, 240, 240, 240), 
	// 					  GetTileCenter(baseTile->TilePosition.X, baseTile->TilePosition.Y, &TileMap),
	// 					  TileMap.TileMeterLength, 
	// 					  TileMap.TileMeterLength);
	// 	}	
 //        baseTile++;
	// }

	// Draw entities
	//TODO(matt)do we need to draw static entities every frame? does it matter?
	for(u32 entityIndex = 0; entityIndex < ArrayCount(gameState->Entities); entityIndex++)
	{
		Entity* entity = &gameState->Entities[entityIndex];

		if(entity->Type == EntityType_Wall)
		{
			DebugDrawRect(buffer,  DEBUGCreateColor(0, 60, 60, 60), entity->Position, entity->Width, entity->Height);
		}
		else if(entity->Type == EntityType_Ground)
		{
			DebugDrawRect(buffer,  DEBUGCreateColor(0, 240, 240, 240), entity->Position, entity->Width, entity->Height);
		}
	}

	DebugDrawCircle(buffer, &playerColor, gameState->Entities[0].Position.X, gameState->Entities[0].Position.Y, gameState->Entities[0].CollisionRadius);		

	if(gameState->Entities[1].IsActive == true)
	{
		DebugDrawCircle(buffer, &playerColColor, gameState->Entities[1].Position.X, gameState->Entities[1].Position.Y, gameState->Entities[1].CollisionRadius);		
	}
}



