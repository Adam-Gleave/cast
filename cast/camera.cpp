#include "../cast/camera.h"
#include "picopng.cpp"

#define TEX_DIMENSION 64
#define CAMERA_HEIGHT 32

CollisionInfo::CollisionInfo() : hit(0), hit_side(0) { }

Camera::Camera() : xpos(5), ypos(5), xdir(-1), ydir(0), xplane(0), yplane(0.6) { }

Camera::Camera(World* w) : xpos(5), ypos(5), xdir(-1), ydir(0), xplane(0), yplane(0.6) {
    world = w;
}

Camera::~Camera() {
    delete world;
}

std::vector<unsigned char> Camera::render_buffer(const int width, const int height) { 
    std::vector<unsigned char> screen_pixels(width*height*4, 0);
	std::vector<std::vector<unsigned char>> textures;
	std::vector<int> zbuff;

    // Placeholder texture
    std::vector<unsigned char> buffer, texture;
    loadFile(buffer, std::string("../data/tex/bricks.png"));
    unsigned long w, h;
    decodePNG(texture, w, h, &buffer[0], (unsigned long)buffer.size());
	textures.push_back(texture);

	loadFile(buffer, std::string("../data/tex/dirt.png"));
	decodePNG(texture, w, h, &buffer[0], (unsigned long)buffer.size());
	textures.push_back(texture);

    for (int x = 0; x < width; x++) {
        RayInfo ray_info;
        CollisionInfo col_info;
        get_ray_info(x, width, ray_info);
        get_collision_info(ray_info, col_info);

        // Wall distance in camera direction
        double wall_dist = (col_info.hit_side == 0)
            ? wall_dist = (ray_info.map_x - xpos + (1 - ray_info.step_x) / 2) / ray_info.ray_x_direction
            : wall_dist = (ray_info.map_y - ypos + (1 - ray_info.step_y) / 2) / ray_info.ray_y_direction;
        
        double wall_hit_x = (col_info.hit_side == 0)
            ? wall_hit_x = ypos + wall_dist * ray_info.ray_y_direction
            : wall_hit_x = xpos + wall_dist * ray_info.ray_x_direction;

        wall_hit_x -= floor(wall_hit_x);
            
        int texture_x = static_cast<int>(wall_hit_x * static_cast<double>(TEX_DIMENSION));
        if (col_info.hit_side == 0 && ray_info.ray_x_direction < 0) {
            texture_x = TEX_DIMENSION - texture_x;
        }
        if (col_info.hit_side == 1 && ray_info.ray_y_direction > 0) {
            texture_x = TEX_DIMENSION - texture_x;
        }

        int line_height = static_cast<int>(height / wall_dist);

        int draw_start = -line_height/2 + height/2;
        if (draw_start < 0) {
            draw_start = 0;
        }

        int draw_end = line_height/2 + height/2;
        if (draw_end > height) {
            draw_end = height - 1;
        }

        for (int y = draw_start; y < draw_end; y++) {
            int d = y*TEX_DIMENSION - height*32 + line_height*32;
            int texture_y = d / line_height;
            texture_x %= TEX_DIMENSION;
            texture_y %= TEX_DIMENSION;

            unsigned char r = textures[0][TEX_DIMENSION * texture_y*4 + texture_x*4];
            unsigned char g = textures[0][TEX_DIMENSION * texture_y*4 + texture_x*4 + 1];
            unsigned char b = textures[0][TEX_DIMENSION * texture_y*4 + texture_x*4 + 2];

			if (col_info.hit_side == 0) {
				r *= 0.75f;
				g *= 0.75f;
				b *= 0.75f;
			}

            const int offset = width * y*4 + x*4;
            screen_pixels[offset] = b;
            screen_pixels[offset + 1] = g;
            screen_pixels[offset + 2] = r;
            screen_pixels[offset + 3] = 255;
        }

		double floor_x_at_wall, floor_y_at_wall;

		if (col_info.hit_side == 0 && ray_info.ray_x_direction > 0) {
			floor_x_at_wall = ray_info.map_x;
			floor_y_at_wall = ray_info.map_y + wall_hit_x;
		}
		else if (col_info.hit_side == 0 && ray_info.ray_x_direction <= 0) {
			floor_x_at_wall = ray_info.map_x + 1.0f;
			floor_y_at_wall = ray_info.map_y + wall_hit_x;
		}
		else if (col_info.hit_side == 1 && ray_info.ray_y_direction > 0) {
			floor_x_at_wall = ray_info.map_x + wall_hit_x;
			floor_y_at_wall = ray_info.map_y;
		}
		else {
			floor_x_at_wall = ray_info.map_x + wall_hit_x;
			floor_y_at_wall = ray_info.map_y + 1.0f;
		}

		double current_dist;

		if (draw_end < 0) {
			draw_end = height;
		}

		for (int y = draw_end + 1; y < height; y++) {
			current_dist = height / (2.0f * y - height);
			double weight = current_dist / wall_dist;
			double current_floor_x = weight * floor_x_at_wall + (1.0f - weight) * xpos;
			double current_floor_y = weight * floor_y_at_wall + (1.0f - weight) * ypos;

			int floor_tex_x = static_cast<int>(current_floor_x * TEX_DIMENSION) % TEX_DIMENSION;
			int floor_tex_y = static_cast<int>(current_floor_y * TEX_DIMENSION) % TEX_DIMENSION;

			unsigned char r = textures[1][TEX_DIMENSION * floor_tex_y * 4 + floor_tex_x * 4];
			unsigned char g = textures[1][TEX_DIMENSION * floor_tex_y * 4 + floor_tex_x * 4 + 1];
			unsigned char b = textures[1][TEX_DIMENSION * floor_tex_y * 4 + floor_tex_x * 4 + 2];

			int offset = width * y * 4 + x * 4;
			screen_pixels[offset] = b;
			screen_pixels[offset + 1] = g;
			screen_pixels[offset + 2] = r;
			screen_pixels[offset + 3] = 255;

			offset = width * (height - y) * 4 + x * 4;
			screen_pixels[offset] = 220;
			screen_pixels[offset + 1] = 170;
			screen_pixels[offset + 2] = 150;
			screen_pixels[offset + 3] = 255;
		}

		zbuff.push_back(wall_dist);
    }

    return screen_pixels;
}

