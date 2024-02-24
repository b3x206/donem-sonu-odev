// -*- encoding: utf-8 -*-

#pragma once

// --------
// Author(s) : 
//     Barbaros : Arayüz fonksiyonları impl., veri tipleri ve fazlasıyla yorum satırları
// 
// TODO : 
//     Barbaros : Event/Eglem sistemi : (optimizasyon) : Eglem::Tip::Ciz ekle, 'cout' fonksiyonunu sadece arayüz değiştiğinde kullan.
// Konsol için Arayüz ile alakalı fonksiyonları içerir.
// IMGUI gibi bir sistem kullanır. Arayüz elemanlarının parametre idlerini hesaplayıp indis yapar ve eleman olarak koyar.
// Konulan elemanlar 'ArayuzEleman' veri tipi olarak bir grup içerisinde bulunur ve buradan değerleri değiştirilebilir.
// Bu grup ile klavye aracılığyla elemanlara eğlemler iletilir (EglemleriAl() fonksiyonu)
// Değerler ve alınan bir eğlem sistemi ile 
// 
// Örnek fonksiyon (arayuz.cpp için geçerli, burada sadece prototipler ve fonksiyonlar var)
// 
// [ bool veya const ArayuzElemani& OrnekArayuzFonksiyonu(Alan a, bool ornekParametre) ] >
//     [ size_t id = ArayuzFonksiyonuIDHesapla(); ]
//     [ ArayuzElemani& reserveEleman = IDDenHedefEleman(id); ] // burada eleman reserve edilir veya alınır. ArayuzEleman::indis otomatik olarak ayarlanır ve değişmesi gerekmez.
//     reserveEleman.alan = a;  
//     reserveEleman.<gerekli alanlar> = <gerekli değerler>     // Gerekli alanlar burada alınır. daha fazla bilgi için 'ArayuzEleman' veri tipine bknz.
// 
//     [ Eglem e = EglemleriAl(reserveEleman.indis, <fonksiyon eğlem ile değiştirebilir / değiştirilebilir mi>); ]
//     switch (e.tip)
//     {
//     case Eglem::KlayveBas:
//          if (!e.ozelTus && e.basilanTus == '\r')
//          {
//               ornekParametre = true;
//          }
//          break;
//     }
//     // Çiz
//     [ KonsolRenkAyarla(ArayuzElemaniRenkAl(reserveEleman)); ]
//     [ KonsolImlecPozisyonuAyarla(a.x, a.y); ]
//     // not : AnaGrup.yaziPozisyonu gibi daha karışık değişkenlere saygı duyulduğu varsayılır. bu örnekte yazı yok, sadece bir tik kutusu
//     cout << u8"[ " << (ornekParametre ? u8"✅" : u8" ") << u8" ]";  
// 
//     // son
//     return ornekParametre;
// >
// 
// Böylelikle kolaylıkla fonksiyonlar kullanılır ve arayüz etkileşimi sağlanır. Daha fazla bilgi için bu dosyadaki fonksiyon açıklamaları daha iyi olur.
// 
// Örnek Kullanım :
// Otomatik olarak arayüz çizer ve arayüz etkileşimini eğlemler ile alır.
// Arayüzün kontrolü bir koşullu while döngüsü içerisinde, 'Sleep' fonksiyonu 
// (veya std::this_thread::sleep_for(std::chrono::<zaman birimi>)) şekliyle bekleyerek çağırılmalıdır.
// (not : bu pek gerekli bir adım değil, konsol çizimleri sürekli çağırılsa bile sorun olmaz)
// <
// Arayüzle iş bittikten sonra (koşullu döngü sona erdi) ArayuzTemizle() fonksiyonu çağırılmalıdır.
// Arayüz döngüsü sonunda 'ArayuzTemizle' çağırılır ise fazla işlem yapılır, 
// sonu hariç içerisinde çağırılırsa arayüz fonksiyonları hatalı davranabilir.
// Arayüz fonksiyonları sürekli olarak çağırıldığında 'Eglem'leri kontrol edilmektedir. (bu sebepten dolayı koşullu bir döngü içerisinde kullanılmalı)
// >
// --------
// Örnek Kod (burada yazı alanları ve düğmeler kullanılmaktadır) : 
// // !! kendime not : yazıyı 'while' döngüsünün içinde tanımlamayın, tanımlanırsa scope ({}) içi 'static' yani kalıcı yerel değişken olarak tanımlanmalı !!
// string yazi;
// 
// while (<koşul>)
// {
//      // "ArayuzAlan alan, string* yazi, const string yerTutucu, const bool sifreAlani"
//      YaziAlani(ArayuzAlan(2, 2, 16, 1), &yazi, "Yazı Giriniz...", false);
// 
//      // "ArayuzAlan alan, string* yazi"
//      if (Dugme(ArayuzAlan(4, 5, 10, 5), "Tamam"))
//      {
//           <koşul> = false;
//      }
// }
// ArayuzTemizle(true, ' '); // "bool arayuzSil, char dolgu = ' '"
// --------

