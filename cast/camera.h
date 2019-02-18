#include "events.h"
#include "world.h"
#include <vector>
#include <memory>

struct RayInfo 
{
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

struct CollisionInfo 
{
    int hit_side;
    int hit;

    CollisionInfo() : hit(0), hit_side(0) {}
};

class Camera : public Handler
{
public:
    Camera(std::shared_ptr<World> w, const int width, const int height);
    ~Camera() {}

    void handle_event(const Event e) override;

    std::vector<unsigned char> render_buffer(int width, int height);
    void update_speeds(double move_speed, double rot_speed);

private:
    void get_ray_info(int x, int width, RayInfo& info);
    void get_collision_info(RayInfo& ray_info, CollisionInfo& col_info);
    double get_wall_distance(RayInfo& ray_info, CollisionInfo& col_info);
    void calc_fog(const double dist, unsigned char* rgb);
    void set_pixel(const size_t offset, const unsigned char* rgb);
    
    int draw_walls(RayInfo& ray_info, CollisionInfo& col_info, double wall_dist, double wall_hit_x, int x);
    void draw_floor_ceiling(RayInfo& ray_info, CollisionInfo& col_info, double wall_dist, double wall_hit_x, int draw_end, int x);

    std::vector<unsigned char> screen_pixels;
    std::vector<std::vector<unsigned char>> textures;
    std::vector<int> zbuff;

    int width;
    int height;
    double move_speed;
    double rot_speed;
    double xpos;
    double ypos;
    double xdir;
    double ydir;
    double xplane;
    double yplane;

    std::shared_ptr<World> world;
};
