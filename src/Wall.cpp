#include "Wall.h"

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

#include <list>

#include "Map.h"

SDL_Color Wall::color = {100, 255, 100, 255};

Wall::Wall(const SDL_FPoint& center) : center(center) {};

Wall::Wall(Map* map, float x, float y) {
    this->center = {x, y};
    this->map = map;

    this->points.push_back({-40.0F, -40.0F});
    this->points.push_back({40.0F, -40.0F});
    this->points.push_back({40.0F, 40.0F});
    this->points.push_back({-40.0F, 40.0F});

    this->most_right = this->most_bottom = 40.0F;
    this->most_left = this->most_top = -40.0F;
}

bool Wall::InsideFrame(const SDL_FRect& frame) {
    if (this->center.x + this->most_right < frame.x ||
        this->center.x + this->most_left > frame.x + frame.w)
        return false;

    if (this->center.y + this->most_bottom < frame.y ||
        this->center.y + this->most_top > frame.y + frame.h)
        return false;

    return true;
}

bool Wall::ContainPoint(float x, float y) {
    x -= this->center.x;
    y -= this->center.y;

    if (x > this->most_right || x < this->most_left) return false;
    if (y > this->most_bottom || y < this->most_top) return false;

    SDL_FPoint* p1 = &this->points.back();

    int num = 0;  // number of intersections
    for (auto& p2 : this->points) {
        if ((y < p1->y && y < p2.y) || (y > p1->y && y > p2.y)) {
            p1 = &p2;
            continue;
        }
        if (x < p1->x && x < p2.x) {
            p1 = &p2;
            continue;
        }

        if (x > p1->x && x > p2.x) {
            num++;
            p1 = &p2;
            continue;
        }

        float d = ((p1->x - x) * (p1->y - p2.y) + (p1->y - y) * (p2.x - p1->x)) * (p1->y - p2.y);

        if (d < 0) num++;
        p1 = &p2;
    }

    if (num & 1)  // if number of intersections is odd then the point is inside of polygon
        return true;

    return false;
}

void Wall::Render(bool selected) {
    const SDL_FRect& frame = this->map->GetFrame();
    float& scale = this->map->scale;
    SDL_Renderer* renderer = this->map->GetRenderer();
    SDL_FPoint c = {(this->center.x - frame.x) * scale, (this->center.y - frame.y) * scale};

    SDL_FRect rect = {c.x - 4.0F, c.y - 4.0F, 8.0F, 8.0F};

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    SDL_RenderFillRectF(renderer, &rect);

    SDL_FPoint p1 = this->points.back();
    p1.x *= scale;
    p1.y *= scale;
    p1.x += c.x;
    p1.y += c.y;

    if (selected)
        SDL_SetRenderDrawColor(renderer, color.g, color.r, color.b, 255);
    else
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    for (auto& p : this->points) {
        SDL_FPoint p2 = {p.x * scale + c.x, p.y * scale + c.y};
        SDL_RenderDrawLineF(renderer, p1.x, p1.y, p2.x, p2.y);
        p1 = p2;
    }

    SDL_SetRenderDrawColor(renderer, color.b, color.r, color.g, 255);
    for (auto it = this->points.begin(); it != this->points.end(); it++) {
        p1 = {it->x * scale + c.x, it->y * scale + c.y};
        rect = {p1.x - 3.0F, p1.y - 3.0F, 6.0F, 6.0F};
        SDL_RenderFillRectF(renderer, &rect);
    }
}

void Wall::Reset() {
    float dx = 0, dy = 0, x = 0, y = 0;

    for (auto& p : this->points) {
        dx += p.x;
        dy += p.y;
    }
    dx /= this->points.size();
    dy /= this->points.size();
    this->center.x += dx;
    this->center.y += dy;

    this->most_bottom = this->most_left = this->most_right = this->most_top = 0.0F;

    for (auto& p : this->points) {
        p.x -= dx;
        p.y -= dy;

        if (p.x > this->most_right)
            this->most_right = p.x;
        else if (p.x < this->most_left)
            this->most_left = p.x;

        if (p.y > this->most_bottom)
            this->most_bottom = p.y;
        else if (p.y < this->most_top)
            this->most_top = p.y;
    }
}

std::list<SDL_FPoint>::iterator Wall::AddPoint(float x, float y) {
    x -= this->center.x;
    y -= this->center.y;
    auto* p1 = &this->points.back();
    for (auto p2 = this->points.begin(); p2 != this->points.end(); p2++) {
        {
            float max, min;

            if (p1->x > p2->x) {
                max = p1->x;
                min = p2->x;
            } else {
                min = p1->x;
                max = p2->x;
            }

            if (x < min - 2.0F || x > max + 2.0F) {
                p1 = &(*p2);
                continue;
            }

            if (p1->y > p2->y) {
                max = p1->y;
                min = p2->y;
            } else {
                min = p1->y;
                max = p2->y;
            }

            if (y < min - 2.0F || y > max + 2.0F) {
                p1 = &(*p2);
                continue;
            }
        }
        SDL_FPoint axis = {p1->y - p2->y, p2->x - p1->x};

        float d = (p1->x - x) * axis.x + (p1->y - y) * axis.y;
        d *= d;
        float d2 = axis.x * axis.x + axis.y * axis.y;
        if (d < 16 * d2) {
            this->points.insert(p2, {x, y});
            p2--;
            return p2;
        }

        p1 = &(*p2);
    }

    return this->points.end();
}

std::list<SDL_FPoint>::iterator Wall::SelectPointAt(float x, float y) {
    x -= this->center.x;
    y -= this->center.y;
    for (auto it = this->points.begin(); it != this->points.end(); it++) {
        float d = abs(it->x - x) + abs(it->y - y);
        if (d < 6.0F) {
            return it;
        }
    }
    return this->points.end();
}

void Wall::RotateBy(float da) {
    float rad = da * M_PI / 180;
    float cos_a = cosf(rad);
    float sin_a = sinf(rad);

    for (SDL_FPoint& p : this->points) p = {p.x * cos_a - p.y * sin_a, p.x * sin_a + p.y * cos_a};
}

void Wall::MovePointsBy(std::list<SDL_FPoint>::iterator it, float dx, float dy, int n) {
    auto end = points.end();
    for (int i = n; i > 0; i--, it++) {
        if (it == end) it = points.begin();
        it->x += dx;
        it->y += dy;
    }
}
