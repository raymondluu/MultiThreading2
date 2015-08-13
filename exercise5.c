//exercise5.c

/* Program: Multi-threading class continued
   Purpose: Exercise 5
   File: exercise5.c
   Programmer: Raymond Luu
   Date: 11/8/2013
   Description: To learn how to program multi-threading.
   
   consumer() 
{
	for (;;)
	{
		mutex_lock (buflock);
		if (bufavail == MAX)
		{
			thread_cond_wait(buf_not_empty, buflock);
		}
		mutex_unlock(buflock);
		// consume data
		mutex_lock(buflock);
		bufavail++;
		thread_cond_signal(buf_not_full);
		mutex_unlock(buflock);
	}
}

This program seems to be putting content into the data buffer 
and then the consumer would print out the data or something. 
I think I have gotten a good understanding of how these condition 
variables will make sure that these two programs stay in sync 
throughout the whole process.
*/

#include <stdio.h>
#include <pthread.h>

void* producer();
void* consumer();

// mutex
pthread_mutex_t lock_mutex = PTHREAD_MUTEX_INITIALIZER;

// condition variables
pthread_cond_t done_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_done_cond = PTHREAD_COND_INITIALIZER;

// flag
int flag = 0;

typedef struct
{
    int num_e, total_leng, end_of_file;
} bundle;

/* Function: main
   Uses library: stdio.h, pthread.h
   Recieves: void
   Returns: int
*/

int main(void)
{
    bundle the_info = {0, 0, 0};
    
    pthread_t prod_thread;
    pthread_t cons_thread;
    
    int prod_thread_ret = pthread_create(&prod_thread, NULL, producer, &the_info);
    int cons_thread_ret = pthread_create(&cons_thread, NULL, consumer, &the_info);
    
    if(prod_thread_ret != 0)
    {
        printf("Thread fail to create! Error: %d", prod_thread_ret);
        return 1;
    }
    if(cons_thread_ret != 0)
    {
        printf("Thread fail to create! Error: %d", cons_thread_ret);
        return 1;
    }
    
    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);
    
    pthread_mutex_destroy(&lock_mutex);
    pthread_cond_destroy(&done_cond);
    pthread_cond_destroy(&not_done_cond);
    pthread_exit(NULL);
    
    return 0;
}

/* Function: Producer thread
   Uses library: stdio.h, pthread.h
   Recieves: void* parameter
   Returns: void*
*/
void* producer(bundle *the_info)
{
	int i, cur_char, temp_e, temp_leng = 0;
    FILE *my_file;

    my_file = fopen("randStrings.txt", "r");
    
    cur_char = fgetc(my_file);
    
    // produce product
    while (the_info->end_of_file != 1)
    {  
        
        while (cur_char != 10)
        {
            if (cur_char == 101)
            {
                temp_e++;
                temp_leng++;
                //printf("%c", cur_char);
            }
            else if (cur_char != 10 && cur_char != 13)
            {   
                temp_leng++;
                //printf("%c", cur_char);
            }
            //printf("%c", cur_char);
            cur_char = fgetc(my_file);
        }
        
        // lock mutex
        pthread_mutex_lock(&lock_mutex);
        
        // thread condition wait
        if (flag == 1)
        {
            pthread_cond_wait(&not_done_cond, &lock_mutex);
        }
        
        // unlock mutex
        pthread_mutex_unlock(&lock_mutex);
        
        // write to shared variable
        the_info->num_e = temp_e;
        the_info->total_leng = temp_leng;
        
        // lock mutex
        pthread_mutex_lock(&lock_mutex);
        
        // change flag
        flag = 1;
        
        // signal thread condition change
        pthread_cond_signal(&done_cond);
        
        // unlock mutex
        pthread_mutex_unlock(&lock_mutex);
        
        // clear variables for next line
        temp_e = 0;
        temp_leng = 0;
        cur_char = fgetc(my_file);
        
        // check for end of file
        if (cur_char == EOF)
        {
            the_info->end_of_file = 1;
        }
    }
    
    return 0;
}

/* Function: Consumer thread
   Uses library: stdio.h, pthread.h
   Recieves: void* parameter
   Returns: void*
*/
void* consumer(bundle *the_info)
{   
    int i, temp_e, temp_leng = 0;
    
    FILE *my_file;

    my_file = fopen("resultStrings.txt", "w");
    
    while (the_info->end_of_file != 1)
    {
    
        // lock mutex
        pthread_mutex_lock(&lock_mutex);
        
        // thread condition wait
        if (flag == 0)
        {
            pthread_cond_wait(&done_cond, &lock_mutex);
        }
        
        // unlock mutex
        pthread_mutex_unlock(&lock_mutex);
        
        // read shared variable
        temp_e = the_info->num_e;
        temp_leng = the_info->total_leng;
        
        // lock mutex
        pthread_mutex_lock(&lock_mutex);
        
        // change flag
        flag = 0;
        
        // signal thread condition change
        pthread_cond_signal(&not_done_cond);
        
        // unlock mutex
        pthread_mutex_unlock(&lock_mutex);
        
        // consume product
        if (temp_e == 0)
        {
            for (i = 0; i < temp_leng; i++)
            {
                //printf("-");
                fputc(45, my_file);
            }
            //printf("\n");
            fputc(13, my_file);
            fputc(10, my_file);
        }
        else
        {
            for (i = 0; i < temp_e; i++)
            {
                //printf("e");
                fputc(101, my_file);
            }
            //printf("\n");
            fputc(13, my_file);
            fputc(10, my_file);
        }
    }
    
    return 0;
}