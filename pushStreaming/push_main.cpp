#include "Rrconfig.h"
#include "streaming_p.h"

#include <iostream>
#include <string>

int main(int argc, int* argv[])
{
    bool ret;
    std::string rtmp_url;
    std::string Filepath;
    std::string Protocol;
    std::string HostName;
    std::string Port;
    std::string Key;
    std::string Pwd;
    char *input_url, *file_path;
    int count = 0;
    int i = 0;
    rr::RrConfig config;
    PushStreaming stream;

    ret = config.ReadConfig("config.ini");
    if(ret == false)
    {
        std::cout << "Failed to read config config.ini" << std::endl;
        return 1;
    }
    Filepath = config.ReadString("PUSHSTREAMING", "Filepath", "");
	Protocol = config.ReadString("PUSHSTREAMING", "Protocol", "");
	HostName = config.ReadString("PUSHSTREAMING", "HostName", "");
	Port = config.ReadString("PUSHSTREAMING", "Port", "");
	Key = config.ReadString("PUSHSTREAMING", "Key", "");
	Pwd = config.ReadString("PUSHSTREAMING", "Pwd", "");
	int Pushtimes = config.ReadInt("PUSHSTREAMING", "PushTimes", 0);
	int KeepPush = config.ReadInt("PUSHSTREAMING", "KeepPush", 0);
    rtmp_url = Protocol+"://"+HostName+":"+Port+"/"+Key+"/"+Pwd;
    input_url = (char*)rtmp_url.c_str();
	file_path = (char*)Filepath.c_str();
    std::cout << rtmp_url << std::endl;
    while(i < Pushtimes)
    {
        if(KeepPush==1){
			i++;
			printf("第 %d 次播放\n",i);
			fflush(stdout); // 强制刷新缓存区
		}
		else{
			count +=1;
			printf("第 %d 次循环播放\n",count);
		}
		ret = stream.PushStream(file_path, input_url);
        if(ret < 0)
        {
            std::cout << "push streaming failed" << std::endl;
            continue;
        }
    }
    return 0;
}