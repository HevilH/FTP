import socket

size = 8192

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("127.0.0.1", 9876))
num = 1
try:
  while True:
    data, address = sock.recvfrom(size)
    data = str(num) + ' ' + data 
    sock.sendto(data.upper(), address)
    num += 1
finally:
  sock.close()