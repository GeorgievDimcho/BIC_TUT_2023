//#ifndef IPC_H
#define IPC_H
#define IPC_ERROR (-1)
#define IPC_SUCCESS (1)
#define SHARED_MEMORY "/shared_mem"
#define SHARED_SEM_R "/shared_sem_r"
#define SHARED_SEM_W "/shared_sem_w"
//
#define SHARED_SEM_MODE S_IRWXU
#define SHARED_MEM_MODE S_IRWXU

#include <stdio.h>
#include <stdlib.h> // For exit() and exit status codes
//
#include <semaphore.h> // For sem_t
//
#include <stdio.h>	   // perror()
#include <unistd.h>	   // getopt()
#include <stdlib.h>	   // atoi()
#include <sys/mman.h>  // POSIX shared memory library
#include <semaphore.h> // POSIX semaphore library
#include <sys/stat.h>  /* For mode constants */
#include <fcntl.h>	   /* For O_* constants */
#include <errno.h>	

typedef struct ringbuffer
{
	unsigned int size;
	char *sem_r_name;
	char *sem_w_name;
	sem_t *sem_read;
	sem_t *sem_write;
	char *memory_name;
	char *memory;
} ringbuffer_t;


int enter_critical_section(sem_t *semaphore)
{
	if (semaphore == NULL)
	{
		errno = EINVAL;
		return IPC_ERROR;
	}
#ifdef DEBUG
	printf("Now entering critical section.\n");
#endif
	while (1)
	{
		int const signal = sem_wait(semaphore);

		if (signal == -1)
		{
			if (errno == EINTR)
			{
				continue;
			}
			perror("ERROR in sem_wait()");
			return IPC_ERROR;
		}
		return IPC_SUCCESS;
	}
}

int exit_critical_section(sem_t *semaphore)
{
	if (semaphore == NULL)
	{
		errno = EINVAL;
		return IPC_ERROR;
	}
#ifdef DEBUG
	printf("Now exiting critical section.\n");
#endif
	int const signal = sem_post(semaphore);
	if (signal == -1)
	{
		perror("ERROR in sem_post()");
		return IPC_ERROR;
	}
	return IPC_SUCCESS;
}

