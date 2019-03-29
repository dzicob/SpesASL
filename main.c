// Code by dzicob 20/02/2019

#define FILTRO '5'
#define NUM_COLONNE 23
#define MAX_TRANS 200
#define MAX_CAPIT 50

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>

int ScanNoSpace(FILE *ptr, char *stringa);
int findTrans(struct macroagg_prog esame, int *min, int *max);
int numbers_only(char *stringa);

struct capitolo   //Tutti i budget sono in centesimi e non in euro
{
	char ID[10];
	int iniziale;    //iniziale + residuo son quello che mi serve
	int residuo;
	int finale;    //questo è invece ciò che ho

	int netto;
	int backupnetto;   //poichè opero sul netto, uso questa variabile come backup per ricordarmi il netto iniziale
};

struct macroagg_prog   //c'è una struct per ogni macroaggregato, ma differenzio anche i macroaggregati uguali ma con programma differente.
{
	int macroagg;
	int prog;
	int netto = 0;

	//Nella prima riga indica l'ammontare di ogni transazione, nella seconda indica DA che capitolo, nella terza indica A che capitolo
	int transazioni[3][MAX_TRANS];
	int numtrans = 0;

	capitolo capitoli[MAX_CAPIT];
	int numCapit = 0;
};

int main()
{
	int i, j, nuova, exit, exit2 = 0, aux = 0;     //variabili ausiliarie
	int intscelt = 0;
	char scelta;
	char pergets[500];

	int amount, da = -1, a = -1;   //inizializzate in modo tale da rendermene conto se non venissero modificate dalla funzione maxmodulo

	int capitoli = 0, capitInEsame = 0;     //counters
	int macroNum = 0; 
	int blackNum = 0;
	
	FILE *exc, *blacklist;
	macroagg_prog macro[50];
	
	char blackL[100][30];          //Tutti gli elementi in blacklist (30 caratteri massimo per elemento)
	
	//variabili per salvataggio temporaneo capitoli
	char check[500], temp_capID[20];
	int temp_macroID, temp_prog;
	int temp_iniz, temp_fin, temp_residuo;  
	double dubiniz, dubfin, dubresiduo;

	blacklist = fopen("blacklist.txt", "a+");
	assert(blacklist != NULL);

	//Carico / creo subito la BlackList
	while (fgets(blackL[blackNum], 30, blacklist) != NULL)    //fgets restituisce NULL se non va a buon fine, lo uso direttamente anche come controllo del while
	{
		blackNum++;
	}

	printf("-----------------------------SpesASL\n-----------------------------25/02/19 Software by Nicolo' Loddo\n");
	printf("Non mi assumo responsabilita' per nessuna delle conseguenze dovute all'utilizzo di questo programma.\n\n\n");
	printf("\tGIUSTO QUATTRO INDICAZIONI PRIMA DI INIZIARE:\n\n");
	printf("1. Fai una copia del tuo file excel originale e opera su questa, cosi' sei sicuro di non modificare il file originale!\n\n");
	printf("2. Togli tutti gli sheet del file excel a parte lo sheet 3 che e' quello che dobbiamo analizzare.\n");
	printf("IMPORTANTISSIMO: le colonne di STANZ. INIZIALE, STANZ. FINALE e RESIDUO, non devono assolutamente contenere lettere.\n\n");
	printf("3. IL FILE EXCEL DEVE ESSERE .csv : USA UN CONVERTITORE ONLINE!\n");
	printf("\nIl sito che trovi sotto a questa riga, ad esempio, va benissimo:\n");
	printf("https://convertio.co/it/xls-csv/ \n");
	printf("Devi solo trascinare il file excel sopra alla pagina web e cliccare converti.\n\n");
	printf("4. Metti il file nella cartella del programma: dovrebbe chiamarsi SpesASL e trovarsi nel Desktop, poi cambiagli il nome in 'spesa'.\n\n\n");

	do
	{
		//reinizializzazioni delle variabili ausiliarie e counter
		da = -1; a = -1;
		capitoli = 0; capitInEsame = 0;
		macroNum = 0;
		exit2 = 0;

		do
		{
			printf("MENU':\n\n");
			printf("\t1. AVVIA     \t\t  premi [a] e INVIO\n");
			printf("\t2. BLACKLIST     \t  premi [b] e INVIO\n");
			printf("\t3. ESCI DAL PROGRAMMA     premi [x] e INVIO\n");
			scanf("%c", &scelta);
		} while (scelta != 'a' && scelta != 'A' && scelta != '1' && scelta != 'b' && scelta != 'B' && scelta != '2' && scelta != 'x' && scelta != 'X' && scelta != '3');

		if (scelta == 'b' || scelta == 'B' || scelta == '2')
		{
			printf("Ricordati che e' consigliabile aprire direttamente il file 'blacklist.txt' della cartella del programma per modificare la blacklist.\n\n");
			printf("Quanti elementi vuoi inserire? ");
			scanf("%d", &intscelt);

			for (int add = 0; add < intscelt; add++)   //aggiungo tutti gli elementi e poi li metto nel file.
			{
				printf("Elemento %d: ", add+1);
				scanf("%s", blackL[blackNum]);
				fprintf(blacklist, "%s\n", blackL[blackNum]);
				blackNum++;
			}

		}
		else if (scelta == 'x' || scelta == 'X' || scelta == '3')
		{
			printf("\nCiao!   (Premi INVIO)\n");
		}
		else if (scelta == 'a' || scelta == 'A' || scelta == '1')   //inizio del case di avvio
		{

			//CARICAMENTO DEI DATI ----------------------------------

			exc = fopen("spesa.csv", "r");    //Apriamo il file
			assert(exc != NULL);

			fgets(pergets, 500, exc);      //Saltiamo la prima riga

			while (!exit2)
			{
				exit = 0;                  //variabile che indica se il capitolo è da utilizzare
				nuova = 1;
				capitoli++;

				for (int k = 0; k < NUM_COLONNE;)   //Analizzazione colonna per colonna per capire se caricare la riga o no
				{
					aux = ScanNoSpace(exc, check);   //I dati son caricati in check per essere analizzati

					if (aux)  
						switch (k)
						{
						case 0:       //Anno di esercizio
							if (!strcmp(check, ""))      //Se non è citato siamo alla riga di conclusione.
							{
								exit = 1;
								exit2 = 1;
							}
							break;

						case 5:       //ID capitolo
							strcpy(temp_capID, check);

							for (int bl = 0; bl < blackNum; bl++)     //Controllo della blacklist
								if (!strcmp(blackL[bl], check))
									exit = 1;

							break;

						case 6:       //Macroaggregato
							if (!strcmp(check, ""))        //Se non è citato il macroaggregato, non mi interessa.
								exit = 1;
							else
							{
								assert(numbers_only(check) == 1);    //strcmp rende 0 se le due stringhe sono uguali
								temp_macroID = atoi(check);
							}							

							for (int bl = 0; bl < blackNum; bl++)     //Controllo della blacklist
								if (!strcmp(blackL[bl], check))
									exit = 1;

							break;

						case 7:       //Programma
							if (strcmp(check, ""))   //strcmp rende 0 se le due stringhe sono uguali
								assert(numbers_only(check) == 1);

							temp_prog = atoi(check);

							for (int bl = 0; bl < blackNum; bl++)     //Controllo della blacklist
								if (!strcmp(blackL[bl], check))
									exit = 1;

							break;

						case 8:       //CntrResp
							if (check[10] != FILTRO)     //se il CntroResp non finisce per 05, è da ignorare.
								exit = 1;

							for (int bl = 0; bl < blackNum; bl++)     //Controllo della blacklist
								if (!strcmp(blackL[bl], check))
									exit = 1;

							break;

						case 10:      //iniziale
							if (strcmp(check, ""))   //strcmp rende 0 se le due stringhe sono uguali
								assert(numbers_only(check) == 1);

							dubiniz = strtod(check, NULL);
							temp_iniz = round(dubiniz * 100);
							break;

						case 11:      //finale
							if (strcmp(check, ""))   //strcmp rende 0 se le due stringhe sono uguali
								assert(numbers_only(check) == 1);

							dubfin = strtod(check, NULL);
							temp_fin = round(dubfin * 100);
							break;

						case 12:      //residuo
							if (strcmp(check, ""))   //strcmp rende 0 se le due stringhe sono uguali
								assert(numbers_only(check) == 1);

							dubresiduo = strtod(check, NULL);
							temp_residuo = round(dubresiduo * 100);
							if (temp_iniz + temp_residuo - temp_fin == 0)   //Se il netto è 0, non ci interessa il capitolo.
								exit = 1;
							break;

						default:
							break;
						}

					if (exit == 1)
					{
						fgets(pergets, 500, exc);     //saltiamo la riga (ossia il capitolo)
						break;
					}

					k += aux;
				}

				if (exit == 0)   //ulteriore controllo. Se exit è rimasto ==0 dopo il controllo di tutte le colonne, carichiamo la riga
				{

					for (j = 0; j < macroNum; j++)    //controllo se la macro esiste già.
					{
						if (macro[j].macroagg == temp_macroID && macro[j].prog == temp_prog)
						{
							nuova = 0;
							break;
						}

					}
					if (nuova)    //se esiste, il for esce e j indica che numero è quella macro
					{
						i = macroNum;
						macroNum++;
					}
					else
						i = j;



					capitInEsame++;

					macro[i].macroagg = temp_macroID;
					macro[i].prog = temp_prog;

					strcpy(macro[i].capitoli[macro[i].numCapit].ID, temp_capID);
					macro[i].capitoli[macro[i].numCapit].iniziale = temp_iniz;
					macro[i].capitoli[macro[i].numCapit].finale = temp_fin;
					macro[i].capitoli[macro[i].numCapit].residuo = temp_residuo;

					macro[i].capitoli[macro[i].numCapit].netto = temp_iniz + temp_residuo - temp_fin;
					macro[i].capitoli[macro[i].numCapit].backupnetto = temp_iniz + temp_residuo - temp_fin;

					macro[i].netto += macro[i].capitoli[macro[i].numCapit].netto;
					macro[i].numCapit++;
				}

			}

			fclose(exc);


			//CALCOLO DELLE TRANSAZIONI    -----------------------------------------------

			for (int mod = 0; mod < macroNum; mod++)
			{
				while (findTrans(macro[mod], &da, &a) == 1)    //eseguo maxmodulo e lo uso come controllo allo stesso tempo
				{
					amount = macro[mod].capitoli[da].netto + macro[mod].capitoli[a].netto;
					if (amount >= 0)
					{
						amount = (-1)*macro[mod].capitoli[a].netto;
						macro[mod].capitoli[da].netto -= amount;
						macro[mod].capitoli[a].netto = 0;
					}
					else
					{
						amount = macro[mod].capitoli[da].netto;
						macro[mod].capitoli[a].netto += amount;
						macro[mod].capitoli[da].netto = 0;
					}

					macro[mod].transazioni[0][macro[mod].numtrans] = amount;
					macro[mod].transazioni[1][macro[mod].numtrans] = da;
					macro[mod].transazioni[2][macro[mod].numtrans] = a;
					macro[mod].numtrans++;
				}
			}


			//OUTPUT      --------------------------------------

			printf("\n\n------------------------------------------------------\n");
			printf("Capitoli esaminati= %d\n", capitInEsame);
			printf("Macroaggregati-Programma (entita' tra le quali si possono eseguire transazioni)= %d:\n\n", macroNum);

			for (int n = 0; n < macroNum; n++)
			{

				printf("%d.----MACROAGGREGATO: %d, PROGRAMMA: %d-------------------------------------------------", n+1, macro[n].macroagg, macro[n].prog);

				if(macro[n].netto >= 0)
					printf("\n\nNetto dei capitoli in questo Macroaggregato-Programma: %d.%d\n", macro[n].netto / 100, macro[n].netto - (macro[n].netto / 100) * 100);
				else
					printf("\n\nNetto dei capitoli in questo Macroaggregato-Programma: %d.%d\n", macro[n].netto / 100, (-1)*(macro[n].netto - (macro[n].netto / 100) * 100));
				
				printf("\n%d CAPITOLI CON NETTO INIZIALE:\n", macro[n].numCapit);

				for (int k = 0; k < macro[n].numCapit; k++)
				{
					if(macro[n].capitoli[k].backupnetto >= 0)
						printf("%s   [%d.%d]\n", macro[n].capitoli[k].ID, macro[n].capitoli[k].backupnetto / 100, macro[n].capitoli[k].backupnetto - (macro[n].capitoli[k].backupnetto / 100) * 100);
					else
						printf("%s   [%d.%d]\n", macro[n].capitoli[k].ID, macro[n].capitoli[k].backupnetto / 100, (-1)*(macro[n].capitoli[k].backupnetto - (macro[n].capitoli[k].backupnetto / 100) * 100));
				}

				for (int k = 0; k < macro[n].numCapit; k++)
				{

				}

				if (macro[n].numtrans > 0)   //Lo stampo solo se ci sono transazioni all'interno.
				{
					printf("\nTRANSAZIONI SUGGERITE:\n");

					for (int k = 0; k < macro[n].numtrans; k++)
					{
						if(macro[n].transazioni[0][k] >= 0)
							printf("%d. DA: %s ----> A: %s    [%d.%d]\n", k+1, macro[n].capitoli[macro[n].transazioni[1][k]].ID, macro[n].capitoli[macro[n].transazioni[2][k]].ID, macro[n].transazioni[0][k] / 100, macro[n].transazioni[0][k] - (macro[n].transazioni[0][k] / 100) * 100);
						else
							printf("%d. DA: %s ----> A: %s    [%d.%d]\n", k+1, macro[n].capitoli[macro[n].transazioni[1][k]].ID, macro[n].capitoli[macro[n].transazioni[2][k]].ID, macro[n].transazioni[0][k] / 100, (-1)*(macro[n].transazioni[0][k] - (macro[n].transazioni[0][k] / 100) * 100));
					}

					printf("\nNETTO FINALE DEI CAPITOLI:\n");

					for (int k = 0; k < macro[n].numCapit; k++)
					{
						if(macro[n].capitoli[k].netto >= 0)
							printf("%s   [%d.%d]\n", macro[n].capitoli[k].ID, macro[n].capitoli[k].netto / 100, macro[n].capitoli[k].netto - (macro[n].capitoli[k].netto / 100) * 100);
						else
							printf("%s   [%d.%d]\n", macro[n].capitoli[k].ID, macro[n].capitoli[k].netto / 100, (-1)*(macro[n].capitoli[k].netto - (macro[n].capitoli[k].netto / 100) * 100));
					}
				}

				printf("\n\n\n");
			}

			//fine del case di avvio

		}

		for (int azz = 0; azz < macroNum; azz++)   //Azzero le macro per poter riavviare il software senza chiuderlo, se necessario
			macro[azz].numCapit = 0;

		getchar();

	}while (scelta != 'x' && scelta != 'X' && scelta != '3');

	fclose(blacklist);

	getchar();
	getchar();
	return 0;
}


