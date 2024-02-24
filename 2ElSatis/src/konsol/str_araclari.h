// -*- encoding: utf-8 -*-

#pragma once
// -----------
// author : Barbaros
// str_araclari.h => std::string ile ilgili özel fonksiyonlar. 
// (özellikle utf8 odaklı, string.fonksiyon() yerine utf8_fonksiyon(string) kullanılmalı)
// Örn : boşluk silme, utf8 yazı fonksiyonları gibi vs.
// -----------

#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>

#pragma region String

// -- Kullanışlı fonksiyonlar

// @brief
// Bir yazıyı okunmayacak ama kıyaslanabilecek bir hale getirir.
// Şifre icin onemli.
// NOT : Düzgün bir hash fonksiyonu olmadığı için bazı şifreler aynı bozulan yazıyı getirebilir, ama bunun olasılığı azdır.
inline std::string StringBoz(const std::string& yazi, const std::vector<char>& degisKarakterList)
{
    // String bozmak için gereken ön tablo
    // python'da rastgele olarak yapıldı
    // >>> from random import randint
    // >>> for i in range(8):
    // ...     print(f"0x{randint(0, 0x7fffffff):x}, 0x{randint(0x7fff, 0x7fffffff):x},")
    // Bu programın çıktısı buraya yazıldı
    static size_t BozTablo[] = {
        0x231738af, 0x7b1cec45,
        0x73231447, 0xcfa23245,
        0x2e823cc5, 0x6a13ccb6,
        0x14f4a2d8, 0x31b0dfc3,
        0x12d19f94, 0xfa73c831,
        0x6dd7f84f, 0xda23f85e,
        0x523538ef, 0x834258f2,
        0x13563cde, 0x2431a323,
    };

    size_t sonucToplam = 0;
    std::string sonuc;
    sonuc.reserve(yazi.size());

    // asıl string boz
    for (size_t i = 0; i < yazi.size(); i++)
    {
        // Yazıdaki charları bit olarak boz, böylelikle okunamaz hale getir
        char sonucAyarla = yazi.at(i % 2 == 0 ? i : yazi.size() - i);
        sonucAyarla ^= BozTablo[i % (sizeof(BozTablo) / sizeof(BozTablo[0]))]; // XOR
        sonucToplam += sonucAyarla;

        std::vector<char>::const_iterator bulunan = std::find(degisKarakterList.begin(), degisKarakterList.end(), sonucAyarla);
        if (bulunan != degisKarakterList.end() || sonucAyarla == '\r' || sonucAyarla == '\n')
        {
            // Rastgele ama sabit bir chara ayarla
            // char = -127, 128 arası, 8 bit.
            sonucAyarla = 'P';
        }

        sonuc.push_back(sonucAyarla);
    }
    // uzunluk boz
    for (size_t i = 0; i < sizeof(size_t) * 8; i++)
    {
        const size_t bit = (1ULL << i);

        if ((sonucToplam & bit) == 0)
        {
            continue;
        }

        // toplam sonucu stringe ekle + boz tablo
        char pushSonuc = (char)(sonucToplam - bit) ^ BozTablo[i % (sizeof(BozTablo) / sizeof(BozTablo[0]))];

        std::vector<char>::const_iterator bulunan = std::find(degisKarakterList.begin(), degisKarakterList.end(), pushSonuc);
        if (bulunan != degisKarakterList.end() || pushSonuc == '\r' || pushSonuc == '\n')
        {
            // Rastgele ama sabit bir chara ayarla
            // char = -127, 128 arası, 8 bit.
            pushSonuc = 'Q';
        }
        if (i % 2 == 0)
        {
            sonuc.push_back(pushSonuc);
        }
        else
        {
            sonuc.insert(sonuc.begin(), pushSonuc);
        }
    }

    return sonuc;
}
// Yaziyi 'parcalamaKarakteri'nden ayırarak parçalar.
// Parçaları bu fonksiyonun değeridir.
inline std::vector<std::string> StringParcala(const std::string& yazi, const char parcalamaKarakteri)
{
    std::stringstream yaziOkuma(yazi);
    std::string veri;
    std::vector<std::string> parcali;
    // 'getline'ı yazi parametresi ile yapılan stringstream yaziOkuma ile parcalamaKarakterine kadar olan yaziyi oku.
    while (std::getline(yaziOkuma, veri, parcalamaKarakteri))
    {
        // veri boş veya sadece boşluktan ibaret ise
        if (veri.empty() || veri.find_first_not_of(' ') == std::string::npos)
            continue;

        parcali.push_back(veri);
    }

    return parcali;
}
// Yazıdaki 'kaldir' parametresine konulan karakterleri ayıklar.
inline void StringKarakterleriKaldir(std::string& yazi, const char kaldir)
{
    // karakter pozisyonunu bul (ayıklama için)
    size_t pozisyon = yazi.find(kaldir);
    // string::npos => find fonksiyonları bulamadığında fonksiyonun getirdiği değer, -1 dir.
    // yazıda kaldırılıcak karakter olduğu sürece kaldır.
    while (pozisyon != std::string::npos)
    {
        // pozisyon var ise o pozisyondaki karakteri kaldır 
        // "erase(pozisyon, karakter sayısı)"
        yazi = yazi.erase(pozisyon, 1);
        // yeniden ara
        pozisyon = yazi.find(kaldir);
    }
}

