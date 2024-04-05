#!/usr/bin/env python
#ipc_tranceiver.py
import time
from ipc_lib import *

class Tranceiver:
    def __init__(self,):
        print('Traceiver init \n')
        pass

    def msg_send(self, str):
        print('testing in ctypes\n')
        buf_id = layer_mem_reserve(str.encode('utf-8'))
        print(f'py1: buf_id = {buf_id} layer_mem_reserve')
        #need to fetch cpid
        cpid = 0
        ret = layer_push_queue(ctypes.c_int(cpid),ctypes.c_int(buf_id))
        print(f'py1: ret = {ret} layer_push_queue')

    def msg_recv(self):
        #need to fetch cpid
        cpid = 0
        buf_id = layer_pop_queue(ctypes.c_int(cpid))
        print(f'py1: buf_id = {buf_id} layer_pop_queue')
        retstr = layer_mem_receive(ctypes.c_int(buf_id))
        print(f'py1: ret = {retstr} layer_mem_receive')
        retstr2 = ctypes.string_at(retstr).decode('utf-8')
        newa = 'PP'.join(retstr2) # convert to msg structure in python defined in class members.
        print(f'py888: ret = from {retstr2} -> new {newa} layer_mem_receive')
        ret = layer_mem_release(ctypes.c_int(buf_id))
        print(f'py1: ret = {ret}  layer_mem_release')
        print(f'py after release= retstr2= {retstr2} in layer_mem_receive')

if __name__ == '__main__':
    print('in main ipc_tranceiver\n')
    server_or_client = int(sys.argv[1])
    print('sys param', server_or_client)
    str = "Hello tranceiver world111"
    tranceiver = Tranceiver()
    if server_or_client == 1:
        print('=====sended11111111111111111======\n')
        tranceiver.msg_send(str)
        str2 = "Hello2 tranceiver world2222"
        print('=====sended22222222222222222======\n')
        tranceiver.msg_send(str2)
        #msg_recv()
        #time.sleep(60)
        print('=====sended======\n')
    else:
        print('=====recv1======\n')
        tranceiver.msg_recv()
        time.sleep(10)
        print('=====recv2======\n')
        tranceiver.msg_recv()
        print('======received=====')

    print('in main end\n')

'''
#main init process
./ipc_setup.py

#service startup:
./ipc_tranceiver.py 1
./ipc_tranceiver.py 0

'''