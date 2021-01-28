#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>

//declare function, their implementation is on end
void handle_sigint(int);
void handle_sigterm(int);

int main (int argc, char *argv[])
{
    //check if he gave more than one arguments.
    if (argc>2){
        printf("\tYou cannot give more than one arguments!");
        return -1;
    }
    DIR *folder;
    char *dir;
    //if he gave me directory save his path otherwise save current directory
    if (argc==2) //so he gave me an argument
    {
        //open given directory 
        dir=argv[1];
    }else{

        //open current directory
        dir="./";
    }
    folder = opendir(dir);
    
    //check if directory opened succesfully
   if(folder == NULL)
     {
            puts("\tUnable to read directory");
            exit(-1);
     }
       



    //declare variables before while, so i don`t make unnecessary statements
    struct dirent *pde;  // Pointer for Directory Entry 
    char * name_of_file;  //use it to take file of each name
    int name_of_file_has_size; //use it to take size of each file name 
    DIR* check_is_file; //use it to check if it`s file
    int  fd ; //use it to open files 
    int number_of_characters; //use it to take the number of characters
    long file_size; //use it to take the size of each file 
    pid_t pid,ppid;
    int forks=0;//sum of forks, so i can check for zombies
    struct stat stat_buffer;
   


    //I use readdir to read  this directory 
    while ((pde = readdir(folder)) != NULL)
    {
         printf("%s\n", pde->d_name); 
        //so now i have the names of whatever is inside of folder, one under the other. 
        name_of_file= pde->d_name;
        //use strlen to find the size of string-name of file 
        name_of_file_has_size=strlen(dir) +1/*for backslash*/ +strlen(name_of_file);  
        //make a buffer for this string 
        char buffer_for_name_of_file[name_of_file_has_size];
        //use sprintf to printf a string (for that is the s on start)
        sprintf(buffer_for_name_of_file,"%s/%s",dir,name_of_file);
        //use opendir to check if it is a file
        
        check_is_file= opendir(buffer_for_name_of_file);
        if(check_is_file!=NULL) // so it`s not a file, go on next
        {
            printf("\t%s: it`s not a file!\n",pde->d_name);
            continue;
        }


        //open each file
        fd=open(buffer_for_name_of_file, O_RDONLY); //we need Read only 
        if(fd<0) //has to be possitive to be right
        {
            perror("open");
            exit(1);
        }
       // printf("\tsuccessfully opened the fd=%d \n",fd);
  
       //calculate file size with lseek use
        file_size= lseek(fd,0,SEEK_END); //now cursor accessed the entire file and stayed at end of it 
        if(file_size==0)
        {
           printf("\tEmpty file!");
           continue; //go to check for next file, you have not words to count here.
        }
        lseek(fd,0,SEEK_SET); //put cursor again on start of file
    
    
    
        //create a buffer to help us read the number of characters
        char buff[file_size];
        //take the number of characters using read()
        number_of_characters=read(fd,buff,file_size); //so now buffer has the characters
       
       
       
        /*
        use buffer (buff) to take characters and
        check if each of them is on ASCII table.
        if not then i print a message and i go on next file.
        */
        int found_no_Ascii=0;
  
        for(int i=0;i<file_size;i++)
         {        //printf("buffer for %d prints ->%d\n\n",i,buff[i]);
             if(buff[i]<0||buff[i]>127)//so it`s out of ASCII
            {
                 found_no_Ascii=1;
                 i+=file_size-i; //so if you find one go on end, it`s enough
            }
        }
        // printf("file size is %ld",file_size);
        if(found_no_Ascii==1)
        {
            printf("\tThis file it`s not with ASCII characters!\n");
            continue; //go to check for other files
        }


        //end of first part of the exercise (basic process)



        //start of creation of new processes


      /*
         I am going to use an exec function
         before that I had to create an array with arguments
        -> the path of counting-program
        -> the buffer of file name  
        -> the buffer of that file
        ->null, to mark the end of the array of pointers
      */
        char *args[]={"./counting_words",buffer_for_name_of_file,buff,NULL};

       /*
        I use fork to create a new process
        for each  file (if a file is on this line
        then sure is on ascii)
       */
        forks++;
        pid=fork();
       

        if(pid<0) //check for fail creation
        {
            perror("creation of a child process was unsuccessful\n");
            exit(1);
        }
        //check if returned child or parent
        if(pid==0)//child
        { 
            //use execv to replace the prosses with another, and count the words of the file
            execv(args[0],args);
        }else{ //parent 
            //use signal for hadling
            signal(SIGINT, handle_sigint); 
		    signal(SIGTERM,handle_sigterm);
        }

     //close file 
      if(close(fd)<0)
      {
          perror("\tUnexpected error!Something went wrong with close.");
          exit(1);
      }
    } 
    //take care for zombies processes, wait to end all the children!
    for(int i=0;i<forks;i++){
        wait(NULL);
    }
     closedir(folder);
     exit(1);
return 0;
}

//FUNCTIONS FOR ERROR HANDLING
void handle_sigterm(int signal) 
{ 
    printf("\n\tIgnored signal %d\n", signal); 
} 

void handle_sigint(int signal) 
{ 
    printf("\n\tIgnored signal %d\n", signal); 
}  
