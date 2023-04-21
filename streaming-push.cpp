#ifdef __cplusplus
extern "C"
{
#endif
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/time.h>
#ifdef __cplusplus
}
#endif
#include <stdio.h>
#include <iostream>

/**
 * compile:
 *  g++ -o output_file analysis_stream.cpp -lavformat -lavcodec -lavutil
 */

int main()
{
    AVFrame *pFrame;
    AVPacket pkt;
    AVOutputFormat *out = NULL;
    AVCodecContext *codec = NULL;
    AVFormatContext *in_ctx = NULL, *out_ctx = NULL;
    char *in_file = "/home/video/recode/1681976244-bears.flv";
    char *out_url = "rtmp://192.168.1.247:1935/out/test";
    int videoindex;
    int ret;
    int i;
    int frame_index = 0;
    int64_t start_time = 0;

    avformat_network_init();

    if ((ret = avformat_open_input(&in_ctx, in_file, NULL, NULL)) < 0)
    {
        std::cout << "Could not open input file" << std::endl;
        goto end;
    }
    if ((ret = avformat_find_stream_info(in_ctx, NULL)) < 0)
    {
        std::cout << "Failed to retrieve input stream information" << std::endl;
        goto end;
    }
    for (i = 0; i < in_ctx->nb_streams; i++)
    {
        if (in_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoindex = i;
            break;
        }
    }
    av_dump_format(in_ctx, 0, in_file, 0);
    if ((ret = avformat_alloc_output_context2(&out_ctx, NULL, "flv", out_url)) < 0)
    {
        std::cout << "Open push streaming url failed" << std::endl;
        goto end;
    }
    if (!out_ctx)
    {
        std::cout << "Could not create output context" << std::endl;
    }
    /**
     * oformat字段指向输出的AVOutputFormat结构体，
     * 它是一个包含有封装格式相关信息的结构体。
     * oformat描述了输出的封装格式、其对应的文件扩展名、一些特殊的标志等
     */
    out = out_ctx->oformat;
    for (i = 0; i < in_ctx->nb_streams; i++)
    {
        AVStream *in_stream = in_ctx->streams[i];
        AVStream *out_stream = avformat_new_stream(out_ctx, in_stream->codec->codec);
        if (!out_stream)
        {
            std::cout << "Failed allocating output stream" << std::endl;
            // AVERROR_UNKNOWN 错误码，代表未知错误
            ret = AVERROR_UNKNOWN;
            goto end;
        }
        if ((ret = avcodec_copy_context(out_stream->codec, in_stream->codec)) < 0)
        {
            std::cout << "Failed to copy context from input to output stream codec context" << std::endl;
            goto end;
        }
        out_stream->codec->codec_tag = 0;
        if (out_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            // out_ctx->oformat->flags编解码器的选项和状态
            out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    av_dump_format(out_ctx, 0, out_url, 1);
    /**
     * AVFMT_NOFILE是其中一个标志位，
     * 表示输出数据不需要使用物理文件来存储，
     * 而是可以通过其他途径进行输出，
     * 例如网络传输
     */
    if (!(out->flags & AVFMT_NOFILE))
    {
        // 打开url
        if ((ret = avio_open(&out_ctx->pb, out_url, AVIO_FLAG_WRITE)) < 0)
        {
            std::cout << "Could not open output URL " << out_url << std::endl;
            goto end;
        }
    }
    if ((ret = avformat_write_header(out_ctx, NULL)) < 0)
    {
        std::cout << "Error occurred when opening output URL" << std::endl;
        goto end;
    }
    std::cout << "Output srteam message" << std::endl;
    av_dump_format(out_ctx, 0, out_url, 1);
    start_time = av_gettime();
    // 初始化 AVFrame
    pFrame = av_frame_alloc();
    /**
     * 上面部分是对输入视频的格式，编码等信息进行解析
     * 对输出的数据格式，编码等信息进行初始化
     * 下面的循环中是对视频数据的处理
     */
    while (1)
    {
        AVStream *in_stream, *out_stream;
        // 读取音视频帧数据
        if ((ret = av_read_frame(in_ctx, &pkt)) < 0)
        {
            std::cout << "Failed to read frame" << std::endl;
            break;
        }
        int got_picture = 0;
        // 判断时间戳是否有效 AV_NOPTS_VALUE代表无效时间戳
        if (pkt.pts == AV_NOPTS_VALUE)
        {
            // 获取时间戳的精度
            AVRational time_base1 = in_ctx->streams[videoindex]->time_base;
            // 计算两帧之间的持续时间
            int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(in_ctx->streams[videoindex]->r_frame_rate);
            // 计算时间戳
            pkt.pts = (double)(frame_index * calc_duration) / (double)(av_q2d(time_base1) * AV_TIME_BASE);
            pkt.dts = pkt.pts;
            // 设置输出pkt的持续时间
            pkt.duration = (double)calc_duration / (double)(av_q2d(time_base1) * AV_TIME_BASE);
        }
        // 等待当前帧应该播放的时间
        if (pkt.stream_index == videoindex)
        {
            AVRational time_base = in_ctx->streams[videoindex]->time_base;
            AVRational time_base_q = {1, AV_TIME_BASE};
            int64_t pts_time = av_rescale_q(pkt.dts, time_base, time_base_q);
            int64_t now_time = av_gettime() - start_time;
            if (pts_time > now_time)
                av_usleep(pts_time - now_time);
        }
        in_stream = in_ctx->streams[pkt.stream_index];
        out_stream = out_ctx->streams[pkt.stream_index];
        // 将输入流的时间戳和时基转换为输出流的时间戳和时基，并且计算包的持续时间
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        if (pkt.stream_index == videoindex)
        {
            frame_index++;
        }
        // 向输出文件写入一个数据包
        if ((ret = av_interleaved_write_frame(out_ctx, &pkt)) < 0)
        {
            std::cout << "Error muxing packet" << std::endl;
            break;
        }
        av_free_packet(&pkt);
    }
end:
    // 释放资源
    avformat_close_input(&in_ctx);
    if (out_ctx && !(out_ctx->flags & AVFMT_NOFILE))
        avio_close(out_ctx->pb);
    avformat_free_context(out_ctx);
    if (ret < 0 && ret != AVERROR_EOF)
    {
        printf("Error occurred.\n");
        return -1;
    }
    return 0;
}
