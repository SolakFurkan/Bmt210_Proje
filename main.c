#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define KRITIK_STOK_ESIGI 10
#define MAX_HEAP_BOYUT 100

typedef struct {                                                                                                        //Öncelikli kuyrukla kritik stok kontrolü yapıları
    int barkod;
    char ilacAdi[50];
    int stokAdedi;
} HeapDugum;

typedef struct {
    HeapDugum dugumler[MAX_HEAP_BOYUT];
    int boyut;
} MinHeap;

typedef struct Ilac {
    int barkodNo;
    char ilacAdi[50];
    int stokAdedi;
    float fiyat;
    int receteTipi;
    struct Ilac* sol;
    struct Ilac* sag;
} Ilac;

typedef struct Satis {
    int barkodNo;
    char ilacAdi[50];
    int adet;
    struct Satis* alt;
} Satis;

//Kuyruk sırası için
typedef struct Hasta {
    int siraNo;
    char hastaAdi[50];
    int barkod;
    struct Hasta* sonraki;
} Hasta;

typedef struct {
    Hasta* on;
    Hasta* arka;
    int boyut;
    int toplamSiraNo;
} ReceteKuyrugu;

float gunlukCiro = 0.0;
int toplamSatisAdedi = 0;
Satis* sonSatislar = NULL;
ReceteKuyrugu receteKuyrugu = {NULL, NULL, 0, 0};
MinHeap kritikHeap = {{}, 0};

int sayiAl(const char* mesaj) {
    int sayi;
    printf("%s", mesaj);

    while (scanf("%d", &sayi) != 1) {
        printf("Hatali giris.\nLutfen sadece sayi giriniz: ");
        while(getchar() != '\n');
    }
    return sayi;
}

Ilac* ilacEkle(Ilac* kok, int barkod, const char* isim, int stok, float fiyat, int recete) {
    if (kok == NULL) {
        Ilac* yeniIlac = (Ilac*)malloc(sizeof(Ilac));
        if (yeniIlac == NULL) {
            return NULL;
        }
        yeniIlac->barkodNo = barkod;
        strcpy(yeniIlac->ilacAdi, isim);
        yeniIlac->stokAdedi = stok;
        yeniIlac->fiyat = fiyat;
        yeniIlac->receteTipi = recete;
        yeniIlac->sol = NULL;
        yeniIlac->sag = NULL;
        return yeniIlac;
    }
    if (barkod < kok->barkodNo) {
        kok->sol = ilacEkle(kok->sol, barkod, isim, stok, fiyat, recete);
    }
    else if (barkod > kok->barkodNo) {
        kok->sag = ilacEkle(kok->sag, barkod, isim, stok, fiyat, recete);
    }
    return kok;
}

Ilac* ilacAra(Ilac* kok, int barkod) {
    if (kok == NULL || kok->barkodNo == barkod) {
        return kok;
    }
    if (barkod < kok->barkodNo) {
        return ilacAra(kok->sol, barkod);
    }
    return ilacAra(kok->sag, barkod);
}

Ilac* minDeger(Ilac* dugum) {
    Ilac* guncel = dugum;
    while (guncel && guncel->sol != NULL)
        guncel = guncel->sol;
    return guncel;
}

Ilac* ilacSil(Ilac* kok, int barkod) {

    if (kok == NULL)
        return kok;

    if (barkod < kok->barkodNo)
        kok->sol = ilacSil(kok->sol, barkod);

    else if (barkod > kok->barkodNo)
        kok->sag = ilacSil(kok->sag, barkod);

    else {
        if (kok->sol == NULL) {
            Ilac* gecici = kok->sag;
            free(kok);
            return gecici;
        }else if (kok->sag == NULL) {
            Ilac* gecici = kok->sol;
            free(kok);
            return gecici;
        }
        Ilac* gecici = minDeger(kok->sag);
        kok->barkodNo = gecici->barkodNo;
        strcpy(kok->ilacAdi, gecici->ilacAdi);
        kok->stokAdedi = gecici->stokAdedi;
        kok->fiyat = gecici->fiyat;
        kok->receteTipi = gecici->receteTipi;
        kok->sag = ilacSil(kok->sag, gecici->barkodNo);
    }
    return kok;
}

