#include <iostream>

#include "SDL.h"

// (int argc, char *argv[])
int main()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_CreateWindowAndRenderer(320, 240, SDL_WINDOW_SHOWN, &window, &renderer);

    SDL_Joystick* joy = SDL_JoystickOpen(0);
    if (joy == nullptr) {
        exit(1);
    }

    std::cout << "Name: " << SDL_JoystickNameForIndex(0) << std::endl;

    SDL_Event event;
    while (true) {
        SDL_WaitEvent(&event);

        if (event.type == SDL_QUIT) {
            break;
        }

        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                break;
            }
        }
        else if (event.type == SDL_JOYAXISMOTION) {
            std::cout << "axis: " << event.jaxis.axis << " "
                      << event.jaxis.value << std::endl;
        }
        else if (event.type == SDL_JOYBUTTONDOWN) {
            std::cout << "button: " << event.jbutton.button << std::endl;
        }
    }

    SDL_JoystickClose(joy);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
