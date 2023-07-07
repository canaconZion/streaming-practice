/**
 * compile
 * g++ video_player.cpp -o D:\soft\mingw\mingw64\msys\home\Administrator\workdir\streaming-practice\ffmpeg\compiled\simplePlayer -ID:\soft\mingw\mingw64\msys\home\Administrator\workdir\streaming-practice\ffmpeg\include\SDL2 -LD:\soft\mingw\mingw64\msys\home\bin -ID:\soft\mingw\mingw64\msys\home\zkPlayer-0.1\include -LD:\soft\mingw\mingw64\msys\home\zkPlayer-0.1/lib -lavutil -lavformat -lavcodec -lavutil -lswscale -lSDL2
 */

#include <iostream>
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "SDL2/SDL.h"
}
#include <thread>
#include <windows.h>
using namespace std;

#define SFM_REFRESH_EVENT (SDL_USEREVENT + 1)
#define SFM_BREAK_EVENT (SDL_USEREVENT + 2)

int thread_exit = 0;
int thread_pause = 0;

int sfp_refresh_thread(void *opaque)
{
    thread_exit = 0;
    thread_pause = 0;

    while (!thread_exit)
    {
        if (!thread_pause)
        {
            SDL_Event event;
            event.type = SFM_REFRESH_EVENT;
            SDL_PushEvent(&event);
        }
        SDL_Delay(20);
    }
    thread_exit = 0;
    thread_pause = 0;

    SDL_Event event;
    event.type = SFM_BREAK_EVENT;
    SDL_PushEvent(&event);

    return 0;
}
struct DecodeTools
{
    char *filepath;
    AVFormatContext *pFormatCtx;
    int i, videoindex;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;
    AVCodecParameters *pCodecParams;
    AVFrame *pFrame, *pFrameYUV;
    unsigned char *out_buffer;
    AVPacket *packet;
    int ret, got_picture;
    struct SwsContext *img_convert_ctx;
};

struct SDLWin
{
    int screen_w, screen_h;
    SDL_Window *screen;
    SDL_Renderer *sdlRenderer;
    SDL_Texture *sdlTexture;
    SDL_Thread *video_tid;
    SDL_Event event;
};

typedef struct ThreadArgs
{
    int num;
    DecodeTools *decodetool;
    SDLWin *sdlwin;
};

int video_decode(DecodeTools *decodetool, SDLWin *sdlwin)
{
    int i;
    decodetool->pFormatCtx = avformat_alloc_context();

    if (avformat_open_input(&decodetool->pFormatCtx, decodetool->filepath, NULL, NULL) != 0)
    {
        cout << "Could not open input stream: " << decodetool->filepath << "\n"
             << endl;
        return -1;
    }
    if (avformat_find_stream_info(decodetool->pFormatCtx, NULL) < 0)
    {
        cout << "Could not find stream information.\n"
             << endl;
        return -1;
    }
    decodetool->videoindex = -1;
    // int num = decodetool->pFormatCtx->nb_streams;
    for (i = 0; i < decodetool->pFormatCtx->nb_streams; i++)
    {
        if (decodetool->pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            decodetool->videoindex = i;
            break;
        }
    }
    if (decodetool->videoindex == -1)
    {
        cout << "Did not find a video stream.\n"
             << endl;
        return -1;
    }
    decodetool->pCodecCtx = decodetool->pFormatCtx->streams[decodetool->videoindex]->codec;
    // pCodecParams = pFormatCtx->streams[videoindex]->codecpar;
    decodetool->pCodec = avcodec_find_decoder(decodetool->pCodecCtx->codec_id);
    if (decodetool->pCodec == NULL)
    {
        cout << "Could not found.\n"
             << endl;
        return -1;
    }
    if (avcodec_open2(decodetool->pCodecCtx, decodetool->pCodec, NULL) < 0)
    {
        cout << "Could not open codec.\n"
             << endl;
        return -1;
    }
    decodetool->pFrame = av_frame_alloc();
    decodetool->pFrameYUV = av_frame_alloc();

    decodetool->out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, decodetool->pCodecCtx->width, decodetool->pCodecCtx->height, 1));
    av_image_fill_arrays(decodetool->pFrameYUV->data, decodetool->pFrameYUV->linesize, decodetool->out_buffer,
                         AV_PIX_FMT_YUV420P, decodetool->pCodecCtx->width, decodetool->pCodecCtx->height, 1);

    cout << "------------------File Information-----------------\n"
         << endl;
    av_dump_format(decodetool->pFormatCtx, 0, decodetool->filepath, 0);
    cout << "--------------------------------------------------\n"
         << endl;

    decodetool->img_convert_ctx = sws_getContext(decodetool->pCodecCtx->width, decodetool->pCodecCtx->height, decodetool->pCodecCtx->pix_fmt,
                                                 decodetool->pCodecCtx->width, decodetool->pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
    decodetool->packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    return 0;
}

