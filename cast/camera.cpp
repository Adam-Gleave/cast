#include "camera.h"
#include "picopng.cpp"

using namespace std;

const int TEX_DIMENSION = 64;
const int CAMERA_HEIGHT = 32;
const int FOG_DIST = 20;
const int SKY_R = 150;
const int SKY_G = 170;
const int SKY_B = 220;

Camera::Camera(shared_ptr<World> w, const int width, const int height) 
    : xpos(8), ypos(8), xdir(-1), ydir(0), xplane(0), yplane(0.6),
      width(width), height(height), move_speed(0.0), rot_speed(0.0)
{
    world = w;
    screen_pixels = vector<unsigned char>(width*height*4, 0);

    // Load textures
    vector<unsigned char> buffer, texture;
    unsigned long width_l = static_cast<unsigned long>(width);
    unsigned long height_l = static_cast<unsigned long>(height);
    loadFile(buffer, string("../data/tex/bricks.png"));
    decodePNG(texture, width_l, height_l, &buffer[0], buffer.size());
    textures.push_back(texture);
    loadFile(buffer, string("../data/tex/dirt.png"));
    decodePNG(texture, width_l, height_l, &buffer[0], buffer.size());
    textures.push_back(texture);
}

vector<unsigned char> Camera::render_buffer(const int width, const int height) 
{ 
    for (int x = 0; x < width; x++) 
    {
        RayInfo ray_info;
        CollisionInfo col_info;
        get_ray_info(x, width, ray_info);
        get_collision_info(ray_info, col_info);

        // Wall distance in camera direction
        double wall_dist = (col_info.hit_side == 0)
            ? wall_dist = (ray_info.map_x - xpos + (1 - ray_info.step_x) / 2) / ray_info.ray_x_direction
            : wall_dist = (ray_info.map_y - ypos + (1 - ray_info.step_y) / 2) / ray_info.ray_y_direction;
        
        // Locate where the ray collides with wall
        double wall_hit_x = (col_info.hit_side == 0)
            ? wall_hit_x = ypos + wall_dist * ray_info.ray_y_direction
            : wall_hit_x = xpos + wall_dist * ray_info.ray_x_direction;

        wall_hit_x -= floor(wall_hit_x);
            
        int draw_end = draw_walls(ray_info, col_info, wall_dist, wall_hit_x, x);
        draw_floor_ceiling(ray_info, col_info, wall_dist, wall_hit_x, draw_end, x);

        zbuff.push_back(wall_dist);
    }

    return screen_pixels;
}

int Camera::draw_walls(RayInfo& ray_info, CollisionInfo& col_info, double wall_dist, double wall_hit_x, int x)
{
    int texture_x = static_cast<int>(wall_hit_x * static_cast<double>(TEX_DIMENSION));
    if (col_info.hit_side == 0 && ray_info.ray_x_direction < 0) 
    {
        texture_x = TEX_DIMENSION - texture_x;
    }
    if (col_info.hit_side == 1 && ray_info.ray_y_direction > 0) 
    {
        texture_x = TEX_DIMENSION - texture_x;
    }

    int line_height = static_cast<int>(height / wall_dist);
    int draw_start = -line_height/2 + height/2;
    if (draw_start < 0) 
    {
        draw_start = 0;
    }

    int draw_end = line_height/2 + height/2;
    if (draw_end > height) 
    {
        draw_end = height - 1;
    }

    for (int y = draw_start; y < draw_end; y++) 
    {
        int d = y*TEX_DIMENSION - height*32 + line_height*32;
        int texture_y = d / line_height;
        texture_x %= TEX_DIMENSION;
        texture_y %= TEX_DIMENSION;

        unsigned char rgb[3];
        rgb[0] = textures[0][TEX_DIMENSION * texture_y*4 + texture_x*4];
        rgb[1] = textures[0][TEX_DIMENSION * texture_y*4 + texture_x*4 + 1];
        rgb[2] = textures[0][TEX_DIMENSION * texture_y*4 + texture_x*4 + 2];

        if (col_info.hit_side == 0) 
        {
            rgb[0] *= 0.75f;
            rgb[1] *= 0.75f;
            rgb[2] *= 0.75f;
        }

        calc_fog(wall_dist, rgb);

        const size_t offset = width * y*4 + x*4;
        set_pixel(offset, rgb);
    }

    return draw_end;
}

