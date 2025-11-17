#include "parser.h"

int main() {
    printf("Syntax Analyzer for Usbong\n");
    
    // Read symbol table from lexer output
    int token_count = 0;
    Token* tokens = read_symbol_table("Symbol Table.txt", &token_count);
    
    if (tokens == NULL || token_count == 0) {
        printf("\nFailed to read symbol table!\n");
        printf("Please make sure:\n");
        printf("1. Run your lexer first to generate 'Symbol Table.txt'\n");
        printf("2. The file is in the same folder as parser.exe\n");
        printf("3. The format is: lexeme | token | line\n\n");
        return 1;
    }
    
    // Display tokens being parsed
    printf("Tokens to parse:\n");
    printf("----------------\n");
    for (int i = 0; i < token_count && i < 20; i++) {
        printf("%2d. %-20s %-25s Line %d\n", 
               i+1, tokens[i].lexeme, tokens[i].type, tokens[i].line);
    }
    if (token_count > 20) {
        printf("... and %d more tokens\n", token_count - 20);
    }
    printf("\n");
    
    // Create parser
    Parser* parser = create_parser(tokens, token_count);
    
    // Parse the program
    bool success = parse_program(parser);
    
    if (success) {
        printf("✓ PARSING SUCCESSFUL! No syntax errors found. ✓✓✓\n");
        
        // Generate parse trees
        printf("Generating parse trees...\n\n");
        
        // Visual tree
        write_parse_tree_to_file("parse_tree_visual.txt", parser->parse_tree, true);
        
        // Parenthesized notation
        write_parse_tree_to_file("parse_tree_parenthesized.txt", parser->parse_tree, false);
        
        printf("\nOutput files created:\n");
        printf("  1. parse_tree_visual.txt         - Tree diagram format\n");
        printf("  2. parse_tree_parenthesized.txt  - Parenthesized notation\n\n");
        
        // Also print to console (abbreviated)
        printf("PARSE TREE (Visual Format - First 50 lines):\n");
        printf("--------------------------------------------\n");
        FILE* fp = fopen("parse_tree_visual.txt", "r");
        if (fp) {
            char line[512];
            int line_count = 0;
            while (fgets(line, sizeof(line), fp) && line_count++ < 54) {
                printf("%s", line);
            }
            if (!feof(fp)) {
                printf("\n... (see parse_tree_visual.txt for complete tree)\n");
            }
            fclose(fp);
        }
        
    } else {
        printf("✗ PARSING FAILED\n");
        printf("\nTotal syntax errors: %d\n\n", parser->error_count);
        printf("Error details:\n");
        printf("--------------\n");
        for (int i = 0; i < parser->error_count; i++) {
            printf("%2d. %s\n", i+1, parser->errors[i]);
        }
    }
    
    printf("PDA Operation Complete\n");
    
    // Cleanup
    free_parser(parser);
    
    return success ? 0 : 1;
}

