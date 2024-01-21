#ifndef __Objects__
#define __Objects__

#include<iostream>
#include<SDL_render.h>
#include<SDL_image.h>
#include<cmath>
#include<list>
#include<fstream>

float scale = 1.0, inverse_scale=1.0F;// scale of map, inverse_scale = 1/scale 
int windowWidth = 1200;
int windowHeight = 600;
int time_elapsed;

SDL_Renderer* main_ren;
SDL_FPoint viewpoint = { 0.0F,-windowHeight };


class Wall
{
private:
	SDL_FPoint center = { 0,0 };// coordinates of object in the play zone
	float most_right;//distance on x axis to farest point on the right 
	float most_left;//distance on x axis to farest point in the left , always negative
	float most_top;//distance on y axis to farest point on the top , always negative
	float most_bottom;//distance on y axis to farest point on the bottom
	/*----------------------------------------------------------------------- till here order of variables must not be chabged*/
	
	//static std::list<Wall>::iterator focusedObject;
	
	std::list<SDL_FPoint> points;// boundary points of polygon
	static std::list<SDL_FPoint>::iterator first_point;// first selected point
	static SDL_Color color;// color code of edges of all object

	//static bool objectSelected;// determines if any Wall selected
	//static bool pointSelected;// determines if any point of selected object is selected (if objectSelected=false==>pointSelected=false)
	static bool pointMoved;// determines if selected point is moved (if poinMoved then max_x and max_y might cahnge)
	static int	num_selected_obj;// number of selected items
	static int num_selected_point;// number of selected points in selected object
public:

	static const short data_size;// size of wall - sizeof points
	static std::list <Wall> walls;

	Wall() {};

	Wall(float x, float y)
	{
		this->center = { x,y };

		this->points.push_back({ -40.0F, -40.0F });
		this->points.push_back({ 40.0F, -40.0F });
		this->points.push_back({ 40.0F, 40.0F });
		this->points.push_back({ -40.0F, 40.0F });

		this->most_right=this->most_bottom = 40.0F;
		this->most_left=this->most_top = -40.0F;
	}

	inline bool InsideScreen()
	{
		if (this->center.x + this->most_right < viewpoint.x || this->center.x + this->most_left > viewpoint.x + windowWidth *inverse_scale)
			return false;

		if (this->center.y + this->most_bottom< viewpoint.y || this->center.y + this->most_top > viewpoint.y + windowHeight *inverse_scale)
			return false;

		return true;
	}

	// return true if given point is inside of Wall, for convex shapes only !!!
	/*
	bool ContainPoint(float x, float y)
	{
		x -= this->center.x;
		y -= this->center.y;

		if (x > this->most_right || x < this->most_left)
			return false;
		if (y > this->most_bottom || y < this->most_top)
			return false;

		SDL_FPoint* p1 = &this->points.back();

		for (auto p2 = this->points.begin(); p2 != this->points.end(); p2++)
		{
			SDL_FPoint axis = { p1->y - p2->y,p2->x - p1->x };

			float d1 = p1->x * axis.x + p1->y * axis.y;

			float d2 = (p1->x - x) * axis.x + (p1->y - y) * axis.y;

			if (d1 * d2 < 0)
				return false;

			p1 = &p2._Ptr->_Myval;
		}

		return true;
	}
	*/

	// return true if given point is inside of Wall, for any polygon !!!
	bool ContainPoint(float x, float y)
	{
		x -= this->center.x;
		y -= this->center.y;

		if (x > this->most_right || x < this->most_left)
			return false;
		if (y > this->most_bottom || y < this->most_top)
			return false;

		SDL_FPoint* p1 = &this->points.back();
		
		int num = 0; // number of intersections
		for (auto p2 = this->points.begin(); p2 != this->points.end(); p1 = &p2._Ptr->_Myval, p2++)
		{
			if ((y < p1->y && y < p2->y) || (y > p1->y && y > p2->y))
				continue;
			if (x < p1->x && x < p2->x)
				continue;

			if (x > p1->x && x > p2->x)
			{
				num++;
				continue;
			}

			float d = ((p1->x - x) * (p1->y - p2->y) + (p1->y - y) * (p2->x - p1->x)) * (p1->y - p2->y);

			if (d < 0)
				num++;
		}

		if (num & 1)// if number of intersections is odd then the point is inside of polygon
			return true;

		return false;
	}
	
