//include libraries
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>

//define macros and constants,colors
#define MAX_LINE 128 // 128 chars per line
#define CREATE_FLAGS (O_WRONLY | O_CREAT | O_APPEND)
#define CREATE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define RESET   "\x1b[0m"
#define WHITE   "\x1b[37m"
#define GREEN   "\x1b[32m"
#define RED     "\x1b[31m"



struct node
{
    int info;
    char content[MAX_LINE];
    struct node *next;
};
struct node *start=NULL;


pid_t BackID;

//declare and initialize variables
char pathArray[100][100];// for environment variable $PATH
char commandLineInput[MAX_LINE];
int counterOfPathArray = 0;
int numberOfArgs;

int createChild(char *args[]);// declaration of function, it uses in several functions
///////////////////////////////////////////////////////////////////////////////////
void setup(char inputBuffer[], char *args[], int *background)
{
    int length,     /* # of characters in the command line */
    i,      /* loop index for accessing inputBuffer array */
    start,  /* index where beginning of next command parameter is */
    ct;
    ct =0;
    length = (int) read(STDIN_FILENO, inputBuffer, MAX_LINE);
    strcpy(commandLineInput,inputBuffer); /* copy input line to a char array */
    start = -1;
    if (length == 0){
        exit(0);
    }

    if ( (length < 0) && (errno != EINTR) ) {
        perror("Error reading the command!");
        exit(-1);           /* terminate with error code of -1 */
    }
    for (i=0;i<length;i++){ /* examine every character in the inputBuffer */
        switch (inputBuffer[i]){
            case ' ':
            case '\t' :               /* argument separators */
                if(start != -1){
                    args[ct] = &inputBuffer[start];    /* set up pointer */
                    ct++;
                }
                inputBuffer[i] = '\0'; /* add a null char; make a C string */
                start = -1;
                break;
            case '\n':                 /* should be the final char examined */
                if (start != -1){
                    args[ct] = &inputBuffer[start];
                    ct++;
                }
                inputBuffer[i] = '\0';
                args[ct] = NULL; /* no more arguments to this command */
                break;
            default :             /* some other character */
                if (start == -1) {
                    start = i;
                }
                if (inputBuffer[i] == '&' && strcmp(args[0],"bookmark") != 0){
                    *background  = 1;
                    inputBuffer[i-1] = '\0';
                    //backgroundProcess++;
                }
        }   /* end of switch */
    }   /* end of for */
    args[ct] = NULL;    /* just in case the input line was > 80 */
    numberOfArgs=ct;
    // printf("\n numberOfArgs: %d", numberOfArgs);
    if(ct>32){
        perror("Maximum Argument Error: ");
    }

} /* end of setup routine */
/////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////
void PathList()
{
    char * pathList;
    pathList = getenv ("PATH");//take Path with the help of getenv.
    char separator[2] = ":";
    char *token;
    token = strtok(pathList, separator);
    int i=0;
    while( token != NULL )
    {
        strcpy( pathArray[i], token );
        token = strtok(NULL, separator);
        i++;
        counterOfPathArray++;
        // printf("\ntoken: %s",token);
    }
}// end of PathList()
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////  
void  execvRedirection(char *argRedirect[])
{
    char fullPath[222];
    strcpy(fullPath,argRedirect[0]);
    argRedirect[0] = ".\\";
    // printf("\nargRedirect: %s",argRedirect);
    execv(fullPath,argRedirect);
}
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
int createChild(char *args[])
{
    char fullPath[222];
    int i =0;
    for(i=0;i<counterOfPathArray;i++)
    {
        strcpy(fullPath,pathArray[i]);
        strcat(fullPath,"/");
        strcat(fullPath,args[0]);
        if( execv(fullPath,args) == -1 ){
        }
    }

    printf("\nfullPath: %s", fullPath);// "/usr/sbin/" sonuna oluşan process neyse onu ekliyor örneğin bookmark,

    printf(RED"No valid path.\n"RESET);
    return 0;
}
///////////////////////////////////////////////////////////////////////////


