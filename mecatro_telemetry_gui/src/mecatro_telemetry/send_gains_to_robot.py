
import struct

def send_gains_to_robot(gains, socket):
    '''
    Send gains to robot.
        - gains: list of float32 to send
        - serial: serial object
    '''
    send_array = bytearray(4 * len(gains) + 2)
    send_array[0] = 0xFF
    for i, f in enumerate(gains):
        send_array[4 * i +1:4 * (i + 1) + 1] = struct.pack("f", f)

    checksum = sum(send_array[1:-1]) % 256
    send_array[-1] = 255 - checksum
    print("sendign")
    for a in send_array:
        print(int(a))
    socket.sendall(send_array)