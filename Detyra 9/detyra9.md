# Raporti i Detyrës - Konfigurimi i Windows Server 2019

## Përmbledhje

Ky raport dokumenton instalimin dhe konfigurimin e plotë të një mjedisi Windows Server 2019 me Active Directory Domain Services (ADDS), DNS, DHCP, si dhe bashkimin e një klienti Windows 10 në domen. Gjithashtu janë krijuar Njësi Organizative (OU), Përdorues, Grupe, Dosje të Ndara me Leje, dhe Politika Grupore (GPO).

---

## Faza 1 — Instalimi i VirtualBox dhe Makinave Virtuale

### 1.1 Krijimi i VM-së për Windows Server 2019

- **Emri i VM:** anxhelaso
- **Sistemi Operativ:** Windows Server 2019 (64-bit)
- **RAM:** 2048 MB
- **CPU:** 1 bërthamë
- **Disku:** 20 GB
- **Rrjeti:** Internal Network → `intnet`

### 1.2 Krijimi i VM-së për Windows 10 Client

- **Emri i VM:** WIN10-Client
- **Sistemi Operativ:** Windows 10 Pro (64-bit)
- **RAM:** 2048 MB
- **CPU:** 2 bërthama
- **Disku:** 20 GB
- **Rrjeti:** Internal Network → `intnet`

> **Shënim:** Të dyja makinat virtuale janë vendosur në të njëjtin rrjet të brendshëm (`intnet`) për të komunikuar me njëra-tjetrën pa akses interneti.

---

## Faza 2 — Konfigurimi i IP Statike në Server

Para instalimit të ADDS, serverit iu caktua një adresë IP statike:

| Parametri | Vlera |
|-----------|-------|
| Adresa IPv4 | `192.168.1.1` |
| Maska e Nënrrjetit | `255.255.255.0` |
| Porta Kryesore | `192.168.1.1` |
| DNS Preferuar | `127.0.0.1` (loopback) |

**Hapat:**
1. `Control Panel` → `Network and Sharing Center`
2. `Change adapter settings` → klik i djathtë mbi adapterin → `Properties`
3. `Internet Protocol Version 4 (TCP/IPv4)` → `Properties`
4. Vendosja e vlerave statike si në tabelë → `OK`

---

## Faza 3 — Instalimi i ADDS dhe Promovimi si Domain Controller

### 3.1 Instalimi i Rolit ADDS

1. Hapja e **Server Manager** → `Add Roles and Features`
2. Zgjedhja e **Active Directory Domain Services**
3. Konfirmimi dhe instalimi i rolit
4. Pas instalimit: klik në **"Promote this server to a domain controller"**

### 3.2 Konfigurimi i Domain Controller

| Parametri | Vlera |
|-----------|-------|
| Lloji i Pyllit | New Forest |
| Emri i Domenit | `INFO3A.TEST` |
| Emri NetBIOS | `INFO3A` |
| Niveli Funksional i Pyllit | Windows Server 2016 |
| DNS Server | Po (i instaluar automatikisht) |

5. Vendosja e fjalëkalimit DSRM (Directory Services Restore Mode)
6. Rishikimi i konfigurimit → `Install`
7. Rinisja automatike e serverit pas instalimit

---

## Faza 4 — Konfigurimi i DNS

DNS u instalua automatikisht gjatë promovimit të Domain Controller. U verifikua si vijon:

1. `Server Manager` → `Tools` → `DNS`
2. Zgjerimi i serverit → `Forward Lookup Zones`
3. Zona `INFO3A.TEST` është e pranishme

**Testimi i DNS nga klienti:**
```cmd
ping INFO3A.TEST
nslookup INFO3A.TEST
```
Të dyja komandat kthyen përgjigje pozitive nga `192.168.1.1`

---

## Faza 5 — Instalimi dhe Konfigurimi i DHCP

### 5.1 Instalimi i Rolit DHCP

1. `Server Manager` → `Add Roles and Features`
2. Zgjedhja e **DHCP Server** → Instalimi
3. Post-instalimi: **"Complete DHCP Configuration"** → Autorizimi me kredenciale Administrator

### 5.2 Krijimi i Fushës DHCP (Scope)

