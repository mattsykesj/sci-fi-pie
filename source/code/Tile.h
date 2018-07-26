#if !defined(TILE_H)

struct TilePosition
{
	u32 X;
	u32 Y;
};
//TODO(matt): Make tile type an enum? do we need entity types and tile types?
struct Tile
{
	u32 TileType; 
	bool IsLoaded;
	TilePosition TilePosition;
};

struct TileMap
{
	f32 TilePixelLength = 60.0f;
	f32 TileMeterLength = 1.35f;
	f32 MetersToPixels;
	u32 CountX;
	u32 CountY;

	Tile* Tiles;
};

internal V2 GetTileCenter(u32 x, u32 y, TileMap* tileMap)
{
	V2 result = {};

	result.X = (tileMap->TileMeterLength * x) + (0.5f * tileMap->TileMeterLength);
	result.Y = (tileMap->TileMeterLength * y) + (0.5f * tileMap->TileMeterLength);

	return result;
}

internal TilePosition GetTilePosition(V2 pos, TileMap* tileMap)
{
	TilePosition result = {};
	result.X = Truncatef32(pos.X / tileMap->TileMeterLength);
	result.Y = Truncatef32(pos.Y / tileMap->TileMeterLength);
	return result;
}


#define TILE_H


#endif 
