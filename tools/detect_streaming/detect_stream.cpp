
/**
 *  检测输入视频源是否有流
*/
#include "detect_stream.h"

int detect_stream(char *url)
{
    AVFormatContext *pFormatCtx = NULL;
    int i, videoindex;
    int ret;
    char *strUrl = url;
    if ((ret = avformat_open_input(&pFormatCtx, strUrl, NULL, NULL)) < 0)
    {
        printf("Could not open input file.\n");
        goto end;
    }
    if ((ret = avformat_find_stream_info(pFormatCtx, NULL)) < 0)
    {
        printf("Could not find stream information.\n");
        goto end;
    }
    for (i = 0; i < pFormatCtx->nb_streams; i++)
    {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO && videoindex < 0)
        {
            videoindex = i;
        }
    }
    av_dump_format(pFormatCtx, 0, strUrl, 0);
end:
    avformat_close_input(&pFormatCtx);
    printf("Return result:%d\n", ret);
    return ret;
}
