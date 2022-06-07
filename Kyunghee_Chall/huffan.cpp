#ifndef Gyeongje
#define Gyeongje
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "Data.h"
#include <ctime>
#include <cstdio>
#include <algorithm>
using namespace std;

string search_code(vector<code>& v, BYTE buffer)
{
	for (code item : v) {
		if (item.name == buffer)
			return item.huffcode;
	}
	return string("NULL");
}

bool cal_frequency(string filename, int* freq)
{
	FILE* file = fopen(filename.c_str(), "rb");
	int size = 0;

	if (file == NULL) {
		//printf("File open error!\n");
		return ERROR;
	}
	
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	if (size >= 5000)
		exit(0);
	
	fseek(file, 0, SEEK_SET);

	BYTE index;

	while (fscanf(file, "%c", &index) != EOF) {
		++freq[index];
	}

	fclose(file);
	return true;
}

void make_node(int* freq, priority_queue<tree, vector<tree>, Mycomp_tree>& pq)
{
	for (int i = 0; i < 256; i++) {
		tree item(i, freq[i]);
		pq.push(item);
	}

	return;
}

void make_tree(priority_queue<tree, vector<tree>, Mycomp_tree>& pq)
{
	while (pq.size() >= 2)
	{
		tree left = pq.top();
		pq.pop();
		tree right = pq.top();
		pq.pop();

		tree parent;
		parent.left_child.push_back(left);
		parent.right_child.push_back(right);
		parent.freq = left.freq + right.freq;

		pq.push(parent);
	}
	return;
}

bool make_code(const priority_queue<tree, vector<tree>, Mycomp_tree>& pq, priority_queue<code, vector<code>, Mycomp_code>& huffcode)
{

	inorder(pq.top(), huffcode, "");
	return true;
}

void inorder(const tree& root, priority_queue<code, vector<code>, Mycomp_code>& huffcode, string cur_code)
{

	if (!root.left_child.empty())
		inorder(root.left_child.front(), huffcode, cur_code + "0");

	if (root.left_child.empty() && root.right_child.empty()) {
		code item;

		item.name = root.name;
		item.huffcode = cur_code;
		huffcode.push(item);
	}

	if (!root.right_child.empty())
		inorder(root.right_child.front(), huffcode, cur_code + "1");

	return;
}

bool huffman_encode(string file, priority_queue<code, vector<code>, Mycomp_code>& huffcode)
{
	int freq[256] = { 0 };
	priority_queue<tree, vector<tree>, Mycomp_tree>  pq;

	if (cal_frequency(file, freq) == ERROR)
		return ERROR;

	make_node(freq, pq);
	make_tree(pq);
	make_code(pq, huffcode);

	convert_binary(file, huffcode);

	return true;
}

void convert_binary(string filename, priority_queue<code, vector<code>, Mycomp_code>& huffcode)
{
	FILE* readfile = fopen(filename.c_str(), "rb");
	int ext_size = filename.substr(filename.find_last_of(".")).length();
	filename.erase(filename.length() - ext_size, ext_size);
	string savefilename = filename + ".kzip";

	FILE* writefile = fopen(savefilename.c_str(), "wb");

	int dummy = 0;
	fprintf(writefile, "%c", dummy);
	fprintf(writefile, "%d", (int)huffcode.size());

	int idx = 0;
	vector<code> v(huffcode.size());

	while (!huffcode.empty())
	{
		code item = huffcode.top();
		huffcode.pop();
		v[idx++] = item;

		fprintf(writefile, "%c", item.name);

		fprintf(writefile, "%c", (char)item.huffcode.length());

		BYTE buffer = 0;
		int msb = -1;

		for (unsigned int i = 0; i < item.huffcode.length(); ++i)
		{
			if (msb == 7) {
				fprintf(writefile, "%c", buffer);
				buffer = 0;
				msb = -1;
			}

			buffer = buffer << 1;
			buffer = buffer | item.huffcode[i] - '0';
			++msb;
		}

		if (msb != -1) {
			while (msb != 7) {
				buffer = buffer << 1;
				msb++;
			}
			fprintf(writefile, "%c", buffer);
		}
	}
	

	BYTE word;
	BYTE buffer = 0;
	int msb = -1;	

	while (fscanf(readfile, "%c", &word) != EOF) {

		string write_code = search_code(v, word);

		for (unsigned int i = 0; i < write_code.length(); ++i)
		{
			if (msb == 7) {
				fprintf(writefile, "%c", buffer);
				buffer = 0;
				msb = -1;
			}

			buffer = buffer << 1;
			buffer = buffer | write_code[i] - '0';
			++msb;

		}
	}
	//last byte
	int lastbit = msb;
	while (lastbit != 7) {
		buffer = buffer << 1;
		lastbit++;
	}

	fprintf(writefile, "%c", buffer);
	fseek(writefile, 0, SEEK_SET);
	fprintf(writefile, "%c", (char)msb);

	fclose(readfile);
	fclose(writefile);

	return;
}
// END ENCODE

