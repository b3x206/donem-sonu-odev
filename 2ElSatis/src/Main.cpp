// -*- encoding: utf-8 -*-
// Author(s) :
//     Barbaros Bayat : Veri işleme ve konsol arayüz fonksiyonları

// Güvenli olmayan fonksiyonlar?
// localtime() fonksiyonu kullanmak için, s_localtime'da kullanılabilir
#define _CRT_SECURE_NO_WARNINGS
// Kütüphaneleri içer
#include <iostream>  // Basit Giriş-Çıkış fonksiyonları
#include <vector>    // Dinamik (yeniden boyutlandırılabilen) dizi 
#include <array>     // Sabit boyutlu dizi 
#include <iomanip>   // Giriş-Çıkış manipülasyonu (formatlama vs.)
#include <string>    // Yazı içeren veri tipi (dizi)
#include <fstream>   // Dosya yazma-okuma kütüphanesi 
#include <sstream>   // String için formatlanabilir arabellek (cout gibi, ama stringe dönüştürmek için)
#include <clocale>   // Türkçe karakter için yerelleştirme kütüphanesi
#include <ctime>     // Zaman / Saat fonksiyonları (localtime vs.)
#include <algorithm> // std::sort

#include "konsol/pencere.h"
#include "konsol/arayuz.h"

using namespace std;

#pragma region Veri
const char VeriKaydetmeAyirac = '\n'; // her bir farklı struct için ayıraç
const char VeriDegiskenAyirac = ',';  // struct ve dizi içi değişkenleri
const char DiziKaydetmeBasSon = '"';  // Dizinin içindeki değerler
const char DiziDegiskenAyrac = 29;    // Dizinin içindeki değerler için ayıraç

// @brief time_t ile alınan zamanı okunabilir, GG/AA/YYYY formatlı zamana dönüştürür.
string ZamanStringeCevirTarih(const time_t zaman)
{
	const size_t yaziBoyutu = 24;
	char zamanYazisiAraBellek[yaziBoyutu];
	strftime(zamanYazisiAraBellek, yaziBoyutu, "%d/%m/%Y", localtime(&zaman));

	return string(zamanYazisiAraBellek);
}
// @brief time_t ile alınan zamanın saat:dakika:saniye olarak çevirir.
string ZamanStringeCevirSaat(const time_t zaman)
{
	const size_t yaziBoyutu = 12;
	char zamanYazisiAraBellek[yaziBoyutu];
	strftime(zamanYazisiAraBellek, yaziBoyutu, "%H:%M:%S", localtime(&zaman));

	return string(zamanYazisiAraBellek);
}

// -- Prototip
struct Urun;
struct Kullanici;
typedef uint64_t fiyat_t; // Fiyat bir 100 bölü sayısı (fixed point gibi ama değil)
string FiyatStringe(const fiyat_t fiyat)
{
	fiyat_t solKol = fiyat / 100;
	fiyat_t sagKol = fiyat % 100;

	string sonuc = to_string(solKol);
	if (sagKol > 0)
	{
		sonuc.append(".").append(to_string(sagKol));
	}

	return sonuc;
}

fiyat_t StringFiyata(const string& fiyat)
{
	if (fiyat.empty())
	{
		return 0;
	}

	const char basamakAyirac = ',' /*use_facet<numpunct<char>>(locale("")).decimal_point()*/;
	size_t ayiracIndis;
	for (ayiracIndis = 0; ayiracIndis < fiyat.size(); ayiracIndis++)
	{
		if (fiyat.at(ayiracIndis) == basamakAyirac)
		{
			break;
		}
	}

	size_t ayiracOncesi = 0, ayiracSonrasi = 0;
	ayiracOncesi = stoull(fiyat.substr(0, ayiracIndis));
	if (ayiracIndis < fiyat.size() - 1)
	{
		ayiracSonrasi = stoull(fiyat.substr(ayiracIndis + 1));
	}

	return (ayiracOncesi * 100) + ayiracSonrasi;
}

const string KullaniciDosyasiIsmi = "Kullanici.txt";
vector<Kullanici> Kullanicilar;

const string UrunlerDosyasiIsmi = "Urunler.txt";
// @brief Ürün listesi.
vector<Urun> Urunler;

// ----
// Satılan ürün ile ilgili veriyi içerir
struct Urun
{
	enum class Durum
	{
		Normal, Satildi
	};

	// tek değişkenler üste
	size_t sahipKimligi = 0;
	time_t ilanZamani = 0;
	string isim;
	string aciklama;
	fiyat_t fiyat = 0;
	size_t sayi = 0;
	// diziler alta
	vector<string> etiketler;
};
bool UrunlerEsit(const Urun u1, const Urun u2)
{
	return u1.sahipKimligi == u2.sahipKimligi &&
		u1.ilanZamani == u2.ilanZamani &&
		u1.isim == u2.isim &&
		u1.fiyat == u2.fiyat &&
		u1.sayi == u2.sayi &&
		u1.aciklama == u2.aciklama &&
		u1.etiketler == u2.etiketler;
}
// Urun icin herhangi bir string içeren değişkenin stringinin karakterlerden ayıklanması.
string UrunStringKarakterAyikla(string etiket)
{
	// Veri karakterlerini kaldır
	StringKarakterleriKaldir(etiket, { DiziKaydetmeBasSon, VeriDegiskenAyirac, DiziDegiskenAyrac });
	return etiket;
}
// Urunu tek satıra sığıcak şekilde bir dosyaya yazılabilen kaydedilebilir bir hale getirir.
string UrunKaydedilebilirHal(const Urun urun)
{
	stringstream yazi;

	// bilgileri ekle
	yazi << urun.sahipKimligi << VeriDegiskenAyirac <<
		urun.ilanZamani << VeriDegiskenAyirac <<
		UrunStringKarakterAyikla(urun.isim) << VeriDegiskenAyirac <<
		UrunStringKarakterAyikla(urun.aciklama) << VeriDegiskenAyirac << 
		urun.fiyat << VeriDegiskenAyirac <<
		urun.sayi << VeriDegiskenAyirac;

	// etiketleri ekle
	for (size_t i = 0; i < urun.etiketler.size(); i++)
	{
		// İlk etiketteysek yine kaydetme karakterini koy.
		if (i == 0)
		{
			yazi << DiziKaydetmeBasSon;
		}

		string etiket = urun.etiketler.at(i);
		// bosEtiket = etiket.size() == 0 || etiket.ilk_(bosluk)_olmayan_karakter_pozisyonu == -1 (yok)
		bool bosEtiket = etiket.empty() || etiket.find_first_not_of(' ') == string::npos;
		// Boş etiket ise yaziya yazdirma
		if (!bosEtiket)
		{
			yazi << UrunStringKarakterAyikla(etiket);
		}

		// Son etiketteysek dizi kaydetme karakterinin sonunu koy.
		if (i == urun.etiketler.size() - 1)
		{
			yazi << DiziKaydetmeBasSon;
		}
		else if (!bosEtiket)
		{
			// Boş etiket ise ayıraç koyma
			yazi << DiziDegiskenAyrac;
		}
	}
	if (urun.etiketler.size() <= 0)
		yazi << DiziKaydetmeBasSon << DiziKaydetmeBasSon;

	yazi << VeriKaydetmeAyirac;

	return yazi.str();
}
// string'den kayıtlı ürün yükler.
bool KayitliUrunCevir(const string& urunVerisi, Urun* hedefUrun)
{
	vector<string> parcaliVeri = StringParcala(urunVerisi, VeriDegiskenAyirac);

	if (hedefUrun == nullptr)
	{
		cerr << "[KayitliUrunCevir] 'hedefUrun' isaretci parametresi bos." << endl;
		return false;
	}
	if (parcaliVeri.size() != 7)
	{
		cerr << "[KayitliUrunCevir] Hatali veya bozuk veri : " << urunVerisi << endl;
		return false;
	}

	// listedeki ilk 5 tek veri
	hedefUrun->sahipKimligi = stoull(parcaliVeri.at(0));
	hedefUrun->ilanZamani = stoll(parcaliVeri.at(1));
	hedefUrun->isim = parcaliVeri.at(2);
	hedefUrun->aciklama = parcaliVeri.at(3);
	hedefUrun->fiyat = stoull(parcaliVeri.at(4));
	hedefUrun->sayi = stoull(parcaliVeri.at(5));

	// indis '6' (son indis), etiket dizisi verisini içeriyor
	if (parcaliVeri.at(6) == "\"\"") // Boş etiketler dizisi değeri = "".
	{
		return true;
	}

	// Parcali etiket verisini oku
	vector<string> parcaliEtiketVerisi = StringParcala(parcaliVeri.at(6), DiziDegiskenAyrac);
	hedefUrun->etiketler.reserve(parcaliEtiketVerisi.size());
	for (size_t i = 0; i < parcaliEtiketVerisi.size(); i++)
	{
		hedefUrun->etiketler.push_back(UrunStringKarakterAyikla(parcaliEtiketVerisi.at(i)));
	}

	return true;
}
// --
// Ürünleri 'Urunler' dizisine yükler
// Hatalı bir olay olursa 'false' getirir.
bool UrunleriYukle()
{
	ifstream urunlerDosyasi(UrunlerDosyasiIsmi);

	if (urunlerDosyasi.bad())
		return false;

	string veri;
	while (getline(urunlerDosyasi, veri))
	{
		Urun yuklenen;
		if (!KayitliUrunCevir(veri, &yuklenen))
			return false;

		// Ürün zaten ürünler dizisinde varmı yokmu kontrol et.
		for (size_t i = 0; i < Urunler.size(); i++)
		{
			if (UrunlerEsit(Urunler.at(i), yuklenen))
			{
				// 2 aynı ürünü koyma
				continue;
			}
		}

		Urunler.push_back(yuklenen);
	}

	// Ürünleri kronolojik sıraya göre sırala
	// en yeni ürün : en baş indis
	sort(Urunler.begin(), Urunler.end(), [](const Urun& u1, const Urun& u2) { return u1.ilanZamani > u2.ilanZamani; });

	urunlerDosyasi.close();
	return true;
}
// 'Urunler' dinamik dizisini kaydeder.
// Kaydetme esnasında hata olursa 'false' getirir.
bool UrunleriKaydet()
{
	// Sadece ürünlerin boş olup olmadığını kontrol et
	// Ürünler yüklendikten sonra halen boş olabilir.
	if (Urunler.empty())
		UrunleriYukle();

	ofstream urunlerDosyasi(UrunlerDosyasiIsmi, ios::trunc);

	if (urunlerDosyasi.bad())
		return false;

	for (size_t i = 0; i < Urunler.size(); i++)
	{
		urunlerDosyasi << UrunKaydedilebilirHal(Urunler.at(i));
	}

	urunlerDosyasi.close();
	return true;
}

