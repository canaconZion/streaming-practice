# FFMPEG+SDL简单视频播放器——视频快进
之前写过一个关于[视频播放器](https://www.cnblogs.com/Azion/p/17525955.html)的文章。播放器只简单实现了视频播放的功能，在此功能的基础上，给它加上一个视频快进的功能。
## 实现
### 添加参数
```cpp
// video play control
    bool do_seek = false; // 播放状态
    int64_t seek_length = 5; // 快进秒数
    int64_t seek_pos; // 视频跳转到的位置
    int seek_dir; // 视频跳转方向（快进或者后退）
    int rem_seek;
```

### 键盘监听
添加SDL键盘监听事件，通过键盘控制视频的快进后退。
```cpp
...
else if (event.type == SDL_KEYDOWN)
{
    if (event.key.keysym.sym == SDLK_SPACE)
        thread_pause = !thread_pause;
    // <- 控制后退
    else if (event.key.keysym.sym == SDLK_LEFT)
    {
        do_seek = true;
        seek_dir = 0;
    }
    // -> 控制快进
    else if (event.key.keysym.sym == SDLK_RIGHT)
    {
        do_seek = true;
        seek_dir = 1;
    }
    // q 退出播放
    else if (event.key.keysym.sym == SDLK_q)
    {
        thread_exit = 1;
    }
}
...
```
### 跳转参数获取
获取视频的时间基。根据时间基和视频实际跳转的秒数，计算出视频每次快进需要跳转的长度。
```cpp
...
for (i = 0; i < pFormatCtx->nb_streams; i++)
{
    if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
    {
        videoindex = i;
        AVRational timebase = pFormatCtx->streams[i]->time_base; // 时间基
        printf("TimeBase: %d/%d\n", timebase.num, timebase.den);
        rem_seek = seek_length;
        seek_length *= timebase.den; // 实际跳转的长度
        printf("seek length: %d\n", seek_length);
        break;
    }
}
...
```
### 快进
在循环中监听视频状态，当视频状态为快进时，执行视频快进或后退操作
```cpp
...
if (do_seek)
{
    printf("current packet pts: %d\n", packet->pts);
    // 快进
    if (seek_dir)
    {
        printf("Fast Forward %d s \n", rem_seek);
        seek_pos = packet->pts + seek_length;
    }
    // 后退
    else
    {
        printf("Rewind %d s \n", rem_seek);
        seek_pos = packet->pts - seek_length;
    }
    // 视频跳转
    if (av_seek_frame(pFormatCtx, videoindex, seek_pos, AVSEEK_FLAG_BACKWARD) < 0)
    {
        printf("Error while seeking\n");
        return -1;
    }
    // 视频跳转状态修改
    do_seek = false;
}
...
```
### 源码
https://github.com/canaconZion/streaming-practice/blob/main/ffmpeg/src/video_player.cpp

关于视频播放部分的代码，可以参考文章[《基于FFMPEG+SDL的简单的视频播放器分析 》](https://www.cnblogs.com/Azion/p/17525955.html)