#include <iostream>
#include <string>
#include <bitset>
#include <cmath>
#include <list>
#include <fstream>

using namespace std;

struct Instruction{
	int N;
	int op;
	string data = "0";
	string str;
};

struct Block{
	bool v;
	bool dirty;
	int tag;
	int word0; //32 bit -> 4 words
	int word1;
	int word2;
	int word3;

};

struct Memory{
	int mem_data[1024];   // 4096/4
	Block cache[64]; // 256/4
	int hits;
	int misses;
	int reads;
	int writes;
	Memory();
	void write(int,int,int,int);
	bool read(int,int,int,int&);
	void mem_search(int, int);
	void write_back(int);
	// void print_cache();
};
/*
void Memory::print_cache(){
	cout<<"CACHE: \n";
	for(int i=0; i<64;i++){
		cout<<"{"<<i<<"}"<<" (V: "<<cache[i].v<<" Dy: "<<cache[i].dirty<<" T: "<<cache[i].tag<<" ";
		cout<<"-> "<<cache[i].word0<<" "<<cache[i].word1<<" "<<cache[i].word2<<" "<<cache[i].word3<<")"<<endl;
	}
}
*/
Memory::Memory(){
	hits = misses = reads = writes = 0;
	for(int i = 0; i<256;i++){
	mem_data[i] = mem_data[i+1] = mem_data[i+2] = mem_data[i+3] = i;
	if(i<64){
		cache[i].v = false;
		cache[i].dirty = false;
		cache[i].tag = 0;
		cache[i].word0 = cache[i].word1 = cache[i].word2 = cache[i].word3 = 0;
		}
	}
}
void Memory::write_back(int i){
	int t = cache[i].tag;
	int addr = t*256 + i*4;

	mem_data[addr]   = cache[i].word0;
	mem_data[addr+1] = cache[i].word1;
	mem_data[addr+2] = cache[i].word2;
	mem_data[addr+3] = cache[i].word3;
	cache[i].dirty = false;
}
void Memory::mem_search(int t, int i){
	int mem_address =  i*4 + t*256; // adress = index*2² + tag*2⁸
	cache[i].v = true;
	cache[i].tag = t;
	cache[i].word0 = mem_data[mem_address];
	cache[i].word1 = mem_data[mem_address+1];
	cache[i].word2 = mem_data[mem_address+2];
	cache[i].word3 = mem_data[mem_address+3];
}
void Memory::write(int tg, int idx, int off, int data){
	
	this->writes++;

	if(this->cache[idx].v == false || cache[idx].tag != tg){ //buscar valor na memoria;
		mem_search(tg,idx);
	}
	if(this->cache[idx].dirty == true){	//atualizar valor na memoria antes de escrever;
		write_back(idx);
	}
	// escrita;
	cache[idx].word0 = data; 
	cache[idx].dirty = true;
}
bool Memory::read(int tg, int idx, int off, int& val){
	
	this-> reads++;

	if(cache[idx].v == true && cache[idx].tag == tg){
		val = cache[idx].word0;
		return true;
	}

	if(cache[idx].v == true && cache[idx].tag != tg){
		if(cache[idx].dirty == false){ //busca na memoria
			mem_search(tg,idx);
			val = cache[idx].word0;
			return false;
		}else{ //atualiza a memoria
			write_back(idx);
			mem_search(tg,idx);
			val = cache[idx].word0;

			return false; 
		}
	}
	return false;
}

int main(){
	int address, op_bit,tag, index,offset, value,d ,n_access = 0;
	Memory mem;
	string writing_data, bin_address;
	list<Instruction> input_data;
	Instruction inst;
	bool hit;
	while(scanf("%d %d", &address,&op_bit) != EOF){
		bin_address = bitset<32>(address).to_string();
		//cout<<"adress"<<"bin_adress = "<<bin_adress<<endl;
		tag = (int) (bitset<22>(bin_address.substr(0,21)).to_ulong());    // 22 bits tag
		index = (int) (bitset<6>(bin_address.substr(22,27)).to_ulong());  // 6  bits indice
		offset = (int) (bitset<4>(bin_address.substr(28,31)).to_ulong()); // 4  bits offset
		// cout<<"{"<<count<<"}\n"<<"tag = "<<tag<<"\nidx = "<<index<<"\noffset = "<<offset<<endl;
		if(op_bit){
			getline(cin,writing_data);
			d = stoi(writing_data,0,2);
			mem.write(tag,index,offset,d);
			inst.data = writing_data;
			inst.str = "W";
		}else{
			n_access++;
			hit = mem.read(tag,index,offset,value);
			if(hit){
				inst.str = "HIT";
				mem.hits++;
			}else{
				inst.str = "MISS";
				mem.misses++;
			}
		}
		//salvar os dados de entrada
		inst.N = address;
		inst.op = op_bit;
		input_data.push_back(inst);
	}

		// escrever no arquivo
		ofstream out_file;
		out_file.open("results.txt");
		out_file<<"READS: "<<mem.reads<<" "<<endl;
		out_file<<"WRITES: "<<mem.writes <<endl;
		out_file<<"HITS: "<<mem.hits<<endl;
		out_file<<"MISSES: "<<mem.misses<<endl;
		out_file<<"HIT RATE: "<<(float)mem.hits/n_access<<endl;
		out_file<<"MISS RATE: "<<(float)mem.misses/n_access<<endl;
		out_file<<"\n \n";

		list<Instruction>::iterator it;
		for(it = input_data.begin();it != input_data.end();it++){
			out_file<<(*it).N<<" "<<(*it).op<<" ";
			if((*it).op){ out_file<<(*it).data<<" ";}
			out_file<<(*it).str<<endl;
		}
		// mem.print_cache();
		out_file.close();

	return 0;
}