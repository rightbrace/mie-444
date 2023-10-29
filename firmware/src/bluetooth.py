import serial
import serial.tools.list_ports

for port in serial.tools.list_ports.comports():
    print(port)

serialPort = serial.Serial(port='/dev/tty.Josef', baudrate=9600, timeout=0, parity=serial.PARITY_EVEN, stopbits=1)
size = 1024

while 1:
    data = serialPort.readline(size).decode("ascii").strip()

    if data:
        print(data)