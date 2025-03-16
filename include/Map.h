#ifndef __MAP__
#define __MAP__

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>

#include <cmath>
#include <fstream>
#include <iostream>
#include <list>

class Wall;  // Forward declaration of the Wall class

class Map {
   public:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_FRect frame = {0.0f, 0.0f, 0.0f, 0.0f};
    SDL_FRect select_rect = {0.0f, 0.0f, 0.0f, 0.0f};
    float scale = 1;
    float inverse_scale = 1;
    float dy = 0, dx = 0;  // moving speed of map
    float selected_dx = 0, selected_dy = 0;
    float rotate_selected = 0;
    short scaling = 0;
    std::list<SDL_FPoint>::iterator first_point;  // first selected point
    SDL_Color color;                              // color code of edges of all object

    std::list<Wall> walls;
    unsigned long time_passed = 0;          // time passed since program started
    unsigned int time_elapsed = 0;          // time passed since last render
    int num_selected_obj;                   // number of selected items
    int num_selected_point;                 // number of selected points in selected object
    SDL_FPoint mouse_point = {0.0F, 0.0F};  // where mouse last clicked relative to origin of map
    unsigned short frame_delay = 16;
    unsigned short old_key = 0;
    bool running = false;
    bool pointMoved;  // determines if selected point is moved
    char select_mode = 0;

    void HandleEvents();
    void Update();

    void OnKeyDown(unsigned short key);
    void OnKeyUp(unsigned short key);
    void OnMouseDown(float x, float y, short mb);
    void OnMouseMove(float x, float y, short mb);
    void OnMouseUp(float x, float y, short mb);

   public:
    Map(int window_width, int window_height, SDL_WindowFlags flags = SDL_WINDOW_SHOWN);
    inline SDL_Renderer* GetRenderer() const { return this->renderer; }
    inline const SDL_FRect GetFrame() const { return this->frame; }
    inline bool IsPointSelected() const { return Map::num_selected_point; }
    inline bool IsObjectSelected() const { return num_selected_obj; }

    void RotateSelectedObjectBy(float da);
    void DestroySelectedObjects();
    void AddPointToSelected(float x, float y);
    void MoveSelectedPointsBy(float dx, float dy);
    void MoveSelectedObjectsBy(float dx, float dy);
    void SelectPointsInRect(SDL_FRect rect);
    void SelectObjectsInRect(SDL_FRect rect);
    void ShowSelectedPoints();
    void RenderAll();
    void DestroySelectedPoints();
    void SelectPointFromSelectedObject(float x, float y);
    void CreateObjectAt(float x, float y);
    bool SelectObjectAt(float x, float y);
    void ResetSelectedObject();
    void LoadMap(const char* file_name);
    void SaveMap(const char* file_name);

    void Start();
    void Stop();
    ~Map();
    friend class Wall;
};

#endif  // !__MAP__
