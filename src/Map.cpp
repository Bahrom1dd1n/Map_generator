#include "Map.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_video.h>

#include "Wall.h"

Map::Map(int window_width, int window_height, SDL_WindowFlags flags) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Init(SDL_INIT_EVENTS);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
    int x = SDL_WINDOWPOS_CENTERED, y = SDL_WINDOWPOS_CENTERED;
    this->window =
        SDL_CreateWindow("Map", x, y, window_width, window_width, SDL_WINDOW_SHOWN | flags);
    this->renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    this->frame_delay = 16;
    int doubleBuffering = 0;
    SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &doubleBuffering);
    printf("Double Buffering ON: %d\n", doubleBuffering);
    SDL_GetWindowSize(window, &window_width, &window_height);
    frame.w = window_width;
    frame.h = window_height;
}
Map::~Map() {
    SDL_DestroyRenderer(this->renderer);
    SDL_DestroyWindow(this->window);
    SDL_Quit();
}

void Map::LoadWallsFromFile(const char* file_name) {
    std::ifstream file(file_name, std::ios::binary);
    if (!file.is_open()) {
        std::cout << " File not found!" << std::endl;
        return;
    }
    int num_obj;
    file.read((char*)&num_obj, sizeof(int));
    while (num_obj-- > 0) {
        walls.emplace_back();
        walls.back().ReadFromFile(file);
    }
    file.close();
}

void Map::SaveWallsToFile(const char* file_name) {
    if (walls.size() == 0) return;

    std::ofstream file(file_name, std::ios::binary | std::ios::trunc);
    file.seekp(0, std::ios::beg);

    int num_objects = walls.size();
    file.write((char*)&num_objects, sizeof(int));

    for (Wall& i : walls) {
        i.SaveToFile(file);
    }

    file.close();
}
void Map::ShowSelectedPoints() {
    if (!num_selected_point) return;
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_FPoint c = this->walls.front().center;

    c.x = (c.x - frame.x) * scale;
    c.y = (c.y - frame.y) * scale;

    SDL_FPoint p1;
    SDL_FRect rect;
    auto it = first_point;
    auto end = walls.front().points.end();
    for (int i = num_selected_point; i > 0; i--, it++) {
        if (it == end) it = walls.front().points.begin();

        p1 = {float(it->x * scale + c.x), float(it->y * scale + c.y)};
        rect = {p1.x - 3.0F, p1.y - 3.0F, 6.0F, 6.0F};
        SDL_RenderFillRectF(renderer, &rect);
    }
}
void Map::RenderAll() {
    auto it = walls.rbegin();
    color = {100, 255, 100, 255};

    for (int i = walls.size() - num_selected_obj; i > 0; i--, it++) it->Render();

    color = {178, 34, 34};
    while (it != walls.rend()) {
        it->Render();
        it++;
    }

    ShowSelectedPoints();
}
void Map::SelectObjectsInRect(SDL_FRect rect) {
    if (rect.w < 0) {
        rect.x += rect.w;
        rect.w = -rect.w;
    }

    if (rect.h < 0) {
        rect.y += rect.h;
        rect.h = -rect.h;
    }

    rect.w += rect.x;
    rect.h += rect.y;

    rect.x = rect.x * inverse_scale + frame.x;  // changin boundries relative to map scale
    rect.y = rect.y * inverse_scale + frame.y;
    rect.w = rect.w * inverse_scale + frame.x;
    rect.h = rect.h * inverse_scale + frame.y;

    num_selected_obj = 0;
    num_selected_point = 0;

    std::list<Wall>::iterator current = walls.begin();

    for (int i = walls.size(); i > 0; i--) {
        if (current->center.x < rect.x || current->center.x > rect.w ||
            current->center.y < rect.y || current->center.y > rect.h) {
            current++;
            continue;
        }

        auto temp = current;
        current++;
        walls.splice(walls.begin(), walls, temp);
        num_selected_obj++;
    }
}
void Map::DestroySelectedObjects() {
    if (num_selected_obj == 0) return;

    std::list<Wall>::iterator it = walls.begin();

    for (int i = num_selected_obj; i > 0; i--) {
        it = walls.erase(it);
    }

    num_selected_point = 0;
    num_selected_obj = 0;
    pointMoved = false;
}

