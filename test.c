#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "osqueue.h"
#include "threadPool.h"


void hello (void* a)
{
   printf("hello\n");
}

void myTest(void* a) {
    int* p_num = (int*)a;
    printf("myTest: %d\n",*p_num);
}


void test_thread_pool_sanity()
{
   int i;
   
   ThreadPool* tp = tpCreate(5);

   
   for(i=0; i<5; ++i)
   {
      tpInsertTask(tp,hello,NULL);
   }


   int x = 17;
    for(i=0; i<5; ++i)
    {
        tpInsertTask(tp,myTest,(void *)&x);
    }

   tpDestroy(tp,1);
}


int main()
{
   test_thread_pool_sanity();

   return 0;
}
