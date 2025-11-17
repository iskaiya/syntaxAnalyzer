#include "parser.h"

Token* peek(Parser* p);
void advance(Parser* p);
bool check_token(Parser* p, const char* type);

// ============ UTILITY FUNCTIONS ============

// Trim whitespace from string
void trim(char* str) {
    char* start = str;
    char* end;
    
    // Trim leading space
    while(isspace((unsigned char)*start)) start++;
    
    if(*start == 0) {
        str[0] = '\0';
        return;
    }
    
    // Trim trailing space
    end = start + strlen(start) - 1;
    while(end > start && isspace((unsigned char)*end)) end--;
    
    // Write new null terminator
    size_t len = (end - start) + 1;
    memmove(str, start, len);
    str[len] = '\0';
}

// Create a new parse tree node
ParseTreeNode* create_node(const char* name, const char* value) {
    ParseTreeNode* node = (ParseTreeNode*)malloc(sizeof(ParseTreeNode));
    strcpy(node->name, name);
    if (value) {
        strcpy(node->value, value);
    } else {
        node->value[0] = '\0';
    }
    node->child_count = 0;
    return node;
}

// Add child to parse tree node
void add_child(ParseTreeNode* parent, ParseTreeNode* child) {
    if (child != NULL && parent->child_count < MAX_CHILDREN) {
        parent->children[parent->child_count++] = child;
    }
}

// Free parse tree memory
void free_tree(ParseTreeNode* node) {
    if (node == NULL) return;
    for (int i = 0; i < node->child_count; i++) {
        free_tree(node->children[i]);
    }
    free(node);
}

// ============ PARSER CORE FUNCTIONS ============

// Create parser
Parser* create_parser(Token* tokens, int count) {
    Parser* p = (Parser*)malloc(sizeof(Parser));
    p->tokens = tokens;
    p->token_count = count;
    p->pos = 0;
    p->current_token = (count > 0) ? &tokens[0] : NULL;
    p->error_count = 0;
    p->parse_tree = NULL;
    return p;
}

// Free parser memory
void free_parser(Parser* p) {
    if (p->parse_tree) {
        free_tree(p->parse_tree);
    }
    if (p->tokens) {
        free(p->tokens);
    }
    free(p);
}

// Record error
void parser_error(Parser* p, const char* message) {
    if (p->error_count < MAX_ERRORS) {
        if (p->current_token) {
            sprintf(p->errors[p->error_count], 
                    "Line %d: %s (Found: %s '%s')",
                    p->current_token->line, message,
                    p->current_token->type, p->current_token->lexeme);
        } else {
            sprintf(p->errors[p->error_count], "End of file: %s", message);
        }
        printf("ERROR: %s\n", p->errors[p->error_count]);
        p->error_count++;
    }
}

// ERROR RECOVERY: Skip tokens until we find a synchronizing token
void synchronize(Parser* p, const char* sync_tokens[], int sync_count) {
    printf("    [ERROR RECOVERY] Synchronizing...\n");
    
    while (peek(p)) {
        // Check if current token is a sync token
        for (int i = 0; i < sync_count; i++) {
            if (check_token(p, sync_tokens[i])) {
                printf("    [ERROR RECOVERY] Synchronized at %s\n", sync_tokens[i]);
                return;
            }
        }
        // Also stop at statement terminators
        if (check_token(p, "D_SEMICOLON") || check_token(p, "D_RBRACE") ||
            check_token(p, "D_LBRACE")) {
            printf("    [ERROR RECOVERY] Synchronized at delimiter\n");
            return;
        }
        advance(p);
    }
    printf("    [ERROR RECOVERY] Reached end of tokens\n");
}

// ERROR RECOVERY: Skip to end of statement (semicolon or closing brace)
void skip_to_statement_end(Parser* p) {
    printf("    [ERROR RECOVERY] Skipping to statement end...\n");
    while (peek(p) && !check_token(p, "D_SEMICOLON") && 
           !check_token(p, "D_RBRACE")) {
        advance(p);
    }
    if (peek(p) && check_token(p, "D_SEMICOLON")) {
        advance(p); // consume the semicolon
    }
}

