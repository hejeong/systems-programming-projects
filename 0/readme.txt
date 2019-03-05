This program separates the letter strings by using a custom string tokenizer that iterates through the given line then marks the first letter it finds and the last letter of the first token it finds.
It adds a null terminator in between the token and the remaining string and moves the pointer of the original string to after the token as a way to split this two strings.
The pointer to the rest of the string is not overwritten between calls of the string tokenizer.
The token the tokenizer returns is added to a linked list through insertion sort.
The sort uses a custom string compare function.
A custom string compare was necessary because capital letters are less than lower case letters so it would be difficult to discern what order to sort the words.
The custom string compare first checks if both are upper case or both are lower case, if they both are then the program compares just like strcmp would.
If they are not however the upper case letter takes precedence.
Whenever the appropriate place was found for the word using insertion sort, the node was inserted in the middle of the linked list.
Once the original line was completely tokenized, a final loop iterates through the linked list and prints out all the words in descending alphabetical order and also frees all the memory.