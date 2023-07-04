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
        SDL_Delay(40);
    }
    thread_exit = 0;
    thread_pause = 0;

    SDL_Event event;
    event.type = SFM_BREAK_EVENT;
    SDL_PushEvent(&event);

    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

    AVFormatContext *pFormatCtx;
    int i, videoindex;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;
    AVCodecParameters *pCodecParams;
    AVFrame *pFrame, *pFrameYUV;
    unsigned char *out_buffer;
    AVPacket *packet;
    int ret, got_picture;

    int screen_w, screen_h;
    SDL_Window *screen;
    SDL_Renderer *sdlRenderer;
    SDL_Texture *sdlTexture;
    SDL_Rect sdlRect_1, sdlRect_2, sdlRect_3, sdlRect_4;
    SDL_Thread *video_tid;
    SDL_Event event;

    struct SwsContext *img_convert_ctx;

    LPWSTR lpWideCmdLine = GetCommandLineW(); // 获取命令行参数字符串
    int argc;
    char *filepath;
    LPWSTR *argv = CommandLineToArgvW(lpWideCmdLine, &argc); // 解析命令行参数字符串为参数数组
    // 打印命令行参数
    // 将参数从宽字符转换为多字节字符
    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, argv[1], -1, NULL, 0, NULL, NULL);
    char *buffer = new char[bufferSize];

    if (argc > 1)
    {
        WideCharToMultiByte(CP_UTF8, 0, argv[1], -1, buffer, bufferSize, NULL, NULL);
        filepath = buffer;
        std::cout << "argv[1]: " << buffer << std::endl;
    }
    else
    {
        cout << "Please add the path to the video file that requires the part.\n"
             << endl;
        return -1;
    }

    // av_register_all();
    // avformat_network_init();
    pFormatCtx = avformat_alloc_context();

    if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0)
    {
        cout << "Could not open input stream: " << filepath << "\n"
             << endl;
        return -1;
    }
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        cout << "Could not find stream information.\n"
             << endl;
        return -1;
    }
    videoindex = -1;
    for (i = 0; i < pFormatCtx->nb_streams; i++)
    {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoindex = i;
            break;
        }
    }
    if (videoindex == -1)
    {
        cout << "Did not find a video stream.\n"
             << endl;
        return -1;
    }
    pCodecCtx = pFormatCtx->streams[videoindex]->codec;
    // pCodecParams = pFormatCtx->streams[videoindex]->codecpar;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL)
    {
        cout << "Could not found.\n"
             << endl;
        return -1;
    }
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
    {
        cout << "Could not open codec.\n"
             << endl;
        return -1;
    }
    pFrame = av_frame_alloc();
    pFrameYUV = av_frame_alloc();

    out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1));
    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer,
                         AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

    cout << "------------------File Information-----------------\n"
         << endl;
    av_dump_format(pFormatCtx, 0, filepath, 0);
    cout << "--------------------------------------------------\n"
         << endl;

    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                     pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        cout << "Could not initialize SDL - " << SDL_GetError() << "\n"
             << endl;
        return -1;
    }
    screen_w = pCodecCtx->width + 6;
    screen_h = pCodecCtx->height + 6;
    screen = SDL_CreateWindow("video Player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_w, screen_h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!screen)
    {
        cout << "SDL: could not creeate window - exiting:\n"
             << SDL_GetError() << "\n"
             << endl;
        return -1;
    }
    sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
    sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width, pCodecCtx->height);
    packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    video_tid = SDL_CreateThread(sfp_refresh_thread, NULL, NULL);

    for (;;)
    {
        SDL_WaitEvent(&event);
        if (event.type == SFM_REFRESH_EVENT)
        {
            sdlRect_1.y = 2;
            sdlRect_1.x = 2;
            sdlRect_1.w = screen_w / 2;
            sdlRect_1.h = screen_h / 2;

            sdlRect_2.y = 2;
            sdlRect_2.x = screen_w / 2 + 4;
            sdlRect_2.w = screen_w / 2;
            sdlRect_2.h = screen_h / 2;

            sdlRect_3.y = screen_h / 2 + 4;
            sdlRect_3.x = 2;
            sdlRect_3.w = screen_w / 2;
            sdlRect_3.h = screen_h / 2;

            sdlRect_4.y = screen_h / 2 + 4;
            sdlRect_4.x = screen_w / 2 + 4;
            sdlRect_4.w = screen_w / 2;
            sdlRect_4.h = screen_h / 2;

            while (1)
            {
                if (av_read_frame(pFormatCtx, packet) < 0)
                    thread_pause = 1;

                if (packet->stream_index == videoindex)
                    break;
            }
            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
            if (ret < 0)
            {
                cout << "Decode Error.\n"
                     << endl;
                return -1;
            }
            if (got_picture)
            {
                sws_scale(img_convert_ctx, (const unsigned char *const *)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
                SDL_UpdateTexture(sdlTexture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0]);
                SDL_RenderClear(sdlRenderer);
                SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect_1);
                SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect_2);
                SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect_3);
                SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect_4);
                SDL_RenderPresent(sdlRenderer);
            }
            av_packet_unref(packet);
        }
        else if (event.type == SDL_WINDOWEVENT)
        {
            SDL_GetWindowSize(screen, &screen_w, &screen_h);
        }
        else if (event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.sym == SDLK_SPACE)
                thread_pause = !thread_pause;
        }
        else if (event.type == SDL_QUIT)
        {
            thread_exit = 1;
        }
        else if (event.type == SFM_BREAK_EVENT)
        {
            break;
        }
    }
    sws_freeContext(img_convert_ctx);

    SDL_Quit();
    av_frame_free(&pFrameYUV);
    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);

    LocalFree(argv);
    delete[] buffer;

    return 0;
}