// ERROR RECOVERY: Skip to matching closing brace
void skip_to_closing_brace(Parser* p) {
    printf("    [ERROR RECOVERY] Skipping to closing brace...\n");
    int brace_count = 1;
    while (peek(p) && brace_count > 0) {
        if (check_token(p, "D_LBRACE")) {
            brace_count++;
        } else if (check_token(p, "D_RBRACE")) {
            brace_count--;
        }
        advance(p);
    }
}

// Advance to next token (PDA: POP operation)
void advance(Parser* p) {
    p->pos++;
    if (p->pos < p->token_count) {
        p->current_token = &p->tokens[p->pos];
    } else {
        p->current_token = NULL;
    }
}

// Match token type with ERROR RECOVERY
ParseTreeNode* match(Parser* p, const char* expected_type) {
    if (p->current_token && strcmp(p->current_token->type, expected_type) == 0) {
        ParseTreeNode* node = create_node(expected_type, p->current_token->lexeme);
        advance(p);
        return node;
    } else {
        char msg[256];
        sprintf(msg, "Expected %s", expected_type);
        parser_error(p, msg);
        
        // ERROR RECOVERY: Create error node and try to continue
        ParseTreeNode* error_node = create_node("ERROR", expected_type);
        
        // Don't advance if we're already at a delimiter that might help recovery
        if (peek(p) && !check_token(p, "D_SEMICOLON") && 
            !check_token(p, "D_RBRACE") && !check_token(p, "D_LBRACE")) {
            advance(p); // Skip the unexpected token
        }
        
        return error_node;
    }
}

// Peek at current token
Token* peek(Parser* p) {
    return p->current_token;
}

// Check if current token matches type
bool check_token(Parser* p, const char* type) {
    return (p->current_token && strcmp(p->current_token->type, type) == 0);
}

// Forward declarations of all parsing functions (PDA states)
ParseTreeNode* parse_main_function(Parser* p);
ParseTreeNode* parse_return_type(Parser* p);
ParseTreeNode* parse_parameter_list(Parser* p);
ParseTreeNode* parse_function_body(Parser* p);
ParseTreeNode* parse_statement_list(Parser* p);
ParseTreeNode* parse_statement(Parser* p);
ParseTreeNode* parse_declaration(Parser* p);
ParseTreeNode* parse_data_type(Parser* p);
ParseTreeNode* parse_identifier_list(Parser* p);
ParseTreeNode* parse_identifier_tail(Parser* p);
ParseTreeNode* parse_assignment(Parser* p);
ParseTreeNode* parse_expression(Parser* p);
ParseTreeNode* parse_expression_tail(Parser* p);
ParseTreeNode* parse_term(Parser* p);
ParseTreeNode* parse_term_tail(Parser* p);
ParseTreeNode* parse_factor(Parser* p);
ParseTreeNode* parse_conditional(Parser* p);
ParseTreeNode* parse_conditional_tail(Parser* p);
ParseTreeNode* parse_boolean_expression(Parser* p);
ParseTreeNode* parse_relop(Parser* p);
ParseTreeNode* parse_iterative(Parser* p);
ParseTreeNode* parse_for_loop(Parser* p);
ParseTreeNode* parse_while_loop(Parser* p);
ParseTreeNode* parse_do_while_loop(Parser* p);
ParseTreeNode* parse_print(Parser* p);
ParseTreeNode* parse_print_args(Parser* p);
ParseTreeNode* parse_scan(Parser* p);
ParseTreeNode* parse_scan_args(Parser* p);
ParseTreeNode* parse_class_definition(Parser* p);

// ============ PROGRAM STRUCTURE ============

bool parse_program(Parser* p) {
    printf("\n=== Starting Syntax Analysis (PDA) ===\n");
    printf("Parsing Program...\n");
    ParseTreeNode* node = create_node("Program", NULL);
    
    if (peek(p) && (check_token(p, "R_BILANG") || check_token(p, "R_VOID") || 
                     check_token(p, "R_WALA"))) {
        add_child(node, parse_main_function(p));
    } else if (peek(p) && check_token(p, "R_PANGKAT")) {
        while (peek(p) && check_token(p, "R_PANGKAT")) {
            add_child(node, parse_class_definition(p));
        }
    } else {
        parser_error(p, "Expected main function or class definition");
    }
    
    p->parse_tree = node;
    printf("Program parsing complete!\n");
    return (p->error_count == 0);
}

