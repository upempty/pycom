python ipc which supports especially multiple none-related processes
```
gcc -c -fPIC layer_messaging.c
gcc -shared -o layer_messaging.so layer_messaging.o

# executing sequences proposed for testing
#main init process
./ipc_setup.py

#service startup:
./ipc_tranceiver.py 1

#wait seconds to exec
./ipc_tranceiver.py 0
```