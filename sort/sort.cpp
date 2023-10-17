#include <stdio.h>

using namespace std;

/*冒泡排序*/
void bubble_sort(int a[], int len)
{
    int i, j;
    int k;
    for (i = 0; i < len; i++)
    {
        for (j = 0; j < len - 1 - i; j++)
        {
            if (a[j] > a[j + 1])
            {
                k = a[j];
                a[j] = a[j + 1];
                a[j + 1] = k;
            }
        }
    }
}

/*插入排序*/
void insert_sort(int a[], int len)
{
    int i, j;
    int key;
    for (i = 1; i < len; i++)
    {
        j = i - 1;
        key = a[i];
        while (j >= 0 && a[j] > key)
        {

            a[j + 1] = a[j];
            j--;
        }
        a[j + 1] = key;
    }
}

/*选择排序*/
void select_sort(int a[], int len)
{
    int i, j;
    int min;
    int k;
    for (i = 0; i < len - 1; i++)
    {
        min = i;
        for (j = i + 1; j < len; j++)
        {
            if (a[min] > a[j])
            {
                min = j;
            }
        }
        k = a[i];
        a[i] = a[min];
        a[min] = k;
    }
}

/*希尔排序*/
void shell_sort(int arr[], int len)
{
    int gap, i, j;
    int temp;
    for (gap = len >> 1; gap > 0; gap >>= 1)
        for (i = gap; i < len; i++)
        {
            temp = arr[i];
            printf("arr[j] = %d \n", arr[i - gap]);
            printf("gap = %d \n", gap);
            printf("temp = arr[i] = %d \n", temp);
            printf("i = gap = %d : j = i-gap = %d\n", i, i - gap);
            for (j = i - gap; j >= 0 && arr[j] > temp; j -= gap)
            {
                printf("++++++++++\n");
                printf("in j = %d\n", j);
                printf("arr[j + gap] = arr[j] : %d = %d\n", arr[j + gap], arr[j]);
                arr[j + gap] = arr[j];
                for (int c = 0; c < len; c++)
                {
                    printf("%d ", arr[c]);
                }
                printf("\ntemp = %d",temp);
                printf("\n++++++++++\n");
            }
            printf("%d %d\n",j,gap);
            arr[j + gap] = temp;
            for (int c = 0; c < len; c++)
            {
                printf("%d ", arr[c]);
            }
            printf("\n--------------------------------------\n");
        }
}

int main()
{
    int a[] = {4, 2, 5, 3, 1};
    int len = sizeof(a) / sizeof(a[0]);
    for (int i = 0; i < len; i++)
    {
        printf("%d ", a[i]);
    }
    printf("\n--------------------------------------\n");
    // bubble_sort(a, len);
    // select_sort(a,len);
    // insert_sort(a, len);
    shell_sort(a, len);
    for (int i = 0; i < len; i++)
    {
        printf("%d ", a[i]);
    }
    printf("\n");
    return 0;
}