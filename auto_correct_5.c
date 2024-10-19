#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>


#define ALPHABET_SIZE 26
#define MAX_WORD_LEN 100
#define LEVENSHTEIN_LIMIT 2 // Max distance for considering suggestions

// Structure of TrieNode
typedef struct TrieNode {
    struct TrieNode *children[ALPHABET_SIZE];
    int weight;
    bool isEndOfWord;
} TrieNode;

// Structure for holding suggestions
typedef struct Suggestion {
    char word[MAX_WORD_LEN];
    double score;
} Suggestion;

// Create a new TrieNode
TrieNode *create_node() {
    TrieNode *new_node = (TrieNode *)malloc(sizeof(TrieNode));
    new_node->isEndOfWord = false;
    new_node->weight = 0;
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        new_node->children[i] = NULL;
    }
    return new_node;
}

// Insert a word into the Trie
void insert(TrieNode *root, const char *key, int weight) {
    TrieNode *temp = root;
    for (int i = 0; i < strlen(key); i++) {
        int index = key[i] - 'a';
        if (!temp->children[index]) {
            temp->children[index] = create_node();
        }
        temp = temp->children[index];
    }
    temp->isEndOfWord = true;
    temp->weight = weight;
}

// Function to clean and insert word from a file character by character
void insert_from_file(TrieNode *root, FILE *f) {
    char word[MAX_WORD_LEN];
    int j = 0;
    char ch;

    while ((ch = fgetc(f)) != EOF) {
        if (isalpha(ch)) {
            word[j++] = tolower(ch);
        } else {
            if (j > 0) {
                word[j] = '\0';
                insert(root, word, 1); // Assign initial weight of 1 for new words
                j = 0;
            }
        }
    }

    if (j > 0) {
        word[j] = '\0';
        insert(root, word, 1);
    }
}

// Calculate Levenshtein distance
int levenshtein_distance(const char *s1, const char *s2) {
    int len1 = strlen(s1), len2 = strlen(s2);
    int dp[len1 + 1][len2 + 1];

    for (int i = 0; i <= len1; i++)
        dp[i][0] = i;
    for (int j = 0; j <= len2; j++)
        dp[0][j] = j;

    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            dp[i][j] = fmin(fmin(dp[i - 1][j] + 1, dp[i][j - 1] + 1), dp[i - 1][j - 1] + cost);
        }
    }
    return dp[len1][len2];
}

// Recursive function to collect suggestions
void collect_suggestions(TrieNode *root, char *prefix, int level, Suggestion *suggestions, int *count, const char *input, double alpha, double max_weight) {
    if (root == NULL) return;

    if (root->isEndOfWord) {
        int lev_dist = levenshtein_distance(prefix, input);
        if (lev_dist <= LEVENSHTEIN_LIMIT) {
            double normalized_weight = (double)root->weight / max_weight;
            double score = alpha * (1.0 / (lev_dist + 1)) + (1 - alpha) * normalized_weight;
            strcpy(suggestions[*count].word, prefix);
            suggestions[*count].score = score;
            (*count)++;
        }
    }

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (root->children[i]) {
            prefix[level] = 'a' + i;
            prefix[level + 1] = '\0';
            collect_suggestions(root->children[i], prefix, level + 1, suggestions, count, input, alpha, max_weight);
            prefix[level] = '\0';
        }
    }
}

// Function to suggest words based on combined score
void suggest_words(TrieNode *currentTrie, TrieNode *pastTrie, const char *input, double alpha) {
    Suggestion suggestions[1000];
    int count = 0;
    char prefix[MAX_WORD_LEN] = "";

    double max_weight_current = 1.0, max_weight_past = 1.0;

    // Get maximum weight for normalization
    max_weight_current = currentTrie->weight ? currentTrie->weight : 1.0;
    max_weight_past = pastTrie->weight ? pastTrie->weight : 1.0;

    collect_suggestions(currentTrie, prefix, 0, suggestions, &count, input, alpha, max_weight_current);
    collect_suggestions(pastTrie, prefix, 0, suggestions, &count, input, alpha, max_weight_past);

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

// Insert each word from a sentence into the Trie
void insert_sentence(TrieNode *root, const char *sentence) {
    char word[MAX_WORD_LEN];
    int index = 0;
    for (int i = 0; sentence[i] != '\0'; i++) {
        if (isalpha(sentence[i])) {
            word[index++] = tolower(sentence[i]);
        } else if (index > 0) {
            word[index] = '\0';
            insert(root, word, 1);
            index = 0;
        }
    }
    if (index > 0) {
        word[index] = '\0';
        insert(root, word, 1);
    }
}

int main() {
    TrieNode *pastTrie = create_node();
    TrieNode *currentTrie = create_node();

    FILE *f = fopen("corpus_sample.txt", "r");
    if (!f) {
        printf("Error opening file\n");
        return 1;
    }

    insert_from_file(pastTrie, f);
    fclose(f);

    // Take a sentence as input and insert each word into the current Trie
    char sentence[256];
    printf("Enter a sentence: ");
    fgets(sentence, sizeof(sentence), stdin);
    insert_sentence(currentTrie, sentence);

    // Take an incorrect word as input to auto-correct
    char input_word[MAX_WORD_LEN];
    printf("Enter a word to auto-correct: ");
    scanf("%s", input_word);

    suggest_words(currentTrie, pastTrie, input_word, 0.7);

    return 0;
}