inline void StringKarakterleriKaldir(std::string& yazi, const std::vector<char> kaldirListe)
{
    for (size_t i = 0; i < kaldirListe.size(); i++)
    {
        StringKarakterleriKaldir(yazi, kaldirListe.at(i));
    }
}

// Yazıdaki başlangıç ve son boşlukları siler.
inline void StringBoslukAyikla(std::string& yazi)
{
    // Yorumlarda '[]' ile çevrili karakterlerin indisleri verilmektedir.
    size_t baslangicBoslukSon = yazi.find_first_not_of(' '); // "  [y]azi ..."
    // string::npos => find fonksiyonları bulamadığında fonksiyonun getirdiği değer, -1 dir.
    if (baslangicBoslukSon != std::string::npos)
    {
        yazi = yazi.substr(baslangicBoslukSon);
    }

    size_t sonBoslukBaslangic = yazi.find_last_not_of(' ');  // "... so[n]   "
    // son boşluk olmayan karakter var && son boşluk olmayan karakter son karakter değil
    if (sonBoslukBaslangic != std::string::npos && sonBoslukBaslangic != yazi.size() - 1)
    {
        yazi = yazi.substr(0, sonBoslukBaslangic + 1);
    }
}
#pragma endregion

#pragma region UTF8

// @brief Verilen UTF8 karakterinin başlangıçmı yoksa devam byte'ı olup olmadığını kontrol et.
inline bool KarakterUTF8ByteBasi(const char k)
{
    // utf-8 de çoklu byte karakterlerin kodlaması şu şekilde
    // U+0000 dan U+007F    -> 0xxxxxxx (normal ascii boyutu)
    // U+0080 dan U+07FF    -> 110xxxxx 10xxxxxx
    // U+0800 dan U+FFFF    -> 1110xxxx 10xxxxxx 10xxxxxx
    // U+10000 dan U+10FFFF -> 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

    // Yani son çoklu byte karakterde (karakteri ters oku)
    // Tekli karakterde (false && !(false)) olur (zaten ilk baytın sonuncu biti 1 değil)
    // Çoklu karakterde (true && !(false)) olur (sonuncu karakter = (true && !(true))) olur
    //return (k & 0b10000000) && !(k & 0b01000000); // -> ters
    
    // üstekki tablodaki karakter dizileriyle bu 'return' değerini kontrol ederek 
    // sadece çoklu karakter sekansındaki ilk byte'ta true döndürür (char'ın utf8 bir stringden alındığı varsayılır ise)
    return !(k & 0b10000000) || (k & 0b01000000);   // -> düz
}

// @brief Verilen 'UTF8' olarak kabul edilen karakterin devam byte'ı olup olmadığını döndürür.
inline bool KarakterUTF8DevamByteli(const char k)
{
    // karakter 0xxxxxxx değil (ve byte başı) ise true.
    return (k & 0b10000000) && (k & 0b01000000);
}

// @brief Verilen 'UTF8' olarak kabul edilen karakterin devam byte'ı sayısını al.
// Karakter tek byte devamlı ise 1, devam byte'ı ise -1 döndürür.
inline size_t KarakterUTF8ByteSayisi(const char k)
{
    if (!KarakterUTF8ByteBasi(k))
    {
        return -1;
    }

    if ((k & 0b10000000) == (0))                   // 1 byte
    {
        return 1;
    }
    if ((k & 0b11100000) == (k & 0b11000000))      // 2 byte
    {
        return 2;
    }
    else if ((k & 0b11110000) == (k & 0b11100000)) // 3 byte
    {
        return 3;
    }
    else if ((k & 0b11111000) == (k & 0b11110000)) // 4 byte
    {
        return 4;
    }
    else // hatalı
    {
        return -1;
    }
}

enum class CharDirection
{
    Left, // Lowest index
    Right // Highest index
};