void error(char *argRedirect[],char *fileName)
{
    int fd;
    fd = open(fileName, CREATE_FLAGS, CREATE_MODE);
    if (fd == -1){
        perror(RED"Failed to open my.file"RESET);
    }
    if (dup2(fd, STDERR_FILENO) == -1){
        perror(RED"Failed to redirect standard output"RESET);
    }
    if (close(fd) == -1){
        perror(RED"Failed to close the file"RESET);
    }
    printf(GREEN"Output will be seen in my.file\n"RESET);
    char *s = *argRedirect;
    if(*s == '/'){
        execvRedirection(argRedirect);
    }else{
        createChild(argRedirect);
    }
}



void redirect2(char *argRedirect[],char *fileName)// >> metodu
{
    //  printf("redirect2");
    int fd = open(fileName, CREATE_FLAGS, CREATE_MODE);
    if (fd == -1){
        perror("Failed to open my.file");
    }
    if (dup2(fd, STDOUT_FILENO) == -1){
        perror("Failed to redirect standard output");
    }
    if (close(fd) == -1){
        perror("Failed to close the file");
    }
    printf("Output will be seen in my.file\n");
    char *s = *argRedirect;
    if(*s == '/'){
        //printf("\n EXE redirect2");
        execvRedirection(argRedirect);
    }else{
        createChild(argRedirect);
    }
}


void redirect1(char *argRedirect[],char *fileName)// > metodu
{
    // printf("redirect1");
    int fd;                                             //0600 üstüne yazmasını sağlıyor içeriği temizleyip yazıyor
    fd = open(fileName, O_CREAT | O_TRUNC | O_WRONLY, 0600);//O_APPEND: The file is opened in append mode.
    if (fd == -1){                                          //O_CREAT : If the file does not exist it will be created.
        perror(RED"Failed to open file.out"RESET);          //O_TRUNC : If the file already exists and is a regular file and the open mode allows writing.
    }
    if (dup2(fd, STDOUT_FILENO) == -1){
        perror(RED"Failed to redirect standard output"RESET);
    }
    if (close(fd) == -1){
        perror(RED"Failed to close the file"RESET);
    }
    printf("Output will be seen in my.file\n");
    char *s = *argRedirect;
    if(*s == '/'){
        printf("\n EXE redirect1");
        execvRedirection(argRedirect);
    }else{
        createChild(argRedirect);
    }
}

void fileIOOperation1(char * args[], char* inputFile, char* outputFile)
{
    printf("fileIOOperation1");
    int fileDescriptor; // between 0 and 19, describing the output or input file
    fileDescriptor = open(inputFile, O_RDONLY, 0600); // O_RDONLY: Open for reading only.
    dup2(fileDescriptor, STDIN_FILENO);

    close(fileDescriptor);
    fileDescriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
    dup2(fileDescriptor, STDOUT_FILENO);
    close(fileDescriptor);
    char *s = *args;
    if(*s == '/')
    {
        printf("\n EXE fileIOOperation1");
        execvRedirection(args);
    }
    else
    {
        printf("CREATE 1");
        createChild(args);
    }
}

void fileIOOperation2(char * args[], char* inputFile) // < metodu
{
    printf("fileIOOperation2");
    int fileDescriptor;
    fileDescriptor = open(inputFile, O_RDONLY, 0600);// O_RDONLY: Open for reading only.
    dup2(fileDescriptor, STDIN_FILENO);
    close(fileDescriptor);
    char *s = *args;
    if(*s == '/'){
        printf("\n EXE fileIOOperation2");
        execvRedirection(args);
    }else{
        printf("CREATE 2");
        createChild(args);
    }
}


