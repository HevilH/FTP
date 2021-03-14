import socket
 
size = 8192
 
try:
  sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
  for i in range(1, 51):
    sock.sendto(str(i), ("127.0.0.1", 9876))
  for i in range(1, 51):
    print sock.recv(size)
  sock.close()
 
except:
  print "cannot reach the server"