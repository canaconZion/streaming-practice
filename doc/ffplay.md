# ffplay
## VideoState结构体
--------------------------------------------------------
VideoState 结构体的作用是在 FFplay 程序中管理和跟踪视频播放状态的关键数据结构。它具体的作用包括：

1. 管理视频文件的读取和解码：VideoState 中包含了一个 SDL_Thread 类型的成员变量 read_tid，它用于处理视频文件的读取和解码操作。这个线程负责从视频文件中读取数据，并将数据送入解码器进行解码。

2. 控制视频播放：abort_request 成员变量用于控制视频播放的终止请求。paused 和 last_paused 成员变量用于控制视频的暂停和播放状态。

3. 视频帧管理：VideoState 中包含了多个 FrameQueue 类型的队列，如 pictq、subpq、sampq，用于管理不同类型的视频帧，包括图像、字幕和音频帧。

4. 音频管理：VideoState 中包含了与音频相关的信息，包括音频时钟、音频解码器、音频流、音频缓冲等。这些信息用于确保音频的正确解码和同步。

5. 视频管理：与音频类似，VideoState 也包含了视频相关的信息，如视频时钟、视频解码器、视频流、视频帧队列等。它们用于管理和播放视频。

6. 字幕管理：VideoState 中包含了字幕相关的信息，包括字幕流、字幕帧队列等，以支持字幕的解码和显示。

7. 帧计时和同步：VideoState 中包含了用于帧计时和同步的信息，如 frame_timer、frame_last_returned_time 等，用于确保视频帧按照正确的速度播放。

8. 文件信息和控制参数：VideoState 中包含了文件名、窗口尺寸和位置、步进值等信息，用于控制和管理视频播放的参数。
```c
typedef struct VideoState {
    SDL_Thread *read_tid; // 用于处理视频文件的读取和解码的SDL线程
    AVInputFormat *iformat; // 视频文件的输入格式
    int abort_request; // 控制视频播放的终止请求
    int force_refresh; // 强制刷新视频帧
    int paused; // 控制视频暂停或播放
    int last_paused; // 跟踪最后一次暂停视频的时间戳
    int queue_attachments_req; // 控制是否需要在视频帧队列中排队附加信息
    int seek_req; // 进度控制标志，为1时表示需要seek
    int seek_flags; // seek的标志
    int64_t seek_pos; // seek的目标位置
    int64_t seek_rel; // seek的相对位置
    int read_pause_return; // 读取暂停返回值
    AVFormatContext *ic; // 视频文件格式上下文
    int realtime; // 是否实时模式

    Clock audclk; // 音频时钟
    Clock vidclk; // 视频时钟
    Clock extclk; // 外部时钟

    FrameQueue pictq; // 图像帧队列
    FrameQueue subpq; // 字幕帧队列
    FrameQueue sampq; // 音频帧队列

    Decoder auddec; // 音频解码器
    Decoder viddec; // 视频解码器
    Decoder subdec; // 字幕解码器

    int audio_stream; // 音频流索引

    int av_sync_type; // 音视频同步类型

    double audio_clock; // 音频时钟
    int audio_clock_serial; // 音频时钟的序列号
    double audio_diff_cum; /* used for AV difference average computation */
    double audio_diff_avg_coef;
    double audio_diff_threshold;
    int audio_diff_avg_count;
    AVStream *audio_st; // 音频流
    PacketQueue audioq; // 音频packet队列
    int audio_hw_buf_size; // 音频硬件缓冲大小
    uint8_t *audio_buf; // 音频缓冲
    uint8_t *audio_buf1; // 音频缓冲1
    unsigned int audio_buf_size; /* in bytes */
    unsigned int audio_buf1_size;
    int audio_buf_index; /* in bytes */
    int audio_write_buf_size; // 音频写缓冲大小
    int audio_volume; // 音频音量
    int muted; // 是否静音
    struct AudioParams audio_src; // 音频源参数
#if CONFIG_AVFILTER
    struct AudioParams audio_filter_src; // 音频过滤器源参数
#endif
    struct AudioParams audio_tgt; // 音频目标参数
    struct SwrContext *swr_ctx; // 音频重采样上下文
    int frame_drops_early; // 早期帧丢弃
    int frame_drops_late; // 晚期帧丢弃

    enum ShowMode {
        SHOW_MODE_NONE = -1, SHOW_MODE_VIDEO = 0, SHOW_MODE_WAVES, SHOW_MODE_RDFT, SHOW_MODE_NB
    } show_mode; // 显示模式
    int16_t sample_array[SAMPLE_ARRAY_SIZE];
    int sample_array_index;
    int last_i_start;
    RDFTContext *rdft; // 傅立叶变换上下文
    int rdft_bits;
    FFTSample *rdft_data;
    int xpos;
    double last_vis_time;
    SDL_Texture *vis_texture; // 可视化纹理
    SDL_Texture *sub_texture; // 字幕显示
    SDL_Texture *vid_texture; // 视频显示

    int subtitle_stream; // 字幕流索引
    AVStream *subtitle_st; // 字幕流
    PacketQueue subtitleq; // 字幕packet队列

    double frame_timer; // 帧计时器
    double frame_last_returned_time; // 上一帧返回时间
    double frame_last_filter_delay; // 上一帧滤波延迟
    int video_stream; // 视频流索引
    AVStream *video_st; // 视频流
    PacketQueue videoq; // 视频packet队列
    double max_frame_duration; // 最大帧持续时间
    struct SwsContext *img_convert_ctx; // 图像转换上下文
    struct SwsContext *sub_convert_ctx; // 字幕转换上下文
    int eof; // 是否到达文件尾

    char *filename; // 文件名
    int width, height, xleft, ytop; // 视频窗口尺寸和位置
    int step; // 步进值

#if CONFIG_AVFILTER
    int vfilter_idx; // 视频过滤器索引
    AVFilterContext *in_video_filter;   // 视频链中的第一个过滤器
    AVFilterContext *out_video_filter;  // 视频链中的最后一个过滤器
    AVFilterContext *in_audio_filter;   // 音频链中的第一个过滤器
    AVFilterContext *out_audio_filter;  // 音频链中的最后一个过滤器
    AVFilterGraph *agraph;              // 音频过滤器图
#endif

    int last_video_stream, last_audio_stream, last_subtitle_stream; // 上一个视频流、音频流、字幕流

    SDL_cond *continue_read_thread; // 继续读取线程
} VideoState;
```
--------------------------------------------------------------