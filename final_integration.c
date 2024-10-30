#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h> 

#define Total_Alphabets 26
#define MAX_WORD_LENGTH 100
#define MAX_SUGGESTIONS 10
#define LEVENSHTEIN_LIMIT 2

// Structure of TrieNode
typedef struct TrieNode {
    struct TrieNode *children[Total_Alphabets];
    double weight;
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
// Helper function to find the maximum weight in a Trie
int findMaxWeight(TrieNode *node) {
    if (node == NULL) return 0;
    int maxWeight = node->weight;
    for (int i = 0; i < Total_Alphabets; i++) {
        if (node->children[i] != NULL) {
            int childMaxWeight = findMaxWeight(node->children[i]);
            if (childMaxWeight > maxWeight) {
                maxWeight = childMaxWeight;
            }
        }
    }
    return maxWeight;
}


// Helper function to normalize weights in Trie to 4 decimal points
void normalizeWeights(TrieNode *node, int maxWeight) {
    if (node == NULL || maxWeight == 0) return;
    
    // Normalize and round to 4 decimal places
    node->weight = round((double)node->weight / (maxWeight * 1.0) * 10000) / 10000.0;
    
    for (int i = 0; i < Total_Alphabets; i++) {
        normalizeWeights(node->children[i], maxWeight);
    }
}


// Helper function to calculate combined weight
double getCombinedWeight(TrieNode *mainNode, TrieNode *corpusNode) {
    double mainWeight = mainNode ? mainNode->weight : 0;
    double corpusWeight = corpusNode ? corpusNode->weight : 0;
    if (mainWeight > 0 && corpusWeight > 0) {
        return (mainWeight + corpusWeight) / 2.0;  
    } else if (mainWeight > 0) {
     
        return mainWeight * 2;
    } else {
        return corpusWeight; 
    }
}

// Suggest words based on prefix and prioritize by combined weight
void suggestWords(TrieNode *corpusTrie, TrieNode *mainTrie, char *prefix, char suggestions[][MAX_WORD_LENGTH], double weights[], int *suggestionCount, int maxSuggestions) {

    if (*suggestionCount >= maxSuggestions) return;


    if ((corpusTrie && corpusTrie->checkisEndOfWord) || (mainTrie && mainTrie->checkisEndOfWord)) {
        if (*suggestionCount < maxSuggestions) {
            double combinedWeight = getCombinedWeight(mainTrie, corpusTrie);

            strcpy(suggestions[*suggestionCount], prefix);
            weights[*suggestionCount] = combinedWeight;
            (*suggestionCount)++;
        }
    }


    for (int i = 0; i < Total_Alphabets; i++) {
        if ((corpusTrie && corpusTrie->children[i]) || (mainTrie && mainTrie->children[i])) {
            char nextPrefix[MAX_WORD_LENGTH];
            sprintf(nextPrefix, "%s%c", prefix, 'a' + i);

            TrieNode *nextCorpusTrie = corpusTrie && corpusTrie->children[i] ? corpusTrie->children[i] : NULL;
            TrieNode *nextMainTrie = mainTrie && mainTrie->children[i] ? mainTrie->children[i] : NULL;

          
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

int levenshtein_distance(const char *s1, const char *s2) {
    int len1 = strlen(s1);
    int len2 = strlen(s2);
    int **d = (int **)malloc((len1 + 1) * sizeof(int *));
    for (int i = 0; i <= len1; i++)
        d[i] = (int *)malloc((len2 + 1) * sizeof(int));

    for (int i = 0; i <= len1; i++) d[i][0] = i;
    for (int j = 0; j <= len2; j++) d[0][j] = j;

    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            d[i][j] = fmin(fmin(d[i - 1][j] + 1, d[i][j - 1] + 1), d[i - 1][j - 1] + cost);
        }
    }
    int distance = d[len1][len2];
    for (int i = 0; i <= len1; i++) free(d[i]);
    free(d);
    return distance;
}


// Collect suggestions based on prefix with auto-correction logic
void collect_suggestions(TrieNode *root, char *prefix, int level, char suggestions[][MAX_WORD_LENGTH], double weights[], int *count, const char *input, double alpha, int target_length) {
    if (root == NULL) return;

    // Check if this is a valid end of a word
    if (root->checkisEndOfWord) {
        int lev_dist = levenshtein_distance(prefix, input);
        // Check if the levenshtein distance is within the acceptable range
        if (lev_dist <= LEVENSHTEIN_LIMIT) {
            double normalized_weight = (double)root->weight;
            double score = alpha * (1.0 / (lev_dist + 1)) + (1 - alpha) * normalized_weight;
            // Add only if there is room for more suggestions
            if (*count < MAX_SUGGESTIONS) {
                strcpy(suggestions[*count], prefix);
                weights[*count] = score;
                (*count)++;
            }
        }
    }

    // Continue exploring children nodes
    for (int i = 0; i < Total_Alphabets; i++) {
        if (root->children[i]) {
            prefix[level] = 'a' + i;
            prefix[level + 1] = '\0';
            collect_suggestions(root->children[i], prefix, level + 1, suggestions, weights, count, input, alpha, target_length);
            prefix[level] = '\0';  // Backtrack
        }
    }
}

// Suggest words based on last word with better handling for suggestions
void suggestWords_for_correction(TrieNode *corpusTrie, TrieNode *mainTrie, char *lastWord, char suggestions1[][MAX_WORD_LENGTH], double weights1[], int *suggestionCount1, int maxSuggestions, char suggestions2[][MAX_WORD_LENGTH], int *suggestionCount2, double weights2[]) {
    int target_length = strlen(lastWord);
    double alpha = 0.7; // Adjust alpha for scoring

    // Collect suggestions from both tries
    collect_suggestions(corpusTrie, lastWord, 0, suggestions1, weights1, suggestionCount1, lastWord, alpha, target_length);
    collect_suggestions(mainTrie, lastWord, 0, suggestions2, weights2, suggestionCount2, lastWord, alpha, target_length);

    // Limit the number of suggestions to maxSuggestions if needed
    if (*suggestionCount1 > maxSuggestions) {
        *suggestionCount1 = maxSuggestions;
    }
    if (*suggestionCount2 > maxSuggestions) {
        *suggestionCount2 = maxSuggestions;
    }

    // Print a message if no suggestions were collected from either Trie
    if (*suggestionCount1 == 0 && *suggestionCount2 == 0) {
        printf("No suggestions found for \"%s\"\n", lastWord);
    }
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

    // Load words from corpus file
    FILE *f = fopen("corpus_sample.txt", "r");
    if (!f) {
        printf("Error opening file\n");
        return 1;
    }
    insert_from_file(root, f);
    fclose(f);

    // Get user input sentence
    char sentence[MAX_WORD_LENGTH * 10];
    printf("Enter a sentence: ");
    fgets(sentence, sizeof(sentence), stdin);
    sentence[strcspn(sentence, "\n")] = '\0';  // Remove newline

    // Process the last word in the sentence
    char *token = strtok(sentence, " ");
    char lastWord[MAX_WORD_LENGTH] = "";

    while (token != NULL) {
        // Convert to lowercase
        for (int i = 0; token[i]; i++) {
            token[i] = tolower(token[i]);
        }
        char *nextToken = strtok(NULL, " ");
        if (nextToken != NULL) {
            insert(mainTrieRoot, token);
        } else {
            strcpy(lastWord, token);
        }
        token = nextToken;
    }

    if (strlen(lastWord) == 0) {
        printf("No valid last word entered.\n");
        return 0;
    }

    // Normalize weights for both tries
    int maxWeight1 = findMaxWeight(root);
    int maxWeight2 = findMaxWeight(mainTrieRoot);
    normalizeWeights(root, maxWeight1);
    normalizeWeights(mainTrieRoot, maxWeight2);

    TrieNode *prefixCorpusNode = findPrefixNode(root, lastWord);
    TrieNode *prefixMainNode = findPrefixNode(mainTrieRoot, lastWord);

    char choice;
    printf("Enter 'f' for Auto-fill and 'c' for Auto-correct: ");
    scanf(" %c", &choice);

    if (choice == 'f') {
        if (prefixCorpusNode || prefixMainNode) {
            char suggestions[MAX_SUGGESTIONS][MAX_WORD_LENGTH];
            double weights[MAX_SUGGESTIONS] = {0};
            int suggestionCount = 0;

            suggestWords(prefixCorpusNode, prefixMainNode, lastWord, suggestions, weights, &suggestionCount, MAX_SUGGESTIONS);
            sortSuggestions(suggestions, weights, suggestionCount);

            printf("Top suggestions for \"%s\":\n", lastWord);
            for (int i = 0; i < suggestionCount && i < 3; i++) {
                printf("%s (Weight: %.4f)\n", suggestions[i], weights[i]);
            }
        } else {
            printf("No suggestions found for \"%s\"\n", lastWord);
        }
    } else if (choice == 'c') {
        // Auto-correct functionality
        char suggestions1[MAX_SUGGESTIONS][MAX_WORD_LENGTH];
        double weights1[MAX_SUGGESTIONS] = {0};
        int suggestionCount1 = 0;

        char suggestions2[MAX_SUGGESTIONS][MAX_WORD_LENGTH];
        double weights2[MAX_SUGGESTIONS] = {0};
        int suggestionCount2 = 0;
        char lastWord_copy[MAX_WORD_LENGTH];
        strcpy(lastWord_copy, lastWord);
        suggestWords_for_correction(root, mainTrieRoot, lastWord, suggestions1, weights1, &suggestionCount1, MAX_SUGGESTIONS, suggestions2, &suggestionCount2, weights2);

        // printf the top suggestion from corpus and mainTrie
         printf("Top 3 suggestions for \"%s\" from the history trie:\n", lastWord_copy);
        for (int i = 0; i < suggestionCount1; i++) {
            printf("%s (Score: %.4f)\n", suggestions1[i], weights1[i]);
        }
        printf("Top 3 suggestions for \"%s\" from the main trie:\n", lastWord_copy);
        for (int i = 0; i < suggestionCount2; i++) {
            printf("%s (Score: %.4f)\n", suggestions2[i], weights2[i]);
        }
    } else {
        printf("Invalid choice. Enter 'f' or 'c'.\n");
    }

    // Free allocated memory
    free_trie(root);
    free_trie(mainTrieRoot);

    return 0;
}