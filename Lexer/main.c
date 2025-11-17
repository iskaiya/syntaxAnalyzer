#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokens.h"
#include <ctype.h>
#include <stdbool.h>
#include "wordhash.h"
//States
typedef enum {
    S_START,   //Start state

    //Words without quotes(" ")
    S_IDENTIFIER,       
    S_KEYWORD,  // Note: These are final states, decided *after* S_IDENTIFIER
    S_RESERVE,  // Note: These are final states, decided *after* S_IDENTIFIER
    S_NOISE,    // Note: These are final states, decided *after* S_IDENTIFIER

    //Numbers
    S_NUMBER_BILANG,
    S_NUMBER_LUTANG,

    //Strings & characters
    S_KWERDAS_HEAD,   //for double quote start
    S_KWERDAS_BODY,    //main string
    S_KWERDAS_TAIL,    //last double quote

    //Strings & characters
    S_TITIK_HEAD,   //for single quote start
    S_TITIK_BODY,    //main charatcer 
    S_TITIK_TAIL,    //last single quote

    // Operators
    S_OP_PLUS,
    S_OP_MINUS,        
    S_OP_MULTIPLY,
    S_OP_POW,
    S_OP_MOD,      
    S_OP_DIVIDE_HEAD,      // /  (may lead to comments)
    S_OP_ASSIGN_HEAD,      // =
    S_OP_ASSIGN_TAIL,  // == (Final State)
    S_OP_NOT_HEAD,         // !
    S_OP_NOT_TAIL,     // != or ! (Final State)
    S_OP_LESS_HEAD,        // <
    S_OP_LESS_TAIL,    // <= or < (Final State)
    S_OP_GREATER_HEAD,     // >
    S_OP_GREATER_TAIL, // >= or > (Final State)
    S_OP_AND_HEAD,     //&
    S_OP_AND_TAIL,     // && (Final State)
    S_OP_OR_HEAD,      // |  
    S_OP_OR_TAIL,      // || (Final State)

    //Comments
    S_COMMENT_SINGLE,  // //
    S_COMMENT_MULTI_HEAD,   // /*
    S_COMMENT_MULTI_TAIL, // checking for */

    // Delimiters
    S_DELIMITER,       // ( ) { } [ ] , . ; etc. (Final State)

    // End / Unknown
    S_UNKNOWN,
    S_DONE // (Unused in this implementation)
} LexerState;

// func prototypes
void lexer(FILE *file, FILE *symbolFileAppend);
Token makeToken(TokenCategory cat, int tokenValue, const char *lexeme, int lineNumber);
void printToken(FILE *file, Token *t);
int checkExtension(const char *filename);
static const char *token_value_name(const Token *t);

int main() {
    char filename[100];
     initialize_table();
    do {
        printf("Please enter file name (should be in the same directory): ");
        scanf("%s", filename);
        //check extension, should be .usb
        if (checkExtension(filename) == 0) {
            printf("File must have .usb extension\n");
            continue;
        } 
        //if .usb, try to open file
        FILE *file = fopen(filename, "r");
        //file check if visible
        if(!file){
            printf("File '%s' not found or cannot be opened.\n", filename);
            continue;
        } else {
            printf("%s opened successfully.\n", filename);
        }
        //create symbol table for output
        fopen("Symbol Table.txt", "w");
        FILE *symbolFileAppend;
        symbolFileAppend = fopen("Symbol Table.txt", "a");
        fprintf(symbolFileAppend, "Lexeme           | Token Name\n");
        lexer(file, symbolFileAppend);
        fclose(file); 
        fclose(symbolFileAppend);
        printf("Symbol Table.txt is created for %s. \n", filename);
        break;
    } while (true);
    
    system("pause"); 
    return EXIT_SUCCESS;
}

