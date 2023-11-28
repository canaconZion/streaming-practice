# RTMP协议学习——Message（消息）
## 前言
之前通过对抓包数据的学习和分析，对RTMP协议有了一个整体的认知，大致了解了RTMP从建立连接到播放视频的流程,文章请看[《RTMP协议学习——从握手到播放》](https://holyzion.host/2023/11/10/RTMP%E5%8D%8F%E8%AE%AE%E5%AD%A6%E4%B9%A0%E2%80%94%E2%80%94%E4%BB%8E%E6%8F%A1%E6%89%8B%E5%88%B0%E6%92%AD%E6%94%BE/)。但是对于RTMP消息传输的载体还没有过多的分析。本文将会从Message的作用，构成，类型等方面，对RTMP协议进行进一步的研究和学习。
## Message的格式
RTMP的消息由两部分构成，分别是header和payload。
### Header
RTMP的header不是固定的。
### Payload