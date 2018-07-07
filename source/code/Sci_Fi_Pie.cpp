#include "Sci_Fi_Pie.h"
#include "Game_Math.h"
#include "Game_Intrinsics.h"

#define Minimum(A, B) ((A < B) ? (A) : (B))
#define Maximum(A, B) ((A > B) ? (A) : (B))
#define ArrayCount( array ) ( sizeof( array ) / sizeof( array[0] ))


#define StickDeadZone 8000
#define StickMaxValue 32767

struct World
{
	f32 TilePixelLength = 60.0f;
	f32 TilePuckLength = 1.35f;
	f32 PucksToPixels;
	u32 CountX;
	u32 CountY;

	u32* Tiles;
};

struct TilePosition
{
	u32 X;
	u32 Y;
};

static u32 TileMap1[9][16] = 
{
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1},
	{1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1},
	{1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
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

internal void PutPixel(int x, int y, Color* color, GameBackBuffer* buffer)
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
          	
        PutPixel(x0 + x, y0 + y, color, buffer);
        PutPixel(x0 + y, y0 + x, color, buffer);
        PutPixel(x0 - y, y0 + x, color, buffer);
        PutPixel(x0 - x, y0 + y, color, buffer);
        PutPixel(x0 - x, y0 - y, color, buffer);
        PutPixel(x0 - y, y0 - x, color, buffer);
        PutPixel(x0 + y, y0 - x, color, buffer);
        PutPixel(x0 + x, y0 - y, color, buffer);

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

	result.X = (world->TilePuckLength * x) + (0.5f * world->TilePuckLength);
	result.Y = (world->TilePuckLength * y) + (0.5f * world->TilePuckLength);

	return result;
}

internal TilePosition GetTilePosition(V2 pos, World* world)
{
	TilePosition result = {};
	result.X = Truncatef32(pos.X / world->TilePuckLength);
	result.Y = Truncatef32(pos.Y / world->TilePuckLength);
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
    		a->Acceleration = a->Acceleration - (a->Bounciness * (Dot(a->Acceleration, (*r)) * (*r)));
    	}
    	if(b->Type == EntityType_Projectile)
    	{
    		b->Acceleration = b->Acceleration - (b->Bounciness * (Dot(b->Acceleration, (*r)) * (*r)));
    	}

	}
}

