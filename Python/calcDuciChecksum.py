


def getChecksum(arr, length):
    checksum = 0
    for i in range(0, length, 1):
        checksum = checksum + arr[i]
    checksum = checksum % 100
    if checksum < 10:
        checksum = '0' + str(checksum)
    else:
        checksum = str(checksum)
    return checksum

def calcChecksum(msg):
    msg = "#" + msg + ":"
    arr = bytes(msg, 'UTF-8')
    print(arr)
    checkSum = getChecksum(arr, len(msg))
    msg = msg + checkSum + '\r\n'
    myMessage = "Message with checksum: " + str(msg)
    print(myMessage)
    arrMsg = msg.encode('utf-8').hex()
    myMessage = "Message in HEX: " + str(arrMsg)
    print(myMessage)


while True:
    msg = input("\n\rEnter duci message without hash, colon and checksum: ")
    calcChecksum(msg)
    myIn = input("\n\rAnother message Y/N?: ")
    if myIn == 'Y' or myIn == 'y':
        continue
    else:
        break

print("Exiting.. bye bye")
