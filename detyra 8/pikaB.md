# Regjistrimi i Kredencialeve me Wireshark
  
**Objektivi:** Kapja e kredencialeve të hyrjes të transmetuara përmes rrjetit duke përdorur Wireshark, fillimisht në një faqe hyrjeje HTTP të pasigurt dhe pastaj në një faqe hyrjeje HTTPS të sigurt, dhe krahasimi i rezultateve.

---

## 1. Objektivi

Detyra ka si qëllim demonstrimin praktik të rrezikut që paraqet transmetimi i kredencialeve të hyrjes (username/password) nëpërmjet protokollit HTTP të pakriptuar, duke e krahasuar me skenarin e sigurt HTTPS. Mjeti i analizës së trafikut të rrjetit i përdorur është **Wireshark**.

Kërkesa e detyrës specifikon:
- Aplikimin e modulit *log in* ndaj një website-i jo të sigurtë (HTTP)
- Tentativën e njëjtë ndaj një platforme të sigurtë (HTTPS)
- Krahasimin e rezultateve të kapura

---

## 2. Mjedisi i Testimit

### 2.1 Konfigurimi i Sistemit

| Komponent | Detaje |
|-----------|--------|
| Sistemi operativ | Fedora Linux 42 (x86_64) |
| Wireshark | v4.6.4 |
| Platforma HTTP (jo e sigurtë) | DVWA — Docker image `vulnerables/web-dvwa` |
| URL e aksesit HTTP | `http://localhost/login.php` |
| Platforma HTTPS (e sigurtë) | Faqe publike me TLS aktiv |
| Interface HTTP (kapja) | `lo` — Loopback |
| Interface HTTPS (kapja) | `wlp1s0` — Wi-Fi |

### 2.2 Konfigurimi i Wireshark (dumpcap)

Para se të fillohej kapja, Wireshark u konfigurua për të lejuar kapjen e paketave pa priviliegje root:

```bash
sudo dnf install wireshark
sudo usermod -a -G wireshark $USER
sudo chgrp wireshark /usr/sbin/dumpcap
sudo chmod 750 /usr/sbin/dumpcap
sudo setcap cap_net_raw,cap_net_admin=eip /usr/sbin/dumpcap
```

Verifikimi i suksesit:
```bash
groups boboci
# Output: boboci wheel docker wireshark

sudo getcap /usr/sbin/dumpcap
# Output: /usr/sbin/dumpcap cap_net_admin,cap_net_raw=eip

ls -l /usr/sbin/dumpcap
# Output: -rwxr-x---. 1 root wireshark 161528 Mar 4 01:00 /usr/sbin/dumpcap
```

---

## 3. Skenari 1 — HTTP (Jo i Sigurtë): Kapja e Kredencialeve Plaintext

### 3.1 Platforma

**DVWA (Damn Vulnerable Web Application)** është një aplikacion web i projektuar qëllimisht me dobësi sigurie, i përdorur gjerësisht për qëllime edukative dhe testimi. Ai ofron një modul login të plotë të aksesueshëm nëpërmjet HTTP të pakriptuar.

DVWA u nisë me Docker:
```bash
docker run -d -p 80:80 vulnerables/web-dvwa
```

Faqja e login-it u aksesua nga shfletuesi në adresën `http://localhost/login.php`.

### 3.2 Konfigurimi i Wireshark

- **Interface:** `lo` (Loopback) — trafiku lokal `127.0.0.1 → 127.0.0.1` kalon vetëm nëpër këtë interface
- **Filtri i aplikuar:** `http.request.method == "POST"`

Filtri `http.request.method == "POST"` mundëson shfaqjen vetëm të kërkesave POST, që janë ato që bartin të dhënat e formularit të login-it.

### 3.3 Procesi i Login

Në faqen `http://localhost/login.php` u futën kredencialet:

| Fushë | Vlera |
|-------|-------|
| Username | `admin` |
| Password | `password` |

Pas klikimit të butonit **Login**, Wireshark kapi menjëherë paketën POST.

### 3.4 Rezultati — Paketa e Kapur

![Wireshark — Paketa HTTP POST e kapur në interface lo]

Wireshark tregoi paketën:
- **No.:** 8
- **Source:** `127.0.0.1`
- **Destination:** `127.0.0.1`
- **Protocol:** HTTP
- **Length:** 796 bytes
- **Info:** `POST /login.php HTTP/1.1`

Shtresa **HTML Form URL Encoded** është e dukshme në Packet Details, që konfirmon praninë e të dhënave të formularit.

### 3.5 TCP Stream — Kredencialet Plaintext

Duke klikuar me të djathtën mbi paketën → **Follow → TCP Stream**, u shfaq i gjithë bisedi HTTP:

```
Content-Type: application/x-www-form-urlencoded
Content-Length: 88
Origin: http://localhost
Connection: keep-alive
Referer: http://localhost/login.php
Cookie: PHPSESSID=lt4nioqp2ib5svpuuiae764bb0; security=low

username=admin&password=password&Login=Login&user_token=59b17d1b257ec335545d23e9f34c3734

HTTP/1.1 302 Found
Date: Wed, 22 Apr 2026 19:12:53 GMT
Server: Apache/2.4.25 (Debian)
Location: index.php
```

**Kredencialet janë plotësisht të dukshme në tekst të qartë (plaintext):**
- `username=admin`
- `password=password`

