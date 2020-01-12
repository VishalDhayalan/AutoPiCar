import serial, time

uart_received = ""
serialESP = serial.Serial("/dev/ttyS0", baudrate=115200, timeout=0.02)

def initComms(timeout):
    global uart_received
    serialESP.write("Pi Ready.".encode())
    serialESP.flush()

    start_time = time.time()
    while uart_received != "OK":
        if time.time() - start_time >= timeout:
            return False
        else:
            uart_received = serialESP.read(2).decode("utf-8")
            time.sleep(0.1)
    
    while uart_received != "ESP Ready.":
        if time.time() - start_time >= timeout:
            return False
        else:
            uart_received = serialESP.read(30).decode("utf-8")
            time.sleep(0.1)

    serialESP.write("OK".encode())
    serialESP.flush()
    uart_received = ""
    return True

if initComms(3):
    print("UART Communication with ESP32 established!")
else:
    print("UART Handshake with ESP32 failed...")
    serialESP.close()
    exit