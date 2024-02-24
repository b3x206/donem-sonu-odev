// -*- encoding: utf-8 -*-
#include "pencere.h"

// Konsol pencere işaretcisi. Bu konsolun yazı ile ilgili işaretcisi değil, pencere ayarları ile ilgili bir işaretci.
HWND konsolPenceresi = GetConsoleWindow();

WORD KonsolRenk()
{
    HANDLE konsol = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO ekranArabellek; // Ekran bilgisinin arabelleği

    // Arabellek bilgisini al
    GetConsoleScreenBufferInfo(konsol, &ekranArabellek);
    return ekranArabellek.wAttributes;
}
WORD KonsolRenkTers()
{
    WORD renk = KonsolRenk();

    // Hangi renklerin olduğunu kontrol et 
    // Var ise kaldır, yok ise ekle
    // TODO : bunun daha basit bir yöntemi olması lazım, ama bu çalışıyor
    if ((renk & BACKGROUND_RED) == BACKGROUND_RED)
        renk &= ~BACKGROUND_RED;
    else
        renk |= BACKGROUND_RED;

    if ((renk & BACKGROUND_GREEN) == BACKGROUND_GREEN)
        renk &= ~BACKGROUND_GREEN;
    else
        renk |= BACKGROUND_GREEN;

    if ((renk & BACKGROUND_BLUE) == BACKGROUND_BLUE)
        renk &= ~BACKGROUND_BLUE;
    else
        renk |= BACKGROUND_BLUE;

    if ((renk & BACKGROUND_INTENSITY) == BACKGROUND_INTENSITY)
        renk &= ~BACKGROUND_INTENSITY;
    else
        renk |= BACKGROUND_INTENSITY;

    if ((renk & FOREGROUND_RED) == FOREGROUND_RED)
        renk &= ~FOREGROUND_RED;
    else
        renk |= FOREGROUND_RED;

    if ((renk & FOREGROUND_GREEN) == FOREGROUND_GREEN)
        renk &= ~FOREGROUND_GREEN;
    else
        renk |= FOREGROUND_GREEN;

    if ((renk & FOREGROUND_BLUE) == FOREGROUND_BLUE)
        renk &= ~FOREGROUND_BLUE;
    else
        renk |= FOREGROUND_BLUE;

    if ((renk & FOREGROUND_INTENSITY) == FOREGROUND_INTENSITY)
        renk &= ~FOREGROUND_INTENSITY;
    else
        renk |= FOREGROUND_INTENSITY;

    return renk;
}
bool KonsolRenkAyarla(WORD renk)
{
    HANDLE konsol = GetStdHandle(STD_OUTPUT_HANDLE);
    return SetConsoleTextAttribute(konsol, renk); // Yazının rengini ayarla
}

bool KonsolImlec(bool goster)
{
    HANDLE konsol = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;        // İmleç bilgisi
    info.bVisible = !goster;         // Görünürlük
    info.dwSize = goster ? 100 : 0;  // Boyut

    return SetConsoleCursorInfo(konsol, &info);
}
COORD KonsolImlecPozisyonu(const SHORT kaymaX, const SHORT kaymaY)
{
    HANDLE konsol = GetStdHandle(STD_OUTPUT_HANDLE);
    
    CONSOLE_SCREEN_BUFFER_INFO arabellek;
    GetConsoleScreenBufferInfo(konsol, &arabellek);
    return { static_cast<SHORT>(arabellek.dwCursorPosition.X + kaymaX), static_cast<SHORT>(arabellek.dwCursorPosition.Y + kaymaY) };
}
bool KonsolImlecPozisyonuAyarla(const COORD nokta)
{
    HANDLE konsol = GetStdHandle(STD_OUTPUT_HANDLE);

    return SetConsoleCursorPosition(konsol, nokta);
}

bool KonsolEglemModu(const DWORD mode)
{
    HANDLE konsolIn = GetStdHandle(STD_INPUT_HANDLE);
    return SetConsoleMode(konsolIn, mode);
}

bool KonsolEglem(INPUT_RECORD* kayit, DWORD* okunanEglemSayisi)
{
    if (kayit == nullptr)
        return false;

    // Input okumak için farklı bir handle al. (yoksa handle bozuluyor)
    HANDLE konsolIn = GetStdHandle(STD_INPUT_HANDLE);
    DWORD tmp_okunanEglemSayisi = 0; // okunan eğlem sayısı (opsiyonel)
    
    // (konsol, giriş kaydı işaretcisi, giriş kaydı işaretcisi dizi ise dizi boyutu, eğlem sayısı)
    return ReadConsoleInput(konsolIn, kayit, 1, okunanEglemSayisi == nullptr ? &tmp_okunanEglemSayisi : okunanEglemSayisi);
}

SIZE KonsolBoyutu()
{
    HANDLE konsol = GetStdHandle(STD_OUTPUT_HANDLE);
    SIZE boyut; // sonuç boyut
    CONSOLE_SCREEN_BUFFER_INFO ekranArabellek; // Konsol ekran arabellek bilgisi

    // Arabellek bilgisini al
    GetConsoleScreenBufferInfo(konsol, &ekranArabellek);
    // dwSize = arabellek boyutu, X de doğru ama Y de yanlış
    // srWindow = pencere boyutu, Y hesaplaması için (çıkarma sonucunu short olarak castlemezsek derleme hatası)
    boyut = { ekranArabellek.dwSize.X, (SHORT)(ekranArabellek.srWindow.Bottom - ekranArabellek.srWindow.Top) };
    return boyut;
}
void KonsolPenceresiBoyutlandirilabilir(bool acik)
{
    LONG pencereStili = GetWindowLong(konsolPenceresi, GWL_STYLE); // Konsolun pencere ayarlarını al
    pencereStili &= acik ? WS_MAXIMIZEBOX & WS_SIZEBOX : ~WS_MAXIMIZEBOX & ~WS_SIZEBOX; // İzine göre yeniden boyutlandırma bitlerini aç veya kapat
    SetWindowLong(konsolPenceresi, GWL_STYLE, pencereStili); // Pencere ayarlarını uygula
}

void KonsolTemizle(const char dolgu)
{
    HANDLE konsol = GetStdHandle(STD_OUTPUT_HANDLE); // Konsol işaretcisi
    CONSOLE_SCREEN_BUFFER_INFO ekranArabellek;       // Ekran bilgisinin arabelleği
    DWORD konsolYazilan;                             // Ekrana yazılacakların arabelleği

    const WORD konsolBeyaz = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE; // Konsoldaki beyaz renk belirteci
    const COORD yukariSol = { 0, 0 }; // Yukari sol pozisyonu

    // Arabellek bilgisini al
    GetConsoleScreenBufferInfo(konsol, &ekranArabellek);
    // konsolYazilan arabellegini 'dolgu' parametresindeki karakter ile konsol boyutunda doldur
    FillConsoleOutputCharacterA(konsol, dolgu, ekranArabellek.dwSize.X * ekranArabellek.dwSize.Y, yukariSol, &konsolYazilan);
    // konsol'a konsolYazilan arabelleğini ayarla
    FillConsoleOutputAttribute(konsol, konsolBeyaz, ekranArabellek.dwSize.X * ekranArabellek.dwSize.Y, yukariSol, &konsolYazilan);
    // imleçi sıfırla
    KonsolImlecPozisyonuAyarla(yukariSol);
}