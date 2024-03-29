from socket import *
import time
import TC2

TXIP = "10.42.42.43"
MYUDPPORT = 5056
UDPPORT = 5056

channel = 6

# Create a UDP socket
# Notice the use of SOCK_DGRAM for UDP packets
serverSocket = socket(AF_INET, SOCK_DGRAM)

# Assign IP address and port number to socket
serverSocket.bind(('', MYUDPPORT))

offset = 40
while 1:
    time.sleep(0.1)
    message = bytearray(253)
    message.extend(channel.to_bytes(2, 'big'))
    serverSocket.sendto(message, (TXIP, UDPPORT))
    for t in range(0, 240):
        s = t*240+offset*240
        e = s+240
        message = bytearray(t.to_bytes(1, 'big'))
        for i in TC2.TC2[s:e]:
            message.extend(i.to_bytes(2, 'big'))

        serverSocket.sendto(message, (TXIP, UDPPORT))