// @returns Verilen 'utf8' stringin 'utf8' çoklu bayt karakter farkındalığı ile boyut atlamalı indisi.
// Örn : u8"Türkçe Ğ | Ascii" => 'Ğ' utf8'de 2 bayt olarak kodlanıyor
// Bu sebeple std::string, utf8'in farkında olmadığı için (sadece utf8 depoluyor ama ascii olarak varsayıyor)
// 'Ğ' karakteri, +1 indis daha atlaması lazım. 
// utf8_boyut fonksiyonu ile beraber kullanılırsa yazının utf8 çoklu bayt kullanımlarının baş indislerini alırsınız.
// (örn : utf8::advance fonksiyonuna benzer)
// @param
// utf8 [const ref in] : İndis alınacak yazı.                                                                                          
// indis [in]          : İstenilen devam baytı olabilecek indis. Döndürülen değer bunun baş değeridir.                   
// yon [in]            : Hangi yöndeki indis alınır? Eğer devam baytına denk gelirse CharDirection::Left o karakterin ana byte'ını, CharDirection::Right ise sonraki karakterin ana byte'ı.
// NOT : Bu yön parametresi CharDirection::Right iken son karakterin devam bytına denk gelirse 'utf8' yazının en son karakter indisi döndürülür. 
// Bu tip sorunlar oluşmaması için fonksiyonun aynı parametrelerle CharDirection::Left çağırılıp sadece bir indis aralığı için kullanılması lazım.
inline size_t utf8_index(const std::string& utf8, size_t indis, CharDirection yon = CharDirection::Left)
{
    if (utf8.empty() && indis == 0)
    {
        return 0;
    }

    if (indis >= utf8.size())
    {
        std::string s = std::string("[utf8_index] indis ").append(std::to_string(indis)).append(" hatali.");

        throw std::range_error(s);
    }

    // (artık kontrol yönü CharDirection ile değiştirilebilir)
    while (indis > 0 && indis < utf8.size() && !KarakterUTF8ByteBasi(utf8.at(indis)))
    {
        switch (yon)
        {
        case CharDirection::Left:
            --indis;
            break;
        case CharDirection::Right:
            ++indis;
            break;
        }
    }

    return indis;
}

// @brief Tek bir karakter siler.
// @returns Silinen asıl karakter sayısı.
inline size_t utf8_pop_back(std::string& utf8)
{
    if (utf8.empty())
        return 0;

    // silinecek karakter sayısı
    size_t kBoyut = 1;
    while (!KarakterUTF8ByteBasi(utf8.at(utf8.size() - kBoyut)) && kBoyut > 0)
    {
        kBoyut++;
    }

    size_t oncekiBoyut = utf8.size();
    size_t yeniBoyut = kBoyut >= oncekiBoyut ? 0 : oncekiBoyut - kBoyut; // size_t overflow olacağından boyut kıyası yapıp 0'a sınırla
    utf8.resize(yeniBoyut);

    return kBoyut;
}

// @returns Verilen 'utf8' stringin 'utf8' boyutu.
// Çoklu byte utf8 bir karakter var ise döndürülen boyut verilen stringin boyutundan daha küçük olabilir.
inline size_t utf8_length(const std::string& utf8)
{
    size_t asilBoyut = 0;
    for (size_t i = 0; i < utf8.size(); i++)
    {
        // Sadece baş karakterleri say (tekli ascii karakterleri byte başı sayılır)
        if (KarakterUTF8ByteBasi(utf8.at(i)))
        {
            asilBoyut++;
        }
    }

    return asilBoyut;
}

// @brief Karakter siler. (not : yazı bozuk utf8 içeriyor ise bu fonksiyon hatalı davranır)
// @param utf8 : yazının kendisinin referansı | sayi : Silinecek 'utf-8' karakter sayısı.
// @returns Silinen asıl karakter sayısı.
inline size_t utf8_erase(std::string& utf8, size_t indis, size_t sayi)
{
    if (utf8.empty())
        return 0;

    // Asıl silinen karakter boyutu hesabında kullanıldığından dolayı utf8_size kullanma
    size_t oncekiBoyutAsil = utf8.size();
    indis = utf8_index(utf8, indis);
    sayi = indis + sayi > oncekiBoyutAsil ? oncekiBoyutAsil - (indis + sayi) : sayi;

    for (size_t i = 0; i < sayi; i++)
    {
        const char hedefK = utf8.at(indis + i);

        // utf8_index'ten alınan ilk indis 
        // utf8_pop_back fonksiyonunun aksine bir sayaç tutmamız gerekir 
        // (çünkü stringin belirli bir kısmını siliyoruz)
        size_t kbSayisi = KarakterUTF8ByteSayisi(hedefK);
        if (kbSayisi == -1)
        {
            // 'utf8_index' fonksiyonu hatalı değer döndürmüş veya yazı bozuk
            std::string s = "[utf8_erase] Hatali karakter : ";
            s.append(std::to_string(static_cast<int>(hedefK)));
            throw std::exception(s.c_str());
        }

        utf8.erase(indis, kbSayisi);
        // Son karakter değil ise indisi yeniden al.
        if (i != sayi - 1)
        {
            indis = utf8_index(utf8, indis - kbSayisi);
        }
    }

    return oncekiBoyutAsil - utf8.size();
}