void agaciTemizle(Ilac* kok) {
    if (kok != NULL) {
        agaciTemizle(kok->sol);
        agaciTemizle(kok->sag);
        free(kok);
    }
}

Ilac* dosyadanOku(Ilac* kok) {
    FILE* dosya = fopen("eczane.txt", "r");

    if (dosya == NULL) {
        printf("\n Dosya bulunamadi,envanter bos\n");
        return kok;
    }

    int barkod, stok, recete;
    float fiyat;
    char isim[50];

    while (fscanf(dosya, "%d,%[^,],%d,%f,%d", &barkod, isim, &stok, &fiyat, &recete) == 5) {
        kok = ilacEkle(kok, barkod, isim, stok, fiyat, recete);
    }
    fclose(dosya);
    return kok;
}

void dosyayaYaz(Ilac* kok, FILE* dosya) {
    if (kok != NULL) {
        fprintf(dosya, "%d,%s,%d,%.2f,%d\n", kok->barkodNo, kok->ilacAdi, kok->stokAdedi, kok->fiyat, kok->receteTipi);
        dosyayaYaz(kok->sol, dosya);
        dosyayaYaz(kok->sag, dosya);
    }
}

void ilacGuncelle(Ilac* kok) {
    int barkod;
    barkod = sayiAl("Guncellenecek Ilacin Barkodunu Girin: ");

    Ilac* bulunan = ilacAra(kok, barkod);
    if (bulunan == NULL) {
        printf(" %d barkodlu ilac bulunamadi.\n", barkod);
        return;
    }

    int secim;
    char receteAdi[25];

     if (bulunan -> receteTipi == 0)
        strcpy(receteAdi, "Normal Recete");
    else if (bulunan->receteTipi == 1)
        strcpy(receteAdi, "Yesil Recete");
    else if (bulunan->receteTipi == 2)
        strcpy(receteAdi, "Kirmizi Recete");
         else
        strcpy(receteAdi, "Bilinmeyen Recete");


    printf("\n     Mevcut Bilgiler  \n");
    printf("Ad: %s | Stok: %d | Fiyat: %.2f TL | Tip: %s\n",bulunan->ilacAdi, bulunan->stokAdedi, bulunan->fiyat, receteAdi);
    secim = sayiAl("1. Stok Ekle\n2. Fiyat Guncelle\nSecim: ");

    if (secim == 1) {
        int yeniStok;

        yeniStok = sayiAl("Kac adet eklendi : ");
        bulunan->stokAdedi += yeniStok;
        printf(" Stok guncellendi. Yeni Stok: %d\n", bulunan->stokAdedi);
    } else if (secim == 2) {
        float yeniFiyat;
        printf("Yeni Satis Fiyati : ");
        while (scanf("%f", &yeniFiyat) != 1 || yeniFiyat<=0) {                                                    //Pozitif fiyat kontrolü eklendi
            printf(" Hatali giris.\n Lutfen gecerli bir fiyat giriniz : ");
            while(getchar() != '\n');
        }
        bulunan->fiyat = yeniFiyat;
        printf(" Fiyat guncellendi.\n");
    } else {
        printf("Gecersiz secim.\n");
    }
}

void ilacArama(Ilac* kok, const char* arananAd, int* bulunduMu) {

    if (kok != NULL) {
        ilacArama(kok->sol, arananAd, bulunduMu);

        if (strstr(kok->ilacAdi, arananAd) != NULL) {
            char tip[15];
            if (kok->receteTipi == 1) {
                strcpy(tip, "YESIL");
            }
            else if (kok->receteTipi == 2) {
                strcpy(tip, "KIRMIZI");
            }
            else strcpy(tip, "NORMAL");

            printf(" Ilac: %-15s |  Barkod: %-6d | Stok: %-3d | Fiyat: %6.2f TL | [%s]\n",
                   kok->ilacAdi, kok->barkodNo, kok->stokAdedi, kok->fiyat, tip);
            *bulunduMu = 1;
        }
        ilacArama(kok->sag, arananAd, bulunduMu);
    }
}

void heapSwap(HeapDugum* a, HeapDugum* b) {                                                                             //Kritik stok fonksiyonları
    HeapDugum temp = *a;
    *a = *b;
    *b = temp;
}

