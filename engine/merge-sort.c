#include <stdio.h>
#include <stdlib.h>

#define MAGIC_NUMBER 42
#define ARRAY_MAX 30000

void insertion_sort(int *a, int n){
   int p, i,key;
   for (p = 1; p < n; p++){
      key = a[p];
      i = p-1;
      while(i>=0 && a[i]>key){
         a[i+1]=a[i];
         i--;
      }
      a[i + 1] = key;
   }
}

void merge_sort(int *a, int *w, int n) {
   int i = 0, x = 0, y = n/2;

   if (n < 2)
      return;

   if (n < MAGIC_NUMBER) {
      insertion_sort(a, n);
   } else {
      merge_sort(a, w, n / 2);
      merge_sort(a+n/2, w+n/2, n - (n/2));
   }
   
   while (i < n && x < n/2 && y < n)
      w[i++] = a[x] < a[y] ? a[x++] : a[y++];
   
   while (x < n/2)
      w[i++] = a[x++];
   
   while (y < n)
      w[i++] = a[y++];
   
   for (i = 0; i < n; i ++)
      a[i] = w[i];
}

/* Usage example:
int main(void){
   int my_array[ARRAY_MAX];
   int work_array[ARRAY_MAX];
   clock_t start, end;
   int i, count = 0;

   while(count < ARRAY_MAX && 1 == scanf("%d", &my_array[count]))
      count++;
   

   start = clock();
   merge_sort(my_array, work_array, count);
   end = clock();
   
   for(i = 0; i < count; i++)
      printf("%d\n", my_array[i]);
   

   fprintf(stderr, "%d %f\n", count, (end - start) / (double) CLOCKS_PER_SEC);
   return EXIT_SUCCESS;

}
*/
