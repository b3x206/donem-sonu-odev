// -*- encoding: utf-8 -*-

#pragma once
// -----------
// Author : Barbaros
// pencere.h => Konsol penceresini, imlecini ve arabelleğini kontrol eder.
// <windows.h> kütüphanesini kullanır. 
// (Windows.h kütüphanesindeki konsol fonksiyonların daha otomatik ve daha anlaşılır hali, arayuz.h ve arayuz.cpp bunu kullanır, 
// linux gibi terminallerin olduğu yerde daha iyi destek için ansi escape chars kullan ama platform sadece windows olduğundan şimdilik iyi)
// -----------

// bazı kullanılmayan windows kütüphanelerini kaldır (örn : internet soketleri vs.)
#define _WIN32_LEAN_AND_MEAN
// kullanılmak istenen windows kütüphanesi sürümünü tanımla (0x0500 = windows 2000)
// bu 'KonsolPenceresiBoyutlandırılabilir' fonksiyonu için gerekli.
#define _WIN32_WINNT 0x0500
#include <Windows.h>  // windows kütüphanesi header

// @brief Konsolun rengini al.
// Döndürülen değer, 16 bitlik bir tam sayı. Bitlerin renk adları BACKGROUND_ veya FOREGROUND_ ile başlar.
// Bitleri kontrol etmek için verilen renk adı #define makroları ile and yapıp aynı makroya eşitliğini kontrol et.
// örn : if ((KonsolRenk() & FOREGROUND_RED) == FOREGROUND_RED) {  ... ön planda kırmızı renk var ... }
WORD KonsolRenk();
// @brief 
// 'KonsolRengi()' fonksiyonundaki renklerin negatifini döndürür.
WORD KonsolRenkTers();
// @brief
// Konsol rengi ayarla
// 'renk' parametresine OR ( '|' veya operatorü ) ile BACKGROUND_ ve FOREGROUND_ ile başlayan renk parametrelerini koy
// FOREGROUND => ön plan, BACKGROUND => arkaplan
// @returns 
// Ayarlamanın başarılı olup olmadığı.
bool KonsolRenkAyarla(WORD);

// @brief
// Konsol imleçini gizler veya gösterir.
bool KonsolImlec(bool);
// @returns
// Konsolun imleç pozisyonunu al.
COORD KonsolImlecPozisyonu(const SHORT kaymaX = 0, const SHORT kaymaY = 0);
// @brief
// Konsoldaki imleç pozisyonunu ayarlar.
bool KonsolImlecPozisyonuAyarla(const COORD);
// @brief
// Konsoldaki imleç pozisyonunu ayarlar.
inline bool KonsolImlecPozisyonuAyarla(const SHORT x, const SHORT y)
{
    // burada 'inline' kullanılmaz ise function overloading çalışmıyor.
    // Çünkü c++'ın headerlarda cpp dosyasında tanımlanan sadece tek fonksiyon kuralı var.
    // 'inline' yazarak veya iki fonksiyonuda headerda tanımlayarak bu sorun gideriliyor.
    return KonsolImlecPozisyonuAyarla({ x, y });
}

// @brief
// Konsolun eğlem modunu ayarlar.
bool KonsolEglemModu(const DWORD mode);

// @brief
// Konsol penceresinde olan olaylari al.
// Tuş okumak için conio.h alternatifi.
// @param kayit : Alınacak 'kayit' değişkeni referansı.
// | eglemSayisi (opsiyonel) : Okunan konsol eğlem sayısını al.
// @returns Alınan eğlem başarılı mı? Başarısız ise 'GetLastError' fonksiyonu veya 'errno' makrosu ile hata kodunu al.
bool KonsolEglem(INPUT_RECORD* kayit, DWORD* okunanEglemSayisi = nullptr);

// @brief
// Konsol boyutunu döndürür.
SIZE KonsolBoyutu();
// @brief 
// Konsolun boyutlandırılabilmesini açıp kapatır.
void KonsolPenceresiBoyutlandirilabilir(bool);

// @brief
// Konsol ekranını temizler.
void KonsolTemizle(const char dolgu = ' ');
