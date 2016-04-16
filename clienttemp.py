import socket
import time

s = socket.socket()
s.connect(("127.0.0.1",9999))
f=open ("recording.wav", "rb") 
l = f.read(1024)
while (l):
    s.send(l)
    l = f.read(1024)
s.close()
