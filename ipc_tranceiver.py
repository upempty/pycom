#!/usr/bin/env python
#ipc_tranceiver.py

from ipc_lib import *

class Tranceiver:
    def __init__(self,):
        print('Traceiver init \n')

    def msg_send(self, str, receiver):
        buf_id = layer_mem_reserve(str.encode('utf-8'))
        print(f'msg_send: buf_id = {buf_id} layer_mem_reserve')
        if buf_id == -1:
            return -1
        cpid = layer_retrieve_cpid_from_serv(receiver.encode('utf-8'))
        print(f'msg_send: cpid of receiver= {cpid} layer_retrieve_cpid_from_serv')
        if cpid == -1:
            return -1
        ret = layer_push_queue(ctypes.c_int(cpid),ctypes.c_int(buf_id))
        print(f'msg_send ret = {ret} layer_push_queue')
        return ret

    def msg_recv(self):
        cpid = layer_get_own_cpid()
        print(f'msg_recv: cpid = {cpid} layer_get_own_cpid')
        if cpid == -1:
            return None
        buf_id = layer_pop_queue(ctypes.c_int(cpid))
        print(f'msg_recv: buf_id = {buf_id} layer_pop_queue')
        if buf_id == -1:
            return None
        recv = layer_mem_receive(ctypes.c_int(buf_id))
        print(f'msg_recv: ret byte type!!! ====== {recv} layer_mem_receive')
        # recv = b'Hello world1 from Server->client' 
        retstr = ctypes.string_at(recv).decode('utf-8')
        ret = layer_mem_release(ctypes.c_int(buf_id))
        print(f'msg_recv: ret = {ret}  layer_mem_release')
        print(f'py after release decode= {retstr} in layer_mem_receive')
        return retstr
'''

if __name__ == '__main__':
    print('in main ipc_tranceiver\n')
    tranceiver = Tranceiver()
    server_or_client = int(sys.argv[1])

    if server_or_client == 1:
        cpid = layer_register_serv_to_cpid("SVC0000".encode('utf-8'))
        if cpid == -1:
            sys.exit()
        rcpid = layer_retrieve_cpid_from_serv("SVC0000".encode('utf-8'))
        owncpid = layer_get_own_cpid()
        print(f'Server=cpid = {cpid}, retrieve cpid = {rcpid}, own cpid = {owncpid}======\n')
        time.sleep(10)
        print('wait 10s ended, server or client', server_or_client)

        str = "Hello world1 from Server->client"
        print('=====sended11111111111111111======\n')
        tranceiver.msg_send(str, "SVC0001")
        str2 = "Hello world2 from Server->Client"
        print('=====sended22222222222222222======\n')
        tranceiver.msg_send(str2, "SVC0001")
    else:
        str = "Hello world from client->server"
        cpid = layer_register_serv_to_cpid("SVC0001".encode('utf-8'))
        if cpid == -1:
            sys.exit()
        rcpid = layer_retrieve_cpid_from_serv("SVC0001".encode('utf-8'))
        owncpid = layer_get_own_cpid()
        print(f'Client=cpid = {cpid}, retrieve cpid = {rcpid}, own cpid = {owncpid}======\n')
        time.sleep(20)
        print('wait 20s ended, server or client', server_or_client)
        print('=====client recv1======\n')
        retstr = tranceiver.msg_recv()
        print(f'tranceiver.msg_recv retvalue= {retstr}')
        time.sleep(2)
        print('=====client recv2======\n')
        retstr = tranceiver.msg_recv()
        print(f'tranceiver.msg_recv retvalue= {retstr}')
        print('======client send=====')
        tranceiver.msg_send(str, "SVC0000")
    time.sleep(60)
    retstr = tranceiver.msg_recv()
    print(f'tranceiver.msg_recv retvalue= {retstr}')
    print('in main end\n')

'''


'''
gcc -c -fPIC layer_messaging.c
gcc -shared -o layer_messaging.so layer_messaging.o

# executing sequences for testing
#main init process
./ipc_setup.py

#service startup:
./ipc_tranceiver.py 1
#wait seconds to exec
./ipc_tranceiver.py 0

tranceiver = Tranceiver()
while True:
    time.sleep()
    own = "SVC0000"
    msg = "Hello world sent from SVC0000 to SVC0001"
    receiver = "SVC00001"
    cpid = layer_register_serv_to_cpid(own.encode('utf-8'))
    print(f'layer_register_serv_to_cpid {own} ret= {cpid}')
    ret = tranceiver.msg_send(msg, receiver)
    print(f'tranceiver.msg_send ret= {ret}')
    retstr = tranceiver.msg_recv()
    print(f'tranceiver.msg_recv retvalue= {retstr}')
'''

