#ifndef LEXER_H
#define LEXER_H

// Expose the hash table functions
void initialize_table(void);
int hashLookup(const char *lexeme, int *category, int *value);

#endif