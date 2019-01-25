#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <math.h>
#include <time.h>

/* THE KINGS GAME :: FINAL PROJJECT
    Written by Roham Zendehdel Nobari at <5:17:29 AM 12/31/2018> for the first time
    © ALL RIGHTS RESERVERED
*/

char *CHOICEDIR = "Data\\CHOICES.txt";
char *TOPDIR = "Data\\Kings.bin";
char *SAVEDIR = "Data\\Save\\";

/*______________________________________________________________
   Initializing a structure for choices from CHOICES.txt
  ______________________________________________________________ */

struct choice
{
    char Problem[255];
    char Choice[2][255];
    int People[2], Treasury[2], Court[2];
    int Probability;
};

/*______________________________________________________________
   Initializing a structure for a saved choice.
  ______________________________________________________________ */

struct Saved_Choices
{
    char Problem[255];
    char Choice[2][255];
    int Chosen;
    int People, Treasury, Court;
};

/*______________________________________________________________
   Initializing a structure for game status.
  ______________________________________________________________ */

struct Saved_Data
{
    char King_Name[255];
    int State;
    int People, Treasury, Court;
    int Auto_Save;
};

/*______________________________________________________________
   Initializing a structure for problem link_lists
  ______________________________________________________________ */

struct node
{
    struct choice Value;
    struct node* Prev;
    struct node* Next;
};

/*______________________________________________________________
   Node creator function ( link_list )
  ______________________________________________________________ */

struct node* create_node(struct choice C)
{
    struct node* nn = (struct node*)malloc(sizeof(struct node));
    nn->Prev = NULL;
    nn->Next = NULL;
    nn->Value = C;
    return nn;
}

/*______________________________________________________________
   Adding Choices from the end of link_list
  ______________________________________________________________ */

void push_back(struct node** list, struct choice C)
{
    struct node* pt = *list;

    if (*list == NULL)
    {
        *list = create_node(C);
        return;
    }
    while(pt->Next != NULL)
        pt = pt->Next;

    pt->Next = create_node(C);
    pt->Next->Prev = pt;
}

/*______________________________________________________________
   Fetching the length of list
  ______________________________________________________________ */

int List_Length(struct node* list)
{
    int Counter = 0;
    struct node* pt = list;
    while (pt != NULL)
    {
        pt = pt->Next;
        Counter++;
    }
    return Counter;
}

/*______________________________________________________________
   Deleting a specified node from the link_list
  ______________________________________________________________ */

void Delete_Problem(struct node** list, struct node* Problem)
{
    struct node* Prev = (Problem)->Prev;
    struct node* Next = (Problem)->Next;

    if (Prev != NULL)
    {
        Prev->Next = Next;
        if (Next != NULL)
            Next->Prev = Prev;
        free(Problem);
    }
    else
    {
        struct node* temp = *list;
        *list = (*list)->Next;
        if (*list != NULL)
            (*list)->Prev = NULL;
        free(temp);
    }
}

/*______________________________________________________________
   Deleting the link_list
  ______________________________________________________________ */

void Delete(struct node** list)
{
    struct node* pt = (*list)->Next;
    while (pt->Next != NULL)
    {
        struct node* temp = pt;
        pt = pt->Next;
        free(temp);
    }
    free(*list);
}

/*______________________________________________________________
   Reading problems from their respective files
  ______________________________________________________________ */

struct choice Read(FILE *fp)
{
    struct choice C;
    fgets(C.Problem, 255, fp);
    for (int i = 0; i < 2; i++)
    {
        fgets(C.Choice[i], 255, fp);
        if (C.Choice[i][strlen(C.Choice[i])-1] == '\n')
            C.Choice[i][strlen(C.Choice[i])-1] = '\0';
        fscanf(fp, "%d", &C.People[i]);
        fscanf(fp, "%d", &C.Court[i]);
        fscanf(fp, "%d", &C.Treasury[i]);
        fgetc(fp);
    }
    C.Probability = 3;
    return C;
}

