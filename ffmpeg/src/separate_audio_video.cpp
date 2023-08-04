/**
 *  分离视频中的视频流和音频流，分别保存为h264和aac
*/
#include <stdio.h>
#include <iostream>

extern "C"
{
#include "libavformat/avformat.h"
};
#define USB_H264BSF 0
using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cout << "miss argv\n"
             << endl;
        return -1;
    }
    AVOutputFormat *ofmt_a = NULL, *ofmt_v = NULL;
    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx_a = NULL, *ofmt_ctx_v = NULL;
    AVPacket pkt;
    int ret, i;
    int videoindex = -1, audioindex = -1;
    int frame_index = 0;

    const char *in_filename = argv[1];
    const char *out_filename_v = "out_video.h264";
    const char *out_filename_a = "out_audio.aac";

    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0)
    {
        cout << "Could not open input file\n"
             << endl;
        goto end;
    }
    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0)
    {
        cout << "Failed to retrieve input stream information" << endl;
        goto end;
    }
    avformat_alloc_output_context2(&ofmt_ctx_v, NULL, NULL, out_filename_v);
    if (!ofmt_ctx_v)
    {
        cout << "Could not create video output context\n"
             << endl;
        ret = AVERROR_UNKNOWN;
        goto end;
    }
    ofmt_v = ofmt_ctx_v->oformat;
    avformat_alloc_output_context2(&ofmt_ctx_a, NULL, NULL, out_filename_a);
    if (!ofmt_ctx_a)
    {
        cout << "Could not create audio output context\n"
             << endl;
        ret = AVERROR_UNKNOWN;
        goto end;
    }
    ofmt_a = ofmt_ctx_a->oformat;

    for (i = 0; i < ifmt_ctx->nb_streams; i++)
    {
        AVFormatContext *ofmt_ctx;
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVStream *out_stream = NULL;
        if (ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoindex = i;
            out_stream = avformat_new_stream(ofmt_ctx_v, in_stream->codec->codec);
            ofmt_ctx = ofmt_ctx_v;
        }
        else if (ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audioindex = i;
            out_stream = avformat_new_stream(ofmt_ctx_a, in_stream->codec->codec);
            ofmt_ctx = ofmt_ctx_a;
        }
        else
        {
            break;
        }
        if (!out_stream)
        {
            cout << "Failed allocating output stream." << endl;
            ret = AVERROR_UNKNOWN;
            goto end;
        }
        if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0)
        {
            cout << "Failed to copy context from input to output stream codec context" << endl;
            goto end;
        }
        out_stream->codec->codec_tag = 0;
        if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    cout << "================Input Video======================" << endl;
    av_dump_format(ifmt_ctx, 0, in_filename, 0);
    cout << "================Output Video======================" << endl;
    av_dump_format(ofmt_ctx_v, 0, out_filename_v, 1);
    cout << "================Output audio======================" << endl;
    av_dump_format(ofmt_ctx_a, 0, out_filename_a, 1);
    cout << "===================================================" << endl;
    if (!(ofmt_v->flags & AVFMT_NOFILE))
    {
        if (avio_open(&ofmt_ctx_v->pb, out_filename_v, AVIO_FLAG_WRITE) < 0)
        {
            cout << "Could not open output file " << out_filename_v << endl;
            goto end;
        }
    }

    if (!(ofmt_a->flags & AVFMT_NOFILE))
    {
        if (avio_open(&ofmt_ctx_a->pb, out_filename_a, AVIO_FLAG_WRITE) < 0)
        {
            cout << "Could not open output file " << out_filename_a << endl;
            goto end;
        }
    }
    if (avformat_write_header(ofmt_ctx_v, NULL) < 0)
    {
        cout << "Error occurred when opening video output file\n"
             << endl;
        goto end;
    }
    if (avformat_write_header(ofmt_ctx_a, NULL) < 0)
    {
        cout << "Error occurred when opening audio output file\n"
             << endl;
        goto end;
    }
    while (1)
    {
        AVFormatContext *ofmt_ctx;
        AVStream *in_stream, *out_stream;
        if (av_read_frame(ifmt_ctx, &pkt) < 0)
            break;
        in_stream = ifmt_ctx->streams[pkt.stream_index];

        if (pkt.stream_index == videoindex)
        {
            out_stream = ofmt_ctx_v->streams[0];
            ofmt_ctx = ofmt_ctx_v;
            printf("Write Video Packet. size:%d\tpts:%lld\n", pkt.size, pkt.pts);
        }
        else if (pkt.stream_index == audioindex)
        {
            out_stream = ofmt_ctx_a->streams[0];
            ofmt_ctx = ofmt_ctx_a;
            printf("Write Audio Packet. size:%d\tpts:%lld\n", pkt.size, pkt.pts);
        }
        else
        {
            continue;
        }
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        pkt.stream_index = 0;
        // Write
        if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0)
        {
            printf("Error muxing packet\n");
            break;
        }
        // printf("Write %8d frames to output file\n",frame_index);
        av_free_packet(&pkt);
        frame_index++;
    }

    av_write_trailer(ofmt_ctx_a);
    av_write_trailer(ofmt_ctx_v);
end:
    avformat_close_input(&ifmt_ctx);
    /* close output */
    if (ofmt_ctx_a && !(ofmt_a->flags & AVFMT_NOFILE))
        avio_close(ofmt_ctx_a->pb);

    if (ofmt_ctx_v && !(ofmt_v->flags & AVFMT_NOFILE))
        avio_close(ofmt_ctx_v->pb);

    avformat_free_context(ofmt_ctx_a);
    avformat_free_context(ofmt_ctx_v);

    if (ret < 0 && ret != AVERROR_EOF)
    {
        printf("Error occurred.\n");
        return -1;
    }
    return 0;
}
