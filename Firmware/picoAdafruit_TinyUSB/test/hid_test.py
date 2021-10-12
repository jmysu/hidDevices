# Install python3 HID package https://pypi.org/project/hid/
import hid
import time
# default is Adafruit VID
VID = 0x2E8A
PID = 0x00C0
print("Openning HID device with VID = 0x%X" % VID)

for dict in hid.enumerate(VID):
    print(dict)
    dev = hid.device()
    dev.open(VID, PID)
    if dev:
        print("dev=", dev)
        while True:
            # Get input from console and encode to UTF8 for array of chars.
            # hid generic inout is single report therefore by HIDAPI requirement
            # it must be preceeded with 0x00 as dummy reportID
            #str_out = b'\x02'
            #str_out += input("Send text to HID Device : ").encode('utf-8')
            #dev.write(str_out)
       
            #d = bytes([0x02, 0xE1])
            #dev.write( d )
            #print('write HID:"{}'.format(d))

            #str_in = dev.read(64, 100)
            #print("Received from HID Device:", str_in, '\n')
            #try:
            #   while True:
            #d = dev.read(64, 1000)
            #print('read HID: "{}"'.format(d))
            dev.write([2,0xc1,0x80,0x10,0x10,0x10,0,0])    
            time.sleep(1)
            dev.write([2,0xc1,0x10,0x80,0x10,0x10,0,0])
            time.sleep(1)
            dev.write([2,0xc1,0x10,0x10,0x80,0x10,0,0])
            time.sleep(1)

            dev.write([2,0xc2,0,0,0,0,0,0])
            l = dev.read(64,500)
            print([hex(x) for x in l])
            
        dev.close()    


