#define WORLD_X 10
#define WORLD_Y 10

class World 
{
public:
    World();
	~World() = default;

    const int getMapAt(const int x, const int y);

private:
    int map[WORLD_X][WORLD_Y];
};