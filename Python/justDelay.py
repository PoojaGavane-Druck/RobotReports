import time

def sleepFor30s():
    count = 30
    print("Waiting for 30 seconds")
    while count > 0:
        count = count - 1
        if count % 5 == 0:
            print(count)
        time.sleep(1)

sleepFor30s()

