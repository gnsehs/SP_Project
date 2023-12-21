import serial,time,pynmea2

dir = '/dev/serial0'
baud = 9600
sp = serial.Serial(dir, baudrate = baud, timeout = 0.5) # 시리얼 포트 
while True:
    
    strl = ''
    try:
        strl = sp.readline().decode().strip()
    except Exception as e:
        print(e)
    
    if strl.find('GGA') > 0:
        try:
            msg = pynmea2.parse(strl)
            print(msg.timestamp,'위도:',round(msg.latitude,6),'경도:',round(msg.longitude,6))
            lat = int(round(msg.latitude,6))
            lon = int(round(msg.longitude,6))
            # write to file
            f = open("data.txt",'w')
            f.write(str(lat))
            f.write("\n")
            f.write(str(lon))
            f.close()
        except Exception as e:
            print(e)
    time.sleep(0.1)
    