// int display_video(DecodeTools *decodetool, SDLWin *sdlwin)
static int display_video(void *a)
{
    ThreadArgs *is = static_cast<ThreadArgs *>(a);
    DecodeTools *decodetool = is->decodetool;
    SDLWin *sdlwin = is->sdlwin;
    SDL_Rect sdlRect_1;
    // ThreadArgs* args = static_cast<ThreadArgs*>(data);
    // DecodeTools* decodetool = args->decodetool;
    // SDLWin* sdlwin = args->sdlwin;
    sdlwin->sdlTexture = SDL_CreateTexture(sdlwin->sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, decodetool->pCodecCtx->width, decodetool->pCodecCtx->height);
    for (;;)
    {
        SDL_WaitEvent(&sdlwin->event);
        cout << sdlwin->event.type << endl;
        if (sdlwin->event.type == SFM_REFRESH_EVENT)
        {
            cout << "SFM_REFRESH_EVENT" << endl;
            if (is->num == 1)
            {
                sdlRect_1.y = 2;
                sdlRect_1.x = 2;
                sdlRect_1.w = sdlwin->screen_w / 2;
                sdlRect_1.h = sdlwin->screen_h / 2;
            }
            else if (is->num == 2)
            {
                sdlRect_1.y = 2;
                sdlRect_1.x = sdlwin->screen_w / 2 + 4;
                sdlRect_1.w = sdlwin->screen_w / 2;
                sdlRect_1.h = sdlwin->screen_h / 2;
            }
            while (1)
            {
                if (av_read_frame(decodetool->pFormatCtx, decodetool->packet) < 0)
                    // thread_pause = 1;
                    break;

                if (decodetool->packet->stream_index == decodetool->videoindex)
                    break;
            }
            int ret = avcodec_decode_video2(decodetool->pCodecCtx, decodetool->pFrame, &decodetool->got_picture, decodetool->packet);
            if (ret < 0)
            {
                cout << "Decode Error.\n"
                     << endl;
                return -1;
            }
            // else
            // {
            //     cout << "---------------decode 1 frame-----------------------\n"
            //          << endl;
            // }
            // int ret;
            if (decodetool->got_picture)
            {
                sws_scale(decodetool->img_convert_ctx, (const unsigned char *const *)decodetool->pFrame->data, decodetool->pFrame->linesize, 0,
                          decodetool->pCodecCtx->height, decodetool->pFrameYUV->data, decodetool->pFrameYUV->linesize);
                ret = SDL_UpdateTexture(sdlwin->sdlTexture, NULL, decodetool->pFrameYUV->data[0], decodetool->pFrameYUV->linesize[0]);
                if (ret != 0)
                {
                    cout << "SDL_UpdateTexture - " << SDL_GetError() << "\n"
                         << endl;
                    return -1;
                }
                ret = SDL_RenderCopy(sdlwin->sdlRenderer, sdlwin->sdlTexture, NULL, &sdlRect_1);
                // SDL_RenderCopy(sdlwin->sdlRenderer, sdlwin->sdlTexture, NULL, NULL);
                // SDL_RenderCopy(sdlwin->sdlRenderer, sdlwin->sdlTexture, NULL, NULL);
                // SDL_RenderCopy(sdlwin->sdlRenderer, sdlwin->sdlTexture, NULL, NULL);
                if (ret != 0)
                {
                    cout << "SDL_UpdateTexture - " << SDL_GetError() << "\n"
                         << endl;
                    return -1;
                }
                SDL_RenderPresent(sdlwin->sdlRenderer);
            }
            av_packet_unref(decodetool->packet);
        }
    }
    return 0;
}

