#ifdef __cplusplus
extern "C"
{
#endif
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/time.h>
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#ifdef __cplusplus
}
#endif
#include <iostream>

/**
 * compile:
 *  gcc -o img_save img_save.cpp -lavformat -lavcodec -lavutil -lstdc++ -lswscale
 */

void SaveFrame(AVFrame *pFrame, int width, int height, int index)
{

    FILE *pFile;
    char szFilename[32];
    int y;

    // Open file
    sprintf(szFilename, "img/frame%d.ppm", index); // 文件名
    pFile = fopen(szFilename, "wb");

    if (pFile == nullptr)
        return;

    // Write header
    fprintf(pFile, "P6 %d %d 255", width, height);

    // Write pixel data
    for (y = 0; y < height; y++)
    {
        fwrite(pFrame->data[0] + y * pFrame->linesize[0], 1, width * 3, pFile);
    }

    // Close file
    fclose(pFile);
}

int main(int argc, char *argv[])
{
    AVFormatContext *pFormatCtx;
    AVFrame *pFrame, *pFrameRGB;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;
    SwsContext *img_convert_ctx = NULL;
    AVPacket *packet;

    int numBytes;
    int i, videoStream;
    int index = 0;
    int ret, got_picture;
    uint8_t *out_buffer;

    const char *file_path = "/home/zion/video/videos/mecha.mp4"; // 自己根据路径定义

    pFormatCtx = avformat_alloc_context();
    if (avformat_open_input(&pFormatCtx, file_path, nullptr, nullptr) != 0)
    {
        printf("can't open the file.");
        return -1;
    }
    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0)
    {
        printf("Could't find stream infomation.");
        return -1;
    }
    videoStream = -1;
    for (i = 0; i < pFormatCtx->nb_streams; i++)
    {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = i;
        }
    }
    if (videoStream == -1)
    {
        printf("Didn't find a video stream.");
        return -1;
    }

    // pCodecCtx = pFormatCtx->streams[videoStream]->codec;//编解码器格式
    pCodecCtx = avcodec_alloc_context3(NULL);
    avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoStream]->codecpar);
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id); // 编解码器的抽象

    if (pCodec == nullptr)
    {
        printf("Codec not found.");
        return -1;
    }
    if (avcodec_open2(pCodecCtx, pCodec, nullptr) < 0)
    {
        printf("Could not open codec.");
        return -1;
    }
    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();
    // 初始化一个SwsContext 成功后返回SwsContext 类型的结构体
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
                                     pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
                                     AV_PIX_FMT_RGB24, SWS_BICUBIC, nullptr, nullptr, nullptr);
    // 计算给定宽度和高度的图片的字节大小
    numBytes = avpicture_get_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
    // 内存分配
    out_buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
    // 将out_buffer指向的数据填充到picture内，但并没有拷贝，只是将picture结构内的data指针指向了out_buffer的数据。
    avpicture_fill((AVPicture *)pFrameRGB, out_buffer, AV_PIX_FMT_RGB24,
                   pCodecCtx->width, pCodecCtx->height);

    int y_size = pCodecCtx->width * pCodecCtx->height;

    packet = (AVPacket *)malloc(sizeof(AVPacket)); // 分配一个packet
    av_new_packet(packet, y_size);                 // 分配packet的数据 分配数据包的有效负载并用默认值初始化其字段

    while (1)
    {
        if (av_read_frame(pFormatCtx, packet) < 0) // 从流中读取读取数据到Packet中
        {
            break; // 这里认为视频读取完了
        }

        // 7.视频里面的数据是经过编码压缩的，因此这里我们需要将其解码：
        if (packet->stream_index == videoStream)
        {
            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
            // 对Packet读取的数据进行解码 结果保存在pFrame
            // 如果没有帧可以被解压，得到的图像指针got_picture为零，否则它是非零的。

            if (ret < 0)
            {
                printf("decode error.");
                return -1;
            }
            // 8.基本上所有解码器解码之后得到的图像数据都是YUV420的格式，而这里我们需要将其保存成图片文件。
            // 因此需要将得到的YUV420数据转换成RGB格式，转换格式也是直接使用FFMPEG来完成：
            if (got_picture)
            {
                sws_scale(img_convert_ctx,
                          (uint8_t const *const *)pFrame->data,
                          pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data,
                          pFrameRGB->linesize);
                if (index % 300 == 0)
                    SaveFrame(pFrameRGB, pCodecCtx->width, pCodecCtx->height, index); // 保存图片
                index++;
                // if (index > 3020) return 0;
            }
        }
        av_free_packet(packet);
    }
    avformat_close_input(&pFormatCtx);
    av_frame_free(&pFrameRGB);
    av_frame_free(&pFrame);
    avcodec_free_context(&pCodecCtx);
    sws_freeContext(img_convert_ctx);
    return 0;
}