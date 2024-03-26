from socket import *

# Create a UDP socket
# Notice the use of SOCK_DGRAM for UDP packets
serverSocket = socket(AF_INET, SOCK_DGRAM)

# Assign IP address and port number to socket
serverSocket.bind(('', 5055))

channels = {}


def addchanip(ch, ip):
    channels.setdefault(ch, [])
    channels[ch].append(ip)


def inchannel(ch, ip):
    ips = channels[ch]
    #return other ip if there are two
    xip = ""
    #print("inch", ips, len(ips))
    if len(ips) > 1:
        if (ips[0] == ip):
            xip = ips[1]
        else:
            xip = ips[0]
    return xip


def channelfromip(ip):
    ch = 0
    #print (channels)
    for c, v in enumerate(channels):
        #print(ip,channels[v])
        if ip in channels[v]:
            ch = v
    return ch

while True:

        message, (rxaddress, port) = serverSocket.recvfrom(1024)
        # print (rxaddress)
        # print(".", end='')
        if message[0] == 253:
            channel = message[1]
            print("Connection from ", rxaddress)
            print("Channel", channel)

            if channel not in channels:
                addchanip(channel, rxaddress)
            else:
                if rxaddress not in channels[channel]:
                    addchanip(channel, rxaddress)
            print(channels[channel])
            print("")
        else:
            channel = channelfromip(rxaddress)
            if channel > 0:
                txip = inchannel(channel, rxaddress)
                #print("ch", channel, "rxAddr", rxaddress, "Toaddr", txip)
                if not txip == "":
                    serverSocket.sendto(message, (txip, 5055))

