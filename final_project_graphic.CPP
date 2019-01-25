#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <conio.h>
#include <graphics.h>
#include <dos.h>

/* THE KINGS GAME :: FINAL PROJJECT
	Written by Roham Zendehdel Nobari at 5:17:29 AM 12/31/2018
	COPYRIGHTS by RKK
*/
#define P_C 7
#define TWIDTH 6
#define MAX_X 639
#define MAX_Y 479

union REGS input, output;

char *CHOICEDIR = "Data\\CHOICES.txt";
char *TOPDIR = "Data\\Kings.bin";
char *SAVEDIR = "Data\\Save\\";

int initmouse()
{
	input.x.ax = 0;
	int86(0x33, &input, &output);
	return (output.x.ax);
}

void showmouseptr()
{
	input.x.ax = 1;
	int86(0x33, &input, &output);
}

void getmousepos(int* button, int* x, int* y)
{
	input.x.ax=3;
	int86(0x33, &input, &output);

	*button = output.x.bx;
	*x = output.x.cx;
	*y = output.x.dx;
}

char* Get_Max_Text(char* Problem, char* Choice1, char* Choice2)
{
	if (strlen(Problem) > strlen(Choice1) == 1)
	{
		if(strlen(Problem) > strlen(Choice2) == 1)
			return Problem;
		else
			return Choice2;
	}
	else
	{
		if  (strlen(Choice1) > strlen(Choice2) == 1)
			return Choice1;
		else
			return Choice2;
			}
}

void Print_Status(int People, int Court, int Treasury)
{
	char Text[255];
	sprintf(Text, "People = %d Court = %d Treasury = %d", People, Court, Treasury);
	setfillstyle(1, RED);
	bar(125,0,510,50);
	setfillstyle(1, YELLOW);
	bar(135,10,500,40);
	settextstyle(7,0,1);
	setcolor(BLUE);
	outtextxy(135,10,Text);
}

int Create_Button(char* Problem, char* Choice1, char* Choice2, int start_y)
{
	int width = textwidth(Get_Max_Text(Problem, Choice1, Choice2));
	//int height = textheight(Get_Max_Text(Problem, Choice1, Choice2));
	int start_x = 319 -  (width/2);
	int end_x = 319 +  (width/2);
	setfillstyle(1, BLUE);
	bar(start_x - 10, start_y - 10, end_x + 10, start_y + 46);
	setfillstyle(1, WHITE);
	bar(start_x - 5, start_y - 5, end_x + 5, start_y + 41);
	setcolor(4);
	settextstyle(2,0,0);
	outtextxy(start_x, start_y, Problem);
	outtextxy(start_x, start_y + 12, Choice1);
	outtextxy(start_x, start_y + 24, Choice2);
	return start_x;
}

struct choice
{
	char Problem[255];
	char Choice[2][255];
	int People[2], Treasury[2], Court[2];
	int Probability;
};

struct Saved_Choices
{
	char Problem[255];
	char Choice[2][255];
	int Chosen;
	int People, Treasury, Court;
};

struct Saved_Data
{
	char King_Name[255];
	int State;
	int Probabilities[P_C];
	int People, Treasury, Court;
	int Auto_Save;
};

struct node
{
	struct choice Value;
	struct node* Prev;
	struct node* Next;
};

struct node* create_node(struct choice C)
{
	struct node* nn = (struct node*)malloc(sizeof(struct node));
	nn->Prev = NULL;
	nn->Next = NULL;
	nn->Value = C;
	return nn;
}

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

int Initiliaze_Choices(struct node** list)
{
	FILE *FCHOICE = fopen(CHOICEDIR, "r");
	char DIR[255];
	int n = 0;
	while (fgets(DIR, 255, FCHOICE)!=NULL)
	{
		char NDIR[255] = "Data\\";
		if (DIR[strlen(DIR)-1] == '\n')
		   DIR[strlen(DIR)-1] = '\0';
		strcat(NDIR,DIR);
		FILE* fp = fopen(NDIR,"r");
		struct choice Choice = Read(fp);
		push_back(list, Choice);
		fclose(fp);
		n++;
	}
	fclose(FCHOICE);
	return n;
}

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

