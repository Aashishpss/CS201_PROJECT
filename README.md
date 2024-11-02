# CS201_PROJECT

## PROJECT NAME:
Contextual Auto-Fill and Auto-Correct with Learning Memory using Trie Data Structure

## INTRODUCTION:
This project implements an advanced auto-suggestion and auto-correction system using the Trie data structure,
targeting applications that require both word completion and error correction based on user input and historical
usage. The system integrates two Trie trees, A Corpus Trie built from a corpus file, storing words with their
frequencies from the corpus. This file is meant to contain the past writings of the user, which corresponds to
the learning memory aspect of this project, and a Main Trie for storing words typed by the user in real-time,
allowing the system to prioritize suggestions based on recent user behavior.

## DESCRIPTION ABOUT FILES WHICH ARE PRESENT IN THIS REPO:
1. auto_correct_5.c--> Visit this file for viewing the code for only auto correct functionalities.
2. autofill.c--> Visit this file for viewing the code for only auto fill functionalities.
3. corpus_sample.txt--> Text file which we have used for creating past trie.
4. CS_201_Project_Grp18.c--> "This final file contains the integrated functionalities of auto-fill and auto-correct use this for evaluation purpose as it contains a common functionalities for input and output
## How to run:
1.Download the "CS_201_Project_Grp18.c" file from this repository.

2. Ensure you have a C compiler installed on your machine:

       For macOS/Linux: You can use GCC, which is often pre-installed or can be installed via Homebrew (brew install gcc).
       For Windows: You can use MinGW or any IDE like Code::Blocks or Dev-C++ that includes a C compiler.


3.Open a terminal (macOS/Linux) or Command Prompt/PowerShell (Windows).

4. Navigate to the directory where the downloaded file is located using the cd command. For example:
       cd path/to/directory
   
5.Compile the program using the following command:

    For macOS/Linux:

    bash

    gcc CS_201_Project_Grp18.c -o CS_201_Project_Grp18

    For Windows (using MinGW):

    bash

    gcc CS_201_Project_Grp18.c -o CS_201_Project_Grp18.exe

6.Once compiled, run the program with:

    For macOS/Linux:

    bash

    ./CS_201_Project_Grp18

    For Windows:

    bash

    CS_201_Project_Grp18.exe

7.Follow the on-screen instructions to interact with the program.

## What to Input:
1. The program will start by asking what to do auto-fill or auto-correct ,choose what you want to run.
2. If you have choosen auto-fill then type a sentence where words are separated by spaces(don't write characters other than alphabets(26)) you have the flexibility to write in both uppercase and lowercase and as it is auto-fill do write the last word incomplete and then press enter to continue.
3. If you have choosen auto-correct then type a sentence where words are separated by spaces(don't write characters other than alphabets(26)) you have the flexibility to write in both uppercase and lowercase and as it is auto-correct do write the last word misspelled and then press enter to continue.

## What to expect as a Output:
1. If you have choosen auto-fill then in the outputs there will be the words which would either be present in current trie or past trie with their weights and if no word is found then no suggestion will be printed and for it you can type same words twice in the sentence and can check the output for its accuracy.
2. If you have choosen auto-correct then in the output there will be the words which will be suggested for correction based on the past and current trie and it can also be tested in the similar fashion as in 1.

# Note:
1. Don't enter characters apart from alphabets(26) like symbols,special characters,numbers etc.
2. Due to constrained memory the past trie isn't that much large so it may happen that there would be no suggestion coming up when you enter some words.