void heapYukariTasi(MinHeap* heap, int i) {
    while (i > 0) {
        int ata = (i - 1) / 2;
        if (heap->dugumler[ata].stokAdedi > heap->dugumler[i].stokAdedi) {
            heapSwap(&heap->dugumler[ata], &heap->dugumler[i]);
            i = ata;
        } else break;
    }
}

void heapAsagiTasi(MinHeap* heap, int i) {
    int en_kucuk = i;
    int sol = 2 * i + 1;
    int sag = 2 * i + 2;

    if (sol < heap->boyut &&
        heap->dugumler[sol].stokAdedi < heap->dugumler[en_kucuk].stokAdedi)
        en_kucuk = sol;

    if (sag < heap->boyut &&
        heap->dugumler[sag].stokAdedi < heap->dugumler[en_kucuk].stokAdedi)
        en_kucuk = sag;

    if (en_kucuk != i) {
        heapSwap(&heap->dugumler[i], &heap->dugumler[en_kucuk]);
        heapAsagiTasi(heap, en_kucuk);
    }
}

void heapeEkle(MinHeap* heap, int barkod, const char* ad, int stok) {
    // Aynı barkod zaten heap'teyse güncelle
    for (int i = 0; i < heap->boyut; i++) {
        if (heap->dugumler[i].barkod == barkod) {
            heap->dugumler[i].stokAdedi = stok;
            heapYukariTasi(heap, i);
            heapAsagiTasi(heap, i);
            return;
        }
    }

    if (heap->boyut >= MAX_HEAP_BOYUT) return;

    HeapDugum yeni;
    yeni.barkod = barkod;
    strcpy(yeni.ilacAdi, ad);
    yeni.stokAdedi = stok;

    heap->dugumler[heap->boyut] = yeni;
    heapYukariTasi(heap, heap->boyut);
    heap->boyut++;
}

void kritikStoklariGoster(MinHeap* heap) {
    if (heap->boyut == 0) {
        printf("\n Kritik stok uyarisi bulunmuyor.\n");
        return;
    }

    printf("\n--- KRITIK STOK UYARILARI (En Az Stoklu Once) ---\n");
    // Heap'i bozmadan sıralı yazdır
    MinHeap gecici = *heap;
    int sira = 1;

    while (gecici.boyut > 0) {
        HeapDugum en_az = gecici.dugumler[0];
        printf("%d. %-18s | Barkod: %-6d | Stok: %d adet\n",
               sira++, en_az.ilacAdi, en_az.barkod, en_az.stokAdedi);

        gecici.dugumler[0] = gecici.dugumler[gecici.boyut - 1];
        gecici.boyut--;
        heapAsagiTasi(&gecici, 0);
    }
}

// Tüm envanteri tarayıp kritik olanları heap'e yükler
void kritikStokTara(Ilac* kok, MinHeap* heap) {
    if (kok == NULL) return;
    kritikStokTara(kok->sol, heap);

    if (kok->stokAdedi < KRITIK_STOK_ESIGI)
        heapeEkle(heap, kok->barkodNo, kok->ilacAdi, kok->stokAdedi);

    kritikStokTara(kok->sag, heap);
}

void ilacSat(Ilac* kok, int barkod) {
    Ilac* satilacak = ilacAra(kok,barkod);

    if (satilacak == NULL) {
        printf("\n Barkod bulunamadi.\n");
        return;
    }

    int adet = 1;

    if (satilacak->receteTipi == 0) {
        adet = sayiAl("Kac adet almak istiyorsunuz?: ");

        if (adet <= 0) {
            printf("Gecersiz adet.\n");
            return;
        }
    }

    if (satilacak->receteTipi > 0) {
        int cevap;
        printf("\nBu ilac icin recete gerekiyor.\n");
        printf("Recete mevcut mu?\n");
        cevap = sayiAl("1 - Evet\n2 - Hayir\nSecim: ");

        if (cevap != 1) {
            printf("Satis iptal edildi.\n");
            return;
        }
        adet = 1;
    }

    if (satilacak->stokAdedi < adet) {
        printf("Yetersiz stok!\n");
        return;
    }

    satilacak->stokAdedi -= adet;

    float toplamTutar = adet * satilacak->fiyat;
    gunlukCiro += toplamTutar;
    toplamSatisAdedi += adet;

    printf("\n  SATIS BASARILI: %s | %d adet | Tutar: %.2f TL\n",satilacak->ilacAdi, adet, toplamTutar);

    // Stok kritik eşiğin altına düştüyse heap'e ekle
    if (satilacak->stokAdedi < KRITIK_STOK_ESIGI) {
        heapeEkle(&kritikHeap, satilacak->barkodNo,
                   satilacak->ilacAdi, satilacak->stokAdedi);
        printf(" UYARI: %s icin stok kritik seviyede! (%d adet kaldi)\n",
               satilacak->ilacAdi, satilacak->stokAdedi);
    }

    Satis* yeniSatis = (Satis*)malloc(sizeof(Satis));
    yeniSatis->barkodNo = satilacak->barkodNo;
    strcpy(yeniSatis->ilacAdi, satilacak->ilacAdi);
    yeniSatis->adet = adet;
    yeniSatis->alt = sonSatislar;
    sonSatislar = yeniSatis;
}

