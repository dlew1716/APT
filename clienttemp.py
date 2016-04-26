import socket
import time

s = socket.socket()
s.connect(("localhost",9999))
f=open ("recording.wav", "rb") 
s.send('herro')
l = f.read(1024)
while (l):
    s.send(l)
    l = f.read(1024)
s.close()
