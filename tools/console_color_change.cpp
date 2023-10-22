#include <iostream>
#include <stdio.h>

#define ESC "\033"
#define RESET "\033[0m"

int main()
{
    int step = 30;
    int red, green, blue;

    red = 255;
    green = 0;
    blue = 0;
    printf(ESC "[38;2;%d;%d;%dm", red, green, blue);
    printf(" ____________________________________________________\n");

    red -= step;
    green += step;
    blue = 0;
    printf(ESC "[38;2;%d;%d;%dm", red, green, blue);
    printf("|                 .__                                |\n");

    red -= step;
    green += step;
    blue = 0;
    printf(ESC "[38;2;%d;%d;%dm", red, green, blue);
    printf("|   ___  ________ |  | _____  ___.__. ___________    |\n" RESET);

    red -= step;
    green += step;
    blue = 0;
    printf(ESC "[38;2;%d;%d;%dm", red, green, blue);
    printf("|   \\  \\/ /\\____ \\|  | \\__  \\<   |  |/ __ \\_  __ \\   |\n");

    red -= step;
    green += step;
    blue = 0;
    printf(ESC "[38;2;%d;%d;%dm", red, green, blue);
    printf("|    \\   / |  |_> >  |__/ __ \\\\___  \\  ___/|  | \\/   |\n" RESET);

    red -= step;
    green += step;
    blue = 0;
    printf(ESC "[38;2;%d;%d;%dm", red, green, blue);
    printf("|     \\_/  |   __/|____(____  / ____|\\___  >__|      |\n");

    red -= step;
    green += step;
    blue = 0;
    printf(ESC "[38;2;%d;%d;%dm", red, green, blue);
    printf("|          |__|             \\/\\/         \\/          |\n" RESET);

    red -= step;
    green += step;
    blue = 0;
    printf(ESC "[38;2;%d;%d;%dm", red, green, blue);
    printf("|____________________________________________________|\n" RESET);

    return 0;
}
