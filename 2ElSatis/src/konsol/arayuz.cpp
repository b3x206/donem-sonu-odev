// -*- encoding: utf-8 -*-

#include "arayuz.h"

#include <iostream>        // std::cout, std::cerr
#include <vector>          // std::vector
#include <string>          // std::string
#include <iomanip>         // std::setw, std::setfill
#include <type_traits>     // std::_Is_invocable_r

using namespace std;

// @brief Önceki arayüz eğlemi.
Eglem OncekiEglem;
// @brief Erişilecek ana grup.
ArayuzGrubu AnaGrup;

// --------
// Yardımcı fonksiyonlar
// --------

size_t ipucuBoyutu = 0;        // Eleman ID ipucu indisi.

// @brief Parametre işaretcilerinden ID hesaplar.                                                                                              
// FIXME : Release build alınca bu fonksiyon farklı değerler döndürüyor! Onun yerine arayüz yapılmadan önce bir ID dizisi objeleri tutulmalı 
// (kullanan kodda static vector<ArayuzEleman> = ArayuzBasla()) ve benzerlik bulunmalı.
// Program zaten Debug buildde yeterince hızlı çalışıyor (directx veya opengl uygulaması değil sonuçta) ama Release'in başka iyilikleride olabilir.
// @param
//     parametreSayisi  [in] : Verilen parametre sayısı. İlk parametreden sonraki parametre sayısını belirtir.           
//     ptrs (va)        [in] : Parametre işaretcileri. Verilen parametrelerden işaretcileri al.
// @returns Hesaplanan ID. Aynı fonksiyonun içinde aynı parametrelerle çağırılırsa aynı ID'yi döndürür.
size_t ArayuzFonksiyonuIDHesapla(int parametreSayisi, const void* ptrs...)
{
    // C Variadic parametresi : Birden çok parametre içerebilir, parametre sayısı bilinmiyor.
    // Tanımlanması : bir basit işaretci tipinden sonra ad tanımladıktan sonra üç nokta (...) koymak
    size_t id = 0;

    // variadic liste 'l' işaretcisini başlat (ptrs parametresi ile)
    va_list l;
    va_start(l, ptrs);

    for (int i = 0; i < parametreSayisi; i++)
    {
        // void* işaretci adreslerini al ve ID'ye topla.
        // Böylelikle parametrelerin eşitliğini kontrol et.
        id += va_arg(l, size_t);
    }
    // liste işaretcisini 'l' bitir.
    va_end(l);

    size_t retDegeri = id + ipucuBoyutu;
    ipucuBoyutu = 0; // Verilen ipucuyu Sıfırla

    return retDegeri;
}
// @brief Eleman ID'sinden hedef elemanı bulur.
// @param
//     elemanID    [in]  : Elemanın 'ArayuzFonksiyonuIDHesapla' kullanılarak hesaplanan ID'si. Her 'arayuz.cpp'deki arayüz fonksiyonu bunu hesaplamalı. (yoksa hatalı durum olabilir)
// @returns Hedef eleman bulunamaz ise yeni bir 'ArayuzElemani' döndürür.
ArayuzElemani& IDDenHedefEleman(size_t elemanID)
{
    // Ara
    size_t elemanIndis = -1;
    for (size_t i = 0; i < AnaGrup.elemanlar.size(); i++)
    {
        if (AnaGrup.elemanlar.at(i).id == elemanID)
        {
            elemanIndis = i;
            break;
        }
    }

    // Arama bulunamadı ise yeni eleman yap.
    if (elemanIndis == -1)
    {
        ArayuzElemani e;
        e.id = elemanID;
        e.indis = AnaGrup.elemanlar.size(); // İndis => push_back çağırmadan önceki boyut.
        elemanIndis = e.indis;

        // 'push_back' yapıldığında eleman, vector'un iç işaretcisine kopyalanır
        // 'eleman'ı işaretci olarak returnleyebilirdik (sonuçta eleman referansı alıyoruz) ama vectore pushleyerek .at fonksiyonu ile almak daha garanti.
        AnaGrup.elemanlar.insert(AnaGrup.elemanlar.end(), e);
    }

    AnaGrup.oncekiElemanIndisi = elemanIndis;
    return AnaGrup.elemanlar.at(elemanIndis);
}
// @brief Verilen elemandan bir renk döndürür.
WORD ArayuzElemaniRenkAl(const ArayuzElemani& e)
{
    bool secili = AnaGrup.seciliArayuzElemani == e.indis;
    bool inaktif = !e.secilebilirEleman;

    if (inaktif)
    {
        return AnaGrup.inaktifRenk;
    }
    else if (secili)
    {
        return AnaGrup.seciliRenk;
    }

    return AnaGrup.varsayilanRenk;
}