ParseTreeNode* parse_main_function(Parser* p) {
    printf("  [PDA PUSH] Parsing Main Function...\n");
    ParseTreeNode* node = create_node("MainFunction", NULL);
    add_child(node, parse_return_type(p));
    add_child(node, match(p, "R_UGAT"));
    add_child(node, match(p, "D_LPAREN"));
    add_child(node, parse_parameter_list(p));
    add_child(node, match(p, "D_RPAREN"));
    add_child(node, parse_function_body(p));
    printf("  [PDA POP] Main Function complete\n");
    return node;
}

ParseTreeNode* parse_return_type(Parser* p) {
    ParseTreeNode* node = create_node("ReturnType", NULL);
    if (peek(p) && (check_token(p, "R_BILANG") || check_token(p, "R_VOID") || 
                     check_token(p, "R_WALA"))) {
        add_child(node, create_node(p->current_token->type, p->current_token->lexeme));
        advance(p);
    } else {
        parser_error(p, "Expected return type (R_BILANG, R_VOID, or R_WALA)");
    }
    return node;
}

ParseTreeNode* parse_parameter_list(Parser* p) {
    ParseTreeNode* node = create_node("ParameterList", NULL);
    if (peek(p) && check_token(p, "R_KWERDAS")) {
        add_child(node, match(p, "R_KWERDAS"));
        add_child(node, match(p, "D_LBRACKET"));
        add_child(node, match(p, "D_RBRACKET"));
        add_child(node, match(p, "L_IDENTIFIER"));
    } else {
        add_child(node, create_node("ε", "empty"));
    }
    return node;
}

ParseTreeNode* parse_function_body(Parser* p) {
    ParseTreeNode* node = create_node("FunctionBody", NULL);
    add_child(node, match(p, "D_LBRACE"));
    add_child(node, parse_statement_list(p));
    add_child(node, match(p, "D_RBRACE"));
    return node;
}

// ============ STATEMENTS ============

ParseTreeNode* parse_statement_list(Parser* p) {
    ParseTreeNode* node = create_node("StatementList", NULL);
    
    if (peek(p) && (check_token(p, "R_BILANG") || check_token(p, "R_LUTANG") ||
                     check_token(p, "R_BULYAN") || check_token(p, "R_KWERDAS") ||
                     check_token(p, "L_IDENTIFIER") || check_token(p, "R_KUNG") ||
                     check_token(p, "R_PARA") || check_token(p, "R_HABANG") ||
                     check_token(p, "R_GAWIN") || check_token(p, "K_ANI") ||
                     check_token(p, "K_TANIM"))) {
        add_child(node, parse_statement(p));
        add_child(node, parse_statement_list(p));
    } else {
        add_child(node, create_node("ε", "empty"));
    }
    
    return node;
}

ParseTreeNode* parse_statement(Parser* p) {
    ParseTreeNode* node = create_node("Statement", NULL);
    
    if (check_token(p, "R_BILANG") || check_token(p, "R_LUTANG") ||
        check_token(p, "R_BULYAN") || check_token(p, "R_KWERDAS")) {
        add_child(node, parse_declaration(p));
    } else if (check_token(p, "L_IDENTIFIER")) {
        add_child(node, parse_assignment(p));
    } else if (check_token(p, "R_KUNG")) {
        add_child(node, parse_conditional(p));
    } else if (check_token(p, "R_PARA") || check_token(p, "R_HABANG") || 
               check_token(p, "R_GAWIN")) {
        add_child(node, parse_iterative(p));
    } else if (check_token(p, "K_ANI")) {
        add_child(node, parse_print(p));
    } else if (check_token(p, "K_TANIM")) {
        add_child(node, parse_scan(p));
    } else {
        parser_error(p, "Unexpected statement");
        if (peek(p)) advance(p);
    }
    
    return node;
}

// ============ DECLARATION ============

ParseTreeNode* parse_declaration(Parser* p) {
    printf("    [PDA PUSH] Parsing Declaration...\n");
    ParseTreeNode* node = create_node("Declaration", NULL);
    add_child(node, parse_data_type(p));
    add_child(node, parse_identifier_list(p));
    add_child(node, match(p, "D_SEMICOLON"));
    printf("    [PDA POP] Declaration complete\n");
    return node;
}