// ----
// Kullanıcı ile ilgili verileri içerir.
struct Kullanici
{
	string ad, soyad;
	string sifre;
};
bool KullanicilarEsit(const Kullanici k1, const Kullanici k2)
{
	return k1.ad == k2.ad && k1.soyad == k2.soyad && k1.sifre == k2.sifre;
}
// Kullanici icin bir basit kimlik hesaplama fonksiyonu.
// NOT 1 : Aynı kullanıcı icin fonksiyonun sonucu her zaman aynı olmalıdır.
// NOT 2 : Kullanıcının adı, soyadı ve şifresi aynı olursa 
// NOT 3 : Kimlik çakışması, düzgün bir hash fonksiyonu kullanmadığım için olası ama düşük bir ihtimal.
size_t KullaniciKimligiHesapla(const Kullanici k)
{
	size_t sonuc = k.ad.size() + k.soyad.size();

	for (size_t i = 0; i < k.ad.size(); i++)
	{
		sonuc += k.ad.at(i) ^ 32;
	}
	for (size_t i = 0; i < k.soyad.size(); i++)
	{
		sonuc += k.soyad.at(i) ^ 64;
	}
	for (size_t i = 0; i < k.sifre.size(); i++)
	{
		sonuc += k.sifre.at(i) ^ 16;
	}

	return sonuc;
}
string KullaniciKaydedilebilirHal(Kullanici k)
{
	stringstream yazi;
	StringBoslukAyikla(k.ad);
	StringBoslukAyikla(k.soyad);

	yazi << k.ad << VeriDegiskenAyirac << k.soyad << VeriDegiskenAyirac << k.sifre << VeriKaydetmeAyirac;
	return yazi.str();
}
bool KayitliKullaniciCevir(const string& kullaniciVerisi, Kullanici* hedefKullanici)
{
	vector<string> veriler = StringParcala(kullaniciVerisi, VeriDegiskenAyirac);

	if (hedefKullanici == nullptr)
	{
		cerr << "[KayitliKullaniciCevir] Verilen 'hedefKullanici' isaretci parametresi bos." << endl;
		return false;
	}
	// 0 = ad, 1 = soyad, 2 = sifre
	if (veriler.size() != 3)
	{
		cerr << "[KayitliKullaniciCevir] Hatali veya bozuk veri : " << kullaniciVerisi << endl;
		return false;
	}

	hedefKullanici->ad = veriler.at(0);
	hedefKullanici->soyad = veriler.at(1);
	hedefKullanici->sifre = veriler.at(2);

	return true;
}
// --
// Kullanıcıları 'Kullanicilar' dizisine yükler
// Hatalı bir olay olursa 'false' getirir.
bool KullanicilariYukle()
{
	ifstream kullanicilarDosyasi(KullaniciDosyasiIsmi);

	if (kullanicilarDosyasi.bad())
		return false;

	string veri;
	while (getline(kullanicilarDosyasi, veri))
	{
		Kullanici yuklenen;

		if (!KayitliKullaniciCevir(veri, &yuklenen))
			return false;

		// Ürün zaten ürünler dizisinde varmı yokmu kontrol et.
		for (size_t i = 0; i < Kullanicilar.size(); i++)
		{
			if (KullanicilarEsit(Kullanicilar.at(i), yuklenen))
			{
				// 2 aynı ürünü koyma
				continue;
			}
		}

		Kullanicilar.push_back(yuklenen);
	}

	kullanicilarDosyasi.close();
	return true;
}
// 'Kullanicilar' dinamik dizisini kaydeder.
// Kaydetme esnasında hata olursa 'false' getirir.
bool KullanicilariKaydet()
{
	// Sadece ürünlerin boş olup olmadığını kontrol et
	// Ürünler yüklendikten sonra halen boş olabilir.
	if (Kullanicilar.empty())
		KullanicilariYukle();

	ofstream kullanicilarDosyasi(KullaniciDosyasiIsmi, ios::trunc);

	if (kullanicilarDosyasi.bad())
		return false;

	for (size_t i = 0; i < Kullanicilar.size(); i++)
	{
		kullanicilarDosyasi << KullaniciKaydedilebilirHal(Kullanicilar.at(i));
	}

	kullanicilarDosyasi.close();
	return true;
}

// ----
// -- Ürün/Kullanıcı fonksiyonları
// ----

// Sahip kimliğinden ürün bulur. Hatalı olay olursa boş bir kullanıcı getirir.
Kullanici UrunKullaniciBul(const size_t sahipKimligi)
{
	Kullanici k;
	k.ad = u8"Boş veya silinmiş";
	k.soyad = u8"Kullanıcı";
	if (Kullanicilar.empty())
	{
		if (!KullanicilariYukle())
		{
			cerr << "[UrunKullaniciBul] KullanicilariYukle() basarisiz." << endl;
			return k;
		}
	}

	for (size_t i = 0; i < Kullanicilar.size(); i++)
	{
		if (KullaniciKimligiHesapla(Kullanicilar.at(i)) == sahipKimligi)
		{
			k = Kullanicilar.at(i);
			break;
		}
	}

	return k;
}
Kullanici UrunKullaniciBul(const Urun urun)
{
	return UrunKullaniciBul(urun.sahipKimligi);
}

// Kullanici'nın KullaniciKimligiHesapla() fonksiyonunu kullanıp Urunlerini bul.
vector<Urun> KullanicininUrunleriniBul(const Kullanici k)
{
	vector<Urun> kullaniciUrunler;

	if (Urunler.empty())
	{
		if (!UrunleriYukle())
		{
			cerr << "[KullanicininUrunleriniBul] 'UrunleriYukle' basarisiz." << endl;
			return kullaniciUrunler;
		}
	}

	size_t kimlik = KullaniciKimligiHesapla(k);
	if (kimlik == 0)
	{
		return vector<Urun>();
	}

	for (size_t i = 0; i < Urunler.size(); i++)
	{
		if (Urunler.at(i).sahipKimligi == kimlik)
		{
			kullaniciUrunler.push_back(Urunler.at(i));
		}
	}

	return kullaniciUrunler;
}
#pragma endregion

#pragma region Arayuz ^ Kullanici
// Arama sonucu :
// Arama, etiketleri ve ürünün adını arıyacak
// Vefa Sorun : Aramanın karakterleri tamamen eşleşmeli.s
// -----------
// Etiketler ve ürün isimleri std::tolower ile kıyaslanacak
// (not : extended ascii setlocale(LC_ALL, "tr") ile tolower() ile alakalı sorun çıkabilir)
// bu nedenle aramada tolower kullanılmayabilir. (stackoverflow'da utf8 karakter çözümü olabilir ama bilmiyorum?)

