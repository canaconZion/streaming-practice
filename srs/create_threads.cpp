#include <stdio.h>
#include <st.h>
#include <string>

void *do_calc(void *arg)
{
    int sleep_ms = (int)(long int)(char *)arg * 10;

    for (;;)
    {
        printf("in sthread #%dms\n", sleep_ms);
        st_usleep(sleep_ms * 1000);
    }

    return NULL;
}

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        printf("Test the concurrence of state-threads!\n"
               "Usage: %s <sthread_count>\n"
               "eg. %s 10000\n",
               argv[0], argv[0]);
        return -1;
    }

    if (st_init() < 0)
    {
        printf("error!");
        return -1;
    }

    int i;
    int count = std::stoi(argv[1]);
    for (i = 1; i <= count; i++)
    {
        if (st_thread_create(do_calc, (void *)i, 0, 0) == NULL)
        {
            printf("error!");
            return -1;
        }
    }

    st_thread_exit(NULL);

    return 0;
}