void Map::DestroySelectedPoints() {
    if (num_selected_obj == 0 || num_selected_point == 0) return;
    auto& obj = walls.front();
    if (obj.points.size() < 4)  // at least polygon must have three vertexes
        return;

    auto it = first_point;
    auto end = obj.points.end();
    auto& selected_points = obj.points;
    for (int i = num_selected_point; i > 0; i--) {
        if (it == end) it = obj.points.begin();
        it = selected_points.erase(it);
    }

    pointMoved = false;
    num_selected_point = 0;
    obj.Reset();
}
void Map::SelectPointsInRect(SDL_FRect rect) {
    if (!(num_selected_obj == 1)) return;

    Wall& obj = walls.front();

    if (rect.w < 0) {
        rect.x += rect.w;
        rect.w = -rect.w;
    }

    if (rect.h < 0) {
        rect.y += rect.h;
        rect.h = -rect.h;
    }

    rect.w += rect.x;
    rect.h += rect.y;

    rect.x =
        rect.x * inverse_scale + frame.x - obj.center.x;  // changin boundries relative to object
    rect.y = rect.y * inverse_scale + frame.y - obj.center.y;
    rect.w = rect.w * inverse_scale + frame.x - obj.center.x;
    rect.h = rect.h * inverse_scale + frame.y - obj.center.y;

    if (obj.most_right < rect.x || obj.most_left > rect.w || obj.most_bottom < rect.y ||
        obj.most_top > rect.h)
        return;
    num_selected_point = 0;

    auto it = obj.points.begin();
    bool gap = true;

    for (int i = obj.points.size(); i > 0; i--, it++) {
        if (it->x < rect.x || it->x > rect.w || it->y < rect.y || it->y > rect.h) {
            gap = true;
            continue;
        }

        if (gap) {
            first_point = it;
            gap = false;
        }

        num_selected_point++;
    }
}

