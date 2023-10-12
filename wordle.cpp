#include <bit>
#include <cassert>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <unordered_set>
#include <stdexcept>
#include <unordered_map>
#include <algorithm>
#include <array>
#include <random>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

// ============== Starter code and helpers below =============

// This function gives you words of length 5 in a dictionary.

std::unordered_set<std::string> GetAllValidWords() {
  std::unordered_set<std::string> words;
  std::ifstream word_file("/home/coderpad/data/words.txt"); 
  
  if (word_file.is_open()) {
    std::string word;
    
    while (std::getline(word_file, word)) {
      if(word.size() == 5) words.insert(word); 
    }
    
    word_file.close();
  }
  return words;
}


/* ===================== LETTER STATES ===================== */

enum LetterState {
  INVALID = 0, 
  CORRECT = 1, 
  CONTAINED = 2, 
  NOT_CONTAINED = 3 
};

std::ostream& operator<<(std::ostream& os, const LetterState& state);
using WordleLetterStates = std::array<LetterState, 5>;
std::ostream& operator<<(std::ostream& os, const WordleLetterStates& states); 

// Will throw an exception if the query is not valid (i.e. not a dictionary word).
// Output is guaranteed to be valid (i.e. no INVALID states).

/* ================== WORDLE GAME CLASS ================== */

class Wordle {
  public:
    explicit Wordle(std::string true_word) : true_word_{std::move(true_word)} {} // ctor
    ~Wordle(); // dtor
    WordleLetterStates CharacterizeWord(const std::string& query) const; // evaluate guess
  private:
    std::string true_word_; // target word
    mutable size_t counter_{0}; // # of guesses
};

/* ======================================================= */




/* Put your code below this comment block. 
- Please note your approach below and why (i.e. design decisions)
- Please also note how you would optimally choose the next guess given enough time.
*/





/* ================ DEVLEOPER NOTES (Isaac Nguyen) ================= */


/* ======================== PROGRAM PURPOSE ======================== */

/*

SolveWordle() can be broken down into 2 components:

1. Given "guess" + corresponding "WordleLetterStates" -> reduce set of possible answers
2. Given set of possible answers -> pick next guess (optimally)


What can we use to develop these components?

1. Wordle.CharacterizeWord() -> for checking our guesses
2. WordleLetterStates -> used to determine valid characters

*/


/* ========================== COMPONENT #1 ========================== */

/*

Function: remainingWords() -> void (modifies set of solutions)

==========
| INPUTS | 
==========

hashset<string> possibleAnswers:
  -> our solution set 

hashmap<char, hashset<int>> contains: 
  -> correct characters, wrong index
  -> hashset (val) contains invalid indices for char

hashmap<int, char> correct:
  -> correct character + correct index
  -> index is key so we can check if words in solution set have same character at index containing correct char
  
  hashset<char> notContains:
  -> invalid chars, not part of answer



===============
| ASSUMPTIONS | 
===============

  - contains, correct, notContains are updated before function is called
  - contains + correct will mostly be disjointed w/ the exception of duplicate characters (will be handled in SolveWordle() instead)
  - function will modify "possibleAnswers" instead of returning new set
    - don't need to copy
  - all 4 inputs are passed in as a reference



=========================
| ALGORITHM (barebones) | 
=========================

1. define new empty set (will contain updated solution set)
2. iterate through "possibleAnswers", for each word, iterate throught characters
3. keep track of boolean to determine if word should be added to updated set
4. also keep track of chars current word + target word share 
  - lets call this "containsInWord"
5. conditionally update boolean:

- if char is in "notContains" -> instantly remove
- if index is in "correct" + char is NOT the same -> instantly remove 
- if char is in "contains":
  - index in set of invalid indices -> instantly remove
  - else -> add to "containsInWord"

6. after the loop for chars is finished:
  - if size of "containsinWord" != "contains" -> instantly remove
    - means that word doesn't contain all possible characters

7. if boolean is still false (should include in updated set) -> we add current word to set.

8. after all words are iterated through, we swap "possibleAnswers" w/ updated set of solutions
  - why? updated set is only in scope of our function -> rvalue + will be destroyed after function goes out of scope
  - std::swap swaps values contained by both set, updated set will now contain old values and "possibleAnswers" will contain updated values
  - updated set will then be destroyed
  - saves us trouble of copying + memory management



==============
| PSEUDOCODE |
==============

Function remainingWords(possibleAnswers, contains, notContains, correct)
    updatedPossibleAnswers = Empty Set
    
    For each word in possibleAnswers
        containsInWord = Empty Set
        shouldRemove = False
        
        For each character, char, in word
            If char is in notContains
                shouldRemove = True
                Break
            
            If correct contains current index and char at that index is not correct
                shouldRemove = True
                Break
            
            If char is in contains
                If index is not allowed in contains[char]
                    shouldRemove = True
                    Break
                Else
                    Add char to containsInWord
            
        If size of contains != size of containsInWord
            shouldRemove = True
        
        If not shouldRemove
            Add word to updatedPossibleAnswers
    
    Swap updatedPossibleAnswers w/ possibleAnswers
End Function



=======================
| TESTING + DEBUGGING |
=======================

For testing, I used my own environment to maximize test coverage.

TESTING HARNESS CODE:

int main() {
    std::unordered_set<std::string> possibleAnswers = {INPUT};
    std::unordered_map<char, std::unordered_set<int>> contains = {INPUT};
    std::unordered_set<char> notContains = {INPUT};
    std::unordered_map<int, char> correct = {INPUT};
    // Call wordle solver function
    remainingWords(possibleAnswers, contains, notContains, correct);
  
    std::cout << "Reduced possibleAnswers set: ";
    for (const auto& word : possibleAnswers) {
        std::cout << word << " ";
    }
    std::cout << std::endl;
    return 0;
}


SAMPLE TEST CASE:
------------------------------------------------------------------------
possibleAnswers = {"apple", "table", "chair", "train", "snake", "appee", "apppe"};

contains = {
  {'p', {3}}
};

correct = {
  {0, 'a'}, 
  {4, 'e'}
};

notContains = {'b', 'c'};


Expected: {apple, appee}
------------------------------------------------------------------------

*/