////////////***********search command****///
void doSearch(char *args[]){

    int length = strlen(args[1]);
    char temp[length];
    char temp2[length];
    strcpy(temp, args[1]);
    fprintf(stderr, "\n%s\n", temp);
    int i;
    int j=0;

    for(i=0; i<length; i++){
        if(temp[i]=='"'){
            continue;
        }
        else{
            temp2[j] = temp[i];
            j++;
        }
    }

    temp2[j] = '\0';

    struct dirent *dirSearch; // Pointer for directory entry

    FILE *filePtr;
    DIR *dirPtr = opendir("."); // opendir)() returns a pointer of DIR type

    if( dirPtr == NULL){
        fprintf(stderr, "\nCould not open current directory\n" );
    
    }

    char *afterDot;
    char buffer[MAX_LINE];
    int lineNumber=0;
    char *occurence;

    while(  (dirSearch = readdir(dirPtr)) != NULL ) {
        if( strcmp(dirSearch->d_name, ".") == 0  || strcmp(dirSearch->d_name, "..") == 0)
            continue; // skip these inorder to keep looking for current directory content

        afterDot = strchr(dirSearch->d_name, '.');
        if (afterDot == NULL) continue;


        fprintf(stderr, "\n\t\tAfter dot:%s\n", afterDot);


        // if current d_name is a header file or main c file
        if((strcmp(afterDot, ".c") == 0) || (strcmp(afterDot, ".h") == 0) || (strcmp(afterDot, ".C") == 0) || (strcmp(afterDot, ".H") == 0) ){

            filePtr = fopen(dirSearch->d_name, "r");

            if(filePtr == NULL){
                fprintf(stderr, "\nFile could not opened\n" );
             
            }
            else{
                lineNumber = 0;
                while(fgets(buffer, MAX_LINE, filePtr)){

                    occurence = strstr(buffer, temp2);

                    if(occurence){

                        fprintf(stderr, "\n\t %d: directory:%s -> \t appearedLine:%s\n",lineNumber, dirSearch->d_name, buffer   );
                    }
                    lineNumber++;
                }
            }
        }

    }

    closedir(dirPtr);

}
////////////***********end of search****///





void display()
{
    struct node *ptr;
    if (start == NULL) {
        printf("nList is empty:n");
        return;
    } else {
        ptr = start;
        while (ptr != NULL) {
            fprintf(stderr, "\n location: %d \t content:%s", ptr->info, ptr->content);
            ptr = ptr->next;
        }
    }
}



char* indexDisplay( int index)
{
    struct node *ptr;
    static char* bookmarkContent;

    if (start == NULL) {
        printf("nList is empty:n");
        return 0;
    } else {
        ptr = start;
        printf("nThe List elements are:n");
        while (ptr != NULL) {
            if(ptr->info == index){

                printf("%dt", ptr->info);
                return ptr->content;
            }
            else{
                ptr = ptr->next;
            }
        }
    }
}





void insert_end(char *args[], int location, int argc)
{
    struct node *temp,*ptr;
    temp=(struct node *)malloc(sizeof(struct node));
    if(temp==NULL)
    {
        printf("nOut of Memory Space:n");
        return;
    }

    int i = 0;


    strcpy(temp->content, args[1] );
    strcat(temp->content, " ");
    for(i=2; i<argc; i++){
        strcat(temp->content, args[i]);
        strcat(temp->content, " ");
    }


    temp->info =location;
    temp->next =NULL;
    if(start==NULL)
    {
        start=temp;
    }
    else
    {
        ptr=start;
        while(ptr->next !=NULL)
        {
            ptr=ptr->next ;
        }
        ptr->next =temp;
    }
}




void delete_pos(int pos)
{
    int i;
    struct node *temp,*ptr;
    if(start==NULL)
    {
        printf("nThe List is Empty:n");
        exit(0);
    }
    else
    {

        if(pos==0)
        {
            ptr=start;
            start=start->next ;
            printf("\nThe deleted element is:%d \tits content is:%s", ptr->info, ptr->content );
            free(ptr);
        }
        else
        {
            ptr=start;
            for(i=0;i<pos;i++) {
                temp=ptr;
                ptr=ptr->next ;

                if(ptr==NULL)
                {
                    printf("nPosition not Found:n");
                    return;
                }
            }
            temp->next =ptr->next ;
            printf("nThe deleted element is:%dt",ptr->info );
            free(ptr);
        }
    }
}