void Camera::get_ray_info(const int x, const int width, RayInfo& info) {
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
    info.delta_to_x = std::abs(1 / info.ray_x_direction);
    info.delta_to_y = std::abs(1 / info.ray_y_direction);

    // Distance from position to nearest side
    info.dist_to_x = (info.ray_x_direction >= 0) 
        ? (info.map_x + 1.0f - xpos) * info.delta_to_x 
        : (xpos - info.map_x) * info.delta_to_x;
    info.dist_to_y = (info.ray_y_direction >= 0)
        ? (info.map_y + 1.0f - ypos) * info.delta_to_y
        : (ypos - info.map_y) * info.delta_to_y;
}

void Camera::get_collision_info(RayInfo& ray_info, CollisionInfo& col_info) {
        int hit = 0;

        // DDA Algorithm
        while (!hit) {
            // Advance ray to next intersection
            if (ray_info.dist_to_x < ray_info.dist_to_y) {
                ray_info.dist_to_x += ray_info.delta_to_x;
                ray_info.map_x += ray_info.step_x;
                col_info.hit_side = 0;
            }
            else {
                ray_info.dist_to_y += ray_info.delta_to_y;
                ray_info.map_y += ray_info.step_y;
                col_info.hit_side = 1;
            }

            // Check ray collision
            hit = world->getMapAt(ray_info.map_x, ray_info.map_y);
        }

        col_info.hit = hit;
}