#include <string.h>
#include <stdlib.h>

char ** httpHeaderReader(char * header, int headerLength){
    char ** headerInformation = malloc(sizeof(char *) * 2);

    int i;
    for(i = 0; header[i] != ' '; i++);

    header[i] = '\0';

    headerInformation[0] = malloc(sizeof(char) * (i+1));
    strcpy(headerInformation[0], header);

    header = header+i+1;

    for(i = 0; header[i] != ' '; i++);

    header[i] = '\0';

    if(!strcmp(header, "/")){
        headerInformation[1] = malloc(strlen("/index.html")+1);
        strcpy(headerInformation[1], "/index.html");
    }
    else{
        headerInformation[1] = malloc(sizeof(char) * (i+1));
        strcpy(headerInformation[1], header);
    }

    return headerInformation;
}