void Map::ResetSelectedObject() {
    if (!Map::num_selected_obj || !pointMoved) return;
    Wall& obj = Map::walls.front();
    obj.Reset();
    Map::pointMoved = false;
}
void Map::MoveSelectedObjectsBy(float dx, float dy) {
    if (Map::num_selected_obj == 0) return;

    std::list<Wall>::iterator it = Map::walls.begin();
    for (int i = Map::num_selected_obj; i > 0; i--, it++) {
        it->center.x += dx;
        it->center.y += dy;
    }
}
void Map::AddPointToSelected(float x, float y) {
    if (num_selected_obj == 0) return;
    Wall& obj = Map::walls.front();
    // if AddPoint return pointer to list::end
    // then point {x,y} is not near to any edge of the wall
    Map::first_point = obj.AddPoint(x, y);
    if (first_point != obj.points.end()) {
        Map::num_selected_point = 1;
        return;
    }
    Map::num_selected_point = 0;
}
bool Map::SelectObjectAt(float x, float y) {
    for (auto it = Map::walls.begin(); it != Map::walls.end(); it++) {
        if (it->ContainPoint(x, y)) {
            // selected object moved to the last potion
            Map::walls.splice(Map::walls.begin(), Map::walls, it);
            Map::num_selected_obj = 1;  // so if num_selected_obj == 1 , last object is selected
            Map::num_selected_point = 0;
            return true;
        }
    }

    Map::num_selected_obj = 0;
    Map::num_selected_point = 0;

    Map::pointMoved = false;
    return false;
}
void Map::CreateObjectAt(float x, float y) {
    Map::walls.emplace_front(this, x, y);
    Map::num_selected_obj = 1;
    Map::num_selected_point = 0;
}
void Map::RotateSelectedObjectBy(float da) {
    if (!da || !Map::num_selected_obj) return;
    Map::walls.front().RotateBy(da);
}
void Map::MoveSelectedPointsBy(float dx, float dy) {
    if (!Map::num_selected_point) return;
    Map::walls.front().MovePointsBy(first_point, dx, dy, num_selected_point);
    Map::pointMoved = true;
}
void Map::SelectPointFromSelectedObject(float x, float y) {
    if (Map::num_selected_obj == 0) return;

    Wall& obj = Map::walls.front();
    x -= obj.center.x;
    y -= obj.center.y;
    for (auto it = obj.points.begin(); it != obj.points.end(); it++) {
        float d = abs(it->x - x) + abs(it->y - y);
        if (d < 6.0F) {
            Map::first_point = it;
            Map::num_selected_point = 1;
            return;
        }
    }
    Map::num_selected_point = 0;
    Map::pointMoved = false;
}
void Map::OnKeyDown(unsigned short key) {
    if (key == old_key) return;

    old_key = key;
    switch (key) {
        case 26:  // key "W"

            dy = -0.3F;
            break;
        case 22:  // key "S"
            // if left ctrl is pressed with "S" then saving to file will be carried uot
            if (SDL_GetModState() == 64) {
                std::cout << " Enter name of file to be saved: ";
                std::string file_name;
                std::cin >> file_name;
                SaveWallsToFile(file_name.c_str());
                break;
            }
            dy = 0.3F;
            break;
        case 7:  // key "D"
            dx = 0.3F;
            break;
        case 4:  // key "A"
            dx = -0.3F;
            break;
        case 87:
            // if Left Ctrl is not pressed then zooming will not work
            if (SDL_GetModState() == 64) scaling = 1;
            break;  // key "+"

        case 86:
            // if L Ctrl is not pressed then zooming will not work
            if (SDL_GetModState() == 64) scaling = -1;
            break;  // key "-"

        case 93:  // key on numpad "5"
        {
            scaling = 0;
            inverse_scale = 1;
            float centrize = (inverse_scale - 1) * 0.5F;

            frame.x += frame.w * centrize;
            frame.y += frame.h * centrize;
            scale = 1;
        } break;

        case SDL_SCANCODE_DELETE:  // when delete key was pressed
            if (IsPointSelected())
                DestroySelectedPoints();
            else
                DestroySelectedObjects();

            break;

        case 15:
            if (SDL_GetModState() == 64) {
                std::cout << " Enter name of file to be loaded: ";
                std::string file_name;
                std::cin >> file_name;
                LoadWallsFromFile(file_name.c_str());
            }
            break;

        case 82:  // up arrow key
            selected_dy = -2.0F;
            break;

        case 81:  // down arrow key
            selected_dy = 2.0F;
            break;

        case 79:  // right arrow key , if  L Alt is pressed then it rotates selected object to
                  // right
            if (SDL_GetModState() == 256) {
                rotate_selected = 2.0F;
                break;
            }
            selected_dx = 2.0F;
            break;

        case 80:  // left arrow key, if L Alt is pressed then it rotates selected object to left
            if (SDL_GetModState() == 256) {
                rotate_selected = -2.0F;
                break;
            }
            selected_dx = -2.0F;
            break;
            // up: 82 ,down 81, right 79,left 80
    }

    std::cout << key << "\t State mode: " << SDL_GetModState() << std::endl;
}
void Map::OnKeyUp(unsigned short key) {
    {
        old_key = 0;
        switch (key) {
            case 26:
                dy = 0;
                break;  // "A" button
            case 22:
                dy = 0;
                break;  // "D" button
            case 7:
                dx = 0;
                break;  //"W" button
            case 4:
                dx = 0;
                break;  // "S" button
            case 87:
                scaling = 0;
                break;  // "+" sign
            case 86:
                scaling = 0;
                break;  // "-" sgin
            case 224:
                scaling = 0;
                break;  // L Ctrl

            case 82:  // up arrow key
                selected_dy = 0;
                if (!selected_dx) ResetSelectedObject();
                break;
            case 81:  // down arrow key
                selected_dy = 0;
                if (!selected_dx) ResetSelectedObject();
                break;
            case 79:  // reight arrow key
                rotate_selected = 0;
                selected_dx = 0;
                if (!selected_dy) ResetSelectedObject();
                break;
            case 80:  // left arrow key
                rotate_selected = 0;
                selected_dx = 0;
                if (!selected_dy) ResetSelectedObject();
                break;
        }
    }
}
void Map::OnMouseDown(float x, float y, short mb) {
    mouse_point.x = x;
    mouse_point.y = y;

    x = x * inverse_scale + frame.x;  // changing value of x relative to map and scale
    y = y * inverse_scale + frame.y;  // changing value of y relative to map and scale

    auto* keystate = SDL_GetKeyboardState(NULL);
    switch (mb) {
        case 1:  // left mouse button
            if (keystate[44]) {
                Map::CreateObjectAt(x, y);
                break;
            }

            if (SDL_GetModState() == 256) {
                select_rect.x = mouse_point.x;
                select_rect.y = mouse_point.y;
                select_mode = 1;
                break;
            }

            SelectObjectAt(x, y);
            break;

        case 3:  // right mouse button
            // if Ctr right mouse button is clicked while holding Ctrl add point to selected obj
            if (keystate[44]) {
                first_point = walls.front().AddPoint(x, y);
                num_selected_point = 1;
                break;
            }

            if (SDL_GetModState() == 256) {
                select_rect.x = mouse_point.x;
                select_rect.y = mouse_point.y;
                select_mode = 2;
                break;
            }

            first_point = walls.front().SelectPointAt(x, y);  // selecting point from selected wall
            num_selected_point = 1;
            break;
    }
}

