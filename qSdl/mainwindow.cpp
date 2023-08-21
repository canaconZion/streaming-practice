#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    imgLabel = new QLabel(this);
    imgLabel->resize(640,480);
    SDL_Init(SDL_INIT_VIDEO);
    window= SDL_CreateWindowFrom((void *)imgLabel->winId());
    std::thread([&]{
        //        render = SDL_CreateRenderer(window,-1,0);
        //        SDL_SetRenderDrawColor(render, 81,196,211,255);
        //        SDL_RenderClear(render);
        //        SDL_RenderPresent(render);
        //        SDL_Delay(10*1000);
        //        SDL_DestroyWindow(window);
        //        SDL_Quit();
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
    }).detach();
}

MainWindow::~MainWindow()
{
    delete ui;
}

