#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define Total_Alphabets 26
#define MAX_WORD_LENGTH 100
#define MAX_SUGGESTIONS 10

// Structure of TrieNode
typedef struct TrieNode {
    struct TrieNode *children[Total_Alphabets];
    int weight;
    bool checkisEndOfWord;
} TrieNode;

// Create a new TrieNode
TrieNode *create_node() {
    TrieNode *new_node = (TrieNode *)malloc(sizeof(TrieNode));
    if (new_node == NULL) {
        printf("Memory allocation failed!\n");
        exit(1);
    }
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

// Helper function to calculate combined weight
double getCombinedWeight(TrieNode *mainNode, TrieNode *corpusNode) {
    double mainWeight = mainNode ? mainNode->weight : 0;
    double corpusWeight = corpusNode ? corpusNode->weight : 0;
    if (mainWeight > 0 && corpusWeight > 0) {
        return (mainWeight + corpusWeight) / 2.0;  // Average if present in both
    } else if (mainWeight > 0) {
        // return twice the weight of main if corpus is absent
        return mainWeight * 2;
    } else {
        return corpusWeight;  // Use corpus weight if main is absent
    }
}

// Suggest words based on prefix and prioritize by combined weight
void suggestWords(TrieNode *corpusTrie, TrieNode *mainTrie, char *prefix, char suggestions[][MAX_WORD_LENGTH], double weights[], int *suggestionCount, int maxSuggestions) {
    // If the number of suggestions has reached the maximum limit, stop the recursion
    if (*suggestionCount >= maxSuggestions) return;

    // If this node marks the end of a word in either Trie
    if ((corpusTrie && corpusTrie->checkisEndOfWord) || (mainTrie && mainTrie->checkisEndOfWord)) {
        if (*suggestionCount < maxSuggestions) {
            double combinedWeight = getCombinedWeight(mainTrie, corpusTrie);

            // Insert the suggestion and its weight only if we haven't exceeded the max limit
            strcpy(suggestions[*suggestionCount], prefix);
            weights[*suggestionCount] = combinedWeight;
            (*suggestionCount)++;
        }
    }

    // Continue searching for suggestions in child nodes
    for (int i = 0; i < Total_Alphabets; i++) {
        if ((corpusTrie && corpusTrie->children[i]) || (mainTrie && mainTrie->children[i])) {
            char nextPrefix[MAX_WORD_LENGTH];
            sprintf(nextPrefix, "%s%c", prefix, 'a' + i);

            TrieNode *nextCorpusTrie = corpusTrie && corpusTrie->children[i] ? corpusTrie->children[i] : NULL;
            TrieNode *nextMainTrie = mainTrie && mainTrie->children[i] ? mainTrie->children[i] : NULL;

            // Recursively call for child nodes
            suggestWords(nextCorpusTrie, nextMainTrie, nextPrefix, suggestions, weights, suggestionCount, maxSuggestions);
        }
    }
}

// Find the node in the trie corresponding to a given prefix
TrieNode* findPrefixNode(TrieNode *root, const char *prefix) {
    TrieNode *current = root;
    while (*prefix) {
        int index = *prefix - 'a';
        if (!current->children[index]) {
            return NULL;
        }
        current = current->children[index];
        prefix++;
    }
    return current;
}

// Function to clean and insert word from a file character by character
void insert_from_file(TrieNode *root, FILE *f) {
    char word[MAX_WORD_LENGTH];
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
    if (j > 0) {
        word[j] = '\0';
        insert(root, word);
    }
}


// Sort suggestions along with their weights
void sortSuggestions(char suggestions[][MAX_WORD_LENGTH], double weights[], int count) {
    // print all the sugggested words
    // for(int i = 0; i < count; i++) {
    //     printf("suggestions[%d] = %s\n", i, suggestions[i]);
    // }

    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
          

            if (weights[i] < weights[j]) {
                double tempWeight = weights[i];
                weights[i] = weights[j];
                weights[j] = tempWeight;
                
                char tempWord[MAX_WORD_LENGTH];
                strcpy(tempWord, suggestions[i]);
                strcpy(suggestions[i], suggestions[j]);
                strcpy(suggestions[j], tempWord);
                
            }
        }
    }
}





// Free Trie memory
void free_trie(TrieNode *root) {
    if (root == NULL) return;

    for (int i = 0; i < Total_Alphabets; i++) {
        if (root->children[i] != NULL) {
            free_trie(root->children[i]);
        }
    }
    free(root);
}

int main() {
    TrieNode *root = create_node();  
    TrieNode *mainTrieRoot = create_node(); 

    FILE *f = fopen("corpus_sample.txt", "r");
    if (!f) {
        printf("Error opening file\n");
        return 1;
    }


    insert_from_file(root, f);
    fclose(f);

    char sentence[MAX_WORD_LENGTH * 10]; 
    printf("Enter a sentence: ");
    fgets(sentence, sizeof(sentence), stdin);

 
    size_t len = strlen(sentence);
    if (len > 0 && sentence[len - 1] == '\n') {
        sentence[len - 1] = '\0';
    }

    char *token = strtok(sentence, " ");
    char lastWord[MAX_WORD_LENGTH] = "";

    while (token != NULL) {
       
        for (int i = 0; token[i]; i++) {
            token[i] = tolower(token[i]);
        }

        char *nextToken = strtok(NULL, " ");
        if (nextToken != NULL) {
           // printf("Inserting: %s\n", token);
            insert(mainTrieRoot, token);
        } else {
           // printf("Last word: %s\n", token);
            strcpy(lastWord, token);
        }
        
        token = nextToken; 
    }

 
    if (strlen(lastWord) == 0) {
        printf("No valid last word entered.\n");
        return 0;
    }

    TrieNode *prefixCorpusNode = findPrefixNode(root, lastWord);
    TrieNode *prefixMainNode = findPrefixNode(mainTrieRoot, lastWord);

    if (prefixCorpusNode || prefixMainNode) {
        char suggestions[MAX_SUGGESTIONS][MAX_WORD_LENGTH];
        double weights[MAX_SUGGESTIONS] = {0};
        int suggestionCount = 0;

        if(prefixCorpusNode && prefixMainNode) {
            suggestWords(prefixCorpusNode, prefixMainNode, lastWord, suggestions, weights, &suggestionCount, MAX_SUGGESTIONS);
        } else if (prefixCorpusNode) {
            suggestWords(prefixCorpusNode, NULL, lastWord, suggestions, weights, &suggestionCount, MAX_SUGGESTIONS);
        } else {
            suggestWords(NULL, prefixMainNode, lastWord, suggestions, weights, &suggestionCount, MAX_SUGGESTIONS);
        }

      
        sortSuggestions(suggestions, weights, suggestionCount);

       
        printf("Top 3 suggestions for \"%s\":\n", lastWord);
        for (int i = 0; i < suggestionCount && i < 3; i++) {
            printf("%s (weight: %.2f)\n", suggestions[i], weights[i]);
        }
    } else {
        printf("No suggestions found for \"%s\"\n", lastWord);
    }

    
    free_trie(root);
    free_trie(mainTrieRoot);

    return 0;
}