/* ========================== COMPONENT #2 ========================== */

/*

Function: getNextGuess() -> string (our next guess)
How can we choose the most optimal guess? GREEDY!
We also want this component to be independent of other components (aka returning optimal answer for current guess from current solution set)



==========
| INPUTS | 
==========

hashset<string> possibleAnswers:
  -> our solution set 

hashset<char> guessedLetters:
  -> set of characters we've already guessed



===============
| ASSUMPTIONS | 
===============

- possibleAnswers is already updated by Component #1 before calling
- guessedLetters is updated in main function (SolveWordle)
- function has no information on contains, correct, notContains
- will have helper function calculateLetterOverlap()



=========================
| ALGORITHM (barebones) | 
=========================

Once again, how do we greedily pick the next optimal guess? My idea is to pick the word least likely to be the answer.

TO DETERMINE THIS: pick the word w/ the least overlap with "guessedLetters"

Reasoning: picking the least likely answer (word w/ minimum overlap) will give us more information on characters that has not been guessed yet. 

Proof of Correctness: 
  - remaining words in solution will have x overlapping characters w/ set of "guessedLetters" (char's we have information on)
  - let y be # of non-overlapping characters. Higher x -> Lower y
  - guessing word w/ y = 2 will eliminate 2 * b_2, where b_y is # of  wordscontaining at least one of the y overlapping characters
  - guessing y = 1 will eliminate 1 * b_1
  - in some cases, 1 * b_1 > 2 * b_2
  - but b_y is arbitrary as it changes w/ each solution set
  - so in most cases, a higher y yields better results.


DISCLAIMER: My algorithm may not be the most optimal, however, I believe it to be suitable for the scope of this assignment


1. Create a helper function that checks for # of overlapping characters a word has given set of "guessedLetters"
  - loop through chars in words -> check if char is in "guessedLetters"

2. Keep track of word w/ least overlap + min # of overlap. The word will be our return value.

3. Loop through words in solution set.
  - for each word, call helper function

4. If value returned < min # of overlap,
  - update answer + min # overlap

5. Return answer



==============
| PSEUDOCODE |
==============

Function calculateLetterOverlap(word, guessedLetters)
    overlap = 0
    For each character, c, in word
        If c is in guessedLetters
            Increment overlap by 1
    End For
    Return overlap
End Function

Function getNextGuess(possibleAnswers, guessedLetters)
    minOverlap = Positive Infinity
    leastLikelyWord = Empty String
    
    For each word in possibleAnswers
        overlap = calculateLetterOverlap(word, guessedLetters)
        If overlap < minOverlap
            Set minOverlap to overlap
            Set leastLikelyWord to word
        End If
    End For
    
    Return leastLikelyWord
End Function



=======================
| TESTING + DEBUGGING |
=======================

For testing, I used my own environment to maximize test coverage.

TESTING HARNESS CODE:

int main() {
    std::unordered_set<std::string> possibleAnswers = {INPUT};
    std::unordered_set<char> guessedLetters = {INPUT};
    std::string guess = getNextGuess(possibleAnswers, guessedLetters);

    std::cout << guess;
    std::cout << std::endl;
    return 0;
}


SAMPLE TEST CASE:
------------------------------------------------------------------------

possibleAnswers = {"pease", "fease", "cease", "abase", "ukase", "erase"};

guessedLetters = {'s', 'l', 'a', 't', 'e'};


Expected: "ukase" -> 2 non-overlaps 'u', 'k'
-----------------------------------------------------------------------

*/


