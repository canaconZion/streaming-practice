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
#include <thread>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <opencv2\face.hpp>
#include <fstream>

using namespace std;
using namespace cv;

#define SFM_REFRESH_EVENT (SDL_USEREVENT + 1)
#define SFM_BREAK_EVENT (SDL_USEREVENT + 2)

int thread_exit = 0;
int thread_pause = 0;
int64_t frameDurationUs;

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
        else
        {
            printf("Pause\n");
        }
        SDL_Delay(frameDurationUs / 1000 + 6);
        // std::this_thread::sleep_for(std::chrono::microseconds(frameDurationUs));
    }
    thread_exit = 0;
    thread_pause = 0;

    SDL_Event event;
    event.type = SFM_BREAK_EVENT;
    SDL_PushEvent(&event);

    return 0;
}

cv::Mat detect_face(Mat yuvimg, int64_t times)
{
    cv::Mat img;
    cv::cvtColor(yuvimg, img, COLOR_YUV2BGR_I420);
    // img = yuvimg;
    CascadeClassifier cascade;
    const string path = "./haarcascade_frontalface_alt2.xml";
    if (!cascade.load(path))
    {
        // cout << "cascade load failed!\n";
        return yuvimg;
    }
    vector<cv::Rect> faces(0);
    cascade.detectMultiScale(img, faces, 1.1, 2, 0, Size(30, 30));

    // cout << "detect face number is :" << faces.size() << endl;

    if (faces.size() > 0)
    {
        for (size_t i = 0; i < faces.size(); i++)
        {
            rectangle(img, faces[i], Scalar(0, 0, 255), 3, 8, 0);
        }
        std::string filename = "./faceImg/" + std::to_string(times) + ".jpg";
        // imwrite(filename, img);
    }
    else
    {
        // cout << "did not detect face" << endl;
        return yuvimg;
    }
    // namedWindow("face_detect");
	// imshow("face_detect", img);
    cv::cvtColor(img, yuvimg, COLOR_BGR2YUV_I420);
    return yuvimg;
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
    SDL_Rect sdlRect_1;
    SDL_Thread *video_tid;
    SDL_Event event;

    // video play control
    bool do_seek = false;
    int64_t seek_length = 5;
    int rem_seek;
    int64_t seek_pos;
    int seek_dir;

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
            AVRational timebase = pFormatCtx->streams[i]->time_base;
            AVRational frameRate = av_guess_frame_rate(pFormatCtx, pFormatCtx->streams[i], NULL);
            frameDurationUs = av_rescale_q(1, av_inv_q(frameRate), AV_TIME_BASE_Q);
            printf("Frame duration: %lld microseconds\n", frameDurationUs);
            printf("TimeBase: %d/%d\n", timebase.num, timebase.den);
            rem_seek = seek_length;
            seek_length *= timebase.den;
            printf("seek length: %d\n", seek_length);
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

    cv::Mat frameMat;

    for (;;)
    {
        SDL_WaitEvent(&event);
        if (event.type == SFM_REFRESH_EVENT)
        {
            sdlRect_1.y = 0;
            sdlRect_1.x = 0;
            sdlRect_1.w = screen_w;
            sdlRect_1.h = screen_h;

            while (1)
            {
                if (av_read_frame(pFormatCtx, packet) < 0)
                    thread_pause = 1;

                if (packet->stream_index == videoindex)
                    break;
            }
            if (do_seek)
            {
                printf("current packet pts: %d\n", packet->pts);
                if (seek_dir)
                {
                    printf("Fast Forward %d s \n", rem_seek);
                    seek_pos = packet->pts + seek_length;
                }
                else
                {
                    printf("Rewind %d s \n", rem_seek);
                    seek_pos = packet->pts - seek_length;
                }
                if (av_seek_frame(pFormatCtx, videoindex, seek_pos, AVSEEK_FLAG_BACKWARD) < 0)
                {
                    printf("Error while seeking\n");
                    return -1;
                }
                do_seek = false;
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
                // if (packet->flags & AV_PKT_FLAG_KEY)

                int64_t pts = packet->pts;
                frameMat = cv::Mat(pCodecCtx->height * 3 / 2, pCodecCtx->width, CV_8UC1, pFrameYUV->data[0]);
                frameMat = detect_face(frameMat, pts);
                SDL_UpdateTexture(sdlTexture, NULL, frameMat.data, frameMat.step);
                // SDL_UpdateTexture(sdlTexture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0]);
                SDL_RenderClear(sdlRenderer);
                SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect_1);
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
            else if (event.key.keysym.sym == SDLK_LEFT)
            {
                // cout << "Rewind" << endl;
                do_seek = true;
                seek_dir = 0;
            }
            else if (event.key.keysym.sym == SDLK_RIGHT)
            {
                do_seek = true;
                seek_dir = 1;
            }
            else if (event.key.keysym.sym == SDLK_q)
            {
                thread_exit = 1;
            }
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
    cout << "Quit play" << endl;
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
