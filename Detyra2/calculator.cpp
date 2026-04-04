#include <iostream>
#include <sstream>
#include <bitset>
#include <cstdint>
#include <string>

using namespace std;


class IP
{
private:
    uint32_t value;

public:
    IP()
    {
        value = 0;
    }

    IP(uint32_t v)
    {
        value = v;
    }

    IP(int o1, int o2, int o3, int o4)
    {
        value = ((uint32_t)o1 << 24) |
                ((uint32_t)o2 << 16) |
                ((uint32_t)o3 << 8) |
                (uint32_t)o4;
    }

    uint32_t get() const
    {
        return value;
    }

    int octet(int n) const
    {
        return (value >> (8 * (4 - n))) & 0xFF;
    }

    string toDecimal() const
    {
        return to_string(octet(1)) + "." +
               to_string(octet(2)) + "." +
               to_string(octet(3)) + "." +
               to_string(octet(4));
    }

    string toBinary() const
    {
        return bitset<8>(octet(1)).to_string() + "." +
               bitset<8>(octet(2)).to_string() + "." +
               bitset<8>(octet(3)).to_string() + "." +
               bitset<8>(octet(4)).to_string();
    }

    char getClass() const
    {
        int o1 = octet(1);
        if (o1 <= 127)
            return 'A';
        if (o1 <= 191)
            return 'B';
        if (o1 <= 223)
            return 'C';
        if (o1 <= 239)
            return 'D';
        return 'E';
    }

    //ip dinamike ose statike 

    bool isPrivate() const
    {
        int o1 = octet(1);
        int o2 = octet(2);

        if (o1 == 10)
            return true;
        if (o1 == 172 && o2 >= 16 && o2 <= 31)
            return true;
        if (o1 == 192 && o2 == 168)
            return true;

        return false;
    }
};

bool validOctet(int x)
{
    return x >= 0 && x <= 255;
}

bool parseIP(const string &input, IP &ip)
{
    int o1, o2, o3, o4;
    char d1, d2, d3;
    stringstream ss(input);

    if (!(ss >> o1 >> d1 >> o2 >> d2 >> o3 >> d3 >> o4))
        return false;

    if (d1 != '.' || d2 != '.' || d3 != '.')
        return false;

    if (!validOctet(o1) || !validOctet(o2) ||
        !validOctet(o3) || !validOctet(o4))
        return false;

    ip = IP(o1, o2, o3, o4);
    return true;
}

bool validPrefix(int p)
{
    return p >= 1 && p <= 30;
}

uint32_t prefixToMask(int prefix)
{
    uint32_t mask = 0;

    for (int i = 31; i >= 32 - prefix; i--)
    {
        mask |= (1u << i);
    }

    return mask;
}

bool validMask(uint32_t mask)
{
    bool zeroFound = false;

    for (int i = 31; i >= 0; i--)
    {
        bool bit = (mask >> i) & 1;

        if (bit == 0)
            zeroFound = true;
        else if (zeroFound)
            return false;
    }
    return true;
}

int countBits(uint32_t x)
{
    int cnt = 0;
    while (x)
    {
        cnt += x & 1;
        x >>= 1;
    }
    return cnt;
}

class Subnet
{
private:
    IP ip;
    IP mask;

public:
    Subnet(const IP &ip, const IP &mask)
    {
        this->ip = ip;
        this->mask = mask;
    }

    IP network() const
    {
        return IP(ip.get() & mask.get());
    }

    IP broadcast() const
    {
        return IP(network().get() | ~mask.get());
    }

    IP firstHost() const
    {
        return IP(network().get() + 1);
    }

    IP lastHost() const
    {
        return IP(broadcast().get() - 1);
    }

    IP wildcard() const
    {
        return IP(~mask.get());
    }

    int prefix() const
    {
        return countBits(mask.get());
    }

    int usableHosts() const
    {
        int p = prefix();
        if (p >= 31)
            return 0;
        return (1 << (32 - p)) - 2;
    }

    void print() const
    {
        cout << "\nAddress:   " << ip.toDecimal() << "\n";
        cout << "Netmask:   " << mask.toDecimal() << " /" << prefix() << "\n";
        cout << "Wildcard:  " << wildcard().toDecimal() << "\n";
        cout << "----------------------------------\n";
        cout << "Network:   " << network().toDecimal() << "\n";
        cout << "Broadcast: " << broadcast().toDecimal() << "\n";
        cout << "HostMin:   " << firstHost().toDecimal() << "\n";
        cout << "HostMax:   " << lastHost().toDecimal() << "\n";
        cout << "Hosts:     " << usableHosts() << "\n";
        cout << "Class:     " << ip.getClass() << "\n";
        cout << "Type:      " << (ip.isPrivate() ? "Private" : "Public") << "\n";
    }
};

int main()
{
    IP ip, mask;
    string input;

    while (true)
    {
        cout << "Vendos IP (x.x.x.x): ";
        cin >> input;
        if (parseIP(input, ip))
            break;
        cout << "IP e pavlefshme!\n";
    }

    while (true)
    {
        cout << "Vendos subnet mask : ";
        cin >> input;

        if (input[0] == '/')
        {
            int p = stoi(input.substr(1));
            if (!validPrefix(p))
            {
                cout << "Prefix duhet 1-30\n";
                continue;
            }
            mask = IP(prefixToMask(p));
            break;
        }

        IP temp;
        if (parseIP(input, temp) && validMask(temp.get()))
        {
            mask = temp;
            break;
        }

        cout << "Subnet mask e pavlefshme!\n";
    }

    Subnet subnet(ip, mask);
    subnet.print();

    return 0;
}
