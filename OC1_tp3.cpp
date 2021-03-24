#include <iostream>
#include <string>
#include <bitset>
#include <cmath>
#include <list>
#include <fstream>

using namespace std;

struct instructions{
	int N;
	int op;
	string data = "0";
	string str;
};

struct block{
	bool v;
	bool dirty;
	int tag;
	int data;
	int word0; //32 bit -> 4 words
	int word1;
	int word2;
	int word3;

};

struct Memory{
	int mem_data[1024];   // 4096/4
	block cache[64]; // 256/4
	int hits;
	int misses;
	int reads;
	int writes;
	Memory();
	void write(int,int,int,string);
	bool read(int,int,int,int,int&);
	void write_back(int);
};

Memory::Memory(){
	hits = misses = reads = writes = 0;
	for(int i = 0; i<256;i++){
	mem_data[i] = mem_data[i+1] = mem_data[i+2] = mem_data[i+3] = i;
	if(i<64){
		cache[i].v = false;
		cache[i].dirty = false;
		cache[i].tag = 0;
		data = 0;
		cache[i].word0 = cache[i].word1 = cache[i].word2 = cache[i].word3 = 0;
		}
	}
}
void Memory::write_back(int i){
	int t = cache[idx].tag;
		int addr = t*256 + idx*4;
		mem_data[addr]   = cache[idx].word0;
		mem_data[addr+1] = cache[idx].word1;
		mem_data[addr+2] = cache[idx].word2;
		mem_data[addr+3] = cache[idx].word3;
		cache[idx].dirty = false;
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
void Memory::write(int tg, int idx, int off, string data){
	
	this->writes++;

	if(this->cache[idx].v == false || cache[idx] != tag){ //buscar valor na memoria;
		mem_search(tg,int idx);
	}
	if(this->cache[idx].dirty = true){	//atualizar valor na memoria antes de escrever;
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
		this->hits ++;
		return true;
	}
	if(cache[idx].v == true && cache[idx].tag != tg){
		if(cache[idx].dirty == false){ //busca na memoria
		mem_search(tg,idx);
		val = cache[idx].word0;
		this->misses++;
		return false;
		}else{ //atualiza a memoria
			write_back(idx);
			mem_search(tg,idx);
			val = cache[idx].word0;
			this->misses++;
			return false; 
		}
	}
}

int main(){
	int adress, op_bit,tag, index,offset, value,d ,n_access = 0;
	bool hit = false;
	Memory mem;
	string writing_data, bin_adress;
	list<instructions> input_data;
	instructions inst;

	while(scanf("%d %d", &adress,&op_bit) != EOF){
		bin_adress = bitset<32>(adress).to_string();
		// cout<<"bin_adress = "<<bin_adress<<endl;
		tag = (int) (bitset<22>(bin_adress.substr(0,21)).to_ulong());    // 22 bits tag
		index = (int) (bitset<6>(bin_adress.substr(22,27)).to_ulong());  // 6  bits indice
		offset = (int) (bitset<4>(bin_adress.substr(28,31)).to_ulong()); // 4  bits offset
		// cout<<"tag = "<<tag<<"\nidx = "<<index<<"\noffset = "<<offset<<endl;
		if(op_bit){
			getline(cin,writing_data);
			d = (int)(bitset<32>(writing_data).to_ulong());
			mem.write(tag,index,offset,d);
			inst.data = writing_data;
			inst.srt = "W";
		}else{
			n_acess++;
			if(mem.read(tag,index,offset,value)){
				inst.str = "HIT";
			}else{
				inst.str = "MISS";
			}
		}
	}

		//salvar os dados de entrada
		inst.N = adress;
		inst.op = op_bit;
		input_data.insert(inst);

		// escrever no arquivo
		ofstream out_file;
		out_file.open("result.txt");
		cout<<"READS: "<<mem.reads<<endl;
		cout<<"WRITES: "<<mem.writes <<endl;
		cout<<"HITS: "<<mem.hits<<endl;
		cout<<"MISSES: "<<mem.misses<<endl;
		cout<<"HIT RATE: "<<mem.hits/n_access <<endl;
		cout<<"MISS RATE: "<<mem.misses/n_access<<endl;

		list::iterator it;
		for(it = inst.begin();it != inst.end();it++){
			cout<<it*.N<<" "<<it*.op<<" ";
			if(it*.data != "0") cout<<it*.data<<" ";
			cout<<it*.str<<endl;
		}

		out_file.close();

	return 0;
}