// Barbaros : Tam kontrollü arayüz fonksiyonları <arayuz.h>'de.
//
//// Vefa Arayüz :
//// Windows konsol boyutu : 120 x  28
////
//// Giriş Sayfası :
//// 
////     Giriş
//// -----------------------------------------------------------------------------------------------------------------------
////                                    Ad
//// ├---------------------------┤ (35) [ ├--------------------------------┤ ] (38 (-2 []), 1)
//// 
////                                    Soyad
////                                    [                                    ]
//// 
////                                    Şifre (Büyük / Küçük harf duyarlı)
////                                    [                                    ]
//// 
//// Ana Sayfa : 
//// (seçili olan sayfanın numarası gözükmeyecek)
//// 
////                      [ Ana Sayfa ] [ 1 : Arama                          ] [ 2 : Profilim ]
//// -----------------------------------------------------------------------------------------------------------------------
////     "Ürün Listesi" (kronolojik olarak sırala)
////     
//// (4) -----------------------------  ├---┤(12) -----------------------------  ├---┤(12) ----------------------------- (4)
////     | [ EÖ1 ] urun.isim         |            | [ EÖ1 ] urun.isim         |            | [ EÖ1 ] urun.isim         |
////     | urun.fiyat ₺ • urun.zaman |            | urun.fiyat ₺ • urun.zaman |            | urun.fiyat ₺ • urun.zaman |
////     | UrunKullaniciBul(urun).ad |            | UrunKullaniciBul(urun).ad |            | UrunKullaniciBul(urun).ad |
////     -----------------------------            -----------------------------            -----------------------------
//// 
//// Ürün Sayfası :
////                    [ < Geri (3) ] [ 1 : Arama                             ] [ 2 : Profilim ]
//// -----------------------------------------------------------------------------------------------------------------------
////     Urun.isim
////     --------- (utf8_length(Urun.isim))
////     Satıcı : UrunKullaniciBul(Urun).ad + soyad
////     Urun.fiyat ₺ • Urun.ilanZamani + Urun.ilanZamani+T(hh:mm)
////     Urun.sayi > 0 ? Stokta Urun.sayi tane kaldı. : Stokta yok.
////     
////     [ Satın Al ] (not : bu eklenmeyebilir)
////     [ Ürünü Sil ] (bu sadece profil aynı ise var)
////
////     Urun.aciklama
////     -------------------------------------------------------------------------------------------------------------------
////     Etiketler : Urun.etiketler
//// 
//// Ürün Ekle Sayfası :
////                   [ < İptal (3) ] [ 1 : Arama                             ] [ 2 : Profilim ]
//// -----------------------------------------------------------------------------------------------------------------------
////     İsim :   [ Urun.isim                   ]
////     ---------
////     Satıcı : OturumBilgisi::kullanici.ad + OturumBilgisi::kullanici.soyad
////     Fiyat  : [ Urun.fiyat                  ] ₺ • Urun.ilanZamani (gg/aa/yyyy) Urun.ilanZamani (hh:mm:ss)
////     Sayı   : [ 0 (stokta yok)              ]
////     
////     Açıklama :  [ Açıklama buraya gelir.                                                                    ]
////                 [                                                                                           ]
////     -------------------------------------------------------------------------------------------------------------------
////     Etiketler : [ Etiketleri virgül ile ayır.                                                               ]
////                 [                                                                                           ]
////
////      (x=KonsolBoyut().cx / 2 - ((w=14) / 2))       [              ]
////                                                    [     Ekle     ] 
////                                                    [              ]
//// 
//// Profilim : 
////                   [ 3 : Ana Sayfa ] [ 1 : Arama                             ] [ < Geri (2) ]
//// -(13)------------------------------------------------------------------------------------------------------------------
//// ---- [ * Görünüm ] [ Ürünler ] [ Şifre ] [ Hesap ] --------------------------------------------------------------------
////   
////             (x=19) Ad :    [ yerTutucu : Kullanici.ad                                  ]  (w=61)
////                    Soyad : [ yerTutucu : Kullanici.soyad                               ]
////                                                                                         
////                    Doğrulama Şifre : [                                                 ]  (w=51)
//// 
////      (x=KonsolBoyut().cx / 2 - ((w=22) / 2)) [                      ]
////                                              [        Uygula        ] 
////                                              [                      ]
//// 
//// -(5) Örnek bilgilendirme yazısı.
//// -----------------------------------------------------------------------------------------------------------------------
//// ---- [ Görünüm ] [ * Ürünler ] [ Şifre ] [ Hesap ] --------------------------------------------------------------------
////
////      ArayuzUrunlerListesi(5, Urunler.Where(urun => KullanicilarEsit(OturumBilgisi::kullanici, UrunKullaniciBul(urun)))); 
//// 
//// -----------------------------------------------------------------------------------------------------------------------
//// ---- [ Görünüm ] [ Ürünler ] [ * Şifre ] [ Hesap ] --------------------------------------------------------------------
////      
////             (x=18) Önceki Şifre :         [                                            ] (w=48)
////                                                                                        
////                    Yeni Şifre :           [                                            ]
////                    Yeni Şifre Doğrula :   [                                            ]
////
////
////      (x=KonsolBoyut().cx / 2 - ((w=22) / 2)) [                      ]
////                                              [        Uygula        ] 
////                                              [                      ]
//// 
//// -(5) Örnek bilgilendirme yazısı.
//// -----------------------------------------------------------------------------------------------------------------------
//// ---- [ Görünüm ] [ Ürünler ] [ Şifre ] [ * Hesap ] --------------------------------------------------------------------
//// 
////             (x=18) Şifre :         [                                                   ] (w=50)
////                    Şifre Doğrula : [                                                   ]
////
////
////      (x=KonsolBoyut().cx / 2 - ((w=22) / 2)) [                      ] (e/h doğrulama box)
////                                              [      Hesap Sil       ] 
////                                              [                      ]
//// 
//// -(5) Örnek bilgilendirme yazısı.

enum class Menu
{
	Yok,                      // Tanımsız
	Giris, Kaydol,            // Giriş menüsü/menüleri
	AnaSayfa, AramaSonuc,     // Ürün dizisi
	Profil, Urun, UrunEkle    // Kullanıcı
};

const size_t SayfaBasiMaxUrunSayisi = 9;
bool arayuzUstBarElemanlarKisayolSecilebilir = true;

namespace OturumBilgisi
{
	// @brief Şu anki seçili menü.
	Menu SeciliMenu = Menu::Giris;
	// @brief Şu anki girilen kullanıcı. (not : şifre bozuk bir stringdir)
	Kullanici kullanici;
	// @brief Şu anda bakılan ürün. (işaretci ref olarak alınır)
	Urun* onizleUrun = nullptr;
	// @brief Arama yerine yazılan yazı.
	string arananYazi;
	// @brief Önceki arama yerine yazılan yazı.
	string oncekiArananYazi;
	// @brief Şu anda bulunulan sayfa.
	size_t Sayfa;
	// @brief Uygulamadan çıkış yapılıp yapılmadığı.
	bool cikisYapildi;

	void MenuDegistir(const Menu hedef)
	{
		// - reset
		ArayuzTemizle(true);
		if (!arananYazi.empty() && hedef != Menu::AramaSonuc)
		{
			arananYazi.clear();
		}
		Sayfa = 0;

		// - set
		SeciliMenu = hedef;
	}
}

// @brief Arayüz için giriş formu yazdırır.
// @param pozisyon [in] : Yazdırılıcak formun pozisyonu. NOT : formun sol tarafındaki tüm yazılar silinir.                  
// loginForm [in] : formun bir giriş formu olup olmadığında göre yazılarını değiştirir.                                            
// yaziAlaniBoyutu [in] : Formun yazı girilecek noktalarının uzunluğu. Not: Bu formun yüksekliği her zaman 6'dır.
// outKullanici [out] : Girilen sonuç kullanıcı. Kullanıcıyı doğrulamak için dışarıda bir 'Dugme()' fonksiyonu kullanılmalıdır.
void ArayuzBilgiGirisFormu(const COORD pozisyon, const int yaziAlaniBoyutu, const bool loginForm, Kullanici& outKullanici)
{
	static bool (*virgulYok)(const int64_t) = [](const int64_t ch) { return ch != ','; };

	const SIZE konsolBoyutu = KonsolBoyutu();

	Yazi(pozisyon, (loginForm ? u8"Ad (Büyük / Küçük harf duyarlı)" : u8"Ad"));
	Yazi(COORD{ pozisyon.X, static_cast<SHORT>(pozisyon.Y + 2) }, (loginForm ? u8"Soyad (Büyük / Küçük harf duyarlı)" : u8"Soyad"));
	Yazi(COORD{ pozisyon.X, static_cast<SHORT>(pozisyon.Y + 4) }, (loginForm ? u8"Şifre (Büyük / Küçük harf duyarlı)" : u8"Şifre"));

	const auto& adAlan = YaziAlani(ArayuzAlan(pozisyon.X, pozisyon.Y + 1, yaziAlaniBoyutu, 1), &outKullanici.ad, false, virgulYok);
	if (AnaGrup.seciliArayuzElemani != adAlan.indis)
	{
		StringBoslukAyikla(outKullanici.ad);
	}

	const auto& soyadAlan = YaziAlani(ArayuzAlan(pozisyon.X, pozisyon.Y + 3, yaziAlaniBoyutu, 1), &outKullanici.soyad, false, virgulYok);
	if (AnaGrup.seciliArayuzElemani != soyadAlan.indis)
	{
		StringBoslukAyikla(outKullanici.soyad);
	}

	const auto& sifreAlan = YaziAlani(ArayuzAlan(pozisyon.X, pozisyon.Y + 5, yaziAlaniBoyutu, 1), &outKullanici.sifre, true, virgulYok);
	if (AnaGrup.seciliArayuzElemani != sifreAlan.indis)
	{
		StringBoslukAyikla(outKullanici.sifre);
	}

	arayuzUstBarElemanlarKisayolSecilebilir = AnaGrup.seciliArayuzElemani != adAlan.indis && 
		AnaGrup.seciliArayuzElemani != soyadAlan.indis && 
		AnaGrup.seciliArayuzElemani != sifreAlan.indis;
}
// @brief Arayüz için ortalı [ <  Geri ] [ İleri > ] şeklinde düğme yazdırır.
// OturumBilgisi::Sayfa isimli değişkeni değiştirir. Düğmelerden herhangi biri basıldıysa 'true' döndürür.
// Eğer 'Urunler' dışında farklı bir dizi var ise (<yön>DevamEdilebilir) parametreleriyle kontrol edilebilir.
bool ArayuzIleriGeriDugme(const COORD pozisyon, const bool geriDevamEdilebilir = true, const bool ileriDevamEdilebilir = true)
{
	const SIZE konsolBoyutu = KonsolBoyutu();
	const size_t oncekiSayfa = OturumBilgisi::Sayfa;
	const size_t maxSayfa = (Urunler.size() / SayfaBasiMaxUrunSayisi); // sayfa 0 indisli
	if (OturumBilgisi::Sayfa > maxSayfa)
	{
		OturumBilgisi::Sayfa = maxSayfa;
	}

	ArayuzIzgaraNav(2);
	AnaGrup.yaziHizasi = YaziPozisyonu::Orta;
	AnaGrup.elemanlarSecilebilir = geriDevamEdilebilir && OturumBilgisi::Sayfa > 0;
	if (Dugme(ArayuzAlan(pozisyon.X - 1, pozisyon.Y, 10, 1), u8" <  Geri "))
	{
		OturumBilgisi::Sayfa--;
	}
	AnaGrup.elemanlarSecilebilir = ileriDevamEdilebilir && (Urunler.size() > ((OturumBilgisi::Sayfa + 1) * SayfaBasiMaxUrunSayisi));
	if (Dugme(ArayuzAlan(pozisyon.X + 10 + 1, pozisyon.Y, 10, 1), u8" İleri > "))
	{
		OturumBilgisi::Sayfa++;
	}
	AnaGrup.elemanlarSecilebilir = true;
	
	YaziPozisyonu oncekiPos = AnaGrup.yaziHizasi;
	AnaGrup.yaziHizasi = YaziPozisyonu::Orta;
	Yazi(ArayuzAlan(pozisyon.X, pozisyon.Y + 1, 20, 1), string(u8"Sayfa : ").append(to_string(OturumBilgisi::Sayfa + 1)));
	AnaGrup.yaziHizasi = oncekiPos;

	return oncekiSayfa != OturumBilgisi::Sayfa;
}
bool ArayuzIleriGeriDugme(const int yPozisyon, const bool geriDevamEdilebilir = true, const bool ileriDevamEdilebilir = true)
{
	const SIZE konsolBoyutu = KonsolBoyutu();

	return ArayuzIleriGeriDugme(COORD{ (SHORT)((konsolBoyutu.cx / 2) - 10), (SHORT)yPozisyon }, geriDevamEdilebilir, ileriDevamEdilebilir);
}