// @brief UTF8 karakter sekansı Unicode kod noktasını döndürür.
// @param 
// str [in] : 4 karakterli karakter dizisi.
// Null ile bitebilir (eğer yeterince yer var ise) ancak utf8 başlangıc karakteri ile başlamalı.
inline uint32_t utf8_to_unicode(const char str[4])
{
    uint32_t unicodeKodNoktasi = 0;
    size_t kbSayisi = KarakterUTF8ByteSayisi(*str);
    if (kbSayisi == -1)
    {
        return 0;
    }

    // [reserve edilmiş bit][kullanılan bit]
    // diğer 1'den çok byte sayısı : [1][7]
    // bunun sayesinde 2097152'e kadar çıkabilir.
    // utf8 karakterler sıralı ama x86_64 sistemdeki tam sayılar ters (big-endian)
    // bu şekilde shiftleyebiliriz, sonra platformun endianını kontrol ederek '_byteswap_ulong' çağırabiliriz
    // ama bu proje için gereksiz. (sadece x86_64 sistemde çalışacak)
    switch (kbSayisi)
    {
    case 1:
        unicodeKodNoktasi = *str & 0x7F; // [1][7]
        break;
    case 2:
        unicodeKodNoktasi = (*str & 0x1F) << 6; // [3][5]

        unicodeKodNoktasi |= (*(str + 1) & 0x3F);
        break;
    case 3:
        unicodeKodNoktasi = (*str & 0x0F) << 12; // [4][4]

        unicodeKodNoktasi |= (*(str + 1) & 0x3F) << 6;
        unicodeKodNoktasi |= (*(str + 2) & 0x3F);
        break;
    case 4:
        unicodeKodNoktasi = (*str & 0x07) << 18; // [5][3]

        unicodeKodNoktasi |= (*(str + 1) & 0x3F) << 12;
        unicodeKodNoktasi |= (*(str + 2) & 0x3F) << 6;
        unicodeKodNoktasi |= (*(str + 3) & 0x3F);
        break;
    }

    return unicodeKodNoktasi;
}

// @brief 'u8indis' teki devam baytının sahibi olan veya utf8 başlangıç byte'ı ile 'ch' unicode noktasını kıyasla.
// 'ch' yerine char, wchar_t vs. tipleri verilebilir.
inline bool utf8_chcompare(const std::string& utf8, size_t u8indis, uint32_t ch)
{
    u8indis = utf8_index(utf8, u8indis);
    char sq[4] = { 0 };
    size_t kbSayisi = KarakterUTF8ByteSayisi(utf8.at(u8indis));
    // bozuk utf8 karakteri/yazısı veya devam baytı (ascii olarak kabul et)
    if (kbSayisi == -1)
    {
        return ch == utf8.at(u8indis);
    }
    
    for (size_t i = 0; i < kbSayisi; i++)
    {
        sq[i] = utf8.at(u8indis + i);
    }
    return ch == utf8_to_unicode(sq);
}

// @brief utf8 yazısını bozmadan 'substr' komutunu çalıştırır.
// not : count'tan daha uzun bir yazı döndürebilir.
inline std::string utf8_substr(const std::string& utf8, size_t offset, size_t count = std::string::npos)
{
    size_t u8boyut = utf8_length(utf8);
    offset = utf8_index(utf8, offset);

    // Offset 0 yazıdan büyük 'substr'
    if (offset == 0 && count >= u8boyut)
    {
        return utf8;
    }

    // 'offset' uygun olarak kaydırıldı zaten + sayı maksimum || offset + sayı utf8 yazı boyutundan büyük
    if ((offset + count) >= u8boyut || count == std::string::npos)
    {
        return utf8.substr(offset);
    }

    // yoksa kendi substr'ımızı çalıştır (karakter üzerinde daha uygun kontrol ile)
    std::string hedef;
    hedef.reserve(count);
    for (size_t n = 0, dbs = 0; n < count; n++)
    {
        // Asıl indis
        size_t indis = offset + n;

        if (dbs == 0)
        {
            // Devam bytelarına saygı duy.
            size_t kByteSayisi = KarakterUTF8ByteSayisi(utf8.at(indis));
            // hata kontrolü
            if (kByteSayisi == -1)
            {
                std::string s = "[utf8_substr] Hatali ascii karakter : ";
                s.append(std::to_string(static_cast<int>(utf8.at(indis))));
                throw std::exception(s.c_str());
            }

            dbs = kByteSayisi - 1; // kByteSayisi - 1 => devam byte sayısı
            count += dbs;
        }
        else // Devam baytı var
        {
            dbs--;
        }
        
        hedef.push_back(utf8.at(indis));
    }

    return hedef;
}
#pragma endregion