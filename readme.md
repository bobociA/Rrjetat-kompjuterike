# Samba server orientuar në makina virtuale

## Kredencialet e përdorura

| Sistemi / Roli | Username | Roli i llogarisë | Password |
|---|---|---|---|
| Fedora Host | `boboci` | Përdorues lokal i host-it | `[Redacted]` |
| Ubuntu Samba Server | `admin` | Administrator i serverit | `AdminFedora2026!` |
| Samba user | `sambauser` | Përdorues i autorizuar për share | `SecureSamba2026!` |
| Win10-Client1 | `studentadmin` | Administrator lokal | `Win10Admin2026!` |
| Win11-Client2 | `studentadmin` | Administrator lokal | `Win11Admin2026!` |
| VM shtesë për skanim | `netadmin` | Administrator lokal | `NetScan2026!` |

---

## Përmbajtja
1. [Pika A – Shpërndarja e Skedarëve me Samba]
2. [Pika B – Konfigurimi i Firewall-eve dhe Krahasimi]
3. [Pika C – Skanimi i Rrjetit dhe Analiza]
4. [Pika D – Aksesi SSH dhe Shkëmbimi i Skedarëve nga Host-i Fedora]
5. [Përfundim & Mësimet e Nxjerra]

---

## Pika A – Funksionaliteti i Shpërndarjes së Skedarëve me Serverin Samba

### Makinat Virtuale të Krijuara

| Emri i VM-së | Sistemi Operativ | Roli | IP Statike | RAM / CPU | Shtegu i Dosjes së Përbashkët |
|---|---|---|---|---|---|
| Samba-Server | Ubuntu Server 22.04 LTS | Server Samba | 192.168.56.101 | 2 GB / 2 | /srv/samba/shared |
| Win10-Client1 | Windows 10 Pro (22H2) | Klient 1 | 192.168.56.102 | 2 GB / 2 | N/A |
| Win11-Client2 | Windows 11 Pro (24H2) | Klient 2 | 192.168.56.103 | 2 GB / 2 | N/A |
| Win10-ScanVM | Windows 10 Pro | VM shtesë për SolarWinds IPAM | 192.168.56.104 | 2 GB / 2 | N/A |

**Cilësimet e Adaptorit të Rrjetit (për të gjitha VM-të):**
- Lidhur me: Rrjet i Brendshëm
- Modaliteti Promiskuus: Lejo të Gjitha


### Konfigurimi i Detajuar i Serverit Samba (Ubuntu 22.04 LTS)

1. Përditësimi i sistemit dhe instalimi i Samba:

```bash
sudo apt update && sudo apt upgrade -y
sudo apt install samba -y
```

2. Krijimi i dosjes së përbashkët dhe lejet:

```bash
sudo mkdir -p /srv/samba/shared
sudo chown -R nobody:nogroup /srv/samba/shared
sudo chmod -R 777 /srv/samba/shared
echo "Ky skedar u krijua drejtpërdrejt në serverin Linux nga Anxhela" > /srv/samba/shared/linux_test.txt
```

3. Krijimi i përdoruesit Samba:

```bash
sudo adduser --no-create-home --disabled-password --gecos "" sambauser
sudo smbpasswd -a sambauser
```

**Kredencialet e përdorura për akses Samba:**
- Username: `sambauser`
- Password: `SecureSamba2026!`

4. Skedari `/etc/samba/smb.conf`:

```ini
[global]
   workgroup = WORKGROUP
   server string = Server Samba Anxhela në Host Fedora
   netbios name = SAMBASRV
   security = user
   map to guest = bad user
   dns proxy = no
   log level = 2

[SharedFolder]
   path = /srv/samba/shared
   browseable = yes
   writable = yes
   guest ok = no
   read only = no
   valid users = sambauser
   force user = sambauser
   create mask = 0664
   directory mask = 0775
```

5. Aplikimi dhe rinisja:

```bash
sudo testparm
sudo systemctl restart smbd
sudo systemctl enable smbd
```

6. Verifikimi në server:

```bash
smbclient -L localhost -U sambauser
```

Rezultati: `SharedFolder` u shfaq me sukses.

### Aksesi nga Klientët Windows

Nga `Win10-Client1` dhe `Win11-Client2` u përdor shtegu:

```text
\\192.168.56.101\SharedFolder
```

