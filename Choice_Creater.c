#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main()
{
    char DIR[255];
    char Entered_KEY;
    char P[255], C1[255], C2[255];
    int People, Treasury, Court;
    int People1, Treasury1, Court1;

    FILE* fp = fopen("Data\\CHOICES.txt", "r+");
    while(fgets(DIR, 255, fp)!=NULL)
        printf("%s", DIR);
    int i = atoi(&DIR[1]);

    printf("\n");

    while (strcmpi(&Entered_KEY, "n") != 0)
    {
        char Choice;
        printf("Do you want to create a new choice file?(Y\\N)\n");
        scanf("%c", &Choice);
        if (Choice == 'Y' || Choice == 'y')
        {
            getchar();
            printf("Problem:\n");
            gets(P);
            printf("Choice 1:\n");
            gets(C1);
            printf("People:\n");
            scanf("%d",&People);
            printf("Treasury:\n");
            scanf("%d",&Treasury);
            printf("Court:\n");
            scanf("%d",&Court);
            getchar();
            printf("Choice 2:\n");
            gets(C2);
            printf("People:\n");
            scanf("%d",&People1);
            printf("Treasury:\n");
            scanf("%d",&Treasury1);
            printf("Court:\n");
            scanf("%d",&Court1);
            getchar();
            i++;
            char NDIR[255] = "Data\\";
            char str[100];
            char D[255] = "c";
            sprintf(str, "%d", i);
            strcat(D, str);
            strcat(D, ".txt");
            strcat(NDIR, D);
            fprintf(fp, "\n%s", D);
            FILE* ff = fopen(NDIR, "w");
            fprintf(ff,"%s\n",P);
            fprintf(ff,"%s\n",C1);
            fprintf(ff,"%d\n",People);
            fprintf(ff,"%d\n",Treasury);
            fprintf(ff,"%d\n",Court);
            fprintf(ff,"%s\n",C2);
            fprintf(ff,"%d\n",People1);
            fprintf(ff,"%d\n",Treasury1);
            fprintf(ff,"%d\n",Court1);
            fclose(ff);
        }
        else break;
    }
    fclose(fp);
}
