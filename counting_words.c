#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/file.h>
#include <unistd.h>
#include <string.h>
#include <error.h>

#define NTHREADS 8  /*maybe you have to change that, based in my system.
                   In case your system can't manage at the same time 8 threads               
                   Then low that number. Or put a greater if your system can 
                    use them     
                   */


//make a struct to take number of thread and the buffer(so what it has to read)
typedef struct dataS data;
struct dataS
{
    char *buffer;//we use the buffer to take the characters
    int thread_counter;//to save the sum of threads
};

//initialize mutex 
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

//initialize the total sum for the threads of one file
int total_number=0;



// DECLARE FUNCTION - IMPLEMENTATION IS ON END


    //check if the given character is one from the separators of a word
int word_seperator(char);
    //counts the words of a file with the help of the threads
void *counter_words_of_file(void *);



//MAIN FUNCTION
int main(int argc, char *argv[])
{
    //initialize the array of the threads
    pthread_t threads[NTHREADS];
    //initialize the struct of the arguments that every thread must has
    data count[NTHREADS];
   
    
    //create threads
    for (int i = 0; i < NTHREADS; i++)
    {
        //first data   is the characters from the file
        count[i].buffer=argv[2];
        //second data is the number of thread
        count[i].thread_counter=i;
        pthread_create(&threads[i], NULL, &counter_words_of_file, (void *)&(count[i]));
            
    }
    
    //wait the threads to complete
    for (int i = 0; i < NTHREADS; i++)
    {
        pthread_join(threads[i],NULL);
    }

    //calculate the size of output file 
    int size_of_output_file=(sizeof(getpid())+strlen(argv[1]) +5 /*for commas and newline*/+sizeof(total_number));
    char buffer_output[size_of_output_file];
    
    int fd;
       /*
        permissions is like  
        r-- ->4                 so i want 644 to give rw--w--w-
        -w- ->2
        --x -<1
    */
    //create an output file and write on it... if already exists append that
    if ((fd=open("output.txt",O_WRONLY | O_CREAT | O_APPEND,0644 ))==-1)
    {
        perror("");
        exit(1);
    }

    //prepare the format of text we will print on output file
    snprintf(buffer_output,size_of_output_file,"%d, %s, %d\n",getpid(),argv[1],total_number);

    //write the array of characters in the file
    write(fd, buffer_output, strlen(buffer_output)); 

    return 0;
}       

//FUNCTIONS IMPLEMENTATIONS
/*I take as words only patterns of letters. For example: “meletis !” is one 
Word or “meletis 219105!” is one word. The reason is that I want more realistic and meaningful counting and I don't want take as words syntax mistakes of user   
Like “Hello my friend , today …” commas etc isn`t words.*/

int word_seperator(char character)
{
    if(character==' ' ||character=='\n'||character=='\t'||character=='\0'
     ||character==',' ||character=='!' ||character=='`' ||character=='('
     ||character==')' ||character=='{' ||character=='}' ||character=='['
     ||character==']' ||character=='+' ||character=='-' ||character=='>'
     ||character=='<' ||character=='.' ||character=='%' ||character=='@' 
     ||character=='*' ||character=='^' ||character=='*' ||character=='"'
     ||character=='~' ||character=='/' ||character==';' ||character=='&'
     ||character=='#' ||character=='$' ||character=='_' ||character==':'
     ||character=='0' ||character=='1' ||character=='2' ||character=='3'
     ||character=='4' ||character=='5' ||character=='6' ||character=='7'
     ||character=='?' ||character=='8' ||character=='9' ||character=='\\'
     ||character=='|')
    {
        return 1;//true,is a seperator

    }
    return 0;//false,its not a seperator
}


//counts the words of a file with the help of the threads
void *counter_words_of_file(void *data_args)
{
    //take the data of the thread
    data *args;
    args =data_args;

    //take the number of threads
    int number_of_threads=args->thread_counter;

    //make a counter for words of a thread
    int counter=0;
   // int i;
    
    //check if is the last thread to read correctly the lasts characters in buffer
   int modulo=0;
    if(number_of_threads==NTHREADS-1 && strlen(args->buffer)%NTHREADS!=0){
       modulo=(strlen(args->buffer)%NTHREADS)-1;
    }
    for(int i=number_of_threads*(strlen(args->buffer)/NTHREADS);i<(number_of_threads+1)*(strlen(args->buffer)/NTHREADS)+modulo;i++)
        {
            //check if it is in the end of a word
            if(!word_seperator(args->buffer[i]) && word_seperator(args->buffer[i+1]))
            {
                //add by one the counter of words
                counter++;
            }
        }

     // "freeze" threads, to control threads we have to  let one thread at the time
    pthread_mutex_lock(&mutex);

     //and then let that thread to add on sum of counter
    total_number+=counter;

    //and unlock - unfreeze threads
    pthread_mutex_unlock(&mutex);

    //exit of thread
    pthread_exit (NULL);
    
}
