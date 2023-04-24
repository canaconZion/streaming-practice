#ifdef __cplusplus
extern "C"
{
#endif
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/time.h>
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#ifdef __cplusplus
}
#endif
#include <stdio.h>
#include <iostream>

/**
 * compile:
 *  g++ -o output_file streaming-push.cpp -lavformat -lavcodec -lavutil
 *  gcc -o push_s streaming-push.cpp -lavformat -lavcodec -lavutil -lstdc++
 */

int main()
{
    AVFrame *pFrame;
    AVPacket pkt;
    AVCodec *decodec;
    AVCodec *codec;
    AVOutputFormat *out = NULL;
    AVCodecContext *dc = NULL;
    AVCodecContext *c = NULL;
    AVFormatContext *in_ctx = NULL, *out_ctx = NULL;
    AVCodecParameters *in_codecpar = NULL;
    char *in_file = "/home/zion/video/videos/mecha.mp4";
    char *out_url = "rtmp://192.168.1.247:1935/out/test";
    char *codec_name = "libx264";
    char *s;
    int videoindex;
    int ret;
    int i;
    int frame_index = -1;
    int64_t start_time = 0;
    AVDictionary *opts = NULL;

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
    // 解码器
    // decodec = avcodec_find_decoder(AV_CODEC_ID_H264);
    decodec = avcodec_find_decoder(in_ctx->streams[videoindex]->codecpar->codec_id);
    if (!decodec)
    {
        std::cout << "Could not find decoder" << std::endl;
        goto end;
    }
    dc = avcodec_alloc_context3(decodec);
    if (!dc)
    {
        std::cout << "Could not allocate decoder" << std::endl;
        goto end;
    }
    // 设置解码器选项
    // avcodec_parameters_to_context(dc, in_ctx->streams[videoindex]->codecpar);
    if ((ret = avcodec_parameters_to_context(dc, in_ctx->streams[videoindex]->codecpar)) < 0)
    {
        std::cout << "Could not copy parameters to AVCodecContext" << std::endl;
        goto end;
    }
    if ((ret = avcodec_open2(dc, decodec, NULL)) < 0)
    {
        std::cout << "Failed to open H264 decoder" << std::endl;
        goto end;
    }
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
        if (videoindex == i)
        {
            // std::cout << "解码器" << in_stream->codec->codec_id << std::endl;
            in_codecpar = in_stream->codecpar;
            out_stream->codec->bit_rate = 400 * 1000;
        }
    }
    av_dump_format(out_ctx, 0, out_url, 1);
    /**
     * AVFMT_NOFILE是其中一个标志位，
     * 表示输出数据不需要使用物理文件来存储，
     * 而是可以通过其他途径进行输出，
     * 例如网络传输
     */
    // out_ctx->bit_rate = 40*1000;
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
    // 配置codec
    codec = avcodec_find_encoder_by_name(codec_name);
    if (!codec)
    {
        std::cout << "Codec " << codec_name << " not find" << std::endl;
        goto end;
    }
    c = avcodec_alloc_context3(codec);
    if (!c)
    {
        std::cout << "Could not allocate video codec context" << std::endl;
        goto end;
    }
    if ((ret = avcodec_parameters_to_context(c, in_codecpar)) < 0)
    {
        std::cout << "Could not copy parameters to AVCodecContext" << std::endl;
        goto end;
    }
    // 设置码率
    c->time_base = in_ctx->streams[videoindex]->time_base;
    // 设置编码器预设值
    if (codec->id == AV_CODEC_ID_H264)
        av_opt_set(c->priv_data, "preset", "slow", 0);
    if ((ret = avcodec_open2(c, codec, NULL)) < 0)
    {
        std::cout << "Could not open codec: " << codec_name << std::endl;
    }
    // 初始化 AVFrame
    pFrame = av_frame_alloc();
    if (!pFrame)
    {
        std::cout << "Could not allocate video frame" << std::endl;
        goto end;
    }
    pFrame->format = c->pix_fmt;
    pFrame->width = c->width;
    pFrame->height = c->height;
    if ((ret = av_frame_get_buffer(pFrame, 0)))
    {
        std::cout << "Could not allocate the video frame data" << std::endl;
        goto end;
    }
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
        // if ((ret = av_frame_make_writable(pFrame)) < 0)
        // {
        //     std::cout << "Frame data is not writeable" << std::endl;
        //     break;
        // }
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
            using namespace std;
            frame_index++;
            if ((ret = avcodec_send_packet(dc, &pkt)) < 0)
            {
                cout << "Error sending packet for decoding" << endl;
                cout << ret << endl;
                break;
            }
            while (ret >= 0)
            {
                ret = avcodec_receive_frame(dc, pFrame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    continue;
                // if (ret == AVERROR_EOF)
                //     break;
                else if (ret < 0)
                {
                    cout << "Error during decoding" << endl;
                    char errbuf[AV_ERROR_MAX_STRING_SIZE] = {0};
                    av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
                    cout << errbuf << endl;
                    goto end;
                }
            }
            // pFrame->pts = pkt.pts;
            ret = avcodec_send_frame(c, pFrame);
            if (ret < 0)
            {
                std::cout << "Error sending a frame for encoding" << ret << std::endl;
                char errStr[256] = { 0 };
                av_strerror(ret, errStr, sizeof(errStr));
                std::cout << "ERROR reason:" << errStr << std::endl;
                break;
            }
            // ret = avcodec_receive_packet(c, &pkt);
            // if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            //     break;
            // if (ret < 0)
            // {
            //     std::cout << "Error during encoding" << std::endl;
            //     break;
            // }
        }
        // 向输出文件写入一个数据包
        if ((ret = av_interleaved_write_frame(out_ctx, &pkt)) < 0)
        {
            std::cout << "Error muxing packet" << std::endl;
            break;
        }
        av_free_packet(&pkt);
        // av_frame_unref(pFrame);
    }
end:
    // 释放资源
    avformat_close_input(&in_ctx);
    if (out_ctx && !(out_ctx->flags & AVFMT_NOFILE))
        avio_close(out_ctx->pb);
    avformat_free_context(out_ctx);
    av_frame_free(&pFrame);
    avcodec_free_context(&c);
    // av_packet_free(&pkt);
    if (ret < 0 && ret != AVERROR_EOF)
    {
        printf("Error occurred.\n");
        return -1;
    }
    std::cout << "end" << std::endl;
    return 0;
}