ParseTreeNode* parse_data_type(Parser* p) {
    ParseTreeNode* node = create_node("DataType", NULL);
    if (peek(p) && (check_token(p, "R_BILANG") || check_token(p, "R_LUTANG") ||
                     check_token(p, "R_BULYAN") || check_token(p, "R_KWERDAS"))) {
        add_child(node, create_node(p->current_token->type, p->current_token->lexeme));
        advance(p);
    } else {
        parser_error(p, "Expected data type");
    }
    return node;
}

ParseTreeNode* parse_identifier_list(Parser* p) {
    ParseTreeNode* node = create_node("IdentifierList", NULL);
    add_child(node, match(p, "L_IDENTIFIER"));
    add_child(node, parse_identifier_tail(p));
    return node;
}

ParseTreeNode* parse_identifier_tail(Parser* p) {
    ParseTreeNode* node = create_node("IdentifierTail", NULL);
    if (peek(p) && check_token(p, "D_COMMA")) {
        add_child(node, match(p, "D_COMMA"));
        add_child(node, match(p, "L_IDENTIFIER"));
        add_child(node, parse_identifier_tail(p));
    } else {
        add_child(node, create_node("ε", "empty"));
    }
    return node;
}

// ============ ASSIGNMENT AND EXPRESSIONS ============

ParseTreeNode* parse_assignment(Parser* p) {
    printf("    [PDA PUSH] Parsing Assignment...\n");
    ParseTreeNode* node = create_node("Assignment", NULL);
    add_child(node, match(p, "L_IDENTIFIER"));
    add_child(node, match(p, "O_ASSIGN"));
    add_child(node, parse_expression(p));
    add_child(node, match(p, "D_SEMICOLON"));
    printf("    [PDA POP] Assignment complete\n");
    return node;
}

ParseTreeNode* parse_expression(Parser* p) {
    ParseTreeNode* node = create_node("Expression", NULL);
    add_child(node, parse_term(p));
    add_child(node, parse_expression_tail(p));
    return node;
}

ParseTreeNode* parse_expression_tail(Parser* p) {
    ParseTreeNode* node = create_node("ExpressionTail", NULL);
    if (peek(p) && (check_token(p, "O_PLUS") || check_token(p, "O_MINUS"))) {
        add_child(node, create_node(p->current_token->type, p->current_token->lexeme));
        advance(p);
        add_child(node, parse_term(p));
        add_child(node, parse_expression_tail(p));
    } else {
        add_child(node, create_node("ε", "empty"));
    }
    return node;
}

ParseTreeNode* parse_term(Parser* p) {
    ParseTreeNode* node = create_node("Term", NULL);
    add_child(node, parse_factor(p));
    add_child(node, parse_term_tail(p));
    return node;
}

ParseTreeNode* parse_term_tail(Parser* p) {
    ParseTreeNode* node = create_node("TermTail", NULL);
    if (peek(p) && (check_token(p, "O_MULTIPLY") || check_token(p, "O_DIVIDE"))) {
        add_child(node, create_node(p->current_token->type, p->current_token->lexeme));
        advance(p);
        add_child(node, parse_factor(p));
        add_child(node, parse_term_tail(p));
    } else {
        add_child(node, create_node("ε", "empty"));
    }
    return node;
}

ParseTreeNode* parse_factor(Parser* p) {
    ParseTreeNode* node = create_node("Factor", NULL);
    if (peek(p) && check_token(p, "L_IDENTIFIER")) {
        add_child(node, match(p, "L_IDENTIFIER"));
    } else if (peek(p) && check_token(p, "L_BILANG_LITERAL")) {
        add_child(node, match(p, "L_BILANG_LITERAL"));
    } else if (peek(p) && check_token(p, "D_LPAREN")) {
        add_child(node, match(p, "D_LPAREN"));
        add_child(node, parse_expression(p));
        add_child(node, match(p, "D_RPAREN"));
    } else {
        parser_error(p, "Expected identifier, literal, or parenthesized expression");
    }
    return node;
}

// ============ CONDITIONALS ============

