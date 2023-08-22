#include "fplayer/simple_player.hpp"

MediaProcess::MediaProcess()
{
    int index = 0;
    std::cout << "----------Init decode----------------\n"
              << std::endl;
    sdl_init();
}

MediaProcess::~MediaProcess()
{
}

int MediaProcess::srp_refresh_thread(void *opaque)
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

int MediaProcess::media_decode()
{
    pFormatCtx = avformat_alloc_context();

    if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0)
    {
        std::cout << "Could not open input stream: " << filepath << "\n"
                  << std::endl;
        goto end;
    }
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        std::cout << "Could not find stream information.\n"
                  << std::endl;
        goto end;
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
    std::cout << "------------------File Information-----------------\n"
              << std::endl;
    av_dump_format(pFormatCtx, 0, filepath, 0);
    std::cout << "--------------------------------------------------\n"
              << std::endl;
    if (videoindex >= 0)
    {
        pCodecCtx = pFormatCtx->streams[videoindex]->codec;
        // pCodecParams = pFormatCtx->streams[videoindex]->codecpar;
        pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
        if (pCodec == NULL)
        {
            std::cout << "Could not found video codec.\n"
                      << std::endl;
            goto end;
        }
        if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
        {
            std::cout << "Could not open video codec.\n"
                      << std::endl;
            goto end;
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
            std::cout << "SDL: could not creeate window - exiting:\n"
                      << SDL_GetError() << "\n"
                      << std::endl;
            goto end;
        }
        sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
        sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width, pCodecCtx->height);
        mVideoStream = pFormatCtx->streams[videoindex];
        // std::thread([&](MediaProcess *v_decoder)
        //             { v_decoder->video_decode(); },
        //             this)
        //     .detach();
    }
    if (audioindex >= 0)
    {
        std::cout << "decode audio\n"
                  << std::endl;
    }
    packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    av_init_packet(packet);
    while (1)
    {
        // int stream_index = -1;
        // if (videoindex >= 0)
        //     stream_index = videoindex;
        // else if (audioindex >= 0)
        //     stream_index = audioindex;

        // if (videoindex >= 0)
        // {
        //     AVPacket packet;
        //     av_new_packet(&packet, 10);
        //     strcpy((char *)packet.data, "FLUSH");
        //     clearVideoQuene();       // 清除队列
        //     inputVideoQuene(packet); // 往队列中存入用来清除的包
        // }
        // std::cout << "In to cycle\n"
        //           << std::endl;
        // if (mVideoPacktList.size() > MAX_VIDEO_SIZE)
        // {
        //     std::this_thread::sleep_for(std::chrono::seconds(10));
        //     continue;
        // }

        // AVPacket packet;
        // if (av_read_frame(pFormatCtx, &packet) < 0)
        // {
        //     mIsReadFinished = true;
        //     std::cout << "end of file" << std::endl;
        //     std::this_thread::sleep_for(std::chrono::seconds(10));
        //     goto end;
        // }
        // if (packet.stream_index == videoindex)
        // {
        //     std::cout << "Try to input video to queue\n"
        //               << std::endl;
        //     inputVideoQuene(packet);
        // }
        // else
        // {
        //     // Free the packet that was allocated by av_read_frame
        //     av_packet_unref(&packet);
        // }
        if (av_read_frame(pFormatCtx, packet) < 0)
            goto end;
        if (packet->stream_index == videoindex)
        {
            // SDL_WaitEvent(&event);
            // if (event.type == SFM_REFRESH_EVENT)
            // {
            sdlRect_1.y = 2;
            sdlRect_1.x = 2;
            sdlRect_1.w = screen_w;
            sdlRect_1.h = screen_h;
            if (packet->stream_index == videoindex)
            {
                ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
                if (ret < 0)
                {
                    std::cout << "Decode Error.\n"
                            << std::endl;
                    goto end;
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
            SDL_Delay(40);
            av_free_packet(packet);
        }
    }
end:
    // clearAudioQuene();
    std::cout << "go to end" << std::endl;
    /** Thread decode **/
    // clearVideoQuene();
    // if (aFrame != nullptr)
    // {
    //     av_frame_free(&aFrame);
    //     aFrame = nullptr;
    // }

    // if (aCodecCtx != nullptr)
    // {
    //     avcodec_close(aCodecCtx);
    //     aCodecCtx = nullptr;
    // }

    // if (pCodecCtx != nullptr)
    // {
    //     avcodec_close(pCodecCtx);
    //     pCodecCtx = nullptr;
    // }

    // avformat_close_input(&pFormatCtx);
    // avformat_free_context(pFormatCtx);

    // SDL_Quit();
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
    return 0;
}

int MediaProcess::video_decode()
{
    while (1)
    {
        std::cout << "Begin to decode video\n"
                  << std::endl;
        mConditon_Video->Lock();
        if (mVideoPacktList.size() <= 0)
        {
            mConditon_Video->Unlock();
            if (mIsReadFinished)
            {
                // 队列里面没有数据了且读取完毕了
                std::cout << "Read packet finished, quit process\n"
                          << std::endl;
                break;
            }
            else
            {
                std::cout << "No packet in queue\n"
                          << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1)); // 队列只是暂时没有数据而已
                continue;
            }
        }
        AVPacket pkt1 = mVideoPacktList.front();
        mVideoPacktList.pop_front();

        mConditon_Video->Unlock();

        AVPacket *packet = &pkt1;
        sdlRect_1.y = 2;
        sdlRect_1.x = 2;
        sdlRect_1.w = screen_w;
        sdlRect_1.h = screen_h;
        ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
        if (ret < 0)
        {
            std::cout << "Decode Error.\n"
                      << std::endl;
            return -1;
        }
        std::cout << "decode 1 video frame.\n"
                  << std::endl;
        if (got_picture)
        {
            sws_scale(img_convert_ctx, (const unsigned char *const *)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
            SDL_UpdateTexture(sdlTexture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0]);
            SDL_RenderClear(sdlRenderer);
            SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect_1);
            SDL_RenderPresent(sdlRenderer);
        }
        SDL_Delay(40);
        av_free_packet(packet);
        av_packet_unref(packet);
    }
    swr_free(&au_convert_ctx);

    av_free(a_out_buffer);
    avcodec_close(aCodecCtx);

    av_free(out_buffer);
    av_frame_free(&pFrameYUV);
    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);
    return 0;
}