// --------
// Arayüz Management
// --------

void ElemanIDIpucu(const size_t ipucu)
{
    ipucuBoyutu += ipucu;
}
bool ElemanGeri(const bool dongu)
{
    Eglem e;
    size_t oncekiSeciliEleman = AnaGrup.seciliArayuzElemani;
    size_t hedefEleman = AnaGrup.seciliArayuzElemani;
    // Sadece Seçilebilir elemanlara atla
    size_t i = 0;
    do
    {
        if (i == AnaGrup.elemanlar.size() - 1)
        {
            // Son seçilecek elemandayız, büyük ihtimalle seçilebilir bir eleman yok arayüzde
            hedefEleman = oncekiSeciliEleman;
            break;
        }

        hedefEleman = (hedefEleman - 1) == -1 ? (dongu ? AnaGrup.elemanlar.size() - 1 : 0) : hedefEleman - 1;
        i++;
    } while (!AnaGrup.elemanlar.at(hedefEleman).secilebilirEleman);

    // Hedef değişmez ise eğlem fonksiyonlarını çağırma.
    if (hedefEleman == oncekiSeciliEleman)
    {
        return false;
    }

    OncekiEglem.tip = Eglem::SecimKaldirma;
    OncekiEglem.secilenEleman = oncekiSeciliEleman;
    AnaGrup.seciliArayuzElemani = hedefEleman;

    return true;
}
bool ElemanIleri(const bool dongu)
{
    size_t oncekiSeciliEleman = AnaGrup.seciliArayuzElemani;
    size_t hedefEleman = AnaGrup.seciliArayuzElemani;

    // Sadece Seçilebilir elemanlara atla
    size_t i = 0;
    do
    {
        if (i == AnaGrup.elemanlar.size() - 1)
        {
            // Son seçilecek elemandayız, büyük ihtimalle seçilebilir bir eleman yok arayüzde
            hedefEleman = oncekiSeciliEleman;
            break;
        }

        hedefEleman = hedefEleman + 1 >= AnaGrup.elemanlar.size() ? (dongu ? 0 : AnaGrup.seciliArayuzElemani) : hedefEleman + 1;
        i++;
    } while (!AnaGrup.elemanlar.at(hedefEleman).secilebilirEleman);

    // Hedef değişmez ise eğlem fonksiyonlarını çağırma.
    if (hedefEleman == oncekiSeciliEleman)
    {
        return false;
    }

    OncekiEglem.tip = Eglem::SecimKaldirma;
    OncekiEglem.eglemAliciEleman = oncekiSeciliEleman;
    AnaGrup.seciliArayuzElemani = hedefEleman;

    return true;
}
bool ElemanSec(const int indis)
{
    // eleman indis dışı
    if (indis < 0 || indis >= AnaGrup.elemanlar.size())
    {
        return false;
    }

    size_t oncekiSeciliEleman = AnaGrup.seciliArayuzElemani;
    ArayuzElemani eleman = AnaGrup.elemanlar.at(indis);

    // 'indis' teki eleman seçilemez
    if (!eleman.secilebilirEleman)
    {
        return false;
    }
    // Zaten seçili
    if (AnaGrup.seciliArayuzElemani == indis)
    {
        return true;
    }

    // Seçim eğlemini ayarla
    OncekiEglem.tip = Eglem::SecimKaldirma;
    OncekiEglem.secilenEleman = indis;
    OncekiEglem.eglemAliciEleman = oncekiSeciliEleman;
    AnaGrup.seciliArayuzElemani = indis;

    return true;
}
size_t ElemanlarBoyut()
{
    return AnaGrup.elemanlar.size();
}

size_t arayuzIzgaraXBoyutu = 0;
void ArayuzIzgaraNav(const size_t sutunSayisi)
{
    arayuzIzgaraXBoyutu = sutunSayisi;
}
bool ArayuzElemanSil(size_t indis, bool arayuzSil, const char dolgu)
{
    if (indis >= AnaGrup.elemanlar.size() || indis == -1)
    {
        return false;
    }

    ArayuzElemani e = AnaGrup.elemanlar.at(indis); // kopya, referans değil!
    // asıl 'ArayuzElemani'nı sil
    std::vector<ArayuzElemani>::iterator it = AnaGrup.elemanlar.begin();
    std::advance(it, indis);
    AnaGrup.elemanlar.erase(it);
    
    if (AnaGrup.seciliArayuzElemani == indis)
    {
        AnaGrup.seciliArayuzElemani = AnaGrup.seciliArayuzElemani <= 0 ? 0 : AnaGrup.seciliArayuzElemani - 1;
    }

    if (arayuzSil)
    {
        if (e.alan.x < 0 || e.alan.y < 0 || e.alan.genislik < 0 || e.alan.yukseklik < 0)
        {
            // Silinecek bir alan yok
            return true;
        }

        // Alan silme, 'y' değişkeni olan y pozisyonunu bildirir
        // setw ve setfill ile boş yazı yazdırarak sil.
        for (size_t y = e.alan.y; y < e.alan.y + e.alan.yukseklik; y++)
        {
            KonsolImlecPozisyonuAyarla(e.alan.x, y);
            // Bazı çizilen şeyler alanı 1 karakter daha geniş varsayabilir.
            cout << setw(e.alan.genislik + 1) << setfill(dolgu) << ""; // Dolguyu her Y pozisyonu için yazdır.
        }
    }


    return true;
}

