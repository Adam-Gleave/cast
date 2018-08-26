#include "world.h"
#include <vector>

struct RayInfo {
    double x_camera_space;
    double ray_x_direction;
    double ray_y_direction;

    int map_x;
    int map_y;
    int step_x;
    int step_y;

    double delta_to_x;
    double delta_to_y;
    double dist_to_x;
    double dist_to_y;
};

struct CollisionInfo {
    int hit_side;
    int hit;

    CollisionInfo();
};

class Camera {
public:
    double xpos;
    double ypos;
    double xdir;
    double ydir;
    double xplane;
    double yplane;

    World* world;

    Camera();
    Camera(World* w);
    ~Camera();

    std::vector<unsigned char> render_buffer(int width, int height);

private:
    void get_ray_info(int x, int width, RayInfo& info);
    void get_collision_info(RayInfo& ray_info, CollisionInfo& col_info);
};