int MediaProcess::audio_decode()
{
}

int MediaProcess::sdl_init()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        std::cout << "Could not initialize SDL - " << SDL_GetError() << "\n"
                  << std::endl;
        return -1;
    }
    std::cout << "Init sdl successfull\n"
              << std::endl;
    // screen = SDL_CreateWindow("video Player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_w, screen_h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    // if (!screen)
    // {
    //     std::cout << "SDL: could not creeate window - exiting:\n"
    //               << SDL_GetError() << "\n"
    //               << std::endl;
    //     return -1;
    // }
    // sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
    // sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width, pCodecCtx->height);
}

bool MediaProcess::inputVideoQuene(const AVPacket &pkt)
{
    int iret = 0;
    std::cout << "In input\n"
              << std::endl;
    if (iret = av_dup_packet((AVPacket *)&pkt) < 0)
    {
        std::cout << "Input failed: " << iret << "\n"
                  << std::endl;
        return false;
    }
    try
    {
        mConditon_Video->Lock();
        mVideoPacktList.push_back(pkt);
        mConditon_Video->Signal();
        mConditon_Video->Unlock();
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Caught exception: " << ex.what() << std::endl;
    }
    std::cout << "inputVideoQuene: successful" << std::endl;

    return true;
}

void MediaProcess::clearVideoQuene()
{
    mConditon_Video->Lock();
    for (AVPacket pkt : mVideoPacktList)
    {
        av_packet_unref(&pkt);
    }
    mVideoPacktList.clear();
    mConditon_Video->Unlock();
}