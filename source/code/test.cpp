				//Collision check
				V2 testTileCenter = GetTileCenter(x, y, &world);

				real32 testTileMinX = testTileCenter.X - (0.5f * world.TilePuckLength * world.PucksToPixels);
				real32 testTileMinY = testTileCenter.Y - (0.5f * world.TilePuckLength * world.PucksToPixels);
				real32 testTileMaxX = testTileCenter.X + (0.5f * world.TilePuckLength * world.PucksToPixels);
				real32 testTileMaxY = testTileCenter.Y + (0.5f * world.TilePuckLength * world.PucksToPixels);

				real32 playerMinX = newPlayerPosition.X - (0.5f * playerWidth); 
				real32 playerMinY = newPlayerPosition.Y - (0.5f * playerHeight);
				real32 playerMaxX = newPlayerPosition.X + (0.5f * playerWidth);
				real32 playerMaxY = newPlayerPosition.Y + (0.5f * playerHeight);

				real32 minkowskiMinX = testTileMinX - playerMaxX;
				real32 minkowskiMinY = testTileMinY - playerMaxY;
				real32 minkowskiMaxX = testTileMaxX - playerMinX;
				real32 minkowskiMaxY = testTileMaxY - playerMinY;

				// if (minkowskiMinX < 0.0f &&
				//     minkowskiMaxX > 0.0f &&
				//     minkowskiMinY < 0.0f &&
				//     minkowskiMaxY > 0.0f)
				// {
				//     colliding = true;
			 //    	//closest point on tile
								    	
				// }
				// else
				// {
				// 	colliding = false;
				// }
