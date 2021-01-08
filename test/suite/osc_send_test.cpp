
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <lo/lo.h>
#include <unistd.h>
#include <fstream>
#include <string>




int main(int argc, char *argv[]){    

    lo_address t = lo_address_new(NULL, "7777");

    char *stuff;
    if (argc == 2){
        char *filename = argv[1];
        std::ifstream ifs(filename);
        std::string content( (std::istreambuf_iterator<char>(ifs) ),
                   (std::istreambuf_iterator<char>()    ) );
        stuff = new char [content.length()+1];
        std::strcpy (stuff, content.c_str());
    }
    else{
        std::cout << "usage: ./sndr <scorefile>" << std::endl;
        return 0;
    }

    std::cout << stuff << std::endl;

    if (lo_send(t, "/RTcmix/ScoreCommands", "s", stuff) == -1){
        printf("OSC error %d: %s\n", lo_address_errno(t),
               lo_address_errstr(t));
    }



    return 0;
}



