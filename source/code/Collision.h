#if !defined(COLLISION_H)

#include "Sci_Fi_Pie.h"

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


#define COLLISION_H


#endif 