internal bool CircleVsSquareCollision(Entity* a, Entity* b, V2* aDelta, f32* tMin, V2* r) 
{
	bool collided = false;

	f32 collisionDiameterX = b->CollisionWidth + (a->CollisionRadius * 2);
	f32 collisionDiameterY = b->CollisionWidth + (a->CollisionRadius * 2); 

	V2 tileMin = -0.5f * V2{collisionDiameterX, collisionDiameterY};
	V2 tileMax =  0.5f * V2{collisionDiameterX, collisionDiameterY};  

	V2 playerRelativePos = a->Position - b->Position;
	V2 newPlayerRelativePos = (a->Position + *aDelta) - b->Position;
	
	if(TestEdge(aDelta->X, aDelta->Y, playerRelativePos.X, playerRelativePos.Y, tileMax.X,
		 tileMin.Y + a->CollisionRadius - 0.1f, tileMax.Y - a->CollisionRadius + 0.1f, tMin))
	{
		*r = V2{1, 0};
		collided = true;
	}
	if(TestEdge(aDelta->X, aDelta->Y, playerRelativePos.X, playerRelativePos.Y, tileMin.X, 
		tileMin.Y + a->CollisionRadius - 0.1f, tileMax.Y - a->CollisionRadius + 0.1f, tMin))
	{
		*r = V2{-1, 0};
		collided = true;
	}					
	if(TestEdge(aDelta->Y, aDelta->X, playerRelativePos.Y, playerRelativePos.X, tileMax.Y, 
		tileMin.X + a->CollisionRadius, tileMax.X - a->CollisionRadius, tMin))
	{
		*r = V2{0, 1};
		collided = true;
	}					
	if(TestEdge(aDelta->Y, aDelta->X, playerRelativePos.Y, playerRelativePos.X, tileMin.Y, 
		tileMin.X + a->CollisionRadius, tileMax.X - a->CollisionRadius, tMin))
	{
		*r = V2{0, -1};
		collided = true;
	}

	if(TestCorner(playerRelativePos, newPlayerRelativePos, aDelta, a->CollisionRadius, r, tMin, 
				(0.5f * b->CollisionWidth),
				(0.5f * b->CollisionWidth) + a->CollisionRadius,
				(0.5f * b->CollisionHeight), 
				(0.5f * b->CollisionHeight) + a->CollisionRadius,
				(0.5f * b->CollisionWidth),
				(0.5f * b->CollisionHeight)))
	{
		collided = true;
	}

	if(TestCorner(playerRelativePos, newPlayerRelativePos, aDelta, a->CollisionRadius, r, tMin, 
				(-0.5f * b->CollisionWidth) - a->CollisionRadius,
				(-0.5f * b->CollisionWidth),
				(0.5f * b->CollisionHeight), 
				(0.5f * b->CollisionHeight) + a->CollisionRadius,
				(-0.5f * b->CollisionWidth),
				(0.5f * b->CollisionHeight)))
	{
		collided = true;
	}

	if(TestCorner(playerRelativePos, newPlayerRelativePos, aDelta, a->CollisionRadius, r, tMin, 
				 (0.5f * b->CollisionWidth),
				 (0.5f * b->CollisionWidth) + a->CollisionRadius,
				 (-0.5f * b->CollisionHeight) - a->CollisionRadius,
				 (-0.5f * b->CollisionHeight), 
				 (0.5f * b->CollisionWidth),
				 (-0.5f * b->CollisionHeight)))
	{
		collided = true;
	}

	if(TestCorner(playerRelativePos, newPlayerRelativePos, aDelta, a->CollisionRadius, r, tMin, 
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
		collided = CircleVsSquareCollision(a, b, aDelta, tMin, r);
	}

	return collided;
}

internal void MoveEntity(Entity* entity, f32* dT, World* world, GameState* gameState)
{
	entity->Velocity =  (entity->Velocity + (entity->Acceleration * (*dT))) * entity->Damping;

	V2 oldEntityPosition = entity->Position;

	V2 newEntityPosition = (entity->Position) + 
						   (entity->Acceleration * Square(*dT)) + 
						   (entity->Velocity  * (*dT));

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

internal void UpdatePlayer(Entity* player, f32* dT, World* world, GameState* gameState)
{
	MoveEntity(player, dT, world, gameState);
}

internal void UpdateProjectile(Entity* projectile, f32* dT, World* world, GameState* gameState)
{
	MoveEntity(projectile, dT, world, gameState);
}

internal void GameLoop(GameBackBuffer* buffer, GameInput* gameInput, GameMemory* gameMemory)
{
	GameState* gameState = (GameState*)gameMemory->MainMemory;

	World world = {};
	world.CountX = 15;
	world.CountY = 8;
	world.Tiles = *TileMap1;
	world.PucksToPixels = world.TilePixelLength / world.TilePuckLength;
	world.TilePuckLength = world.TilePuckLength * world.PucksToPixels;

	Color playerColor = CreateColor(0, 32, 32, 255);
	Color playerColColor = CreateColor(0, 255, 0, 0);

	if(!gameMemory->IsInitialized)
	{
		gameState->dT = 33.33f / 1000.0f;

		gameState->Entities[0].Index = 0;	
		gameState->Entities[0].Position = world.PucksToPixels* V2{2.5f, 2.5f};
		gameState->Entities[0].AccelerationMag = 120.0f * world.PucksToPixels;
		gameState->Entities[0].Width = 1.0f * world.PucksToPixels;
		gameState->Entities[0].Height = 1.2f * world.PucksToPixels;
		gameState->Entities[0].CollisionRadius = 0.5f * world.PucksToPixels;
		gameState->Entities[0].Damping = 0.2f;
		gameState->Entities[0].Bounciness = 1.0f;
		gameState->Entities[0].Type = EntityType_Player;
		gameState->Entities[0].Collider = ColliderType_Circle;

		gameState->Entities[1].Index = 1;	
		gameState->Entities[1].Position = world.PucksToPixels* V2{10.0f, 10.0f};
		gameState->Entities[1].AccelerationMag = 180.0f * world.PucksToPixels;
		gameState->Entities[1].Width = 0.8f * world.PucksToPixels;
		gameState->Entities[1].Height = 0.8f * world.PucksToPixels;
		gameState->Entities[1].CollisionRadius = gameState->Entities[1].Width / 2;
		gameState->Entities[1].Damping = 0.2f;
		gameState->Entities[1].Direction = V2 { -0.5f, 0.5f };
		gameState->Entities[1].Bounciness = 2.0f;
		gameState->Entities[1].Acceleration = gameState->Entities[1].Direction * gameState->Entities[1].AccelerationMag;
		gameState->Entities[1].Type = EntityType_Projectile;
		gameState->Entities[1].Collider = ColliderType_Circle;

		u32 index = 2;
		for(s32 y = 0; y <= world.CountY; y++)
		{

			for(s32 x = 0; x <= world.CountX; x++)
			{
				if(TileMap1[y][x] == 1)
				{
					Entity* wall = &gameState->Entities[index];
					wall->Position = GetTileCenter(x, y, &world);
					wall->Width = world.TilePuckLength;
					wall->Height = world.TilePuckLength;
					wall->Type = EntityType_Wall;
					wall->Index = index;
					wall->Collider = ColliderType_Box;
					wall->CollisionWidth = world.TilePuckLength;
					wall->CollisionHeight = world.TilePuckLength;

					index++;
				}
			}
		}

		gameMemory->IsInitialized = true;
	}

	//Get Input from all controllers
	gameState->Entities[0].Direction = V2{ 0,0 };

	//Make controllers specific to player entities 
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

		gameState->Entities[0].Acceleration = gameState->Entities[0].AccelerationMag * gameState->Entities[0].Direction;
	}

	UpdatePlayer(&gameState->Entities[0], &gameState->dT, &world, gameState);
	UpdateProjectile(&gameState->Entities[1], &gameState->dT, &world, gameState);
	
	//Draw Tile map	
	for(s32 y = 0; y <= world.CountY; y++)
	{
		for(s32 x = 0; x <= world.CountX; x++)
		{
			if(TileMap1[y][x] == 1)
			{
				DebugDrawRect(buffer, 
							  CreateColor(0, 60, 60, 60), 
							  GetTileCenter(x, y, &world),
							  world.TilePuckLength - 1.0f, 
							  world.TilePuckLength - 1.0f);
			}
			else if(TileMap1[y][x] == 0)
			{
				DebugDrawRect(buffer, 
							  CreateColor(0, 240, 240, 240), 
							  GetTileCenter(x, y, &world),
							  world.TilePuckLength, 
							  world.TilePuckLength);
			}
		}
	}


	DebugDrawCircle(buffer, &playerColor, gameState->Entities[0].Position.X, gameState->Entities[0].Position.Y, gameState->Entities[0].CollisionRadius);		

	DebugDrawCircle(buffer, &playerColColor, gameState->Entities[1].Position.X, gameState->Entities[1].Position.Y, gameState->Entities[1].CollisionRadius);		
}