/* ======================== MAIN COMPONENT ========================= */

/*

Function: SolveWordle() -> string (answer to Wordle Game)
DISCLAIMER: please see "Design Decisions" section for details regarding modifying class "Wordle"

=========================
| ALGORITHM (barebones) | 
=========================

This is pretty much the wrapper function for both components 1 and 2. SolveWordle must perform the following tasks:

- Contain our data structures
- Make calls to "CharacterizeWord()" to verify guesses
- MOST IMPORTANTLY: correctly classify character states based on array returned by CharacterizeWord() 
  - our assumptions for Component 1 heavily rely on this task. Incorrectly classified character sets "contains", "correct", and "notContains" will result in invalid solution set.
- Catch errors + provide debugging harness for our components



1. Initialize all data structures (contains, correct, notContains, possibleAnswers, etc.)
2. Add all 5 letter words in "words.txt" to possibleAnswers (call "GetAllValidWords()")
3. Set starting "guess" to "slate" (see "Design Decisions")
4. Initialize while loop to only break if size of "possibleAnswers" == 1

In loop:
  5. Call "CharacterizeWord()" w/ our guess as input
  6. Let "states" contain the value returned (WordleLetterStates array)
  7. Iterate through letters in "guess", add each character to set of "guessedLetters"
  8. For each character, compare its corresponding state in "states":

    - char's state = "CORRECT" -> add index + char to "correct"
      -> if char in "notContains" -> remove it from "notContains"

    - char's state = "CONTAINED" -> add char to "contains" + append index to list of invalid indices (value)

    - char's state  = "NOT CONTAINED":
      - we check if it is in "correct" or "contains" already
      - if yes -> add char + index to "contains"
      - else -> add char to "notContains"

  9. Call "remainingWords()" to reduce solution space 
  10. Error Catching:
    - if after reducing solution space, size of "possibleAnswers" = 0 -> throw exception
    - to help debug, we print out all of our data structures + current guess
  11. if "possibleAnswers" is still valid, call "getNextGuess()" to update our guess


12. While loop will loop until size of "possibleAnswers" = 1, the answer will be the sole element
13. Return this element



==============
| PSEUDOCODE |
==============

Function SolveWordle(wordle)
    
    starting = "slate"
    possibleAnswers = GetAllValidWords()  
    contains = Empty Map  
    notContains = Empty Set  
    correct = Empty Map 
    
    guess = starting
    guessedLetters = Empty Set
    
    While size of possibleAnswers > 1
        states = wordle.CharacterizeWord(guess)
        
        For each idx in 0 to size of guess - 1
            char = guess[idx]
            
            If states[idx] is CORRECT
                correct[idx] = char
                If char is in notContains
                    Remove char from notContains
                Continue
            
            If states[idx] is CONTAINED
                Add idx to contains[char]
                Continue
            
            If states[idx] is NOT_CONTAINED
                isContained = False
                
                For each pair in correct
                    If pair.second is char
                        isContained = True
                        Break
                
                For each pair in contains
                    If pair.first is char
                        isContained = True
                        Break
                
                If isContained
                    Add idx to contains[char]
                Else
                    Add char to notContains
                Continue
        
        Call remainingWords(possibleAnswers, contains, notContains, correct)
        
        If size of possibleAnswers is 0
            Throw Error "Error Encountered"
        
        guess = getNextGuess(possibleAnswers, guessedLetters)
    
    Output "Word: " + guess
    Return guess
End Function


=======================
| TESTING + DEBUGGING |
=======================

I used the provided testing harness to test the validity of my code. 
To improve testing efficiency, I enabled the testing harness to run an arbitrary amount of tests.
Each test will get a random word from the set of all valid words using a helper function
For testing specific cases, I just used the harness that was provided

Improved Testing Harness Code:

std::unordered_set<std::string> & fiveLetterWord = GetAllValidWords();

std::string getRandomWord (const std::unordered_set<std::string> & fiveLetterWords) {
  std::random_device rd;
  std::mt19937 rng(rd());
  auto it = fiveLetterWords.begin();
  std::advance(it, rng() % fiveLetterWords.size());
  return *it;
}

int numberOfTests = 380; // max 380 tests at once
TEST_CASE("WordleTest_", "[given_test]") {
  for (int i = 0; i < numberOfTests; ++i) {
    std::string randomWord = getRandomWord(fiveLetterWords);
    Wordle wordle{randomWord};
    REQUIRE_THAT(SolveWordle(wordle), Equals(randomWord));
  }
}

*/

