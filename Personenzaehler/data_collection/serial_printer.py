import serial


def readserial(comport, baudrate):

    try:
        ser = serial.Serial(port='/dev/ttyACM0',   baudrate=115200, timeout=.1)
    
        ser.readline()
    except:
        print('arduino on port ACM1')
        ser = serial.Serial(port='/dev/ttyACM1',   baudrate=115200, timeout=.1)

    while True:
        data = ser.readline().decode().strip()
        if data:
            print(data)


if __name__ == '__main__':

    readserial('/dev/ttyACM0', 115200)