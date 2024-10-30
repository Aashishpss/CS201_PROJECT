#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h> 

#define Total_Alphabets 26
#define MAX_WORD_LENGTH 100
#define MAX_SUGGESTIONS 3
#define LEVENSHTEIN_LIMIT 2

// Structure of TrieNode
typedef struct TrieNode {
    struct TrieNode *children[Total_Alphabets];
    double weight;
    bool checkisEndOfWord;
} TrieNode;

// Structure for holding suggestions
typedef struct Suggestion {
    char word[MAX_WORD_LENGTH];
    double score;
    bool combined; // flag to mark if weight is combined from both tries
} Suggestion;


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

// Function to find or update existing suggestion, averaging the weight if found
int find_or_update_suggestion(Suggestion *suggestions, int *count, const char *word, double new_score) {
    for (int i = 0; i < *count; i++) {
        if (strcmp(suggestions[i].word, word) == 0) {
            if (!suggestions[i].combined) {
                suggestions[i].score = (suggestions[i].score + new_score) / 2;
                suggestions[i].combined = true;
            }
            return i;
        }
    }
    return -1;
}

// Recursive function to collect suggestions of the same length
void collect_suggestions(TrieNode *root, char *prefix, int level, Suggestion *suggestions, int *count, const char *input, double alpha, double max_weight, int target_length) {
    if (root == NULL) return;

    if (root->checkisEndOfWord && level == target_length) {
        int lev_dist = levenshtein_distance(prefix, input);
        if (lev_dist <= LEVENSHTEIN_LIMIT) {
            double normalized_weight = (double)root->weight / max_weight;
            double score = alpha * (1.0 / (lev_dist + 1)) + (1 - alpha) * normalized_weight;
            int index = find_or_update_suggestion(suggestions, count, prefix, score);
            if (index == -1) {
                strcpy(suggestions[*count].word, prefix);
                suggestions[*count].score = score;
                suggestions[*count].combined = false;
                (*count)++;
            }
        }
    }

    if (level < target_length) {
        for (int i = 0; i < Total_Alphabets; i++) {
            if (root->children[i]) {
                prefix[level] = 'a' + i;
                prefix[level + 1] = '\0';
                collect_suggestions(root->children[i], prefix, level + 1, suggestions, count, input, alpha, max_weight, target_length);
                prefix[level] = '\0';
            }
        }
    }
}

// Function to suggest words based on combined score and matching length
void suggest_words(TrieNode *currentTrie, TrieNode *pastTrie, const char *input, double alpha) {
    Suggestion suggestions[1000];
    int count = 0;
    char prefix[MAX_WORD_LENGTH] = "";
    int input_length = strlen(input);

    double max_weight_current = 1.0, max_weight_past = 1.0;

    // Get maximum weight for normalization
    max_weight_current = currentTrie->weight ? currentTrie->weight : 1.0;
    max_weight_past = pastTrie->weight ? pastTrie->weight : 1.0;

    // Collect suggestions of the same length as the input word
    collect_suggestions(currentTrie, prefix, 0, suggestions, &count, input, alpha, max_weight_current, input_length);
    collect_suggestions(pastTrie, prefix, 0, suggestions, &count, input, alpha, max_weight_past, input_length);

    // Sort suggestions based on score
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (suggestions[i].score < suggestions[j].score) {
                Suggestion temp = suggestions[i];
                suggestions[i] = suggestions[j];
                suggestions[j] = temp;
            }
        }
    }

    printf("Suggestions for '%s':\n", input);
    for (int i = 0; i < count; i++) {
        printf("%s (Score: %.2f)\n", suggestions[i].word, suggestions[i].score);
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
        
        suggest_words(mainTrieRoot, root, lastWord, 0.7);
        
    } else {
        printf("Invalid choice. Enter 'f' or 'c'.\n");
    }

    // Free allocated memory
    free_trie(root);
    free_trie(mainTrieRoot);

    return 0;
}
