struct head
{
	struct node* row;
	int index;
	struct head* next;
	char* value;
}

struct node 
{
	struct node* next;
	int pos;
	char* value;
}

struct head* sort(struct head* root, int size, int position)
{
	if (root -> next == NULL)
	{
		return root;
	}
	
	struct node* ptr = root
	struct node* mid = root -> next;
	int middle = size / 2;
	for(i; i < middle; i++)
	{
		ptr = mid;
		mid = mid -> next;
	}
	ptr -> next = NULL;
	
	return(merge(sort(root, middle), sort(mid, size - mid)), position);
}

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
	
	if (strcomp(left -> value, right -> value) < 0)
	{
		root = left;
		left = left -> next;
	}
	else
	{
		root = right;
		right = right -> next;
	}
	
	struct head* ptr = root;
	
	while(left != NULL && right != NULL)
	{
		if (strcomp(left -> value, right -> value) < 0)
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
	if(left == NULL && right != NULL)
	{
		ptr -> next = right;
	}
	else if(left != NULL && right == NULL)
	{
		ptr -> next = left;
	}
	i = 0;
	return root;
}
