#!/usr/bin/env python3

import socket
import struct

ETHER_TYPE = 0x0806
IFACE = "eth0"

if __name__ == "__main__":
    dst_mac = struct.pack("!6B", 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF)    
    src_mac = struct.pack("!6B", 0x08, 0x00, 0x27, 0xDD, 0x15, 0x89)
    ether_type = struct.pack("!H", ETHER_TYPE) 
    eth_hdr = dst_mac + src_mac + ether_type
    
    sock = socket.socket(socket.AF_PACKET, socket.SOCK_RAW)
    sock.bind((IFACE, 0))
    sock.send(eth_hdr)
    
    sock.close()
    
    
    