/*______________________________________________________________
   Initializing the link_list with it's problems
  ______________________________________________________________ */

int Initiliaze_Choices(struct node** list)
{
    FILE *FCHOICE = fopen(CHOICEDIR, "r");
    if (FCHOICE == NULL)
    {
        printf("Error: Function %s - Cannot open file %s!\n", __func__, CHOICEDIR);
        return 0;
    }
    char DIR[255];
    int n = 0;
    while (fgets(DIR, 255, FCHOICE)!=NULL)
    {
        char NDIR[255] = "Data\\";
        if (DIR[strlen(DIR)-1] == '\n')
           DIR[strlen(DIR)-1] = '\0';
        strcat(NDIR,DIR);
        FILE* fp = fopen(NDIR,"r");
        if (fp == NULL)
        {
            printf("Error: Function %s - Cannot open file %s!\n", __func__, NDIR);
            return 0;
        }
        struct choice Choice = Read(fp);
        push_back(list, Choice);
        fclose(fp);
        n++;
    }
    fclose(FCHOICE);
    return n;
}

/*______________________________________________________________
   Checking if the game is lost or not
  ______________________________________________________________ */

int Loss_Check(int KPeople, int KTreasury, int KCourt)
{
    if (KPeople <= 0 || KTreasury <= 0 || KCourt <= 0)
        return 1;
    else
    {
        if ( ((KPeople + KTreasury + KCourt) * 1.0 / 10) < 10.0)
            return 1;
    }
    return 0;
}

/*______________________________________________________________
   Selecting a random node(problem) based on their probability
  ______________________________________________________________ */

struct node* Random_Problem(struct node** list, int Choice_Count)
{
    int Counter = 0 , Random_Prob = 0;
    for (int i = 3; i >= 1; i--)
    {
        struct node* pt = *list;
        struct node* Equal_Probability[Choice_Count];
        while (pt != NULL)
        {
            if (pt->Value.Probability == i)
            {
                Equal_Probability[Counter] = pt;
                Counter++;
            }
            pt = pt->Next;
        }
        if (Counter > 1)
            Random_Prob = rand() % Counter;
        else if (Counter == 1)
            Random_Prob = 0;
        else
            continue;
        return Equal_Probability[Random_Prob];
    }
}

/*______________________________________________________________
   Printing the given node(problem)
  ______________________________________________________________ */

void Print_Problem(struct node* Problem)
{
    printf("\n%s\n",Problem->Value.Problem);
    for (int i = 0 ; i < 2 ; i++)
        printf("[%d]%s\n", i + 1, Problem->Value.Choice[i]);
}

/*_______________________________________________________________________________________________________________
   Adding the answered problem to the dynamic array S_C for saving the previous decisions of the king
  _______________________________________________________________________________________________________________ */

void Save_Problem(struct node* Problem, struct Saved_Choices** S_C, int* Saved_Choices_Count, int Chosen, int People, int Treasury, int Court)
{
    if (Saved_Choices_Count == 0)
    {
        *Saved_Choices_Count += 1;
        (*S_C) = (struct Saved_Choices*)malloc(sizeof(struct Saved_Choices));
        strcpy((*S_C)[0].Problem, Problem->Value.Problem);
        strcpy((*S_C)[0].Choice[0], Problem->Value.Choice[0]);
        strcpy((*S_C)[0].Choice[1], Problem->Value.Choice[1]);
        (*S_C)[0].Chosen = Chosen;
        (*S_C)[0].People = People;
        (*S_C)[0].Court = Court;
        (*S_C)[0].Treasury = Treasury;
    }
    else
    {
        *Saved_Choices_Count += 1;
        int n = *Saved_Choices_Count;
        (*S_C) = (struct Saved_Choices*)realloc(*S_C, n * sizeof(struct Saved_Choices));
        strcpy((*S_C)[n-1].Problem, Problem->Value.Problem);
        strcpy((*S_C)[n-1].Choice[0], Problem->Value.Choice[0]);
        strcpy((*S_C)[n-1].Choice[1], Problem->Value.Choice[1]);
        (*S_C)[n-1].Chosen = Chosen;
        (*S_C)[n-1].People = People;
        (*S_C)[n-1].Court = Court;
        (*S_C)[n-1].Treasury = Treasury;
    }
}