int p_qiut(ThreadArgs *targ)
{
    SDL_Quit();
    sws_freeContext(targ->decodetool->img_convert_ctx);
    av_frame_free(&targ->decodetool->pFrameYUV);
    av_frame_free(&targ->decodetool->pFrame);
    avcodec_close(targ->decodetool->pCodecCtx);
    avformat_close_input(&targ->decodetool->pFormatCtx);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    int i, ret;
    // int screen_w, screen_h;
    // SDL_Window *screen;
    // SDL_Renderer *sdlRenderer;
    // SDL_Texture *sdlTexture;
    // SDL_Rect sdlRect_1, sdlRect_2, sdlRect_3, sdlRect_4;
    // SDL_Thread *video_tid;
    // SDL_Event event;

    DecodeTools *decodetool = new DecodeTools;
    SDLWin *sdlwin = new SDLWin;
    ThreadArgs *args = new ThreadArgs;
    args->decodetool = decodetool;
    args->sdlwin = sdlwin;
    args->num = 2;
    LPWSTR lpWideCmdLine = GetCommandLineW(); // 获取命令行参数字符串
    int argc;
    LPWSTR *argv = CommandLineToArgvW(lpWideCmdLine, &argc); // 解析命令行参数字符串为参数数组
    // 打印命令行参数
    // 将参数从宽字符转换为多字节字符
    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, argv[1], -1, NULL, 0, NULL, NULL);
    char *buffer = new char[bufferSize];

    if (argc > 1)
    {
        WideCharToMultiByte(CP_UTF8, 0, argv[1], -1, buffer, bufferSize, NULL, NULL);
        decodetool->filepath = buffer;
        std::cout << "argv[1]: " << buffer << std::endl;
    }
    else
    {
        decodetool->filepath = "cat.flv";
        // return -1;
    }

    // av_register_all();
    // avformat_network_init();
    DecodeTools *decodetool2 = new DecodeTools;
    decodetool2->filepath = "mecha.flv";
    ThreadArgs *args2 = new ThreadArgs;
    args2->decodetool = decodetool2;
    args2->sdlwin = sdlwin;
    args2->num = 1;
    video_decode(decodetool, sdlwin);
    video_decode(decodetool2, sdlwin);
    // video_decode(decodetool_2);
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        cout << "Could not initialize SDL - " << SDL_GetError() << "\n"
             << endl;
        return -1;
    }
    sdlwin->screen_w = decodetool->pCodecCtx->width + 6;
    sdlwin->screen_h = decodetool->pCodecCtx->height + 6;
    sdlwin->screen_w = 1280;
    sdlwin->screen_h = 720;
    sdlwin->screen = SDL_CreateWindow("video Player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, sdlwin->screen_w, sdlwin->screen_h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!sdlwin->screen)
    {
        cout << "SDL: could not creeate window - exiting:\n"
             << SDL_GetError() << "\n"
             << endl;
        return -1;
    }
    sdlwin->sdlRenderer = SDL_CreateRenderer(sdlwin->screen, -1, 0);
    sdlwin->video_tid = SDL_CreateThread(sfp_refresh_thread, NULL, NULL);
    // std::thread myThread(display_video,decodetool,sdlwin);
    // myThread.join();
    // display_video(decodetool,sdlwin);
    SDL_Thread *thread = SDL_CreateThread(display_video, "DisplayThread", static_cast<void *>(args));
    SDL_Thread *thread2 = SDL_CreateThread(display_video, "DisplayThread2", static_cast<void *>(args2));
    while (1)
    {
        // SDL_WaitEvent(&sdlwin->event);
        if (sdlwin->event.type == SFM_REFRESH_EVENT)
        {
            // ret = SDL_RenderClear(sdlwin->sdlRenderer);
            SDL_Delay(40);
        }
        else if (sdlwin->event.type == SDL_WINDOWEVENT)
        {
            SDL_GetWindowSize(sdlwin->screen, &sdlwin->screen_w, &sdlwin->screen_h);
        }
        else if (sdlwin->event.type == SDL_KEYDOWN)
        {
            if (sdlwin->event.key.keysym.sym == SDLK_SPACE)
                thread_pause = !thread_pause;
        }
        else if (sdlwin->event.type == SDL_QUIT)
        {
            thread_exit = 1;
        }
        else if (sdlwin->event.type == SFM_BREAK_EVENT)
        {
            break;
        }
        if (thread_exit == 1)
        {
            break;
        }
    }
    p_qiut(args);
    p_qiut(args2);
    LocalFree(argv);
    delete[] buffer;
    delete decodetool;
    delete decodetool2;
    delete sdlwin;
    delete args;
    delete args2;
    return 0;
}