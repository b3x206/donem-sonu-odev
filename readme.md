# 2ElSatis
* Dönem sonu projesi.

Yerel olarak sahibinden/2.el satış websitesi gibi bir sistem, kullanıcı satmak istediği ürünü girip olan ürünlerini (ve diğer kullanıcı ürünlerini) konsol arayüzünde görüntüleyebilecek.

# A Used-Goods Website Simulation
* 1st year ending project for the computer engineering uni that I'm failing

An second-hand selling website-like system, where the user can input the data about their thing that they want to sell and display their things in the console.

> All (*most, I didn't bother much with the utf8 parsing code and winapi code was with the help of microsoft docs) code is written by me.

## Why it's Turkish?
To prove that I have written it and in such cases where the teacher could want the source code, nobody except 1 person wanted the source (and he couldn't probably decypher it, but let's just not do assumptions, perhaps he could be way better than reading bad code compared to me).

## But why it's public?
Some people were way too impressed with it (this isn't anything much, it's just old console related winapi misuse + C and C++ programming footguns \[i know those are pretty different programming languages, i felt constrainted in features but towards the last parts i just went overboard with spooky C++ features\]) so I am making it public so that anybody can look and use the code in it.. You really shouldn't use it for anything though (imo, maybe you can find something valuable in this repository, yes, the commits are cringe so i have removed them.. sorry, \[and perhaps i could have said potty words on some commits, i wish i shouldn't have to do that but whatever.\])

## So, what libraries does it use
* the c++ stl, c stl and the windows.h header.

yes, the data is stored in a custom (horrible, i was lazy ok) file format that, well, mixes utf8 with malformed (meant to be ascii-only) utf8. But it worked so whatever.

## Will I fix this?
* Yes (on a different branch), however I do have to audit the source code and unfortunately, delete the commit history.