#include "pencere.h"       // Konsol<EğlemAdı>();
#include "str_araclari.h"  // utf8_<string fonksiyonu adı>();

#include <iosfwd>          // std::cout, std::cerr forward decl.

//prototip
struct ArayuzElemani;
struct ArayuzGrubu;
struct ArayuzAlan;
struct Eglem;

// @brief Arayüz içerisinde yazı pozisyonu belirtir. 
enum class YaziPozisyonu
{
    Sol, Orta, Sag
};

// @brief Karesel bir alan belirtir.
struct ArayuzAlan
{
    int x, y;
    int genislik, yukseklik;

    // @brief Arayüz alanının değişkenlerini hazırlar.
    ArayuzAlan() :
        x(0), y(0), genislik(0), yukseklik(0)
    {}

    // @brief Arayüz alanının değişkenlerini hazırlar.
    ArayuzAlan(int px, int py, int gen, int yuk) :
        x(px), y(py), genislik(gen), yukseklik(yuk)
    {}
};

// @brief Arayuz grubu değişkenlerini tutar.
struct ArayuzGrubu
{
    // @brief Arayüz tab '\\t' tuşu ile seçili arayüz elemanını değiştirebilir (tabIndisi'ne göre)
    bool arayuzTekTusNavigasyonu = true;
    // @brief Basılınca eleman değiştirme düğmesi.
    // Elemanların kendisine de seçmek için tuş atanabilir.
    char arayuzTusNavigasyonuHedef = '\t';
    // @brief Seçili arayüz elemanı indisi.
    size_t seciliArayuzElemani = 0;
    // @brief Önceden çağırılan fonksiyonun kullandığı elemanın indisi.
    size_t oncekiElemanIndisi = 0; 

    // @brief Elemanların varsayılan rengi.
    WORD varsayilanRenk = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    // @brief Elemanların seçildiği renk.
    WORD seciliRenk = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    // @brief Seçilemeyecek olan elemanların renkleri (Yazi() gibi fonksiyonlar hariç)
    WORD inaktifRenk = FOREGROUND_INTENSITY;
    // @brief Yazının ne tarafa hizalanacağı hakkında bilgi.
    // Not : Yazı alanlarında çalışmaz.
    YaziPozisyonu yaziHizasi = YaziPozisyonu::Sol;

    // @brief Grupta olan elemanlar.
    // Herhangi bir eleman fonksiyonu çağırıldıktan sonra buraya bir eleman eklenir. 
    // Temizlemek için 'ArayuzTemizle' fonksiyonunu çağır (elemanlar.clear() hata yaratabilir)
    std::vector<ArayuzElemani> elemanlar;
    // @brief Çağırılan eleman fonksiyonların seçilebilir olup olmadığı.
    // NOT : Bu değişken sıfırlanmaz ise diğer elemanlarıda etkileyebilir. (renk değişkenleri gibi)
    bool elemanlarSecilebilir = true;
};

// @brief Eleman bilgisi içerir.
// Her 'arayuz.h' deki arayüz fonksiyonun 'SuankiGrup'daki 'elemanlar' dizisine bir 'ArayuzElemani' atması gerekir.
// Arayüz elemanının tab indisi, 'arayuz.h'deki fonksiyonların çağırılış sırasına göredir. 
// Yani önce çağırılan bir çizim fonksiyonu tab tuşuna basıldığında daha önce seçilir.
struct ArayuzElemani
{
    // @brief Elemanın idsi.
    size_t id = 0;
    // @brief Elemanın arayüzdeki kapladığı alan.
    ArayuzAlan alan;
    // @brief Elemanın 'elemanlar' dizisindeki indisi.
    size_t indis = 0;
    
    // @brief Bu elemanda bir eğlem bulunup bulunmaması ile ilgili bir fonksiyon.
    // Örn : Görsel elemanlarda (büyük ihtimalle) eğlem bulunmaz ve o fonksiyon çağırıldığında sadece çizilir.
    bool secilebilirEleman = true;
    // @brief Bu eleman seçili iken ok tuşlarına basıldığında ok tuşu için diğer olabilecek eğlemlere izin verir.
    bool okTusuBasilabilir = true;

    // @brief Bu elemanı seçmek için basılacak tuş.
    uint32_t secmeTusu = '\0';
    // @brief Basılan seçme tuşu özel?
    bool secmeTusuOzel = false;
};