// @brief Arayüz üst barı için elemanlar yazdırır.
// @param
//      'Yazi' ile biten tüm değişkenler : Düğme yazıları
//      'Menu' ile biten değişkenler : Düğmelere tıklandığında olucak eğlem
//      'aramaBarBosIseDegistir' : Arama barındaki yazı boşsa menü değişimi gerçekleşir.
// @returns Ana fonksiyon içerisinde 'return;' yapılıp yapılmaması gerektiği.
bool ArayuzUstBarElemanlar(const string& solDugmeYazi, const Menu solDugmeMenu, const string& aramaBarYerTutucu, const Menu aramaBarMenu, const bool aramaBarBosIseDegistir, const string& sagDugmeYazi, const Menu sagDugmeMenu)
{
	SIZE konsolBoyutu = KonsolBoyutu();

	const size_t solDugmeYaziUzunluk = utf8_length(solDugmeYazi) + 1;
	const size_t sagDugmeYaziUzunluk = utf8_length(sagDugmeYazi) + 1;
	const size_t aramaBarUzunluk = 50;

	AnaGrup.yaziHizasi = YaziPozisyonu::Orta;

	int baslangicPos = konsolBoyutu.cx - (solDugmeYaziUzunluk + 2 + aramaBarUzunluk + 2 + sagDugmeYaziUzunluk + 2 + 13);
	baslangicPos /= 2;

	const auto& aramaElemani = YaziAlani(ArayuzAlan(baslangicPos + (solDugmeYaziUzunluk + 2), 0, aramaBarUzunluk, 1), &OturumBilgisi::arananYazi, aramaBarYerTutucu, 
		arayuzUstBarElemanlarKisayolSecilebilir ? '1' : '\0');
	// 'AramaSonuc'u ArayuzSonuc arayüz döngüsünde otomatik olarak yenile?
	if (AnaGrup.seciliArayuzElemani != aramaElemani.indis && (aramaBarBosIseDegistir ? OturumBilgisi::arananYazi.empty() : !OturumBilgisi::arananYazi.empty()))
	{
		OturumBilgisi::oncekiArananYazi = OturumBilgisi::arananYazi;
		OturumBilgisi::MenuDegistir(aramaBarMenu);

		// don't really know if i want to keep this.
		static bool eggFlag = false;
		if (OturumBilgisi::arananYazi == u8"rickroll" && !eggFlag)
		{
			ShellExecute(nullptr, L"open", L"https://youtu.be/dQw4w9WgXcQ", nullptr, nullptr, SW_HIDE);
			eggFlag = true;
		}

		return true; // we can 'return true;' as the entire function was called, and no weird bugs will occur (like those buttons)
	}

	// w : 18
	Menu degisMenu = Menu::Yok;
	const bool secTusBasilabilir = AnaGrup.seciliArayuzElemani != aramaElemani.indis && arayuzUstBarElemanlarKisayolSecilebilir;
	if (Dugme(ArayuzAlan(baslangicPos, 0, solDugmeYaziUzunluk, 1), solDugmeYazi, solDugmeMenu != Menu::Yok ? '\r' : '\0', secTusBasilabilir ? '3' : '\0'))
	{
		degisMenu = solDugmeMenu; // don't 'return true;' or change menus here as it doesn't get out of scope completely, causing a stray interface element
	}
	baslangicPos += aramaBarUzunluk + 2;     // arama barı zaten önceden çiziliyor (secTusBasilabilir)
	baslangicPos += solDugmeYaziUzunluk + 2;

	if (Dugme(ArayuzAlan(baslangicPos, 0, sagDugmeYaziUzunluk, 1), sagDugmeYazi, sagDugmeMenu != Menu::Yok ? '\r' : '\0', secTusBasilabilir ? '2' : '\0'))
	{
		degisMenu = sagDugmeMenu;
	}
	baslangicPos += sagDugmeYaziUzunluk + 2;

	if (Dugme(ArayuzAlan(baslangicPos, 0, 12, 1), u8" Q : Çıkış ", '\r', secTusBasilabilir ? 'q' : '\0'))
	{
		OturumBilgisi::cikisYapildi = true;
	}

	if (degisMenu != Menu::Yok)
	{
		OturumBilgisi::MenuDegistir(degisMenu);
		return true;
	}

	return false;
}

// @brief Ürünleri listeli olarak çizer. (sadece çizilen y pozisyonu değiştirilebilir, ürünler tam ekran çizilir)
// @returns Son kullanılan Y pozisyonu.
bool ArayuzUrunlerListesi(const int yPozisyon, vector<Urun>& urunler, size_t* sonYPos = nullptr)
{
	const SIZE konsolBoyut = KonsolBoyutu();
	ArayuzIzgaraNav(3); // 3 sütün

	stringstream urunSS;
	const int cizUrunSayisi = ((SayfaBasiMaxUrunSayisi * (OturumBilgisi::Sayfa + 1)) < urunler.size() ? (SayfaBasiMaxUrunSayisi * (OturumBilgisi::Sayfa + 1)) : urunler.size()) -
		(SayfaBasiMaxUrunSayisi * OturumBilgisi::Sayfa);

	size_t posX = 4, posY = yPozisyon;
	for (size_t i = 0; cizUrunSayisi > -1 && i < cizUrunSayisi; i++)
	{
		const size_t urunIndisi = i * (OturumBilgisi::Sayfa + 1);
		const SIZE dugmeBoyut = { 30, 3 };
		const SIZE araBosluk = { 11, 1 };

		Urun& u = urunler.at(urunIndisi);
		Kullanici k = UrunKullaniciBul(u);
		urunSS.clear();
		urunSS.str(string());

		urunSS << u.isim << '\n'
			<< FiyatStringe(u.fiyat) << u8" ₺ • " << ZamanStringeCevirTarih(u.ilanZamani) << '\n'
			<< k.ad << " " << k.soyad;

		ElemanIDIpucu(i);
		if (Dugme(ArayuzAlan(posX, posY, dugmeBoyut.cx, dugmeBoyut.cy), urunSS.str()))
		{
			OturumBilgisi::onizleUrun = &u;
			OturumBilgisi::MenuDegistir(Menu::Urun);
	
			if (sonYPos != nullptr)
			{
				*sonYPos = posY;
			}
			return true;
		}

		posX += dugmeBoyut.cx + araBosluk.cx;
		if (posX > konsolBoyut.cx)
		{
			posY += dugmeBoyut.cy + araBosluk.cy;
			posX = 4;
		}

		if (i == cizUrunSayisi - 1)
		{
			if (ArayuzIleriGeriDugme(posY + (posX == 4 ? 0 : dugmeBoyut.cy + araBosluk.cy) + 1))
			{
				ArayuzTemizle(true);
				return posY;
			}
		}
	}

	if (sonYPos != nullptr)
	{
		*sonYPos = posY;
	}
	return false;
}