void satisiIptalEt(Ilac* kok,MinHeap* heap) {
    if (sonSatislar == NULL) {
        printf("\nIptal edilecek bir islem yok.\n");
        return;
    }

    Satis* iptal = sonSatislar;
    sonSatislar = sonSatislar->alt;

    Ilac* iadeIlac = ilacAra(kok, iptal->barkodNo);
    if (iadeIlac != NULL) {

        iadeIlac->stokAdedi += iptal->adet;

        gunlukCiro -= iptal->adet * iadeIlac->fiyat;
        toplamSatisAdedi -= iptal->adet;

        printf("\n   Iade Tamamlandı: %s | %d adet geri alindi.\n",iptal->ilacAdi, iptal->adet);
    }

    // Stok eşiğin üstüne çıktıysa heap'i güncelle
    if (iadeIlac->stokAdedi >= KRITIK_STOK_ESIGI) {
        // Heap'ten çıkar — yeniden tara
        kritikHeap.boyut = 0;
        kritikStokTara(kok, heap);
    }
    free(iptal);
}

void kuyruğaEkle(ReceteKuyrugu* kuyruk, const char* hastaAdi, int barkod) {                                             //Hasta kuyruğu için
    Hasta* yeni = (Hasta*)malloc(sizeof(Hasta));
    kuyruk->toplamSiraNo++;
    yeni->siraNo = kuyruk->toplamSiraNo;
    strcpy(yeni->hastaAdi, hastaAdi);
    yeni->barkod = barkod;
    yeni->sonraki = NULL;

    if (kuyruk->arka == NULL) {
        kuyruk->on = yeni;
        kuyruk->arka = yeni;
    } else {
        kuyruk->arka->sonraki = yeni;
        kuyruk->arka = yeni;
    }
    kuyruk->boyut++;
    printf("\n Hasta kuyroguna alindi. Sira No: %d | Hasta: %s\n",
           yeni->siraNo, yeni->hastaAdi);
}

void kuyruktenCikar(ReceteKuyrugu* kuyruk, Ilac* root) {
    if (kuyruk->on == NULL) {
        printf("\n Kuyrukta bekleyen hasta yok.\n");
        return;
    }

    Hasta* islenen = kuyruk->on;
    kuyruk->on = kuyruk->on->sonraki;
    if (kuyruk->on == NULL)
        kuyruk->arka = NULL;
    kuyruk->boyut--;

    printf("\n Islenen hasta: %s | Barkod: %d\n",
           islenen->hastaAdi, islenen->barkod);

    ilacSat(root, islenen->barkod);
    free(islenen);
}

void kuyruguListele(ReceteKuyrugu* kuyruk) {
    if (kuyruk->on == NULL) {
        printf("\n Kuyruk bos.\n");
        return;
    }
    printf("\n--- Recete Bekleme Sirasi --\n");
    Hasta* gecici = kuyruk->on;
    int sira = 1;
    while (gecici != NULL) {
        printf("%d. Sira | Hasta: %-15s | Barkod: %d\n",
               sira++, gecici->hastaAdi, gecici->barkod);
        gecici = gecici->sonraki;
    }
    printf("Toplam Bekleyen: %d hasta\n", kuyruk->boyut);
}

