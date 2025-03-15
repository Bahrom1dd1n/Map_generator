#include <SDL2/SDL_video.h>

#include "Map.h"
#include "Wall.h"

int main(int argc, char *argv[]) {
    Map map(500, 500, SDL_WINDOW_FULLSCREEN_DESKTOP);
    map.Start();
    return 0;
}