**Kredencialet e futura në dritaren e autentikimit:**
- Username: `sambauser`
- Password: `SecureSamba2026!`

Të gjitha ndryshimet u shfaqën menjëherë në serverin Linux:

```bash
ls -la /srv/samba/shared
```

### Testi Ndërmjet Klientëve Windows

- Në `Win10-Client1` u përdor llogaria lokale administratore `studentadmin / Win10Admin2026!`.
- Në `Win11-Client2` u përdor llogaria lokale administratore `studentadmin / Win11Admin2026!`.
- Dosja e përbashkët: `C:\Win10Shared` në `Win10-Client1`.
- Akses dhe transferim skedarësh në të dy drejtimet nga `Win11-Client2` përmes `\\192.168.56.102\Win10Shared`.
- Testuar me skedarë të mëdhenj dhe kopjime simultane, pa gabime.

### Dëshmi

- **A1:** Dosja Samba e montuar në Windows 10 Explorer
- **A2:** Dosja Samba e montuar në Windows 11 Explorer
- **A3:** Skedarët e krijuar nga Windows të dukshëm në terminalin Linux (`ls -la`)
- **A4:** Transferimi bidireksional i skedarëve ndërmjet dy klientëve Windows
- **A5:** Validimi i `smb.conf` dhe dalja `systemctl status smbd`

---

## Pika B – Konfigurimi i Firewall-eve dhe Krahasimi

### 1. Firewall-i i Serverit Samba (Ubuntu – UFW)

Konfigurimi u krye nga llogaria administratore `admin` në Ubuntu server.

```bash
sudo ufw default deny incoming
sudo ufw default allow outgoing
sudo ufw allow ssh
sudo ufw allow samba
sudo ufw allow 139/tcp
sudo ufw allow 445/tcp
sudo ufw --force enable
sudo ufw status verbose
```

**Rezultati:** Vetëm portat 22, 139 dhe 445 të hapura. Çdo gjë tjetër e bllokuar.

### 2. Windows Firewall në Win10-Client1 (i integruar)

- Konfiguruar përmes **Windows Defender Firewall with Advanced Security**.
- Hyrja u krye me llogarinë `studentadmin`.
- Aktivizuar rregulla `File and Printer Sharing (SMB-In)` për profilin **Private**.
- Krijuar rregullë e personalizuar inbound: TCP 139 dhe 445, e kufizuar vetëm në `192.168.56.0/24`.
- Rregullat e eksportuara si `Win10_Firewall_Rules.wfw`.

### 3. Firewall i Tretë në Win11-Client2 (GlassWire Elite)

- Instaluar dhe konfiguruar nga llogaria `studentadmin`.
- Aktivizuar modaliteti i plotë Firewall (`default deny`).
- Krijuar rregullë e personalizuar për SMB (139/445) vetëm nga `192.168.56.0/24`.
- Aktivizuar grafikët në kohë reale dhe `Ask to Connect`.

### Krahasimi i Detajuar i Firewall-eve

| Veçori | Windows Defender Firewall (i integruar) | GlassWire Elite (i instaluar) | Fitues |
|---|---|---|---|
| Ndërfaqja e Konfigurimit | GUI standard Windows | Panel modern vizual me grafikë | GlassWire |
| Kontrolli i Aplikacioneve | Nivel porti/programi | Per-app + analizë inteligjente e sjelljes | GlassWire |
| Monitorimi në Kohë Reale | Asnjë grafik | Grafikë trafiku live + alarme të menjëhershme | GlassWire |
| Kontrolli Outbound | Shumë i kufizuar | Kontroll i plotë + njoftime | GlassWire |
| Përdorimi i Burimeve | Pothuajse zero | I ulët (~35 MB RAM) | Windows |
| Logjet & Raportet | Event Viewer bazik | Logje të detajuara të eksportueshme + WHOIS | GlassWire |
| Veçori Shtesë Sigurie | Asnjë | Zbulim kërcënimesh & monitorim bandwidth-i | GlassWire |
| Më i mirë për Laboratorë Studentësh | I shpejtë dhe i thjeshtë | Vështrim edukativ dhe profesional | GlassWire |

**Përfundim:** Firewall-i i integruar është i mjaftueshëm për përdorim bazik, por GlassWire është më i përshtatshëm për mësim dhe monitorim vizual.

