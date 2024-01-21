#ifndef __Objects__
#define __Objects__

#include<iostream>
#include<SDL_render.h>
#include<SDL_image.h>
#include<cmath>
#include<vector>
#include<list>

float scale = 1.0;
int windowWidth = 1200;
int windowHeight = 600;
int time_elapsed;

SDL_Renderer* main_ren;
SDL_FPoint viewpoint = { 0.0F,-windowHeight};


class Wall
{
private:
	SDL_FPoint center = { 0,0 };// coordinates of object in the play zone
	std::vector<SDL_FPoint> points;// boundary points of polygon
	int num_points = 0;// number of boundary points
	float radious = 0;// distance to farest points2 from center
	static std::list<Wall>::iterator focusedObject;
	static bool objectSelected;
public:
	static std::list <Wall> walls;

	bool ContainPoint(float x, float y)
	{
		x -= this->center.x;
		y -= this->center.y;
		{
			/*
				if object far enough
				we initially suppose both object as circles then check for collision

				if discantec between circles > sum of their radiouses then they are not colliding
				thus both objects are not collidiong

			*/

			float d = x * x + y * y;
			float q = this->radious;
			q *= q;

			if (d > q)
				return false;
		}

		SDL_FPoint* p1 = &this->points.back();
		auto p2 = this->points.begin();

		for(auto p2=this->points.begin();p2!=this->points.end();p2++)
		{
			SDL_FPoint axis = { p1->y - p2->y,p2->x - p1->x };

			float d1 = p1->x * axis.x + p1->y * axis.y;
			
			float d2 = (p1->x - x) * axis.x + (p1->y - y) * axis.y;

			if (d1 * d2 < 0)
				return false;

			p1 = p2._Ptr;
		}

		return true;
	}

	Wall() {};

	Wall(float x, float y)
	{
		this->center = { x,y };
		this->num_points = 4;

		this->points.push_back({ -40.0F, -40.0F });
		this->points.push_back({ 40.0F, -40.0F });
		this->points.push_back({40.0F, 40.0F});
		this->points.push_back({ -40.0F, 40.0F });
		
		this->radious =56.57F;
	}

	inline bool InsideScreen()
	{
		if (this->center.x + this->radious < viewpoint.x || this->center.x - this->radious > viewpoint.x + windowWidth/scale)
			return false;

		if (this->center.y + this->radious < viewpoint.y || this->center.y - this->radious > viewpoint.y + windowHeight/scale)
			return false;

		return true;
	}

	void Render()
	{
		if (!this->InsideScreen())
			return;

		SDL_Color color;
		if (this == &Wall::focusedObject._Ptr->_Myval)
			color = { 255, 100, 100 ,255};//color code of firebrick
		else
			color = { 100,255,100,255 };//color code of coral

		SDL_FPoint c = {(this->center.x - viewpoint.x)*scale,(this->center.y - viewpoint.y)*scale};
		
		SDL_FRect rect = { c.x - 4.0F,c.y - 4.0F,8.0F,8.0F };
		SDL_SetRenderDrawColor(main_ren, color.r, color.g, color.b,255);
		SDL_RenderFillRectF(main_ren, &rect);

		for (int i=0; i<this->num_points;i++)
		{
			SDL_FPoint p1 = this->points[i];
			p1.x *= scale; p1.y *= scale;
			SDL_FPoint& p2 = this->points[(i + 1) % this->num_points];

			SDL_SetRenderDrawColor(main_ren, color.r,color.g,color.b,255);
			SDL_RenderDrawLineF(main_ren, p1.x+c.x, p1.y+c.y, p2.x*scale+c.x, p2.y*scale+c.y);
		
			rect = { p1.x + c.x - 3.0F,p1.y + c.y - 3.0F,6.0F,6.0F };
			SDL_SetRenderDrawColor(main_ren, color.b, color.r, color.g, 255);
			SDL_RenderFillRectF(main_ren, &rect);
		}


	}

	static void SelectObjectAtPoint(float x, float y)
	{

		for (auto it = Wall::walls.begin(); it != Wall::walls.end(); it++)
		{
			if (it->ContainPoint(x, y))
			{
				Wall::focusedObject = it;
				Wall::objectSelected = true;
				return;
			}
		}

		Wall::focusedObject = Wall::walls.end();
		Wall::objectSelected = false;
	}
	
	inline static void DestroySelectedObject()
	{
		if (Wall::objectSelected)
			Wall::walls.erase(Wall::focusedObject);

		Wall::objectSelected = false;
	}

	inline static void CreateObjectAt(float x, float y)
	{
		Wall::walls.emplace_back(x, y);
		Wall::focusedObject = Wall::walls.end();
		Wall::focusedObject--;
		Wall::objectSelected = true;
	}
};
bool Wall::objectSelected = false;
std::list<Wall> Wall::walls;
std::list<Wall>::iterator Wall::focusedObject;

#endif // !__Objects__
