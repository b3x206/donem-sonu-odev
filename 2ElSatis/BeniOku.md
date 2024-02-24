--------------------------------------- Ana Notlar
# Proje ne hakkında?
2ElSatis projesi, kullanıcıların kullanılmış ürünlerini satışa çıkarmak veya almak için kullanabilecekleri bir uygulamadır. Aşağıda, projeyle ilgili daha fazla detayı genişletebiliriz:

• Kullanıcı Profili Oluşturma: Kullanıcılar, uygulamaya kaydolarak bir profil oluşturabilirler. Profil oluşturma işlemi, temel bilgiler (ad, soyad, şifre vb.) verilerek yeni bir profilin yerel makineye kaydedilmesidir.

• Ürün Listeleme: Kullanıcılar, uygulama üzerindeki konsol arayüzü veya klavye kontrolüyle ürünlerini ekleyebilirler. Bu işlem, ürünün adı, açıklaması, fiyatı ve diğer ilgili bilgilerin girilmesini içerebilir. Kullanıcılar, ürünleri kategorilere veya etiketlere göre düzenleyebilir.

• Gezinme ve Arama: Kullanıcılar, uygulama içindeki ürünler arasında gezinebilir ve istedikleri ürünleri arayabilirler. Etiket kullanarak istedikleri özelliklere veya kriterlere göre ürünleri sıralayabilirler. Ayrıca, kullanıcılar belirli bir ürünü arama çubuğuna yazarak doğrudan o ürünü bulabilirler.

# Uygulama nasıl kullanılır?

* Uygulama ilk açıldığında giriş yerindeki 3 yazı alanı `tab` tuşu kullanılarak gezinilebilir. Uygulamanın fare desteği yok (çünkü konsol fare desteği pek çalışmıyor). 
* Seçili eleman yeşil renktedir. (koddan `AnaGrup.seciliRenk` değişkeni, seçili olmayan nesneler ise `AnaGrup.varsayilanRenk` değişkeni ile kontrol edilir. 
* `AnaGrup`, arayuz.cpp'de; arayuz.h'de `extern` olarak tanımlı.)
* 3 yazı alanına veri girildikten sonra uygun olan alttaki 2 düğmeden biri seçilir.

* Ana menü açıldığında üstte ve altta menüler olucaktır.
Üstekki menüde arama barı, geri tuşu, profil ve çıkış tuşu olucaktır
üstekki menüdeki yazan harfler veya tuşlar basılır ise o elemanlar seçilir.

Alttaki menüde ise olan içerikler olucaktır. Bazı menüler ok tuşlarıylada gezinilebilir.

--------------------------------------- Diğer Notlar

> sahte kullanıcı isimleri, soyisimleri ve şifreleri bu şekilde
! Not : Kullanici.txt sadece veri depolar. Elle değiştirilirse şifre geri döndürülemez şekilde bozulur!
Sadece bu verileri uygulama üzerinden girin ve değiştirin.

```
Gülseren Göncü (id:545)  : GGoncu1984
Yavuz Selim    (id:702)  : YSBGiller1234
Beril Sadıç    (id:627)  : BSDÇ*123!!
Hanife Tuğçe   (id:599)  : HANTUĞAY949
Ayşe Ahsen     (id:1030) : hunter47
```

--------------------------------------- Nasıl yaptım?

`Sadece UTF-8             >` Konsola yazılan tüm yazılar ve olan tüm stringler utf-8 olmak üzere tasarlanmıştır.

`str_araclari.h           >` UTF-8 karakterleri stringte kullanabilmek için yapılan kullanışlı fonksiyonlar. Tüm bilgiler o dosyada bulunur. (ve birkaç daha farklı string fonksiyon bulunmaktadır)

`arayuz.cpp ve arayuz.h   >` Unity oyun motoru'nun GUI sınıfı/sisteminden esinlenerek konsolda klayve aracılığyla gezinilebilir ve etkileşilebilir arayüz elemanlarıyla 

`pencere.cpp ve pencere.h >` Windows'un konsol apisini basitleştirmek üzere yaptığım dosyalar.

`Güvenlik                 >` Şifrelerin hepsi bozulup depolanır. (not : std::hash kullanabilirdim [oda kriptografik değil] ama düzgün bir kriptografi kütüphanesi olmadığı sürece pek bir işe yaramaz)

`Main.cpp                 >` Ana uygulamanın hepsi burada. Arayüz elemanlarını modüler şekilde ayırıp OturumBilgisi::menu'ye göre farklı fonksiyonlar çağırılır.

--------------------------------------- Hatalar

`arayuz.cpp > ArayuzElemanID() >` Alınan ID, program 'Release' modunda derlendiğinde sürekli değişiyor.
* Olası çözümler :
1. Arabellek tabanlı bir arayüz çizimine geç. Her arayüz çizimi yapıldığında arabellek temizlenmeli ve yeni ID'ler yapılmalı.
2. Çağırılan arayüz fonksiyonları bir 'ArayuzListesi' gibi veri tipine eklenip kontrol edilmeli, 'ArayuzListesi'nde sadece farklı değerler depolanmalı.
Yine aynı değer, aynı fonksiyon çağrısıyla (büyük ihtimalle fonksiyon işaretcileri yer değiştirmiyor, ama parametreler değişiyor) eklenmeye çalışılındığında o fonksiyon 'Eglem'leri kontrol etmelidir.
3. Pek bir fikrim yok, belki fonksiyon işaretcileri alan arayüz elemanları değiştirilebilir veya sabit boyutlu bir dizide depolanıp 'ArayuzCiz()' gibi bir fonksiyona geçirilebilir?

**Yani ana sorun şu şekilde:** Aynı fonksiyonun aynı konumda aynı (ama değişebilen) parametrelerle çağırılmasını kontrol etmek.

`arayuz.cpp > YaziAlani() >` Yazı alanı fonksiyonu hatalı utf8 yazısı alırsa programda sorun çıkabilir.
* Çözümler :
1. UTF8Dogrula() diye bir fonksiyon yapmak. Hatalı dönerse yazı **ascii** gibiymiş davran.