// @brief Bir arayüz eğlemi belirtir.
struct Eglem
{
    // @brief Eğlemin tipini belirtir.
    // Eğer herhangi bir tuşa basma olmaz ise eğlemin bir tipi olmaz.
    // Tipli eğlem sadece seçili olan eleman(lara) gönderilir.
    enum Tip
    {
        // Eğlem yok ise
        Yok,
        // Interaksiyon eğlemleri
        Secim,
        SecimKaldirma,
        KlayveBas,
    };

    // @brief Eglem tipi. Eglem::Tip enum'unu gör.
    Tip tip = Tip::Yok;
    // @brief Basılan tuşun kodu. (varsayılan olarak -1)
    // ozelTus false ise bir wchar_t belirtir, yoksa VK_ makrolarından bir sanal tuş belirtir.
    // KEY_EVENT_RECORD sınıfının çevirilmesidir.
    uint32_t basilanTus = -1;
    // @brief Basılan tuş ok tuşu gibi özel bir tuş?
    bool ozelTus = false;
    // @brief Basılan tuşun UTF-8 kodlamasındaki çoklu byte (mb) karşılığı.
    // ozelTus true ise boş olur.
    char basilanTusMB[MB_LEN_MAX]{};
    // @brief Tuş basıldığında ctrl tuşuyda basılıydı.
    // Not : Bu döndürülen karakteri etkilemez, sadece ctrl'nin basılı olup olmadığını döndürür.
    bool ctrlBasili = false;
    // @brief Tuş basıldığında alt tuşuyda basılıydı.
    // Not : Bu döndürülen karakteri etkilemez, sadece alt'ın basılı olup olmadığını döndürür.
    bool altBasili = false;
    // @brief Tuş basıldığında shift tuşuyda basılıydı.
    // Not : Bu döndürülen karakteri etkilemez, sadece shift'in basılı olup olmadığını döndürür.
    bool shiftBasili = false;

    // @brief Eğlemi alan eleman indisi.
    size_t eglemAliciEleman = -1;
    // @brief Eğlemden sonra seçilecek eleman.
    size_t secilenEleman = -1;
};

// Ana grubu 'extern' olarak tanımlayarak tanımını '.cpp' dosyasında yapıcağımız belirtilir.
// @brief Erişilecek ana grup.
extern ArayuzGrubu AnaGrup;

// @brief Elemanları bir for döngüsü veya aynı işaretcili parametrelerle çizildiğinde
// ayırt edebilmek için ekstra parametre kullanılacak çağırılacak fonksiyon.
// Her bir eleman çizme fonksiyonu çağırıldıktan sonra ipucu sıfırlanır.
// Bir döngüde aynı parametrelerle çağırılan arayüz fonksiyonları için elverişli.
void ElemanIDIpucu(const size_t ipucu);
// @brief Seçili olan elemandan geri elemana gider. Uygun eğlemleri elemanlar için çağırır.
// @param döngü [in] : En baş elemandan sonra geri atlanınca ileriye git.
// @returns Geri dönüldüğünde seçili elemanda bir değişiklik olup olmadığı.
bool ElemanGeri(const bool dongu = false);
// @brief Seçili olan elemandan ileri elemana atlar. Uygun eğlemleri elemanlar için çağırır.
// @param döngü [in] : En son elemandan sonra ileri atlanınca geri başa dön.
// @returns İleri gidildiğinde seçili elemanda bir değişiklik olup olmadığı.
bool ElemanIleri(const bool dongu = false);
// @brief İndisteki elemanı seçer.
// @param indis [in] : Seçilecek eleman indisi.
// @returns Seçimin başarılı olup olmadığı. 
// 'indis' parametreli indiste bir eleman yok veya eleman seçilemez ise false döndürür.
bool ElemanSec(const int indis);
// @brief Eleman boyutunu döndürür.
size_t ElemanlarBoyut();

// @brief Arayüz üzerinde ok tuşları ile navigasyon yapılabilen bir ızgara yap.
// Not : Bazı arayüz elemanları ok tuşlarının kontrolünü üzerine alabilir. (sadece tek tuş navigasyonu ile atlanabilen elemanlar)
// Not 2 : Arayüz üzerinde ızgara alanının boyutu belirlenmediği için ızgara içi arayüz elemanı boyutları otomatikman belirlenmez, 
// bu sebepten dolayı bu fonksiyon sadece ok tuşları navigasyonunu etkinleştirir (bunun için GUI Layouting sistemi lazım zaten)
void ArayuzIzgaraNav(const size_t sutunSayisi);