	void Render()
	{
		if (!this->InsideScreen())
			return;

		SDL_FPoint c = { (this->center.x - viewpoint.x) * scale,(this->center.y - viewpoint.y) * scale };

		SDL_FRect rect = { c.x - 4.0F,c.y - 4.0F,8.0F,8.0F };
		
		SDL_SetRenderDrawColor(main_ren, color.r, color.g, color.b, 255);
		SDL_RenderFillRectF(main_ren, &rect);

		SDL_FPoint p1 = this->points.back();
		p1.x *= scale; p1.y *= scale;
		p1.x += c.x; p1.y += c.y;

		SDL_SetRenderDrawColor(main_ren, color.r, color.g, color.b, 255);
		for (auto it = this->points.begin(); it != this->points.end(); it++)
		{
			SDL_FPoint p2 = { it->x * scale + c.x,it->y * scale + c.y };
			SDL_RenderDrawLineF(main_ren, p1.x, p1.y, p2.x, p2.y);
			p1 = p2;
		}
		
		SDL_SetRenderDrawColor(main_ren, color.b, color.r, color.g, 255);
		for (auto it = this->points.begin(); it != this->points.end(); it++)
		{
			p1 = { it->x * scale + c.x,it->y * scale + c.y };
			rect = { p1.x - 3.0F,p1.y - 3.0F,6.0F,6.0F };
			SDL_RenderFillRectF(main_ren, &rect);
		}
	}

	void static ShowSelectedPoints()
	{
		if (!Wall::num_selected_point)
			return;

		SDL_SetRenderDrawColor(main_ren,255,0,0, 255);
		SDL_FPoint c = Wall::walls.front().center;
		
		c.x = (c.x - viewpoint.x) * scale;
		c.y = (c.y - viewpoint.y) * scale;

		SDL_FPoint p1;
		SDL_FRect rect;
		auto it = Wall::first_point;
		auto end = Wall::walls.front().points.end();
		for (int i = Wall::num_selected_point; i > 0; i--,it++)
		{
			if (it == end)
				it = Wall::walls.front().points.begin();

			p1 = { it->x * scale + c.x,it->y * scale + c.y };
			rect = { p1.x - 3.0F,p1.y - 3.0F,6.0F,6.0F };
			SDL_RenderFillRectF(main_ren, &rect);
		}
	}

	static void RenderAll()
	{
		auto it = Wall::walls.rbegin();
		Wall::color = { 100,255,100,255 };

		for (int i =Wall::walls.size() - num_selected_obj; i > 0; i--, it++)
			it->Render();

		Wall::color = { 178, 34, 34};
		while (it!=Wall::walls.rend())
		{
			it->Render();
			it++;
		}
		
		ShowSelectedPoints();
	}

	// adds point so Selected object, if object not selected returns false
	static bool AddPoint(float x, float y)
	{
		if (num_selected_obj==0)
			return false;
		Wall& obj = Wall::walls.front();

		x -= obj.center.x; y -= obj.center.y;
		auto* p1 = &obj.points.back();
		for (auto p2 = obj.points.begin(); p2 != obj.points.end(); p2++)
		{
			{
				float max, min;
				
				if (p1->x > p2->x)
				{
					max = p1->x;
					min = p2->x;
				}
				else
				{
					min = p1->x;
					max = p2->x;
				}

				if (x<min-2.0F || x>max+2.0F)
				{
					p1 = &p2._Ptr->_Myval;
					continue;
				}

				if (p1->y > p2->y)
				{
					max = p1->y;
					min = p2->y;
				}
				else
				{
					min = p1->y;
					max = p2->y;
				}

				if (y<min-2.0F || y>max+2.0F)
				{
					p1 = &p2._Ptr->_Myval;
					continue;
				}
			}
			SDL_FPoint axis = { p1->y - p2->y,p2->x - p1->x };

			float d = (p1->x - x) * axis.x + (p1->y - y) * axis.y;
			d *= d;
			float d2 = axis.x * axis.x + axis.y * axis.y;
			if (d < 16 * d2)
			{
				obj.points.insert(p2, { x, y });
				Wall::first_point = p2.operator--();
				Wall::num_selected_point = 1;
				return true;
			}

			p1 = &p2._Ptr->_Myval;
		}

		Wall::num_selected_point = 0;
		return false;
	}

	//Selects a single object at position (x,y), If there is no object ,returns false
	static bool SelectObjectAt(float x, float y)
	{
		for (auto it = Wall::walls.begin(); it != Wall::walls.end(); it++)
		{
			if (it->ContainPoint(x, y))
			{
				
				Wall::walls.splice(Wall::walls.begin(), Wall::walls, it);// selected object moved to the last potion
				Wall::num_selected_obj = 1;// so if num_selected_obj == 1 , last object is selected
				Wall::num_selected_point = 0;
				return true;
			}
		}

		Wall::num_selected_obj = 0;
		Wall::num_selected_point = 0;
		
		Wall::pointMoved = false;
		return false;
	}
	
