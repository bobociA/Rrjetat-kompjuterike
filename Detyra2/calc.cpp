#include <iostream>
#include <bitset>
#include <cstdint>

using namespace std;

class IP {
private:
    uint32_t ip;

public:
    IP(uint8_t o1=0, uint8_t o2=0, uint8_t o3=0, uint8_t o4=0) {
        ip = ((uint32_t)o1 << 24) | ((uint32_t)o2 << 16) | ((uint32_t)o3 << 8) | ((uint32_t)o4);
    }

    IP(uint32_t ip) : ip(ip) {}

    uint32_t get_ip() const { return ip; }
    void set_ip(uint32_t ipVal) { ip = ipVal; }

    uint8_t get_octet(int n) const {
        if(n<1 || n>4) return 0;
        return (ip >> (8*(4-n))) & 0xFF;
    }

    char get_ip_class() const {
        uint8_t o1 = get_octet(1);
        if((o1 & 0b10000000) == 0) return 'A';
        if((o1 & 0b11000000) == 0b10000000) return 'B';
        if((o1 & 0b11100000) == 0b11000000) return 'C';
        if((o1 & 0b11110000) == 0b11100000) return 'D';
        return 'E';
    }

    bool is_private() const {
        uint8_t o1 = get_octet(1);
        uint8_t o2 = get_octet(2);
        if(o1==10) return true;
        if(o1==172 && o2>=16 && o2<=31) return true;
        if(o1==192 && o2==168) return true;
        return false;
    }

    friend ostream &operator<<(ostream &os, const IP &ip);
    friend ostream &operator<<(ostream &os, const struct IPBinaryView &view);
};

struct IPBinaryView {
    const IP &ip;
};

ostream &operator<<(ostream &os, const IP &ip) {
    os << (int)ip.get_octet(1) << "." << (int)ip.get_octet(2) << "." 
       << (int)ip.get_octet(3) << "." << (int)ip.get_octet(4);
    return os;
}

ostream &operator<<(ostream &os, const IPBinaryView &view) {
    uint32_t ip = view.ip.get_ip();
    os << bitset<8>((ip>>24)&0xFF) << "."
       << bitset<8>((ip>>16)&0xFF) << "."
       << bitset<8>((ip>>8)&0xFF) << "."
       << bitset<8>(ip&0xFF);
    return os;
}

IPBinaryView to_binary(const IP &ip) { return IPBinaryView{ip}; }

class Network {
private:
    IP ip_address;
    IP subnet_mask;

public:
    Network(IP ip, IP mask) : ip_address(ip), subnet_mask(mask) {}

    IP get_ip() const { return ip_address; }
    IP get_subnet_mask() const { return subnet_mask; }

    IP get_network_id() const { return IP(ip_address.get_ip() & subnet_mask.get_ip()); }
    IP get_broadcast_id() const { return IP(get_network_id().get_ip() | ~subnet_mask.get_ip()); }
    IP get_first_ip() const { return IP(get_network_id().get_ip() + 1); }
    IP get_last_ip() const { return IP(get_broadcast_id().get_ip() - 1); }
    int get_total_hosts() const { return (~subnet_mask.get_ip()) + 1; }
    int get_usable_hosts() const { return get_total_hosts() - 2; }

    int get_prefix_length() const {
        uint32_t mask = subnet_mask.get_ip();
        int count = 0;
        while(mask) { count += mask & 1; mask >>=1; }
        return count;
    }

    IP get_wildcard_mask() const { return IP(~subnet_mask.get_ip()); }

    bool is_ip_in_network(IP ip) const {
        return (ip.get_ip() & subnet_mask.get_ip()) == get_network_id().get_ip();
    }

    void print_subnets(int subnet_bits) const {
        int hosts_per_subnet = (~subnet_mask.get_ip() >> subnet_bits) + 1;
        int total_subnets = (1 << subnet_bits);
        IP subnet_id = get_network_id();

        cout << "\nSubnet\t\tUsable Hosts\t\tBroadcast\n";
        cout << "-----------------------------------------------------------\n";
        for(int i=0;i<total_subnets;i++){
            cout << subnet_id << "\t"
                 << IP(subnet_id.get_ip()+1) << " - " << IP(subnet_id.get_ip()+hosts_per_subnet-2) << "\t"
                 << IP(subnet_id.get_ip()+hosts_per_subnet-1) << "\n";
            subnet_id.set_ip(subnet_id.get_ip()+hosts_per_subnet);
        }
        cout << "\nTotal subnets: " << total_subnets 
             << "\nHosts per subnet: " << hosts_per_subnet-2 << "\n";
    }
};

int main() {
    IP ip1(192,168,1,10);
    IP ip2(192,168,2,20);
    IP mask(255,255,255,0);
    Network n(ip1, mask);

    cout << "IP Address:\t" << ip1 << "\t" << to_binary(ip1) << "\n";
    cout << "Network ID:\t" << n.get_network_id() << "\t" << to_binary(n.get_network_id()) << "\n";
    cout << "Broadcast:\t" << n.get_broadcast_id() << "\t" << to_binary(n.get_broadcast_id()) << "\n";
    cout << "Subnet Mask:\t" << n.get_subnet_mask() << " (/" << n.get_prefix_length() << ")\n";
    cout << "Wildcard Mask:\t" << n.get_wildcard_mask() << "\n";
    cout << "First Host:\t" << n.get_first_ip() << "\n";
    cout << "Last Host:\t" << n.get_last_ip() << "\n";
    cout << "Total Hosts:\t" << n.get_total_hosts() << "\n";
    cout << "Usable Hosts:\t" << n.get_usable_hosts() << "\n";
    cout << "Network Class:\t" << ip1.get_ip_class() << "\n";
    cout << "Is " << ip2 << " in the network? " << (n.is_ip_in_network(ip2) ? "Yes" : "No") << "\n";
    cout << "Is network private? " << (ip1.is_private() ? "Yes" : "No") << "\n";

    n.print_subnets(2); // example: 2 bits for subnetting
    return 0;
}
