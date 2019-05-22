#!/usr/bin/env python3

import socket

DST_IP="0.0.0.0"
PORT=9999
QUEUE_SIZE=10

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind((DST_IP, PORT))
sock.listen(QUEUE_SIZE)

(clientSock, clientAddr) = sock.accept()
msg = clientSock.recv(100)
print("Message from: {0}:{1} msg: {2}".format(clientAddr[0], clientAddr[1], msg.decode())) 

clientSock.close()
sock.close()