struct node* Random_Problem(struct node** list)
{
	int Counter = 0 , Random_Prob = 0;
	for (int i = 3; i >= 1; i--)
	{
		struct node* pt = *list;
		struct node* Equal_Probability[P_C];
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
	return NULL;
}

void Print_Problem(struct node* Problem, int start_y, int* start_x)
{
	settextstyle(2,0,0);
	*start_x = Create_Button(Problem->Value.Problem, Problem->Value.Choice[0], Problem->Value.Choice[1], start_y);
}

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

void PRINT_TOP()
{
	char KING_DIR[255];
	struct Saved_Data* S_D = NULL;
	int King_Count = 0;

	FILE* FK = fopen(TOPDIR, "rb+");
	while(fread(KING_DIR, sizeof(KING_DIR), 1, FK) == 1)
	{
		FILE* SAVE = fopen(KING_DIR, "rb");
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

void Save_Data(int Probabilities[P_C], struct Saved_Choices* S_C, int Saved_Choices_Count, char King_Name[], int KPeople, int KTreasury,
			   int KCourt, int State, int Auto)
{
	char DIR[255];

	strcpy(DIR, "Data\\Save\\");

	struct Saved_Data S_D;
	for (int i = 0 ; i < P_C; i++)
	{
		S_D.Probabilities[i] = Probabilities[i];
	}
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
	fwrite(&Saved_Choices_Count, sizeof(int), 1, F);
	fwrite(S_C, sizeof(struct Saved_Choices), Saved_Choices_Count, F);
	fclose(F);
	Find_King(S_D);
}

int Load_Data(char King_Name[255], int Probabilities[P_C], struct Saved_Choices** S_C, int *Saved_Choices_Count, int *KPeople,
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
		fread(&S, sizeof(S), 1, F);
		if (S.State == -1 && Condition == 0)
			return -1;
		if (S.Auto_Save == 1 && Condition == 0)
			return -3;
		fread(&SCC, sizeof(int), 1, F);
		(*S_C) = (struct Saved_Choices*)malloc(SCC * sizeof(struct Saved_Choices));
		fread(*S_C, sizeof(struct Saved_Choices), SCC, F);
		*KPeople = S.People;
		*KTreasury = S.Treasury;
		*KCourt = S.Court;
		*Saved_Choices_Count = SCC;
		*State = S.State;
		for (int i = 0 ; i < P_C; i++)
		{
			Probabilities[i] = S.Probabilities[i];
		}
	}
	fclose(F);
	return 0;
}

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

void Update_Probabilities(struct node* list, int Probabilities[P_C])
{
	struct node* pt = list;
	for (int i = 0 ; i < P_C; i++)
	{
		if (Probabilities[i] != 0)
		{
			Probabilities[i] = pt->Value.Probability;
			pt = pt->Next;
		}
	}
}

void Update_List(struct node** list, int Probabilities[P_C])
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
	int Mouse_x, Mouse_y, Button;

	//INITIALIZE THE LIST OF PROBLEMS
	struct node* list = NULL;

	Initiliaze_Choices(&list);
	// LOADED_LIST
	struct node* list_L = NULL;
	Initiliaze_Choices(&list_L);

	int Probabilities[P_C], Probabilities_L[P_C];

	for (int i = 0 ; i < P_C; i++)
		Probabilities[i] = 3;
	for (i = 0 ; i < P_C; i++)
		Probabilities_L[i] = 3;
	// END OF PROBLEM INITIALIZATION

	struct Saved_Choices *S_C = NULL;
	int Saved_Choices_Count = 0;
	int KPeople = 50, KTreasury = 50, KCourt = 50;
	int First_Choice = 0;
	int State = 0;
	//LOADED_DATA
	struct Saved_Choices *S_C_L = NULL;
	int Saved_Choices_Count_L = 0;
	int KPeople_L , KTreasury_L , KCourt_L;
	int State_L = 0;

	char King_Name[255];
	int Load_Condition = 0;

	srand(time(0));
	printf("Welcome to THE KINGS GAME:\n");
	printf("\nPlease enter your name:\n");
	scanf("%s", King_Name);
	printf("\n1) New Game.\n");
	printf("2) Resume Game.\n");
	printf("3) Show High Scores.\n");
	scanf("%d", &First_Choice);
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
		if (Load_Data(King_Name, Probabilities, &S_C, &Saved_Choices_Count, &KPeople, &KTreasury, &KCourt, &State, 0) == -2)
		{
			int C;
			printf("You don't have a previous game...!\n1) New Game\n2) Exit\n");
			scanf("%d", &C);
			if (C == 2)
			{
				free(S_C);
				free(S_C_L);
				Delete(&list);
				return 0;
			}
		}
		else if (Load_Data(King_Name, Probabilities, &S_C, &Saved_Choices_Count, &KPeople, &KTreasury, &KCourt, &State, 0) == -1)
		{
			int C;
			printf("You lost your previous game...!\n1) New Game\n2) Show Details\n");
			scanf("%d", &C);
			if (C == 2)
			{
				Load_Data(King_Name, Probabilities, &S_C, &Saved_Choices_Count, &KPeople, &KTreasury, &KCourt, &State, 1);
				Print_Previous_Choices(S_C, Saved_Choices_Count);
			}
		}
		else if (Load_Data(King_Name, Probabilities, &S_C, &Saved_Choices_Count, &KPeople, &KTreasury, &KCourt, &State, 0) == -3)
		{
			int C;
			printf("You didn't save your previous progress...!\n1) New Game\n2) Resume from auto_save\n");
			scanf("%d", &C);
			if (C == 2)
			{
				Load_Condition = 1;
				Load_Data(King_Name, Probabilities, &S_C, &Saved_Choices_Count, &KPeople, &KTreasury, &KCourt, &State, 1);
				Load_Data(King_Name, Probabilities_L, &S_C_L, &Saved_Choices_Count_L, &KPeople_L, &KTreasury_L, &KCourt_L, &State_L, 1);
				Print_Previous_Choices(S_C, Saved_Choices_Count);
				Update_List(&list, Probabilities);
				Update_List(&list_L, Probabilities_L);
			}
		}
		else
		{
			Load_Condition = 1;
			Load_Data(King_Name, Probabilities_L, &S_C_L, &Saved_Choices_Count_L, &KPeople_L, &KTreasury_L, &KCourt_L, &State_L, 1);
			Print_Previous_Choices(S_C, Saved_Choices_Count);
			Update_List(&list, Probabilities);
			Update_List(&list_L, Probabilities_L);
		}
	}

