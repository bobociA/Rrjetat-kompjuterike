# Regjistrimi i Kredencialeve me Wireshark
  
**Objektivi:** Kapja e kredencialeve të hyrjes të transmetuara përmes rrjetit duke përdorur Wireshark, fillimisht në një faqe hyrjeje HTTP të pasigurt dhe pastaj në një faqe hyrjeje HTTPS të sigurt, dhe krahasimi i rezultateve.

---


## 2. Instalimi i Wireshark në Fedora


```bash
sudo dnf install wireshark

sudo usermod -a -G wireshark boboci

sudo chgrp wireshark /usr/sbin/dumpcap
sudo chmod 750 /usr/sbin/dumpcap
sudo setcap cap_net_raw,cap_net_admin=eip /usr/sbin/dumpcap

# Logout / login
```

Pas ri-hyrjes,ekzekutova `wireshark` në terminal.

---

## 3. Konfigurimi me Dy Pajisje

- **Laptop:** Pajisja ku është instaluar Wireshark dhe ku kryhet kapja e paketave.
- **Smartphone:** Pajisja nga e cila u kryen hyrjet në faqet e testimit.

Të dyja pajisjet ishin të lidhura në të njëjtin rrjet Wi-Fi shtëpiak/lokal.

ip a

wlp1s0 inet 192.168.100.93/24 

Smartphone-i kishte adresën,`192.168.100.90` 

## 4. Kapja e Kredencialeve 

Për faqen HTTP të testimit, përdora **`http://testphp.vulnweb.com/login.php`** — një aplikacion web i cenueshëm i dedikuar qëllimisht për praktikë edukative dhe testim sigurie. Kjo faqe komunikon ekskluzivisht mbi HTTP dhe pranon hyrje pa enkriptim.

Kredencialet e testimit të njohura publike për këtë sit janë:
- Emri i përdoruesit: `boboci`
- Fjalëkalimi: `bobi123`

### Kapja në Wireshark

1. Hapa Wireshark në laptop.
2. Zgjodha ndërfaqen e rrjetit Wi-Fi (`wlp1s0`) nga lista e ndërfaqeve të disponueshme.
3. Klikova butonin për të filluar kapjen.


Pasi nisa kapjen, shkrova filtrin e mëposhtëm në shiritin e filtrit të shfaqjes:

```text
http.request.method == "POST"
```

Ky filtër tregon vetëm kërkesat HTTP POST, të cilat zakonisht mbajnë të dhënat e dërguara nga formularët e hyrjes.



Nga smartphone-i im, hapa shfletuesin dhe navigova në:
http://testphp.vulnweb.com/login.php 

Shfletuesi tregoi ikonën e bllokimit me vijë të kuqe, duke konfirmuar që faqja komunikonte mbi HTTP të pa sigurt.

Shkrova:
- **Username:** `boboci`
- **Password:** `bobi123`

Klikova butonin **Login**.

Menjëherë pashë një paketë të re të shfaqur në Wireshark me informacionin:

POST /login.php HTTP/1.1


Klikova mbi paketën dhe në panelin **Packet Details** zgjerova seksionin:


▸ Hypertext Transfer Protocol
▸ HTML Form URL Encoded: application/x-www-form-urlencoded
uname: boboci
pass: bob123



Kredencialet `uname=boboci` dhe `pass=bobi123` ishin plotësisht të dukshëm në tekst të pastër.

### Ndoqa rrjedhën TCP

Klikova me të djathtën mbi paketën → **Follow → TCP Stream** dhe pashë të gjithë bisedën e HTTP riprodhuar si tekst të qartë, duke përfshirë të gjithë formën e dërguar:
 POST /login.php HTTP/1.1
Host: testphp.vulnweb.com
...
uname=boboci&pass=bobi123&submit=Login


**Rezultati:**
- Emri i përdoruesit dhe fjalëkalimi u shfaqën plotësisht.
- HTTP nuk enkriptoi asnjë gjë nga të dhënat e dërguara.
- Çdokush në të njëjtin rrjet mund t'i kishte lexuar po aq lehtë.

---

## 5. Kapja e Kredencialeve në Faqe HTTPS (e Sigurt)

### Faqja e zgjedhur

Për testin HTTPS, zgjodha **`https://github.com/login`** — një faqe e njohur dhe e sigurt me enkriptim TLS aktiv.

### 5.2 Rifillova kapjen në Wireshark

Ndalova kapjen e mëparshme dhe fillova një të re të pastër në të njëjtën ndërfaqe `wlp1s0`. Aplikova të njëjtin filtër:

```text
http.request.method == "POST"
```

### 5.3 Kreva hyrjen nga Smartphone

Nga smartphone-i, navigova në `https://github.com/login` dhe shkrova kredenciale dhe klikova **Sign In**.


Pas dërgimit të formularit, asnjë paketë e dukshme POST nuk u shfaq nën filtrin `http.request.method == "POST"`.

Hoqa filtrin dhe aplikova:

```text
tls
```

Pashë vetëm rekorde TLS të enkriptuara si:
TLSv1.3 Application Data
TLSv1.3 Application Data


Kur klikova mbi këto paketa dhe u përpoqa të zgjeroja detajet, nuk pashë asnjë fushë të lexueshme — vetëm të dhëna binare të enkriptuara pa kuptim.

**Rezultati**
- Asnjë fjalëkalim ose emër përdoruesi nuk ishte i dukshëm.
- TLS enkriptoi të gjithë ngarkesën HTTP; Wireshark tregoi vetëm rekorde të enkriptuara të aplikacionit.
- Pa çelësin privat të serverit ose skedarit `SSLKEYLOGFILE`, deshifrimi nuk është i mundur.

---

## 6 Tabela e Krahasimit

| Prova | Protokolli | Çfarë pa Wireshark | Kredencialet të Dukshme? | Arsyeja |
|---|---|---|---|---|
| `testphp.vulnweb.com/login.php` | HTTP | POST me `uname=test&pass=test` në tekst të pastër | **Po** | HTTP nuk enkipton trafikun; ngarkesa është e lexueshme direkt. |
| `github.com/login` | HTTPS / TLS | Rekorde TLS të enkriptuara; asnjë fushë formularësh | **Jo** | TLS enkipton ngarkesën HTTP; pa çelësin e deshifrimit, përmbajtja është e pakuptueshme. |

---

## 8. Konkluzioni

- Në faqen HTTP `testphp.vulnweb.com`, Wireshark kapi menjëherë kredencialet `uname=test` dhe `pass=test` në tekst të pastër brenda kërkesës POST. Kjo konfirmon se çdokush në të njëjtin rrjet mund të kapë lehtësisht fjalëkalimet e transmetuara mbi HTTP.
- Në faqen HTTPS `github.com`, e njëjta metodë kapjeje nuk prodhoi asnjë të dhënë të lexueshme. Wireshark tregoi vetëm rekorde TLS të enkriptuara, duke konfirmuar se TLS mbron trafikun në mënyrë efektive.
- Funksioni opsional GeoIP lejoi vizualizimin gjeografik të pikave të komunikimit rrjetor mbi hartë, duke ofruar kontekst shtesë analitik mbi origjinën dhe destinacionin e paketave.