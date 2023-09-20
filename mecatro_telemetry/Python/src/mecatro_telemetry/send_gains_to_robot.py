
GAINS = [1.5,   # MAHONY_TAU
         8.0,   # KP
         0.05,  # KD
         0.05,  # KP_PHI
         0.015, # KD_PHI
         ]

def send_gains_to_robot(gains, serial):
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
    send_array[-1] = checksum

    serial.write(send_array)