/*______________________________________________________________
   Compare function used for finding the top 10 kings
  ______________________________________________________________ */

int KING_CMP(const void* a, const void* b)
{
    struct Saved_Data* S_D_A = (struct Saved_Data*)a;
    struct Saved_Data* S_D_B = (struct Saved_Data*)b;
    if ((S_D_A->People + S_D_A->Court + S_D_A->Treasury) > (S_D_B->People + S_D_B->Treasury + S_D_B->Court))
        return 1;
    else if((S_D_A->People + S_D_A->Court + S_D_A->Treasury) < (S_D_B->People + S_D_B->Treasury + S_D_B->Court))
        return -1;
    else
    {
        if (S_D_A->People > S_D_B->People)
            return 1;
        else if (S_D_A->People < S_D_B->People)
            return -1;
        else
        {
            if (S_D_A->Court > S_D_B->Court)
                return 1;
            else if (S_D_A->Court < S_D_B->Court)
                return -1;
            else
            {
                if (S_D_A->Treasury > S_D_B->Treasury)
                    return 1;
                else if (S_D_A->Treasury < S_D_B->Treasury)
                    return -1;
                else
                    return 0;
            }
        }
    }
}

/*______________________________________________________________
   Printing the top 10 Kings
  ______________________________________________________________ */

void PRINT_TOP()
{
    char KING_DIR[255];
    struct Saved_Data* S_D = NULL;
    int King_Count = 0;

    FILE* FK = fopen(TOPDIR, "rb+");
    if (FK == NULL)
    {
        printf("Error: Function %s - Cannot open file %s!\n", __func__, TOPDIR);
        return;
    }
    while(fread(KING_DIR, sizeof(KING_DIR), 1, FK) == 1)
    {
        FILE* SAVE = fopen(KING_DIR, "rb");
        if (SAVE == NULL)
        {
            printf("Error: Function %s - Cannot open file %s!\n", __func__, KING_DIR);
            return;
        }
        if (King_Count == 0)
        {
            S_D = (struct Saved_Data*)malloc(sizeof(struct Saved_Data));
            King_Count++;
        }
        else
        {
            King_Count++;
            S_D = (struct Saved_Data*)realloc(S_D, King_Count * sizeof(struct Saved_Data));
        }
        fread(&S_D[King_Count - 1], sizeof(struct Saved_Data), 1, SAVE);
        fclose(SAVE);
    }
    fclose(FK);
    qsort(S_D, King_Count, sizeof(struct Saved_Data), KING_CMP);
    int Counter = 0;
    for (int i = King_Count - 1 ; i >= 0; i--)
    {
        Counter++;
        printf("[%d] %s\nPeople: %d Court: %d Treasury: %d Sum: %d\n\n", Counter,
                S_D[i].King_Name, S_D[i].People, S_D[i].Court, S_D[i].Treasury, S_D[i].People + S_D[i].Court + S_D[i].Treasury);
        if (Counter == 10)
            break;
    }
}

/*______________________________________________________________
   Finding if the king has had a previous game or not
  ______________________________________________________________ */

void Find_King(struct Saved_Data S_D)
{
    char KING_NAME[255];

    char SAVE_DIRECTORY[255] = "Data\\Save\\";
    strcat(S_D.King_Name,".bin");
    strcat(SAVE_DIRECTORY, S_D.King_Name);

    FILE* FK = fopen(TOPDIR, "rb+");
    if (FK == NULL)
        FK = fopen(TOPDIR, "wb+");

    while(fread(KING_NAME, sizeof(KING_NAME), 1, FK) == 1)
        if (strcmp(KING_NAME, SAVE_DIRECTORY) == 0)
            return;

    fseek(FK, 0, SEEK_END);
    fwrite(SAVE_DIRECTORY, sizeof(SAVE_DIRECTORY), 1, FK);
    fclose(FK);
}

