import struct
import socket
import time

server_host = '192.168.221.4'
server_port = 8888

# 서버에 연결
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((server_host, server_port))
    
    while True:
        message = "TWO"
        s.send(message.encode('utf-8'))
        time.sleep(2)
        

    # 데이터 전송
    