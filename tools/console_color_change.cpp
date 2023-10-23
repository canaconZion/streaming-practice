#include <iostream>
#include <stdio.h>
#include <stdio.h>

#define ESC "\033"
#define RESET "\033[0m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RED "\033[31m"
#define BLUE "\033[34m"
#define RESET "\033[0m"
int main()
{
    printf("__________________________________________________\n");
    printf(RED "              .__\n");
    printf("___  ________ |  | _____  ___.__. ___________ \n" RESET);
    printf(YELLOW "\\  \\/ /\\____ \\|  | \\__  \\<   |  |/ __ \\_  __ \\\n");
    printf(" \\   / |  |_> >  |__/ __ \\\\___  \\  ___/|  | \\/\n" RESET);
    printf(GREEN "  \\_/  |   __/|____(____  / ____|\\___  >__| \n");
    printf("       |__|             \\/\\/         \\/  \n" RESET);
    printf("__________________________________________________\n\n\n");
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
    printf("|____________________________________________________|\n\n\n" RESET);

    printf(" ____________________________________________________\n");
    printf("|                 .__                                |\n");
    printf("|   ___  ________ |  | _____  ___.__. ___________    |\n");
    printf("|   \\  \\/ /\\____ \\|  | \\__  \\<   |  |/ __ \\_  __ \\   |\n");
    printf("|    \\   / |  |_> >  |__/ __ \\\\___  \\  ___/|  | \\/   |\n");
    printf("|     \\_/  |   __/|____(____  / ____|\\___  >__|      |\n");
    printf("|          |__|             \\/\\/         \\/          |\n");
    printf("|____________________________________________________|\n");

    return 0;
}
