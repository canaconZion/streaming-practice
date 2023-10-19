- tcp三次握手
- rtmp三次握手
- connect c->s
    连接成功，服务端向客户端发送_result消息，否则发送_error消息
    ![](./img/connect.png)

![](./img/rtmpconn.png)

----------------------------

- chunk stream id 
    ![](./img/chunk.png)
- rtmp header
    ![](./img/rtmpheader.png)
- rtmp body
AMF英文全称Action Message Format
RTMP数据包的序列化就是按照AMF0的格式进行
    ![](./img/AMF.png)