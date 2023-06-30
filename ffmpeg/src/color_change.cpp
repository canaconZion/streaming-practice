// #include <iostream>
// extern "C"
// {
//     #include "SDL2/SDL.h"
// }
// #include <windows.h>

// int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
// {
//     int ret = -1;
//     int video_width = 640;
//     int video_height = 480;

//     SDL_Window* window;
//     SDL_Renderer* renderer;
//     SDL_Texture* texture;
//     SDL_Rect sdlRect{0,0,video_width,video_height};

//     if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO |SDL_INIT_TIMER))
//     {
//         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"ERROR SDL_Init - %s\n",SDL_GetError());
//         return ret;
//     }
//     window = SDL_CreateWindow("Easy palyer",
//             SDL_WINDOWPOS_UNDEFINED,
//             SDL_WINDOWPOS_UNDEFINED,
//             video_width, video_height,
//             SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    
//     if(window == NULL)
//     {
//         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ERROR SDL_CreateWindow - %s\n",SDL_GetError());
//         return -1;
//     }

//     renderer = SDL_CreateRenderer(window,-1,0);
//     if(!renderer)
//     {
//         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ERROR SDL_CreateRenderer - %s\n",SDL_GetError());
//         return -1;
//     }


//     int pixformat = SDL_PIXELFORMAT_IYUV;
//     texture = SDL_CreateTexture(renderer,
//             pixformat,
//             SDL_TEXTUREACCESS_STREAMING,
//             video_width,video_height);
//     if(!texture)
//     {
//         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ERROR SDL_CreateTexture - %s\n",SDL_GetError());
//         return -1;
//     }

//     int count = 100;
//     int color_r = 255;
//     int color_g = 255;
//     int color_b = 255;
//     while (count--)
//     {
//         color_r = (color_r+=1)%255;
//         color_g = (color_g+=2)%255;
//         color_b = (color_b+=3)%255;
//         std::cout << "color_r = " << color_r << "   color_g = " << color_g << "   color_b = " << color_b << std::endl;
//         SDL_SetRenderDrawColor(renderer,color_r,color_g,color_b,255);
//         SDL_RenderClear(renderer);
//         SDL_RenderPresent(renderer);
//         SDL_Delay(40);
//     }
//     if(texture){SDL_DestroyTexture(texture);}
//     if(renderer){SDL_DestroyRenderer(renderer);}
//     if(window){SDL_DestroyWindow(window);}
//     SDL_Quit();
//     return 0;
// }
#include <iostream>
extern "C"
{
    #include "SDL2/SDL.h"
};

#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    int ret = -1;
    int video_width = 640;
    int video_height = 480;

    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    SDL_Rect sdlRect1{ 0, 0, video_width / 2, video_height / 2 };
    SDL_Rect sdlRect2{ video_width / 2, 0, video_width / 2, video_height / 2 };
    SDL_Rect sdlRect3{ 0, video_height / 2, video_width / 2, video_height / 2 };
    SDL_Rect sdlRect4{ video_width / 2, video_height / 2, video_width / 2, video_height / 2 };

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ERROR SDL_Init - %s\n", SDL_GetError());
        return ret;
    }

    window = SDL_CreateWindow("Easy player",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        video_width, video_height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (window == nullptr)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ERROR SDL_CreateWindow - %s\n", SDL_GetError());
        SDL_Quit();
        return ret;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == nullptr)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ERROR SDL_CreateRenderer - %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return ret;
    }

    int pixformat = SDL_PIXELFORMAT_IYUV;
    texture = SDL_CreateTexture(renderer,
        pixformat,
        SDL_TEXTUREACCESS_STREAMING,
        video_width, video_height);
    if (texture == nullptr)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ERROR SDL_CreateTexture - %s\n", SDL_GetError());
        SDL_RenderCopy(renderer, texture, NULL, &sdlRect1);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return ret;
    }

    int count = 1000;
    int color_r = 255;
    int color_g = 255;
    int color_b = 255;
    while (count--)
    {
        color_r = (color_r + 1) % 255;
        color_g = (color_g + 2) % 255;
        color_b = (color_b + 3) % 255;
        std::cout << "color_r = " << color_r << "   color_g = " << color_g << "   color_b = " << color_b << std::endl;
        SDL_SetRenderDrawColor(renderer, color_r, color_g, color_b, 255);
        SDL_RenderFillRect(renderer, &sdlRect1);

        SDL_SetRenderDrawColor(renderer,color_g, color_r, color_b, 255);
        SDL_RenderFillRect(renderer, &sdlRect2);

        SDL_SetRenderDrawColor(renderer, color_r, color_b, color_g, 255);
        SDL_RenderFillRect(renderer, &sdlRect3);

        SDL_SetRenderDrawColor(renderer, color_b, color_r, color_g, 255);
        SDL_RenderFillRect(renderer, &sdlRect4);

        SDL_RenderPresent(renderer);
        SDL_Delay(40);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
