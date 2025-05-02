// https://cstack.github.io/db_tutorial/parts/part1.html

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct 
{
    char* buffer;
    size_t buffer_length;
    ssize_t input_length;

} InputBuffer;

InputBuffer* new_input_buffer() {
    InputBuffer* input_buffer = malloc(sizeof(InputBuffer));
    input_buffer -> buffer = NULL;
    input_buffer -> buffer_length = 0;
    input_buffer -> input_length = 0;
    
    return input_buffer;
}

void free_input_buffer(InputBuffer* input_buffer) {
    free(input_buffer -> buffer);
    free(input_buffer);
}

void print_prompt() {
    printf("DB > ");
}

void read_input(InputBuffer* input_buffer) {
    ssize_t input = getline(&(input_buffer -> buffer), &(input_buffer -> buffer_length), stdin);
    if (input <= 0) {
        printf("Error Reading Line \n");
        exit(EXIT_FAILURE);
    }
    input_buffer -> input_length = input-1;
    input_buffer -> buffer[input - 1] = 0;

}

void close_input_buffer(InputBuffer* input_buffer) {
    free(input_buffer->buffer);
    free(input_buffer);
}


int main(int argc, char* argv[]) {
    InputBuffer* input_buffer = new_input_buffer();

    while (true) {
        print_prompt();
        read_input(input_buffer);

        if (strcmp(input_buffer->buffer, ".exit") == 0) {
            close_input_buffer(input_buffer);
            exit(EXIT_SUCCESS);
          } else {
            printf("Unrecognized command '%s'.\n", input_buffer->buffer);
    }
    
    }
}