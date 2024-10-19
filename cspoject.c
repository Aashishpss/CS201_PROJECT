#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define Total_Alphabets 26

// Structure of TrieNode
typedef struct TrieNode {
    struct TrieNode *children[Total_Alphabets];
    int weight;
    bool checkisEndOfWord;
} TrieNode;

// Create a new TrieNode
TrieNode *create_node() {
    TrieNode *new_node = (TrieNode *)malloc(sizeof(TrieNode));
    new_node->checkisEndOfWord = false;
    new_node->weight = 0;
    for (int i = 0; i < Total_Alphabets; i++) {
        new_node->children[i] = NULL;
    }
    return new_node;
}

// Insert a cleaned key in Trie
void insert(TrieNode *root, const char *key) {
    TrieNode *temp = root;
    for (int i = 0; i < strlen(key); i++) {
        int index = key[i] - 'a';
        if (!temp->children[index]) {
            temp->children[index] = create_node();
        }
        temp = temp->children[index];
    }
    temp->checkisEndOfWord = true;
    temp->weight++;
}

// Search for a key in Trie and return its weight
int search(TrieNode *root, const char *key) {
    TrieNode *temp = root;
    for (int i = 0; i < strlen(key); i++) {
        int index = key[i] - 'a';
        if (!temp->children[index]) {
            return 0;
        }
        temp = temp->children[index];
    }
    return (temp != NULL && temp->checkisEndOfWord) ? temp->weight : 0;
}

// Function to clean and insert word from a file character by character
void insert_from_file(TrieNode *root, FILE *f) {
    char word[100];  
    int j = 0;
    char ch;

    while ((ch = fgetc(f)) != EOF) {
        if (isalpha(ch)) {  
            word[j++] = tolower(ch);
        } else {
            if (j > 0) {  
                word[j] = '\0'; 
                insert(root, word); 
                j = 0;  
            }
        }
    }

    // Handle the last word in the file if it exists
    if (j > 0) {
        word[j] = '\0';
        insert(root, word);
    }
}

int main() {
    TrieNode *root = create_node();
    FILE *f = fopen("corpus_sample.txt", "r");

    if (!f) {
        printf("Error opening file\n");
        return 1;
    }

   
    insert_from_file(root, f);
    fclose(f);

  
    char search_word[100];
    printf("Enter the word to search in the Trie: ");
    scanf("%s", search_word);

   
    char cleaned_search_word[100];
    int j = 0;
    for (int i = 0; search_word[i] != '\0'; i++) {
        if (isalpha(search_word[i])) {
            cleaned_search_word[j++] = tolower(search_word[i]);
        }
    }
    cleaned_search_word[j] = '\0'; 

    int weight = search(root, cleaned_search_word);
    if (weight > 0) {
        printf("The word '%s' is present in the Trie and its weight is %d\n", cleaned_search_word, weight);
    } else {
        printf("The word '%s' is not present in the Trie\n", cleaned_search_word);
    }

    return 0;
}
