import os
drives = []
numDrives = 0
for y in range(65, 91):
    if os.path.exists(chr(y) + ":"):  
        drives.append(chr(y) + ":")
        numDrives = numDrives + 1
        print(numDrives)
        print(drives)

