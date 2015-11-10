# ProducerConsumer
SYSC 4001 Operating Systems, Fall 2015
Programming Assignment 2: Producer and Consumer
Monica Ruttle
Bronwyn Skelley

This assignment contains two C files, consumer.c and producer.c, and one header file pcinfo.h.

In order for these two files to communicate, shared memory is used. It contains 100 buffers of size 128, as well as an int for each buffer that counts the size of what is in that buffer.

The producer.c file contains the logic for the producer. It will attach to the shared memory space and copy the contents of a file into a character buffer. The file's text must not contain the character '$' until the last index. This is used to signal the end of the file. Once it is able to enter the shared memory (ie the buffers are not full and nothing else is in shared memory), it places the contents of the character buffer into individual buffers of size 128 in shared memory. Once this has been done, it prints out the number of bytes were read in from the file.

The consumer.c file contains the logic for the consumer. It will attach to the shared memory space and wait to enter it. Once a buffer has been filled and no other program is accessing shared memory, it enters to read in the buffers. It individually reads in the buffers of size 128 and writes them to a file one at a time. There will be a space between each 128 characters, delimiting the end of that buffer. After each read/write, it checks to see if the shared memory buffer is in the same size as the bytes written to a file. After the buffers are all read in from shared memory, it prints the total bytes written to the file.

The program is run by first starting consumer.c. This will wait for the producer to begin in another terminal. Once the producer is run, it will wait for input or a text file. The input can have a maximum size of 12800 characters. The output on the producer's terminal will simply be the number of bytes read. The output on the consumer's terminal will be the text inputted from the producer and the number of bytes written to the file/console. The number of bytes should match on the consumer and producer sides.

[1] Matthew, Neil, and Richard Stones. Beginning Linux Programming. 4th ed. Birmingham, UK: Wrox, 2007. Print.
