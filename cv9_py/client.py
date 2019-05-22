#!/usr/bin/env python3

import socket

DST_IP="127.0.0.1"
PORT=9999

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((DST_IP, PORT))

msg = input("Enter message:")
sock.send(msg.encode())
sock.close()