void Camera::draw_floor_ceiling(RayInfo& ray_info, CollisionInfo& col_info, double wall_dist, double wall_hit_x, int draw_end, int x)
{
    double floor_x_at_wall, floor_y_at_wall;

    if (col_info.hit_side == 0 && ray_info.ray_x_direction > 0) 
    {
        floor_x_at_wall = ray_info.map_x;
        floor_y_at_wall = ray_info.map_y + wall_hit_x;
    }
    else if (col_info.hit_side == 0 && ray_info.ray_x_direction <= 0) 
    {
        floor_x_at_wall = ray_info.map_x + 1.0f;
        floor_y_at_wall = ray_info.map_y + wall_hit_x;
    }
    else if (col_info.hit_side == 1 && ray_info.ray_y_direction > 0) 
    {
        floor_x_at_wall = ray_info.map_x + wall_hit_x;
        floor_y_at_wall = ray_info.map_y;
    }
    else 
    {
        floor_x_at_wall = ray_info.map_x + wall_hit_x;
        floor_y_at_wall = ray_info.map_y + 1.0f;
    }

    double current_dist;

    if (draw_end < 0) 
    {
        draw_end = height;
    }

    for (int y = draw_end + 1; y < height; y++) 
    {
        current_dist = height / (2.0f * y - height);
        double weight = current_dist / wall_dist;
        double current_floor_x = weight * floor_x_at_wall + (1.0f - weight) * xpos;
        double current_floor_y = weight * floor_y_at_wall + (1.0f - weight) * ypos;

        int floor_tex_x = static_cast<int>(current_floor_x * TEX_DIMENSION) % TEX_DIMENSION;
        int floor_tex_y = static_cast<int>(current_floor_y * TEX_DIMENSION) % TEX_DIMENSION;

        unsigned char rgb[3];
        rgb[0] = textures[1][TEX_DIMENSION * floor_tex_y * 4 + floor_tex_x * 4];
        rgb[1] = textures[1][TEX_DIMENSION * floor_tex_y * 4 + floor_tex_x * 4 + 1];
        rgb[2] = textures[1][TEX_DIMENSION * floor_tex_y * 4 + floor_tex_x * 4 + 2];
        calc_fog(current_dist, rgb);

        size_t buf_offset = width * y * 4 + x * 4;
        set_pixel(buf_offset, rgb);

        buf_offset = width * (height - y) * 4 + x * 4;
        set_pixel(buf_offset, rgb);
    }
}

void Camera::calc_fog(const double dist, unsigned char* rgb)
{
    double fog_amount = dist > FOG_DIST ? FOG_DIST : dist;
    rgb[0] += (static_cast<double>(SKY_R - rgb[0]) / FOG_DIST) * fog_amount;
    rgb[1] += (static_cast<double>(SKY_B- rgb[1]) / FOG_DIST) * fog_amount;
    rgb[2] += (static_cast<double>(SKY_B - rgb[2]) / FOG_DIST) * fog_amount;
}

void Camera::set_pixel(const size_t buf_offset, const unsigned char* rgb)
{
    screen_pixels[buf_offset] = rgb[2];
    screen_pixels[buf_offset + 1] = rgb[1];
    screen_pixels[buf_offset + 2] = rgb[0];
    screen_pixels[buf_offset + 3] = 255;
}

