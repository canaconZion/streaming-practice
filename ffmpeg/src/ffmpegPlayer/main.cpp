#include "fplayer/simple_player.hpp"

int WinMain(int argc, char *argv[])
{
    MediaProcess a;
    a.filepath = "D:\\workdir\\ffmpeg\\streaming-practice\\ffmpeg\\bin\\cat.flv";
    a.media_decode();
}