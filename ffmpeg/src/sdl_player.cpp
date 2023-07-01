/**
 * compile
 * g++ sdl_player.cpp -o D:\soft\mingw\mingw64\msys\home\Administrator\workdir\streaming-practice\ffmpeg\compiled\sdlPlayer -ID:\soft\mingw\mingw64\msys\home\Administrator\workdir\streaming-practice\ffmpeg\include\SDL2 -LD:\soft\mingw\mingw64\msys\home\bin -lSDL2
 */

#include <iostream>
extern "C"
{
#include "SDL.h"
};
#include <Windows.h>

const int bpp = 12;

int screen_w = 1280, screen_h = 800;
const int pixel_w = 640, pixel_h = 272;

// YUV420,在一个像素点中，Y占8位，UV占4位，总共占12位，一个字节8位。
// 计算一帧图片所需的总大小
unsigned char buffer[pixel_w * pixel_h * bpp / 8];

#define REFRESH_EVENT (SDL_USEREVENT + 1)

#define BREAK_EVENT (SDL_USEREVENT + 2)

int thread_exit = 0;

int refresh_video(void *opaque)
{
    thread_exit = 0;
    while (thread_exit == 0)
    {
        std::cout << "thread_exit: " << thread_exit << std::endl;
        SDL_Event event;
        event.type = REFRESH_EVENT;
        SDL_PushEvent(&event);
        SDL_Delay(40);
    }
    std::cout << "thread_exit: " << thread_exit << std::endl;
    thread_exit = 0;
    SDL_Event event;
    event.type = BREAK_EVENT;
    SDL_PushEvent(&event);

    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    using namespace std;
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        cout << "Could not initialize SDL -" << SDL_GetError() << "\n"
             << endl;
        return -1;
    }

    SDL_Window *screen;

    screen = SDL_CreateWindow("Simplest Viode Play SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              screen_w, screen_h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!screen)
    {
        cout << "SDL: Could not create window - exiting: " << SDL_GetError() << "\n"
             << endl;
        return -1;
    }
    SDL_Renderer *sdlRenderer = SDL_CreateRenderer(screen, -1, 0);

    Uint32 pixformat = 0;

    pixformat = SDL_PIXELFORMAT_IYUV;
    SDL_Texture *sdlTexture = SDL_CreateTexture(sdlRenderer, pixformat, SDL_TEXTUREACCESS_STREAMING, pixel_w, pixel_h);
    FILE *fp = NULL;
    fp = fopen("output.yuv", "rb+");
    if (fp == NULL)
    {
        cout << "Can not open this file\n"
             << endl;
        return -1;
    }
    SDL_Rect sdlRect;
    SDL_Rect sdlRect1;
    SDL_Thread *refresh_thread = SDL_CreateThread(refresh_video, NULL, NULL);
    SDL_Event event;
    while (1)
    {
        SDL_WaitEvent(&event);
        if (event.type == REFRESH_EVENT)
        {
            if (fread(buffer, 1, pixel_w * pixel_h * bpp / 8, fp) != pixel_w * pixel_h * bpp / 8)
            {
                // fseek(fp, 0, SEEK_SET);
                // fread(buffer, 1, pixel_w * pixel_h * bpp / 8, fp);
                thread_exit = 1;
                cout << "The end of video" << endl;
            }

            SDL_UpdateTexture(sdlTexture, NULL, buffer, pixel_w);

            sdlRect.x = 0;
            sdlRect.y = 0;
            sdlRect.w = pixel_w;
            sdlRect.h = pixel_h;
            sdlRect1.x = 640;
            sdlRect1.y = 0;
            sdlRect1.w = pixel_w;
            sdlRect1.h = pixel_h;

            SDL_RenderClear(sdlRenderer);
            SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
            SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect1);
            SDL_RenderPresent(sdlRenderer);
        }
        else if (event.type == SDL_WINDOWEVENT)
        {
            SDL_GetWindowSize(screen, &screen_w, &screen_h);
        }
        else if (event.type == SDL_QUIT)
        {
            thread_exit = 1;
        }
        else if (event.type == BREAK_EVENT)
        {
            break;
        }
    }
    SDL_Quit();
    return 0;
}