	int gd = DETECT, gm;
	int start_y = 65;
	int start_x = 0;
	initgraph(&gd, &gm, "C:\\TURBOC3\\BGI");
	initmouse();
	showmouseptr();

	while (Loss_Check(KPeople, KTreasury, KCourt) == 0 && State != -1)
	{
		Save_Data(Probabilities, S_C, Saved_Choices_Count, King_Name, KPeople, KTreasury, KCourt, 1, 1);
		struct node* Problem = NULL;
		int Choice = 0;
		if (State == 0)
		{
			Print_Status(KPeople, KCourt, KTreasury);
			Problem = Random_Problem(&list);
			Print_Problem(Problem, start_y, &start_x);
			//start_y += 56;
		}
		else if (State == 1)
		{
			Problem = Find_Problem(&list, S_C, Saved_Choices_Count);
			Print_Problem(Problem, start_y, &start_x);
			//start_y += 56;
		}

		char Key;

		while(1)
		{
			if (kbhit())
				Key = getch();
			getmousepos(&Button, &Mouse_x, &Mouse_y);
			if (Button == 1)
			{
				if ((Mouse_x >= start_x) && (Mouse_x <= (start_x + textwidth(Problem->Value.Choice[0]))))
					if( (Mouse_y >= (start_y + 17)) && (Mouse_y <= (start_y + 24)) )
						Choice = 1;
				if ((Mouse_x >= start_x) && (Mouse_x <= (start_x + textwidth(Problem->Value.Choice[1]))))
					if( (Mouse_y >= (start_y + 28)) && (Mouse_y <= (start_y + 35)) )
						Choice = 2;
			}
			else
			{
				if ((Mouse_x >= start_x) && (Mouse_x <= (start_x + textwidth(Problem->Value.Choice[0]))))
				{
					if( (Mouse_y >= (start_y + 17)) && (Mouse_y <= (start_y + 24)) )
					{
						setcolor(GREEN);
						settextstyle(2,0,0);
						outtextxy(start_x, start_y + 12, Problem->Value.Choice[0]);
					}
					else
					{
						setcolor(RED);
						settextstyle(2,0,0);
						outtextxy(start_x, start_y + 12, Problem->Value.Choice[0]);
					}
				}
				else
				{
					setcolor(RED);
					settextstyle(2,0,0);
					outtextxy(start_x, start_y + 12, Problem->Value.Choice[0]);
				}
				if ((Mouse_x >= start_x) && (Mouse_x <= (start_x + textwidth(Problem->Value.Choice[1]))))
				{
					if( (Mouse_y >= (start_y + 28)) && (Mouse_y <= (start_y + 35)) )
					{
						setcolor(GREEN);
						settextstyle(2,0,0);
						outtextxy(start_x, start_y + 24, Problem->Value.Choice[1]);
					}
					else
					{
						setcolor(RED);
						settextstyle(2,0,0);
						outtextxy(start_x, start_y + 24, Problem->Value.Choice[1]);
					}
				}
				else
				{
					setcolor(RED);
					settextstyle(2,0,0);
					outtextxy(start_x, start_y + 24, Problem->Value.Choice[1]);
				}
			}
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

				start_y += 56;
				if (start_y >= 401)
				{
					cleardevice();
					closegraph();
					initgraph(&gd,&gm,"C:\\TURBOC3\\BGI");
					initmouse();
					showmouseptr();
					start_y = 65;
				}
				Button = -1;
				Choice = 0;
				break;
			}
			if (Key == 'e' || Key == 'E')
			{
				//getchar();
				printf("Do you want to save your progress?(Y/N)\n");
				char Choice;
				scanf("%c", &Choice);
				if (Choice == 'y' || Choice == 'Y')
				{
					Save_Problem(Problem, &S_C, &Saved_Choices_Count, -1, KPeople, KTreasury, KCourt);
					Save_Data(Probabilities, S_C, Saved_Choices_Count, King_Name, KPeople, KTreasury, KCourt, 1, 0);
					printf("Thank You For Playing!\n");
					free(S_C);
					Delete(&list);
					getchar();
					printf("\nPRESS ANY KEY TO EXIT...!");
					getchar();
					return 0;
				}
				else
				{
					if (Load_Condition == 1)
						Save_Data(Probabilities_L, S_C_L, Saved_Choices_Count_L, King_Name, KPeople_L, KTreasury_L, KCourt_L, 1, 0);
					free(S_C);
					free(S_C_L);
					Delete(&list);
					getchar();
					printf("\nPRESS ANY KEY TO EXIT...!");
					getchar();
					return 0;
				}
			}
		}
		if (State == 0)
			Save_Problem(Problem, &S_C, &Saved_Choices_Count, Choice, KPeople, KTreasury, KCourt);
		else if (State == 1)
		{
			S_C[Saved_Choices_Count-1].Chosen = Choice;
			S_C[Saved_Choices_Count-1].People = KPeople;
			S_C[Saved_Choices_Count-1].Treasury = KTreasury;
			S_C[Saved_Choices_Count-1].Court = KCourt;
			State = 0;
		}