// Lexer function that reads characters from the file and produces Token structs
void lexer (FILE *file, FILE *symbolFileAppend) {
   
    LexerState currentState = S_START;
    char lexemeBuffer[1024]; //can hold max of 1024 characters of a single lexeme
    int lexemeIndex = 0;
    int lineNumber = 1;
    int tokenStartLine = 1; 
    
    int c; // Current character

    //START --> reads/chcks 1 character per iteration.
    while (true) { //keep looping until encounter eof (use return to exit lexer)
        
        c = fgetc(file); // Get first char
        Token tok; //declare struct for tokens
        switch (currentState) {

            //START STATE:
            case S_START:
                lexemeIndex = 0; //set buffer index to 0
                memset(lexemeBuffer, 0, sizeof(lexemeBuffer));//clear lexeme buffer array
                tokenStartLine = lineNumber;

                if (c == EOF) {
                    return; //get out of lexer if eof is enocountered
                }

                if (isspace(c)) { 
                    //ignore white spaces
                    if (c == '\n') {
                        lineNumber++;
                    }
                    currentState = S_START;//remain in state
                    continue; 
                }

                //if not space, then input current char to buffer
                lexemeBuffer[lexemeIndex++] = (char)c;

                //check character 
                if (isalpha(c)) { 
                    currentState = S_IDENTIFIER;
                } else if(c == '_'){
                    currentState = S_UNKNOWN;
                } else if (isdigit(c)) {
                    currentState = S_NUMBER_BILANG;
                } else if (c == '"') {
                    currentState = S_KWERDAS_HEAD;
                } else if (c == '\'') {
                    currentState = S_TITIK_HEAD;
                } else if (c == '/') {
                    currentState = S_OP_DIVIDE_HEAD;
                } else if (c == '&') {
                    currentState = S_OP_AND_HEAD;
                } else if (c == '|') {
                    currentState = S_OP_OR_HEAD;
                } else if (c == '=') {
                    currentState = S_OP_ASSIGN_HEAD;
                } else if (c == '!') {
                    currentState = S_OP_NOT_HEAD;
                } else if (c == '<') {
                    currentState = S_OP_LESS_HEAD;
                } else if (c == '>') {
                    currentState = S_OP_GREATER_HEAD;
                } else {
                    //Single character lexemes are auto final state
                    lexemeBuffer[lexemeIndex] = '\0';
                    switch (c) {
                        // OPERATORS transition to their own state
                        case '+': currentState = S_OP_PLUS; break;
                        case '-': currentState = S_OP_MINUS; break;
                        case '*': currentState = S_OP_MULTIPLY; break;
                        case '^': currentState = S_OP_POW; break; 
                        case '%': currentState = S_OP_MOD; break;
                        
                        // DELIMITERS transition to the S_DELIMITER state
                        case ';': currentState = S_DELIMITER; break;
                        case '{': currentState = S_DELIMITER; break;
                        case '}': currentState = S_DELIMITER; break;
                        case '(': currentState = S_DELIMITER; break;
                        case ')': currentState = S_DELIMITER; break;
                        case '[': currentState = S_DELIMITER; break;
                        case ']': currentState = S_DELIMITER; break;
                        case ',': currentState = S_DELIMITER; break;
                        case '.': currentState = S_DELIMITER; break;
                        default: currentState = S_UNKNOWN; break;
                    }
                }
                break;

    
            case S_IDENTIFIER: 
                if (isalnum(c) || c == '_') {
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    currentState = S_IDENTIFIER;
                } else {
                    if (c != EOF){
                        ungetc(c, file);
                    } 
                        lexemeBuffer[lexemeIndex] = '\0'; // Finalize
                        HashEntry *entry = hashLookUp(lexemeBuffer);
                    if (entry) {
                        tok = makeToken(entry->category, entry->tokenValue, lexemeBuffer, tokenStartLine);
                    } else {
                        tok = makeToken(CAT_LITERAL, L_IDENTIFIER, lexemeBuffer, tokenStartLine);
                    }
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START; //reset to start
                }
                break;

            //Numbers (BILANG & LUTANG) States
            case S_NUMBER_BILANG:
                if (isdigit(c)) {
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    // Stay in S_NUMBER_BILANG
                } else if (c == '.') {
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    currentState = S_NUMBER_LUTANG; // Transition
                } else if(isalpha(c)){ //unexpected char 
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    currentState = S_UNKNOWN;
                }else {
                    if (c != EOF){
                        ungetc(c, file);
                    }
                    lexemeBuffer[lexemeIndex] = '\0';
                    tok = makeToken(CAT_LITERAL, L_BILANG_LITERAL, lexemeBuffer, tokenStartLine);
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START; // Reset
                }
                break; 

            case S_NUMBER_LUTANG:
                if (isdigit(c)) {
                    lexemeBuffer[lexemeIndex++] = (char)c;
                } else {
                    if (c != EOF){
                         ungetc(c, file);
                    }
                    lexemeBuffer[lexemeIndex] = '\0';
                    //check if . is last number (error checking)
                    if (lexemeBuffer[lexemeIndex - 1] == '.') { // e.g., "123."
                        tok = makeToken(CAT_UNKNOWN, 0, lexemeBuffer, tokenStartLine);
                    } else {
                        tok = makeToken(CAT_LITERAL, L_LUTANG_LITERAL, lexemeBuffer, tokenStartLine);
                    }
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START; // Reset
                }
            break; 

            //KWERDAS STATES
            case S_KWERDAS_HEAD: //previous input is double quotes
                if (c == '"') {//means end of string
                    currentState = S_KWERDAS_TAIL; // Go to TAIL state
                    continue; 
                }
                if (c == EOF || c == '\n') {
                    if (c != EOF){
                        ungetc(c, file);
                    }
                        lexemeBuffer[0] = '"'; // Show the unterminated quote
                        lexemeBuffer[1] = '\0';
                        tok = makeToken(CAT_DELIMITER, D_QUOTE, lexemeBuffer, tokenStartLine);
                        printToken(symbolFileAppend, &tok);
                        //current state is final state therefore go to start state
                        currentState = S_START;
                    if(c == '\n') 
                        lineNumber++;
                } else {
                    //not eof or next line therefore part of the kwerdas
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    currentState = S_KWERDAS_BODY;
                }
            break;

            case S_KWERDAS_BODY:
                if (c == '"') {
                    currentState = S_KWERDAS_TAIL; //second quote --> end of string
                } else if (c == EOF || c == '\n') {
                    //error check
                    if (c != EOF){
                        ungetc(c, file);
                    } 
                    lexemeBuffer[lexemeIndex] = '\0';
                    tok = makeToken(CAT_UNKNOWN, 0, lexemeBuffer, tokenStartLine); // Unterminated string
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START; //go to next lexeme
                    if(c == '\n') 
                        lineNumber++;
                    } else {
                        lexemeBuffer[lexemeIndex++] = (char)c;
                }
            break;

            case S_KWERDAS_TAIL: //input: second " (final state)
                if (c != EOF){
                     ungetc(c, file);
                } 
                    lexemeBuffer[lexemeIndex++] = '\"'; 
                    lexemeBuffer[lexemeIndex] = '\0';
                    tok = makeToken(CAT_LITERAL, L_KWERDAS_LITERAL, lexemeBuffer, tokenStartLine);
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START; //move on to next lexeme
            break;

            // for potential chars
            case S_TITIK_HEAD: //previous input '
                if (c == '\'' || c == EOF || c == '\n') {
                    //error or final state
                    if (c != EOF)
                        ungetc(c, file);
                        lexemeBuffer[0] = '\'';
                        lexemeBuffer[1] = '\0';
                        Token tok = makeToken(CAT_DELIMITER, D_SQUOTE, lexemeBuffer, tokenStartLine); 
                        printToken(symbolFileAppend, &tok);
                        currentState = S_START;
                    if(c == '\n') 
                        lineNumber++;
                } else {
                    // this mean character or space is the next input
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    currentState = S_TITIK_BODY;
                }
                break;
            
            case S_TITIK_BODY: //previous input is alphanum
                if (c == '\'') {
                    currentState = S_TITIK_TAIL; //send to final state
                } else {
                    if (c != EOF) 
                        ungetc(c, file); 
                    lexemeBuffer[lexemeIndex] = '\0';
                    tok = makeToken(CAT_UNKNOWN, 0, lexemeBuffer, tokenStartLine);
                    printToken(symbolFileAppend, &tok);
                    //go to next lexeme
                    currentState = S_START;
                }
                break;

            case S_TITIK_TAIL: //previous input: char or soace
                if (c != EOF){
                    ungetc(c, file);
                }
                lexemeBuffer[lexemeIndex++] = '\'';
                lexemeBuffer[lexemeIndex] = '\0';
                tok = makeToken(CAT_LITERAL, L_TITIK_LITERAL, lexemeBuffer, tokenStartLine);
                printToken(symbolFileAppend, &tok);
                currentState = S_START;
                break;
            
            case S_OP_DIVIDE_HEAD: //prev input: /
                if (c == '/') {
                    //comment 
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    currentState = S_COMMENT_SINGLE;
                } else if (c == '*') {
                    // commment 
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    currentState = S_COMMENT_MULTI_HEAD;
                } else {
                    //divide operator
                    if (c != EOF) ungetc(c, file); 
                    lexemeBuffer[lexemeIndex] = '\0';
                    tok = makeToken(CAT_OPERATOR, O_DIVIDE, lexemeBuffer, tokenStartLine);
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START; 
                }
                break; 

            case S_COMMENT_SINGLE:
                if (c == '\n' || c == EOF) {
                    //single line
                    if (c != EOF) ungetc(c, file); 
                    lexemeBuffer[lexemeIndex] = '\0';
                    tok = makeToken(CAT_COMMENT, C_SINGLE_LINE, lexemeBuffer, tokenStartLine);
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START; 
                } else {
                    lexemeBuffer[lexemeIndex++] = (char)c;
                }
                break;

            case S_COMMENT_MULTI_HEAD:
                if (c == '\n') 
                lineNumber++;
            
                if (c == '*') {
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    currentState = S_COMMENT_MULTI_TAIL;
                } else if (c == EOF) {
                    lexemeBuffer[lexemeIndex] = '\0';
                    tok = makeToken(CAT_UNKNOWN, 0, lexemeBuffer, tokenStartLine); // Unterminated comment
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START; // Will be caught by EOF check
                } else {
                    lexemeBuffer[lexemeIndex++] = (char)c;
                   currentState = S_COMMENT_MULTI_HEAD;
                }
                break; 

            case S_COMMENT_MULTI_TAIL: //prev input: *
                 if (c == '\n') 
                 lineNumber++;
                 
                if (c == '/') {
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    lexemeBuffer[lexemeIndex] = '\0';
                    tok = makeToken(CAT_COMMENT, C_MULTI_LINE, lexemeBuffer, tokenStartLine);
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START; 
                } else if (c == '*') {
                    lexemeBuffer[lexemeIndex++] = (char)c; // Saw another *, e.g. "/***"
                    // Stay in S_COMMENT_MULTI_TAIL
                } else if (c == EOF) {
                    lexemeBuffer[lexemeIndex] = '\0';
                    tok = makeToken(CAT_UNKNOWN, 0, lexemeBuffer, tokenStartLine); // Unterminated comment
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START;
                } else {
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    currentState = S_COMMENT_MULTI_HEAD; // Not a /, go back
                }
                break; 

            case S_OP_AND_HEAD: //prev input: &
                if (c == '&') {
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    currentState = S_OP_AND_TAIL;
                    
                } else {
                    if (c != EOF) {
                        ungetc(c, file);
                    }
                    lexemeBuffer[lexemeIndex] = '\0';
                    currentState = S_UNKNOWN;
                }
                break;
            
            case S_OP_OR_HEAD: // prev inp: |
                 if (c == '|') {
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    currentState = S_OP_OR_TAIL; 
                } else {
                    if (c != EOF) {
                        ungetc(c, file);
                    }
                    lexemeBuffer[lexemeIndex] = '\0'; 
                    currentState = S_UNKNOWN;
                }
                break; 

            case S_OP_ASSIGN_HEAD: //prev inp: = 
                if (c == '=') {
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    currentState = S_OP_ASSIGN_TAIL; 
                } else {
                    if (c != EOF){
                        ungetc(c, file);
                    } 
                    lexemeBuffer[lexemeIndex] = '\0'; // Lexeme is just "="
                    tok = makeToken(CAT_OPERATOR, O_ASSIGN, lexemeBuffer, tokenStartLine);
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START;
                }
                break;
            
            case S_OP_NOT_HEAD: //prev inputt: !
                if (c == '=') {
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    currentState = S_OP_NOT_TAIL; 
                } else {
                    if (c != EOF) ungetc(c, file);
                    lexemeBuffer[lexemeIndex] = '\0';
                    tok = makeToken(CAT_OPERATOR, O_NOT, lexemeBuffer, tokenStartLine);
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START;
                }
                break;

            case S_OP_LESS_HEAD: //prev input : <
                if (c == '=') {
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    currentState = S_OP_LESS_TAIL; 
                } else {
                    if (c != EOF) { 
                        ungetc(c, file);
                    }
                    lexemeBuffer[lexemeIndex] = '\0'; // Lexeme is just "<"
                    tok = makeToken(CAT_OPERATOR, O_LESS, lexemeBuffer, tokenStartLine);
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START;
                }
                break;

            case S_OP_GREATER_HEAD: // Saw >
                if (c == '=') {
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    currentState = S_OP_GREATER_TAIL; 
                } else {
                    if (c != EOF) ungetc(c, file);
                    lexemeBuffer[lexemeIndex] = '\0'; // Lexeme is just ">"
                    tok = makeToken(CAT_OPERATOR, O_GREATER, lexemeBuffer, tokenStartLine);
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START;
                }
                break;
            
            
            case S_UNKNOWN:
                if (isspace(c) || c == EOF || strchr("+-*/^%&|=!<>(){}[],.;", c)) {
                    if (c != EOF){ 
                        ungetc(c, file);
                    }
                    lexemeBuffer[lexemeIndex] = '\0'; //terminator
                    tok = makeToken(CAT_UNKNOWN, 0, lexemeBuffer, tokenStartLine);
                    printToken(symbolFileAppend, &tok);
                    currentState = S_START; //reset to start state
                    if (c == '\n') 
                        lineNumber++; 
                } else {
                    //input all invalid characters to the buffer
                    //remain in state
                    lexemeBuffer[lexemeIndex++] = (char)c;
                    currentState = S_UNKNOWN;
                }
                break;

            case S_OP_PLUS:
                if (c != EOF) ungetc(c, file); 
                tok = makeToken(CAT_OPERATOR, O_PLUS, lexemeBuffer, tokenStartLine);
                printToken(symbolFileAppend, &tok);
                currentState = S_START;
                break;

            case S_OP_MINUS:
                if (c != EOF) ungetc(c, file); 
                tok = makeToken(CAT_OPERATOR, O_MINUS, lexemeBuffer, tokenStartLine);
                printToken(symbolFileAppend, &tok);
                currentState = S_START;
                break;

            case S_OP_MULTIPLY:
                if (c != EOF) ungetc(c, file); 
                tok = makeToken(CAT_OPERATOR, O_MULTIPLY, lexemeBuffer, tokenStartLine);
                printToken(symbolFileAppend, &tok);
                currentState = S_START;
                break;
            
            case S_OP_POW:
                if (c != EOF) ungetc(c, file); 
                tok = makeToken(CAT_OPERATOR, O_POW, lexemeBuffer, tokenStartLine); 
                printToken(symbolFileAppend, &tok); 
                currentState = S_START; 
                break; 

            case S_OP_MOD:
                if (c != EOF) ungetc(c, file); 
                tok = makeToken(CAT_OPERATOR, O_MODULO, lexemeBuffer, tokenStartLine); 
                printToken(symbolFileAppend, &tok); 
                currentState = S_START; 
                break; 

            case S_DELIMITER:
                if (c != EOF) ungetc(c, file); // Put back the char we just read

                // Switch on the character *in the buffer*
                switch (lexemeBuffer[0]) {
                    case ';': 
                        tok = makeToken(CAT_DELIMITER, D_SEMICOLON, lexemeBuffer, tokenStartLine); 
                        break;
                    case '{': 
                        tok = makeToken(CAT_DELIMITER, D_LBRACE, lexemeBuffer, tokenStartLine); 
                        break;
                    case '}': 
                        tok = makeToken(CAT_DELIMITER, D_RBRACE, lexemeBuffer, tokenStartLine); 
                        break;
                    case '(': 
                        tok = makeToken(CAT_DELIMITER, D_LPAREN, lexemeBuffer, tokenStartLine); 
                        break;
                    case ')': 
                        tok = makeToken(CAT_DELIMITER, D_RPAREN, lexemeBuffer, tokenStartLine); 
                        break;
                    case '[': 
                        tok = makeToken(CAT_DELIMITER, D_LBRACKET, lexemeBuffer, tokenStartLine); 
                        break;
                    case ']': 
                        tok = makeToken(CAT_DELIMITER, D_RBRACKET, lexemeBuffer, tokenStartLine); 
                        break;
                    case ',': 
                        tok = makeToken(CAT_DELIMITER, D_COMMA, lexemeBuffer, tokenStartLine); 
                        break;
                    case '.': 
                        tok = makeToken(CAT_DELIMITER, D_DOT, lexemeBuffer, tokenStartLine); 
                        break;
                    default:
                        tok = makeToken(CAT_UNKNOWN, 0, lexemeBuffer, tokenStartLine);
                        break;
                }
                
                printToken(symbolFileAppend, &tok);
                currentState = S_START;
                break;

            case S_OP_ASSIGN_TAIL:
                if (c != EOF) {
                    ungetc(c, file);
                }
                lexemeBuffer[lexemeIndex] = '\0';
                tok = makeToken(CAT_OPERATOR, O_EQUAL, lexemeBuffer, tokenStartLine);
                printToken(symbolFileAppend, &tok);
                currentState = S_START; // Reset
                break;
            case S_OP_NOT_TAIL: //prev input is = 
                if (c != EOF) {
                    ungetc(c, file);
                }
                lexemeBuffer[lexemeIndex] = '\0';
                tok = makeToken(CAT_OPERATOR, O_NOT_EQUAL, lexemeBuffer, tokenStartLine);
                printToken(symbolFileAppend, &tok);
                currentState = S_START;
                break;
            case S_OP_LESS_TAIL:
                if (c != EOF) {
                    ungetc(c, file);
                }
                lexemeBuffer[lexemeIndex] = '\0';
                tok = makeToken(CAT_OPERATOR, O_LESS_EQ, lexemeBuffer, tokenStartLine);
                printToken(symbolFileAppend, &tok);
                currentState = S_START; // Reset
                break;
            case S_OP_GREATER_TAIL: //prev input is = 
             if (c != EOF) {
                    ungetc(c, file);
                }
                lexemeBuffer[lexemeIndex] = '\0';
                tok = makeToken(CAT_OPERATOR, O_GREATER_EQ, lexemeBuffer, tokenStartLine);
                printToken(symbolFileAppend, &tok);
                currentState = S_START; // Reset
                break;
            case S_OP_AND_TAIL: //prev input is &
                if (c != EOF) {
                    ungetc(c, file);
                }
                lexemeBuffer[lexemeIndex] = '\0';
                tok = makeToken(CAT_OPERATOR, O_AND, lexemeBuffer, tokenStartLine);
                printToken(symbolFileAppend, &tok);
                currentState = S_START; 
                break;
            case S_OP_OR_TAIL://prev input is | 
                if (c != EOF) {
                    ungetc(c, file);
                }
                lexemeBuffer[lexemeIndex] = '\0';
                tok = makeToken(CAT_OPERATOR, O_OR, lexemeBuffer, tokenStartLine);
                printToken(symbolFileAppend, &tok);
                currentState = S_START; 
                break;
            case S_DONE:
                fprintf(stderr, "Lexer Error: Entered unreachable state %d on line %d.\n", currentState, lineNumber);
                currentState = S_START;
                break;
            
                
            /*
            case S_KEYWORD:
            case S_RESERVE:
            case S_NOISE:
            */

        } //end switch(currentState)
    } //end while
}//end lexer

