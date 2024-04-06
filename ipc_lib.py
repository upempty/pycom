#!/usr/bin/env python
#ipc_lib.py
import sys
import ctypes
from ctypes import *
libcom = ctypes.CDLL("./layer_messaging.so")
initContainers = libcom.initContainers
initQueues = libcom.initQueues
initCpidPool = libcom.initCpidPool


layer_mem_reserve = libcom.layer_mem_reserve
layer_mem_reserve.argtypes = [ctypes.c_char_p]
layer_mem_reserve.restype = ctypes.c_int

layer_push_queue = libcom.layer_push_queue
layer_push_queue.argtypes = [ctypes.c_int, ctypes.c_int]
layer_push_queue.restype = ctypes.c_int

layer_pop_queue = libcom.layer_pop_queue
layer_pop_queue.argtypes = [ctypes.c_int]
layer_pop_queue.restype = ctypes.c_int

layer_mem_receive = libcom.layer_mem_receive
layer_mem_receive.argtypes = [ctypes.c_int]
layer_mem_receive.restype = ctypes.c_char_p

layer_mem_release = libcom.layer_mem_release
layer_mem_release.argtypes = [ctypes.c_int]
layer_mem_release.restype = ctypes.c_int


layer_register_serv_to_cpid = libcom.layer_register_serv_to_cpid
layer_register_serv_to_cpid.argtypes = [ctypes.c_char_p]
layer_register_serv_to_cpid.restype = ctypes.c_int

layer_retrieve_cpid_from_serv = libcom.layer_retrieve_cpid_from_serv
layer_retrieve_cpid_from_serv.argtypes = [ctypes.c_char_p]
layer_retrieve_cpid_from_serv.restype = ctypes.c_int

layer_get_own_cpid = libcom.layer_get_own_cpid
layer_mem_release.restype = ctypes.c_int


'''
# below is wrong example for dict() sharing used in seperate processes.
# as the precondition is that Manager instance was created seperately if to executing the python file multiple times.
import os
import time
import multiprocessing
srv_dict = multiprocessing.Manager().dict()
lock = multiprocessing.Manager().Lock()
class ServiceRegister:
    MAX_PROCESS_COUNT = 64
    def __init__(self):
        # dict = {'serv0000': [0, os.getpid()], 'serv00001': [1, os.getpid()]}
        pass

    def register_serv_to_cpid(self, name):
        lock.acquire()
        if srv_dict.get(name):
            print(f'already exists for this service {name}')
            lock.release()
            return -1
        ids_in_dict =  [item[0] for item in srv_dict.values()]
        ids_full = list(range(ServiceRegister.MAX_PROCESS_COUNT))
        avail_ids = [i for i in ids_full if i not in set(ids_in_dict)]
        if len(avail_ids) == 0:
            print(f'cpids({ServiceRegister.MAX_PROCESS_COUNT}) full, so cannot allocate for {name}')
            lock.release()
            return -1
        srv_dict[name] = [avail_ids[0], os.getpid()]
        lock.release()
        return avail_ids[0]
    
    def retrieve_cpid_from_serv(self, name):
        lock.acquire()
        cpidset = srv_dict.get(name)
        if not cpidset:
            lock.release()
            return -1
        lock.release()
        return cpidset[0]
    
    def own_cpid(self):
        lock.acquire()
        for cpid_pid in srv_dict.values():
            if cpid_pid[1] == os.getpid():
                lock.release()
                return cpid_pid[0]
        lock.release()
        return -1
    
if __name__ == '__main__':
    print('in main ipc lib\n')
    sr = ServiceRegister()
    cpid = sr.register_serv_to_cpid('SVC0000')
    getcpid = sr.retrieve_cpid_from_serv('SVC0000')
    ownpid = sr.own_cpid()
    print(f'cpid={cpid}, getcpid={getcpid}, ownpid={ownpid}')

    cpid = sr.register_serv_to_cpid('SVC0001')
    getcpid = sr.retrieve_cpid_from_serv('SVC0001')
    ownpid = sr.own_cpid()
    print(f'2222cpid={cpid}, getcpid={getcpid}, ownpid={ownpid}')
    while True:
        time.sleep(20)
        print('lib loop to sleep each 60s on 121.43.133.208 \n')
'''