void ArayuzTemizle(bool arayuzSil, const char dolgu)
{
    if (arayuzSil)
    {
        while (!AnaGrup.elemanlar.empty())
        {
            ArayuzElemanSil(0, arayuzSil, dolgu);
        }
    }

    arayuzIzgaraXBoyutu = 0;
    KonsolRenkAyarla(AnaGrup.varsayilanRenk);
    KonsolImlecPozisyonuAyarla(0, 0);
    AnaGrup.seciliArayuzElemani = 0;
}

void OncekiCizilenArayuzElemanSil(bool arayuzSil, const char dolgu)
{
    ArayuzElemanSil(AnaGrup.oncekiElemanIndisi, arayuzSil, dolgu);

    AnaGrup.oncekiElemanIndisi = AnaGrup.oncekiElemanIndisi <= 0 ? 0 : AnaGrup.oncekiElemanIndisi - 1;
}

Eglem EglemleriAl(const size_t hedefIndis, const bool kullan)
{
    Eglem e;
    e.eglemAliciEleman = hedefIndis;
    if (!AnaGrup.elemanlar.at(AnaGrup.seciliArayuzElemani).secilebilirEleman)
    {
        // Yanlış bir elemandayız (bu fonksiyon çağırılmazsa zaten çoğu eleman seçilebilir değil)
        ElemanIleri(true);
        return OncekiEglem;
    }

    // Eğer OncekiEglem'in tipi SecimKaldirma ve sonraki çağırmada eğlem kullanılırsa
    if (OncekiEglem.tip == Eglem::SecimKaldirma)
    {
        // Seçilen elemanı ara.
        if (hedefIndis == OncekiEglem.secilenEleman)
        {
            if (!kullan)
            {
                return e;
            }

            e.tip = Eglem::Secim;
            e.secilenEleman = AnaGrup.seciliArayuzElemani;

            OncekiEglem = e;
            return e;
        }
        // Eğer tek bir eleman var ise seçim kaldırmayı çağır
        // >-- Bu büyük ihtimalle tek bir arayüz elemanı ile döngüye giricektir.
        else if (hedefIndis == OncekiEglem.eglemAliciEleman)
        {
            if (!kullan)
            {
                return e;
            }

            e.tip = Eglem::SecimKaldirma;
            e.secilenEleman = AnaGrup.seciliArayuzElemani;

            // Bu sebeple döngüye girmemesi için önceki eğlemi modifiye et
            // Bu sadece bu eğlem moduna has bir istisna. --<
            Eglem sahteOncekiEglem = e;
            sahteOncekiEglem.tip = Eglem::Yok;
            sahteOncekiEglem.secilenEleman = -1;
            OncekiEglem = sahteOncekiEglem;

            return e;
        }
    }

    // Sadece seçili elemanlarda klavye eğlemlerini al
    // Seçili olmayan elemanlar eğlemleri etkileyemez.
    if (AnaGrup.seciliArayuzElemani != hedefIndis)
    {
        return e;
    }

    // Konsol eğlemlerini al
    INPUT_RECORD anaKonsolEglem;
    KonsolEglem(&anaKonsolEglem);

    if (anaKonsolEglem.EventType == KEY_EVENT)
    {
        // Tuş eğlemini al
        KEY_EVENT_RECORD tusEglem = anaKonsolEglem.Event.KeyEvent;

        if (tusEglem.bKeyDown)
        {
            e.tip = Eglem::KlayveBas;
            e.ctrlBasili = (tusEglem.dwControlKeyState & LEFT_CTRL_PRESSED) == LEFT_CTRL_PRESSED ||
                (tusEglem.dwControlKeyState & RIGHT_CTRL_PRESSED) == RIGHT_CTRL_PRESSED;
            e.altBasili = (tusEglem.dwControlKeyState & LEFT_ALT_PRESSED) == LEFT_ALT_PRESSED ||
                (tusEglem.dwControlKeyState & RIGHT_ALT_PRESSED) == RIGHT_ALT_PRESSED;
            e.shiftBasili = (tusEglem.dwControlKeyState & SHIFT_PRESSED) == SHIFT_PRESSED;

            switch (tusEglem.uChar.UnicodeChar)
            {
                // Özel tuş (tusEglem.wVirtualKeyCode) + özel tuş olmalı
            case 0:
                if (tusEglem.wVirtualKeyCode != 0)
                {
                    if (AnaGrup.elemanlar.at(AnaGrup.seciliArayuzElemani).okTusuBasilabilir &&
                        (arayuzIzgaraXBoyutu > 0))
                    {
                        switch (tusEglem.wVirtualKeyCode)
                        {
                        case VK_LEFT:
                            ElemanGeri(false);
                            return OncekiEglem;
                        case VK_RIGHT:
                            ElemanIleri(false);
                            return OncekiEglem;

                        case VK_UP:
                            // Azalan boyutu sınırlandır
                            ElemanSec(arayuzIzgaraXBoyutu >= AnaGrup.seciliArayuzElemani ? 0 : AnaGrup.seciliArayuzElemani - arayuzIzgaraXBoyutu);
                            return OncekiEglem;
                        case VK_DOWN:
                            // Yükselen boyutu sınırlandır
                            ElemanSec(AnaGrup.seciliArayuzElemani + arayuzIzgaraXBoyutu >= AnaGrup.elemanlar.size() ? AnaGrup.elemanlar.size() - 1 : AnaGrup.seciliArayuzElemani + arayuzIzgaraXBoyutu);
                            return OncekiEglem;

                        default:
                            break;
                        }
                    }

                    e.ozelTus = true;
                    e.basilanTus = tusEglem.wVirtualKeyCode;
                }
                break;

                // Normal tuşlar (esc, tab gibi tuşlar bunla bozulmuyor)
            default:
                if (tusEglem.uChar.UnicodeChar == AnaGrup.arayuzTusNavigasyonuHedef && AnaGrup.arayuzTekTusNavigasyonu)
                {
                    ElemanIleri(true);
                    return OncekiEglem; // 'ElemanIleri' çağırıldığında gerekli eğlem = 'OncekiEglem'
                }
                else
                {
                    // Alınan wchar_t karakteri utf8 karakter dizisine dönüştür
                    // SetConsoleOutputCP(CP_UTF8) kullandığımız için 'u8s' dizisini 'cout' ladığımızda düzgün karakterler çıkıyor.
                    char u8s[MB_LEN_MAX];
                    // wctomb, setlocale'yi göze alarak çalışıyor, bu sebepten dolayı türkçe karakterlerde 'setlocale' çağırılmadığı sürece hata veriyor.
                    // WideCharToMultiByte windows kütüphanesinden olduğu için düzgün çalışıyor
                    size_t chcount = WideCharToMultiByte(CP_UTF8, 0, &tusEglem.uChar.UnicodeChar, 1, u8s, MB_LEN_MAX, NULL, NULL);
                    // boyut kontrolü (chcount en fazla 4 olmalı ve overflow -1 olmamalı)
                    if (chcount >= MB_LEN_MAX)
                    {
                        string s = u8"[EglemleriAl] WideCharToMultiByte hatası : ";
                        s.append(to_string(errno));
                        throw exception(s.c_str());
                    }
                    u8s[chcount] = '\0'; // Son null karakterini koy.

                    // Dizileri kopyala
                    for (size_t i = 0; i <= chcount; i++)
                    {
                        e.basilanTusMB[i] = u8s[i];
                    }
                    e.basilanTus = tusEglem.uChar.UnicodeChar;
                }
                break;
            }
        }

        // Elemanlarda seçilebilir tuşlu ara.
        for (size_t i = 0; i < AnaGrup.elemanlar.size(); i++)
        {
            ArayuzElemani elem = AnaGrup.elemanlar.at(i);
            if (elem.secmeTusu == '\0') // Seçilemez eleman
            {
                continue;
            }

            if (e.ozelTus)
            {
                if (!elem.secmeTusuOzel)
                {
                    continue;
                }
            }

            if (elem.secmeTusu == e.basilanTus)
            {
                ElemanSec(elem.indis);
            }
        }
    }

    OncekiEglem = e;
    return e;
}