//È un fscanf per stringhe di file excel (.csv) N.B. elimina spazi e virgole dai dati di excel oltre a registrarli. 
//Restituisce 1 se ha captato una stringa, 0 altrimenti.
int ScanNoSpace(FILE *ptr, char *stringa)   
{
	int i = 0;
	char aux;

	fscanf(ptr, "%c", &aux);
	if (aux == '"') //prima controlliamo che il primo carattere sia un ", poi scannerizziamo ciò che è dentro le virgolette saltando gli spazi
	{
		fscanf(ptr, "%c", &aux);

		while (aux != '"')
		{
			if (aux != ' ' && aux != ',')
			{
				stringa[i] = aux;
				i++;
			}

			fscanf(ptr, "%c", &aux);
		}

		stringa[i] = '\0';

		i = 1;
	}
	else
		strcpy(stringa, "niente stringa!");

	return i;

}


//cerca il massimo in modulo tra i budget netti dei capitoli di una macro, sia tra i numeri negativi che positivi e li mette nei due int in input
int findTrans(macroagg_prog esame, int *positivo, int *negativo)   
{
	int max = 0, min_neg = 0, min_negindex = 0;
	int near = 0;
	int checkpos = 0, checkneg = 0;
	int scarti[MAX_CAPIT];
	int num_scarti = 0;
	int scarta = 0;

	do
	{
		max = 0;

		for (int i = 0; i < esame.numCapit; i++)   //Trova il massimo in modulo dei negativi e controlla che non sia tra gli scarti
		{
			scarta = 0;

			for (int j = 0; j < num_scarti; j++)
				if (i == scarti[j])
				{
					scarta = 1;
					break;
				}
					
			if (scarta==0)    //Cerco il negativo maggiore in modulo che non sia da scartare
			{
				if (esame.capitoli[i].netto*(-1) > max)
				{
					max = esame.capitoli[i].netto*(-1);
					*negativo = i;
					checkneg = 1;


					//Inizializziamo min_neg col negativo più piccolo in modulo che troviamo per ora: ci servirà se non abbiamo numeri positivi sufficientemente grandi!
					min_neg = esame.capitoli[i].netto*(-1);   
					min_negindex = i;
						
				}

			}

		}
			
		max = 0;

		for (int i = 0; i < esame.numCapit; i++)   //Cerco il positivo maggiore in modulo
			if (esame.capitoli[i].netto > max)
			{
				max = esame.capitoli[i].netto;
				*positivo = i;
				checkpos = 1;
			}
		
		if (checkpos == 0 || checkneg == 0)   //Se tutti i negativi o tutti i positivi son stati azzerati, possiamo uscire
			break;

		near = esame.capitoli[*positivo].netto + esame.capitoli[*negativo].netto;   //Calcolo la loro differenza come punto di riferimento

		if (near >= 0)
		{
			for (int i = 0; i < esame.numCapit; i++)   //cerca il numero positivo più piccolo tra i più grandi in modulo del negativo
				if (esame.capitoli[i].netto > 0 && esame.capitoli[i].netto + esame.capitoli[*negativo].netto >= 0 && esame.capitoli[i].netto + esame.capitoli[*negativo].netto < near)
				{
					near = esame.capitoli[i].netto + esame.capitoli[*negativo].netto;
					*positivo = i;
				}
		}
		else
		{
			scarti[num_scarti] = *negativo;
			num_scarti++;
			checkneg = 0;
		}

	} while (near<0 && num_scarti < esame.numCapit);

	if (checkneg == 0)  //Se succede questo, vuol dire che tutti i negativi erano più grandi in modulo dei positivi: procedo cercando di azzerare almeno i negativi più piccoli in modulo.
	{
		max = 0;

		for (int i = 0; i < esame.numCapit; i++)  //Trovo il positivo maggiore in modulo
			if (esame.capitoli[i].netto > max)
			{
				max = esame.capitoli[i].netto;
				*positivo = i;
				checkpos = 1;
			}

		if (min_neg > 0)     //Se quello che abbiamo preso come minore di riferimento fosse effettivamente il minore ma venisse scartato sempre, il checkneg rimarrebbe ingiustamente a 0
			checkneg = 1;

		for (int i = 0; i < esame.numCapit; i++)   //Trovo il negativo minore in modulo
			if (esame.capitoli[i].netto*(-1) < min_neg && esame.capitoli[i].netto < 0)
			{
				min_neg = esame.capitoli[i].netto;
				min_negindex = i;
				checkneg = 1;
			}

		*negativo = min_negindex;
	}

	
	//Da qui inizia la griglia dei return
	if (checkneg == 0 && checkpos == 0)    //Tutti i capitoli hanno netto pari a 0
	{
		return 0;
	}
	if (checkneg == 0 && checkpos == 1)   //Tutti i negativi hanno netto pari a 0
	{
		return 2;
	}
	if (checkneg == 1 && checkpos == 0)   //Tutti i positivi hanno netto pari a 0
	{
		return 3;
	}
	if (checkneg == 1 && checkpos == 1)   //La funzione è stata eseguita interamente correttamente e i valori di negativo e positivo son cambiati
	{
		return 1;
	}
}

int numbers_only(char *stringa)    //controlla che nella stringa vi siano solo numeri o al limite il punto per indicare la virgola
{
	int i = 0;

	while (stringa[i] != '\0')
	{
		if ((stringa[i] <= 47 || stringa[i] >= 58) && stringa[i] != 46)
			return 0;
		i++;
	}

	return 1;
}
