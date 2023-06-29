import socket, io, time
from PIL import Image

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(("192.168.43.54", 4444))

data_ = b""
while 1:
    data_ = s.recv(100000)
    print(data_)
    time.sleep(0.3)
    