/*______________________________________________________________________________________________
   Saving the previous decisions of the king and the game status in a bin file
  ______________________________________________________________________________________________ */

void Save_Data(struct node* list, int list_size, int Probabilities[list_size], struct Saved_Choices* S_C, int Saved_Choices_Count, char King_Name[], int KPeople, int KTreasury,
               int KCourt, int State, int Auto)
{
    char DIR[255];

    strcpy(DIR, "Data\\Save\\");

    struct Saved_Data S_D;
    struct node* pt = list;
    strcpy(S_D.King_Name, King_Name);
    S_D.People = KPeople;
    S_D.Treasury = KTreasury;
    S_D.Court = KCourt;
    S_D.State = State;
    S_D.Auto_Save = Auto;
    strcat(DIR, S_D.King_Name);
    strcat(DIR, ".bin");
    FILE* F = fopen(DIR,"wb");
    fwrite(&S_D, sizeof(S_D), 1, F);
    fwrite(&list_size, sizeof(int), 1, F);
    fwrite(Probabilities, sizeof(int), list_size, F);
    fwrite(&Saved_Choices_Count, sizeof(int), 1, F);
    fwrite(S_C, sizeof(struct Saved_Choices), Saved_Choices_Count, F);
    fclose(F);
    Find_King(S_D);
}

/*___________________________________________________________________________________________________________________________________________________
   Loading the previous game of the king and resuming it ( returning -1 if there is no such file / returning -3 if the file was in auto_save status )
  ___________________________________________________________________________________________________________________________________________________ */

int Load_Data(char King_Name[255], struct node** list, int list_size, int Probabilities[list_size], struct Saved_Choices** S_C, int *Saved_Choices_Count, int *KPeople,
              int *KTreasury, int *KCourt, int *State, int Condition)
{
    char DIR[255];

    strcpy(DIR, "Data\\Save\\");

    strcat(DIR, King_Name);
    strcat(DIR, ".bin");
    FILE* F = fopen(DIR,"rb");
    if (F == NULL)
        return -2;
    else
    {
        struct Saved_Data S;
        int SCC;
        int L_S;
        struct node* pt = *list;
        fread(&S, sizeof(S), 1, F);
        if (S.State == -1 && Condition == 0)
            return -1;
        if (S.Auto_Save == 1 && Condition == 0)
            return -3;
        fread(&L_S, sizeof(int), 1, F);
        int Prob[L_S];
        fread(Prob, sizeof(int), L_S, F);
        fread(&SCC, sizeof(int), 1, F);
        (*S_C) = (struct Saved_Choices*)malloc(SCC * sizeof(struct Saved_Choices));
        fread(*S_C, sizeof(struct Saved_Choices), SCC, F);
        *KPeople = S.People;
        *KTreasury = S.Treasury;
        *KCourt = S.Court;
        *Saved_Choices_Count = SCC;
        *State = S.State;
        for (int i = 0 ; i < L_S; i++)
            Probabilities[i] = Prob[i];
    }
    fclose(F);
    return 0;
}

/*__________________________________________________________________________________________
   Printing the previous decisions that the king had made in it's saved game
  __________________________________________________________________________________________ */

void Print_Previous_Choices(struct Saved_Choices *S_C, int Saved_Choices_Count)
{
    printf("\n=================================\nPeople: 50 Treasury: 50 Court: 50\n=================================\n");
    for (int i = 0 ; i < Saved_Choices_Count-1; i++)
    {
        printf("%s\n", S_C[i].Problem);
        printf("[1]%s\n", S_C[i].Choice[0]);
        printf("[2]%s\n", S_C[i].Choice[1]);
        printf("Decision = %d\n", S_C[i].Chosen);
        printf("\n\n\n");
        printf("=================================\nPeople: %d Treasury: %d Court: %d\n=================================\n",
        S_C[i].People, S_C[i].Treasury, S_C[i].Court);

    }
}

