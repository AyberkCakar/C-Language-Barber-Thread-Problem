#include <stdio.h>
#include <stdlib.h>     // K�t�phanelerimizi tan�mlad�k
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define TRAS_ZAMANI 3  	// Tra� S�resini Belirledik
#define koltukSayisi 1		// Berber 1 tane ve berber koltu�umuzda 1 tane
#define sandalyeSayisi 5	// 10 tane bekleme sandalyemiz var


int musteriSayisi=0;			// �lk de�erlerimizi 0 atad�k
int bosBeklemeSandalyeSayisi=0;
int trasEdilecekMusteri =0;
int oturalacakSandalye =0;
int ilkTras=0;
int musteriDurumSayisi=0;

int* koltuk;


sem_t berber_sem;			// Kullanaca��m�z Semaforlar� tan�mlad�k
sem_t musteriler_sem;
sem_t mutex_sem;


int Berber()
{
    int sonrakiMusteri, musteriID;
    
    if(ilkTras==0)  	// Berber ilk m��teri grubunda d�kkan� a�ar
    {
    	printf("Berber\tdukkana girdi.\n\n");
    }
    else		// Di�er M��teri gruplar�nda ise uyan�r.
    {
    	printf("\nBerber\tuyandi.\n\n");
    }

    while(1)  		// Sonsuz D�ng� ��erisinde
    {
    
        sem_wait(&berber_sem);      // M��terinin yoksa koltu�a ileti�im engellenir.
        sem_wait(&mutex_sem);

        trasEdilecekMusteri = (++trasEdilecekMusteri) % sandalyeSayisi;  	// Tra� edilecek m��teri bekleyen m��terilerden se�ilir	
        sonrakiMusteri=trasEdilecekMusteri;
        musteriID=koltuk[sonrakiMusteri];
        koltuk[sonrakiMusteri]=pthread_self();
	musteriDurumSayisi=musteriDurumSayisi-1;
	
        sem_post(&mutex_sem);		// Koltu�a eri�im a��l�r berber m��teri ile ilgilenmeye ba�lar
        sem_post(&musteriler_sem);

        printf(" Berber \t%d. musterinin trasina basladi. \n\n",musteriID);  // Berber tra�a ba�lar ve belirledi�imiz s�re sonunda tra�� bitirir
        sleep(TRAS_ZAMANI);
        printf(" Berber \t%d. musterinin trasini bitirdi. \n\n",musteriID);
        
        if(!musteriDurumSayisi)     			// Berber tra� edilecek m��teriler bitti�inde uyur.
        {
            printf("Berber uyudu\n\n\n");
        }
    }
    pthread_exit(0);		// Thread sonland�r�l�r
}

void Musteri(void* sayi)	// M��teri fonksiyonu m��teri numaras�n� parametre olarak al�r
{
    int kimlik = *(int*)sayi + 1;  // M��terimim kimli�in al�n�r
    int oturulanSandalye;

    sem_wait(&mutex_sem);  	    // Koltu�a eri�im engellenir

    printf("%d. Musteri\tdukkana geldi. \n",kimlik);  // Gelen m��teri ekrana yazd�r�l�r

    if(bosBeklemeSandalyeSayisi > 0)
    {
        bosBeklemeSandalyeSayisi--;					// E�er bo� bekleme sandalyesi varsa sandalye say�s� bir azalt�l�r ve
        printf("%d. Musteri\tsandalyede bekliyor.\n\n",kimlik);	// M��teri beklemeye ba�lar 

        oturalacakSandalye=(++oturalacakSandalye)%sandalyeSayisi;	// M��terinin bo� sandalyelerden birine oturmas� sa�lan�r
        oturulanSandalye = oturalacakSandalye;
        koltuk[oturulanSandalye]=kimlik;

        sem_post(&mutex_sem);		// Koltu�a eri�im engeli kald�rl�r
        sem_post(&berber_sem);	// Berber uyuyorsa uyand�r�l�r ve koltu�a ge�mesi sa�lan�r

        sem_wait(&musteriler_sem);	// M��teri tra� olmak i�in bekler
        sem_wait(&mutex_sem);		// M��terilerin ayn� anda tra� olmas� �nlenir

        bosBeklemeSandalyeSayisi++; // M��teri tra� olmaya ba�lad���nda d�kkandaki sandelye say�s� bir artt�r�l�r.

        sem_post(&mutex_sem);		// Eri�im engellenir
    }
    else{
        sem_post(&mutex_sem); 	// Engel kald�r�l�r , e�er bo� sandalye yoksa m��teri d�kkandan ayr�l�r
        printf("%d. Musteri\tbeklemek icin sandalye bulamadi. Dukkandan ayriliyor.\n\n",kimlik);  
    }
    pthread_exit(0);	// Thread sonland�r�l�r
}


