#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokens.h"
#include <stdbool.h>
#include "wordhash.h"

HashEntry hash_table[TABLE_SIZE]; //declare hash table

//hash function
unsigned int hash(const char *key) {
    unsigned int hash_val = 5381;
    int c;
       while ((c = *key++)) {
        hash_val = hash_val * 33 + c;
    }
    return hash_val % TABLE_SIZE;
}

// hashInsert key/moise/reserve word into hash table
void hashInsert(const char *key, TokenCategory category, int token_value) {
    unsigned int index = hash(key);
    unsigned int start = index;

    do {
        if (hash_table[index].key[0] == '\0') {
            strcpy(hash_table[index].key, key);
            hash_table[index].category = category;
            hash_table[index].tokenValue = token_value;
            return;
        }
        index = (index + 1) % TABLE_SIZE;
    } while (index != start);

    fprintf(stderr, "Error: hash table full\n");
    exit(1);
}

// Lookup a key in the hash table
HashEntry *hashLookUp(const char *key) {
    unsigned int index = hash(key);
    unsigned int start = index;

    do {
        if (hash_table[index].key[0] == '\0') {
            return NULL;
        }
        if (strcmp(hash_table[index].key, key) == 0) {
            return &hash_table[index];
        }
        index = (index + 1) % TABLE_SIZE;
    } while (index != start);

    return NULL;
}

void initialize_table(void) {
    // Keywords
    hashInsert("ani", CAT_KEYWORD, K_ANI);
    hashInsert("tanim", CAT_KEYWORD, K_TANIM);
    hashInsert("para", CAT_KEYWORD, K_PARA);
    hashInsert("habang", CAT_KEYWORD, K_HABANG);
    hashInsert("kung", CAT_KEYWORD, K_KUNG);
    hashInsert("kundi", CAT_KEYWORD, K_KUNDI);
    hashInsert("kundiman", CAT_KEYWORD, K_KUNDIMAN);
    hashInsert("gawin", CAT_KEYWORD, K_GAWIN);
    hashInsert("tibag", CAT_KEYWORD, K_TIBAG);
    hashInsert("tuloy", CAT_KEYWORD, K_TULOY);
    hashInsert("pangkat", CAT_KEYWORD, K_PANGKAT);
    hashInsert("statik", CAT_KEYWORD, K_STATIK);
    hashInsert("pribado", CAT_KEYWORD, K_PRIBADO);
    hashInsert("protektado", CAT_KEYWORD, K_PROTEKTADO);
    hashInsert("publiko", CAT_KEYWORD, K_PUBLIKO);

    // Reserved words
    hashInsert("tama", CAT_RESERVED, R_TAMA);
    hashInsert("mali", CAT_RESERVED, R_MALI);
    hashInsert("ugat", CAT_RESERVED, R_UGAT);
    hashInsert("balik", CAT_RESERVED, R_BALIK);
    hashInsert("bilang", CAT_RESERVED, R_BILANG);
    hashInsert("kwerdas", CAT_RESERVED, R_KWERDAS);
    hashInsert("titik", CAT_RESERVED, R_TITIK);
    hashInsert("lutang", CAT_RESERVED, R_LUTANG);
    hashInsert("bulyan", CAT_RESERVED, R_BULYAN);
    hashInsert("doble", CAT_RESERVED, R_DOBLE);
    hashInsert("wala", CAT_RESERVED, R_WALA);

    // Noise words
    hashInsert("ng", CAT_NOISEWORD, N_NG);
    hashInsert("ay", CAT_NOISEWORD, N_AY);
    hashInsert("bunga", CAT_NOISEWORD, N_BUNGA);
    hashInsert("wakas", CAT_NOISEWORD, N_WAKAS);
    hashInsert("sa", CAT_NOISEWORD, N_SA);
    hashInsert("ang", CAT_NOISEWORD, N_ANG);
    hashInsert("mula", CAT_NOISEWORD, N_MULA);
    hashInsert("itakda", CAT_NOISEWORD, N_ITAKDA);
    //Const
    hashInsert("pi", CAT_RESERVED, R_PI);
    hashInsert("E_num", CAT_RESERVED, R_E_NUM);
    hashInsert("kiss", CAT_RESERVED, R_Kiss);
    hashInsert("sampleConstString", CAT_RESERVED, R_SAMPLE_CONST_STRING);
    
}

// Added for parser
int hashLookup(const char *lexeme, int *category, int *value) {
    HashEntry *entry = hashLookUp(lexeme);
    
    if (entry != NULL) {
        *category = entry->category;
        *value = entry->tokenValue;
        return 1;  // Found
    }
    return 0;  // Not found
}