/*______________________________________________________________
   Find the random problem in the list
  ______________________________________________________________ */

struct node* Find_Problem(struct node** list, struct Saved_Choices* S_C, int Saved_Choices_Count)
{
    struct node* pt = *list;
    while (pt != NULL)
    {
        if (strcmp(pt->Value.Problem, S_C[Saved_Choices_Count-1].Problem) == 0)
        {
            return pt;
        }
        else
            pt = pt->Next;
    }
    return NULL;
}

/*______________________________________________________________
   Updating the probabilities of the nodes
  ______________________________________________________________ */

void Update_Probabilities(struct node* list, int list_size, int Probabilities[list_size])
{
    struct node* pt = list;
    for (int i = 0 ; i < list_size; i++)
    {
        if (Probabilities[i] != 0)
        {
            Probabilities[i] = pt->Value.Probability;
            pt = pt->Next;
        }
    }
}

/*________________________________________________________________________________________________________
   Updating the probability of the list based on the loaded probabilities ( from the previous game )
  ________________________________________________________________________________________________________ */

void Update_List(struct node** list, int list_size, int Probabilities[list_size])
{
    struct node* pt = *list;
    int counter = 0;
    while(pt != NULL)
    {
        pt->Value.Probability = Probabilities[counter];
        if (Probabilities[counter] == 0)
            Delete_Problem(list, pt);
        pt = pt->Next;
        counter++;
    }
}