//fn extension checker
int checkExtension(const char *filename) {
    const char *dot = strrchr(filename, '.');  //find last dot in filename
    if (!dot) return 0;                        // if no dot found = no extension
    if (strcmp(dot, ".usb") == 0) {  //if it matches .usb
        return 1;
    } else return 0;  //file is not .usb file
}

//tokenValue to String
static const char *token_value_name(const Token *t) {
    if (!t) return "(null)";
    switch (t->category) {
        case CAT_DELIMITER:
            switch (t->tokenValue) {
                case D_LPAREN: return "D_LPAREN";
                case D_RPAREN: return "D_RPAREN";
                case D_LBRACE: return "D_LBRACE";
                case D_RBRACE: return "D_RBRACE";
                case D_LBRACKET: return "D_LBRACKET";
                case D_RBRACKET: return "D_RBRACKET";
                case D_COMMA: return "D_COMMA";
                case D_SEMICOLON: return "D_SEMICOLON";
                case D_COLON: return "D_COLON";
                case D_DOT: return "D_DOT";
                case D_QUOTE: return "D_QUOTE";
                case D_SQUOTE: return "D_SQUOTE";
                default: return "D_UNKNOWN";
            }

        case CAT_OPERATOR:
            switch (t->tokenValue) {
                case O_PLUS: return "O_PLUS";
                case O_MINUS: return "O_MINUS";
                case O_MULTIPLY: return "O_MULTIPLY";
                case O_DIVIDE: return "O_DIVIDE";
                case O_POW: return "O_POW";
                case O_MODULO: return "O_MODULO";
                case O_ASSIGN: return "O_ASSIGN";
                case O_EQUAL: return "O_EQUAL";
                case O_NOT_EQUAL: return "O_NOT_EQUAL";
                case O_LESS: return "O_LESS";
                case O_GREATER: return "O_GREATER";
                case O_LESS_EQ: return "O_LESS_EQ";
                case O_GREATER_EQ: return "O_GREATER_EQ";
                case O_AND: return "O_AND";
                case O_OR: return "O_OR";
                case O_NOT: return "O_NOT";
                default: return "O_UNKNOWN";
            }

        case CAT_LITERAL:
            switch (t->tokenValue) {
                case L_IDENTIFIER: return "L_IDENTIFIER";
                case L_BILANG_LITERAL: return "L_BILANG_LITERAL";
                case L_LUTANG_LITERAL: return "L_LUTANG_LITERAL";
                case L_KWERDAS_LITERAL: return "L_KWERDAS_LITERAL";
                case L_TITIK_LITERAL: return "L_TITIK_LITERAL";
                case L_BULYAN_LITERAL: return "L_BULYAN_LITERAL";
                default: return "L_UNKNOWN";
            }

         case CAT_KEYWORD:
            switch (t->tokenValue) {
                case K_ANI: return "K_ANI";
                case K_TANIM: return "K_TANIM";
                case K_PARA: return "K_PARA";
                case K_HABANG: return "K_HABANG";
                case K_KUNG: return "K_KUNG";
                case K_KUNDI: return "K_KUNDI";
                case K_KUNDIMAN: return "K_KUNDIMAN";
                case K_GAWIN: return "K_GAWIN";
                case K_TIBAG: return "K_TIBAG";
                case K_TULOY: return "K_TULOY";
                case K_PANGKAT: return "K_PANGKAT";
                case K_STATIK: return "K_STATIK";
                case K_PRIBADO: return "K_PRIBADO";
                case K_PROTEKTADO: return "K_PROTEKTADO";
                case K_PUBLIKO: return "K_PUBLIKO";
                default: return "K_UNKNOWN";
            };

        case CAT_RESERVED:
            switch (t->tokenValue) {
                case R_TAMA: return "R_TAMA";
                case R_MALI: return "R_MALI";
                case R_UGAT: return "R_UGAT";
                case R_BALIK: return "R_BALIK";
                case R_BILANG: return "R_BILANG";
                case R_KWERDAS: return "R_KWERDAS";
                case R_TITIK: return "R_TITIK";
                case R_LUTANG: return "R_LUTANG";
                case R_BULYAN: return "R_BULYAN";
                case R_DOBLE: return "R_DOBLE";
                case R_WALA: return "R_WALA";
                case R_PI: return "R_C_PI";
                case R_E_NUM: return "R_C_E";
                case R_Kiss: return "R_C_KISS";
                case R_SAMPLE_CONST_STRING: return "R_C_SAMPLE_CONST_STRING";
                default: return "R_UNKNOWN";
            };

        case CAT_NOISEWORD:
            switch (t->tokenValue) {
                case N_NG: return "N_NG";
                case N_AY: return "N_AY";
                case N_BUNGA: return "N_BUNGA";
                case N_WAKAS: return "N_WAKAS";
                case N_SA: return "N_SA";
                case N_ANG: return "N_ANG";
                case N_MULA: return "N_MULA";
                case N_ITAKDA: return "N_ITAKDA";
                default: return "N_UNKNOWN";
            };
         case CAT_COMMENT:
            switch (t->tokenValue) {
                case C_SINGLE_LINE: return "C_SINGLE_LINE";
                case C_MULTI_LINE: return "C_MULTI_LINE";
                default: return "C_UNKNOWN";
            }
        default: 
            return "UNKNOWN_CATEGORY";
    }
}

//Print token as in this format:
// Lexeme | Token | LineNumber
void printToken(FILE *file, Token *t) {
    const char *lex;
    lex = t->lexeme;
    const char *name = token_value_name(t);
    fprintf(file, "%-15s | %-20s | %d \n", lex, name, t -> lineNumber);
}

//create a token
Token makeToken(TokenCategory cat, int tokenValue, const char *lexeme, int lineNumber) {
    Token t;
    t.category = cat;
    t.tokenValue = tokenValue;
    t.lexeme = malloc(strlen(lexeme) + 1);
    strcpy(t.lexeme, lexeme);
    t.lineNumber = lineNumber;
    return t;
}
