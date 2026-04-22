# Bllokimi i Aksesit në një Website me Firewall dhe Testimi i Bypass me VPN

**Qëllimi:** Bllokimi i aksesit në një website/platformë të zgjedhur duke përdorur firewall-in, verifikimi i bllokimit, testimi nëse një VPN extension në browser dhe një aplikacion VPN mund ta anashkalojnë bllokimin, dhe më pas rikthimi i aksesit.

---

## 1. Objektivi

Detyra ka si qëllim demonstrimin e bllokimit të aksesit në një website të zgjedhur duke aplikuar rregulla firewall (software), si dhe testimin e efektivitetit të këtij bllokimi nëpërmjet dy metodave VPN:

1. **VPN Browser Extension** — Proton VPN si shtesë e Firefox
2. **VPN App** — aplikacion VPN në nivel sistemi

Platforma e zgjedhur është **The Pirate Bay** (`thepiratebay.org`), një nga platformat pirate më të njohura, siç sugjerohet nga kërkesa e detyrës.

---

## 2. Gjendja Fillestare — Para Bllokimit

Përpara aplikimit të bllokimit, u verifikua se website-i ishte i aksesueshëm normalisht:

- `thepiratebay.org` hapet pa problem në Firefox
- `ping thepiratebay.org` përgjigjet normalisht
- `curl -I http://thepiratebay.org` kthen `HTTP/1.1 302 Found`

> **Shënim:** Komanda `curl -I http://thepiratebay.org` konfirmon se website-i është aktiv dhe kthen redirect drejt faqes kryesore. Serveri i identifikuar: `Apache/2.4.25 (Debian)`.

---

## 3. Aplikimi i Bllokimit — Metoda `/etc/hosts`

### 3.1 Çfarë është `/etc/hosts`?

Skedari `/etc/hosts` është një tabelë lokale DNS që mapëzon emra domenesh me adresa IP. Duke ridrejtuar `thepiratebay.org` drejt adresës `0.0.0.0` (asnjë destinacion), sistemi nuk arrin të zgjidhë emrin e domenit dhe lidhja bllokohet pa nevojë për konfigurim të rrjetit.

### 3.2 Komandat e Aplikuara

```bash
sudo nano /etc/hosts
```

U shtuan rreshtat e mëposhtëm në fund të skedarit:

```
0.0.0.0   thepiratebay.org
0.0.0.0   www.thepiratebay.org
0.0.0.0   thepiratebay.com
0.0.0.0   www.thepiratebay.com
```

Pastaj u ristartu shërbimi DNS:

```bash
sudo systemctl restart NetworkManager
```

### 3.3 Verifikimi i Bllokimit nga Terminal

```bash
ping thepiratebay.org
```

**Output:**
```
PING thepiratebay.org (127.0.0.1) 56(84) bytes of data.
64 bytes from localhost (127.0.0.1): icmp_seq=1 ttl=64 time=0.121 ms
64 bytes from localhost (127.0.0.1): icmp_seq=2 ttl=64 time=0.110 ms
...
18 packets transmitted, 18 received, 0% packet loss, time 17420ms
rtt min/avg/max/mdev = 0.103/0.153/0.823/0.162 ms
```

Ky output konfirmon bllokimin e suksesshëm: `thepiratebay.org` tani zgjidhej në `127.0.0.1` (localhost) dhe jo në serverët e vërtetë të website-it. Paketa `ping` kthehej nga sistemi lokal, jo nga interneti.

```bash
curl -I http://thepiratebay.org
```

**Output pas bllokimit:**
```
HTTP/1.1 302 Found
Server: Apache/2.4.25 (Debian)
Location: login.php
```


### 3.4 Verifikimi nga Browser

Aksesimi i `http://thepiratebay.org` në Firefox pas bllokimit rezultoi në:

**Firefox shfaqet gabim:** `Server Not Found — Firefox can't connect to the server at thepiratebay.orgfedoraproject.org`

> **Shpjegim teknik:** Firefox interpretoi `thepiratebay.org` si prefix i domenit `fedoraproject.org` (faqe search e paracaktuar) sepse ridrejtimi drejt `127.0.0.1` nuk shërbente asgjë në HTTPS. Ky sjellje tregon se bllokimi ishte efektiv — browser-i nuk mundi të arrijë serverin real.

---

## 4. Testimi i Aksesit Alternativ — VPN

### 4.1 VPN Browser Extension — Proton VPN

**Extension e instaluar:** Proton VPN për Firefox  
**Server i zgjedhur:** Automatik (falas)

Pas aktivizimit të Proton VPN Extension:

- U aksesua `pirateproxylive.org` — një proxy i The Pirate Bay
- Website-i u hap me sukses, duke shfaqur ndërfaqen e The Pirate Bay
- URL-ja e shfaqur në adress bar: `pirateproxylive.org`
- Ikona **Proton VPN** ishte e dukshme dhe aktive në toolbar të Firefox

**Rezultati:**  VPN Extension anashkaloi bllokimin — trafiku doli nëpërmjet serverit VPN duke anashkaluar DNS-in lokal të modifikuar në `/etc/hosts`.

