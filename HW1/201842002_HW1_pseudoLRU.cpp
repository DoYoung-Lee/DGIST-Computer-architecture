/*
Filename : 201842002_HW1.cpp
Author : "Doyoung Lee" from "CSP lab, ICE, DGIST"
Description : C++ code for assignment #1 - Cache model and performance analysis
*/
#define WORKLOAD_FILENAME "400_perlbench.out"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include <iterator>
#include <cmath>

// Following to functions are from 'Silex' in stackoverflow.
const char* hex_char_to_bin(char c)
{
    // TODO handle default / error
    switch(toupper(c))
    {
        case '0': return "0000";
        case '1': return "0001";
        case '2': return "0010";
        case '3': return "0011";
        case '4': return "0100";
        case '5': return "0101";
        case '6': return "0110";
        case '7': return "0111";
        case '8': return "1000";
        case '9': return "1001";
        case 'A': return "1010";
        case 'B': return "1011";
        case 'C': return "1100";
        case 'D': return "1101";
        case 'E': return "1110";
        case 'F': return "1111";
    }
}

std::string hex_str_to_bin_str(const std::string& hex)
{
    // TODO use a loop from <algorithm> or smth
    std::string bin;
    for(unsigned i = 0; i != hex.length(); ++i)
       bin += hex_char_to_bin(hex[i]);
    return bin;
}

struct Node {
	unsigned int tag;
	bool dirty = false;
};

class pLRUCache {
	// Cache properties and stats.
	unsigned int capacity, associativity, block_size;
	unsigned int cache_access[2] = {0, 0}; // Read, write.
	unsigned int cache_miss[2] = {0, 0}; // Read, write.
	unsigned int cache_eviction[2] = {0, 0}; // Clean, dirty.
	// Nodes for LRU algorithm.
	int max_index;
	std::vector<std::list<Node>> node_h;
	std::vector<std::list<bool>> pLRU_table;
public:
	pLRUCache(int, int, int);
	void Access(const char*, std::string);
	void Statistics(std::string);
};

pLRUCache::pLRUCache(int c, int a, int s) {
	capacity = c;
	associativity = a;
	block_size = s;
	max_index = (capacity * 1024 / block_size) / associativity;
	if (max_index==0) {
		std::cout << "Configuration input is wrong." << std::endl;
		exit(0);
	}
	node_h.resize(max_index);
	pLRU_table.resize(max_index);
	for (int i=0; i<associativity; i++) {
		pLRU_table[i].assign(associativity-1, 0);
	}
}

void pLRUCache::Access(const char* cmd, std::string address) {
	unsigned int target_index;
	unsigned int target_tag;
	int address_size;
	int num_i = 0;
	int asdf = 0;
	bool cmd_i;
	if (cmd[0]=='R') { cmd_i = false; }
	else { cmd_i = true; }

	cache_access[cmd_i] += 1;
	// Convert address to tag.
	// Assume byte offset = 3 (64-bit system).
	address = address.erase(0, 2);
	address = hex_str_to_bin_str(address);
	
	int index_offset_size = std::log2(max_index);
	int block_offset_size = std::log2(block_size);
	address_size = address.length();
	address.erase(address_size - block_offset_size, block_offset_size); // 4 = length of byte offset + block offset.
	address_size -= block_offset_size;

	std::string str_tag = address;
	
	target_index = std::stoull(address.erase(0, address_size - index_offset_size), 0, 2);
	target_tag = std::stoull(str_tag.erase(address_size - index_offset_size, index_offset_size), 0, 2);
	
	// Find whether tag exists in cache.
	auto target_loc = node_h[target_index].begin();
	auto plru_loc = pLRU_table[target_index].begin();
	while (target_loc != node_h[target_index].end()) {
		if (target_loc->tag == target_tag) {
			break;
		} else {
			target_loc = std::next(target_loc, 1);
			num_i++;
		}
	}

	if (target_loc == node_h[target_index].end()) {
		// Not in cache
		cache_miss[cmd_i] += 1;
		if (node_h[target_index].size() == associativity) {
			// Cache index is full.
			// Then remove pLRU.
			num_i = 0;
			for (int i=0; i < std::log2(associativity); i++) {
				num_i += (int)(*plru_loc) * std::pow(2, std::log2(associativity) - i - 1);
				asdf = 2*asdf + 1 + (int)(*plru_loc);
				plru_loc = std::next(pLRU_table[target_index].begin(), asdf);
			}
			target_loc = std::next(node_h[target_index].begin(), num_i);
			cache_eviction[target_loc->dirty] += 1;
			target_loc->tag = target_tag;
			target_loc->dirty = cmd_i;
		} else {
			// Cache index is not full.
			// Then add value.
			node_h[target_index].push_back(Node{target_tag, cmd_i});
		}
	} else {
		// In cache
		target_loc->dirty |= cmd_i;
	}

	num_i += associativity - 1;
	// Upload pLRU location.
	while(num_i > 0) {
		asdf = (num_i % 2 == 1);
		num_i = (num_i - 1) / 2;
		plru_loc = std::next(pLRU_table[target_index].begin(), num_i);
		*plru_loc = asdf;
	}
}