int main()
{
    //INITIALIZE THE LIST OF PROBLEMS
    struct node* list = NULL;
    int Choice_Count = Initiliaze_Choices(&list);
    int List_Size = List_Length(list);
    // LOADED_LIST
    struct node* list_L = NULL;
    int Choice_Count_L = Initiliaze_Choices(&list_L);

    int Probabilities[List_Size], Probabilities_L[List_Size];

    for (int i = 0 ; i < List_Size; i++)
        Probabilities[i] = 3;
    for (int i = 0 ; i < List_Size; i++)
        Probabilities_L[i] = 3;
    // END OF PROBLEM INITIALIZATION

    // LOAD_DATA INITIALIZATION
    struct Saved_Choices *S_C = NULL;
    int Saved_Choices_Count = 0;
    int KPeople = 50, KTreasury = 50, KCourt = 50;
    int First_Choice = 0;
    int State = 0;
    // TEMP LOAD_DATA
    struct Saved_Choices *S_C_L = NULL;
    int Saved_Choices_Count_L = 0;
    int KPeople_L , KTreasury_L , KCourt_L;
    int State_L = 0;
    // END OF LOAD INITIALIZATION

    char King_Name[255];
    int Load_Condition = 0;

    srand(time(0));
    printf("=========================\nWelcome to THE KINGS GAME\n=========================\n");
    printf("\nPlease enter your name:\n=> ");
    scanf("%s", King_Name);
    printf("\n1) New Game.\n");
    printf("2) Resume Game.\n");
    printf("3) Show High Scores.\n=> ");
    scanf("%d", &First_Choice);
    while (First_Choice != 1 && First_Choice != 2 && First_Choice != 3)
    {
        printf("Error: Invalid Response!\n=> ");
        scanf("%d", &First_Choice);
    }
    if (First_Choice == 3)
    {
        PRINT_TOP();
        getchar();
        printf("\nPRESS ANY KEY TO EXIT...!");
        getchar();
        free(S_C);
        free(S_C_L);
        Delete(&list);
        return 0;
    }
    if (First_Choice == 2)
    {
        if (Load_Data(King_Name, &list, List_Size, Probabilities, &S_C, &Saved_Choices_Count, &KPeople, &KTreasury, &KCourt, &State, 0) == -2)
        {
            int C;
            printf("You don't have a previous game...!\n1) New Game\n2) Exit\n=> ");
            scanf("%d", &C);
            if (C == 2)
            {
                // Freeing memory
                free(S_C);
                free(S_C_L);
                Delete(&list);
                return 0;
            }
        }
        else if (Load_Data(King_Name, &list, List_Size, Probabilities, &S_C, &Saved_Choices_Count, &KPeople, &KTreasury, &KCourt, &State, 0) == -1)
        {
            int C;
            printf("You lost your previous game...!\n1) New Game\n2) Show Details\n=> ");
            scanf("%d", &C);
            if (C == 2)
            {
                // Loading the data of the lost game
                Load_Data(King_Name, &list, List_Size, Probabilities, &S_C, &Saved_Choices_Count, &KPeople, &KTreasury, &KCourt, &State, 1);
                Print_Previous_Choices(S_C, Saved_Choices_Count);
            }
        }
        else if (Load_Data(King_Name, &list, List_Size, Probabilities, &S_C, &Saved_Choices_Count, &KPeople, &KTreasury, &KCourt, &State, 0) == -3)
        {
            int C;
            printf("You didn't save your previous progress...!\n1) New Game\n2) Resume from auto_save\n=> ");
            scanf("%d", &C);
            if (C == 2)
            {
                // Loading the data from the auto_saved progress
                Load_Condition = 1;
                Load_Data(King_Name, &list, List_Size, Probabilities, &S_C, &Saved_Choices_Count, &KPeople, &KTreasury, &KCourt, &State, 1);
                Load_Data(King_Name, &list_L, List_Size, Probabilities_L, &S_C_L, &Saved_Choices_Count_L, &KPeople_L, &KTreasury_L, &KCourt_L, &State_L, 1);
                Print_Previous_Choices(S_C, Saved_Choices_Count);
                Update_List(&list, List_Size, Probabilities);
                Update_List(&list_L, List_Size, Probabilities_L);
            }
        }
        else
        {
            // Loading the data from the demanded saved progress
            Load_Condition = 1;
            Load_Data(King_Name, &list_L, List_Size, Probabilities_L, &S_C_L, &Saved_Choices_Count_L, &KPeople_L, &KTreasury_L, &KCourt_L, &State_L, 1);
            Print_Previous_Choices(S_C, Saved_Choices_Count);
            Update_List(&list, List_Size, Probabilities);
            Update_List(&list_L, List_Size, Probabilities_L);
        }
    }

    while (Loss_Check(KPeople, KTreasury, KCourt) == 0 && State != -1)
    {
        // Auto_saving every step
        Save_Data(list, List_Size, Probabilities, S_C, Saved_Choices_Count, King_Name, KPeople, KTreasury, KCourt, 1, 1);
        struct node* Problem = NULL;
        int Choice = 0;
        if (State == 0) // Random problem creation
        {
            printf("\n=================================\nPeople: %d Treasury: %d Court: %d\n=================================\n", KPeople, KTreasury, KCourt);
            Problem = Random_Problem(&list, Choice_Count);
            Print_Problem(Problem);
        }
        else if (State == 1) // The last problem from the previous game that was not answered
        {
            Problem = Find_Problem(&list, S_C, Saved_Choices_Count);
            Print_Problem(Problem);
        }

        while(1)
        {
            printf("=> ");
            scanf("%d",&Choice);
            if (Choice == 1 || Choice == 2)
            {
                int P = KPeople + Problem->Value.People[Choice - 1];
                int T = KTreasury + Problem->Value.Treasury[Choice - 1];
                int C = KCourt + Problem->Value.Court[Choice - 1];
                if (P <= 100)
                    KPeople = P;
                else
                    KPeople = 100;
                if (T <= 100)
                    KTreasury = T;
                else
                    KTreasury = 100;
                if (C <= 100)
                    KCourt = C;
                else
                    KCourt = 100;

                break;
            }
            if (Choice == -1)
            {
                getchar();
                printf("Do you want to save your progress?(Y/N)\n=> ");
                char Choice;
                scanf("%c", &Choice);
                getchar();
                while (Choice != 'y' && Choice != 'Y' && Choice != 'n' && Choice != 'N')
                {
                    printf("\nError: Invalid Response!\n=> ");
                    scanf("%c", &Choice);
                    getchar();
                }
                if (Choice == 'y' || Choice == 'Y')
                {
                    // Saving the last problem
                    Save_Problem(Problem, &S_C, &Saved_Choices_Count, -1, KPeople, KTreasury, KCourt);
                    Save_Data(list, List_Size, Probabilities, S_C, Saved_Choices_Count, King_Name, KPeople, KTreasury, KCourt, 1, 0);
                    printf("Thank You For Playing!\n");
                    // Freeing memory
                    free(S_C);
                    Delete(&list);
                    printf("\nPRESS ANY KEY TO EXIT...!");
                    getchar();
                    return 0;
                }
                else if (Choice == 'n' || Choice == 'N')// Not wanting to save the progress including the auto saves -> saving from temp
                {
                    if (Load_Condition == 1)
                    Save_Data(list_L, List_Size, Probabilities_L, S_C_L, Saved_Choices_Count_L, King_Name, KPeople_L, KTreasury_L, KCourt_L, 1, 0);
                    // Freeing memory
                    free(S_C);
                    free(S_C_L);
                    Delete(&list);
                    printf("\nPRESS ANY KEY TO EXIT...!");
                    getchar();
                    return 0;
                }
            }
            else
            {
                printf("Error: Invalid Response!\n");
            }
        }
        if (State == 0) // Saving the problem in S_C
            Save_Problem(Problem, &S_C, &Saved_Choices_Count, Choice, KPeople, KTreasury, KCourt);
        else if (State == 1) // The last problem that was not answered.
        {
            S_C[Saved_Choices_Count-1].Chosen = Choice;
            S_C[Saved_Choices_Count-1].People = KPeople;
            S_C[Saved_Choices_Count-1].Treasury = KTreasury;
            S_C[Saved_Choices_Count-1].Court = KCourt;
            State = 0;
        }

        // Decreasing the probability of the random problem
        Problem->Value.Probability = (Problem->Value.Probability) - 1;
        Update_Probabilities(list, List_Size, Probabilities);

        if (Problem->Value.Probability == 0)
        {
            Delete_Problem(&list, Problem);
        }
        // Reinitializing the link_list
        if (List_Length(list) == 0)
        {
            Initiliaze_Choices(&list);
            for (int i = 0 ; i < List_Size; i++)
                Probabilities[i] = 3;
            for (int i = 0 ; i < List_Size; i++)
                Probabilities_L[i] = 3;
        }
    }
    if (State != -1) // The game has been lost
    {
        printf("GAME OVER!\n");
        printf("Do you want to save your progress?(Y/N)\n =>");
        getchar();
        char Choice;
        scanf("%c", &Choice);
        getchar();
        while (Choice != 'y' && Choice != 'Y' && Choice != 'n' && Choice != 'N')
        {
            printf("\nError: Invalid Response!\n=> ");
            scanf("%c", &Choice);
            getchar();
        }
        if (Choice == 'y' || Choice == 'Y') // Normal Saving
        {
            Save_Data(list, List_Size, Probabilities, S_C, Saved_Choices_Count+1, King_Name, KPeople, KTreasury, KCourt, -1, 0);
            printf("Thank You For Playing!\n");
            // Freeing memory
            free(S_C);
            free(S_C_L);
            Delete(&list);
            printf("\nPRESS ANY KEY TO EXIT...!");
            getchar();
            return 0;
        }
        else
        {
            if (Load_Condition == 1) // Saving from temp
                Save_Data(list_L, List_Size, Probabilities_L, S_C_L, Saved_Choices_Count_L, King_Name, KPeople_L, KTreasury_L, KCourt_L, 1, 0);
            // Freeing memory
            free(S_C);
            free(S_C_L);
            Delete(&list);
            printf("\nPRESS ANY KEY TO EXIT...!");
            getchar();
            return 0;
        }
    }
    else
    {
        // Freeing memory
        free(S_C);
        free(S_C_L);
        Delete(&list);
        return 0;
    }
}

