#include <cstdio>
#include <iostream>

#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <SDL2_rotozoom.h>
#include <SDL_image.h>

extern "C" {
#include "e8910.h"
#include "vecx.h"
}


static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;
static SDL_Texture* texture = nullptr;
static SDL_Surface* screen = nullptr;
static SDL_Surface* overlay_orginal_surface = nullptr;
static SDL_Surface* overlay_scaled_surface = nullptr;

static long scl_factor;
static long offx;
static long offy;

static char* cartfilename = nullptr;


extern "C" {
// referenced from vecx
void osint_render()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    if (overlay_scaled_surface != nullptr) {
        auto _offx = static_cast<int>(offx);
        auto _offy = static_cast<int>(offy);

        SDL_Rect dest_rect = {_offx, _offy, 0, 0};
        SDL_BlitSurface(overlay_scaled_surface, nullptr, screen, &dest_rect);
    }
    SDL_UpdateTexture(texture, nullptr, screen->pixels, screen->pitch);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    int v = 0;
    for (v = 0; v < vector_draw_cnt; v++) {
        Uint8 c = vectors_draw[v].color * 256 / VECTREX_COLORS;
        auto x1 = static_cast<Sint16>(offx + vectors_draw[v].x0 / scl_factor);
        auto y1 = static_cast<Sint16>(offy + vectors_draw[v].y0 / scl_factor);
        auto x2 = static_cast<Sint16>(offx + vectors_draw[v].x1 / scl_factor);
        auto y2 = static_cast<Sint16>(offy + vectors_draw[v].y1 / scl_factor);
        aalineRGBA(renderer, x1, y1, x2, y2, c, c, c, 0xff);
    }
    SDL_RenderPresent(renderer);
    // SDL_Flip(screen);
}
}


static void init()
{
    FILE* f = nullptr;
    const char* romfilename = getenv("VECTREX_ROM");
    if (romfilename == nullptr) {
        romfilename = "rom.dat";
    }
    if (nullptr == (f = fopen(romfilename, "rb"))) {
        std::perror(romfilename);
        exit(EXIT_FAILURE);
    }
    if (fread(rom, 1, sizeof(rom), f) != sizeof(rom)) {
        std::cerr << "Invalid rom length" << std::endl;
        exit(EXIT_FAILURE);
    }
    fclose(f);

    memset(cart, 0, sizeof(cart));
    if (cartfilename != nullptr) {
        FILE* f = nullptr;
        if (nullptr == (f = fopen(cartfilename, "rb"))) {
            std::perror(cartfilename);
            exit(EXIT_FAILURE);
        }
        fread(cart, 1, sizeof(cart), f);
        fclose(f);
    }
}


void screen_init(int screenx, int screeny)
{
    window = SDL_CreateWindow("vecxl", SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED, screenx, screeny,
                              SDL_SWSURFACE);
    renderer = SDL_CreateRenderer(window, -1, 0);
    screen = SDL_CreateRGBSurface(0, screenx, screeny, 32, 0x00FF0000,
                                  0x0000FF00, 0x000000FF, 0xFF000000);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STREAMING, screenx, screeny);

    int sclx = ALG_MAX_X / screen->w;
    int scly = ALG_MAX_Y / screen->h;

    scl_factor = sclx > scly ? sclx : scly;

    offx = (screenx - ALG_MAX_X / scl_factor) / 2;
    offy = (screeny - ALG_MAX_Y / scl_factor) / 2;

    if (overlay_orginal_surface != nullptr) {
        if (overlay_scaled_surface != nullptr) {
            SDL_FreeSurface(overlay_scaled_surface);
        }

        double zoom =
            (static_cast<double>(ALG_MAX_X) / static_cast<double>(scl_factor)) /
            static_cast<double>(overlay_orginal_surface->w);
        overlay_scaled_surface =
            zoomSurface(overlay_orginal_surface, zoom, zoom, 0);
    }
}


static void readevents()
{
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        switch (e.type) {
        case SDL_QUIT:
            exit(EXIT_SUCCESS);
            break;
        case SDL_KEYDOWN:
            switch (e.key.keysym.sym) {
            case SDLK_ESCAPE:
                exit(EXIT_SUCCESS);
            case SDLK_a:
                snd_regs[14] &= ~0x01;
                break;
            case SDLK_s:
                snd_regs[14] &= ~0x02;
                break;
            case SDLK_d:
                snd_regs[14] &= ~0x04;
                break;
            case SDLK_f:
                snd_regs[14] &= ~0x08;
                break;
            case SDLK_LEFT:
                alg_jch0 = 0x00;
                break;
            case SDLK_RIGHT:
                alg_jch0 = 0xff;
                break;
            case SDLK_UP:
                alg_jch1 = 0xff;
                break;
            case SDLK_DOWN:
                alg_jch1 = 0x00;
                break;
            default:
                break;
            }
            break;
        case SDL_KEYUP:
            switch (e.key.keysym.sym) {
            case SDLK_a:
                snd_regs[14] |= 0x01;
                break;
            case SDLK_s:
                snd_regs[14] |= 0x02;
                break;
            case SDLK_d:
                snd_regs[14] |= 0x04;
                break;
            case SDLK_f:
                snd_regs[14] |= 0x08;
                break;
            case SDLK_LEFT:
                alg_jch0 = 0x80;
                break;
            case SDLK_RIGHT:
                alg_jch0 = 0x80;
                break;
            case SDLK_UP:
                alg_jch1 = 0x80;
                break;
            case SDLK_DOWN:
                alg_jch1 = 0x80;
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
}


void osint_emuloop()
{
    constexpr Uint32 EMU_TIMER =
        20; /* the emulators heart beats at 20 milliseconds */

    auto next_time = SDL_GetTicks() + EMU_TIMER;
    vecx_reset();
    for (;;) {
        vecx_emu((VECTREX_MHZ / 1000) * EMU_TIMER);
        readevents();

        {
            Uint32 now = SDL_GetTicks();
            if (now < next_time) {
                SDL_Delay(next_time - now);
            }
            else {
                next_time = now;
            }

            next_time += EMU_TIMER;
        }
    }
}


void load_overlay(const char* filename)
{
    SDL_Surface* image = IMG_Load(filename);
    if (image != nullptr) {
        overlay_orginal_surface = image;
        std::cout << "Loaded overlay image from file=" << filename << std::endl;
    }
    else {
        std::cerr << "IMG_Load: " << IMG_GetError() << std::endl;
    }
}


int main(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError()
                  << std::endl;
        exit(-1);
    }

    if (argc > 1) {
        cartfilename = argv[1];
    }

    if (argc > 2) {
        load_overlay(argv[2]);
    }

    screen_init(330 * 3 / 2, 410 * 3 / 2);
    init();
    e8910_init_sound();
    osint_emuloop();
    e8910_done_sound();
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