int main(int argc , char** args)
{	
		// �lk m��eriler i�in bilgilerimizi al�yoruz

	    printf("Musteri Sayisi Giriniz: "); 
	    scanf("%d",&musteriSayisi);
	    
	    musteriDurumSayisi =musteriSayisi;
	    bosBeklemeSandalyeSayisi = sandalyeSayisi;
	    koltuk = (int*) malloc(sizeof(int)*sandalyeSayisi);  // Belirlenen Eleman Kadar Koltuk Dizisini Olu�turduk


	    pthread_t berber[koltukSayisi], musteri[musteriSayisi];   // Berber ve m��teri thread de�i�kenlerimizi olu�turduk

	    sem_init(&berber_sem, 0,0);		// Sem init methoduyla semaforlar�m�z� ba�latt�k
	    sem_init(&musteriler_sem, 0,0); 
	    sem_init(&mutex_sem, 0,1);

	    printf("\n Berber Dukkani acti. \n\n");

	    for(int i=0; i < koltukSayisi;i++)
	    {
		pthread_create(&berber[i],NULL, (void*)Berber,(void*)&i);  // Berber thread'lerini pthread_create methodu ile olu�turduk
		sleep(1);
	    }
	    for(int i=0; i < musteriSayisi;i++)
	    {
		pthread_create(&musteri[i],NULL, (void*)Musteri,(void*)&i); // M��teri thread'lerini pthread_create methodu ile olu�turduk
		sleep(1);
	    }
	    for(int i=0; i < musteriSayisi;i++)       		// T�m M��terilerin i�lemlerinin bitmesini bekliyoruz
	    {
		pthread_join(musteri[i],NULL);
		sleep(1);
	    }
	    ilkTras=1;
	    
    do
    {
    				// �lk M��teri grubundan sonraki her grup i�in
    	
    	sleep(3);
    	musteriDurumSayisi=0;
    	printf("Musteri Sayisi Giriniz: ");
	scanf("%d",&musteriSayisi);
	
	musteriDurumSayisi=musteriSayisi;

    	if(!musteriSayisi==0 )
    	{
	    bosBeklemeSandalyeSayisi = sandalyeSayisi;
	    koltuk = (int*) malloc(sizeof(int)*sandalyeSayisi);   // Belirlenen Eleman Kadar Koltuk Dizisini Olu�turduk


	    pthread_t berber[koltukSayisi], musteri[musteriSayisi]; // Berber ve m��teri thread de�i�kenlerimizi olu�turduk

	    sem_init(&berber_sem, 0,0);
	    sem_init(&musteriler_sem, 0,0);  			// Sem init methoduyla semaforlar�m�z� ba�latt�k
	    sem_init(&mutex_sem, 0,1);


	    for(int i=0; i < koltukSayisi;i++)
	    {
		pthread_create(&berber[i],NULL, (void*)Berber,(void*)&i);  // Berber thread'lerini pthread_create methodu ile olu�turduk
		sleep(1);
	    }
	    for(int i=0; i < musteriSayisi;i++)
	    {
		pthread_create(&musteri[i],NULL, (void*)Musteri,(void*)&i);  // M��teri thread'lerini pthread_create methodu ile olu�turduk
		sleep(1);
	    }
	    for(int i=0; i < musteriSayisi;i++)			// T�m M��terilerin i�lemlerinin bitmesini bekliyoruz
	    {
		pthread_join(musteri[i],NULL);
		sleep(1);
	    }
    	}
    	else if(musteriSayisi==0 )				// Girilen m��teri say�s� 0 ise berber uyumaya devam eder
    	{
	    printf("\nBerber Uyuyor.\n\n");
    	}
	    
    }while(1);
    
}
