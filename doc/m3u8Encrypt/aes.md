# HLS AES加密
## HLS AES加密介绍

HLS AES加密是一种用于保护HLS流内容安全的加密技术。它通过将HLS媒体文件进行分段，并使用AES加密算法对每个片段进行加密，从而防止未经授权的访问和内容盗取。
HLS AES加密的功能主要包括以下方面：
- 内容保护：使用AES加密算法对HLS媒体片段进行加密，确保媒体内容在传输和存储过程中的安全性。只有拥有相应密钥的用户才能解密和播放内容。

- 防止盗链：通过加密HLS媒体片段，防止未经授权的第三方站点直接链接到您的媒体内容。只有通过合法的许可方式获得密钥的用户才能成功解密并播放媒体。

- 安全传输：加密后的HLS媒体片段在传输过程中具有更高的安全性，即使在不安全的网络环境下，也能有效保护媒体内容的机密性。

- 多级加密：HLS AES加密支持多种加密模式和密钥管理方案，可以根据需求选择合适的加密算法和密钥管理策略，提供更高级别的安全保护。

- 通过使用HLS AES加密，您可以保护您的媒体内容免受未经授权的访问和盗取，确保内容在传输和播放过程中的安全性和机密性。
  
在HLS加密中，有几个重要字段和参数用于描述和控制加密操作。

- EXT-X-KEY: 这是一个标签，用于指定HLS媒体流的加密参数。它包含了以下几个属性：
- METHOD：指定使用的加密方法，常见的有AES-128、SAMPLE-AES等。
- URI: 这是密钥的URL或文件路径，用于指定加密所需的密钥。通常，密钥会单独保存在另一个文件中或通过网络进行获取。
- IV: 这是初始化向量(Initialization Vector)，用于AES加密过程中的初始状态。它与密钥一起用于生成加密密钥流。
示例如下
```m3u8
#EXTM3U
#EXT-X-VERSION:3
#EXT-X-TARGETDURATION:129
#EXT-X-MEDIA-SEQUENCE:0
#EXT-X-PLAYLIST-TYPE:VOD
#EXT-X-KEY:METHOD=AES-128,URI="http://192.168.1.247/aes.key",IV=0x0102030405060707090a0b0c0d0e0f10
#EXTINF:129.221022,
file0.ts
#EXT-X-ENDLIST
```
## HLS AES加密实现
```sh
ffmpeg -i cat.flv -c:v libx264 -c:a copy -f hls -hls_time 180 -hls_list_size 0 -hls_key_info_file enc.keyinfo -hls_playlist_type vod -hls_segment_filename ./file%d.ts ./playlist.m3u8
```
HLS加密需要key文件和IV，可以选择使用OpenSSL手动生成key，和IV，然后将其分别写入key文件和keyinfo文件（文件名并非固定，可随意命名，此处分别命名为encrypt.key和encrypt.keyinfo），然后通过ffmpeg对源视频进行切片及AES加密。手动生成命令如下
- 生成encrypt.key
    ```sh
    openssl rand 16 > [密钥存放位置]

    ```
- 生成IV
    ```sh
    openssl rand -hex 16
    # 假设本次生成的字符串为：2e38dfa3b8e04ef036e3e09d2bca08e7
    ```
- keyinfo文件内容如下
  - 第一行为解密文件的url，也可以替换为加密文件的路径，用于播放时，播放器寻找key文件
  - 第二行为加密文件的路径
  - 第三行为上述生成的IV
    encrypt.keyinfo
    ```
    http://localhost:8000/encrypt.key
    D:\video\test\encrypt.key
    2e38dfa3b8e04ef036e3e09d2bca08e7
    ```
如果不想一步一步的手动生成，也可使用下面的python脚本，修改url和需要存放加密文件的路径，运行脚本，就会自动生成16位AES密钥和IV，并将其写入encrypt.key和encrypt.keyinfo
- key生成脚本
  generat_key.py
  ```python3
  import os
  import binascii


  def generate_key(file_path: str, key_url=None, length=16) -> str:
      # 随机生成16位AES密钥
      key = os.urandom(length)
      # 随机生成16位IV
      iv = os.urandom(length)
      # 将IV转化为16进制，用字符串表示，生成的16进制字符串将具有双倍的长度，因为每个字节对应两个十六进制字符
      iv_string = binascii.hexlify(iv).decode('utf-8')
      file_name = f"{file_path}/encrypt.key"
      key_info = f"{file_path}/encrypt.keyinfo"
      if key_url is None:
          key_url = file_name
      # 将密钥写入key文件
      with open(file_name, 'wb') as f:
          f.write(key)
      # 将密钥信息和IV写入keyinfo文件
      with open(key_info, 'wb') as f:
          f.write(key_url.encode('utf-8'))
          f.write('\n'.encode('utf-8'))
          f.write(file_name.encode('utf-8'))
          f.write('\n'.encode('utf-8'))
          f.write(iv_string.encode('utf-8'))
      return file_name


  if __name__ == "__main__":
      ret = generate_key("D:/video/aes/test")
      print(f"key file path < {ret} >")

  ```
生成encrypt.key和encrypt.keyinfo文件后，执行命令对源视频进行切片及AES加密
```sh
ffmpeg -i cat.flv -c:v libx264 -c:a copy -f hls -hls_time 10 -hls_list_size 0 -hls_key_info_file encrypt.keyinfo -hls_playlist_type vod -hls_segment_filename ./file%d.ts ./playlist.m3u8
```
> 注意，需要先安装ffmpeg，安装方法可参考ffmpeg官网https://ffmpeg.org/download.html#build-windows。如果不想自己编译，可直接下载官方的exe文件

生成的m3u8文件如下
```
#EXTM3U
#EXT-X-VERSION:3
#EXT-X-TARGETDURATION:18
#EXT-X-MEDIA-SEQUENCE:0
#EXT-X-PLAYLIST-TYPE:VOD
#EXT-X-KEY:METHOD=AES-128,URI="D:/video/aes/test/encrypt.key",IV=0x2e38dfa3b8e04ef036e3e09d2bca08e7
#EXTINF:11.886878,
file0.ts
#EXTINF:11.136122,
......省略......
file11.ts
#EXT-X-ENDLIST
```
## 可能遇到的问题
当m3u8文件中，URL为文件路径时，直接播放playlist.m3u8文件可能会因为协议类型而无法找到encrypt.key，可以通过ffply加上`-allowed_extensions ALL`参数，即可正常播放视频。添加了`-allowed_extensions ALL`选项来允许处理所有类型的文件，包括具有非常见扩展名的文件。完整命令如下：
```
ffplay -allowed_extensions ALL .\playlist.m3u8
```