void Camera::handle_event(const Event e)
{
    switch (e)
    {
        case KEY_UP:
        {
            if (!world->getMapAt(static_cast<int>(xpos + xdir * move_speed), static_cast<int>(ypos))) 
            {
                xpos += xdir * move_speed; 
            }

            if (!world->getMapAt(static_cast<int>(xpos), static_cast<int>(ypos + ydir * move_speed))) 
            {
                ypos += ydir * move_speed;
            }

            break;
        }
        case KEY_DOWN:
        {
            if (!world->getMapAt(static_cast<int>(xpos - xdir * move_speed), static_cast<int>(ypos))) 
            {
                xpos -= xdir * move_speed; 
            }

            if (!world->getMapAt(static_cast<int>(xpos), static_cast<int>(ypos - ydir * move_speed))) 
            {
                ypos -= ydir * move_speed;
            }

            break;
        }
        case KEY_LEFT:
        {
            double prev_xdir = xdir;
            xdir = xdir*cos(rot_speed) - ydir*sin(rot_speed);
            ydir = prev_xdir*sin(rot_speed) + ydir*cos(rot_speed);
            double prev_xplane = xplane;
            xplane = xplane*cos(rot_speed) - yplane*sin(rot_speed);
            yplane = prev_xplane*sin(rot_speed) + yplane*cos(rot_speed);  

            break;
        }
        case KEY_RIGHT:
        {
            double prev_xdir = xdir;
            xdir = xdir*cos(-rot_speed) - ydir*sin(-rot_speed);
            ydir = prev_xdir*sin(-rot_speed) + ydir*cos(-rot_speed);
            double prev_xplane = xplane;
            xplane = xplane*cos(-rot_speed) - yplane*sin(-rot_speed);
            yplane = prev_xplane*sin(-rot_speed) + yplane*cos(-rot_speed);

            break;
        }
    }
}

void Camera::update_speeds(double move, double rot)
{
    move_speed = move;
    rot_speed = rot;
}

void Camera::get_ray_info(const int x, const int width, RayInfo& info) 
{
    info.x_camera_space = 2.0f * x / width - 1.0f;
    info.ray_x_direction = xdir + info.x_camera_space*xplane;
    info.ray_y_direction = ydir + info.x_camera_space*yplane;

    // Position in the map
    info.map_x = static_cast<int>(xpos);
    info.map_y = static_cast<int>(ypos);

    // Direction to step
    info.step_x = (info.ray_x_direction >= 0) ? 1 : -1;
    info.step_y = (info.ray_y_direction >= 0) ? 1 : -1;

    // Distance from side to next corresponding side
    info.delta_to_x = abs(1 / info.ray_x_direction);
    info.delta_to_y = abs(1 / info.ray_y_direction);

    // Distance from position to nearest side
    info.dist_to_x = (info.ray_x_direction >= 0) 
        ? (info.map_x + 1.0f - xpos) * info.delta_to_x 
        : (xpos - info.map_x) * info.delta_to_x;
    info.dist_to_y = (info.ray_y_direction >= 0)
        ? (info.map_y + 1.0f - ypos) * info.delta_to_y
        : (ypos - info.map_y) * info.delta_to_y;
}

void Camera::get_collision_info(RayInfo& ray_info, CollisionInfo& col_info) 
{
    int hit = 0;

    // DDA Algorithm
    while (!hit) 
    {
        // Advance ray to next intersection
        if (ray_info.dist_to_x < ray_info.dist_to_y) 
        {
            ray_info.dist_to_x += ray_info.delta_to_x;
            ray_info.map_x += ray_info.step_x;
            col_info.hit_side = 0;
        }
        else 
        {
            ray_info.dist_to_y += ray_info.delta_to_y;
            ray_info.map_y += ray_info.step_y;
            col_info.hit_side = 1;
        }

        // Check ray collision
        hit = world->getMapAt(ray_info.map_x, ray_info.map_y);
    }

    col_info.hit = hit;
}