Gjithashtu është i dukshëm dhe `user_token` CSRF, session cookie (`PHPSESSID`), si dhe serveri i aplikacionit (`Apache/2.4.25`).

### 3.6 Vlerësimi i Rrezikut

Çdo sulmues që ka akses në rrjetin e njëjtë (ose si ndërmjetës — MITM) mund të:
1. Kapë kredencialet e login-it në kohë reale
2. Ripërdorë session cookie-n për rrëmbim sesioni (session hijacking)
3. Identifikojë infrastrukturën e serverit (version Apache, PHP session)

---

## 4. Skenari 2 — HTTPS (I Sigurtë): Trafiku i Enkriptuar

### 4.1 Konfigurimi i Wireshark

- **Interface:** `wlp1s0` (Wi-Fi) — trafiku drejt internetit
- **Filtri i aplikuar:** `tls`

### 4.2 Tentativa me Filtrin POST

Fillimisht u aplikua i njëjti filtër `http.request.method == "POST"` gjatë aksesimit të një faqeje HTTPS.

**Rezultati:** 0 paketa të shfaqura (Displayed: 0 — 0.0%)

Kjo ndodhi sepse trafiku HTTPS nuk kalon si HTTP i qartë — enkriptimi TLS ndodh para transmetimit, kështu që Wireshark nuk mund ta njohë si HTTP.

### 4.3 Kapja me Filtrin TLS

Me filtrin `tls`, Wireshark kapi paketat e enkriptuara:

**Paketat e kapura tregojnë:**
- **Protocol:** TLSv1.2 (dhe TLSv1.3 në disa raste)
- **Info:** `Application Data` (të enkriptuara)
- **Source/Destination:** IP-të reale (`104.18.27.48`, `192.168.100.93`, `3.233.158.25`)

### 4.4 Analiza e Hex Dump

Hex dump-i i paketave TLS tregon bytes të rastësishëm, plotësisht të palexueshëm:

```
0000  b2 5e f4 fe da 01 c8 a7  76 4b 6b 89 ...
0010  00 64 2f 30 40 00 29 06  fe ae 8c 52 ...
0020  4a 5d 01 bb 9d 5e af 12  ee ac 60 f3 ...
```

Asnjë element i lexueshëm si `username=`, `password=` apo të dhëna të tjera nuk është i dalluar.

### 4.5 Rezultati Final HTTPS

Kur u aksesua faqja HTTPS dhe u krye login, Wireshark me filtrin `http.request.method == "POST"` nuk shfaqi **asnjë paketë** — trafiku HTTP nuk ekziston sepse gjithçka është e mbështjellë në TLS.

---

## 5. Krahasimi i Rezultateve

| Karakteristika | HTTP — DVWA | HTTPS — Faqe e sigurtë |
|---|---|---|
| Protokolli | HTTP (port 80) | HTTPS/TLS (port 443) |
| Interface i kapur | `lo` (Loopback) | `wlp1s0` (Wi-Fi) |
| Filtri efektiv | `http.request.method == "POST"` | `tls` |
| Kredencialet | **Plaintext të dukshme** | Të enkriptuara — të palexueshme |
| Paketa POST e dukshme |  Po — `username=admin&password=password` |  Jo — asnjë paketë HTTP |
| Session Cookie | E dukshme (`PHPSESSID=...`) | E enkriptuar |
| Informacione serveri | E dukshme (`Apache/2.4.25`) | E fshehur |
| Rreziku MITM |  Shumë i lartë |  Minimal |
| Versioni TLS | — | TLSv1.2 |

---

## 6. Konkluzionet

Eksperimenti demonstroi qartë ndryshimin kritik ndërmjet transmetimit HTTP dhe HTTPS:

1. **HTTP është i pasigurtë për të dhëna sensitive.** Kredencialet `username=admin&password=password` u kapën plotësisht në plaintext me një filtër të thjeshtë Wireshark. Çdo person me akses në rrjet mund t'i lexojë.

2. **HTTPS me TLS e mbron efektivisht trafikun.** Asnjë informacion i lexueshëm nuk ishte i aksesueshëm — Wireshark kapi vetëm `TLSv1.2 Application Data` e enkriptuar, pa asnjë mundësi deshifrimi pa çelësin privat të serverit.

3. **Sasia e informacionit të ekspozuar nëpërmjet HTTP është e madhe.** Përveç kredencialeve, u ekspozuan: session cookies (të shfrytëzueshme për session hijacking), versioni i serverit web (Apache/2.4.25), teknologjia e backend-it (PHP) dhe struktura e URL-ve.

4. **Rregulli themelor i sigurisë:** Çdo aplikacion web që kërkon autentifikim duhet të përdorë HTTPS. Protokolli HTTP duhet konsideruar i pasigurtë dhe i papërshtatshëm për çdo transmetim të dhënash sensitive.

---

## 7. Referenca

- DVWA — Damn Vulnerable Web Application: https://github.com/digininja/DVWA
- Wireshark Documentation: https://www.wireshark.org/docs/
- OWASP — Transport Layer Protection Cheat Sheet: https://cheatsheetseries.owasp.org/cheatsheets/Transport_Layer_Security_Cheat_Sheet.html
- RFC 8446 — The Transport Layer Security (TLS) Protocol Version 1.3
