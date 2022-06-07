#ifndef Gyeongje_Data
#define Gyeongje_Data
#define MAX_WORD 2478
#define _CRT_SECURE_NO_WARNINGS
typedef unsigned char BYTE;
#include <queue>	
#include <list>
#include <string>
#include <vector>
using namespace std;
const bool ERROR = false;

#define ARCHIVE_NAME "archive.bin" 
#pragma pack(push, 1)
#pragma pack(pop)

class tree
{
public :
	tree(){}
	tree(BYTE name_, int freq_) { name = name_; freq = freq_;  left_child = list<tree>(); right_child = list<tree>(); }
	//data

	BYTE name;
	int freq;	

	//child
	list<tree>  left_child, right_child;
};

class code
{
public:

	//data
	BYTE name;
	string huffcode;	
};

struct Mycomp_tree {
	bool operator()(const tree& l, const tree& r) {
		return l.freq > r.freq;
	}
};

struct Mycomp_code {
	bool operator()(const code& l, const code& r) {
		return l.huffcode > r.huffcode;
	}
};

//Archive
typedef struct _ARCHIVE_HEADER {
	uint16_t magic;
	uint16_t version;
} ARCHIVE_HEADER, * PARCHIVE_HEADER;

typedef struct _FILE_DESC {
	char     name[256];
	uint32_t size;
	uint32_t dataOffset;
} FILE_DESC, * PFILE_DESC;

typedef struct _FILE_NODE {
	struct _FILE_NODE* next;
	FILE_DESC desc;
} FILE_NODE, * PFILE_NODE;

typedef struct _ARCHIVE {
	ARCHIVE_HEADER header;
	FILE* fp;
	FILE_NODE fileList;
} ARCHIVE, * PARCHIVE;

//archive
PARCHIVE initialize();
void finalize(PARCHIVE);
bool isExist(PARCHIVE, char*);
int append(PARCHIVE, char*);
void file_list(PARCHIVE);
int extract(PARCHIVE, char*);
uint32_t getFileSize(FILE*);

/// <ENCODE>
string search_code(vector<code>&, BYTE);
bool cal_frequency(string, int*);
void make_node(int*, priority_queue<tree, vector<tree>, Mycomp_tree>&);
void make_tree(priority_queue<tree, vector<tree>, Mycomp_tree>&);
void inorder(const tree&, priority_queue<code, vector<code>, Mycomp_code>&, string);
bool make_code(const priority_queue<tree, vector<tree>, Mycomp_tree>&, priority_queue<code, vector<code>, Mycomp_code>&);
bool huffman_encode(string, priority_queue<code, vector<code>, Mycomp_code>&);
void convert_binary(string, priority_queue<code, vector<code>, Mycomp_code>&);
/// </ENCODE>

/// <DECODE>
bool huffman_decode(string);
bool decode_search_code(vector<code>&, string&, BYTE*);
bool MySort(const code&, const code&);
/// </DECODE>

int intro();

#endif