ParseTreeNode* parse_conditional(Parser* p) {
    printf("    [PDA PUSH] Parsing Conditional...\n");
    ParseTreeNode* node = create_node("Conditional", NULL);
    add_child(node, match(p, "R_KUNG"));
    add_child(node, match(p, "D_LPAREN"));
    add_child(node, parse_boolean_expression(p));
    add_child(node, match(p, "D_RPAREN"));
    add_child(node, match(p, "D_LBRACE"));
    add_child(node, parse_statement_list(p));
    add_child(node, match(p, "D_RBRACE"));
    add_child(node, parse_conditional_tail(p));
    printf("    [PDA POP] Conditional complete\n");
    return node;
}

ParseTreeNode* parse_conditional_tail(Parser* p) {
    ParseTreeNode* node = create_node("ConditionalTail", NULL);
    if (peek(p) && check_token(p, "R_KUNDI")) {
        add_child(node, match(p, "R_KUNDI"));
        add_child(node, match(p, "D_LBRACE"));
        add_child(node, parse_statement_list(p));
        add_child(node, match(p, "D_RBRACE"));
    } else if (peek(p) && check_token(p, "R_KUNDIMAN")) {
        add_child(node, match(p, "R_KUNDIMAN"));
        add_child(node, match(p, "D_LPAREN"));
        add_child(node, parse_boolean_expression(p));
        add_child(node, match(p, "D_RPAREN"));
        add_child(node, match(p, "D_LBRACE"));
        add_child(node, parse_statement_list(p));
        add_child(node, match(p, "D_RBRACE"));
        add_child(node, parse_conditional_tail(p));
    } else {
        add_child(node, create_node("ε", "empty"));
    }
    return node;
}

ParseTreeNode* parse_boolean_expression(Parser* p) {
    ParseTreeNode* node = create_node("BooleanExpression", NULL);
    add_child(node, parse_expression(p));
    add_child(node, parse_relop(p));
    add_child(node, parse_expression(p));
    return node;
}

ParseTreeNode* parse_relop(Parser* p) {
    ParseTreeNode* node = create_node("RelOp", NULL);
    if (peek(p) && (check_token(p, "O_EQUAL") || check_token(p, "O_NOT_EQUAL") ||
                     check_token(p, "O_GREATER") || check_token(p, "O_LESS") ||
                     check_token(p, "O_GREATER_EQUAL") || check_token(p, "O_LESS_EQUAL"))) {
        add_child(node, create_node(p->current_token->type, p->current_token->lexeme));
        advance(p);
    } else {
        parser_error(p, "Expected relational operator");
    }
    return node;
}

// ============ ITERATIONS/LOOPS ============

ParseTreeNode* parse_iterative(Parser* p) {
    ParseTreeNode* node = create_node("Iterative", NULL);
    if (check_token(p, "R_PARA")) {
        add_child(node, parse_for_loop(p));
    } else if (check_token(p, "R_HABANG")) {
        add_child(node, parse_while_loop(p));
    } else if (check_token(p, "R_GAWIN")) {
        add_child(node, parse_do_while_loop(p));
    }
    return node;
}

ParseTreeNode* parse_for_loop(Parser* p) {
    printf("    [PDA PUSH] Parsing For Loop...\n");
    ParseTreeNode* node = create_node("ForLoop", NULL);
    add_child(node, match(p, "R_PARA"));
    add_child(node, match(p, "D_LPAREN"));
    
    if (check_token(p, "R_BILANG") || check_token(p, "R_LUTANG") ||
        check_token(p, "R_BULYAN") || check_token(p, "R_KWERDAS")) {
        add_child(node, parse_declaration(p));
    } else {
        ParseTreeNode* assign = create_node("Assignment", NULL);
        add_child(assign, match(p, "L_IDENTIFIER"));
        add_child(assign, match(p, "O_ASSIGN"));
        add_child(assign, parse_expression(p));
        add_child(assign, match(p, "D_SEMICOLON"));
        add_child(node, assign);
    }
    
    add_child(node, parse_boolean_expression(p));
    add_child(node, match(p, "D_SEMICOLON"));
    
    ParseTreeNode* incr = create_node("Assignment", NULL);
    add_child(incr, match(p, "L_IDENTIFIER"));
    add_child(incr, match(p, "O_ASSIGN"));
    add_child(incr, parse_expression(p));
    add_child(node, incr);
    
    add_child(node, match(p, "D_RPAREN"));
    add_child(node, match(p, "D_LBRACE"));
    add_child(node, parse_statement_list(p));
    add_child(node, match(p, "D_RBRACE"));
    printf("    [PDA POP] For Loop complete\n");
    return node;
}

