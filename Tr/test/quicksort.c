void quicksort(int A[],int left,int right)
{
    int i,j,mid;
    int temp;
    i=left;
    j=right;
    mid=A[(left+right)/2];
    while(i<=j)
    {
        while(A[i]<mid)
        {
            i++;
        }
        while(mid<A[j])
        {
            j--;
        }
        if(i<=j)
        {
            temp=A[i];
            A[i]=A[j];
            A[j]=temp;
            i++;
            j--;
        }
    }
    if(left<j) quicksort(A,left,j);
    if(i<right) quicksort(A,i,right);
    return;
}