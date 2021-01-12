import serial

from datetime import date
from datetime import datetime

ser = serial.Serial("/dev/ttyUSB0", baudrate = 115200)

while True:

    ser.read(1);

    today = date.today()
    now = datetime.now()

    d = today.day
    m = today.month
    y = today.year - 2000
    w = today.weekday()

    ser.write(bytes([d]))
    ser.write(bytes([m]))
    ser.write(bytes([y]))
    ser.write(bytes([w]))

    H = now.strftime("%H")
    M = now.strftime("%M")
    S = now.strftime("%S")

    ser.write(bytes([int(H)]))
    ser.write(bytes([int(M)]))
    ser.write(bytes([int(S)]))

    print('done');

