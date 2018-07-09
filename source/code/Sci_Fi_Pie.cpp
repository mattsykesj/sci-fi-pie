#include "Sci_Fi_Pie.h"
#include "Game_Math.h"
#include "Game_Intrinsics.h"

#define Minimum(A, B) ((A < B) ? (A) : (B))
#define Maximum(A, B) ((A > B) ? (A) : (B))
#define ArrayCount( array ) ( sizeof( array ) / sizeof( array[0] ))


#define StickDeadZone 8000
#define StickMaxValue 32767

struct Tile
{
	u32 TileType; 
	bool HasGeometryLoaded;
};

struct World
{
	f32 TilePixelLength = 60.0f;
	f32 TileMeterLength = 1.35f;
	f32 MetersToPixels;
	u32 CountX;
	u32 CountY;

	Tile* Tiles;
};

struct TilePosition
{
	u32 X;
	u32 Y;
};

static Tile TileMap1[9][16] = 
{
	{{1, false}, {1, false}, {1, false}, {1, false}, {1, false}, {1, false}, {1, false}, {1, false}, {1, false}, {1, false}, {1, false}, {1, false}, {1, false}, {1, false}, {1, false}, {1, false}},
	{{1, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {1, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {1, false}},
	{{1, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {1, false}},
	{{1, false}, {0, false}, {0, false}, {1, false}, {0, false}, {0, false}, {0, false}, {1, false}, {0, false}, {0, false}, {0, false}, {1, false}, {0, false}, {0, false}, {0, false}, {1, false}},
	{{1, false}, {0, false}, {0, false}, {1, false}, {0, false}, {1, false}, {0, false}, {1, false}, {0, false}, {1, false}, {0, false}, {1, false}, {0, false}, {1, false}, {0, false}, {1, false}},
	{{1, false}, {0, false}, {0, false}, {1, false}, {0, false}, {0, false}, {0, false}, {1, false}, {0, false}, {0, false}, {0, false}, {1, false}, {0, false}, {0, false}, {0, false}, {1, false}},
	{{1, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {1, false}},
	{{1, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {0, false}, {1, false}},
	{{1, false}, {1, false}, {1, false}, {1, false}, {1, false}, {1, false}, {1, false}, {1, false}, {1, false}, {1, false}, {1, false}, {1, false}, {1, false}, {1, false}, {1, false}, {1, false}},
};

internal Color CreateColor(u8 alpha, u8 red, u8 green, u8 blue)
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

internal bool TestEdge(f32 entityDeltaX, f32 entityDeltaY, 
						f32 entityRelativePositionX, f32 entityRelativePositionY, 
						f32 xCoord, f32 minY, f32 maxY, 
						f32* tMin)
{
	bool collided = false;

	if(entityDeltaX != 0)	
	{
		f32 tWall = (xCoord - entityRelativePositionX) / entityDeltaX;

		if(tWall > 0 && tWall < *tMin)
		{
			f32 collisionPosY = entityRelativePositionY + (tWall * entityDeltaY);

			if(*tMin > tWall)
			{
				if(collisionPosY <= maxY && collisionPosY >= minY)
				{
					*tMin = tWall;
					collided = true;
				}							
			}
		}
	}

	return collided;	
}

internal bool TestCorner(V2 entityRelativePos, V2 newEntityRelativePos, V2* entityDelta, f32 radius,
						 V2* r, f32* tMin, f32 minX, f32 maxX, f32 minY, f32 maxY, f32 h, f32 k)
{
	bool collided = false;

	f32 X1 = entityRelativePos.X; 
	f32 X2 = newEntityRelativePos.X;
	f32 Y1 = entityRelativePos.Y; 					
	f32 Y2 = newEntityRelativePos.Y;

	f32 H =  h;
	f32 K =  k;	

	f32 R = radius;

	f32 A = (Square(X2 - X1) + Square(Y2 - Y1));
	f32 B = (2 * (X2 - X1) * (X1 - H)) + (2 * (Y2 - Y1) * (Y1 - K)); 					
	f32 C = Square(X1 - H) + Square(Y1 - K) - Square(R);

	if ((Square(B) - (4 * A * C)) > 0)
	{
		f32 T = ((2 * C) / (-B + Sqrtf32(Square(B) - 4 * A * C)));

		if (T > 0 && T < *tMin)
		{
			f32 collisionPosX = entityRelativePos.X + (T * entityDelta->X);
			f32 collisionPosY = entityRelativePos.Y + (T * entityDelta->Y);
			
			if(collisionPosY < maxY && collisionPosY > minY &&  
				collisionPosX < maxX && collisionPosX > minX)
			{
				*tMin = T;
				collided = true;

				f32 relativeColX = H - collisionPosX;
				f32 relativeColY = K - collisionPosY;

				f32 rX = relativeColX / radius;
				f32 rY = relativeColY / radius;

				*r = {rX, rY};
			}							
		}
	}

	return collided;
}

internal V2 GetTileCenter(u32 x, u32 y, World* world)
{
	V2 result = {};

	result.X = (world->TileMeterLength * x) + (0.5f * world->TileMeterLength);
	result.Y = (world->TileMeterLength * y) + (0.5f * world->TileMeterLength);

	return result;
}

internal TilePosition GetTilePosition(V2 pos, World* world)
{
	TilePosition result = {};
	result.X = Truncatef32(pos.X / world->TileMeterLength);
	result.Y = Truncatef32(pos.Y / world->TileMeterLength);
	return result;
}

internal bool ShouldCollide(Entity* a, Entity* b)
{
    bool result = false;

    if(a != b)
    {
    	if(a->Type == EntityType_Player && b->Type == EntityType_Wall ||
    		b->Type == EntityType_Player && a->Type == EntityType_Wall)
    	{
	    	result = true;
    	}
    	if(a->Type == EntityType_Player && b->Type == EntityType_Projectile ||
    		b->Type == EntityType_Player && a->Type == EntityType_Projectile)
    	{
	    	result = true;
    	}
    	if(a->Type == EntityType_Projectile && b->Type == EntityType_Wall ||
    		b->Type == EntityType_Projectile && a->Type == EntityType_Wall)
    	{
	    	result = true;
    	}
    }

	return result;
}

internal void HandleCollision(Entity* a, Entity* b, V2* r)
{
	if(a->Type == EntityType_Projectile && b->Type == EntityType_Wall ||
		b->Type == EntityType_Projectile && a->Type == EntityType_Wall)
	{
		//TODO(matt): Buggy because of edge cases and no nice geometry yet
    	if(a->Type == EntityType_Projectile)
    	{
    		// a->IsActive = false;
    		a->Acceleration = a->Acceleration - (a->Bounciness * (Dot(a->Acceleration, (*r)) * (*r)));
    	}
    	if(b->Type == EntityType_Projectile)
    	{
    		// a->IsActive = false;
    		b->Acceleration = b->Acceleration - (b->Bounciness * (Dot(b->Acceleration, (*r)) * (*r)));
    	}

	}
}

internal bool CircleColliderVsSquareCollider(Entity* a, Entity* b, V2* aDelta, f32* tMin, V2* r) 
{
	bool collided = false;

	f32 collisionDiameterX = b->CollisionWidth + (a->CollisionRadius * 2);
	f32 collisionDiameterY = b->CollisionHeight + (a->CollisionRadius * 2); 

	V2 tileMin = -0.5f * V2{collisionDiameterX, collisionDiameterY};
	V2 tileMax =  0.5f * V2{collisionDiameterX, collisionDiameterY};  

	V2 entityRelativePos = a->Position - b->Position;
	V2 newEntityRelativePos = (a->Position + *aDelta) - b->Position;
	
	if(TestEdge(aDelta->X, aDelta->Y, entityRelativePos.X, entityRelativePos.Y, tileMax.X,
		 tileMin.Y + a->CollisionRadius - 0.1f, tileMax.Y - a->CollisionRadius + 0.1f, tMin))
	{
		*r = V2{1, 0};
		collided = true;
	}
	if(TestEdge(aDelta->X, aDelta->Y, entityRelativePos.X, entityRelativePos.Y, tileMin.X, 
		tileMin.Y + a->CollisionRadius - 0.1f, tileMax.Y - a->CollisionRadius + 0.1f, tMin))
	{
		*r = V2{-1, 0};
		collided = true;
	}					
	if(TestEdge(aDelta->Y, aDelta->X, entityRelativePos.Y, entityRelativePos.X, tileMax.Y, 
		tileMin.X + a->CollisionRadius, tileMax.X - a->CollisionRadius, tMin))
	{
		*r = V2{0, 1};
		collided = true;
	}					
	if(TestEdge(aDelta->Y, aDelta->X, entityRelativePos.Y, entityRelativePos.X, tileMin.Y, 
		tileMin.X + a->CollisionRadius, tileMax.X - a->CollisionRadius, tMin))
	{
		*r = V2{0, -1};
		collided = true;
	}

	if(TestCorner(entityRelativePos, newEntityRelativePos, aDelta, a->CollisionRadius, r, tMin, 
				(0.5f * b->CollisionWidth),
				(0.5f * b->CollisionWidth) + a->CollisionRadius,
				(0.5f * b->CollisionHeight), 
				(0.5f * b->CollisionHeight) + a->CollisionRadius,
				(0.5f * b->CollisionWidth),
				(0.5f * b->CollisionHeight)))
	{
		collided = true;
	}

	if(TestCorner(entityRelativePos, newEntityRelativePos, aDelta, a->CollisionRadius, r, tMin, 
				(-0.5f * b->CollisionWidth) - a->CollisionRadius,
				(-0.5f * b->CollisionWidth),
				(0.5f * b->CollisionHeight), 
				(0.5f * b->CollisionHeight) + a->CollisionRadius,
				(-0.5f * b->CollisionWidth),
				(0.5f * b->CollisionHeight)))
	{
		collided = true;
	}

	if(TestCorner(entityRelativePos, newEntityRelativePos, aDelta, a->CollisionRadius, r, tMin, 
				 (0.5f * b->CollisionWidth),
				 (0.5f * b->CollisionWidth) + a->CollisionRadius,
				 (-0.5f * b->CollisionHeight) - a->CollisionRadius,
				 (-0.5f * b->CollisionHeight), 
				 (0.5f * b->CollisionWidth),
				 (-0.5f * b->CollisionHeight)))
	{
		collided = true;
	}

	if(TestCorner(entityRelativePos, newEntityRelativePos, aDelta, a->CollisionRadius, r, tMin, 
				 (-0.5f * b->CollisionWidth) - a->CollisionRadius,
				 (-0.5f * b->CollisionWidth),
				 (-0.5f * b->CollisionHeight) - a->CollisionRadius,
				 (-0.5f * b->CollisionHeight), 
				 (-0.5f * b->CollisionWidth),
				 (-0.5f * b->CollisionHeight)))
	{
		collided = true;
	}

	return collided;
}

internal bool DoesCollide(Entity* a, Entity* b, V2* aDelta, f32* tMin, V2* r) 
{
	bool collided = false;

	if(a->Collider == ColliderType_Circle && b->Collider == ColliderType_Box)
	{
		collided = CircleColliderVsSquareCollider(a, b, aDelta, tMin, r);
	}

	return collided;
}

internal void MoveEntity(Entity* entity, GameState* gameState)
{
	entity->Velocity =  (entity->Velocity + (entity->Acceleration * gameState->dT)) * entity->Damping;

	V2 oldEntityPosition = entity->Position;

	V2 newEntityPosition = (entity->Position) + 
						   (entity->Acceleration * Square(gameState->dT)) + 
						   (entity->Velocity  * gameState->dT);

    // TilePosition playerTile = GetTilePosition(oldEntityPosition, world);
    // TilePosition newPlayerTile = GetTilePosition(newEntityPosition, world);

	V2 entityDelta = newEntityPosition - oldEntityPosition;

	u32 maxIterations = 4;
				
	bool hasCollided = false;
	f32 tRemaining = 1.0f;

	for(u32 iterations = 0; (iterations < maxIterations && tRemaining > 0); iterations++)
	{
	    f32 tMin = 1.0f;
		V2 r = { 0,0 };	

		for(u32 entityIndex = 0; entityIndex <= ArrayCount(gameState->Entities); entityIndex++)
		{
			if(ShouldCollide(entity, &gameState->Entities[entityIndex]) && 
			   DoesCollide(entity, &gameState->Entities[entityIndex], &entityDelta, &tMin, &r))
			{
				if(!hasCollided)
				{
					hasCollided = true;
					HandleCollision(entity, &gameState->Entities[entityIndex], &r);
				}
			}
		}

		f32 epsilon = 0.01f;
		tRemaining -= tMin*tRemaining;

		if ((tMin - epsilon) < 0)
		{
			tMin = 0;
		}

		entity->Position = entity->Position + ((tMin - epsilon) * entityDelta);
		entity->Velocity = entity->Velocity - (entity->Bounciness * (Dot(entity->Velocity, r) * r));
		entityDelta =  entityDelta - (entity->Bounciness * (Dot(entityDelta, r)* r));
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

	World world = {};
	world.CountX = 15;
	world.CountY = 8;
	world.Tiles = *TileMap1;
	world.MetersToPixels = world.TilePixelLength / world.TileMeterLength;
	world.TileMeterLength = world.TileMeterLength * world.MetersToPixels;

	Color playerColor = CreateColor(0, 32, 32, 255);
	Color playerColColor = CreateColor(0, 255, 0, 0);

	if(!gameMemory->IsInitialized)
	{
		gameState->dT = 33.33f / 1000.0f;

		gameState->Entities[0].Index = 0;	
		gameState->Entities[0].Position = world.MetersToPixels* V2{2.5f, 2.5f};
		gameState->Entities[0].Speed = 120.0f * world.MetersToPixels;
		gameState->Entities[0].Width = 1.0f * world.MetersToPixels;
		gameState->Entities[0].Height = 1.2f * world.MetersToPixels;
		gameState->Entities[0].CollisionRadius = 0.5f * world.MetersToPixels;
		gameState->Entities[0].Damping = 0.2f;
		gameState->Entities[0].Bounciness = 1.0f;
		gameState->Entities[0].Type = EntityType_Player;
		gameState->Entities[0].Collider = ColliderType_Circle;

		gameState->Entities[1].Index = 1;	
		gameState->Entities[1].Speed = 180.0f * world.MetersToPixels;
		gameState->Entities[1].Width = 0.8f * world.MetersToPixels;
		gameState->Entities[1].Height = 0.8f * world.MetersToPixels;
		gameState->Entities[1].CollisionRadius = gameState->Entities[1].Width / 2;
		gameState->Entities[1].Damping = 0.2f;
		gameState->Entities[1].Bounciness = 2.0f;
		gameState->Entities[1].Type = EntityType_Projectile;
		gameState->Entities[1].Collider = ColliderType_Circle;
		gameState->Entities[1].Direction = V2{0.7f, 0.7f};
		gameState->Entities[1].Acceleration = gameState->Entities[1].Direction * gameState->Entities[1].Speed;
		gameState->Entities[1].IsActive = true;
		gameState->Entities[1].Position = GetTileCenter(1, 2, &world);

		u32 index = 2;

		for(s32 y = 0; y <= world.CountY; y++)
		{
			for(s32 x = 0; x <= world.CountX; x++)
			{
				if(TileMap1[y][x].TileType == 1 && TileMap1[y][x].HasGeometryLoaded == false)
				{
					TileMap1[y][x].HasGeometryLoaded = true;

					Entity* wall = &gameState->Entities[index];
					wall->Position = GetTileCenter(x, y, &world);
					wall->Width = world.TileMeterLength;
					wall->Height = world.TileMeterLength;
					wall->Type = EntityType_Wall;
					wall->Index = index;
					wall->Collider = ColliderType_Box;
					wall->CollisionWidth = world.TileMeterLength;
					wall->CollisionHeight = world.TileMeterLength;

					//TODO(matt)FIX THIS!!

					u32 wallCountY = 1;
					u32 wallCountX = 1;

					while(TileMap1[y + wallCountY][x].TileType == 1)
					{
						TileMap1[y + wallCountY][x].HasGeometryLoaded = true;

                        wall->Position.Y += world.TileMeterLength / 2;
						wall->CollisionHeight += world.TileMeterLength;
						wall->Height += world.TileMeterLength;
						wallCountY++;
					}

					if(TileMap1[y][x + wallCountX].TileType == 1)
					{
						index++;
						wall = &gameState->Entities[index];
						wall->Position = GetTileCenter(x, y, &world);
						wall->Width = world.TileMeterLength;
						wall->Height = world.TileMeterLength;
						wall->Type = EntityType_Wall;
						wall->Index = index;
						wall->Collider = ColliderType_Box;
						wall->CollisionWidth = world.TileMeterLength;
						wall->CollisionHeight = world.TileMeterLength;

						while(TileMap1[y][x + wallCountX].TileType == 1)
						{
							TileMap1[y][x  + wallCountX].HasGeometryLoaded = true;

	                        wall->Position.X += world.TileMeterLength / 2;
							wall->CollisionWidth += world.TileMeterLength;
							wall->Width += world.TileMeterLength;
							wallCountX++;
						}					
					}

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
	
	//Draw Tile map	
	for(s32 y = 0; y <= world.CountY; y++)
	{
		for(s32 x = 0; x <= world.CountX; x++)
		{
			if(TileMap1[y][x].TileType == 1)
			{
				DebugDrawRect(buffer, 
							  CreateColor(0, 60, 60, 60), 
							  GetTileCenter(x, y, &world),
							  world.TileMeterLength - 1.0f, 
							  world.TileMeterLength - 1.0f);
			}
			else if(TileMap1[y][x].TileType == 0)
			{
				DebugDrawRect(buffer, 
							  CreateColor(0, 240, 240, 240), 
							  GetTileCenter(x, y, &world),
							  world.TileMeterLength, 
							  world.TileMeterLength);
			}
		}
	}

	DebugDrawCircle(buffer, &playerColor, gameState->Entities[0].Position.X, gameState->Entities[0].Position.Y, gameState->Entities[0].CollisionRadius);		

	if(gameState->Entities[1].IsActive == true)
	{
		DebugDrawCircle(buffer, &playerColColor, gameState->Entities[1].Position.X, gameState->Entities[1].Position.Y, gameState->Entities[1].CollisionRadius);		
	}
}