bool MySort(const code& l, const code& r)
{
	return l.huffcode < r.huffcode;
}

bool decode_search_code(vector<code>& v, string& s, BYTE* word)
{
	int left = 0;
	int right = v.size() - 1;

	while (left <= right)
	{
		int mid = (left + right) / 2;

		if (v[mid].huffcode < s)
			left = mid + 1;

		else if (s < v[mid].huffcode)
			right = mid - 1;

		else {
			*word = v[mid].name;
			return true;
		}
	}
	return false;
}

bool huffman_decode(string name)
{
	vector<code> v;

	FILE* file = fopen(name.c_str(), "rb");
	if (file == NULL)
	{
		return false;
	}
	string outfilename = name;
	outfilename.pop_back();
	outfilename.pop_back();
	outfilename.pop_back();
	outfilename += "txt";
	FILE* decoded = fopen(outfilename.c_str(), "wb");
	char msb;
	int codenum;

	fscanf(file, "%c", &msb);
	fscanf(file, "%d", &codenum);

	for (int i = 0; i < codenum; ++i)
	{
		code item;
		char validbit;

		fscanf(file, "%c", &item.name);

		fscanf(file, "%c", &validbit);

		BYTE buffer = 0;
		while (validbit > 0) 
		{
			fscanf(file, "%c", &buffer);

			for (int j = 7; j >= 0; --j) {
				if (validbit <= 0)
					break;
				char bitdata = (buffer >> j) & 1;

				item.huffcode.push_back(bitdata + '0');
				--validbit;
			}
		}
		v.push_back(item);
	}

	sort(v.begin(), v.end(), MySort);

	BYTE buffer, EOFcheck;
	int cnt = 0;
	string huffcode;
	while (fscanf(file, "%c", &buffer) != EOF)
	{
		if (fscanf(file, "%c", &EOFcheck) == EOF)
		{
			for (int i = 7; i >= 7 - msb; --i)
			{
				BYTE bitdata = (buffer >> i) & 1;
				huffcode.push_back(bitdata + '0');

				BYTE write_word = 0;
				bool found = false;


				found = decode_search_code(v, huffcode, &write_word);

				if (found)
				{
					fprintf(decoded, "%c", write_word);
					huffcode.clear();
					break;
				}
			}
		}
		else
		{
			fseek(file, -1, SEEK_CUR);
			for (int i = 7; i >= 0; --i)
			{
				BYTE bitdata = (buffer >> i) & 1;
				huffcode.push_back(bitdata + '0');

				BYTE write_word = 0;
				bool found = false;


				found = decode_search_code(v, huffcode, &write_word);

				if (found)
				{
					fprintf(decoded, "%c", write_word);
					huffcode.clear();
				}
			}
		}
	}

	fclose(file);
	fclose(decoded);

	return true;
}
//END DECODE

int intro()
{
	int input;

	cout << ">> 1. Compression  2. Decompression 3. Exit \n>> ";
	cin >> input;
	clock_t start;
	string name;
	if (input == 1)
	{
		priority_queue<code, vector<code>, Mycomp_code> huffcode;
		cout << ">> Enter the file that you want to compress\n>> ";
		cin >> name;

		start = clock();
		if (huffman_encode(name, huffcode) == false)
		{
			printf("File open error!\n");
			return false;
		}
		cout << "Compression Time : " << ((float)clock() - start) / CLOCKS_PER_SEC << "(s)\n\n";
	}
	else if (input == 2)
	{
		cout << ">> Enter the file that you want to decompress\n>> ";
		cin >> name;
		start = clock();
		if (huffman_decode(name) == false)
		{
			printf("File open error!\n");
			return false;
		}
		cout << "Decode time : " << ((float)clock() - start) / CLOCKS_PER_SEC << "(s)\n\n";
	}
	else if (input == 3)
	{
		cout << ">> Exit...\n";
		return false;
	}
	else
	{
		cout << ">> Please enter 0 to 3 \n";
		return false;
	}
	return true;
}
/*
int main()
{
	while (intro());
}
*/
#endif