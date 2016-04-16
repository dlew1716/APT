import socket
import sys
s = socket.socket()
s.bind(("localhost",9999))
s.listen(10) # Acepta hasta 10 conexiones entrantes.

while True:
    sc, address = s.accept()

    print address
    i=1
    f = open('file_'+ str(i)+".wav",'wb') #open in binary
    i=i+1
         
    # recibimos y escribimos en el fichero
    l = sc.recv(10)
    while (l):
    	f.write(l)
    	l = sc.recv(1024)
    	#print l
    f.close()


    sc.close()
    print 'Done'

s.close()
