int A[100000];
int B[100000];
int C[100000];

int main()
{
    int A_C;
    int A_R;
    int B_C;
    int B_R;
    scanf("%d %d\n",&A_C,&A_R);
    int i=0;
    int j=0;
    int k=0;
    while(i<A_C)
    {
        j=0;
        while(j<A_R)
        {
            scanf("%d",&A[i*A_R]+j);
            j++;
        }
        i++;
    }
    scanf("%d %d",&B_C,&B_R);
    if(A_C!=B_R)
    {
        printf("Incompatible Dimensions");
        return 0;
    }
    i=0;
    while(i<B_C)
    {
        j=0;
        while(j<B_R)
        {
            scanf("%d",&B[i*B_R+j]);
            j++;
        }
        i++;
    }
    i=0;
    while(i<A_C)
    {
        j=0;
        while(j<A_R)
        {
            C[i*A_R+j]=0;
            k=0;
            while(k<A_R)    //存疑
            {
                C[i*B_R+j]+=A[i*A_R+k]*B[k*B_R+j];
                k++;
            }
            j++;
        }
        i++;
    }
    i=0;
    while(i<A_C)
    {
        j=0;
        while(j<B_R)
        {
            printf("%10d",C[i*B_R+j]);
            j++;
        }
        printf("\n");
        i++;
    }
    return 0;
}