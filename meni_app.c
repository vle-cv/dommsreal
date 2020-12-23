#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define fajl "/dev/stred"
#define STRED_SIZE 100

FILE *fp;
int br=STRED_SIZE;


void meni (){
	puts("Izaberi jednu od ponudjenih opcija");
	puts("Format poziva je [mod] [opciona_vrednost]");
	puts("");
	puts("1: Procitaj trenutno stanje stringa");
	puts("2: Upisi novi string (zahteva vrednost)");
	puts("3: Konkataniraj string na trenutni (zahteva vrednost)");
	puts("4: Izbrisi citav string");
	puts("5: Izbrisi vodece i pratece space karaktere");
	puts("6: Izbrisi izraz iz stringa (zahteva vrednost)");
	puts("7: izbrisi poslednjih n karaktera iz stringa (zahteva vrednost)");
	puts("Q: quit");
}

void str_read(){
	char *buff;

	fp=fopen(fajl, "r");
	if (fp==NULL){
		puts("Greska pri otvaranju datoteke");
		return;
	}
	
	buff=(char *)malloc(br+1);
	getline(&buff, &br, fp);
	printf("%s\n", buff);

	if(fclose(fp)){
		puts("Problem pri zatvaranju datoteke");
		return;
	}	


}

void str_write(char *buff){

	fp=fopen(fajl, "w");
	if (fp==NULL){
		puts("Greska pri otvaranju datoteke");
		return;
	}
	fprintf(fp,"string=%s", buff);

	if(fclose(fp)){
		puts("Problem pri zatvaranju datoteke");
		return;
	}	


}

void konkat_str(char *buff){
	fp=fopen(fajl, "w");
	if (fp==NULL){
		puts("Greska pri otvaranju datoteke");
		return;
	}
	
	fprintf(fp,"append=%s", buff);

	if(fclose(fp)){
		puts("Problem pri zatvaranju datoteke");
		return;
	}	


}

void izbrisi(){
	fp=fopen(fajl, "w");
	if (fp==NULL){
		puts("Greska pri otvaranju datoteke");
		return;
	}
	
	fprintf(fp,"clear\n");

	if(fclose(fp)){
		puts("Problem pri zatvaranju datoteke");
		return;
	}	


}

void shrink(){
	fp=fopen(fajl, "w");
	if (fp==NULL){
		puts("Greska pri otvaranju datoteke");
		return;
	}
	
	fprintf(fp,"shrink\n");

	if(fclose(fp)){
		puts("Problem pri zatvaranju datoteke");
		return;
	}	


}

void remove_substr(char *buff){
	fp=fopen(fajl, "w");
	if (fp==NULL){
		puts("Greska pri otvaranju datoteke");
		return;
	}
	
	fprintf(fp,"remove=%s", buff);

	if(fclose(fp)){
		puts("Problem pri zatvaranju datoteke");
		return;
	}	


}

void truncate(char *buff){
	fp=fopen(fajl, "w");
	if (fp==NULL){
		puts("Greska pri otvaranju datoteke");
		return;
	}
	fprintf(fp,"truncate=%s", buff);

	if(fclose(fp)){
		puts("Problem pri zatvaranju datoteke");
		return;
	}	


}

int main(){
	char mod;
	char *str;

	while(1){
	meni();
	str=(char *)malloc(br+1);
	getline(&str, &br, stdin);
		
	switch(str[0]){
		case '1':
			str_read();
			break;
		case '2':
			str_write(str+2);
			break;
		case '3': 
			konkat_str(str+2);
			break;
		case '4':
			izbrisi();
			break;
		case '5': 
			shrink();
			break;
		case '6':
			remove_substr(str+2);
			break;
		case '7':
			truncate(str+2);
			break;
		case 'Q':
			puts("Dovidjenja okruti svete");
			exit(0);	
			break;
		default:
			puts("Pogresan unos, uneti broj od 1 do 7");
			}
	}	
}
