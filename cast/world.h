const int WORLD_X = 16;
const int WORLD_Y = 16;

class World 
{
public:
    World();
    ~World() {}

    const int getMapAt(const int x, const int y);

private:
    int map[WORLD_X][WORLD_Y];
};