void reConstruct(){

    struct node *temp;
    int location = 0;
    int count = 0;

    if(start == NULL){
        fprintf(stderr, "\nList is Empty\n");
    }
    else {
        temp = start;
        while (temp->next != NULL){
//            temp->info = location;
            count++;
            temp = temp->next;
        }
        count++;
        temp = start;
        for(location=0; location<count; location++){
            temp->info = location;
            temp = temp->next;
        }
    }
}



//////

int main(void)
{

    //declarations
    char bookmark[] = "bookmark";
    char ps_all[] = "ps_all";
    char exitText[] = "exit";
    char search[] = "search";
    int state;
    int location = 0;
    int index = 1;




    PathList(); /* gets environment paths like,
                        token: /home/userName/bin
                        token: /usr/share/Modules/bin
                        token: /usr/local/bin
                        token: /usr/local/sbin
                        token: /usr/bin
                        token: /usr/sbin
                        token: (null)*/


    while (1) {

        int argc = 0;

        char inputBuffer[MAX_LINE]; /*buffer to hold command entered */
        char *args[MAX_LINE / 2 + 1]; /*command line arguments */
        char *argsFileIO[256];
        int background = 0; /* equals 1 if a command is followed by '&' */
        int j = 0;
        pid_t childPid;
        int ioBoolean = 0;

        printf(GREEN"\nMyShell: "RESET);
        fflush(stdout); //using in order to be handle use of printf
        /*setup() calls exit() when Control-D is entered */
        setup(inputBuffer, args, &background);

        int a = 0;
        while(args[a] != NULL)
        {
            a++;
        }
        //printf();
        childPid = fork();//System call fork() is used to create processes
        if( childPid<0)
        {
            perror(RED"\nError in fork.\n"RESET);
        }
        if (childPid == 0) // this part is taken from execmd.c lab example.
        {
            while ( args[j] != NULL)
            { // I research (">", "<",">>","2>") these carecter if there is exist
                argc++;

                if ( (strcmp(args[j],">") == 0) || (strcmp(args[j],"<") == 0) || (strcmp(args[j],">>") == 0) || (strcmp(args[j],"2>") == 0) ){
                    ioBoolean = 1;
                    printf("\n İO boolean aktifleşti\n");
                    break;
                }
                argsFileIO[j] = args[j];
                if (strcmp(args[j],"&") == 0){
                    background = 1;
                    args[j] = NULL;
                }
                j++;
            } //end of while
            argsFileIO[j] = NULL;

            if (strcmp(ps_all, args[0]) == 0)
            {
                // ps_all
            }
            else if (strcmp(search, args[0]) == 0)
            {

            	doSearch(args);
            }
            else if (strcmp(bookmark, args[0]) == 0)
            {

                if(args[1][0] == '"'){// adding to bookmark

                    insert_end(args, location, argc);
//                display();
                    location++;

                }else if(args[1][0] == '-' && args[1][1] == 'i'){// -i idx executes the command bookmarked at index idx

                    index = atoi(args[2]);

                    // displaye bi index koy indexteki contenti return et
                    char *result = indexDisplay(index);
                    //retrun edilen stringi al
                    // stringi tırnaklarını çıkar

                    int length = strlen(result);
                    char temp[length];
                    char temp2[length];
                    strcpy(temp, result);

                    int i;
                    int j=0;
                    for(i=0; i<length; i++){
                        if(temp[i]=='"'){
                            continue;
                        }
                        else{
                            temp2[j] = temp[i];
                            j++;
                        }
                    }

                    temp2[j] = '\0';
                    //tırnaktan çıkanı al git createChild a ver

                    strcpy(args[0], temp2);

                    int k = 1;
                    int m = 0;

                    for(k=0; k<length; k++){
                        if(temp2[k] == ' '){
                            m++;
                        }
                    }

                    int l = 0;
                    int t = 0;

                    if(m<=argc){
                        for(k=m; k<argc; k++){
                            args[k] = NULL;
                        }
                    }
                    m = 0;

                    char tempString[length];
                    for(l=0; l<length; l++){
                        tempString[l] = NULL;
                    }

                    for(k=0; k<strlen(temp2); k++){
                        if(temp2[k] == ' ' ){
                            strcpy(args[m], tempString);
                            m++;

                            for(l=0; l<t; l++){
                                tempString[l] = NULL;
                            }
                            t = 0;
                            continue;
                        }
                        else if(temp2[k] == NULL){
                            break;
                        }
                        else{
                            tempString[t] = temp2[k];
                            t++;
                        }
                    }


                    createChild(args);

                }else if(args[1][0] == '-' && args[1][1] == 'd'){// -d idx deletes the command at index idx, shift rest

                    index = atoi(args[2]);

                    delete_pos(index);
                    display();
                    reConstruct();
                    display();
                    location--;

                }else if(args[1][0] == '-' && args[1][1] == 'l'){// list all

                    display();
                }
            }

            else if (strcmp(exitText, args[0]) == 0)
            {
                printf(WHITE"Logging out...\n"RESET);
                kill(childPid,SIGTERM); // kiil yaparken macos da hata verdi kodun en üstündeki define ı ekledim
                exit(0);

            }

            else if(ioBoolean == 1)//if ioBoolean is one, it means that we will do file oeration.
            {
                int i = 0;
                while(args[i] != NULL){               // burada doğru input formatı giriliyor mu onu kontrol ediyoruz if ler ile.
                    if(strcmp(args[i],"<") == 0){
                        if(args[i+1] != NULL){
                            if(args[i+2] != NULL){
                                if(strcmp(args[i+2],">") == 0 && args[i+3] != NULL){
                                    if(args[i+4] != NULL){
                                        printf("read/write case error.\n");
                                        break;
                                    }else{
                                        printf("read/write case works\n");
                                        fileIOOperation1(argsFileIO, args[i + 1], args[i + 3]);//5
                                        break;
                                    }
                                }else{
                                    printf(RED"read/write case error.\n"RESET);
                                    break;
                                }
                            }else{
                                fileIOOperation2(argsFileIO,args[i+1]);//3
                                break;
                            }
                        }else{
                            printf(RED"read case error\n"RESET);
                            break;
                        }
                    }else if(strcmp(args[i],">") == 0){// 1
                        if(args[i+1] != NULL && args[i+2] == NULL){
                            redirect1(argsFileIO,args[i+1]);
                            break;
                        }else{
                            printf(RED"write case error\n"RESET);
                        }
                    }else if(strcmp(args[i],">>") == 0){// 2
                        if(args[i+1] != NULL && args[i+2] == NULL){
                            redirect2(argsFileIO,args[i+1]);
                            break;
                        }else{
                            printf(RED">> case error"RESET);
                            break;
                        }
                    }else if(strcmp(args[i],"2>") == 0){
                        if(args[i+1] != NULL && args[i+2] == NULL){
                            error(argsFileIO,args[i+1]);//4
                        }else{
                            printf(RED"2> case error"RESET);
                            break;
                        }
                    }
                    ///////   <  > son file işlemi var
                    i++;
                }   //end of while loop
            } // file kısmı bitiyo
            else
            { //execute
                char *s = *args;
                if(*s == '/'){
                    execvRedirection(args);
                }else{
                    createChild(args);
                }
            }
        }
        else//parent process scope
        {
            if (background == 0)
            {
                waitpid(childPid,NULL,0);
                if(strcmp(args[0],"exit") == 0)
                {
                    int state;
                    pid_t result = waitpid(BackID, &state, WNOHANG);
                    if (result == 0)
                    {
                        printf("There are background processes..\n");
                        // Child still alive
                    }
                    else
                    {
                        // Child exited
                        printf("Goodbye...\n");
                        kill(getppid(),SIGTERM);
                        exit(0);
                    }

                }
            }
            else
            {
                BackID = childPid;
                printf("\nProcess created with PID: %d\n",childPid);
                printf("Parent id is = %d\n",getpid());
            }
        }

    } // end of while loop
} //end of main method