/* ================== DESIGN DECISIONS + CHALLENGES ================= */

/*

Throughout this assignment, I faced certain challenges that requires me as a developer to make decisions that alters my approach to the task

Below is a list of challenges and the decisions made to address them:


================
| CHALLENGE #1 |
================

Can't call "CharacterizeWord()" from SolveWordle()

Reason: 
  - Wordle Object was passed in as a constant reference
  - Since "CharacterizeWord()" mutates the private data field "counter",
    it is invalid to call the function on a constant reference.

Decision:
  - Make "CharacterizeWord()" a const function
  - Make "counter" data field "mutable"

Rationale:
  - Making "CharacterizeWord()" constant allows us preserve "constant-ness" of the Wordle class.
  - However we still need to update counter.
  - Making "counter" data field "mutable" allows the Wordle Object to be modified internally from "CharacterizeWord()"
  - We preserve encapsulation by allowing this operation to be performed internally while still abstracting private fields they are immutable outside of the Object


================
| CHALLENGE #2 |
================

Choosing starting word

Reason:
  - Technically we can start with any word in the valid list of words.
  - But that is not optimal, different starting words yields different consequent solution sets.

Decision:
  - Starting word is always "slate"

Rationale:
  - From a quick google search, "slate" is one of the best starting words for minimizing guesses
  - It was also in the list of valid words.


================
| CHALLENGE #3 |
================

Duplicate characters (whether in target or guessing word)

Reason: 
  - words containing duplicate characters yields different results from calling "CharacterizeWord()"
  - several variations include:
    - duplicate in target word
    - duplicate in guessing word
      - first/second duplicate is in correct/incorrect index

    and etc.

Decision:
  - not quite a design decision, but rather changes to implementation through rigorous testing, but:
  - Conditional checking in "SolveWordle()" when classifying characters for each edge case.
  - Sample Edge Cases:
    - duplicate in target word, but char is not duplicated in guess, vice versa
    - first duplicate in guess is "CORRECT", second is "NOT CONTAINED", vice versa
    - multiple duplicates in either one

Rationale:
  - as this is moreso a "challenge" rather than a "design decision", all implementation changes are done based on testing edge cases until algorithm considers all possible cases. 

TLDR: There are too many cases to list out here + their corresponding fixes. If markers are interested in hearing about them, I'd be happy to discuss them on call. But the algorithm correctly considers all possible cases



*/

/* ================== IMPROVEMENTS + OPTIMIZATIONS ================== */

/*

Given the chance to continue on this project, I would do the following:

==================
| IMPROVEMENT #1 |
==================

Develop a better "picking next guess" algorithm, preferably DP solution using trees that partions each word in solution set depending on guess + target word. 

I believe this would be a lot more optimal than picking next guess based on its overlapped with letters guessed so far.

Reason: this method of greedily choosing next guess doesn't consider information from previous guesses, which could be "cached" in a DP solution and used to more accurately determine next guess


==================
| IMPROVEMENT #2 |
==================

Write potentially cleaner code + be more rigorous with error handling

This implementation handles one major error, but relies on certain assumptions to run flawlessly

Some includes:
  - words in "possibleAnswers" are always of length 5
  - length of "states" and "guess" is the same
  - etc.
 
These assumptions (whether specified or not) allow our implementation to be less rigorous.

However in the real world, these assumptions do not always hold and its always important to write rigorous code that doesn't break, especially for a marketable product.

*/

/* THANK YOU FOR THIS OPPORTUNITY! I had a lot of fun + learned a lot from this assignment! */

/* ========================= END OF NOTES ========================== */


