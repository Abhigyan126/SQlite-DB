// https://cstack.github.io/db_tutorial/parts/part1.html

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

// constants

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0) -> Attribute)

//structs and enums
typedef struct {
    char* buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;

typedef enum {
    EXECUTE_SUCCESS,
    EXECUTE_TABLE_FULL
} ExecuteResult;

typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
}MetaCommandResult;

typedef enum {
    PREPARE_SUCCESS,
    PREPARE_SYNTAX_ERROR,
    PREPARE_UNRECOGNIZED_STATEMENT
}PrepareResult;

typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT 
}StatementType;

typedef struct {
    u_int32_t id;
    char username[COLUMN_USERNAME_SIZE];
    char email[COLUMN_EMAIL_SIZE];
} Row;

typedef struct {
    StatementType type;
    Row row_to_insert; // used only by insert statement
} Statement;

// constants

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

const uint32_t PAGE_SIZE = 4096;
#define TABLE_MAX_PAGES 100
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

//structs

typedef struct {
    uint32_t num_rows;
    void* pages[TABLE_MAX_PAGES];
} Table;

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
* @brief function to print attribute of Row struct
* @param pointer to a struct Row
* @return None
*/
void print_row(Row* row) {
    printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}

/*
* @brief this function serializes by copying attributes 
* from Row struct to raw block of memory
* @param pointer to a struct Row, source 
* @param void pointer destination (address of raw memory)
* @return NONE
*/
void serialize_row(Row* source, void* destination) {
    memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy(destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
    memcpy(destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

/*
* @brief this function deserializes by copying raw block of memory into a struct Row
* @param void pointer source (address of raw memory)
* @param pointer to a struct Row, destination
* @return NONE
*/
void deserialize_row(void* source, Row* destination) {
    memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);

}

/*
* @brief return a pointer to the memory slot where a specific row should be stored in table
* calculates pag_num and checks if table->page exists by cmo NULL if it doesnot memory is
* allocated using malloc and the page and byte offset is returned
* @param pointer to a struct Table, table
* @param row number
* @ return pointer to memory slot of specified row 
*/
void* row_slot(Table* table, u_int32_t row_num) {
    u_int32_t page_num = row_num/ ROWS_PER_PAGE;
    void *page = table->pages[page_num];
    if (page == NULL) {
        page = table->pages[page_num] = malloc(PAGE_SIZE);
    }
    uint32_t row_offset = row_num % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * ROW_SIZE;
    return page+byte_offset;
}

/*
* @brief this function creates a Table struct and initialises its attributes
* @param NONE
* @return pointer to initailised Table struct
*/
Table* new_table() {
    Table* table = (Table*)malloc(sizeof(Table));
    table->num_rows = 0;
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        table->pages[i] = NULL;
    }
    return table;
}

/*
* @brief this function free the struct Table and its attributes
* @param pointer to a struct of type Table, table
* @return NULL
*/
void free_table(Table* table) {
    for (int i = 0; table->pages[i]; i++) {
        free(table->pages[i]);
    }
    free(table);
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
        int arg_assigned = sscanf(input_buffer->buffer, "insert %d %s %s", &(statement->row_to_insert.id), &(statement->row_to_insert.username), &(statement->row_to_insert.email));
        if (arg_assigned < 3) {
            return PREPARE_SYNTAX_ERROR;
        }
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
@brief function to insert information into table by serialising to specific location
@param pointer to type Statement
@param pointer to type Table
@return enum ~ Table full and success
*/
ExecuteResult execute_insert(Statement* statement, Table* table) {
    if (table->num_rows >= TABLE_MAX_ROWS) {
        return EXECUTE_TABLE_FULL;
    }
    Row* row_to_insert = &(statement->row_to_insert);
    serialize_row(row_to_insert, row_slot(table, table->num_rows));
    table->num_rows += 1;

    return EXIT_SUCCESS;
}

/*
@brief function to handle select statement by handeling the deserialised rows iterativly
@param pointer to type Statement
@param pointer to type Table
@return enum ~ success
*/
ExecuteResult execute_select(Statement* statement, Table* table) {
    Row row;
    for (uint32_t i = 0; i < table->num_rows; i++) {
        deserialize_row(row_slot(table, i), &row);
        print_row(&row);
    }
    return EXIT_SUCCESS;
}

/*
* @brief this function switches the context based on the statment -> type
* @param pointer to a Statement
* @return NONE
*/
ExecuteResult execute_statement(Statement* statement, Table* table) {
    switch (statement -> type) {
        case (STATEMENT_INSERT):
            return execute_insert(statement, table);
        case (STATEMENT_SELECT):
            return execute_select(statement, table);
    }
}

int main(int argc, char* argv[]) {
    Table* table = new_table();
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
            case(PREPARE_SYNTAX_ERROR):
                printf("Syantax error \n");
                continue;;
            case (PREPARE_UNRECOGNIZED_STATEMENT):
                printf("Unrecognized keyword at start of '%s'.\n",input_buffer->buffer);
                continue;
        }

        switch (execute_statement(&statement, table)) {
            case(EXECUTE_SUCCESS):
                printf("Executed \n");
                break;
            case(EXECUTE_TABLE_FULL):
                printf("Table full \n");
                break;
        }    
    }
}