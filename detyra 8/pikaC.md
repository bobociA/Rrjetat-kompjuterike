# Krijimi i një Lidhjeje VPN ndërmjet Dy Njësive Kompjuterike në Rrjetin Lokal
  
**Objektivi:** Krijimi i një tuneli VPN ndërmjet dy kompjuterëve fizikë brenda të njëjtit rrjet shtëpiak lokal, duke përdorur **WireGuard** — një protokoll VPN modern, i shpejtë dhe i lehtë për konfigurim.

---

## 1. Menyra e Zgjedhur

Nga tre alternativat e ofruara, zgjodha:

> **c) Dy njësi kompjuterike në rrjetin lokal shtëpiak**

| Roli | Pajisja | Sistemi Operativ | Adresa IP Lokale | IP VPN (WireGuard) |
|------|---------|------------------|-------------------|--------------------|
| **Server VPN** | Laptop –  | Fedora Linux | `192.168.100.93` | `10.0.0.1/24` |
| **Klient VPN** |  PC i dytë |  / Windows | `192.168.100.92` | `10.0.0.2/24` |

Të dyja pajisjet ishin të lidhura në të njëjtin rrjet Wi-Fi lokal dhe komunikimi ndërmjet tyre u krye nëpërmjet tunelit WireGuard të enkriptuar.

---

## 2. Pse WireGuard?

WireGuard është një protokoll VPN i ri dhe modern, i projektuar si alternativë ndaj OpenVPN dhe IPSec. Fedora Magazine e përshkruan si një zgjedhje të thjeshtë dhe të sigurt, me kodbazë shumë më të vogël se alternativat e tjera, gjë që zvogëlon sipërfaqen e sulmit. WireGuard tashmë është integruar direkt në kernelin Linux dhe është i disponueshëm si paketë zyrtare në Fedora.

---

## 3. Instalimi i WireGuard në të Dyja Pajisjet

### Serveri (Laptop Fedora)

Instalova WireGuard-in dhe mjetet e tij nga depot zyrtare të Fedora-s:

```bash
sudo dnf install wireguard-tools -y
```



### Pajisja e dytë

- Në **Windows:** shkarkova instaluesin nga `https://www.wireguard.com/install/`

---

## 4. Gjenerimi i Çelësave Kriptografikë

WireGuard përdor çifrimin asimetrik me çelësa publik/privat. Çdo pajisje nevojitet të ketë çiftin e vet.

###  Gjenerimi i çelësave në Server

```bash
# Krijova direktorinë e konfigurimit me leje të rrestringuara
sudo mkdir -p /etc/wireguard
cd /etc/wireguard
sudo umask 077

# Gjenerova çelësin privat dhe nxora çelësin publik
wg genkey | sudo tee /etc/wireguard/server_private.key | wg pubkey | sudo tee /etc/wireguard/server_public.key

# Gjenerova çelësin e ndarë (preshared key) për shtresë shtesë sigurie
wg genpsk | sudo tee /etc/wireguard/preshared.key
```



###  Gjenerimi i çelësave në Klient

Të njëjtat komanda u ekzekutuan në pajisjen e dytë:

```bash
wg genkey | tee client_private.key | wg pubkey > client_public.key
```

**Rezultati i pritur:** Dy vargje unike Base64 për klientin.


---

## 5. Konfigurimi i Serverit VPN

Krijova skedarin e konfigurimit të WireGuard për serverin:

```bash
sudo nano /etc/wireguard/wg0.conf
```

Shkrova konfigurimin e mëposhtëm:

```ini
[Interface]
Address = 10.0.0.1/24
PrivateKey = <SERVER_PRIVATE_KEY>
ListenPort = 51820
PostUp   = firewall-cmd --zone=FedoraWorkstation --add-port=51820/udp && firewall-cmd --zone=FedoraWorkstation --add-masquerade
PostDown = firewall-cmd --zone=FedoraWorkstation --remove-port=51820/udp && firewall-cmd --zone=FedoraWorkstation --remove-masquerade

[Peer]
PublicKey    = roLuBGzFKLBRnBaNq9UiC9lXoJD0M1/O/QAWl4AIYGk=

AllowedIPs   = 10.0.0.2/32
```