// --------
// Arayüz Eleman Fonksiyonları
// --------

// @brief Şu anki düzeltme işareti pozisyonu
// YaziAlani() fonksiyonunda kullanılır.
size_t DuzeltmeIsaretiIndex = 0;

// Ayıraç karakter listesi.
vector<char> ayirKarakter =
{
    '\n',
};
// @brief Yazıyı substr ile arayüz alanı için ayırır.
// utf8_substr fonksiyonunun aksine indis daha büyük ise boş string döndürür
// (hata vermek yerine) ve 'ayirKarakter' dizisindekileri ayıraç olarak varsayar.
string AlanUTF8StringAyir(const string& yazi, const size_t baslangicIndisi, const size_t boyut)
{
    if (yazi.empty())
    {
        return yazi;
    }

    if (baslangicIndisi >= yazi.size() - 1)
    {
        return string();
    }

    for (size_t i = 0; i < ayirKarakter.size(); i++)
    {
        // 'baslangicIndisi' eşit ise yine aynı pozisyondaki karakteri bir daha bulabilir.
        size_t ayirKarakterIndis = yazi.find(ayirKarakter.at(i), baslangicIndisi);
        if (ayirKarakterIndis == baslangicIndisi)
        {
            // Aynı indisteysek bu fonksiyonu 'baslangicIndisi' 1 kaymış şekilde çağır
            return AlanUTF8StringAyir(yazi, baslangicIndisi + 1, boyut);
        }

        if (ayirKarakterIndis != string::npos)
        {
            return yazi.substr(baslangicIndisi, ayirKarakterIndis - baslangicIndisi);
            //return utf8_substr(yazi, baslangicIndisi, ayirKarakterIndis - baslangicIndisi);
        }
    }

    return utf8_substr(yazi, baslangicIndisi, boyut);
}