ParseTreeNode* parse_while_loop(Parser* p) {
    printf("    [PDA PUSH] Parsing While Loop...\n");
    ParseTreeNode* node = create_node("WhileLoop", NULL);
    add_child(node, match(p, "R_HABANG"));
    add_child(node, match(p, "D_LPAREN"));
    add_child(node, parse_boolean_expression(p));
    add_child(node, match(p, "D_RPAREN"));
    add_child(node, match(p, "D_LBRACE"));
    add_child(node, parse_statement_list(p));
    add_child(node, match(p, "D_RBRACE"));
    printf("    [PDA POP] While Loop complete\n");
    return node;
}

ParseTreeNode* parse_do_while_loop(Parser* p) {
    printf("    [PDA PUSH] Parsing Do-While Loop...\n");
    ParseTreeNode* node = create_node("DoWhileLoop", NULL);
    add_child(node, match(p, "R_GAWIN"));
    add_child(node, match(p, "D_LBRACE"));
    add_child(node, parse_statement_list(p));
    add_child(node, match(p, "D_RBRACE"));
    add_child(node, match(p, "R_HABANG"));
    add_child(node, match(p, "D_LPAREN"));
    add_child(node, parse_boolean_expression(p));
    add_child(node, match(p, "D_RPAREN"));
    add_child(node, match(p, "D_SEMICOLON"));
    printf("    [PDA POP] Do-While Loop complete\n");
    return node;
}

// ============ INPUT/OUTPUT ============

ParseTreeNode* parse_print(Parser* p) {
    printf("    [PDA PUSH] Parsing Print...\n");
    ParseTreeNode* node = create_node("Print", NULL);
    add_child(node, match(p, "K_ANI"));
    add_child(node, match(p, "D_LPAREN"));
    add_child(node, parse_print_args(p));
    add_child(node, match(p, "D_RPAREN"));
    add_child(node, match(p, "D_SEMICOLON"));
    printf("    [PDA POP] Print complete\n");
    return node;
}

ParseTreeNode* parse_print_args(Parser* p) {
    ParseTreeNode* node = create_node("PrintArgs", NULL);
    add_child(node, parse_expression(p));
    if (peek(p) && check_token(p, "D_COMMA")) {
        add_child(node, match(p, "D_COMMA"));
        add_child(node, parse_print_args(p));
    }
    return node;
}

ParseTreeNode* parse_scan(Parser* p) {
    printf("    [PDA PUSH] Parsing Scan...\n");
    ParseTreeNode* node = create_node("Scan", NULL);
    add_child(node, match(p, "K_TANIM"));
    add_child(node, match(p, "D_LPAREN"));
    add_child(node, parse_scan_args(p));
    add_child(node, match(p, "D_RPAREN"));
    add_child(node, match(p, "D_SEMICOLON"));
    printf("    [PDA POP] Scan complete\n");
    return node;
}

ParseTreeNode* parse_scan_args(Parser* p) {
    ParseTreeNode* node = create_node("ScanArgs", NULL);
    add_child(node, match(p, "L_IDENTIFIER"));
    if (peek(p) && check_token(p, "D_COMMA")) {
        add_child(node, match(p, "D_COMMA"));
        add_child(node, parse_scan_args(p));
    }
    return node;
}

// ============ CLASS DEFINITION ============

ParseTreeNode* parse_class_definition(Parser* p) {
    printf("  [PDA PUSH] Parsing Class Definition...\n");
    ParseTreeNode* node = create_node("ClassDefinition", NULL);
    add_child(node, match(p, "R_PANGKAT"));
    add_child(node, match(p, "L_IDENTIFIER"));
    add_child(node, match(p, "D_LBRACE"));
    add_child(node, match(p, "D_RBRACE"));
    printf("  [PDA POP] Class Definition complete\n");
    return node;
}

// ============ FILE I/O ============

