// C Program to design a shell in Linux 
#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<sys/wait.h> 
#include<readline/readline.h> 
#include<readline/history.h> 
#include<signal.h>
#include<fcntl.h>
#include<sys/stat.h>

#define MESSAGELENGTH 200 //max length of message
#define MAXCOMMANDLENGTH 512 // max number of letters to be supported 
#define MAXLISTLENGTH 500 // max number of commands to be supported 
#define clear() printf("\033[H\033[J") 

int indexOfHistory=0;
char historyArray[MAXLISTLENGTH][MAXCOMMANDLENGTH];
int signalSw=0;

// Greeting shell during startup 
void say_hello() 
{ 
    clear(); 
    printf("--------------------------------------------------"); 
    printf("\n\n\n"); 
    char* username = getenv("USER"); 
    printf("Hello @%s, welcom to this shell", username);
    printf("\n\n\n"); 
    printf("--------------------------------------------------"); 
    printf("\n"); 
    sleep(1); 
    clear(); 
}
//Function for save history
void print_history(){
	printf("History :\n");
	for(int i=0;i<indexOfHistory;i++){
		printf("%d : %s\n",(i+1),historyArray[i]);
	}

} 
//function for handle ctrl+c
void ctrl_c_func(int signal){
    signalSw = 1;
}
// Function to take input 
int take_input(char* str) 
{ 
    char* buffer; 
    buffer = readline("::command>> ");
    if(buffer==NULL) {
        printf("\n");  
	exit(0);
    }
   // signal(SIGINT,ctrl_c_func);
    if(strlen(buffer)>MAXCOMMANDLENGTH){
        fprintf(stderr,"your command length is too long!");
    }else if (strlen(buffer) != 0) { 
        add_history(buffer); 
        strcpy(str, buffer);
	strcpy(historyArray[indexOfHistory] , str);
	indexOfHistory++; 
        return 0; 
    } else {
//if the command is empty
        return 1; 
    } 
} 
  
// Function to print Current Directory. 
void print_direction_address() 
{ 
    char cwd[1024]; 
    getcwd(cwd, sizeof(cwd)); 
    printf("\nDir: %s", cwd); 
}
// Function for writing to another shell
void write_to_shell()
{
    printf("type your message here :\n");
    int fd;
    // FIFO file path
    char *myfifo = "/tmp/myfifo";
    mkfifo(myfifo, 0666);
    char message[MESSAGELENGTH];

    fd = open(myfifo, O_WRONLY);
    fgets(message,MESSAGELENGTH, stdin);
    write(fd, message, strlen(message) + 1);
    close(fd);
}
// Function for reading message from another shell
void read_from_shell()
{
    int fd;
    // FIFO file path
    char *myfifo = "/tmp/myfifo";
    mkfifo(myfifo, 0666);
    char message[MESSAGELENGTH];
    fd = open(myfifo, O_RDONLY);
    read(fd , message , MESSAGELENGTH);
    printf("you have message: %s\n", message);
    close(fd);
}

  
// Function where the system command is executed 
void execute_commands(char** parsed) 
{ 
    // Forking a child 
    pid_t pid = fork();  
  
    if (pid == -1) { 
        fprintf(stderr,"\nFailed forking child!"); 
        return; 
    } else if (pid == 0) { 
        if (execvp(parsed[0], parsed) < 0) {
            fprintf(stderr,"\nyour command is wrong or not executable!"); 
        } 
        exit(0); 
    } else { 
         //waiting for child to terminate 
         wait(NULL);  
         return; 
    } 
} 
  
// Function where the piped system commands is executed 
void execute_piped_commands(char** parsed, char** parsedpipe) 
{ 
    // 0 is read end, 1 is write end 
    int pipefd[2];  
    pid_t p1, p2; 
  
    if (pipe(pipefd) < 0) { 
        fprintf(stderr,"\nsomething is wrong,Pipe could not be initialized!"); 
        return; 
    } 
    p1 = fork(); 
    if (p1 < 0) { 
        fprintf(stderr,"\nCould not fork!"); 
        return; 
    } 
  
    if (p1 == 0) { 
        // Child 1 executing.. 
        // It only needs to write at the write end 
        close(pipefd[0]); 
        dup2(pipefd[1], STDIN_FILENO); 
        close(pipefd[1]); 
        execlp(parsedpipe[0],parsedpipe[0],parsedpipe[1],(char*)NULL);
    } else { 
        // Parent executing 
        p2 = fork(); 
  
        if (p2 < 0) { 
            fprintf(stderr,"\nCould not fork!"); 
            return; 
        } 
  
        // Child 2 executing.. 
        // It only needs to read at the read end 
        if (p2 == 0) { 
            close(pipefd[1]); 
            dup2(pipefd[0], STDOUT_FILENO); 
            close(pipefd[0]); 
            execlp(parsed[0],parsed[0],parsed[1],(char*)NULL);
        }
       	close(pipefd[1]);
	close(pipefd[0]);
	waitpid(-1,NULL,0);
	waitpid(-1,NULL,0); 
    } 
} 
  
