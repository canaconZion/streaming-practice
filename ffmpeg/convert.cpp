#include <stdio.h>
#include <iostream>
#ifdef __cplusplus
extern "C" 
{
#endif
#include <libavformat/avformat.h>
#ifdef __cplusplus
};
#endif

/**
 * gcc -o convert convert.cpp -lavformat -lavcodec -lavutil -lstdc++
*/

int main(int argc, char* argv[])
{
    AVFormatContext *in_ctx = NULL, *out_ctx = NULL; 
    AVOutputFormat  *ofmt = NULL;
    AVPacket pkt;
    int ret;
    char *in_file = "/home/zion/video/videos/test.mp4";
    char *out_file = "/home/zion/video/videos/test_con.flv";
    int i;
    int frame_index = 0;

    // 打开视频源文件
    if((ret = avformat_open_input(&in_ctx, in_file, 0, 0)) < 0)
    {
        printf("Could not open input file\n");
        goto end;
    }
    // 获取视频源信息
    if((ret = avformat_find_stream_info(in_ctx, 0)) < 0)
    {
        printf("Failed to retrieve input stream inforation\n");
        goto end;
    }
    // 输出视频源信息到控制台
    av_dump_format(in_ctx, 0, in_file, 0);
    //分配输出格式的上下文
    avformat_alloc_output_context2(&out_ctx, NULL, NULL, out_file);
    if(!out_ctx)
    {
        printf("Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }
    for(i=0; i<in_ctx->nb_streams;i++)
    {
        AVStream *in_stream = in_ctx->streams[i];
        // 初始化输出流
        AVStream *out_stream = avformat_new_stream(out_ctx, in_stream->codec->codec);
        if(!out_stream)
        {
            printf("Failed allocating output stream\n");
            ret = AVERROR_UNKNOWN;
            goto end;
        }
        // 从输入流复制编码设置到输出流
        if(avcodec_copy_context(out_stream->codec, in_stream->codec) < 0)
        {
            printf("Failed to copy context from input to output stream codec context\n");
        }
        // codec 设置为0，使ffmpeg根据封装格式和编码格式自动选择合适的codec_tag
        out_stream->codec->codec_tag = 0;
        // 判断输出格式是否包含全局头信息AVFMT_GLOBALHEADER
        if(out_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        {
            out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }
    }
    // 输出输出流的信息
    av_dump_format(out_ctx, 0, out_file, 1);
    ofmt = out_ctx->oformat;
    // 通过AVFMT_NOFLIE 输出流是否需要保存到磁盘
    if(!(ofmt->flags & AVFMT_NOFILE))
    {
        if((ret = avio_open(&out_ctx->pb, out_file, AVIO_FLAG_WRITE))<0)
        {
            printf("Could not open output file '%s'",out_file);
            goto end;
        }
    }
    //写入文件头
    if(avformat_write_header(out_ctx, NULL)<0)
    {
        printf("Error occurred when opening output file\n");
        goto end;
    }
    while (1)
    {
        AVStream *in_stream ,*out_stream;
        // 读取packet数据包
        if(ret = av_read_frame(in_ctx, &pkt) < 0)
            break;
        in_stream = in_ctx->streams[pkt.stream_index];
        out_stream = out_ctx->streams[pkt.stream_index];

        // PTS/DTS转换
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;
        // 写入数据包
        if(av_interleaved_write_frame(out_ctx, &pkt) < 0)
        {
            printf("Error muxing packet\n");
            break;
        }
        printf("Write %8d frames to output file\n",frame_index);
        frame_index++;
    }
    av_write_trailer(out_ctx);

end:
    avformat_close_input(&in_ctx);
    if(out_ctx && ! (ofmt -> flags & AVFMT_NOFILE))
        avio_close(out_ctx->pb);
    avformat_free_context(out_ctx);

    return 0;
}