// @brief Yazı alanı çizer.
const ArayuzElemani& YaziAlani(ArayuzAlan alan, string* yazi, const string& yerTutucu, bool sifreAlani, const char secTus, bool (*karakterCtrl)(const int64_t))
{
    size_t elemanID = ArayuzFonksiyonuIDHesapla(3, &alan, yazi, &yerTutucu);
    ArayuzElemani& eleman = IDDenHedefEleman(elemanID);
    eleman.alan = alan;
    eleman.okTusuBasilabilir = false;
    eleman.secilebilirEleman = AnaGrup.elemanlarSecilebilir;
    eleman.secmeTusu = secTus;

    Eglem e = EglemleriAl(eleman.indis, true);

    if (alan.x < 0 || alan.y < 0 || alan.genislik < 0 || alan.yukseklik < 0)
    {
        return eleman;
    }

    // bu fonksiyon ve bu iç fonksiyon her zaman çizim eğlemi için çağırılır
    KonsolImlec(false);
    KonsolImlecPozisyonuAyarla(alan.x, alan.y);
    WORD oncekiAnaKonsolRenk = KonsolRenk();
    KonsolRenkAyarla(ArayuzElemaniRenkAl(eleman));

    // Seçim kaldırılırsa düzeltme işaretini sil.
    if (e.tip == Eglem::SecimKaldirma)
    {
        WORD oncekiKonsolRengi = KonsolRenk();
        size_t isaretXIndex = (DuzeltmeIsaretiIndex % alan.genislik) + 1;              // işaret X pozisyon
        size_t isaretYIndex = DuzeltmeIsaretiIndex / (alan.genislik * alan.yukseklik); // işaret Y pozisyon
        KonsolImlecPozisyonuAyarla(alan.x + isaretXIndex, alan.y + isaretYIndex);
        KonsolRenkAyarla(oncekiKonsolRengi);
        cout << " "; // Duzeltme işareti rengini sil.

        return eleman;
    }

    // Klayvede bir karaktere basılırsa yazıyı modifiye et.
    if (e.tip == Eglem::KlayveBas)
    {
        // DuzeltmeIsaretiIndex değişkenini kontrol et
        if (DuzeltmeIsaretiIndex > yazi->size())
        {
            DuzeltmeIsaretiIndex = yazi->size();
        }

        // Ok tuşu değil
        if (!e.ozelTus)
        {
            // Fonksiyon işaretcisi yerine std::vector<char> kullan?
            if (karakterCtrl != nullptr && !karakterCtrl(e.basilanTus))
            {
                return eleman;
            }

            switch (e.basilanTus)
            {
                // Backspace
            case '\b':
            {
                if (yazi->empty() || DuzeltmeIsaretiIndex == 0)
                    break;

                bool indisOncedenDusur = DuzeltmeIsaretiIndex == yazi->size();
                if (indisOncedenDusur)
                {
                    // onceden 1 düşür
                    DuzeltmeIsaretiIndex -= 1;
                }

                size_t indisDusurSayisi = utf8_erase(*yazi, DuzeltmeIsaretiIndex, 1);
                if (indisOncedenDusur)
                {
                    indisDusurSayisi -= 1;
                }

                DuzeltmeIsaretiIndex = (DuzeltmeIsaretiIndex >= indisDusurSayisi && DuzeltmeIsaretiIndex - indisDusurSayisi > 0) ? DuzeltmeIsaretiIndex - indisDusurSayisi : 0;
            }
            break;
            // esc
            case 27:
            {
                // Sıfırla ve seçili elemanı değiştir.
                DuzeltmeIsaretiIndex = 0;
                yazi->clear();
                ElemanIleri();

                return eleman;
            }
            // Enter
            case '\r':
            {
                DuzeltmeIsaretiIndex = 0;
                ElemanIleri();

                return eleman;
            }

            // Diğer tüm tuşlar
            default:
            {
                //yazi->insert(DuzeltmeIsaretiIndex, 1, e.basilanTus);
                yazi->insert(DuzeltmeIsaretiIndex, e.basilanTusMB);

                size_t yaziBoyutu = strlen(e.basilanTusMB);
                // sabit boyutlu dizilerin boyutu böyle alınabilir
                size_t diziBoyutu = sizeof(e.basilanTusMB) / sizeof(e.basilanTusMB[0]);
                // indis kaymasını dizi boyutuna sabitle.
                DuzeltmeIsaretiIndex += yaziBoyutu > diziBoyutu ? diziBoyutu : yaziBoyutu;
            }
            break;
            }
        }
        else
        {
            // Atanma yapılmadan önceki düzeltme işareti indisi
            const size_t oncekiIsaretIndex = DuzeltmeIsaretiIndex;

            // Ok tuşları
            if (!yazi->empty())
            {
                switch (e.basilanTus)
                {
                case VK_LEFT: // Sol
                {
                    // Sol karakterde utf8 çoklu bayt karakterlerinden bir karakter seçilir ise başlangıç karakterinin indisi verilir
                    // Sadece sağ karakterlere atlarken bu sorun yapar, o sebepten dolayı yazı indisi değişene kadar atla.
                    size_t hedefIndis = (DuzeltmeIsaretiIndex - 1 != string::npos) ? DuzeltmeIsaretiIndex - 1 : 0;

                    // ctrl boşluk atlaması
                    if (e.ctrlBasili)
                    {
                        // ctrl basılıyken ok tuşuna basılır ise boşluk bulana kadar atla.
                        // Boşluk yok ise en başa git
                        size_t boslukIndis = yazi->rfind(' ', hedefIndis);
                        hedefIndis = boslukIndis != string::npos ? boslukIndis : 0;
                    }
                    // asıl indis ayarlaması
                    DuzeltmeIsaretiIndex = utf8_index(*yazi, hedefIndis);
                    break;
                }

                case VK_RIGHT: // Sağ
                {
                    // Yazıdaki indis değişene kadar utf8 karakteri atla.
                    size_t hedefIndis = DuzeltmeIsaretiIndex;
                    // ctrl boşluk atlaması
                    if (e.ctrlBasili)
                    {
                        // Sağ taraf ctrl boşluğa atlar
                        size_t boslukIndis = yazi->find(' ', hedefIndis);
                        hedefIndis = boslukIndis != string::npos ? boslukIndis : yazi->size();
                    }
                    // asıl indis ayarlaması (sağa atlarken hedef indisi indis değişene kadar değiştir, 
                    // utf8_index bir devam baytının indisine işaret ederse gerideki baş baytın indisini döndürür)
                    do
                    {
                        hedefIndis++; // utf8_index aynı indisi artan sayıda döndürebileceğinden kontrolsüz arttır.
                        DuzeltmeIsaretiIndex = (hedefIndis < yazi->size()) ? utf8_index(*yazi, hedefIndis) : yazi->size();
                    } while (DuzeltmeIsaretiIndex == oncekiIsaretIndex && oncekiIsaretIndex != yazi->size());

                    break;
                }
                }
            }
        }
    }

    // TODO (optimization) : Use an index range instead of an 'substr' string.

    // Çizimi yap
    // Çizim indisini yazının boyutuna göre sürükle
    size_t yaziCizimIndis = AnaGrup.seciliArayuzElemani == eleman.indis ? 
        (DuzeltmeIsaretiIndex <= (alan.genislik - 1) * alan.yukseklik ? 0 : DuzeltmeIsaretiIndex - ((alan.genislik - 1) * alan.yukseklik)) : 
        0;
    // Yer tutucu ve onun ayarları
    bool yerTutucuCiz = yazi->empty();
    const string* cizilecekYazi = yerTutucuCiz ? &yerTutucu : yazi;
    sifreAlani = sifreAlani && !yerTutucuCiz;
    // Çizilecek yazının parçasını al (boyuta göre)
    string yaziParcasi = yaziCizimIndis >= utf8_length(*cizilecekYazi) ? string() : utf8_substr(*cizilecekYazi, yaziCizimIndis);

    alan.genislik -= 1; // X boyutundan 1 tane köşeli parantez karakteri çıkar (yazı 1. köşeli parantez sonrasından başlıyor)
    // Her Y boyutu için çizimi yap.
    for (int i = 1; i <= alan.yukseklik; i++)
    {
        cout << setw(0) << setfill(' ') << "[";

        int jTarget = alan.genislik * i;
        for (int j = alan.genislik * (i - 1); j < jTarget; j++)
        {
            WORD oncekiKonsolRengi = KonsolRenk();

            // Eleman seçili ve hedef indis utf8 indiste eşit ise konsol rengini ters al.
            if (eleman.indis == AnaGrup.seciliArayuzElemani)
            {
                if (DuzeltmeIsaretiIndex == (j >= yazi->size() ? j : utf8_index(*yazi, j)) && !yerTutucuCiz)
                {
                    KonsolRenkAyarla(KonsolRenkTers());
                }
            }

            // Eğer j yaziParcasi indis boyutu içindeyse
            if (j < yaziParcasi.size())
            {
                // TODO : bug occurs when a password field is present because the first char is replaced with '*'
                // after that we should NOT print the continuation bytes
                // if we do print malformed utf8 to the console it freezes until we call KonsolTemizle
                char yazilacakKarakter = yaziParcasi.at(j);

                if (!KarakterUTF8ByteBasi(yazilacakKarakter))
                {
                    // Bu karakterin yazıldığını sayma
                    jTarget++;
                }
                // karakter byte başıysa '*' karakterini şifre için yazdır.
                if (sifreAlani && utf8_index(yaziParcasi, j) != yaziParcasi.size() - 1 && utf8_length(yaziParcasi) > 1)
                {
                    if (KarakterUTF8ByteBasi(yaziParcasi.at(j)))
                    {
                        yazilacakKarakter = '*';
                    }
                    else
                    {
                        yazilacakKarakter = '\0';
                    }
                }

                if (yazilacakKarakter != '\0')
                {
                    cout << yazilacakKarakter;
                }
            }
            else
            {
                cout << " ";
            }

            KonsolRenkAyarla(oncekiKonsolRengi);
        }
        cout << "]";

        yaziCizimIndis += alan.genislik - 1;
        alan.y++;
        KonsolImlecPozisyonuAyarla(alan.x, alan.y);
    }

    KonsolRenkAyarla(oncekiAnaKonsolRenk);
    KonsolImlec(true);
    return eleman;
}