unsigned int parse_args(int argc, char *const argv[])
{
	char option;
	int elements = -1; // Number of elements given by option
	char *valid_options = "hm:";
	if (argc == 1)
	{
		fprintf(stderr, "Usage: %s -m size\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	while ((option = getopt(argc, argv, valid_options)) != -1)
	{
		switch (option)
		{
		case 'm':
			elements = atoi(optarg);
			break;
		case 'h':
			//print_help(argv[0]);
			exit(EXIT_SUCCESS);
		case '?':
			fprintf(stderr, "Usage: %s -m size\n", argv[0]);
			exit(EXIT_FAILURE);
		default:
			fprintf(stderr, "Usage: %s -m size\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}
	if (elements < 0)
	{
		errno = EINVAL;
		perror("Given number was negative or 0");
		exit(EXIT_FAILURE);
	}
	return elements;
}


static int open_shared_memory(char *filename, int oflag)
{
	if (filename == NULL)
	{
		errno = EINVAL;
		return IPC_ERROR;
	}
	int shared_fd;
	//  Get or create shared memory object
	shared_fd = shm_open(filename, oflag, S_IRWXU);
	if (shared_fd == -1)
	{
		perror("ERROR in shm_open() during open");
		return IPC_ERROR;
	}
	return shared_fd;
}

int create_shared_memory(char *filename, int size)
{
	// RDWR == Read-Write | O_CREAT == Create, if it doesn't exist | O_EXCL == Throw error, if it exists
	int shared_fd = open_shared_memory(filename, O_RDWR | O_CREAT);
	if (shared_fd == IPC_ERROR)
	{
		// Error already written
		return IPC_ERROR;
	}
	if (ftruncate(shared_fd, size) == -1)
	{
		perror("ERROR in ftruncate() during creation");
		return IPC_ERROR;
	}
	return shared_fd;
}

char *attach_shared_memory(int shared_fd, size_t length)
{
	char *shared_mem = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, shared_fd, 0);
	if (shared_mem == MAP_FAILED)
	{
		perror("ERROR in mmap() during attach");
		shm_unlink(SHARED_MEMORY);
		return NULL;
	}
	close(shared_fd); // File-descriptor can be closed because memory is attached
	return shared_mem;
}
int detach_shared_memory(char *shared_mem, size_t length)
{
	if (shared_mem == NULL)
	{
		errno = EINVAL;
		return IPC_ERROR;
	}
	if (munmap(shared_mem, length) == -1)
	{
		perror("ERROR in munmap() during detach");
		return IPC_ERROR;
	}
	return IPC_SUCCESS;
}
int destroy_shared_memory(char *filename)
{
	if (filename == NULL)
	{
		errno = EINVAL;
		return IPC_ERROR;
	}
	if (shm_unlink(filename) == -1)
	{
		if (errno == ENOENT)
		{
			return IPC_SUCCESS;
		}
		perror("ERROR in shm_unlink() during destroy");
		return IPC_ERROR;
	}
	return IPC_SUCCESS;
}

// SHARED_SEMAPHORE ------------------------------

static sem_t *open_shared_semaphore(char *filename, int oflag, unsigned int initial)
{
	// filename was already checkout in outer-function
	sem_t *shared_sem;
	shared_sem = sem_open(filename, oflag, SHARED_SEM_MODE, initial);
	if (shared_sem == SEM_FAILED)
	{
		perror("ERROR in sem_open() during creation");
		return NULL;
	}
	return shared_sem;
}

sem_t *create_shared_semaphore(char *filename, unsigned int initial)
{
	if (filename == NULL)
	{
		errno = EINVAL;
		return NULL;
	}
	sem_t *shared_sem;
	shared_sem = open_shared_semaphore(filename, O_CREAT, initial);
	if (shared_sem == NULL)
	{
		// Error already written
		return NULL;
	}
	return shared_sem;
}

int detach_shared_semaphore(char *filename)
{
	if (filename == NULL)
	{
		errno = EINVAL;
		return IPC_ERROR;
	}
	if (sem_unlink(filename) == -1)
	{
		perror("ERROR in sem_unlink() during detach");
		return IPC_ERROR;
	}
	return IPC_SUCCESS;
}

int destroy_shared_semaphore(char *filename, sem_t *shared_sem)
{
	if (filename == NULL)
	{
		errno = EINVAL;
		return IPC_ERROR;
	}
	if (sem_close(shared_sem) == -1)
	{
		perror("ERROR in sem_close() during destroy");
		return IPC_ERROR;
	}
	if (sem_unlink(filename) == -1)
	{
		perror("ERROR in sem_unlink() during destroy");
		return IPC_ERROR;
	}
	return IPC_SUCCESS;
}

void initialize_ringbuffer(ringbuffer_t *rb, unsigned int elements)
{
	rb->size = elements;
	rb->sem_r_name = SHARED_SEM_R;
	rb->sem_w_name = SHARED_SEM_W;
	rb->memory_name = SHARED_MEMORY;

	rb->sem_read = create_shared_semaphore(rb->sem_r_name, 0);
	rb->sem_write = create_shared_semaphore(rb->sem_w_name, 1);
	if (rb->sem_read == NULL || rb->sem_write == NULL)
	{
		fprintf(stderr, "Couldn't create semaphores!\n");
		exit(EXIT_FAILURE);
	}
	int shared_fd = create_shared_memory(rb->memory_name, rb->size * sizeof(char *));
	if (shared_fd == IPC_ERROR)
	{
		destroy_shared_semaphore(rb->sem_r_name, rb->sem_read);
		destroy_shared_semaphore(rb->sem_w_name, rb->sem_write);
		exit(EXIT_FAILURE);
	}
	rb->memory = attach_shared_memory(shared_fd, rb->size * sizeof(char *));
	close(shared_fd);
	if (rb->memory == NULL)
	{
		destroy_shared_semaphore(rb->sem_r_name, rb->sem_read);
		destroy_shared_semaphore(rb->sem_w_name, rb->sem_write);
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char *argv[]) 
{
    unsigned int elements = parse_args(argc, argv);

	ringbuffer_t rb = {0};
	initialize_ringbuffer(&rb, elements);
	int ipc = 0;
	int character = EOF; // MUST be int to have range for EOF
	unsigned int write_index = 0;

	do
	{
		character = getchar();
		ipc = enter_critical_section(rb.sem_write); // Lock write semaphore
		rb.memory[write_index] = character;
		ipc = exit_critical_section(rb.sem_read); // Unlock read semaphore
		write_index++;
		write_index = write_index % rb.size; // sodass rb.size nicht übersprungen wird
		if (character == EOF)
		{
			ipc = 0;
			break;
		}
	} while (ipc == IPC_SUCCESS);

	if (destroy_shared_semaphore(rb.sem_w_name, rb.sem_write) == IPC_ERROR)
	{
		exit(EXIT_FAILURE);
	}
	if (detach_shared_memory(rb.memory, sizeof(char) * rb.size) == IPC_ERROR)
	{
		exit(EXIT_FAILURE);
	}
	if (destroy_shared_memory(rb.memory_name) == IPC_ERROR)
	{
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
    
    return 0;
}