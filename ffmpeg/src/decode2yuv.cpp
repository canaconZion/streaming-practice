/**
 * compile:
 *  g++ -o toyuv decode2yuv.cpp  -ID:\soft\mingw\mingw64\msys\home\zkPlayer-0.1\include -LD:\soft\mingw\mingw64\msys\home\zkPlayer-0.1/lib -lavutil -lavformat -lavcodec -lavutil -lswscale
*/

#include <iostream>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
}

#include <Windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    AVFormatContext *pFormatCtx;
    int i, videoindex;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;
    AVFrame *pFrame, *pFrameYUV;
    unsigned char *out_buffer;
    AVPacket *packet;
    int y_size;
    int ret, got_picture;
    struct SwsContext *img_convert_ctx;

    char filepath[] = "D:/video/videos/test.mp4";
    FILE *fp_yuv = fopen("mecha.yuv", "wb+");

    av_register_all();
    avformat_network_init();
    pFormatCtx = avformat_alloc_context();
    if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0)
    {
        std::cout << "Couldn't open input stream.\n"
                  << std::endl;
        return -1;
    }
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        std::cout << "Couldn't find stream info.\n"
                  << std::endl;
        return -1;
    }

    videoindex = -1;
    for (i - 0; i < pFormatCtx->nb_streams; i++)
    {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoindex = i;
            break;
        }
    }
    if (videoindex == -1)
    {
        std::cout << "Did not find a video stream.\n"
                  << std::endl;
        return -1;
    }
    pCodecCtx = pFormatCtx->streams[videoindex]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL)
    {
        std::cout << "Could not open codec.\n"
                  << std::endl;
        return -1;
    }
    if(avcodec_open2(pCodecCtx, pCodec,NULL)<0){
        std::cout << "Could not open codec.\n" <<std::endl;
        return -1;
    }
    pFrame = av_frame_alloc();
    pFrameYUV = av_frame_alloc();
    // 在堆上分配指定大小的内存块
    out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->width, 1));
    // 设置图像数据的指针数组和行大小
    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer,
                         AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);
    // if(ret < 0){
    //     std::cout << "Could not fill arrays.\n"
    //               << std::endl;
    //     return -1;
    // }
    // else{
    //     std::cout << "Fill arrays successful.\n"
    //               << std::endl;
    // }
    packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    std::cout << "-------------------------File Information--------------------\n" << std::endl;
    av_dump_format(pFormatCtx,0,filepath,0);
    std::cout << "-------------------------------------------------------------\n" << std::endl;
    img_convert_ctx = sws_getContext(pCodecCtx->width,pCodecCtx->height,pCodecCtx->pix_fmt,
        pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC,NULL,NULL,NULL);
    
    while (av_read_frame(pFormatCtx, packet)>=0)
    {
        if (packet->stream_index == videoindex)
        {
            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
            if (ret < 0)
            {
                std::cout << "Decode Error.\n"
                          << std::endl;
                return -1;
            }
            if (got_picture)
            {
                // 图像像素格式转换
                sws_scale(img_convert_ctx, (const unsigned char *const *)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
                          pFrameYUV->data, pFrameYUV->linesize);

                y_size = pCodecCtx->width * pCodecCtx->height;
                fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);     // Y
                fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_yuv); // U
                fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_yuv); // V
                std::cout << "Succeed to decode 1 frame!\n"
                          << std::endl;
            }
        }
        av_free_packet(packet);
    }


	// while (1) {
	// 	ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
	// 	if (ret < 0)
	// 		break;
	// 	if (!got_picture)
	// 		break;
	// 	sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, 
	// 		pFrameYUV->data, pFrameYUV->linesize);
 
	// 	int y_size=pCodecCtx->width*pCodecCtx->height;  
	// 	fwrite(pFrameYUV->data[0],1,y_size,fp_yuv);    //Y 
	// 	fwrite(pFrameYUV->data[1],1,y_size/4,fp_yuv);  //U
	// 	fwrite(pFrameYUV->data[2],1,y_size/4,fp_yuv);  //V
 
	// 	printf("Flush Decoder: Succeed to decode 1 frame!\n");
	// }
    sws_freeContext(img_convert_ctx);
    fclose(fp_yuv);

    av_frame_free(&pFrameYUV);
    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);

    return 0;
}