
maxFileLength = 640 * 1024
padByte = 255
total = 0

fileName = input("Enter exact file name to convert: ")

try:
    inputFile = open(fileName, 'r')
    fileData = inputFile.read()
    fileLength = len(fileData)
    encodedData = fileData.encode()
    byteData = bytearray(encodedData)
    outFile = open("PM620TFW.cpp", "w", encoding='utf-8')
    outData = ""

    length = len(fileData)
    total = length
    print("File Length: " + str(total))
    
    if length > 1:

        #outData = "const char[" + str(length) +"] = {"
        for index in range(length - 1):
            outData = outData + str(ord(fileData[index])) + ', '
            if str(ord(fileData[index])) == '10':
                outData = outData + '13, \n'
                total = total + 1
                print(total)
            remaining = length - index
            
            print(remaining, str(ord(fileData[index])))

        outData = outData + '10, 13};'
        total = total + 2
        outData = "const char[" + str(total) +"] = {" + outData
        
        outFile.write(outData)

        print(outData)
    else:
        print("File too short")
except:
    print("File not found!")

