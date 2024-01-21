#include<iostream>
#include<SDL_render.h>
#include"Objects_2.h"
#include<string>
#include<SDL_keyboard.h>

#include"Chrono_Time.h"


int delay = 20;

float horizontal = 0,vertical=0;// move map horizobtally and verstically
float move_selected_X = 0, move_selected_Y = 0;// move selected object on x - y - axis
float rotate_selected = 0;// rotate selected object
short scaling = 0;// zooming in or out 

char select_mode=0;// if select_mode = 1 then objects are being selected, else points are being selected 
SDL_FRect selecting_rect;// selectiong zone 

void MoveMap()// moves map up or down
{
	if (scaling)
	{
		if ((scale < 0.5 && scaling < 0)|| (scale > 3 && scaling > 0))
		{
			scaling = 0;
			return;
		}

		float dS = scaling * 0.01F;
		/* oreder of calculations is important*/
		scale += dS;
		float centrize = (inverse_scale - 1 /scale) * 0.5F;
		inverse_scale = 1 / scale;

		viewpoint.x += windowWidth * centrize;
		viewpoint.y += windowHeight * centrize;

	}

	if (vertical)
		viewpoint.y+= vertical * time_elapsed;

	if (horizontal)
		viewpoint.x += horizontal * time_elapsed;

	if (move_selected_X || move_selected_Y)
	{
		if (Wall::IsPointSelected())
		{
			Wall::MoveSelectedPointsBy(move_selected_X , move_selected_Y );
		}
		else
			Wall::MoveSelectedObjectsBy(move_selected_X, move_selected_Y);
	}
	if (rotate_selected)
		{
			Wall::RotateSelectedObjectBy(rotate_selected);
		};
}