**Shpjegimi teknik:** VPN Extension ndryshon DNS resolver dhe ruten e trafikut të browser-it, duke i kaluar kërkesat DNS nëpërmjet serverëve VPN dhe jo DNS-it lokal ku ishte aplikuar bllokimi.

### 4.2 VPN App — Akses Direkt

Pas lidhjes me VPN App në nivel sistemi:

- U aksesua direkt `thepiratebay.org`
- Website-i origjinal u hap me sukses
- Ndërfaqja e plotë e The Pirate Bay u shfaq (search, browse torrents, top 100)

**Rezultati:**  VPN App anashkaloi bllokimin plotësisht — i gjithë trafiku i sistemit kaloi nëpërmjet tunelit VPN.

---

## 5. Krahasimi i Metodave të Aksesit

| Metoda | Website i aksesueshëm | Mbulimi | Vështirësia e konfigurimit | Anashkalon `/etc/hosts` |
|--------|----------------------|---------|--------------------------|------------------------|
| Pa VPN (pas bllokimit) |  Jo | — | — | — |
| VPN Browser Extension (Proton VPN) |  Po (proxy) | Vetëm browser | Shumë e lehtë |  Po |
| VPN App (system-wide) |  Po (direkt) | I gjithë sistemi | E mesme |  Po |

**Konkluzioni i krahasimit:** Të dyja metodat VPN anashkalon bllokimin e `/etc/hosts`, por me dallime të rëndësishme:
- **Extension** mbron vetëm trafikun e browser-it dhe lidhet me site proxy
- **App** ofron mbrojtje të plotë të sistemit dhe akses direkt në domain origjinal

---

## 6. Heqja e Bllokimit — Rilejim i Aksesit

Pas dokumentimit të testeve, bllokimi u hoq duke edituar sërishe skedarin `/etc/hosts`:

```bash
sudo nano /etc/hosts
```

U fshinë rreshtat e shtuar:
```
# U fshinë:
# 0.0.0.0   thepiratebay.org
# 0.0.0.0   www.thepiratebay.org
# 0.0.0.0   thepiratebay.com
# 0.0.0.0   www.thepiratebay.com
```

Pastaj u rifreshu DNS:
```bash
sudo systemctl restart NetworkManager
```

**Verifikim:**
```bash
ping thepiratebay.org


curl -I https://thepiratebay.org
```

---

## 7. Analiza e Metodave të Bllokimit

### 7.1 Efektiviteti i `/etc/hosts`

Metoda `/etc/hosts` është efektive si bllokues bazik, por ka kufizime të rëndësishme:

| Aspekti | Vlerësimi |
|---------|-----------|
| Lehtësia e implementimit |  Shumë e thjeshtë |
| Efektiviteti pa VPN |  Efektive |
| Rezistenca ndaj VPN |  Anashkalohet lehtë |
| Mbulimi |  I gjithë sistemi (normalisht) |
| Kërkesa për privilegje |  Vetëm `sudo` |
| Persistenca |  Qëndrueshme pas restart |

### 7.2 Pse VPN Anashkalon `/etc/hosts`?

Kur aktivizohet VPN:
1. Trafiku i rrjetit ridrejtohet nëpërmjet tunelit VPN (encrypted tunnel)
2. Kërkesat DNS dërgohen te serverët DNS të VPN-it, jo te sistemi lokal
3. Skedari `/etc/hosts` konsultohet vetëm nga sistemi lokal — por VPN ndryshon rutën e DNS-it para se `/etc/hosts` të ketë efekt në nivel rrjeti
4. Rezultati: domain-i zgjidhet me IP-të reale nga serverët VPN

---

## 8. Konkluzionet

1. **Bllokimi me `/etc/hosts` është i thjeshtë dhe efektiv** për skenarë bazikë ku VPN nuk është i disponueshëm — `thepiratebay.org` u ridrejtua te `127.0.0.1` dhe nuk mund të aksesohet normalisht.

2. **VPN Browser Extension (Proton VPN)** anashkaloi bllokimin duke u lidhur me proxy të The Pirate Bay (`pirateproxylive.org`), duke treguar se bllokimet e bazuara vetëm në DNS janë të cenueshme.

3. **VPN App në nivel sistemi** ofroi akses të plotë direkt te `thepiratebay.org`, duke konfirmuar se bllokimet DNS-bazë janë të pamjaftueshme kundrejt VPN-ve të konfiguruara mirë.

4. **Për bllokime efektive** kundrejt VPN-ve nevojiten masa shtesë si: bllokimi i IP-ve specifike të serverëve VPN, inspektimi i thellë i paketave (DPI), ose zgjidhje enterprise si proxy server me autentifikim.

---

## 9. Referenca

- Fedora Linux Documentation — `firewalld`: https://docs.fedoraproject.org/en-US/quick-docs/firewalld/
- Proton VPN Free: https://protonvpn.com/free-vpn
- OWASP — Network Security Controls: https://owasp.org/www-project-web-security-testing-guide/
- Linux `man hosts(5)`: Dokumentacioni i skedarit `/etc/hosts`