### Dëshmi

- **B1:** `ufw status verbose` në serverin Ubuntu
- **B2:** Rregullat e avancuara të Windows Defender Firewall (Win10)
- **B3:** Paneli GlassWire dhe rregulla e personalizuar (Win11)

---

## Pika C – Skanimi i Rrjetit dhe Analiza

### 1. SolarWinds IP Address Manager (IPAM)

- Vegël: **SolarWinds IPAM** (trial 30-ditore).
- Për shkak se host-i është Fedora, vegla u ekzekutua brenda VM-së `Win10-ScanVM`.
- Në këtë VM u përdor llogaria administratore `netadmin / NetScan2026!`.
- U skanua subnet-i `192.168.56.0/24`.

**Rezultatet dhe interpretimi:**
- 4 pajisje të zbuluara.
- `192.168.56.101` -> Ubuntu 22.04 LTS, MAC `08:00:27:xx:xx:xx`, porta të hapura 22, 139, 445.
- `192.168.56.102` -> Windows 10, porta të hapura 135, 139, 445, 3389.
- `192.168.56.103` -> Windows 11, porta të hapura 135, 139, 445, 3389.
- Asnjë konflikt IP, tabelë ARP e pastër.
- Interpretimi: Të gjitha VM-të janë të shëndetshme dhe të arritshme. Portat Samba janë të kufizuara sipas konfigurimit dhe rregullat e firewall-it funksionojnë siç pritet.

### 2. Vegël Shtesë Skanimi – Nmap (në Fedora host)

```bash
nmap -sS -O -p- -T4 192.168.56.0/24
```

**Gjetjet kryesore:** Të njëjta me SolarWinds; OS dhe portat e hapura u konfirmuan.

### 3. Skanim kundër platformës së jashtme (`scanme.nmap.org`)

```bash
nmap -sV -O scanme.nmap.org
```

**Rezultatet:**
- `22/tcp open ssh` (OpenSSH)
- `80/tcp open http` (Apache)
- OS: Linux

**Interpretimi:** Demonstron përdorim praktik të veglave të skanimit si në laborator lokal ashtu edhe në objektiva testues publikë.

### Dëshmi

- **C1:** Raporti i skanimit SolarWinds IPAM
- **C2:** Dalja e Zenmap GUI (subnet lokal)
- **C3:** Skanimi Nmap i `scanme.nmap.org` me versionet e shërbimeve

---

## Pika D – Aksesi SSH dhe Shkëmbimi i Skedarëve nga Host-i Fedora

### Veglat e përdorura

- Komanda `ssh` në terminal (klienti OpenSSH, i instaluar si parazgjedhje në Fedora)
- Komanda `scp` për transferim skedarësh

### Hapat e Kryer (të gjitha nga terminali `boboci@fedora`)

1. Lidhja SSH me `Samba-Server`:

```bash
ssh admin@192.168.56.101
```

**Kredencialet e përdorura për SSH:**
- Username: `admin`
- Password: `AdminFedora2026!`

- U pranua fingerprint-i i çelësit të host-it.
- Hyrja ishte e suksesshme.

2. Brenda sesionit SSH:

```bash
cd /srv/samba/shared
ls -la
echo "Ky skedar u krijua përmes SSH nga host-i im Fedora (boboci@fedora)" > host_via_ssh.txt
```

3. Shkëmbimi i skedarëve (`Host <-> VM`) duke përdorur `scp`:

Nga Fedora host drejt VM:

```bash
scp ~/Documents/host_document.pdf admin@192.168.56.101:/srv/samba/shared/
```

Nga VM drejt Fedora host:

```bash
scp admin@192.168.56.101:/srv/samba/shared/vm_photo.jpg ~/Downloads/
```

### Verifikimi

- Skedarët e rinj u shfaqën në dosjen Samba.
- Të dukshëm edhe nga të dy klientët Windows.
- Integriteti i skedarëve u kontrollua me `md5sum` në të dy anët.

### Dëshmi

- **D1:** Sesioni i terminalit SSH duke treguar listën e dosjeve dhe krijimin e skedarit të ri
- **D2:** Komandat `scp` dhe dalja e suksesshme (host -> VM dhe VM -> host)
- **D3:** Skedarët e dukshëm në dosjen Samba pas transferimit SSH (të parë nga klientët Windows)

---