// Help command builtin 
void print_help() 
{ 
    printf("\n-----------------HELP-----------------"
        "\nList of Commands supported:"
        "\n>cd -- for change directory"
        "\n>msg -- for write a message to a shell"
	"\n>readmsg -- for read a message of a shell"
	"\n>history -- for show the history of commands"
        "\n>quit -- for exit from the shell"
	"\n>help -- for show this:)"
        "\n>all other general commands available in UNIX shell"
        "\n>pipe handling"); 
  
    return; 
} 
  
// Function to execute extra commands 
int extra_commands(char** parsed) 
{ 
    int extraCommandsN = 6, i, sw = 0; 
    char* extraCommandList[extraCommandsN]; 
    char* username; 
  
    extraCommandList[0] = "quit"; 
    extraCommandList[1] = "cd"; 
    extraCommandList[2] = "help";
    extraCommandList[3] = "history";
    extraCommandList[4] = "msg";//write message 
    extraCommandList[5] = "readmsg";//read message  
  
    for (i = 0; i < extraCommandsN; i++) { 
        if (strcmp(parsed[0], extraCommandList[i]) == 0) { 
            sw = i + 1; 
            break; 
        } 
    } 
  
    switch (sw) { 
    case 1: 
        exit(0); 
    case 2: 
        chdir(parsed[1]); 
        return 1; 
    case 3: 
        print_help(); 
        return 1; 
    case 4:
	print_history();
	return 1;
    case 5:
        write_to_shell();
        return 1;
    case 6:
        read_from_shell();
        return 1;
    default: 
        break; 
    } 
    return 0; 
} 
  
// function for finding pipe 
int parse_pipe(char* str, char** strpiped) 
{ 
    int i; 
    for (i = 0; i < 2; i++) { 
        strpiped[i] = strsep(&str, "|"); 
        if (strpiped[i] == NULL) 
            break; 
    } 
  
    if (strpiped[1] == NULL) 
        return 0; // returns zero if no pipe is found. 
    else { 
        return 1; 
    } 
} 
  
// function for parsing command words 
void parse_command(char* str, char** parsed) 
{ 
    int i; 
    for (i = 0; i < MAXLISTLENGTH; i++) { 
        parsed[i] = strsep(&str, " ");
        if (parsed[i] == NULL) 
            break; 
        if (strlen(parsed[i]) == 0) 
            i--; 
    } 
} 
  
int process_commands(char* str, char** parsed, char** parsedpipe) 
{ 
  
    char* strpiped[2]; 
    int piped = 0; 
    piped = parse_pipe(str, strpiped); 
    if (piped) { 
        parse_command(strpiped[0], parsed); 
        parse_command(strpiped[1], parsedpipe); 
  
    } else { 
  
        parse_command(str, parsed); 
    } 
  
    if (extra_commands(parsed)) {
        	return 0;
	 }
    else
        return 1 + piped; 
        // returns 0 if there is no command or it is a builtin command, 
        // 1 if it is a simple command 
        // 2 if it is including a pipe. 
} 

//function for batch file
void  read_batchfile(char **commandsInFile){	
	FILE *fpointer;
	char chunk[MAXLISTLENGTH];
	size_t len=sizeof(chunk);
	int index=0;
        char *parsedArgs[MAXLISTLENGTH];
        char* parsedArgsPiped[MAXLISTLENGTH]; 
        int execFlag = 0; 
        int sw=0;    
	char str[MAXCOMMANDLENGTH];

	if((fpointer=fopen("sample.sh","r"))==NULL){
        	printf("cant openinig the file!");
        	exit(1);
	}
	while(fgets(chunk,len,fpointer)!=NULL){
		if(strlen(chunk)>MAXCOMMANDLENGTH){
        		fprintf(stderr,"your command length is too long!");
    		}else if (strlen(chunk) != 0) { 
        		add_history(chunk); 
			strcpy(str,chunk);
			commandsInFile[index]=str;
			printf("command:%s",commandsInFile[index]);
			index=index+1;
			sw=0; 
    		} else {
		//if the command is empty
        		sw=1; 
    		} 
		printf("sw=%d",sw);
		if(sw==0){
			print_direction_address();
			printf("\nc%d: %s",index,str);
		 	execFlag = process_commands(str, parsedArgs, parsedArgsPiped);
	        // execute 
        		if (execFlag == 1) 
           		 execute_commands(parsedArgs); 
        		if (execFlag == 2) 
	            	execute_piped_commands(parsedArgs, parsedArgsPiped); 
			index=index+1;
		//because the command is empty
		}else{
			continue;
		}
	}
	fclose(fpointer);
}

int main(){

    char inputString[MAXCOMMANDLENGTH], *parsedArgs[MAXLISTLENGTH]; 
    char* parsedArgsPiped[MAXLISTLENGTH]; 
    int execFlag = 0; 
    say_hello(); 
  
    while (1) {
	signal(SIGINT,ctrl_c_func);  
        print_direction_address(); 
        // take input 
        if (take_input(inputString) || signalSw==1){
		signalSw=0;
		continue;
        }  
        // process 
        execFlag = process_commands(inputString, parsedArgs, parsedArgsPiped); 
        // execute 
        if (execFlag == 1) 
            execute_commands(parsedArgs); 
        if (execFlag == 2) 
            execute_piped_commands(parsedArgs, parsedArgsPiped); 
    }
    return 0; 
}