/*     calculateLetterOverlap() -> calculates # of overlapping chars     */
int calculateLetterOverlap(const std::string & word, const std::unordered_set<char> & guessedLetters) {
  int overlap = 0;
  for (char c: word) {
    if (guessedLetters.find(c) != guessedLetters.end()) {
      overlap++;
    }
  }

  return overlap;
}

/*     getNextGuess() -> get next best guess     */
std::string getNextGuess(const std::unordered_set<std::string> & possibleAnswers, const std::unordered_set<char> & guessedLetters) {

  int minOverlap = std::numeric_limits<int>::max();
  std::string leastLikelyWord;

  for (const auto & word : possibleAnswers) {

      /*     evaluate overlap + update minOverlap/leastLikelyWord      */
      int overlap = calculateLetterOverlap(word, guessedLetters);
      if (overlap < minOverlap) {
          minOverlap = overlap;
          leastLikelyWord = word;
      }
  }

  return leastLikelyWord;

}

/*     remainingWords() -> reduce solution set     */
void remainingWords(std::unordered_set<std::string> & possibleAnswers, std::unordered_map<char, std::unordered_set<int>> & contains, const std::unordered_set<char> & notContains,  std::unordered_map<int, char> & correct) { 
  std::unordered_set<std::string> updatedPossibleAnswers; 

  /*     loop through solution set     */
  for (const std::string & word: possibleAnswers) {
    std::unordered_set<char> containsInWord; 
    bool shouldRemove = false; 

    /*     loop through chars     */
    for (size_t idx = 0; idx < word.size(); ++idx) {
      if(notContains.find(word[idx]) != notContains.end()) {
        shouldRemove = true; 
        break; 
      }
      if((correct.find(idx) != correct.end()) && !(word[idx] == correct[idx])) { 
        shouldRemove = true; 
        break; 
      }
      if((contains.find(word[idx]) != contains.end())) {
        if (contains[word[idx]].find(idx) != contains[word[idx]].end()) {
          shouldRemove = true; 
          break; 
        } 

        containsInWord.insert(word[idx]); // add to compare later
        continue;
      }
    }

    if (contains.size() != containsInWord.size()) {
      shouldRemove = true;
    }
    if (!shouldRemove) {
      updatedPossibleAnswers.insert(word);
    }
  }

  std::swap(updatedPossibleAnswers, possibleAnswers);
}

/*     SolveWordle() -> returns answer to wordle game     */
std::string SolveWordle(const Wordle& wordle) {
  const std::string starting = "slate";  

  /*     Solution Set     */
  std::unordered_set<std::string> possibleAnswers = GetAllValidWords(); 

  /*     Letter State Sets     */
  std::unordered_map<char, std::unordered_set<int>> contains; 
  std::unordered_set<char> notContains; 
  std::unordered_map<int, char> correct; 

  /*     Other     */
  std::string guess = starting; 
  WordleLetterStates states;
  std::unordered_set<char> guessedLetters;

  /*     iterating guesses     */
  while (possibleAnswers.size() > 1) {
    states = wordle.CharacterizeWord(guess);

    /*     classifying characters     */
    for (size_t idx = 0; idx < guess.size(); ++idx) {
      guessedLetters.insert(guess[idx]);
      if (states[idx] == CORRECT) {
        correct[idx]= guess[idx];
        if(notContains.find(guess[idx]) != notContains.end()) {
          notContains.erase(guess[idx]); // remove
        }
        continue;
      }
      if (states[idx] == CONTAINED) {
        contains[guess[idx]].insert(idx);
        continue;
      }
      if (states[idx] == NOT_CONTAINED) {
        bool isContained = false;
        
        /*     check if char already in "correct"     */
        for (const auto& pair: correct) {
          if (pair.second == guess[idx]) {
            isContained = true;
            break;
          }
        }

        /*     check if char already in "contains"     */
        for (const auto& pair: contains) {
          if (pair.first == guess[idx]) {
            isContained = true;
            break;
          }
        }

        if (isContained) {
          contains[guess[idx]].insert(idx);
        } 
        else {
          notContains.insert(guess[idx]);
          continue;
        }
      }
    }

    /*     reduce solution set     */
    remainingWords(possibleAnswers, contains, notContains, correct);
  
    /*     error catching     */
    if (possibleAnswers.size() == 0) {
      throw std::logic_error{"Error Encountered"};
      std::cout << "Guess: " << guess << std::endl;
      for (size_t idx = 0; idx < guess.size(); ++idx) {
        std:: cout << guess[idx] << " " << states[idx] << std::endl;
      }
      std::cout << "Contains: " << std::endl;
      for (auto & word: contains) {
        std::cout << word.first << ": ";
        for (auto & cc: word.second) {
          std::cout << cc << ", ";
        }
        std::cout << std::endl;
      }
      std::cout << "Correct: " << std::endl;
      for (auto & word: correct) {
        std::cout << word.first << ": " << word.second << std::endl;
      }
      std::cout << "Not Contains: " << std::endl;
      for (auto & word: notContains) {
        std::cout << word << std::endl;
      }
      break;
    }

    guess = getNextGuess(possibleAnswers, guessedLetters);
  } 

  std::cout << "Word: " << guess << std::endl;
  return guess;
}

