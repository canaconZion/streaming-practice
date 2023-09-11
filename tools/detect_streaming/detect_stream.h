#ifndef STREAM_DETECTION_H
#define STREAM_DETECTION_H

#ifdef __cplusplus
extern "C"
{
#endif
#include <libavformat/avformat.h>
#ifdef __cplusplus
}
#endif
#include <stdio.h>
#include <iostream>

int detect_stream(char *url);

#endif // STREAM_DETECTION_H
