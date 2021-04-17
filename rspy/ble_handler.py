import binascii
import time
import struct
from bluepy import btle
address = "D4:2D:FC:0C:F8:58"

temp_uuid = btle.UUID(0x2a6e)


class MyDelegate(DefaultDelegate):
    # Constructor (run once on startup)
    def __init__(self, params):
        DefaultDelegate.__init__(self)

    # func is caled on notifications
    def handleNotification(self, cHandle, data):
         print("Notification from Handle: 0x" + format(cHandle,
               '02X') + " Value: " + format(ord(data[0])))


p = btle.Peripheral(address)
p.setDelegate(MyDelegate())

try:
    ch = p.getCharacteristics(uuid=temp_uuid)[0]
   while 1:
       if p.waitForNotifications(1.0):
           continue

        print("Waiting...")
finally:
    p.disconnect()