void Map::OnMouseMove(float x, float y, short mb) {
    if (SDL_GetModState() == 256 && select_mode) {
        select_rect.w = x - select_rect.x;
        select_rect.h = y - select_rect.y;
        return;
    }

    if (mb == 1) {
        MoveSelectedObjectsBy((x - mouse_point.x) * inverse_scale,
                              (y - mouse_point.y) * inverse_scale);
        mouse_point.x = x;
        mouse_point.y = y;
    }
    if (mb == 2) {
        dx = (x - mouse_point.x) * 0.005F;
        dy = (y - mouse_point.y) * 0.005F;
    }
    if (mb == 4) {
        MoveSelectedPointsBy((x - mouse_point.x) * inverse_scale,
                             (y - mouse_point.y) * inverse_scale);
        mouse_point.x = x;
        mouse_point.y = y;
    }
}
void Map::OnMouseUp(float x, float y, short mb) {
    if (mb == 1) {
        if (select_mode == 1) {
            Map::SelectObjectsInRect(select_rect);
            select_mode = 0;
            std::printf("Rect: {%f, %f, %f, %f}", select_rect.x, select_rect.y, select_rect.w,
                        select_rect.h);
            select_rect.w = select_rect.h = 0.0F;
            return;
        }
        return;
    }
    if (mb == 2) dy = dx = 0;

    if (mb == 3) {
        if (select_mode == 2) {
            SelectPointsInRect(select_rect);
            select_mode = 0;
            select_rect.w = select_rect.h = 0.0F;
            return;
        }

        ResetSelectedObject();
    }
}
void Map::HandleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;

            case SDL_KEYDOWN:
                OnKeyDown(event.key.keysym.scancode);
                break;

            case SDL_KEYUP:
                OnKeyUp(event.key.keysym.scancode);
                break;

            case SDL_MOUSEMOTION:
                OnMouseMove(event.button.x, event.button.y, event.button.button);
                break;
            case SDL_MOUSEBUTTONDOWN:
                OnMouseDown(event.button.x, event.button.y, event.button.button);
                break;

            case SDL_MOUSEBUTTONUP:
                OnMouseUp(event.button.x, event.button.y, event.button.button);
                break;

            case SDL_MOUSEWHEEL:
                if (SDL_GetModState() != 64)  // if L Ctrl is not pressed then zooming will not work
                    break;
                if (event.wheel.y > 0)
                    scaling = 1;
                else
                    scaling = -1;
                break;
        }
    }
}
void Map::Update() {
    SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, 255);
    SDL_RenderClear(this->renderer);
    if (scaling) {
        if ((scale < 0.5 && scaling < 0) || (scale > 3 && scaling > 0)) {
            scaling = 0;
            return;
        }

        float dS = scaling * 0.01F;
        /* oreder of calculations is important*/
        scale += dS;
        float centrize = (inverse_scale - 1 / scale) * 0.5F;
        inverse_scale = 1 / scale;

        frame.x += frame.w * centrize;
        frame.y += frame.h * centrize;
    }

    if (dy) frame.y += dy * time_elapsed;

    if (dx) frame.x += dx * time_elapsed;

    if (selected_dx || selected_dy) {
        if (IsPointSelected()) {
            MoveSelectedPointsBy(selected_dx, selected_dy);
        } else
            MoveSelectedObjectsBy(selected_dx, selected_dy);
    }
    if (rotate_selected) {
        RotateSelectedObjectBy(rotate_selected);
    };
    auto it = this->walls.begin();
    for (int i = 0; i < this->num_selected_obj; i++) {
        if (it->InsideFrame(this->frame)) it->Render(true);
        it++;
    }
    while (it != walls.end()) {
        if (it->InsideFrame(this->frame)) it->Render();
        it++;
    }
    ShowSelectedPoints();
    SDL_RenderPresent(this->renderer);
}
void Map::Start() {
    this->running = true;

    while (running) {
        /*auto start = SDL_GetTicks();*/
        // event handling
        HandleEvents();
        Update();
        // controlling frame rate
        auto now = SDL_GetTicks();
        this->time_elapsed = now - this->time_passed;
        this->time_passed = now;
        if (this->time_elapsed < this->frame_delay) {
            SDL_Delay(this->frame_delay - this->time_elapsed);
            this->time_elapsed = frame_delay;
        }
    }
};

void Map::Stop() { running = false; }