Token* read_symbol_table(const char* filename, int* count) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        printf("\nERROR: Cannot open file '%s'\n", filename);
        printf("Make sure 'Symbol Table.txt' exists in the same folder!\n");
        *count = 0;
        return NULL;
    }
    
    Token* tokens = (Token*)malloc(MAX_TOKENS * sizeof(Token));
    *count = 0;
    char line[512];
    
    printf("\nReading symbol table from '%s'...\n", filename);
    
    // Skip header line
    if (fgets(line, sizeof(line), fp)) {
        printf("Header: %s", line);
    }
    
    // Read tokens
    while (fgets(line, sizeof(line), fp) && *count < MAX_TOKENS) {
        // Parse format: lexeme | token | line
        char lexeme[MAX_TOKEN_LENGTH];
        char token_type[MAX_TOKEN_LENGTH];
        int line_num;
        
        char* lex_ptr = strtok(line, "|");
        char* token_ptr = strtok(NULL, "|");
        char* line_ptr = strtok(NULL, "|");
        
        if (lex_ptr && token_ptr && line_ptr) {
            strcpy(lexeme, lex_ptr);
            strcpy(token_type, token_ptr);
            line_num = atoi(line_ptr);
            
            // Trim whitespace
            trim(lexeme);
            trim(token_type);
            
            strcpy(tokens[*count].lexeme, lexeme);
            strcpy(tokens[*count].type, token_type);
            tokens[*count].line = line_num;
            (*count)++;
        }
    }
    
    fclose(fp);
    printf("Successfully read %d tokens from symbol table.\n\n", *count);
    return tokens;
}

void print_parse_tree_visual_helper(ParseTreeNode* node, FILE* fp, char* prefix, bool is_last) {
    if (!node) return;
    
    fprintf(fp, "%s", prefix);
    fprintf(fp, "%s", is_last ? "└── " : "├── ");
    fprintf(fp, "%s", node->name);
    if (strlen(node->value) > 0 && strcmp(node->value, "empty") != 0) {
        fprintf(fp, " [%s]", node->value);
    }
    fprintf(fp, "\n");
    
    char new_prefix[512];
    sprintf(new_prefix, "%s%s", prefix, is_last ? "    " : "│   ");
    
    for (int i = 0; i < node->child_count; i++) {
        print_parse_tree_visual_helper(node->children[i], fp, new_prefix, 
                                       i == node->child_count - 1);
    }
}

void print_parse_tree_parenthesized_helper(ParseTreeNode* node, FILE* fp, int indent) {
    if (!node) return;
    
    // Print indentation
    for (int j = 0; j < indent; j++) fprintf(fp, "  ");
    
    if (node->child_count == 0) {
        fprintf(fp, "%s", node->name);
        if (strlen(node->value) > 0 && strcmp(node->value, "empty") != 0) {
            fprintf(fp, "(%s)", node->value);
        }
        return;
    }
    
    fprintf(fp, "%s(\n", node->name);
    for (int i = 0; i < node->child_count; i++) {
        print_parse_tree_parenthesized_helper(node->children[i], fp, indent + 1);
        if (i < node->child_count - 1) {
            fprintf(fp, ",");
        }
        fprintf(fp, "\n");
    }
    
    // Closing parenthesis
    for (int j = 0; j < indent; j++) fprintf(fp, "  ");
    fprintf(fp, ")");
}

void write_parse_tree_to_file(const char* filename, ParseTreeNode* tree, bool is_visual) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        printf("ERROR: Cannot create output file '%s'\n", filename);
        return;
    }
    
    fprintf(fp, "======================================================================\n");
    fprintf(fp, "PARSE TREE (%s FORMAT)\n", is_visual ? "VISUAL" : "PARENTHESIZED");
    fprintf(fp, "Generated by Recursive Descent Parser (Pushdown Automaton)\n");
    fprintf(fp, "======================================================================\n\n");
    
    if (is_visual) {
        char prefix[512] = "";
        print_parse_tree_visual_helper(tree, fp, prefix, true);
    } else {
        print_parse_tree_parenthesized_helper(tree, fp, 0);
        fprintf(fp, "\n");
    }
    
    fprintf(fp, "\n======================================================================\n");
    fprintf(fp, "End of Parse Tree\n");
    fprintf(fp, "======================================================================\n");
    
    fclose(fp);
    printf("Parse tree written to '%s'\n", filename);
}