struct head
{
	struct node* row;
	int index;
	struct head* next;
	char* value;
};

struct node 
{
	struct node* next;
	int pos;
	char* value;
};

struct node* createNode(){
 struct node* newNode = (struct node*)malloc(sizeof(struct node));

 newNode->next = NULL;
 return newNode;
}

//the merging part of merge sort
struct head* merge(struct head* rootL, struct head* rootR, int position)
{
	struct head* left = rootL;
	struct head* right = rootR;
	struct head* root;
	
	struct node* ptrLeft = rootL -> row;
	struct node* ptrRight = rootR -> row;
	
//	while(ptrLeft -> pos != position)
//	{
//		ptrLeft = ptrLeft -> next;
//		ptrRight = ptrRight -> next;
//	}
//	leftVal = ptrLeft -> value;
//	rightVal = ptrRight -> value;
	
	//checks if the two elements should be checked numerically or lexicographically
	int leftIsStr = 0;
	int rightIsStr = 0;
	//goes through each element of the string to see if it is a digit
	for(i; i < leftLength; i++)
	{
		if(isdigit(s1[i]) == 0 && isdigit(s1[i]) != '.' && isdigit(s1[i]) != 'E' && isdigit(s1[i]) != '+' && isdigit(s1[i]) != '-')
		{
			leftIsStr = 1;
		}				
	}
		
	i = 0;
		
	for(i; i < rightLength; i++)
	{
		if(isdigit(s1[i]) == 0 && isdigit(s2[i]) != '.' && isdigit(s2[i]) != 'E' && isdigit(s2[i]) != '+' && isdigit(s2[i]) != '-')
		{
			rightIsStr = 1;
		}				
	}
	
	//creates the head of the new sorted linked list
	if(rightIsStr == 1 || leftIsStr == 1)
	{
		//str compares if either element is a string
		if (strcmp(left -> value, right -> value) < 0)
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
		sscanf(left -> value, "%lf", &leftVal);
		sscanf(right -> value, "%lf", &rightVal);
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
	while(left != NULL && right != NULL)
	{
		char* s1 = left -> value;
		char* s2 = right -> value;
		
		leftIsStr = 0;
		rightIsStr = 0;
		
		int i = 0;
		
		int leftLength = strlen(s1);
		int rightLength = strlen(s2);
		
		//the same process from before to check if either is a string
		for(i; i < leftLength; i++)
		{
			if(isdigit(s1[i]) == 0 && isdigit(s1[i]) != '.' && isdigit(s1[i]) != 'E' && isdigit(s1[i]) != '+' && isdigit(s1[i]) != '-')
			{
				leftIsStr = 1;
			}				
		}
		
		i = 0;
		
		for(i; i < rightLength; i++)
		{
			if(isdigit(s1[i]) == 0 && isdigit(s2[i]) != '.' && isdigit(s2[i]) != 'E' && isdigit(s2[i]) != '+' && isdigit(s2[i]) != '-')
			{
				rightIsStr = 1;
			}				
		}
		//the same process from before when creating the head of the new linked list
		if(rightIsStr == 1 || leftIsStr == 1)
		{
			if (strcmp(left -> value, right -> value) < 0)
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
		else
		{
			double leftVal;
			double rightVal;
			sscanf(left -> value, "%lf", &leftVal);
			sscanf(right -> value, "%lf", &rightVal);
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
	
	//if there are no nodes left in one of the elements, just adds the rest of the remaining element to the end
	if(left == NULL && right != NULL)
	{
		ptr -> next = right;
	}
	else if(left != NULL && right == NULL)
	{
		ptr -> next = left;
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
	struct head* ptr = root;
	struct head* mid = root -> next;
	int middle = size / 2;
	//splits the linked list into two separate lists down the middle
	for(i; i < middle; i++)
	{
		ptr = mid;
		mid = mid -> next;
	}
	ptr -> next = NULL;
	
	struct head* sorted = merge(sort(root, middle, position), sort(mid, size - middle, position), position);
	return(sorted);
}