void gunSonuRaporu(Ilac* kok) {
    printf("\n       GUN SONU RAPORU    \n");

    if (sonSatislar == NULL) {
        printf("Bugun hic satis yapilmadi.\n");
    } else {
        printf("   Bugun Yapilan Satislar:  \n");
        Satis* gecici = sonSatislar;

        while (gecici != NULL) {
            Ilac* satilanIlac = ilacAra(kok, gecici->barkodNo);
            float islemTutari = 0.0;

            if (satilanIlac != NULL) {
                islemTutari = gecici->adet * satilanIlac->fiyat;
            }

            printf(" --> %s | %2d Adet | Islem Tutari: %.2f TL\n", gecici->ilacAdi, gecici->adet, islemTutari);
            gecici = gecici->alt;
        }
    }

    printf("\nToplam Satilan Urun Adedi : %d\n", toplamSatisAdedi);
    printf("Gunluk Ciro        : %.2f TL\n", gunlukCiro);
}

void envanterListele(Ilac* kok) {
    if (kok != NULL) {
        envanterListele(kok->sol);
        char tip[15];
        if (kok->receteTipi == 1) strcpy(tip, "YESIL");
        else if (kok->receteTipi == 2) strcpy(tip, "KIRMIZI");
        else strcpy(tip, "NORMAL");

        printf("Barkod: %-6d | Ilac: %-18s | Stok: %-4d | Fiyat: %6.2f TL | [%s]\n",
               kok->barkodNo, kok->ilacAdi, kok->stokAdedi, kok->fiyat, tip);
        envanterListele(kok->sag);
    }
}

void performans() {
    printf("\n    PERFORMANS ANALIZI (BST vs DIZI) \n");

    int veriSetleri[3] = {1000, 10000, 50000};
    srand(time(NULL));

    for (int v = 0; v < 3; v++) {

        int veriSayisi = veriSetleri[v];
        int tekrar = 100000;

        printf("\nVeri Sayisi %d icin karsilastirma :\n", veriSayisi);

        int* dizi = (int*)malloc(veriSayisi * sizeof(int));
        Ilac* testAgac = NULL;

        for (int i = 0; i < veriSayisi; i++) {
            int rastgele = rand() % 1000000;
            dizi[i] = rastgele;
            testAgac = ilacEkle(testAgac, rastgele, "Test", 1, 1.0, 0);
        }

        int aranan = dizi[veriSayisi / 2];

        clock_t basla, bitir;

        //  BST
        basla = clock();
        for (int i = 0; i < tekrar; i++) {
            ilacAra(testAgac, aranan);
        }
        bitir = clock();
        double bstSure = (double)(bitir - basla) *1000 / CLOCKS_PER_SEC;

        //  DIZI
        basla = clock();
        for (int k = 0; k < tekrar; k++) {
            for (int i = 0; i < veriSayisi; i++) {
                if (dizi[i] == aranan)
                    break;
            }
        }
        bitir = clock();
        double diziSure = (double)(bitir - basla) *1000/ CLOCKS_PER_SEC;

        printf("BST Sure  : %.4f milisaniye\n", bstSure);
        printf("Dizi Sure : %.4f milisaniye\n", diziSure);

        if (bstSure > 0)
            printf("Hiz Orani : %.2f kat\n", diziSure / bstSure);

        free(dizi);
        agaciTemizle(testAgac);
    }
}

