python ipc which supports especially multiple none-related processes
```
gcc -c -fPIC layer_messaging.c
gcc -shared -o layer_messaging.so layer_messaging.o

# executing sequences proposed for testing
#main init process
./ipc_setup.py

#start client0 and client1 to communicate each other.
./ipc_client0.py
./ipc_client1.py



==legacy way
#service startup:
./ipc_tranceiver.py 1

#wait seconds to exec
./ipc_tranceiver.py 0
```
