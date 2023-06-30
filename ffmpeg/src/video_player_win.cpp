#include <iostream>
#include <String>
extern "C"
{
    #include "SDL2/SDL.h"
}
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    /*
        获取启动参数
    */
    // LPWSTR lpWideCmdLine = GetCommandLineW(); // 获取命令行参数字符串
    // int argc;
    // LPWSTR* argv = CommandLineToArgvW(lpWideCmdLine, &argc); // 解析命令行参数字符串为参数数组
    // // std::string file = argv[1];
    // // 打印命令行参数
    // // 将参数从宽字符转换为多字节字符
    // int bufferSize = WideCharToMultiByte(CP_UTF8, 0, argv[1], -1, NULL, 0, NULL, NULL);
    // char* buffer = new char[bufferSize];
    // WideCharToMultiByte(CP_UTF8, 0, argv[1], -1, buffer, bufferSize, NULL, NULL);

    // std::cout << "argv[1]: " << buffer << std::endl;

    // delete[] buffer;

    // // 释放内存
    // LocalFree(argv);
    int window_width = 640, window_height = 360;
    const int video_width = 640, video_height = 360;
    const int frame_size = video_height*video_width*3/2;
    unsigned char frame_buffer[frame_size];

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *win = SDL_CreateWindow("SDL2 playre",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,window_width,window_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_TOOLTIP | SDL_WINDOW_MOUSE_CAPTURE);
    if(win == NULL){
        std::cout << "Create Window failed" << std::endl;
    }
    SDL_Renderer *render = SDL_CreateRenderer(win,-1,0);
    SDL_Texture *texture = SDL_CreateTexture(render,SDL_PIXELFORMAT_IYUV,SDL_TEXTUREACCESS_STREAMING, video_width , video_height);
    SDL_Rect sdlRect{0,0,window_width,window_height};

    FILE* fp = fopen("D:/video/videos/cat.yuv","rb+");
    if(fp == NULL)return -1;
    while(1){
        // size_t elements_read = fread(frame_buffer,1,frame_size,fp);
        // std::cout << elements_read << std::endl;
        if( fread(frame_buffer,1,frame_size,fp) != frame_size){
            fread(frame_buffer,1,frame_size,fp);
            // std::cout << "what to do" << std::endl;
        }
        SDL_UpdateTexture(texture,NULL,frame_buffer,video_width);
        SDL_RenderClear(render);
        SDL_RenderCopy(render,texture,NULL,&sdlRect);
        SDL_RenderPresent(render);
        SDL_Delay(40);
    }
    fclose(fp);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(render);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}