int main(int argc, char* args[])
{

	SDL_Window* win = SDL_CreateWindow("Map editor", 0, 20, windowWidth, windowHeight, SDL_WINDOW_SHOWN);
	main_ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	bool running = true;

	int old_key = 0;
	auto keydown = [&](int key)
	{
		if (key == old_key)
			return;

		old_key = key;
		switch (key)
		{
		case 26: ::vertical = -0.3F; break;//key "W"
		case 22:
			if (SDL_GetModState() == 64)// if left ctrl is pressed with "S" then saving to file will be carried uot
			{
				std::cout << " Enter name of file to be saved: ";
				std::string file_name;
				std::cin >> file_name;
				SaveWallsToFile(file_name.c_str());
				break;
			}
			::vertical = 0.3F; break;// key "S"
		case 7: ::horizontal = 0.3F; break;//key "D"
		case 4: ::horizontal = -0.3F; break;//key "A"
		case 87:
			
			if (SDL_GetModState() == 64)// if Left Ctrl is not pressed then zooming will not work
				::scaling = 1;
			break;//key "+"

		case 86:
			
			if (SDL_GetModState() == 64)// if L Ctrl is not pressed then zooming will not work
				::scaling = -1; 
			break;//key "-"
		
		case 93: //key on numpad "5"
		{
			scaling = 0;
			inverse_scale = 1;
			float centrize = (inverse_scale - 1) * 0.5F;

			viewpoint.x += windowWidth * centrize;
			viewpoint.y += windowHeight * centrize;
			scale = 1;
		}
			break;

		case 76:
			if (Wall::IsPointSelected())
				Wall::DestroySelectedPoints();
			else
				Wall::DestroySelectedObjects();

			break;

		case 15:
			if (SDL_GetModState() == 64)
			{
				std::cout << " Enter name of file to be loaded: ";
				std::string file_name;
				std::cin >> file_name;
				LoadWallsFromFile(file_name.c_str());
			}
			break;

		case 82:// up arrow key
			move_selected_Y = -2.0F;
			break;

		case 81:// down arrow key
			move_selected_Y = 2.0F;
			break;

		case 79:// right arrow key , if  L Alt is pressed then it rotates selected object to right 
			if (SDL_GetModState() == 256)
			{
				::rotate_selected = 2.0F;
				break;
			}
			move_selected_X = 2.0F;
			break;

		case 80:// left arrow key, if L Alt is pressed then it rotates selected object to left
			if (SDL_GetModState() == 256)
			{
				::rotate_selected = -2.0F;
				break;
			}
			move_selected_X = -2.0F;
			break;
			// up: 82 ,down 81, right 79,left 80
		}

		std::cout << key<<"\t State mode: "<< SDL_GetModState()<< std::endl;
	};
	auto keyup = [&](int key)
	{
		old_key = 0;
		switch (key)
		{
		case 26: ::vertical =NULL; break;// "A" button
		case 22: ::vertical = NULL; break;// "D" button
		case 7:  ::horizontal = NULL; break;//"W" button
		case 4:  ::horizontal = NULL; break;// "S" button
		case 87: ::scaling = NULL; break;// "+" sign
		case 86: ::scaling = NULL; break;// "-" sgin 
		case 224: scaling = NULL; break;// L Ctrl
		
		case 82:// up arrow key
			move_selected_Y = NULL;
			if (!move_selected_X)Wall::ResetSelectedObject();
			break;
		case 81:// down arrow key
			move_selected_Y = NULL;	
			if(!move_selected_X)Wall::ResetSelectedObject(); 
			break;
		case 79:// reight arrow key
			rotate_selected = NULL;
			move_selected_X = NULL;
			if (!move_selected_Y)Wall::ResetSelectedObject();
			break;
		case 80:// left arrow key
			rotate_selected = NULL;
			move_selected_X =NULL;
			if (!move_selected_Y)Wall::ResetSelectedObject();
			break;
		}
	};
	
	float initialX = 0, initilaY = 0;// to scroll using mouse wheel down!!!
	auto mousemove = [&](float x, float y,short mb)
	{
		if (SDL_GetModState() == 256 && select_mode)
		{
			selecting_rect.w = x - selecting_rect.x;
			selecting_rect.h = y - selecting_rect.y;
			return;
		}

		if (mb == 1)
		{
			Wall::MoveSelectedObjectsBy((x-initialX)*inverse_scale, (y-initilaY)*inverse_scale);
			initialX = x;
			initilaY = y;
		}
		if (mb == 2)
		{
			horizontal = (x - initialX)*0.005F;
			vertical = (y - initilaY)*0.005F;
		}
		if (mb == 4)
		{
			Wall::MoveSelectedPointsBy((x - initialX) * inverse_scale, (y - initilaY) * inverse_scale);
			initialX = x;
			initilaY = y;
		}
	};

	auto mousedown = [&](float x, float y,char mb)// mb is mouse button
	{
		std::cout << " mouse button: " << (short)mb << std::endl;
		auto* keystate = SDL_GetKeyboardState(NULL);
		
		initialX = x; initilaY = y;

		x = x * inverse_scale + viewpoint.x;// changing value of x relative to map and scale
		y = y * inverse_scale + viewpoint.y;// changing value of y relative to map and scale

		switch (mb)
		{
		case 1:// left mouse button
			if(keystate[44])
			{
				Wall::CreateObjectAt(x, y);
				break;
			}

			if (SDL_GetModState() == 256)
			{
				selecting_rect.x = initialX;
				selecting_rect.y = initilaY;
				select_mode = 1;
				break;
			}

			Wall::SelectObjectAt(x, y);
		break;
		
		case 3:// right mouse button
			if (keystate[44])
			{
				Wall::AddPoint(x, y);
				break;
			}
			
			if (SDL_GetModState() == 256)
			{
				selecting_rect.x = initialX;
				selecting_rect.y = initilaY;
				select_mode = 2;
				break;
			}

			Wall::SelectPointAt(x, y);
			break;
		}
	};
	auto mouseup = [&](int x, int y, char mb)
	{
		if (mb == 1)
		{
			if (select_mode == 1)
			{
				Wall::SelectObjectsInRect(selecting_rect);
				select_mode = 0;
				std::printf("Rect: {%f, %f, %f, %f}", selecting_rect.x, selecting_rect.y, selecting_rect.w, selecting_rect.h);
				selecting_rect.w = selecting_rect.h = 0.0F;
				return;
			}
			return;
		}
		if (mb == 2)
			horizontal = vertical = 0;

		if (mb == 3)
		{
			if (select_mode==2)
			{
				Wall::SelectPointsInRect(selecting_rect);
				select_mode = 0;
				selecting_rect.w = selecting_rect.h = 0.0F;
				return;
			}
			
			Wall::ResetSelectedObject();
		}
	};

	auto* keys = SDL_GetKeyboardState;
	SDL_Event event;
	int temp = 9898;
	while (running)
	{
		int start = SDL_GetTicks();
		// event handling
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				running = false;
				break;

			case SDL_KEYDOWN:
				keydown(event.key.keysym.scancode);
				break;

			case SDL_KEYUP:
				keyup(event.key.keysym.scancode);
				break;

			case SDL_MOUSEMOTION:
				mousemove(event.button.x, event.button.y, event.button.button);
				break;
			case SDL_MOUSEBUTTONDOWN:
				mousedown(event.button.x, event.button.y,event.button.button);
				break;
				
			case SDL_MOUSEBUTTONUP:
				mouseup(event.button.x, event.button.y, event.button.button);
				break;

			case SDL_MOUSEWHEEL:
				if (SDL_GetModState() != 64)// if L Ctrl is not pressed then zooming will not work
					break;
				if (event.wheel.y > 0)
					scaling = 1;
				else
					scaling=-1;
				break;

			}
		}
		
		SDL_SetRenderDrawColor(main_ren, 0, 0, 0, 255);
		SDL_RenderClear(main_ren);
		
		if (select_mode == 1)
		{
			SDL_SetRenderDrawColor(main_ren, 100, 255, 100, 255);
			SDL_RenderDrawRectF(main_ren, &selecting_rect);
		}

		if (select_mode == 2)
		{
			SDL_SetRenderDrawColor(main_ren, 100, 100, 255, 255);
			SDL_RenderDrawRectF(main_ren, &selecting_rect);
		}

		Wall::RenderAll();

		MoveMap();
		SDL_RenderPresent(main_ren);
		//stricting frame rate to 40 fps
		{
			time_elapsed = SDL_GetTicks() - start;

			std::string name = "dt: " + std::to_string(time_elapsed) + "; objects: " + std::to_string(Wall::walls.size());
			SDL_SetWindowTitle(win, name.c_str());
			if (time_elapsed < ::delay)
			{
				SDL_Delay(::delay - time_elapsed);
				time_elapsed = ::delay;
			}

		}
	}

	SDL_DestroyRenderer(main_ren);
	SDL_DestroyWindow(win);
	return 0;
}