	//Selects a single point at (x,y) from selected object, if object not selected or there is no point at (x,y) returns false
	static bool SelectPointAt(float x, float y)
	{
		if (Wall::num_selected_obj == 0)
			return false;

		Wall& obj = Wall::walls.front();
		x -= obj.center.x; y -= obj.center.y;
		for (auto it = obj.points.begin(); it != obj.points.end(); it++)
		{
			float d = abs(it->x - x) + abs(it->y - y);
			if (d < 6.0F)
			{
				Wall::first_point = it;
				Wall::num_selected_point = 1;
				return true;
			}
		}
		Wall::num_selected_point = 0;
		Wall::pointMoved = false;
		return false;
	}

	// Selects objects in given rect relative to window
	static void SelectObjectsInRect(SDL_FRect rect/*unscaled rect*/)
	{
		if (rect.w<0)
		{
			rect.x += rect.w;
			rect.w =-rect.w;
		}

		if (rect.h < 0)
		{
			rect.y += rect.h;
			rect.h = - rect.h;
		}

		rect.w += rect.x;
		rect.h += rect.y;

		rect.x = rect.x * inverse_scale + viewpoint.x;// changin boundries relative to map scale
		rect.y = rect.y * inverse_scale + viewpoint.y;
		rect.w = rect.w * inverse_scale + viewpoint.x;
		rect.h = rect.h * inverse_scale + viewpoint.y;

		Wall::num_selected_obj = 0;
		Wall::num_selected_point = 0;

		std::list<Wall>::iterator current = Wall::walls.begin();
		
		for (int i = Wall::walls.size(); i > 0; i--)
		{
			if (current->center.x<rect.x || current->center.x>rect.w||current->center.y<rect.y||current->center.y>rect.h)
			{
				current++;
				continue;
			}

			auto temp = current;
			current++;
			Wall::walls.splice(Wall::walls.begin(), Wall::walls, temp);
			Wall::num_selected_obj++;
		}
	}

	// Selects points of a single selected object
	static void SelectPointsInRect(SDL_FRect rect)
	{
		if (!(Wall::num_selected_obj == 1))
			return;

		Wall& obj = Wall::walls.front();

		if (rect.w < 0)
		{
			rect.x += rect.w;
			rect.w = -rect.w;
		}

		if (rect.h < 0)
		{
			rect.y += rect.h;
			rect.h = -rect.h;
		}

		rect.w += rect.x;
		rect.h += rect.y;

		rect.x = rect.x * inverse_scale + viewpoint.x-obj.center.x;// changin boundries relative to object
		rect.y = rect.y * inverse_scale + viewpoint.y-obj.center.y;
		rect.w = rect.w * inverse_scale + viewpoint.x-obj.center.x;
		rect.h = rect.h * inverse_scale + viewpoint.y-obj.center.y;
		
		if (obj.most_right<rect.x || obj.most_left>rect.w || obj.most_bottom<rect.y || obj.most_top>rect.h)
			return;
		Wall::num_selected_point = 0;

		auto it = obj.points.begin();
		bool gap = true;
		
		for (int i=obj.points.size();i>0;i--,it++)
		{
			if (it->x<rect.x || it->x>rect.w || it->y<rect.y || it->y>rect.h)
			{
				gap = true;
				continue;
			}

			if (gap)
			{
				Wall::first_point = it;
				gap = false;
			}

			Wall::num_selected_point++;
		}
	}

	// removes selected objects
	inline static void DestroySelectedObjects()
	{
		if (Wall::num_selected_obj==0)
			return;

		std::list<Wall>::iterator it;
		std::list<Wall>::iterator next = Wall::walls.begin();
		
		for (int i = Wall::num_selected_obj; i > 0; i--)
		{
			it = next;
			next++;
			Wall::walls.erase(it);
		}

		Wall::num_selected_point = 0;
		Wall::num_selected_obj = 0;
		Wall::pointMoved = false;
	}
	
	inline static void DestroySelectedPoints()// destroyes selected point in selected object , if no point is selected returns false
	{
		if (Wall::num_selected_obj == 0||Wall::num_selected_point==0)
			return;

		if (Wall::walls.front().points.size() < 4)// at least polygon must have three vertexes 
			return;


		auto next = Wall::first_point;
		auto end = Wall::walls.front().points.end();
		auto it = next;
		auto& selected_points = Wall::walls.front().points;
		for (int i = Wall::num_selected_point; i > 0; i--)
		{
			if (next == end)
				next = Wall::walls.front().points.begin();
			it = next;
			next++;
			selected_points.erase(it);
		}

		Wall::pointMoved = true;
		Wall::num_selected_point = 0;

		Wall::ResetSelectedObject();
	}
	
	inline static void MoveSelectedObjectsBy(float dx, float dy)
	{
		if (Wall::num_selected_obj==0)
			return;

		std::list<Wall>::iterator it = Wall::walls.begin();
		for (int i = Wall::num_selected_obj; i > 0; i--, it++)
		{
			it->center.x += dx;
			it->center.y += dy;
		}
	}
	