| Parametri | Vlera |
|-----------|-------|
| Emri i Fushës | `INFO3A-Scope` |
| IP Fillestare | `192.168.1.100` |
| IP Fundore | `192.168.1.200` |
| Maska e Nënrrjetit | `255.255.255.0` |
| Porta Kryesore | `192.168.1.1` |
| DNS Server | `192.168.1.1` |
| Kohëzgjatja e Dhënies | 8 ditë |

### 5.3 Autorizimi i DHCP Server

- `Server Manager` → `Tools` → `DHCP`
- Klik i djathtë mbi emrin e serverit → `Authorize`
- Shigjetë e gjelbër konfirmon autorizimin e suksesshëm

---

## Faza 6 — Bashkimi i Klientit Windows 10 në Domen

### 6.1 Konfigurimi i Rrjetit në Klient

Klienti Windows 10 mori automatikisht adresë IP nga DHCP:

| Parametri | Vlera |
|-----------|-------|
| Adresa IPv4 | `192.168.1.100` |
| Maska e Nënrrjetit | `255.255.255.0` |
| Porta Kryesore | `192.168.1.1` |
| DHCP Server | `192.168.1.1` |
| DNS Server | `192.168.1.1` |

### 6.2 Caktimi Manual i DNS

```cmd
netsh interface ip set dns "Ethernet" static 192.168.1.1
```

### 6.3 Testimi i Lidhjes

```cmd
ping 192.168.1.1
ping INFO3A.TEST
```
Të dyja pinguet u kryen me sukses

### 6.4 Bashkimi në Domen

1. `Win + R` → `sysdm.cpl` → Enter
2. Skeda **Computer Name** → `Change`
3. Emri i kompjuterit: `CLIENT1`
4. Zgjedhja e **Domain** → `INFO3A.TEST` → `OK`
5. Kredencialet: `Administrator` / fjalëkalimi i serverit
6. Mesazhi: **"Welcome to the INFO3A.TEST domain!"**
7. Rinisja e klientit

### 6.5 Hyrja me Llogari Domeni

- Klik **Other user** → Username: `INFO3A\Administrator`
- Fjalëkalimi i serverit → Enter

---

## Faza 7 — Krijimi i Njësive Organizative (OU)

**Mjeti:** Active Directory Users and Computers (`dsa.msc`)

| OU | Vendndodhja |
|----|-------------|
| `INFO3A_Students` | `INFO3A.TEST` |
| `INFO3A_Teachers` | `INFO3A.TEST` |

**Hapat:**
1. Klik i djathtë mbi `INFO3A.TEST` → `New` → `Organizational Unit`
2. Emri: `INFO3A_Students` → `OK`
3. Përsëritja e hapave për `INFO3A_Teachers` → `OK`

---

## Faza 8 — Krijimi i Përdoruesve dhe Grupeve

### 8.1 Përdoruesit e Krijuar

| Emri | Logon Name | OU | Grupi |
|------|------------|----|-------|
| Student1 Info3a | `student1` | `INFO3A_Students` | Students |
| Student2 Info3a | `student2` | `INFO3A_Students` | Students |
| Teacher1 Info3a | `teacher1` | `INFO3A_Teachers` | Teachers |

**Hapat për çdo përdorues:**
1. Klik mbi OU-në → klik i djathtë → `New` → `User`
2. Plotësimi i emrit dhe logon name
3. Fjalëkalimi: `Pass1234!`
4. Çaktivizimi i "User must change password at next logon"
5. Aktivizimi i "Password never expires" → `Finish`

### 8.2 Grupet e Krijuar

| Grupi | Lloji | Fushëveprimi | OU |
|-------|-------|--------------|-----|
| `Students` | Security | Global | `INFO3A_Students` |
| `Teachers` | Security | Global | `INFO3A_Teachers` |

**Shtimi i anëtarëve:**
1. Klik i dyfishtë mbi grupin → skeda `Members` → `Add`
2. Shtimi i `student1` dhe `student2` në grupin `Students`
3. Shtimi i `teacher1` në grupin `Teachers`

---

## Faza 9 — Dosjet e Ndara dhe Lejet (Shared Folders & Permissions)

### 9.1 Struktura e Dosjeve

