# ring
This is a c++ header library for FIFO queue that has a statically allocated capacity. 
There are 2 types of queue, ```queue.hpp``` is a non thread safe queue and ```sync_queue.hpp``` is a thread safe queue.
## Usage
Include the header file in the source code.
Note when when using ```transaction``` ```with sync_queue```, a transaction must first prepare, then execute, then commit. 
Failure to do 1 of the steps will stop new elements from being added or removed from the queue.