bool ArayuzUstBarYazdir()
{
	KonsolImlecPozisyonuAyarla(0, 0);
	SIZE konsolBoyutu = KonsolBoyutu();

	switch (OturumBilgisi::SeciliMenu)
	{
	default:
	{
		WORD oncekiRenk = AnaGrup.varsayilanRenk;
		AnaGrup.varsayilanRenk = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
		Yazi({ 0, 0 }, string(u8"Interface unimplemented / Arayüz yok : ").append(to_string((int)OturumBilgisi::SeciliMenu)));
		AnaGrup.varsayilanRenk = oncekiRenk;
		break;
	}

	case Menu::Giris:
	{
		AnaGrup.yaziHizasi = YaziPozisyonu::Sag;
		Yazi({ 0, 0, 14, 1 }, u8"[ Giriş ]");
		break;
	}
	case Menu::Kaydol:
	{
		AnaGrup.yaziHizasi = YaziPozisyonu::Sol;
		Yazi({ 20, 0, 14, 1 }, u8"[ Kaydol ]");
		AnaGrup.yaziHizasi = YaziPozisyonu::Orta;
		if (Dugme({ 4, 0, 14, 1 }, u8"< (3) Geri ", '\r', arayuzUstBarElemanlarKisayolSecilebilir ? '3' : '\0'))
		{
			OturumBilgisi::MenuDegistir(Menu::Giris);
			return true;
		}
		break;
	}
	case Menu::AramaSonuc:
	{
		if (ArayuzUstBarElemanlar(u8" < Geri (3) ", Menu::AnaSayfa,
			u8" 1 : Ara... ", Menu::AnaSayfa, true,
			u8" 2 : Profilim ", Menu::Profil))
		{
			return true;
		}
		break;
	}
	case Menu::AnaSayfa:
	{
		if (ArayuzUstBarElemanlar(u8" Ana Sayfa ", Menu::Yok,
			u8" 1 : Ara... ", Menu::AramaSonuc, false,
			u8" 2 : Profilim ", Menu::Profil))
		{
			return true;
		}
		break;
	}

	case Menu::Urun:
	{
		if (ArayuzUstBarElemanlar(u8" < Geri (3) ", OturumBilgisi::arananYazi.empty() ? Menu::AnaSayfa : Menu::AramaSonuc,
			u8" 1 : Ara... ", Menu::AramaSonuc, false,
			u8" 2 : Profilim ", Menu::Profil))
		{
			return true;
		}
		break;
	}

	case Menu::Profil:
	{
		if (ArayuzUstBarElemanlar(u8" < Geri (3) ", Menu::AnaSayfa,
			u8" 1 : Ara... ", Menu::AramaSonuc, false,
			u8" 2 : Profilim ", Menu::Yok))
		{
			return true;
		}
		break;
	}
	case Menu::UrunEkle:
		if (ArayuzUstBarElemanlar(u8" < İptal (3) ", Menu::AnaSayfa,
			u8" 1 : Ara... ", Menu::AramaSonuc, false,
			u8" 2 : Profilim ", Menu::Profil))
		{
			return true;
		}
		break;
	}

	// foreground => ön plan
	KonsolRenkAyarla(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	KonsolImlecPozisyonuAyarla(0, 1);
	cout << setw(konsolBoyutu.cx) << setfill('-') << "" << setfill(' ');
	KonsolRenkAyarla(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY); // renkleri karıştırarak beyaz renk yap
	return false;
}

// @brief Giriş formuna alınan kullanıcı.
static Kullanici alinanKullanici;
// @brief Ürün ekleme formuna girilen ürün.
static Urun alinanUrun;

// @brief Bilgi girme menüsünü yazdırıp sonuç kullanıcıyı götürür.
void ArayuzGiris()
{
	static bool kullaniciBulunmadi = false; // Kullanıcı yok
	static bool kullaniciHatali = false;    // Girilen şifre hatalı (kullanıcı yok sayılır)
	static bool hataOldu = false;           // 'KullanicilariYukle' fonksiyonu hatalı değil ise.

	const SIZE konsolBoyutu = KonsolBoyutu();
	const size_t yaziAlaniBoslukAlan = 35;
	const COORD formPozisyonu = { yaziAlaniBoslukAlan, 3 };
	const int yaziAlaniBoyutu = konsolBoyutu.cx - (yaziAlaniBoslukAlan * 2);

	ArayuzBilgiGirisFormu(formPozisyonu, yaziAlaniBoyutu, true, alinanKullanici);

	ArayuzIzgaraNav(2);
	// -- Düğmeler
	AnaGrup.yaziHizasi = YaziPozisyonu::Orta;
	if (Dugme(ArayuzAlan(formPozisyonu.X + (yaziAlaniBoyutu / 2) - 21, formPozisyonu.Y + (6 + 2), 20, 3), u8" \nGiriş"))
	{
		if (Kullanicilar.empty())
		{
			if (!KullanicilariYukle())
			{
				hataOldu = true;
			}
		}

		bool ayniIsimliKullaniciVar = false;
		alinanKullanici.sifre = StringBoz(alinanKullanici.sifre, { ',' });
		for (size_t i = 0; i < Kullanicilar.size(); i++)
		{
			if (Kullanicilar.at(i).ad == alinanKullanici.ad && Kullanicilar.at(i).soyad == alinanKullanici.soyad)
			{
				ayniIsimliKullaniciVar = true;
			}

			if (KullanicilarEsit(Kullanicilar.at(i), alinanKullanici))
			{
				OturumBilgisi::kullanici = Kullanicilar.at(i);
			}
		}

		kullaniciBulunmadi = KullaniciKimligiHesapla(OturumBilgisi::kullanici) == 0;
		kullaniciHatali = ayniIsimliKullaniciVar && kullaniciBulunmadi;
		alinanKullanici.sifre.clear();

		if (!kullaniciBulunmadi)
		{
			alinanKullanici.ad.clear();
			alinanKullanici.soyad.clear();
			alinanKullanici.sifre.clear();

			OturumBilgisi::MenuDegistir(Menu::AnaSayfa);
			return;
		}
	}
	if (Dugme(ArayuzAlan(formPozisyonu.X + (yaziAlaniBoyutu / 2) + 1, formPozisyonu.Y + (6 + 2), 20, 3), u8" \nKayıt"))
	{
		// Bu fonksiyonu çağırdıktan sonra arayüz yazdırmaya devam etme 
		// (yoksa kod devam ettiği için sonradan boş arayüz elemanları geçiyor)
		OturumBilgisi::MenuDegistir(Menu::Kaydol);
		// Şifre alanını sil
		alinanKullanici.sifre.clear();

		return;
	}

	// -- Uyarılar
	if (kullaniciBulunmadi)
	{
		WORD oncekiRenk = AnaGrup.varsayilanRenk;
		AnaGrup.varsayilanRenk = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
		Yazi(ArayuzAlan(4, konsolBoyutu.cy - 2, 0, 0), kullaniciHatali ? u8"Kullanıcı şifresi yanlış." : u8"Böyle bir kullanıcı yok.\nLütfen 'Kayıt' seçeneğini seçerek yeni kayıt yaptırın.");
		AnaGrup.varsayilanRenk = oncekiRenk;
	}
	if (hataOldu)
	{
		WORD oncekiRenk = AnaGrup.varsayilanRenk;
		AnaGrup.varsayilanRenk = FOREGROUND_RED | FOREGROUND_INTENSITY;
		Yazi(ArayuzAlan(4, konsolBoyutu.cy - 2, 0, 0), u8"Giriş esnasında bir hata oluştu.");
		AnaGrup.varsayilanRenk = oncekiRenk;
	}
}

// @brief Kaydolmak için bir bilgi giriş menüsü verir.
void ArayuzKaydol()
{
	static bool kullaniciZatenVar = false;
	static bool kullaniciBos = false; // 'alinanKullanici' yerinde boş alanlar var.
	static bool hataOldu = false;

	const SIZE konsolBoyutu = KonsolBoyutu();
	const size_t yaziAlaniBoslukAlan = 35;
	const COORD formPozisyonu = { yaziAlaniBoslukAlan, 3 };
	const int yaziAlaniBoyutu = konsolBoyutu.cx - (yaziAlaniBoslukAlan * 2);

	ArayuzBilgiGirisFormu(formPozisyonu, yaziAlaniBoyutu, false, alinanKullanici);

	ArayuzIzgaraNav(2);
	// -- Düğmeler
	AnaGrup.yaziHizasi = YaziPozisyonu::Orta;
	if (Dugme(ArayuzAlan(formPozisyonu.X + (yaziAlaniBoyutu / 2) - 10, formPozisyonu.Y + (6 + 2), 20, 3), u8" \nKayıt Yap"))
	{
		kullaniciBos = alinanKullanici.ad.empty() || alinanKullanici.soyad.empty() || alinanKullanici.sifre.empty();

		if (kullaniciBos)
		{
			alinanKullanici.sifre.clear();
			return;
		}

		alinanKullanici.sifre = StringBoz(alinanKullanici.sifre, { ',' });

		for (size_t i = 0; i < Kullanicilar.size(); i++)
		{
			if (alinanKullanici.ad == Kullanicilar.at(i).ad && alinanKullanici.soyad == Kullanicilar.at(i).soyad)
			{
				kullaniciZatenVar = true;
				alinanKullanici.sifre.clear();

				return;
			}
		}

		// kullanıcı yok.
		Kullanicilar.push_back(alinanKullanici);
		OturumBilgisi::kullanici = alinanKullanici;

		if (!KullanicilariKaydet())
		{
			hataOldu = true;
			return;
		}

		alinanKullanici.sifre.clear();
		OturumBilgisi::MenuDegistir(Menu::AnaSayfa);
		return;
	}

	if (kullaniciBos)
	{
		WORD oncekiRenk = AnaGrup.varsayilanRenk;
		AnaGrup.varsayilanRenk = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
		Yazi(ArayuzAlan(4, konsolBoyutu.cy - 2, 0, 0), u8"Girilen alanlarda boş yerler var. Lütfen tüm alanları doldurun.");

		AnaGrup.varsayilanRenk = oncekiRenk;
	}
	else if (kullaniciZatenVar)
	{
		WORD oncekiRenk = AnaGrup.varsayilanRenk;
		AnaGrup.varsayilanRenk = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
		Yazi(ArayuzAlan(4, konsolBoyutu.cy - 2, 0, 0), u8"Verilen isimli kullanıcı zaten var.");

		AnaGrup.varsayilanRenk = oncekiRenk;
	}
	else if (hataOldu)
	{
		WORD oncekiRenk = AnaGrup.varsayilanRenk;
		AnaGrup.varsayilanRenk = FOREGROUND_RED | FOREGROUND_INTENSITY;
		Yazi(ArayuzAlan(4, konsolBoyutu.cy - 2, 0, 0), u8"Kaydolma esnasında bir hata oluştu.");

		AnaGrup.varsayilanRenk = oncekiRenk;
	}
}

// @brief Ana sayfayı çizer. Ana sayfa içerisinde son konulan ürünler gösterilir.
void ArayuzAnaSayfa()
{
	ArayuzUrunlerListesi(4, Urunler);
}

// @brief Arama sonuçlarını çizer. OturumBilgisi::arananYazi ve static bir dizi kullanır.
void ArayuzAramaSonuc()
{
	static vector<Urun> urunList;

	// TODO : utf8 tolower? veya utf8 karakter sekansının unicode cpsi alınıp std::towlower veya _wcslwr kullanılabilir?
	if (OturumBilgisi::arananYazi != OturumBilgisi::oncekiArananYazi)
	{
		for (size_t i = 0; i < Urunler.size(); i++)
		{
			Urun& u = Urunler.at(i);
			if (u.isim.find(OturumBilgisi::arananYazi) != string::npos || find(u.etiketler.begin(), u.etiketler.end(), OturumBilgisi::arananYazi) != u.etiketler.end())
			{
				urunList.push_back(u);
			}
		}
	}

	OturumBilgisi::oncekiArananYazi = OturumBilgisi::arananYazi;

	ArayuzUrunlerListesi(4, urunList);
}

// @brief Profil arayüzünü çizer. OturumBilgisi::kullanici'yı kullanır.
void ArayuzProfil()
{
	const SIZE konsolBoyut = KonsolBoyutu();
	static int seciliIcMenu = 0; // 0 : görünüm, 1 : arama, 2 : şifre
	int oncekiIcMenu = seciliIcMenu;

	Yazi({ 0, 2 }, string(4, '-'));

	ArayuzIzgaraNav(4); // sekme sayısı
	AnaGrup.yaziHizasi = YaziPozisyonu::Orta;
	if (Dugme(ArayuzAlan(5, 2, 12, 1), seciliIcMenu == 0 ? u8" * Görünüm " : u8" Görünüm "))
	{
		seciliIcMenu = 0;
	}
	if (Dugme(ArayuzAlan(19, 2, 12, 1), seciliIcMenu == 1 ? u8" * Ürünler " : u8" Ürünler "))
	{
		seciliIcMenu = 1;
	}
	if (Dugme(ArayuzAlan(33, 2, 12, 1), seciliIcMenu == 2 ? u8" * Şifre " : u8" Şifre "))
	{
		seciliIcMenu = 2;
	}
	if (Dugme(ArayuzAlan(47, 2, 12, 1), seciliIcMenu == 3 ? u8" * Hesap " : u8" Hesap "))
	{
		seciliIcMenu = 3;
	}
	Yazi({ 61, 2 }, string(59, '-'));

	static bool isimSoyisimAyni = false, sifreYanlis = false, dogrulamaYanlis = false, hataOldu = false;
	static bool (*virgulYok)(const int64_t) = [](const int64_t ch) { return ch != ','; };

	if (seciliIcMenu != oncekiIcMenu)
	{
		isimSoyisimAyni = false;
		sifreYanlis = false;
		dogrulamaYanlis = false;
		hataOldu = false;
		ArayuzTemizle(true);
		return;
	}

	switch (seciliIcMenu)
	{
	default:
		seciliIcMenu = 0;
		break;

	case 0:  // 'Görünüm'
	{
		Yazi({ 10, 4 }, string(u8"Kullanıcı kimliği : #").append(to_string(KullaniciKimligiHesapla(OturumBilgisi::kullanici))));

		Yazi({ 19, 5 }, u8"Ad : ");
		arayuzUstBarElemanlarKisayolSecilebilir = YaziAlani(ArayuzAlan(27, 5, 61, 1), &alinanKullanici.ad, OturumBilgisi::kullanici.ad, false, virgulYok).indis != AnaGrup.seciliArayuzElemani;

		Yazi({ 19, 6 }, u8"Soyad : ");
		arayuzUstBarElemanlarKisayolSecilebilir &= YaziAlani(ArayuzAlan(27, 6, 61, 1), &alinanKullanici.soyad, OturumBilgisi::kullanici.soyad, false, virgulYok).indis != AnaGrup.seciliArayuzElemani;

		Yazi({ 19, 8 }, u8"Doğrulama Şifre : ");
		arayuzUstBarElemanlarKisayolSecilebilir &= YaziAlani(ArayuzAlan(37, 8, 51, 1), &alinanKullanici.sifre, u8"********", true).indis != AnaGrup.seciliArayuzElemani;

		if (Dugme(ArayuzAlan((konsolBoyut.cx / 2) - 11, 10, 22, 3), u8" \nUygula"))
		{
			alinanKullanici.sifre = StringBoz(alinanKullanici.sifre, { ',' });
			sifreYanlis = OturumBilgisi::kullanici.sifre != alinanKullanici.sifre;
			isimSoyisimAyni = OturumBilgisi::kullanici.ad == alinanKullanici.ad && OturumBilgisi::kullanici.soyad == alinanKullanici.soyad;

			if (!sifreYanlis && !isimSoyisimAyni)
			{
				const size_t oncekiKimlik = KullaniciKimligiHesapla(OturumBilgisi::kullanici);
				for (size_t i = 0; i < Kullanicilar.size(); i++)
				{
					if (OturumBilgisi::kullanici.ad == Kullanicilar.at(i).ad && OturumBilgisi::kullanici.soyad == Kullanicilar.at(i).soyad)
					{
						// isimi bu indiste değiştir + oturum açan kullanıcının ismini değiştir.
						// OturumBilgisi::kullanici, ürün gibi işaretci olsaydı sorun olmazdı.
						Kullanicilar.at(i).ad          = alinanKullanici.ad;
						Kullanicilar.at(i).soyad       = alinanKullanici.soyad;
						OturumBilgisi::kullanici.ad    = alinanKullanici.ad;
						OturumBilgisi::kullanici.soyad = alinanKullanici.soyad;
					
						alinanKullanici.ad.clear();
						alinanKullanici.soyad.clear();

						hataOldu = !KullanicilariKaydet();
						break;
					}
				}
				const size_t yeniKimlik = KullaniciKimligiHesapla(OturumBilgisi::kullanici);

				// belki : oncekiKimlik ile yeniKimlik kıyaslanarak hata önleme?

				// Ürünleri güncelle
				for (size_t i = 0; i < Urunler.size(); i++)
				{
					if (Urunler.at(i).sahipKimligi == oncekiKimlik)
					{
						Urunler.at(i).sahipKimligi = yeniKimlik;
					}
				}

				hataOldu &= !UrunleriKaydet();
			}

			alinanKullanici.sifre.clear();
		}

		SHORT bilgilendirmeYazisiY = 14;
		if (sifreYanlis)
		{
			Yazi({ 5, bilgilendirmeYazisiY }, u8"Verilen doğrulama şifresi hatalı.");
			bilgilendirmeYazisiY++;
		}
		if (isimSoyisimAyni)
		{
			Yazi({ 5, bilgilendirmeYazisiY }, u8"Verilen isim ve soyisim zaten şu ankiyle aynı.");
			bilgilendirmeYazisiY++;
		}
		if (hataOldu)
		{
			Yazi({ 5, bilgilendirmeYazisiY }, u8"'KullanicilariKaydet()' veya 'UrunleriKaydet()' fonksiyonu hata döndürdü.");
			bilgilendirmeYazisiY++;
		}
		break;
	}
	case 1:  // 'Ürünler'
	{
		static vector<Urun> kullaniciUrunler;
		static size_t oncekiUrunSayisi = 0;
		if (Urunler.size() != oncekiUrunSayisi)
		{
			kullaniciUrunler.clear();
		
			for (size_t i = 0; i < Urunler.size(); i++)
			{
				if (KullanicilarEsit(OturumBilgisi::kullanici, UrunKullaniciBul(Urunler.at(i))))
				{
					kullaniciUrunler.push_back(Urunler.at(i));
				}
			}

			oncekiUrunSayisi = Urunler.size();
		}

		size_t sonYPos = 4;
		if (kullaniciUrunler.empty())
		{
			Yazi({ 4, (SHORT)sonYPos }, u8"Ürününüz yok.\n'Ürün Ekle' yazan düğmeye gelip 'enter' basarak ürün ekleyebilirsiniz.");
		}
		else
		{
			if (ArayuzUrunlerListesi(4, kullaniciUrunler, &sonYPos))
			{
				return;
			}
		}

		if (Dugme(ArayuzAlan(4, sonYPos + 4, 19, 3), u8" \n(4) Ürün Ekle ", '\r', '4'))
		{
			OturumBilgisi::MenuDegistir(Menu::UrunEkle);
			return;
		}
		break;
	}
	case 2: // 'Şifre'
	{
		static string oncekiSifre, yeniSifre, yeniDogrulamaSifre;

		Yazi({ 19, 4 }, u8"Önceki Şifre : ");
		arayuzUstBarElemanlarKisayolSecilebilir = YaziAlani(ArayuzAlan(39, 4, 51, 1), &oncekiSifre, u8"********", true).indis != AnaGrup.seciliArayuzElemani;

		Yazi({ 19, 6 }, u8"Yeni Şifre : ");
		arayuzUstBarElemanlarKisayolSecilebilir &= YaziAlani(ArayuzAlan(39, 6, 51, 1), &yeniSifre, true).indis != AnaGrup.seciliArayuzElemani;

		Yazi({ 19, 7 }, u8"Yeni Şifre Doğrula : ");
		arayuzUstBarElemanlarKisayolSecilebilir &= YaziAlani(ArayuzAlan(39, 7, 51, 1), &yeniDogrulamaSifre, true).indis != AnaGrup.seciliArayuzElemani;

		if (Dugme(ArayuzAlan((konsolBoyut.cx / 2) - 11, 9, 22, 3), u8" \nUygula"))
		{
			oncekiSifre = StringBoz(oncekiSifre, { ',' });
			yeniSifre = StringBoz(yeniSifre, { ',' });
			yeniDogrulamaSifre = StringBoz(yeniDogrulamaSifre, { ',' });

			sifreYanlis = OturumBilgisi::kullanici.sifre != oncekiSifre;
			dogrulamaYanlis = yeniSifre != yeniDogrulamaSifre;

			if (!sifreYanlis && !dogrulamaYanlis)
			{
				const size_t oncekiKimlik = KullaniciKimligiHesapla(OturumBilgisi::kullanici);
				for (size_t i = 0; i < Kullanicilar.size(); i++)
				{
					if (KullanicilarEsit(OturumBilgisi::kullanici, Kullanicilar.at(i)))
					{
						// isimi bu indiste değiştir + oturum açan kullanıcının ismini değiştir.
						// OturumBilgisi::kullanici, ürün gibi işaretci olsaydı sorun olmazdı.
						Kullanicilar.at(i).sifre = yeniSifre;
						OturumBilgisi::kullanici.sifre = yeniSifre;

						hataOldu = !KullanicilariKaydet();
						break;
					}
				}
				const size_t yeniKimlik = KullaniciKimligiHesapla(OturumBilgisi::kullanici);
				// belki : oncekiKimlik ile yeniKimlik kıyaslanarak hata önleme?

				// Ürünleri güncelle
				for (size_t i = 0; i < Urunler.size(); i++)
				{
					if (Urunler.at(i).sahipKimligi == oncekiKimlik)
					{
						Urunler.at(i).sahipKimligi = yeniKimlik;
					}
				}

				hataOldu &= !UrunleriKaydet();

				oncekiSifre.clear();
				yeniSifre.clear();
				yeniDogrulamaSifre.clear();
			}
			
			oncekiSifre.clear();
			yeniSifre.clear();
			yeniDogrulamaSifre.clear();
		}

		SHORT bilgilendirmeYazisiY = 14;
		if (sifreYanlis)
		{
			Yazi({ 5, bilgilendirmeYazisiY }, u8"Verilen önceki şifre hatalı.");
			bilgilendirmeYazisiY++;
		}
		if (dogrulamaYanlis)
		{
			Yazi({ 5, bilgilendirmeYazisiY }, u8"Verilen doğrulama şifresi hatalı.");
			bilgilendirmeYazisiY++;
		}
		if (hataOldu)
		{
			Yazi({ 5, bilgilendirmeYazisiY }, u8"'KullanicilariKaydet()' veya 'UrunleriKaydet()' fonksiyonu hata döndürdü.");
			bilgilendirmeYazisiY++;
		}
		break;
	}
	case 3: // 'Hesap'
	{
		static bool silmeDugmeBasildi = false;
		static string hesapSifre, dogrulaSifre;

		Yazi({ 19, 4 }, u8"Şifre : ");
		arayuzUstBarElemanlarKisayolSecilebilir = YaziAlani(ArayuzAlan(35, 4, 50, 1), &hesapSifre, u8"********", true).indis != AnaGrup.seciliArayuzElemani;

		Yazi({ 19, 5 }, u8"Şifre Doğrula : ");
		arayuzUstBarElemanlarKisayolSecilebilir &= YaziAlani(ArayuzAlan(35, 5, 50, 1), &dogrulaSifre, u8"********", true).indis != AnaGrup.seciliArayuzElemani;

		if (Dugme(ArayuzAlan((konsolBoyut.cx / 2) - 11, 9, 22, 3), u8" \nHesap Sil"))
		{
			hesapSifre = StringBoz(hesapSifre, { ',' });
			dogrulaSifre = StringBoz(dogrulaSifre, { ',' });

			sifreYanlis = OturumBilgisi::kullanici.sifre != hesapSifre;
			dogrulamaYanlis = hesapSifre != dogrulaSifre;
			silmeDugmeBasildi = !sifreYanlis && !dogrulamaYanlis;

			if (sifreYanlis)
			{
				hesapSifre.clear();
				dogrulaSifre.clear();
			}
			if (dogrulamaYanlis)
			{
				dogrulaSifre.clear();
			}

			if (silmeDugmeBasildi)
			{
				hesapSifre.clear();
				dogrulaSifre.clear();			
			}
		}

		if (silmeDugmeBasildi)
		{
			AnaGrup.elemanlarSecilebilir = true;
			Yazi(ArayuzAlan((konsolBoyut.cx / 2) - 25, 10, 0, 0), u8"[!! UYARI !!]\nBu kullanıcı profili silindiğinde, şunlarda silinecektir :\nÜrünler\nDevam etmek istediğinizden emin misiniz?");
			if (Dugme(ArayuzAlan((konsolBoyut.cx / 2) - 25, 15, 10, 1), u8"(1) Evet", '\r', '1'))
			{
				const size_t kimlik = KullaniciKimligiHesapla(OturumBilgisi::kullanici);
				for (size_t i = 0; i < Kullanicilar.size(); i++)
				{
					if (KullanicilarEsit(OturumBilgisi::kullanici, Kullanicilar.at(i)))
					{
						// isimi bu indiste değiştir + oturum açan kullanıcının ismini değiştir.
						// OturumBilgisi::kullanici, ürün gibi işaretci olsaydı sorun olmazdı.
						vector<Kullanici>::iterator it = Kullanicilar.begin();
						advance(it, i);

						Kullanicilar.erase(it);
						hataOldu = !KullanicilariKaydet();
						break;
					}
				}

				// Olan ürünleri sil
				vector<Urun>::iterator it = find_if(Urunler.begin(), Urunler.end(),
					[kimlik](const Urun& u)
					{
						return u.sahipKimligi == kimlik;
					});
				while (it != Urunler.end())
				{
					Urunler.erase(it);

					it = find_if(Urunler.begin(), Urunler.end(),
					[kimlik](const Urun& u)
					{
						return u.sahipKimligi == kimlik;
					});
				}

				hataOldu &= !UrunleriKaydet();

				hesapSifre.clear();
				dogrulaSifre.clear();

				OturumBilgisi::kullanici.ad.clear();
				OturumBilgisi::kullanici.soyad.clear();
				OturumBilgisi::kullanici.sifre.clear();

				OturumBilgisi::MenuDegistir(Menu::Giris);
				return;
			}
			if (Dugme(ArayuzAlan((konsolBoyut.cx / 2) - 9, 15, 10, 1), u8"(2) Hayır", '\r', '2'))
			{
				silmeDugmeBasildi = false;
				OncekiCizilenArayuzElemanSil(true);
				OncekiCizilenArayuzElemanSil(true);
				OncekiCizilenArayuzElemanSil(true);
			}

			AnaGrup.elemanlarSecilebilir = false;
		}
		else
		{
			AnaGrup.elemanlarSecilebilir = true;
		}

		SHORT bilgilendirmeYazisiY = 14;
		if (sifreYanlis)
		{
			Yazi({ 5, bilgilendirmeYazisiY }, u8"Verilen önceki şifre hatalı.");
			bilgilendirmeYazisiY++;
		}
		if (dogrulamaYanlis)
		{
			Yazi({ 5, bilgilendirmeYazisiY }, u8"Verilen doğrulama şifresi hatalı.");
			bilgilendirmeYazisiY++;
		}
		break;
	}
	}
}

// @brief Ürün görünümünü çizer. OturumBilgisi::Urun'ü kullanır.
void ArayuzUrun()
{
	if (OturumBilgisi::onizleUrun == nullptr)
	{
		WORD oncekiRenk = AnaGrup.varsayilanRenk;
		AnaGrup.varsayilanRenk = FOREGROUND_RED | FOREGROUND_GREEN;
		Yazi({ 4, 4 }, u8"OturumBilgisi::onizleUrun nullptr! Üst menüyü kullanarak geri dön veya debugger.");
		AnaGrup.varsayilanRenk = oncekiRenk;
		return;
	}

	const SIZE konsolBoyutu = KonsolBoyutu();
	const Kullanici sahipKullanici = UrunKullaniciBul(*OturumBilgisi::onizleUrun);

	stringstream ss;
	ss << OturumBilgisi::onizleUrun->isim << '\n'
		<< setfill('-') << setw(utf8_length(OturumBilgisi::onizleUrun->isim)) << "" << setfill(' ') << setw(0) << '\n'
		<< FiyatStringe(OturumBilgisi::onizleUrun->fiyat) << u8" ₺ • İlan Tarihi : " << ZamanStringeCevirTarih(OturumBilgisi::onizleUrun->ilanZamani) << u8" " << ZamanStringeCevirSaat(OturumBilgisi::onizleUrun->ilanZamani) << '\n'
		<< u8"Satıcı : " << sahipKullanici.ad << " " << sahipKullanici.soyad << '\n';
	if (OturumBilgisi::onizleUrun->sayi <= 0)
	{
		ss << u8"Stokta yok.";
	}
	else
	{
		ss << u8"Stokta " << OturumBilgisi::onizleUrun->sayi << u8" tane kaldı.";
	}
	Yazi({ 4, 4 }, ss.str());

	//if (Dugme(ArayuzAlan(4, 8, 12, 1), u8" Satın al "))
	//{
	//    // satın alma buraya gider? (not : bu uygulamanın amacı olan ürünleri ve fiyatları listelemek, satın alma değil)
	//}
	if (KullanicilarEsit(OturumBilgisi::kullanici, sahipKullanici))
	{
		static bool silBasildi = false, silDogrulandi = false;

		if (!silBasildi)
		{
			if (Dugme(ArayuzAlan(4, 9, 13, 1), u8" Ürünü Sil "))
			{
				// TODO : doğrulama ve static değer.
				silBasildi = true;
				OncekiCizilenArayuzElemanSil(true);
			}
		}
		else if (!silDogrulandi)
		{
			if (Dugme(ArayuzAlan(4, 9, 8, 1), u8" Evet? "))
			{
				vector<Urun>::iterator it = Urunler.begin();
				for (size_t i = 0; i < Urunler.size(); i++)
				{
					if (UrunlerEsit(*OturumBilgisi::onizleUrun, Urunler.at(i)))
					{
						advance(it, i);
						break;
					}
				}

				Urunler.erase(it);
				OturumBilgisi::onizleUrun = nullptr; // ürün zaten dizideki elemana işaretci.
				OturumBilgisi::MenuDegistir(Menu::AramaSonuc);
				UrunleriKaydet();

				return;
			}
			if (Dugme(ArayuzAlan(13, 9, 10, 1), u8" Hayır? "))
			{
				silBasildi = false;
				silDogrulandi = false;
			}

			if (!silBasildi)
			{
				// 2 düğmeyi sil
				OncekiCizilenArayuzElemanSil(true);
				OncekiCizilenArayuzElemanSil(true);
			}
		}
	}

	ss.str(string());
	ss << u8"Açıklama : " << (OturumBilgisi::onizleUrun->aciklama.empty() ? u8"<Boş>" : OturumBilgisi::onizleUrun->aciklama) << '\n'
		<< setfill('-') << setw(konsolBoyutu.cx - 4) << "" << setfill(' ') << setw(0) << '\n'
		<< u8"Etiketler : ";
	if (OturumBilgisi::onizleUrun->etiketler.empty())
	{
		ss << u8"<Boş>";
	}

	for (size_t i = 0; i < OturumBilgisi::onizleUrun->etiketler.size(); i++)
	{
		ss << OturumBilgisi::onizleUrun->etiketler.at(i) << (i == OturumBilgisi::onizleUrun->etiketler.size() - 1 ? "" : ", ");
	}
	Yazi({ 4, 11 }, ss.str());
}

// @brief Ürün ekleme arayüzünü gösterir.
void ArayuzUrunEkle()
{
	const SIZE konsolBoyut = KonsolBoyutu();
	static bool bosAlanVar = false;
	static string tarihString;
	if (tarihString.empty())
	{
		tarihString = ZamanStringeCevirTarih(time(nullptr));
	}
	
	static bool (*virgulYok)(const int64_t) = [](const int64_t ch) { return ch != ','; };
	static bool (*sadeceRakam)(const int64_t) = [](const int64_t ch) { return ((bool)isdigit(ch)) || ch == '\b'; };
	static bool (*sadeceRakamVirgullu)(const int64_t) = [](const int64_t ch) { return ((bool)isdigit(ch)) || ch == ',' || ch == '\b'; };

	Yazi({ 4, 3 }, u8"İsim : ");
	arayuzUstBarElemanlarKisayolSecilebilir = YaziAlani(ArayuzAlan(13, 3, 31, 1), &alinanUrun.isim, false, virgulYok).indis != AnaGrup.seciliArayuzElemani;
	Yazi({ 4, 4 }, string(8, '-'));

	Yazi({ 4, 5 }, string(u8"Satıcı : ").append(OturumBilgisi::kullanici.ad).append(u8" ").append(OturumBilgisi::kullanici.soyad));

	Yazi({ 4, 6 }, u8"Fiyat : ");
	static string fiyatString = u8"0";
	arayuzUstBarElemanlarKisayolSecilebilir &= YaziAlani(ArayuzAlan(13, 6, 31, 1), &fiyatString, false, sadeceRakamVirgullu).indis != AnaGrup.seciliArayuzElemani;
	for (size_t i = 0, virgulSayisi = 0; i < fiyatString.size(); i++)
	{
		char fiyatK = fiyatString.at(i);
		if (fiyatK == ',')
		{
			// Gereksiz virgülleri sil
			if (virgulSayisi >= 1)
			{
				string::iterator it = fiyatString.begin();
				advance(it, i);
				fiyatString.erase(it);
				i--;
				continue;
			}

			virgulSayisi++;
		}
	}
	Yazi({ 38, 6 }, string(u8"₺ • ").append(tarihString));
	alinanUrun.fiyat = StringFiyata(fiyatString);

	string sayiString = to_string(alinanUrun.sayi);
	Yazi({ 4, 7 }, u8"Sayı : ");
	arayuzUstBarElemanlarKisayolSecilebilir &= YaziAlani(ArayuzAlan(13, 7, 31, 1), &sayiString, false, sadeceRakam).indis != AnaGrup.seciliArayuzElemani;
	try
	{
		alinanUrun.sayi = stoull(sayiString);
	}
	catch (const exception& _) { sayiString = "0"; }

	Yazi({ 4, 9 }, u8"Açıklama : ");
	arayuzUstBarElemanlarKisayolSecilebilir &= YaziAlani(ArayuzAlan(16, 9, 93, 2), &alinanUrun.aciklama, false, virgulYok).indis != AnaGrup.seciliArayuzElemani;
	
	Yazi({ 4, 11 }, string(konsolBoyut.cx - 4, '-'));

	static string etiketlerString;
	Yazi({ 4, 12 }, u8"Etiketler : ");
	arayuzUstBarElemanlarKisayolSecilebilir &= YaziAlani(ArayuzAlan(16, 12, 93, 2), &etiketlerString, false).indis != AnaGrup.seciliArayuzElemani;

	AnaGrup.yaziHizasi = YaziPozisyonu::Orta;
	if (Dugme(ArayuzAlan((konsolBoyut.cx / 2) - 7, 15, 14, 3), u8" \nEkle"))
	{
		bosAlanVar = alinanUrun.isim.empty();

		if (bosAlanVar)
		{
			return;
		}

		// 'etiketlerString' hazırla
		vector<string> etiketler;
		string arabellek;
		for (size_t i = 0; i < etiketlerString.size(); i++)
		{
			const char ayiracKarakter = ',';
			char strKarakter = etiketlerString.at(i);
		
			if (strKarakter == ayiracKarakter)
			{
				StringBoslukAyikla(arabellek);
			
				if (!arabellek.empty())
				{
					etiketler.push_back(arabellek);
					arabellek.clear();
				}

				continue;
			}

			arabellek.push_back(strKarakter);
		}

		// 'alinanUrun' hazırla
		alinanUrun.etiketler = etiketler;
		alinanUrun.ilanZamani = time(nullptr);
		alinanUrun.sahipKimligi = KullaniciKimligiHesapla(OturumBilgisi::kullanici);

		// 'Urunler' dizisine ekle + kaydet
		Urunler.insert(Urunler.begin(), alinanUrun);

		alinanUrun.isim.clear();
		alinanUrun.aciklama.clear();
		alinanUrun.etiketler.clear();
		alinanUrun.fiyat = 0;
		alinanUrun.sahipKimligi = 0;

		tarihString.clear();

		UrunleriKaydet();

		// Ürünü önizle (OturumBilgisi::onizleUrun = &Urunler.at(0); OturumBilgisi::MenuDegistir(Menu::Urun);)
		OturumBilgisi::onizleUrun = &Urunler.at(0);
		OturumBilgisi::MenuDegistir(Menu::Urun);
		return;
	}

	if (bosAlanVar)
	{
		Yazi({ 4, 18 }, u8"Ürün ismi boş.");
	}
}

#pragma endregion

int main()
{
	// Ön Ayarlama
	SetConsoleCP(CP_UTF8);       // Giriş kod alanını UTF8 olarak ayarla
	SetConsoleOutputCP(CP_UTF8); // Çıkış kod alanını UTF8 yap
	//// Bunun sayesinde türkçe yazıları setlocale'siz 'u8' prefix ile yazdırabiliriz.
	//// (zaten setlocale kullanınca bütün 'std::cin' yazıları bozuluyor idi, iyi oldu)
	//// Konsol penceresi yeniden boyutlandırılabilir ise yazı bozulur (ctrl ile yaklaştırılabilir ama)
	KonsolPenceresiBoyutlandirilabilir(false);

	if (!KullanicilariYukle())
	{
		cerr << u8"[main() -> Ön Ayarlama] KullanicilariYukle başarısız. Veri kaybı olabilir. (enter'a basarak devam et)" << endl;
		cin.ignore();
	}
	if (!UrunleriYukle())
	{
		cerr << u8"[main() -> Ön Ayarlama] UrunleriYukle başarısız. Veri kaybı olabilir. (enter'a basarak devam et)" << endl;
		cin.ignore();
	}

	// Insert the most recent item to be in the start.

	while (!OturumBilgisi::cikisYapildi)
	{
		if (ArayuzUstBarYazdir())
		{
			continue;
		}

		switch (OturumBilgisi::SeciliMenu)
		{
			// -- Olmayan arayüz
		default:
		{
			WORD oncekiRenk = AnaGrup.varsayilanRenk;
			AnaGrup.varsayilanRenk = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
			Yazi(ArayuzAlan(0, 3, 0, 0), string(u8"Interface unimplemented / Arayüz yok : ").append(to_string((int)OturumBilgisi::SeciliMenu)));
			AnaGrup.varsayilanRenk = oncekiRenk;
			break;
		}

		// -- Arayüzler
		case Menu::Giris:
			ArayuzGiris();
			break;
		case Menu::Kaydol:
			ArayuzKaydol();
			break;
		case Menu::Profil:
			ArayuzProfil();
			break;
		case Menu::AnaSayfa:
			ArayuzAnaSayfa();
			break;
		case Menu::AramaSonuc:
			ArayuzAramaSonuc();
			break;
		case Menu::Urun:
			ArayuzUrun();
			break;
		case Menu::UrunEkle:
			ArayuzUrunEkle();
			break;
		}
	}

	ArayuzTemizle(true);

	KonsolImlecPozisyonuAyarla(2, KonsolImlecPozisyonu().Y + 2);
	cout << (KullanicilariKaydet() && UrunleriKaydet() ? u8"Başarıyla kaydedildi." : u8"Kaydetme başarısız.");
	cout << u8" • Enter'a basarak kapat.";
	cin.ignore();

	return 0;
}