```
C:└── Shares    ├── Students    └── Teachers```

### 9.2 Konfigurimi i Dosjes Students

**Share Permissions:**
| Grupi | Leja |
|-------|------|
| `Students` | Read, Change |

**NTFS Permissions:**
| Grupi | Leja |
|-------|------|
| `Students` | Modify, Read & Execute, Write |

**Hapat:**
1. Klik i djathtë mbi `C:\Shares\Students` → `Properties`
2. Skeda `Sharing` → `Advanced Sharing` → Aktivizimi i ndarjes
3. `Permissions` → Heqja e `Everyone` → Shtimi i `Students`
4. Caktimi i lejeve të ndarjes → `Apply`
5. Skeda `Security` → `Edit` → `Add` → `Students`
6. Caktimi i NTFS lejeve → `Apply` → `OK`

### 9.3 Konfigurimi i Dosjes Teachers

I njëjti konfigurim si për Students, por me grupin `Teachers`.

---

## Faza 10 — Politikat Grupore (GPO)

### 10.1 Krijimi i GPO për Students

**Mjeti:** Group Policy Management (`gpmc.msc`)

1. Zgjerimi i `INFO3A.TEST` në panelin e majtë
2. Klik i djathtë mbi `INFO3A_Students` → `"Create a GPO in this domain, and Link it here"`
3. Emri: `Students_Policy` → `OK`

### 10.2 Konfigurimi i GPO

**Rruga e cilësimit:**
```
User Configuration
└── Policies
    └── Administrative Templates
        └── Control Panel
            └── Prohibit access to Control Panel and PC settings
```

1. Klik i djathtë mbi `Students_Policy` → `Edit`
2. Navigimi te: `User Configuration` → `Policies` → `Administrative Templates` → `Control Panel`
3. Klik i dyfishtë mbi **"Prohibit access to Control Panel and PC settings"**
4. Zgjedhja e **Enabled** → `Apply` → `OK`
5. Mbyllja e editorit

### 10.3 Testimi i GPO

Nga klienti Windows 10, i hyrë si `INFO3A\student1`:

```cmd
gpupdate /force
```

**Rezultati:** Pas ekzekutimit të `gpupdate /force`, hapja e Control Panel shfaqi mesazhin:

> *"This operation has been canceled due to restrictions in effect on this computer. Please contact your system administrator."*

**GPO aplikohet me sukses**

---

## Verifikimi i Plotë

### Nga Klienti (CMD)

```cmd
whoami
# Rezultati: info3a\student1

systeminfo | findstr /B /C:"Domain"
# Rezultati: Domain: INFO3A.TEST

ipconfig /all
# IPv4: 192.168.1.100
# DHCP Server: 192.168.1.1
# DNS Server: 192.168.1.1

gpresult /r
# Applied GPO: Students_Policy
```

---

## Tabela Përmbledhëse e Konfigurimit

| Komponenti | Vlera/Statusi |
|------------|---------------|
| Domeni | `INFO3A.TEST` |
| Domain Controller | Windows Server 2019 |
| IP e Serverit | `192.168.1.1` |
| IP e Klientit (DHCP) | `192.168.1.100` |
| Gama DHCP | `192.168.1.100 - 192.168.1.200` |
| DNS Server | `192.168.1.1` |
| OU Students | `INFO3A_Students` |
| OU Teachers | `INFO3A_Teachers` |
| Grupe | `Students`, `Teachers` |
| Përdorues | `student1`, `student2`, `teacher1` |
| Dosje të Ndara | `C:\Shares\Students`, `C:\Shares\Teachers` |
| GPO | `Students_Policy` (bllokimi i Control Panel) |

---

## Përfundim

- Instalimi i VirtualBox dhe dy makinave virtuale
- Konfigurimi i IP statike dhe rrjetit të brendshëm
- Instalimi i ADDS dhe promovimi si Domain Controller
- Konfigurimi i DNS dhe DHCP
- Bashkimi i klientit Windows 10 në domenin `INFO3A.TEST`
- Krijimi i OU-ve, Përdoruesve dhe Grupeve
- Konfigurimi i dosjeve të ndara me leje NTFS
- Aplikimi dhe testimi i GPO-ve

---