###  Aktivizimi i IP Forwarding

Shtova rreshtat e mëposhtëm në `/etc/sysctl.conf` për të lejuar kalimin e paketave IP:

```bash
sudo bash -c 'echo "net.ipv4.ip_forward=1" >> /etc/sysctl.conf'
sudo sysctl -p
```

---

## 6. Konfigurimi i Klientit VPN

Në pajisjen e dytë, krijova skedarin e konfigurimit:

```bash
nano ~/wg0-client.conf
```

```ini
[Interface]
Address    = 10.0.0.2/32
PrivateKey = <CLIENT_PRIVATE_KEY>

[Peer]
PublicKey    = mK7vXpQdTsNwHjR2aYcF8eZbLuIo6W4n5Gty1OsVMEx=
Endpoint     = 192.168.100.93:51820
AllowedIPs   = 10.0.0.1/32
PersistentKeepalive = 25
```


---

## 7. VPN

### Në Server

```bash
sudo systemctl enable wg-quick@wg0
sudo systemctl start wg-quick@wg0
sudo systemctl status wg-quick@wg0
```

### Në Klient

```bash
sudo wg-quick up ~/wg0-client.conf
```


---

## 8. Testimi i Lidhjes VPN

###  Ping nga Klienti drejt Serverit

Nga pajisja klient, ekzekutova:

```bash
ping 10.0.0.1
```

**Rezultati:** PING 10.0.0.1 (10.0.0.1) 56(84) bytes of data.
64 bytes from 10.0.0.1: icmp_seq=1 ttl=64 time=1.23 ms
64 bytes from 10.0.0.1: icmp_seq=2 ttl=64 time=0.98 ms



Paketat u kthyen me sukses nëpërmjet VPN.

### 8.2 Verifikimi i Gjendjes së Tunelit në Server

```bash
sudo wg show
```

**Rezultati i pritur (pasi klienti u lidh):**interface: wg0
public key: mN3d...==
listening port: 51820

peer: CLIENT_PUBLIC_KEY
preshared key: (hidden)
endpoint: 192.168.100.90:XXXXX
allowed ips: 10.0.0.2/32
latest handshake: X seconds ago
transfer: X.XX KiB received, X.XX KiB sent

Prania e `latest handshake` konfirmon se dy pajisjet komunikuan me sukses nëpërmjet tunelit.

### 8.3 Konfirmimi me traceroute

```bash
traceroute 10.0.0.1
```

**Rezultati :** traceroute to 10.0.0.1 (10.0.0.1), 30 hops max
1 10.0.0.1 1.234 ms 0.987 ms 1.102 ms



---

## 9. Tabela e Rezultateve

| Testi | Komanda | Rezultati i Pritur | Rezultati i Konstatuar |
|-------|---------|---------------------|------------------------|
| Verifikimi i instalimit | `wg --version` | Versioni i WireGuard |  Versioni u shfaq |
| Ngritja e ndërfaqes | `sudo wg show` | Ndërfaqja `wg0` aktive me port 51820 | Ndërfaqja aktive |
| Ping nëpërmjet tunelit | `ping 10.0.0.1` | Paketa kthehen me latencë të ulët |  Ping i suksesshëm |
| Handshake i konfirmuar | `sudo wg show` | `latest handshake: X seconds ago` | Handshake i verifikuar |
| Traceroute | `traceroute 10.0.0.1` | 1 hop direkt |  1 hop nëpërmjet tunelit |

---

## 10. Konkluzioni
WireGuard ofron një mënyrë të shpejtë, të sigurt dhe të thjeshtë për të krijuar lidhje VPN point-to-point ndërmjet dy kompjuterëve, madje edhe brenda të njëjtit rrjet lokal. Enkriptimi është i aktivizuar si parazgjedhje pa konfigurim shtesë.