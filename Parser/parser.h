#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_TOKEN_LENGTH 256
#define MAX_CHILDREN 20
#define MAX_ERRORS 100
#define MAX_TOKENS 1000

// Token structure
typedef struct {
    char lexeme[MAX_TOKEN_LENGTH];
    char type[MAX_TOKEN_LENGTH];
    int line;
} Token;

// Parse Tree Node structure
typedef struct ParseTreeNode {
    char name[MAX_TOKEN_LENGTH];
    char value[MAX_TOKEN_LENGTH];
    struct ParseTreeNode* children[MAX_CHILDREN];
    int child_count;
} ParseTreeNode;

// Parser structure
typedef struct {
    Token* tokens;
    int token_count;
    int pos;
    Token* current_token;
    char errors[MAX_ERRORS][512];
    int error_count;
    ParseTreeNode* parse_tree;
} Parser;

// Function declarations
Parser* create_parser(Token* tokens, int count);
void free_parser(Parser* parser);
bool parse_program(Parser* p);
Token* read_symbol_table(const char* filename, int* count);
void write_parse_tree_to_file(const char* filename, ParseTreeNode* tree, bool is_visual);

// Parse tree node functions
ParseTreeNode* create_node(const char* name, const char* value);
void add_child(ParseTreeNode* parent, ParseTreeNode* child);
void free_tree(ParseTreeNode* node);

// Transition tracking functions
void init_transition_tracking();
void enter_nonterminal(const char* nonterminal, const char* lookahead);
void exit_nonterminal(const char* nonterminal, const char* lookahead);
void match_terminal(const char* terminal, const char* value);
void apply_production(const char* production_rule);
void write_transition_table(const char* filename);
void write_transition_diagram(const char* filename);

// Transition tracking functions
void init_transition_tracking();
void enter_nonterminal(const char* nonterminal, const char* lookahead);
void exit_nonterminal(const char* nonterminal, const char* lookahead);
void match_terminal(const char* terminal, const char* value);
void apply_production(const char* production_rule);
void write_transition_table(const char* filename);
void write_transition_diagram(const char* filename);
void write_transition_summary(const char* filename);

extern int transition_count;

#endif