	inline static void MoveSelectedPointsBy(float dx, float dy)
	{
		if (!Wall::num_selected_point)
			return;
		
		auto it = Wall::first_point;
		auto end = Wall::walls.front().points.end();
		auto& selected_points = Wall::walls.front().points;
		for (int i = Wall::num_selected_point; i > 0; i--, it++)
		{
			if (it == end)
				it = Wall::walls.front().points.begin();
			
			it->x += dx;
			it->y += dy;
		}

		Wall::pointMoved = true;
	}

	inline static void ResetSelectedObject()
	{
		if (!Wall::num_selected_obj || !pointMoved)
			return;

		Wall& obj = Wall::walls.front();
		float dx = 0, dy = 0,x=0,y=0;
		
		for (auto& p: obj.points) 
		{
			dx += p.x;
			dy += p.y;
		}
		dx /= obj.points.size();
		dy /= obj.points.size();
		obj.center.x += dx;
		obj.center.y += dy;

		obj.most_bottom = obj.most_left = obj.most_right = obj.most_top = 0.0F;

		for (auto& p:obj.points)
		{
			p.x -= dx;
			p.y -= dy;

			if (p.x>obj.most_right)
				obj.most_right= p.x;
			else
				if (p.x < obj.most_left)
					obj.most_left = p.x;

			if (p.y > obj.most_bottom)
				obj.most_bottom = p.y;
			else
				if (p.y < obj.most_top)
					obj.most_top = p.y;
		}

		Wall::pointMoved = false;
	}
	
	inline static void CreateObjectAt(float x, float y)
	{
		Wall::walls.emplace_front(x, y);
		Wall::num_selected_obj = 1;
		Wall::num_selected_point = 0;
	}

	inline static bool IsPointSelected()
	{
		return Wall::num_selected_point;
	}
	inline static bool IsObjectSelected()
	{
		return Wall::num_selected_obj;
	}

	static void RotateSelectedObjectBy(float da)
	{
		if (!da||!Wall::num_selected_obj)
			return;

		float rad = da * M_PI / 180;
		float cos_a = cosf(rad);
		float sin_a = sinf(rad);
		Wall& obj = Wall::walls.front();

		for (SDL_FPoint& p: obj.points)
			p = { p.x * cos_a - p.y * sin_a,p.x * sin_a + p.y * cos_a };

	}
	void SaveToFile(std::ofstream& file,int position=-1)
	{
		if (position > 0)//if position given -1 then it will writo to posion where "write poiter" loacated of file
			file.seekp(position);

		file.write((char*)this, Wall::data_size);

		int num_points = this->points.size();
		file.write((char*)&num_points, sizeof(int));
		
		for (SDL_FPoint& p : this->points)
		{
			file.write((char*)&p, sizeof(SDL_FPoint));
		}
	}

	void ReadFromFile(std::ifstream& file, int position = -1)
	{
		if (position > 0)//if position given -1 then it will writo to posion where "write poiter" loacated of file
			file.seekg(position);
		
		file.read((char*)this, Wall::data_size);

		int num_points;
		file.read((char*)&num_points,sizeof(int));
		this->points.clear();

		SDL_FPoint p;
		while (num_points-->0)
		{	
			file.read((char*)&p.x, sizeof(float));
			file.read((char*)&p.y, sizeof(float));
			this->points.push_back(p);
		}
	}

	friend void SaveWallsToFile();
	friend void LoadWallsFromFile();

};
const short Wall::data_size = sizeof(SDL_FPoint) + 4 * sizeof(float);
//bool Wall::pointSelected = false;
bool Wall::pointMoved = false;
int Wall::num_selected_obj = 0;
int Wall::num_selected_point = 0;
SDL_Color Wall::color = { 100,255,100,255 };
std::list<Wall> Wall::walls;
std::list<SDL_FPoint>::iterator Wall::first_point;
//std::list<Wall>::iterator Wall::focusedObject;

void LoadWallsFromFile(const char* file_name)
{
	std::ifstream file(file_name, std::ios::binary);
	if (!file.is_open())
	{
		std::cout << " File not found!" << std::endl;
		return;
	}
	int num_obj;
	file.read((char*)&num_obj, sizeof(int));
	while (num_obj-->0)
	{
		Wall::walls.emplace_back();
		Wall::walls.back().ReadFromFile(file);
	}
	file.close();
}

void SaveWallsToFile(const char *file_name)
{
	if (Wall::walls.size() == 0)
		return;
	
	std::ofstream file(file_name,std::ios::binary|std::ios::trunc);
	file.seekp(0, std::ios::beg);

	int num_objects = Wall::walls.size();
	file.write((char*)&num_objects, sizeof(int));

	for (Wall& i:Wall::walls)
	{
		i.SaveToFile(file);
	}

	file.close();
}

#endif // !__Objects__