		Problem->Value.Probability = (Problem->Value.Probability) - 1;
		Update_Probabilities(list, Probabilities);

		if (Problem->Value.Probability == 0)
		{
			Delete_Problem(&list, Problem);
		}
		if (List_Length(list) == 0)
		{
			Initiliaze_Choices(&list);
			for (int i = 0 ; i < P_C; i++)
				Probabilities[i] = 3;
			for (i = 0 ; i < P_C; i++)
				Probabilities_L[i] = 3;
		}
	}
	if (State != -1)
	{
		printf("GAME OVER!\n");
		printf("Do you want to save your progress?(Y/N)\n");
		getchar();
		char Choice;
		scanf("%c", &Choice);
		if (Choice == 'y' || Choice == 'Y')
		{
			Save_Data(Probabilities, S_C, Saved_Choices_Count+1, King_Name, KPeople, KTreasury, KCourt, -1, 0);
			printf("Thank You For Playing!\n");
			free(S_C);
			free(S_C_L);
			Delete(&list);
			getchar();
			printf("\nPRESS ANY KEY TO EXIT...!");
			getchar();
			return 0;
		}
		else
		{
			if (Load_Condition == 1)
				Save_Data(Probabilities_L, S_C_L, Saved_Choices_Count_L, King_Name, KPeople_L, KTreasury_L, KCourt_L, 1, 0);
			free(S_C);
			free(S_C_L);
			Delete(&list);
			getchar();
			printf("\nPRESS ANY KEY TO EXIT...!");
			getchar();
			return 0;
		}
	}
	else
	{
		free(S_C);
		free(S_C_L);
		Delete(&list);
		return 0;
	}
}