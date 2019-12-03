# Requires pySerial and keyboard
# python -m pip install pyserial keyboard

import serial
import serial.tools.list_ports
import keyboard
import socket
import select

LISTEN_PORT = 9999
VERBOSE = False
MAX_CONNECTIONS = 2

activeSerialPorts = {}
activeSockets = []


print("Starting relay. Press q to stop")

serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
serverSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
serverSocket.bind(('', LISTEN_PORT))
serverSocket.listen(5)
serverSocket.setblocking(False)
activeSockets.append(serverSocket)
print("Listening on port " + str(LISTEN_PORT))

def numActiveConnections():
	return len(activeSerialPorts) + len(activeSockets) - 1

def printStatus():
	print("Now connected to", len(activeSerialPorts), "serial port(s) and", (len(activeSockets) - 1), "TCP socket(s)")	

def sendData(source, data):
	if VERBOSE:
		if source in activeSockets:
			print("Relaying data from", source.getpeername(), "-", str(data))
		else:
			print("Relaying data from", str(source.port), "-", str(data))
		
	for portName in activeSerialPorts:
		serialPort = activeSerialPorts[portName]
		if not source is serialPort:
			serialPort.write(data)
			
	for sock in activeSockets:
		if (sock != source) and (sock != serverSocket):
			sock.send(data)

while True:
	if keyboard.is_pressed('q'):
		break
	
	# Check for new serial ports
	if numActiveConnections() < MAX_CONNECTIONS:
		availablePorts = serial.tools.list_ports.comports()
		
		for port in availablePorts:
			if not port.device in activeSerialPorts:
				try:
					activeSerialPorts[str(port.device)] = serial.Serial(port.device, 9600)
					print("Opened port " + str(port.device))
					printStatus()
				except:
					continue
	
	# Process serial ports
	disconnectedSerialPorts = []
	for portName in activeSerialPorts:
		try:
			serialPort = activeSerialPorts[portName]
			if serialPort.in_waiting > 0:
				inData = serialPort.read()
				sendData(serialPort, inData)
		except:
			disconnectedSerialPorts.append(portName)

	# Remove closed serial ports
	for portName in disconnectedSerialPorts:
		print("Lost connection to", portName)
		activeSerialPorts.pop(portName)
		printStatus()
		
	# Check for socket connections
	socketsToClose = []
	readable, writable, errored = select.select(activeSockets, [], [], 0)
	for s in readable:
		if s is serverSocket:
			clientSocket, address = serverSocket.accept()
			if numActiveConnections() < MAX_CONNECTIONS:
				activeSockets.append(clientSocket)
				print("Connection from", address)
				printStatus()
			else:
				print("Rejecting connection from", address, "- too many connections!")
				clientSocket.close()
		else:
			try:
				data = s.recv(1)
				if data:
					sendData(s, data)
				else:
					socketsToClose.append(s)
			except:
				socketsToClose.append(s)
	
	for s in socketsToClose:
		print("Closing socket", s.getpeername())
		s.close()
		activeSockets.remove(s)
		printStatus()
				
print("Shutting down..")
		
# Cleanup serial ports
for portName in activeSerialPorts:
	try:
		print("Disconnecting port", portName)
		serialPort = activeSerialPorts[portName]
		serialPort.close();
	except:
		print("Error closing port", portName)

# Cleanup sockets
print("Closing listening socket")
serverSocket.close()
activeSockets.remove(serverSocket)
for s in activeSockets:
	print("Closing socket", s.getpeername())
	s.close()