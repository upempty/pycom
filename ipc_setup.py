#!/usr/bin/env python
#ipc_setup.py
import time
from ipc_lib import *

ret = initContainers()
ret = initQueues()

if __name__ == '__main__':
    print('in main ipc_setup\n')
    while True:
        time.sleep(60)
        print('loop to sleep each 60s on 121.43.133.208 \n')