bool Dugme(ArayuzAlan alan, const string& yazi, const char etkinTus, const char secTus)
{
    size_t elemanID = ArayuzFonksiyonuIDHesapla(3, &alan, &yazi, &etkinTus);
    ArayuzElemani& eleman = IDDenHedefEleman(elemanID);
    eleman.alan = alan;
    eleman.secmeTusu = secTus;
    eleman.okTusuBasilabilir = true;
    eleman.secilebilirEleman = AnaGrup.elemanlarSecilebilir && etkinTus != '\0';

    Eglem e = EglemleriAl(eleman.indis, true);

    if (alan.x < 0 || alan.y < 0 || alan.genislik <= 0 || alan.yukseklik <= 0)
    {
        return false;
    }

    // bu fonksiyon ve bu iç fonksiyon her zaman çizim eğlemi için çağırılır
    KonsolImlec(false);
    KonsolImlecPozisyonuAyarla(alan.x, alan.y);
    WORD oncekiAnaKonsolRenk = KonsolRenk();
    KonsolRenkAyarla(ArayuzElemaniRenkAl(eleman));

    // TODO (Optimizasyon) : Use string index range instead of substr.
    size_t yaziCizimIndis = 0;
    alan.genislik -= 1; // X boyutundan 1 tane köşeli parantez karakteri çıkar (yazı 1. köşeli parantez sonrasından başlıyor)
    string yaziParcasi = AlanUTF8StringAyir(yazi, 0, alan.genislik);

    for (size_t i = 0; i < alan.yukseklik; i++)
    {
        cout << "[";
        switch (AnaGrup.yaziHizasi)
        {
        case YaziPozisyonu::Sol:
        {
            cout << setw(alan.genislik) << left << yaziParcasi;
            break;
        }
        case YaziPozisyonu::Orta:
        {
            size_t yaziBoyutu = utf8_length(yaziParcasi);
            size_t bosluk = yaziBoyutu >= alan.genislik ? 0 : alan.genislik - yaziBoyutu;
            size_t solBosluk = bosluk / 2;
            size_t sagBosluk = bosluk - solBosluk;

            cout << setw(solBosluk) << ""; // 'setw' dolgusunu yazdır.
            cout << yaziParcasi;
            cout << setw(sagBosluk) << ""; // 'setw' dolgusunu yazdır.
            break;
        }
        case YaziPozisyonu::Sag:
        {
            cout << setw(alan.genislik) << right << yaziParcasi;
            break;
        }
        }

        cout << "]";

        // utf8 yazının tam boyutunu ekle
        // (hack) birde yükseklik indisi 0'dan büyük ise büyük ihtimalle yazı ayrıldıysa cizimIndis 1 geri.
        yaziCizimIndis += yaziParcasi.size() + (i > 0 ? 1 : 0);
        alan.y++;

        yaziParcasi = AlanUTF8StringAyir(yazi, yaziCizimIndis, alan.genislik);
        KonsolImlecPozisyonuAyarla(alan.x, alan.y);
    }

    KonsolRenkAyarla(oncekiAnaKonsolRenk);
    KonsolImlec(true);
    return eleman.indis == e.eglemAliciEleman && e.tip == Eglem::KlayveBas && e.basilanTus == etkinTus;
}

