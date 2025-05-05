// https://cstack.github.io/db_tutorial/parts/part1.html

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

//structs and enums
typedef struct {
    char* buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;

typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
}MetaCommandResult;

typedef enum {
    PREPARE_SUCCESS, 
    PREPARE_UNRECOGNIZED_STATEMENT
}PrepareResult;

typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT 
}StatementType;

typedef struct {
    StatementType type;
} Statement;

/*
* @brief this function creates a new buffer of type InputBuffer by allocating memory
* @param NONE
* @return pointer to a buffer of type InputBuffer
*/
InputBuffer* new_input_buffer() {
    InputBuffer* input_buffer = malloc(sizeof(InputBuffer));
    input_buffer -> buffer = NULL;
    input_buffer -> buffer_length = 0;
    input_buffer -> input_length = 0;
    
    return input_buffer;
}

/*
* @brief this function deallocate memory to a buffer
* @param pointer to a buffer of type InputBuffer
* @return NONE
*/
void free_input_buffer(InputBuffer* input_buffer) {
    free(input_buffer -> buffer);
    free(input_buffer);
}

/*
* @brief this function deallocate memory to a buffer
* @param pointer to a buffer of type InputBuffer
* @return NONE
*/
void close_input_buffer(InputBuffer* input_buffer) {
    free(input_buffer->buffer);
    free(input_buffer);
}

/*
* @brief funvtion to display DB > before taking input
* @param NONE
* @return NONE
*/
void print_prompt() {
    printf("DB > ");
}

/*
* @brief this function takes in the input from the console and saves it in a buffer 
* and also removes the trailing null terminator \0
* @param pointer to input buffer of type InputBuffer
* @return NONE
*/
void read_input(InputBuffer* input_buffer) {
    ssize_t input = getline(&(input_buffer -> buffer), &(input_buffer -> buffer_length), stdin);
    if (input <= 0) {
        printf("Error Reading Line \n");
        exit(EXIT_FAILURE);
    }
    input_buffer -> input_length = input-1;
    input_buffer -> buffer[input - 1] = 0;
}

/*
* @brief this function handles and executes the meta command
* @param pointer to a buffer of type InputBuffer
* @return status of command execution
*/
MetaCommandResult do_meta_command(InputBuffer* input_buffer) {
    if (strcmp(input_buffer -> buffer, ".exit") == 0) {
        close_input_buffer(input_buffer);
        exit(EXIT_SUCCESS);
    } else {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

/*
* @brief this function handles the prepare statements enum / type detection
* @param pointer to buffer of type IputBuffer
* @param pointer to statement of type Statement
* @return Status of prepare statement detected
*/
PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement) {
    if (strncmp(input_buffer->buffer, "insert", 6) == 0) {
        statement->type = STATEMENT_INSERT;
        return PREPARE_SUCCESS;
    }
    // @warning can be logic error here
    if (strcmp(input_buffer->buffer, "select") == 0) {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}

/*
* @brief this function switches the context based on the statment -> type
* @param pointer to a Statement
* @return NONE
*/
void execute_statement(Statement* statement) {
    switch (statement -> type) {
        case (STATEMENT_INSERT):
            printf("Insert Logic will go here \n");
            break;
        case (STATEMENT_SELECT):
            printf("Select logic will go here \n");
            break;
    }
}

int main(int argc, char* argv[]) {
    InputBuffer* input_buffer = new_input_buffer();

    while (true) {
        print_prompt();
        read_input(input_buffer);

        if (input_buffer -> buffer[0] == '.') {
            switch (do_meta_command(input_buffer)) {
                case (META_COMMAND_SUCCESS):
                    continue;
                case (META_COMMAND_UNRECOGNIZED_COMMAND):
                    printf("Unrecognized command '%s'\n", input_buffer->buffer);
                    continue;
            }
        }
        Statement statement;
        switch (prepare_statement(input_buffer, &statement)) {
            case (PREPARE_SUCCESS):
                break;
            case (PREPARE_UNRECOGNIZED_STATEMENT):
                printf("Unrecognized keyword at start of '%s'.\n",input_buffer->buffer);
                continue;
        }

        execute_statement(&statement);
        printf("Executed \n");
    
    }
}