# -*- coding: utf-8 -*-
import socket
import requests
import re
import geoip2.database

class IP:
    @staticmethod
    def get_ip_public():
        """
        获取本机公网IP
        :return:
        """
        try:
            text = requests.get("http://txt.go.sohu.com/ip/soip").text
            ip_public = re.findall(r'\d+.\d+.\d+.\d+', text)[0]
            return ip_public
        except:
            return '0.0.0.0'

    @staticmethod
    def get_ip_local():
        """
        查询本机内网IP
        :return:
        """
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.connect(('8.8.8.8', 80))
            ip = s.getsockname()[0]
        finally:
            s.close()

        return ip


def get_localtor(ip):
    reader = geoip2.database.Reader('./GeoLite2-City.mmdb')
    response = reader.city(ip)
    print("IP:", response)
    print("你查询的IP的地理位置是:")

    print("地区：{}({})".format(response.continent.names["es"],
                                        response.continent.names["zh-CN"]))

    print("国家：{}({}) ，简称:{}".format(response.country.name,
                                                            response.country.names["zh-CN"],
                                                            response.country.iso_code))

    print("洲／省：{}({})".format(response.subdivisions.most_specific.name,
                                            response.subdivisions.most_specific.names["zh-CN"]))

    print("城市：{}({})".format(response.city.name, 
                                            response.city.names["zh-CN"]))

    print("经度：{}，纬度{}".format(response.location.longitude,
                                                response.location.latitude))

    print("时区：{}".format(response.location.time_zone))

    print("邮编:{}".format(response.postal.code))

if __name__ == '__main__':
    print("内网IP：{}".format(IP.get_ip_local()))
    print("外网IP：{}".format(IP.get_ip_public()))
    pub_ip = IP.get_ip_public()
    get_localtor(pub_ip)
