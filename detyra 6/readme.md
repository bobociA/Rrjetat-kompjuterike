# Dokumentim i detyrës:Analiza e cënueshmërisë 



## Qëllimi i projektit
Ky raport paraqet një vlerësim të sigurisë së aplikacionit web që krahas on një target HTTP dhe një target HTTPS duke përdorur pesë mjete të automatizuara dhe pesë platforma online. Qëllimi ishte të nxjerrë në pah dallimet në sipërfaqen e sulmit dhe të demonstrojë se HTTPS siguron mbrojtje në nivel transporti, por nuk adreson vulnerabilitetet në nivel aplikacioni.

Të dy target-et treguan probleme serioze si injeksion SQL, cross-site scripting (XSS) dhe kontroll i thyer i aksesit. Target-i HTTP kishte një sipërfaqe sulmi më të madhe për shkak të mungesës së enkriptimit, duke bërë që kredencialet dhe të dhënat e sesionit të kapen lehtësisht. Target-i HTTPS ofroi mbrojtje më të mirë për të dhënat në transit, por vuante ende nga defekte të aplikacionit dhe konfigurime të gabuara TLS.

### Përmbledhje e Gjetjeve Kryesore

| Kategoria                        | Target HTTP | Target HTTPS |
|----------------------------------|-------------|--------------|
| Vulnerabilitete Kritike          | 9           | 5            |
| Vulnerabilitete të Larta         | 7           | 6            |
| Vulnerabilitete Mesatare         | 11          | 8            |
| Të Ulëta / Informative           | 14          | 12           |
| Siguria e Transportit            | Asnjë       | TLS 1.2/1.3  |
| Injeksion SQL                    | U zbulua    | U zbulua     |
| XSS                              | U zbulua    | U zbulua     |
| Ekspozim i të Dhënave Sensitive  | Kritik      | Mesatar      |

---

## Fusha dhe Metodologjia

Vlerësimi ndoqi Udhëzuesin e Testimit OWASP dhe përdori një qasje black-box/grey-box në një mjedis laboratorik të kontrolluar. Nuk u krye asnjë shfrytëzim përtej provës së konceptit dhe asnjë test denial-of-service.

**Fazat:**
- Rekognicion pasiv duke përdorur platforma online
- Skanim aktiv me mjete të automatizuara
- Analizë e vulnerabiliteteve dhe dokumentim
- Rekomandime për remedim

---

## Zgjedhja e Target-eve

**Target HTTP:** `http://testphp.vulnweb.com`  
- Simulim i-commerce PHP/MySQL vulnerabël me qëllim (site test Acunetix)  
- Funksionon me PHP dhe nginx të vjetruar

**Target HTTPS:** `https://demo.testfire.net`  
- Simulim banking online vulnerabël me qëllim (IBM Altoro Mutual)  
- Funksionon me Java/Tomcat me TLS

Të dy aplikacionet mirëmbahen publikisht për testim të mjeteve të sigurisë dhe edukim.

---

## Përmbledhje e Mjeteve të Automatizuara

Pesë mjete u përdorën për mbulim komplementar:

| Mjet          | Kategoria             | Përdorimi Kryesor                     |
|---------------|-----------------------|---------------------------------------|
| OWASP ZAP     | Skaner DAST           | Skanim aktiv dhe pasiv web            |
| Burp Suite    | Proxy/DAST            | Interceptim trafiku dhe testime       |
| Nikto         | Skaner Web            | Konfigurime të gabuara të serverit    |
| Nmap          | Skaner Rrjeti         | Enumerim portash dhe skripte          |
| SQLMap        | Injeksion SQL         | Testim i automatizuar i injeksionit   |

---

## Përmbledhje e Platformave Online

Pesë platforma pasive rekognicioni u përdorën:

| Platforma            | Qëllimi                               |
|----------------------|---------------------------------------|
| Shodan               | Rekognicion në shkallë interneti      |
| SecurityHeaders.com  | Analizë e header-ëve të sigurisë HTTP |
| SSL Labs             | Vlerësim i konfigurimit TLS/SSL       |
| Mozilla Observatory  | Vlerësim i përgjithshëm i sigurisë web|
| VirusTotal           | Reputacion i URL-së dhe domenit       |

---

## Rezultatet e Mjeteve: Target HTTP (`http://testphp.vulnweb.com`)

Skanimet zbuluan disa probleme kritike, duke përfshirë injeksion SQL, XSS të reflektuar, listim drejtorish dhe kredenciale të transmetuara në plaintext. Nikto dhe Nmap identifikuan PHP 5.6 të vjetruar (end-of-life), portin MySQL 3306 të ekspozuar dhe konfigurime të gabuara të serverit. SQLMap konfirmoi disa lloje injeksioni SQL, duke lejuar enumerim të bazës së të dhënave.

Burp Suite nxori në pah menaxhim të pasigurt të sesionit (cookie sesioni përmbante emër përdoruesi/fjalëkalim) dhe tregues path traversal.

---

## Rezultatet e Mjeteve: Target HTTPS (`https://demo.testfire.net`)

Skanimet treguan injeksion SQL (duke përfshirë anashkalim login), XSS të ruajtur dhe autentikim të thyer. Nikto vuri në dukje mungesën e HSTS, përmbajtje të përzier dhe një lidhës të vjetër Tomcat. Nmap zbuloi një certifikatë TLS të skaduar, mbështetje për shifra të dobëta (përfshirë RC4) dhe TLS 1.0 të vjetruar.

SQLMap konfirmoi injeksion në parametrat e login-it kundrejt Microsoft SQL Server. Burp Suite identifikoi enumerim të emrave të përdoruesve dhe mungesë të flamujve HttpOnly në cookie-t.


