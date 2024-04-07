#!/usr/bin/env python
import time
from ipc_lib import *
from ipc_tranceiver import Tranceiver

if __name__ == '__main__':
    print('in main ipc_tranceiver\n')
    tranceiver = Tranceiver()
    own = "SVC0000"
    msg = "Hello world sent from SVC0000 to SVC0001"
    receiver = "SVC0001"
    cpid = layer_register_serv_to_cpid(own.encode('utf-8'))
    print(f'layer_register_serv_to_cpid {own} ret= {cpid}')
    while True:
        time.sleep(3)
        ret = tranceiver.msg_send(msg, receiver)
        print(f'tranceiver.msg_send ret=-------------------------------- {ret}')
        retstr = tranceiver.msg_recv()
        print(f'tranceiver.msg_recv retvalue=--------------------------- {retstr}')