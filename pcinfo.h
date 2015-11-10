#define	MEMSIZE 13200 //Each buffer has a total size of 132 bytes, and there are 100 of them
#define TEXTSIZE 128
#define NBUFFER 100
#define key 1234
#define key_n 5678
#define key_s 9012
#define key_e 9876

//the struct in shared memory with 100 buffers
struct buffer_msg{
	struct{
		int size;
		char msg[TEXTSIZE];
	}buffer[NBUFFER];
};


//This was taken from the code provided by "Beginning Linux Programmer", 4th Edition, Neil Matthews and Richard Stones
union semun {
        int val;                    /* value for SETVAL */
        struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
        unsigned short int *array;  /* array for GETALL, SETALL */
        struct seminfo *__buf;      /* buffer for IPC_INFO */
};
