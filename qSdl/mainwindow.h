#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "SDL.h"
#include <QLabel>
#include <thread>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QLabel *imgLabel;
//    SDL_Window *window;
//    SDL_Renderer *render;
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

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
