/**
 * TODO:
 *  create audio and video packet queue, get packet from queue
*/

#include <iostream>
#include <thread>
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libswresample/swresample.h"
#include "SDL2/SDL.h"
}
#include <windows.h>
using namespace std;

#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio

#define SFM_REFRESH_EVENT (SDL_USEREVENT + 1)
#define SFM_BREAK_EVENT (SDL_USEREVENT + 2)

int thread_exit = 0;
int thread_pause = 0;

// Buffer:
//|-----------|-------------|
// chunk-------pos---len-----|
static Uint8 *audio_chunk;
static Uint32 audio_len;
static Uint8 *audio_pos;

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

void fill_audio(void *udata, Uint8 *stream, int len)
{
    // SDL 2.0
    SDL_memset(stream, 0, len);
    if (audio_len == 0)
        return;

    len = (len > audio_len ? audio_len : len); /*  Mix  as  much  data  as  possible  */

    SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
    audio_pos += len;
    audio_len -= len;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    //  for videoStream
    AVFormatContext *pFormatCtx;
    int i, videoindex;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;
    AVCodecParameters *pCodecParams;
    AVFrame *pFrame, *pFrameYUV;
    unsigned char *out_buffer;
    AVPacket *packet;
    int ret, got_picture;

    // for audioStream
    int index = 0;
    int audioindex;
    AVCodecContext *aCodecCtx;
    AVCodec *aCodec;
    uint8_t *a_out_buffer;
    AVFrame *aFrame;
    SDL_AudioSpec wanted_spec;
    int64_t in_channel_layout;
    struct SwrContext *au_convert_ctx;
    int  out_buffer_size;
    int a_got_picture;

    // for SDL
    int screen_w, screen_h;
    SDL_Window *screen;
    SDL_Renderer *sdlRenderer;
    SDL_Texture *sdlTexture;
    SDL_Rect sdlRect_1;
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

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        cout << "Could not initialize SDL - " << SDL_GetError() << "\n"
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
    audioindex = -1;
    for (i = 0; i < pFormatCtx->nb_streams; i++)
    {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoindex = i;
        }
        else if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audioindex = i;
        }
    }

    if (videoindex == -1)
    {
        cout << "Did not find a video stream.\n"
             << endl;
        return -1;
    }
    else
    {
        pCodecCtx = pFormatCtx->streams[videoindex]->codec;
        // pCodecParams = pFormatCtx->streams[videoindex]->codecpar;
        pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
        if (pCodec == NULL)
        {
            cout << "Could not found video codec.\n"
                 << endl;
            return -1;
        }
        if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
        {
            cout << "Could not open video codec.\n"
                 << endl;
            return -1;
        }
        pFrame = av_frame_alloc();
        pFrameYUV = av_frame_alloc();

        out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1));
        av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer,
                             AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

        img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                         pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
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
        video_tid = SDL_CreateThread(sfp_refresh_thread, NULL, NULL);
    }

    if (audioindex == -1)
    {
        cout << "Could not find audioStream" << endl;
        return -1;
    }
    else
    {
        aCodecCtx = pFormatCtx->streams[audioindex]->codec;
        aCodec = avcodec_find_decoder(aCodecCtx->codec_id);
        if (aCodec == NULL)
        {
            printf("Audio Codec not found.\n");
            return -1;
        }
        if (avcodec_open2(aCodecCtx, aCodec, NULL) < 0)
        {
            printf("Could not open codec.\n");
            return -1;
        }
        uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
        // nb_samples: AAC-1024 MP3-1152
        int out_nb_samples = aCodecCtx->frame_size;
        AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
        int out_sample_rate = 44100;
        int out_channels = av_get_channel_layout_nb_channels(out_channel_layout);
        // Out Buffer Size
        out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, out_sample_fmt, 1);

        a_out_buffer = (uint8_t *)av_malloc(MAX_AUDIO_FRAME_SIZE * 2);
        aFrame = av_frame_alloc();
        cout << "VideoIndex" << videoindex << "\nAudioINdex" << audioindex << endl;
        wanted_spec.freq = out_sample_rate;
        wanted_spec.format = AUDIO_S16SYS;
        wanted_spec.channels = out_channels;
        wanted_spec.silence = 0;
        wanted_spec.samples = out_nb_samples;
        wanted_spec.callback = fill_audio;
        wanted_spec.userdata = aCodecCtx;

        if (SDL_OpenAudio(&wanted_spec, NULL) < 0)
        {
            printf("can't open audio.\n");
            return -1;
        }
        // FIX:Some Codec's Context Information is missing
        in_channel_layout = av_get_default_channel_layout(aCodecCtx->channels);
        // Swr

        au_convert_ctx = swr_alloc();
        au_convert_ctx = swr_alloc_set_opts(au_convert_ctx, out_channel_layout, out_sample_fmt, out_sample_rate,
                                            in_channel_layout, pCodecCtx->sample_fmt, aCodecCtx->sample_rate, 0, NULL);
        swr_init(au_convert_ctx);

        // Play
        SDL_PauseAudio(0);
    }

    cout << "------------------File Information-----------------\n"
         << endl;
    av_dump_format(pFormatCtx, 0, filepath, 0);
    cout << "--------------------------------------------------\n"
         << endl;

    packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    av_init_packet(packet);

    for (;;)
    {
        if (av_read_frame(pFormatCtx, packet) < 0)
            break;
        if (packet->stream_index == videoindex)
        {
            SDL_WaitEvent(&event);
            if (event.type == SFM_REFRESH_EVENT)
            {
                sdlRect_1.y = 2;
                sdlRect_1.x = 2;
                sdlRect_1.w = screen_w;
                sdlRect_1.h = screen_h;
                if (packet->stream_index == videoindex)
                {
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
                        SDL_RenderPresent(sdlRenderer);
                    }
                }
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
           av_free_packet(packet);
        }
        else if (packet->stream_index == audioindex)
        {
            cout << "decode audio packet" << endl;
        //     std::thread([&]()
        //     {
        //         ret = avcodec_decode_audio4(aCodecCtx, aFrame, &a_got_picture, packet);
        //         if (ret < 0)
        //         {
        //             printf("Error in decoding audio frame.\n");
        //             return -1;
        //         }
        //         if (a_got_picture > 0)
        //         {
        //             swr_convert(au_convert_ctx, &a_out_buffer, MAX_AUDIO_FRAME_SIZE, (const uint8_t **)pFrame->data, pFrame->nb_samples);

        //             printf("index:%5d\t pts:%lld\t packet size:%d\n", index, packet->pts, packet->size);

        //             index++;
        //         }

        //         while (audio_len > 0) // Wait until finish
        //             SDL_Delay(1);

        //         // Set audio buffer (PCM data)
        //         audio_chunk = (Uint8 *)a_out_buffer;
        //         // Audio buffer length
        //         audio_len = out_buffer_size;
        //         audio_pos = audio_chunk;
		//         av_free_packet(packet);
        // }).detach();
    }
}
sws_freeContext(img_convert_ctx);

SDL_Quit();
SDL_CloseAudio();

swr_free(&au_convert_ctx);

av_free(a_out_buffer);
avcodec_close(aCodecCtx);

av_free(out_buffer);
av_frame_free(&pFrameYUV);
av_frame_free(&pFrame);
avcodec_close(pCodecCtx);
avformat_close_input(&pFormatCtx);

LocalFree(argv);
delete[] buffer;

return 0;
}