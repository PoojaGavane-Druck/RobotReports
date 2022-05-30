


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
    arr = bytes(msg, 'UTF-8')
    print(arr)
    checkSum = getChecksum(arr, len(msg))
    msg = msg + checkSum + '\r\n'
    print(msg)

msg = input("Enter duci message without checksum: ")
calcChecksum(msg)