using Catch::Matchers::Equals;



/* ============== TESTING ============== */


/*==========================*/
/* WORD GENERATING FUNCTION */
/*==========================*/
std::string getRandomWord (const std::unordered_set<std::string> & fiveLetterWords) {
  std::random_device rd;
  std::mt19937 rng(rd());
  auto it = fiveLetterWords.begin();
  std::advance(it, rng() % fiveLetterWords.size());
  return *it;
}

std::unordered_set<std::string>fiveLetterWords = GetAllValidWords();



/*=========================*/
/* INDIVIDUAL UNIT TESTING */
/*=========================*/
/*
TEST_CASE("WordleTest_pross", "[given_test]") {
 Wordle wordle{"pross"};
 REQUIRE_THAT(SolveWordle(wordle), Equals("pross"));
}
*/

/*===================*/
/* MAIN TEST HARNESS */
/*============= =====*/
int numberOfTests = 380; // max 380 tests at once
TEST_CASE("WordleTest_", "[given_test]") {
  for (int i = 0; i < numberOfTests; ++i) {
    std::string randomWord = getRandomWord(fiveLetterWords);
    Wordle wordle{randomWord};
    REQUIRE_THAT(SolveWordle(wordle), Equals(randomWord));
  }
}




/*
==================================Starter code definitions ===============================================
*/

std::ostream& operator<<(std::ostream& os, const LetterState& state) {
 switch (state) {
   case INVALID:
     os << "INVALID";
     break;
   case CORRECT:
     os <<"CORRECT";
     break;
   case CONTAINED:
     os << "CONTAINED";
     break;
   case NOT_CONTAINED:
     os << "NOT_CONTAINED";
     break;
 }
  return os;
}

// ValidateWord(string) -> determines if "word" is in dictionary 

void ValidateWord(const std::string& word) {
  static const auto all_words = GetAllValidWords();
  if (all_words.count(word) == 0) throw std::logic_error{"Word " + word+ " is not valid."}; 
}

void ValidateStates(const WordleLetterStates& states) {
  for(const auto& state : states) {
    if(state == INVALID) throw std::logic_error{"Detected an invalid state."}; 
  }
}

std::ostream& operator<<(std::ostream& os, const WordleLetterStates& states) {
  os << "States: ";
  for(const auto& state: states) os << state << ", ";
  return os;
}

WordleLetterStates Wordle::CharacterizeWord(const std::string& query) const {
  counter_++;
  ValidateWord(query);
  std::unordered_map<char, size_t> letter_counts;
  for(char c : true_word_) letter_counts[c]++; 
  WordleLetterStates states;
  states.fill(INVALID);
  

  for(size_t idx = 0; idx < query.size(); ++idx) {
    if(true_word_[idx] == query[idx]) {
      states[idx] = CORRECT; 
      auto letter_count_it = letter_counts.find(true_word_[idx]);
      if(letter_count_it == letter_counts.end()) throw;
      if(--(letter_count_it->second) == 0u) letter_counts.erase(letter_count_it);
    }
  }

  for(size_t idx = 0; idx < query.size(); ++idx) {
    if(states[idx] == CORRECT) continue; 
    auto letter_count_it = letter_counts.find(query[idx]); 
    if(letter_count_it == letter_counts.end()) {
      states[idx] = NOT_CONTAINED; 
    } else {
      states[idx] = CONTAINED; 
      if(--(letter_count_it->second) == 0u) letter_counts.erase(letter_count_it); 
    }
  }
  
  ValidateStates(states);
  return states;
}

Wordle::~Wordle() { 
  std::cout << "Number of guesses: " << counter_ << std::endl;
}

