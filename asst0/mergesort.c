//the merging part of merge sort
struct head* merge(struct head* rootL, struct head* rootR, int position)
{
	struct head* left = rootL;
	struct head* right = rootR;
	struct head* root;
	
	struct node* ptrLeft = rootL -> row;
	struct node* ptrRight = rootR -> row;

	char* s1 = strip(left -> value);  //the elements with no leading or trailing whitespaces or quotes, ready for comparison
	char* s2 = strip(right -> value);
	
	int leftIsStr = 0; //conditions to check if the two elements to compare are strings or not
	int rightIsStr = 0;
	int i;
	int leftLength = strlen(s1);
	int rightLength = strlen(s2);
	
	//checks if the two elements should be checked in numerical or lexicographical order
	//goes through each element of the string to see if it is a digit
	for(i; i < leftLength; i++)
	{
		if(isdigit(s1[i]) == 0 && s1[i] != '\0')
		{
			leftIsStr = 1;
		}				
	}
		
	i = 0;
		
	for(i; i < rightLength; i++)
	{
		if(isdigit(s2[i]) == 0 && s2[i] != '\0')
		{
			rightIsStr = 1;
		}				
	}
	//creates the head of the new sorted linked list
	if(rightIsStr == 1 || leftIsStr == 1)
	{
		//str compares if either element is a string
		if (strcmp(strip(left -> value), strip(right -> value)) < 0)
		{
			root = left;
			left = left -> next;
		}
		else
		{
			root = right;
			right = right -> next;
		}
	}
	else
	{
		//compares as doubles if both elements are numbers
		double leftVal;
		double rightVal;
		//scans the string into a double
		sscanf(strip(left -> value), "%lf", &leftVal);
		sscanf(strip(right -> value), "%lf", &rightVal);
		if (leftVal < rightVal)
		{
			root = left;
			left = left -> next;
		}
		else
		{
			root = right;
			right = right -> next;
		}
	}

	struct head* ptr = root;
	
	//iterates through all the nodes in both elements and adds whichever one is lower to the sorted linked list
	while(left != NULL || right != NULL)
	{
		//if either element is empty just adds the rest of the other element to the end
		if(left == NULL)
		{
			ptr -> next = right;
			ptr = ptr -> next;
			right = right -> next;
		}
		else if(right == NULL)
		{
			ptr -> next = left;
			ptr = ptr -> next;
			left = left -> next;
		}
		else
		{
			s1 = strip(left -> value);
			s2 = strip(right -> value);
		
			leftIsStr = 0; 
			rightIsStr = 0;
		
			int i = 0;
		
			leftLength = strlen(s1);
			rightLength = strlen(s2);
		
			//the same process from before to check if either is a string
			for(i; i < leftLength; i++)
			{
				if(isdigit(s1[i]) == 0 && s1[i] != '\0')
				{
					leftIsStr = 1;
				}				
			}
		
			i = 0;
				
			for(i; i < rightLength; i++)
			{
				if(isdigit(s2[i]) == 0 && s2[i] != '\0')
				{
					rightIsStr = 1;
				}				
			}
			//the same process from before when creating the head of the new linked list
			
			if(rightIsStr == 1 || leftIsStr == 1) //compares strings
			{
				if (strcmp(strip(left -> value), strip(right -> value)) < 0)
				{
					ptr -> next = left;
					ptr = ptr -> next;
					left = left -> next;
				}
				else
				{
					ptr -> next = right;
					ptr = ptr -> next;
					right = right -> next;
				}
			}
			else //compares numbers
			{
				double leftVal;
				double rightVal;
				sscanf(strip(left -> value), "%lf", &leftVal);
				sscanf(strip(right -> value), "%lf", &rightVal);
				if (leftVal < rightVal)
				{
					ptr -> next = left;
					ptr = ptr -> next;
					left = left -> next;
				}
				else
				{
					ptr -> next = right;
					ptr = ptr -> next;
					right = right -> next;
				}
			}
		}

	}
	return root;
}

//the part that separates all elements of merge sort
struct head* sort(struct head* root, int size, int position)
{
	if (root -> next == NULL)
	{
		return root;
	}
	int i = 0;
	//splits the list into two elements down the middle
	struct head* ptr = root;
	struct head* mid = root -> next;
	int middle = size / 2;
	//splits the linked list into two separate lists down the middle
	for(i; i < middle - 1; i++)
	{
		ptr = mid;
		mid = mid -> next;
	}
	ptr -> next = NULL;
	//recursive call included in the merge call
	struct head* sorted = merge(sort(root, middle, position), sort(mid, size - middle, position), position);
	return(sorted);
}