// @brief 'indis' parametreli elemanı siler.
// @param
//     indis     [in] : Elemanın indisi.                                                                                   
//     arayuzSil [in] : Arayüzdeki alanları sil.                                                                          
//	   dolgu     [in] : arayuzSil 'true' ise silinecek karakterler yerine doldurulacak dolgu.
// @returns Silinen eleman varmıydı ve indis doğrumu?
bool ArayuzElemanSil(size_t indis, bool arayuzSil, const char dolgu = ' ');
// @brief Arayüz çizim grubunu bitirir ve siler.
// Not : Bunu her arayüz döngüsünden çıktıktan sonra çağır.
// Yoksa fazla arayüz noktası olursa hafıza sızıntısı olabilir.
// @param 
//     arayuzSil [in] : Arayüzdeki alanları sil.                                                                          
//	   dolgu     [in] : arayuzSil 'true' ise silinecek karakterler yerine doldurulacak dolgu.
void ArayuzTemizle(bool arayuzSil, const char dolgu = ' ');

// @brief Önceden çağırılan arayüz elemanını siler.
void OncekiCizilenArayuzElemanSil(bool arayuzSil, const char dolgu = ' ');

// @brief Eğlemleri işler. Çoğu görsel olmayan arayüz fonksiyonun sonunda çağırılır.
// @param 
//     kullan     [in] : Eğlemi kullanarak seçilebileceğini belirt.                                                       
//     hedefIndis [in] : Eğlemi kullanıcak arayüz elemanının indisi.
Eglem EglemleriAl(const size_t hedefIndis, const bool kullan = false);

// @brief Bir yazı alanını çizer.
const ArayuzElemani& YaziAlani(ArayuzAlan alan, std::string* yazi, const std::string& yerTutucu, bool sifreAlani, const char secTus, bool (*karakterCtrl)(const int64_t));

// @brief Bir yazı alanını çizer.
inline const ArayuzElemani& YaziAlani(ArayuzAlan alan, std::string* yazi, const std::string& yerTutucu, bool sifreAlani, const char secTus)
{
    return YaziAlani(alan, yazi, yerTutucu, sifreAlani, secTus, nullptr);
}
// @brief Bir yazı alanını çizer.
inline const ArayuzElemani& YaziAlani(ArayuzAlan alan, std::string* yazi, const std::string& yerTutucu, const char secTus)
{
    return YaziAlani(alan, yazi, yerTutucu, false, secTus, nullptr);
}
// @brief Bir yazı alanını çizer.
inline const ArayuzElemani& YaziAlani(ArayuzAlan alan, std::string* yazi, const std::string& yerTutucu, bool sifreAlani)
{
    return YaziAlani(alan, yazi, yerTutucu, sifreAlani, '\0', nullptr);
}
// @brief Bir yazı alanını çizer.
inline const ArayuzElemani& YaziAlani(ArayuzAlan alan, std::string* yazi, const std::string& yerTutucu, bool sifreAlani, bool (*karakterCtrl)(const int64_t))
{
    return YaziAlani(alan, yazi, yerTutucu, sifreAlani, '\0', karakterCtrl);
}
// @brief Bir yazı alanını çizer.
inline const ArayuzElemani& YaziAlani(ArayuzAlan alan, std::string* yazi, bool sifreAlani, bool (*karakterCtrl)(const int64_t))
{
    return YaziAlani(alan, yazi, std::string(), sifreAlani, '\0', karakterCtrl);
}
// @brief Bir yazı alanını çizer.
inline const ArayuzElemani& YaziAlani(ArayuzAlan alan, std::string* yazi, bool sifreAlani)
{
    return YaziAlani(alan, yazi, std::string(), sifreAlani, '\0', nullptr);
}

// @brief Basıldığında 'true' döndürür.
// Basma tuşu karakterini değiştirebilirsiniz. 
// 'etkinTus' 0'a olur ise bu arayüz elemanı etkileşilemez olarak ayarlanır.
// Varsayılan olarak '\\r'(Enter) tuşu etkinleştirir.
bool Dugme(ArayuzAlan alan, const std::string& yazi, const char etkinTus = '\r', const char secTus = '\0');

// @brief Belirlenen sınırlar içerisinde yazı yazdırır.
// Yazı boyutunu sınırlamamak için 'alan' parametresinin boyutu 0 veya 0'dan küçük olabilir.
// Not : Bu fonksiyon bir arayüz elemanı olarak geçer, ama tamamen inaktiftir (seçilemez, sadece ArayuzTemizle komutunda temizlenir) ve rengi sürekli AnaGrup.varsayilanRenk'dir.
void Yazi(ArayuzAlan alan, const std::string& yazi);

// @brief Verilen pozisyon içerisinde yazı yazdırır.
// Yazı boyutu sınırlandırılmaz.
inline void Yazi(COORD pozisyon, const std::string& yazi)
{
    Yazi(ArayuzAlan(pozisyon.X, pozisyon.Y, 0, 0), yazi);
}