---

## Rezultatet e Platformave Online

- **Shodan**: Target-i HTTP tregoi port MySQL të ekspozuar dhe software të vjetruar. Target-i HTTPS tregoi certifikatë të skaduar dhe porte shtesë të hapura.
- **SecurityHeaders.com**: Target-i HTTP mori notë **F** ( pothuajse pa header-e). Target-i HTTPS mori notë **D** (pak më mirë, por ende pa header-e kyçe si CSP dhe HSTS).
- **SSL Labs**: Target-i HTTP nuk kishte TLS. Target-i HTTPS mori notë **C** për shkak të certifikatës së skaduar, shifrave të dobëta dhe mungesës së HSTS.
- **Mozilla Observatory**: Target-i HTTP mori **0/100**. Target-i HTTPS mori **35/100**.
- **VirusTotal**: Të dy target-et u njohën si mjedise legjitime kërkimi sigurie pa flamuj keqdashës.

---

## Analizë Krahasuese HTTP vs HTTPS

### Shtresa e Transportit
- **HTTP**: Pa enkriptim. Kredencialet, sesionet dhe të dhënat transmetohen në plaintext dhe janë vulnerabël ndaj interceptimit në rrjet.
- **HTTPS**: Siguron enkriptim TLS, duke mbrojtur të dhënat në transit. Megjithatë, mungesa e HSTS, certifikata e skaduar dhe shifrat e dobëta ulin efektivitetin.

### Shtresa e Aplikacionit
Të dy target-et vuajtën nga probleme të ngjashme (injeksion SQL, XSS, CSRF, autentikim i thyer). Këto defekte ekzistojnë në kod dhe nuk parandalohen nga HTTPS.

### Sipërfaqja e Përgjithshme e Sulmit
Target-i HTTP kishte dukshëm më shumë vektora sulmi për shkak të mungesës së enkriptimit dhe ekspozimit të rrjetit. Target-i HTTPS kishte më pak vektora, por përmbante ende dobësi serioze të aplikacionit dhe TLS.

**Ide Kryesore:** HTTPS enkripton kanalin e komunikimit, por nuk e siguron vetë aplikacionin. Praktikat e kodimit të sigurt kërkohen pavarësisht nga protokolli i përdorur.

---

## Tabela e Konsoliduar e Vulnerabiliteteve

| ID   | Vulnerabiliteti                        | CWE     | CVSS | Target | U zbulua nga              |
|------|----------------------------------------|---------|------|--------|---------------------------|
| V-01 | Injeksion SQL                          | CWE-89  | 9.8  | Të dy  | ZAP, Burp, SQLMap         |
| V-02 | Kredenciale në Plaintext               | CWE-319 | 9.1  | HTTP   | ZAP, Burp                 |
| V-03 | Anashkalim Autentikimi                 | CWE-287 | 9.8  | HTTPS  | Burp, SQLMap              |
| V-04 | XSS i Reflektuar / i Ruajtur           | CWE-79  | 7.4–8.2 | Të dy | ZAP, Burp                 |
| V-05 | PHP i Vjetruar (End-of-Life)           | CWE-1104| 9.8  | HTTP   | Nikto, Nmap               |
| V-06 | CSRF (pa token-e)                      | CWE-352 | 6.5  | Të dy  | ZAP                       |
| V-07 | Port Baze të Dhënash i Ekspozuar (MySQL 3306) | CWE-16 | 8.6 | HTTP   | Nmap, Shodan              |
| V-08 | Mungesë HSTS                           | CWE-311 | 6.8  | HTTPS  | Nikto, Observatory        |
| V-09 | Certifikatë TLS e Skaduar              | CWE-295 | 7.5  | HTTPS  | Nmap, SSL Labs            |
| V-10 | Shifra të Dobëta (RC4)                 | CWE-327 | 7.4  | HTTPS  | SSL Labs, Nmap            |
| V-11 | Mungesë Header-ësh Sigurie            | CWE-693 / CWE-1021 | 4.7–6.1 | Të dy | SecurityHeaders, ZAP    |

---

## Konkluzioni
Rezultatet treguan se target-i HTTP paraqet një rrezik dukshëm më të lartë për shkak të mungesës së enkriptimit, duke ekspozuar plotësisht kredencialet, sesionet dhe të dhënat e ndjeshme ndaj interceptimit të thjeshtë në rrjet. Ndërkohë, target-i HTTPS siguron mbrojtje efektive në nivel transporti, por nuk arrin të eliminojë vulnerabilitetet kritike në nivel aplikacioni si injeksioni SQL, XSS dhe thyerja e mekanizmit të autentikimit.

Ky studim konfirmon një parim themelor të sigurisë së web-it: **HTTPS enkripton kanalin e komunikimit, por nuk e siguron aplikacionin vetë**. Vulnerabilitetet më të rënda mbeten të pavarura nga protokolli i përdorur dhe varen kryesisht nga cilësia e kodimit, menaxhimi i inputit dhe konfigurimi i serverit.

**Në përfundim**, ky punim nxjerr në pah se:
1. Përdorimi i HTTPS është një kërkesë minimale dhe e detyrueshme por nuk përbën siguri të plotë.
2. Siguria reale e aplikacioneve web kërkon një qasje me shtresa (defense in depth) që kombinon enkriptimin e transportit me praktika të sigurta zhvillimi dhe konfigurime të forta të serverit.
3. Testimi i rregullt i sigurisë dhe ndërgjegjësimi për kufizimet e çdo teknologjie mbeten thelbësorë për mbrojtjen efektive të sistemeve.
