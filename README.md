# CS201_PROJECT
## PROJECT NAME--Contextual Auto-Fill and Auto-Correct with Learning Memory using Trie Data Structure
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
2.Ensure you have a C compiler installed on your machine:
    
    For macOS/Linux: You can use GCC, which is often pre-installed or can be installed via Homebrew (brew install gcc).
    For Windows: You can use MinGW or any IDE like Code::Blocks or Dev-C++ that includes a C compiler.


3.Open a terminal (macOS/Linux) or Command Prompt/PowerShell (Windows).
4.Navigate to the directory where the downloaded file is located using the cd command. For example
