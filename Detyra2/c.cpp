#include <iostream>
#include <bitset>
#include <cstdint>
#include <string>

using namespace std;

class IP {
    uint32_t ip;
public:
    IP(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4) {
        ip = ((uint32_t)o1 << 24) | ((uint32_t)o2 << 16) | ((uint32_t)o3 << 8) | (uint32_t)o4;
    }
    //vendos oktetin e pare ne bitet me te majta
    IP(uint32_t ip): ip(ip) {}
    uint32_t get() const { return ip; }

    uint8_t get_octet(int n) const {
        return (ip >> (8 * (4 - n))) & 0xFF;
    }

    char get_class() const {
        uint8_t o1 = get_octet(1);
        if ((o1 & 0b10000000) == 0) return 'A';
        if ((o1 & 0b11000000) == 0b10000000) return 'B';
        if ((o1 & 0b11100000) == 0b11000000) return 'C';
        if ((o1 & 0b11110000) == 0b11100000) return 'D';
        return 'E';
    }

    bool is_private() const {
        uint8_t o1 = get_octet(1);
        uint8_t o2 = get_octet(2);
        return (o1 == 10) || (o1 == 172 && o2 >= 16 && o2 <= 31) || (o1 == 192 && o2 == 168);
    }

    string to_decimal() const {
        return to_string(get_octet(1)) + "." + to_string(get_octet(2)) + "." +
               to_string(get_octet(3)) + "." + to_string(get_octet(4));
    }

    string to_binary() const {
        return bitset<8>(get_octet(1)).to_string() + "." +
               bitset<8>(get_octet(2)).to_string() + "." +
               bitset<8>(get_octet(3)).to_string() + "." +
               bitset<8>(get_octet(4)).to_string();
    }
};

int count_mask_bits(uint32_t mask) {
    int cnt = 0;
    while (mask) {
        cnt += mask & 1;
        mask >>= 1;
    }
    return cnt;
}

int main() {
  
    IP ip(10, 12, 5, 33);
    IP mask(255, 255, 255, 240);

    uint32_t network = ip.get() & mask.get();
    uint32_t broadcast = network | ~mask.get();

    IP network_ip(network);
    IP broadcast_ip(broadcast);
    IP first_host(network + 1);
    IP last_host(broadcast - 1);
    IP wildcard(~mask.get());

    int prefix_len = count_mask_bits(mask.get());
    int total_hosts = (~mask.get()) + 1;
    int usable_hosts = total_hosts - 2;

    cout << "\nAddress:   " << ip.to_decimal() << "  " << ip.to_binary() << "\n";
    cout << "Netmask:   " << mask.to_decimal() << " = " << prefix_len << "  " << mask.to_binary() << "\n";
    cout << "Wildcard:  " << wildcard.to_decimal() << "  " << wildcard.to_binary() << "\n";
    cout << "=>\n";
    cout << "Network:   " << network_ip.to_decimal() << "/" << prefix_len << "  " << network_ip.to_binary() 
         << " (Class " << ip.get_class() << ")\n";
    cout << "Broadcast: " << broadcast_ip.to_decimal() << "  " << broadcast_ip.to_binary() << "\n";
    cout << "HostMin:   " << first_host.to_decimal() << "  " << first_host.to_binary() << "\n";
    cout << "HostMax:   " << last_host.to_decimal() << "  " << last_host.to_binary() << "\n";
    cout << "Hosts/Net: " << usable_hosts << "  " 
         << (ip.is_private() ? "(Private Internet)" : "(Public Internet)") << "\n";

    return 0;
}
