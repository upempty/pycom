#!/usr/bin/env python
#ipc_lib.py
import sys
import ctypes
from ctypes import *
libcom = ctypes.CDLL("/myhome/programming/common_service/layer_messaging.so")
initContainers = libcom.initContainers
initQueues = libcom.initQueues

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

#ret = layer_register_service('SVC0000')
#ret = layer_retrieve_service('SVC0000')