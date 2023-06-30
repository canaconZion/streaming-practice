/*
compile
   g++ win.cpp -o wincreate -I/home/Administrator/workdir/SDL2 -L/home/bin -lSDL2
 */

#include <iostream>
extern "C"
{
    #include "SDL2/SDL.h"
}

int WinMain(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
        // SDL_WINDOW_FULLSCREEN：窗口全屏显示。
        // SDL_WINDOW_OPENGL：窗口支持OpenGL渲染。
        // SDL_WINDOW_SHOWN：窗口可见。
        // SDL_WINDOW_HIDDEN：窗口隐藏。
        // SDL_WINDOW_BORDERLESS：窗口无边框。
        // SDL_WINDOW_RESIZABLE：窗口可调整大小。
        // SDL_WINDOW_MINIMIZED：窗口最小化。
        // SDL_WINDOW_MAXIMIZED：窗口最大化。
        // SDL_WINDOW_INPUT_GRABBED：窗口捕获输入焦点。
        // SDL_WINDOW_ALLOW_HIGHDPI：窗口支持高DPI。
        // SDL_WINDOW_MOUSE_CAPTURE：窗口捕获鼠标。
        // SDL_WINDOW_ALWAYS_ON_TOP：窗口始终置顶。
        // SDL_WINDOW_SKIP_TASKBAR：窗口不显示在任务栏上。
        // SDL_WINDOW_UTILITY：窗口以实用程序方式显示。
        // SDL_WINDOW_TOOLTIP：窗口为工具提示窗口。
        // SDL_WINDOW_POPUP_MENU：窗口为弹出菜单窗口。
    SDL_Window *window = SDL_CreateWindow("SDL2 window",100,100,640,480,SDL_WINDOW_SHOWN);
    SDL_Renderer *render = SDL_CreateRenderer(window,-1,0);
    SDL_SetRenderDrawColor(render, 255,255,0,255);
    SDL_RenderClear(render);
    SDL_RenderPresent(render);
    SDL_Delay(5000);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}