struct node 
{
	struct node* next;
	int pos;
	char* value;
}

struct node* sort(struct node* root, int size, int position)
{
	if (root -> next == NULL)
	{
		return root;
	}
	struct node* ptr = root
	struct node* mid = root -> next;
	int middle = size / 2;
	int i = 1;
	for(i; i < middle; i++)
	{
		ptr = mid;
		mid = mid -> next;
	}
	ptr -> next = NULL;
	return(merge(sort(root, middle), sort(mid, size - mid)), position);
}

struct node* merge(struct node* first, struct node* second, int position)
{
	struct node* left = first;
	struct node* right = second;
	struct node* root;
	char* leftVal;
	char* rightVal;
	int i = 0;
	for(i ; i < position; i++)
	{
		
	}
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
	struct node* ptr = root;
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
	if(left != NULL && right == NULL)
	{
		ptr -> next = left;
	}
	return root;
}
