#ifndef __WALL__
#define __WALL__

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

#include <cmath>
#include <fstream>
#include <iostream>
#include <list>

class Map;  // Forward declaration

class Wall {
   private:
    SDL_FPoint center = {0, 0};  // coordinates of object in the play zone
    float most_right;            // distance on x axis to farest point on the right
    float most_left;             // distance on x axis to farest point in the left , always negative
    float most_top;              // distance on y axis to farest point on the top , always negative
    float most_bottom;           // distance on y axis to farest point on the bottom
    static SDL_Color color;
    std::list<SDL_FPoint> points;  // boundary points of polygon
    Map* map = nullptr;
    Wall();
    Wall(float x, float y);

   public:
    static const short data_size;  // size of wall - sizeof points

    bool InsideFrame(const SDL_FRect& frame);
    bool ContainPoint(float x, float y);
    std::list<SDL_FPoint>::iterator AddPoint(float x, float y);
    void Render();
    void Reset();
    inline void MoveBy(float dx, float dy) {
        this->center.x += dx;
        this->center.y += dy;
    }
    std::list<SDL_FPoint>::iterator SelectPointsInRect(const SDL_FRect& rect);
    std::list<SDL_FPoint>::iterator SelectPointAt(float x, float y);

    void RotateBy(float da);
    void SaveToFile(std::ofstream& file, int position = -1);
    void ReadFromFile(std::ifstream& file, int position = -1);
    void MovePointsBy(std::list<SDL_FPoint>::iterator start, float dx, float dy, int n = 1);

    friend class Map;
};

#endif  // __WALL__
