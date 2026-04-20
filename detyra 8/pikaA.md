# Bllokimi i Aksesit në një Website me Firewall dhe Testimi i Bypass me VPN

**Qëllimi:** Bllokimi i aksesit në një website/platformë të zgjedhur duke përdorur firewall-in, verifikimi i bllokimit, testimi nëse një VPN extension në browser dhe një aplikacion VPN mund ta anashkalojnë bllokimin, dhe më pas rikthimi i aksesit.

---

## 1. Qëllimi i Detyrës

Qëllimi ishte të bllokohej aksesi në një website/platformë duke përdorur kontrolle të rrjetit përmes firewall-it, të testohej aksesi normal dhe përmes metodave VPN, dhe më pas të hiqej bllokimi.

Për këtë ushtrim, zgjodha **thepiratebay.org** si website target, sepse përdoret shpesh si shembull në bllokime firewall dhe lidhet me akses torrent. Të dhënat DNS/IP të këtij site tregojnë A records aktive si `162.159.136.6` dhe `162.159.137.6`, që do të thotë se bllokimi me IP është i mundur në nivel hosti, ndërsa bllokimi me domain nuk trajtohet drejtpërdrejt nga `firewalld`.

---

## 2. Shënim i Rëndësishëm Para Fillimit

Një firewall në nivel hosti si `firewalld` punon me **porta, adresa dhe rich rules**, jo me emra websitesh si një DNS filter ose proxy.

Kjo do të thotë që:
- Nuk mund të bllokosh direkt një domain.
- Duhet të përdorësh adresën IP të zgjidhur.

Në praktikë, përdora adresën IP të site-it dhe më pas verifikova rezultatin në browser dhe me teste VPN.

---

## 3. Verifikimi i Firewall-it

Së pari kontrollova nëse `firewalld` ishte aktiv:

```bash
systemctl status firewalld
sudo firewall-cmd --state
```

Rezultati:
- `firewalld.service` është `active (running)`.
- `firewall-cmd --state` është `running`.

---

## 4. Identifikimi i Adresës IP të Synuar

Meqenëse `firewalld` bllokon trafikun IP dhe jo emrat e domain-eve, rezolvova faqen e synuar në një adresë IP.

```bash
nslookup thepiratebay.org
```

Rezultati:
- Rezultatet e DNS publik tregojnë adresa si `162.159.136.6` dhe `162.159.137.6` për `thepiratebay.org`.

---

## 5. Bllokimi i Faqes

Shtova një rregull (rich rule) për të refuzuar trafikun drejt adresës IP të synuar në zonën aktive.

```bash
sudo firewall-cmd --permanent --add-rich-rule='rule family="ipv4" destination address="162.159.136.6" reject'
sudo firewall-cmd --reload
```

Rezultati:
- Komanda ktheu `success`.
- `firewalld` ringarkohet pa gabime.

---

## 6. Testimi i Aksesit pa VPN

Pas aplikimit të bllokimit, testova faqen në Firefox me lidhjen normale.

Metoda e testimit:
1. Hap Firefox.
2. Shko te `https://thepiratebay.org`.
3. Rifresko faqen ose provo një test të drejtpërdrejtë të bazuar në IP nëse nevojitet.

Rezultati:
- Faqja dështoi të ngarkohet.

Ky është një kufizim i rëndësishëm: bllokimi i një adrese IP mund të mos bllokojë çdo rrugë të përdorur nga domain-i, veçanërisht nëse faqja përdor CDN ose shumë rekorde A.

---

## 7. Testimi me VPN të Shfletuesit

Pastaj testova VPN të shfletuesit. Proton VPN është i disponueshëm për përdoruesit falas në Firefox.

Metoda e testimit:
1. Instalo Proton VPN në Firefox.
2. Hyr me një llogari falas Proton.
3. Lidhu me një server falas.
4. Provo përsëri të hapësh `https://thepiratebay.org`.

Rezultati i pritshëm:
- Extensioni i shfletuesit ndryshon vetëm trafikun e shfletuesit.

---

## 8. Rivendosja e Aksesit

Pas testimit, hoqa rregullën e bllokimit për të rivendosur aksesin normal.

```bash
sudo firewall-cmd --permanent --remove-rich-rule='rule family="ipv4" destination address="162.159.136.6" reject'
sudo firewall-cmd --reload
```

Rezultati:
- Komanda kthen `success`.
- Faqja ngarkohet përsëri normalisht pa VPN.

---

## 9. Përfundim

Kufizimi kryesor është se `firewalld` nuk është filtër domain-esh, prandaj një faqe që ndryshon adresat IP ose përdor adresa të shumta mund të kërkojë rregulla shtesë për ta mbajtur të bllokuar.