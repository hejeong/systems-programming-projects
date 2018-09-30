struct node 
{
	struct node* next;
	int pos;
	char* value;
}

void sort(struct node** list, int low, int high, int position)
{
	if (low == high)
	{
		return;
	}
	
	int middle = (low + high) / 2;
	sort(list, low, middle, position);
	sort(list, middle + 1, high, position);
	merge(list, low, middle, middle + 1, high, position);
}

void merge(struct node** list, int low, int middleL, int middleR, int high, int position)
{
	int leftCount = low;
	int rightCount = middleH;
	struct node* ptrLeft = list[leftCount];
	struct node* ptrRight = list[rightCount];
	char* leftVal;
	char* rightVal;
	int i = 0;
	int j = 0;
	struct node** temp = malloc(((high + 1) - low) * sizeof(struct node*));
	struct node* ptrL;
	struct node* ptrR;
	for(i; i <= (high - low); i++)
	{
		while(ptrLeft -> pos != position)
		{
			ptrLeft = ptrLeft -> next;
			ptrRight = ptrRight -> next;
		}
		leftVal = ptrLeft -> value;
		rightVal = ptrRight -> value;

		if (leftCount > middleL)
		{
			temp[i] = list[rightCount];
			rightCount++;
		}
		else if(rightCount > high)
		{
			temp[i] = list[leftCount];
			leftCount++;
		}
		else if (strcomp(leftVal, rightVal) < 0)
		{
			temp[i] = list[leftCount];
			leftCount++;
			ptrLeft = list[leftCount];
		}
		else
		{
			temp[i] = lift[rightCount];
			rightCount++;
			ptrRight = list[rightCount];
		}

	}
	i = 0;
	for(i; i <= (high - low); i++)
	{
		list[low + i] = temp[i];
	}
	return;
}