int main() {
    Ilac* root = NULL;
    int secim, barkod, stok, rTip;
    float fiyat;
    char ad[50];

    root = dosyadanOku(root);

    do {
        printf("\n\n        ECZANE STOK VE KASA YONETIM SISTEMI      \n");
        printf("---------------------------------------------------\n");
        printf("1. Yeni Ilac Kaydi  \n");
        printf("2. Ilac Kaydini Sil \n");
        printf("3. Stok/Fiyat Bilgisi Guncelle\n");
        printf("4. Ilac Ismine Gore Arama Yap \n");
        printf("5. Satis Yap \n");
        printf("6. Son Satisi Iptal Et (Iade Al)\n");
        printf("7. Ilac Listesi \n");
        printf("8. Sistem Performans Testi\n");
        printf("9. Gun Sonu Raporu\n");
        printf("10. Cikis\n");
        printf("11. Recete Kuyroguna Hasta Ekle\n");
        printf("12. Kuyruktan Hasta Isle\n");
        printf("13. Bekleme Sirasini Listele\n");
        printf("14. Kritik Stok Uyarilari\n");
        printf("Seciminiz: ");

        if (scanf("%d", &secim) != 1) {
            while (getchar() != '\n');
            printf("Lutfen sadece rakam giriniz. \n");
            continue;
        }

        switch(secim) {
            case 1:
                barkod = sayiAl("Yeni Barkod: ");
                if(ilacAra(root, barkod)) {
                    printf(" Bu barkod zaten kayitli.\n");
                } else {
                    printf("Ilac Adi: ");
                    scanf(" %49[^\n]", ad);                                                                       //Çok karakterli ad girildiğinde programın çökmemesi için sınır

                    stok = sayiAl("Stok: ");
                    printf("Fiyat: ");
                    while (scanf("%f", &fiyat) != 1 || fiyat<=0) {                                                //Pozitif fiyat kontrolüeklendi
                        printf("Hatali giris! Lutfen gecerli bir fiyat giriniz: ");
                        while(getchar() != '\n');
                    }
                    do {
                        rTip = sayiAl("Recete Tipi (0:Normal, 1:Yesil, 2:Kirmizi): ");
                        if (rTip < 0 || rTip > 2) {
                            printf("Lutfen sadece 0, 1 veya 2 rakamlarindan birini giriniz.\n");
                        }
                    } while (rTip < 0 || rTip > 2);

                    root = ilacEkle(root, barkod, ad, stok, fiyat, rTip);
                    printf("  Ilac basariyla eklendi.\n");
                }
                break;
            case 2:
                barkod = sayiAl("Silinecek Barkod: ");
                if(ilacAra(root, barkod)) {
                    root = ilacSil(root, barkod);
                    printf(" Kayit silindi.\n");
                } else
                    printf("Kayit bulunamadı.\n");
                break;
            case 3:
                ilacGuncelle(root);
                break;
            case 4: {
                char arananIsim[50];
                int bulunduMu = 0;
                printf("Aradiginiz ilacin ismini veya bir kismini girin: ");
                scanf(" %49[^\n]", arananIsim);                                                                   //Ayrılan bellek alanını aşmaması için sınır

                printf("\n '%s' Icin Arama Sonuclari :\n", arananIsim);
                ilacArama(root, arananIsim, &bulunduMu);

                if (bulunduMu == 0) {
                    printf("Sistemde bu ismi iceren bir ilac bulunamadi.\n");
                }
                break;
            }
            case 5:
                barkod = sayiAl("Barkodu Okutun: ");
                ilacSat(root, barkod);
                break;
            case 6:
                satisiIptalEt(root, &kritikHeap);
                break;
            case 7:
                printf("\n--- Mevcut Envanter Listesi ---\n");
                envanterListele(root);
                break;
            case 8:
                performans();
                break;
            case 9:
                gunSonuRaporu(root);
                break;
            case 10:
                printf("\nVeriler 'eczane.txt' dosyasina kaydediliyor.\n");
                FILE* dosya = fopen("eczane.txt", "w");
                if(dosya) {
                    dosyayaYaz(root, dosya);
                    fclose(dosya);
                }
                break;
            case 11: {
                char hastaAdi[50];
                printf("Hasta Adi: ");
                scanf(" %49[^\n]", hastaAdi);
                barkod = sayiAl("Ilac Barkodu: ");
                kuyruğaEkle(&receteKuyrugu, hastaAdi, barkod);
                break;
            }
            case 12:
                kuyruktenCikar(&receteKuyrugu, root);
                break;
            case 13:
                kuyruguListele(&receteKuyrugu);
                break;
            case 14:
                kritikStokTara(root, &kritikHeap);  // güncel tarama yap
                kritikStoklariGoster(&kritikHeap);
                break;
            default:
                printf("Gecersiz secim.\n");
        }
    } while (secim != 10);

    agaciTemizle(root);
    while (receteKuyrugu.on != NULL) {
        Hasta* silinecek = receteKuyrugu.on;
        receteKuyrugu.on = receteKuyrugu.on->sonraki;
        free(silinecek);
    }

    while (sonSatislar != NULL) {
        Satis* silinecek = sonSatislar;
        sonSatislar = sonSatislar -> alt;
        free(silinecek);
    }
    return 0;
}