void Yazi(ArayuzAlan alan, const string& yazi)
{
    size_t elemanID = ArayuzFonksiyonuIDHesapla(3, &alan, &yazi);
    ArayuzElemani& eleman = IDDenHedefEleman(elemanID);
    eleman.alan = alan;
    eleman.secilebilirEleman = false;

    if (alan.x < 0 || alan.y < 0 || yazi.empty())
    {
        return;
    }

    bool otoYukseklik = alan.yukseklik <= 0, otoGenislik = alan.genislik <= 0;
    // yüksekliği otomatik hesapla + yazı var
    if (otoYukseklik)
    {
        eleman.alan.yukseklik = 1;
    }

    // bu fonksiyon ve bu iç fonksiyon her zaman çizim eğlemi için çağırılır
    KonsolImlec(false);
    KonsolImlecPozisyonuAyarla(alan.x, alan.y);
    WORD oncekiAnaKonsolRenk = KonsolRenk();
    KonsolRenkAyarla(AnaGrup.varsayilanRenk);

    if (otoYukseklik || otoGenislik)
    {
        int gen = 0; // şu anki yazı genişliği
        for (size_t i = 0; i < yazi.size(); i++)
        {
            char yKarakter = yazi.at(i);
            cout << yKarakter;
            // utf8 genişliği ekle
            gen += 1 - static_cast<int>(KarakterUTF8ByteBasi(yKarakter) ? KarakterUTF8ByteSayisi(yKarakter) - 1 : 0);
            if (gen > eleman.alan.genislik) 
            {
                eleman.alan.genislik = gen;
            }

            if (yazi.at(i) == '\n')
            {
                gen = 0; // olan genişliği sıfırla
                eleman.alan.yukseklik++; // yüksekliği 1 arttır
                KonsolImlecPozisyonuAyarla(alan.x, KonsolImlecPozisyonu().Y);
            }
        }
    }
    else
    {
        // düğmenin yazı çizme kodu ile aynı (belirli bir alan var)
        size_t yaziCizimIndis = 0;
        string yaziParcasi = AlanUTF8StringAyir(yazi, 0, alan.genislik);

        for (size_t i = 0; i < alan.yukseklik; i++)
        {
            switch (AnaGrup.yaziHizasi)
            {
            case YaziPozisyonu::Sol:
            {
                cout << setw(alan.genislik) << left << yaziParcasi;
                break;
            }
            case YaziPozisyonu::Orta:
            {
                size_t yaziBoyutu = utf8_length(yaziParcasi);
                size_t bosluk = yaziBoyutu >= alan.genislik ? 0 : alan.genislik - yaziBoyutu;
                size_t solBosluk = bosluk / 2;
                size_t sagBosluk = bosluk - solBosluk;

                cout << setw(solBosluk) << ""; // 'setw' dolgusunu yazdır.
                cout << yaziParcasi;
                cout << setw(sagBosluk) << ""; // 'setw' dolgusunu yazdır.
                break;
            }
            case YaziPozisyonu::Sag:
            {
                cout << setw(alan.genislik) << right << yaziParcasi;
                break;
            }
            }

            yaziCizimIndis += yaziParcasi.size(); // utf8 yazının tam boyutunu ekle
            alan.y++;

            //yaziParcasi = yaziCizimIndis >= yazi.size() ? string() : utf8_substr(yazi, yaziCizimIndis, alan.genislik);
            yaziParcasi = AlanUTF8StringAyir(yazi, yaziCizimIndis, alan.genislik);
            KonsolImlecPozisyonuAyarla(alan.x, alan.y);
        }
    }

    KonsolRenkAyarla(oncekiAnaKonsolRenk);
    KonsolImlec(true);
}