void pLRUCache::Statistics(std::string path_out) {
	// Calculate miss rates
	float missr_r = 0.0;
	float missr_w = 0.0;
	if (cache_miss[0] != 0 && cache_miss[1] != 0) {
		missr_r = (float) cache_miss[0] / cache_access[0] * 100.0;
		missr_w = (float) cache_miss[1] / cache_access[1] * 100.0;
	}
	// Calculate checksum
	unsigned int checksum = 0;
	for (int i=0; i<max_index; i++) {
		auto temp_loc = node_h[i].begin();
		int debugging = 0;
		while (temp_loc != node_h[i].end()) {
			checksum = checksum ^ (((temp_loc->tag ^ i) << 1) | temp_loc->dirty);
			temp_loc = std::next(temp_loc, 1);
		}
	}
	path_out.erase(path_out.length() - 4, 4);
	path_out.append("_" + std::to_string(capacity) + "_" + std::to_string(associativity) + "_" + std::to_string(block_size) + ".out");
	std::ofstream writeFile(path_out.data());
	if( writeFile.is_open() ) {
		writeFile << "-- General Stats --\n";
		writeFile << "Capacity: " << capacity << "\n";
		writeFile << "Way: " << associativity << "\n";
		writeFile << "Block size: " << block_size << "\n";
		writeFile << "Total accesses: " << (cache_access[0] + cache_access[1]) << "\n";
		writeFile << "Read accesses: " << cache_access[0] << "\n";
		writeFile << "Write accesses: " << cache_access[1] << "\n";
		writeFile << "Read misses: " << cache_miss[0] << "\n";
		writeFile << "Write misses: " << cache_miss[1] << "\n";
		writeFile << "Read miss rate: " << missr_r << "%" << "\n";
		writeFile << "Write miss rate: " << missr_w << "%" << "\n";
		writeFile << "Clean evictions: " << cache_eviction[0] << "\n";
		writeFile << "Dirty evictions: " << cache_eviction[1] << "\n";
		writeFile << "Checksum: 0x" << std::hex << checksum << "\n";
		writeFile.close();
	}
}

int stocmd(std::string work, std::string *c, std::string *b) {

	std::stringstream ss_temp(work);
	ss_temp >> *c;
	ss_temp >> *b;
	return 0;
} 

int main(void) {
	std::string path_workload = WORKLOAD_FILENAME;
	std::string path_config = "config.txt";
	unsigned int config[3];
	// Read config file.
	// Config file 형태에 맞춰서 읽은 값을 저장하도록 수정해야 함.
	std::ifstream openFile(path_config.data());
	if( openFile.is_open() ) {
		std::string line;
		std::cout << "Configuration is :" << std::endl;
		for (int i=0; i<3; i++) {
			getline(openFile, line);
			std::cout << line << std::endl;
			config[i] = std::stoul(line);
		}
		openFile.close();
	}

	// Create LRU cache according to the configuration.
	pLRUCache cache = pLRUCache(config[0],config[1],config[2]);

	// Open test input file.
	// And loop while input commands are left.
	openFile.open(path_workload.data());
	if (openFile.is_open()) {
		std::string work_line;
		std::string cmd_type;
		std::string cmd_address;
		while (getline(openFile, work_line)) {
			stocmd(work_line, &cmd_type, &cmd_address);
			cache.Access(cmd_type.c_str(), cmd_address);
		}
	}

	// Print output file.
	cache.Statistics(path_workload);


	std::cout << "All process ended" << std::endl;

	return 0;
}