#include <iostream>
#include <stdio.h>
#include "str.h"
#include <fstream>
#include <sys/stat.h>
#include <time.h>
// std namespace use defn's
using std::cerr;
using std::cin;
using std::cout;
using std::fstream;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::endl;
using std::flush;

typedef enum error_state
{
	BST_VALUE_ALREADY_PRESENT = -5,
	BST_VALUE_NOT_FOUND,
	BST_COULD_NOT_OPEN_FILE,
	BST_SOMETHING_WENT_WRONG
}enum_error_state;
typedef struct bst_node
{
	int key;     // Key value
	long l;      // File offset of left child node
	long r;      // File offset of right child node
}bst_node_t;

typedef struct bst_node_location
{
	bst_node_t located_node;
	long location;
	bool right;
}bst_node_location_t;

typedef struct bst_printqueue_node
{
	bst_node_t node;
	bool level_change;
	long location;
	struct bst_printqueue_node *next;
}bst_printqueue_node_t;

typedef struct print_node
{
	bst_node_t node;
	long location;
	struct print_node* next;
}print_node_t;

string *pIndex_file_name;
int num_nodes;

bst_printqueue_node_t *phead;
bst_printqueue_node_t *ptail;

print_node_t *pLevel1 , *pLevel2;

void add_to_queue(bst_node_t node, bool level_change , long location)
{	
	bst_printqueue_node_t *newNode = (bst_printqueue_node_t *)malloc(sizeof(bst_printqueue_node_t));
	newNode->location = location;
	memcpy(&newNode->node,&node,sizeof(bst_node_t));
	newNode->level_change = level_change;
	newNode->next = NULL;
	if(phead)
	{
		ptail->next = newNode;
		ptail = newNode;
	}
	else
	{
		phead = newNode;
		ptail = phead;
	}
}
void remove_from_queue(void)
{
	bst_printqueue_node_t *delNode;
	if(phead)
	{
		delNode = phead;
		phead = phead->next;
		if(phead == NULL)
			ptail = NULL;
		free(delNode);
	}
}

bst_printqueue_node_t* get_head_of_queue(void)
{
	return phead;
}

void print_queue(void)
{
	bst_printqueue_node_t*	currentNode = get_head_of_queue();
	while(currentNode)
	{
		//cout << currentNode->node.key << endl;
		currentNode = currentNode->next;
	}
}

void print_bst(void)
{
	bst_printqueue_node_t parent_node,child_node;
	bst_printqueue_node_t *qhead , *qParent;
	ifstream indexfile;
	int level_count = 1;
	bool level_change_set = true , level_change_left_shft = false;

	indexfile.open(pIndex_file_name->operator char *(),ios::in|ios::binary);
	if(indexfile.is_open())
	{
		indexfile.read((char*)&parent_node.node,sizeof(bst_node_t));		
		add_to_queue(parent_node.node,true,0);
		parent_node.level_change = true;

		while(get_head_of_queue() != NULL)
		{
			qParent = get_head_of_queue();
			memcpy(&parent_node,qParent,sizeof(bst_printqueue_node_t));
			
			if(parent_node.node.l != -1)
			{
				level_change_set = false;
				indexfile.seekg(parent_node.node.l,ios::beg);
				child_node.location = indexfile.tellg();
				indexfile.read((char*)&child_node.node,sizeof(bst_node_t));
		
				if(parent_node.level_change == true)
					child_node.level_change = true;
				else
					child_node.level_change = false;
				/*If the previous level node was the last the level changer must shift.*/
				if(level_change_left_shft)
				{	
					child_node.level_change = true;
					level_change_left_shft = false;
				}
				add_to_queue(child_node.node,child_node.level_change,child_node.location);
			}
			memset(&child_node,0,sizeof(bst_printqueue_node_t));
			if(parent_node.node.r != -1)
			{
				indexfile.seekg(parent_node.node.r,ios::beg);
				child_node.location = indexfile.tellg();
				indexfile.read((char*)&child_node.node,sizeof(bst_node_t));
		
				if(parent_node.level_change == true)
				{
					if(level_change_set == true)
						child_node.level_change = true;
					else
						child_node.level_change = false;
				}
				else
					child_node.level_change = false;
				/*If the previous level node was the last the level changer must shift.*/
				if(level_change_left_shft)
				{	
					child_node.level_change = true;
					level_change_left_shft = false;
				}
				
				add_to_queue(child_node.node,child_node.level_change,child_node.location);
				level_change_set = false;
			}			
			qhead = get_head_of_queue();
			
			//TODO: if a parent has level change but no children the level change of next node must be set.
			if(level_change_set == true)				
				level_change_left_shft = true;
			
			if(qhead != NULL)
			{
				if(qhead->level_change == true)
					cout << endl << level_count++ << ":";
				cout << " " << qhead->node.key << "/" << qhead->location;
				remove_from_queue();
			}
			level_change_set = true;
		}
		indexfile.close();
	}
	cout << endl;
}

void intialize(void)
{
	struct 	stat st;
	fstream f;
	/*Remove the index file if it exists*/
	remove(pIndex_file_name->operator char *());
	num_nodes = 0;
	if(stat(pIndex_file_name->operator char *(),&st)!= 0)
	{
	//	f.open( pIndex_file_name->operator char *(), ios::out);
	//	f << flush;
	//	f.close();
	}
	phead = NULL;
	ptail = NULL;
	pLevel1 = pLevel2 = NULL;
}
void read_node_from_file(bst_node_location_t *node)
{
	ifstream indexfile;
	indexfile.open(pIndex_file_name->operator char *(),ios::in|ios::out|ios::binary);
	if(indexfile.is_open())
	{
		indexfile.seekg(node->location,ios::beg);
		indexfile.read((char*)&node->located_node,sizeof(bst_node_t));
		indexfile.close();
	}
	else
		cout<< "Could not open file"<< endl; 	
}

void update_node(bst_node_location_t *inNode)
{
	ofstream indexfile;
	long node_location = num_nodes*sizeof(bst_node_t);

	indexfile.open(pIndex_file_name->operator char *(),ios::out|ios::in|ios::binary);
	if(indexfile.is_open())
	{
		if(inNode->right == true)
			inNode->located_node.r = node_location;
		else
			inNode->located_node.l = node_location;
		indexfile.seekp(inNode->location,ios::beg);
		indexfile.write((char*)&inNode->located_node,sizeof(bst_node_t));
		indexfile << flush;
		indexfile.close();
		if(indexfile.fail())
			cout << "update_node failed!!" << endl;
	}
}
void add_node(int newVal)
{
	bst_node_t newNode;
	ofstream indexfile;
	/*Initialize new node*/
	newNode.key = newVal;
	newNode.l = -1;
	newNode.r = -1;
	indexfile.open(pIndex_file_name->operator char *(),ios::app|ios::in|ios::binary);
	if(indexfile.is_open())
	{
		indexfile.seekp(0,ios::end);
		indexfile.write((char*)&newNode,sizeof(bst_node_t));
		indexfile << flush;
		indexfile.close();
		num_nodes++;
		if(indexfile.fail())
			cout << "add_node failed!!" << endl;
	}
}


void load_level(bst_node_location *pRead_node,int level)
{
	print_node_t * pCurrent_node;
	print_node_t *newNode = (print_node_t *)malloc(sizeof(print_node_t));
	memcpy(&newNode->node,&pRead_node->located_node,sizeof(bst_node_t));
	newNode->location = pRead_node->location;
	newNode->next = NULL;
	
	if(level == 1)
		pCurrent_node = pLevel1;
	else
		pCurrent_node = pLevel2;

	if(pCurrent_node == NULL)
	{
		if(level == 1)
			pLevel1 = newNode;
		else
			pLevel2 = newNode;
	}
	else
	{
		while(pCurrent_node->next != NULL)
			pCurrent_node = pCurrent_node->next;
		pCurrent_node->next = newNode;
	}
}

void switch_levels(void)
{
	pLevel1 = pLevel2;
	pLevel2 = NULL;
}

void delete_level(void)
{
	print_node_t * pCurrent_node , *delNode;
	pCurrent_node = pLevel1;
	pLevel1 = NULL;
	while(pCurrent_node!= NULL)
	{		
		delNode = pCurrent_node;
		pCurrent_node = pCurrent_node->next;
		if(delNode)
			free(delNode);
	}
}
print_node_t* get_level_head(void)
{
	return pLevel1;
}

void print_level(int level)
{
	print_node_t *pCurrent = get_level_head();	
	cout << endl;
	cout << level << ":";
	while(pCurrent)
	{		
		cout << " ";		
		cout << pCurrent->node.key<<"/"<< pCurrent->location;
		pCurrent = pCurrent->next;
	}
}

void load_second_level(void)
{
	long current_offset;
	bst_node_location read_node;
	print_node_t *pCurrent = get_level_head();
	while(pCurrent)
	{
		current_offset = pCurrent->node.l;
		if(current_offset != -1)
		{
			read_node.location = current_offset;
			read_node_from_file(&read_node);
			load_level(&read_node,2);
		}
		current_offset = pCurrent->node.r;
		if(current_offset != -1)
		{
			read_node.location = current_offset;
			read_node_from_file(&read_node);
			load_level(&read_node,2);
		}
		pCurrent = pCurrent->next;
	}
}
void print_tree(void)
{
	bst_node_location read_node;
	long current_offset = 0;
	int level = 1;
	read_node.location = current_offset;
	read_node_from_file(&read_node);
	load_level(&read_node,1);
	
	while(get_level_head() != NULL)
	{
		print_level(level);
		load_second_level();
		delete_level();
		switch_levels();
		level++;
	}
	cout << endl;
}

void find_node_in_tree(int val)
{
	ifstream indexfile;
	long currentoffset = 0;
	bst_node_t node;

	indexfile.open(pIndex_file_name->operator char *(),ios::in|ios::binary);
	if(indexfile.is_open())
	{
		while(1)
		{
			/*Read the node from the file*/
			indexfile.read((char*)&node,sizeof(bst_node_t));
			/*If value already present*/
			if(node.key == val)
			{
				cout << "Record "<< val<<" exists." << endl;
				return;
			}
			else 
			if(node.key > val) /*Value is less than node key*/
			{ 
				if(node.l != -1)
					indexfile.seekg(node.l,ios::beg);
				else
				{
					cout << "Record "<<val<<" does not exist." << endl;
					break;
				}
			}
			else /*Value is greater than node key*/
			{
				if(node.r != -1)
					indexfile.seekg(node.r,ios::beg);
				else
				{
					cout << "Record "<<val<<" does not exist." << endl;
					break;
				}
			}
			/*Store the current location of the file pointer.*/
			currentoffset = indexfile.tellg();
			/*Error check to prevent read beyond end of file.*/
			if(indexfile.fail())
			{
				cout << "find_node_in_tree failed!!" << endl;
			}
		}
		indexfile.close();
	}
	else
		cout << "find_node_in_tree Could not open file!!" << endl;
}
long find_parent(bst_node_location_t* outNode,int val)
{
	ifstream indexfile;
	long currentoffset = 0 , retVal = BST_SOMETHING_WENT_WRONG;
	indexfile.open(pIndex_file_name->operator char *(),ios::in|ios::binary);

	if(indexfile.is_open())
	{
		indexfile.seekg(0,ios::beg);
		while(1)
		{
			/*Read the node from the file*/
			indexfile.read((char*)&outNode->located_node,sizeof(bst_node_t));
			outNode->location = currentoffset;
			/*If value already present*/
			if(outNode->located_node.key == val)
			{
				retVal = BST_VALUE_ALREADY_PRESENT;
				break;
			}
			else 
			if(outNode->located_node.key > val) /*Value is less than node key*/
			{ 
				if(outNode->located_node.l != -1)
					indexfile.seekg(outNode->located_node.l,ios::beg);
				else
				{
					retVal = currentoffset;
					outNode->right = false;
					break;
				}
			}
			else /*Value is greater than node key*/
			{
				if(outNode->located_node.r != -1)
					indexfile.seekg(outNode->located_node.r,ios::beg);
				else
				{
					retVal = currentoffset;
					outNode->right = true;
					break;
				}
			}
			/*Store the current location of the file pointer.*/
			currentoffset = indexfile.tellg();
			/*Error check to prevent read beyond end of file.*/
		}
		indexfile.close();
	}
	else
		retVal = BST_COULD_NOT_OPEN_FILE;
	return retVal;
}
void add_to_tree(int newVal)
{
		bst_node_location_t outNode;
		long parent_location = 0;	
		memset(&outNode,0,sizeof(bst_node_location_t));

		if(num_nodes)
		{
			//cout<<"Not first node!!"<<endl;
			parent_location = find_parent(&outNode,newVal);
			/*Value already present*/
			if(parent_location == BST_VALUE_ALREADY_PRESENT)
			{
				//cout << "Value already present!!" << endl;
			}
			else
			if(parent_location >= 0)
			{
				//cout << "Found parent, add new node at end and update the l/r offset of the parent."<< endl;
				update_node(&outNode);
				add_node(newVal);
			}
			else /*error condition*/
				cout << "add_to_tree Error code : !!"<< parent_location << endl;
		}
		else
		{
			add_node(newVal);
		}		
}

int main(int argc, char* argv[])
{
	string current_line;
	string command_token[2];	
	pIndex_file_name = new string(argv[1]);
	int num_commands = 0;
	clock_t start, finish;
        double  duration = 0;


	intialize();
	while(1)
	{
		cin >> current_line;
		current_line.token(command_token,2);
		if(command_token[0] == "add")
		{
			//cout << "Add command!!" << endl;
			add_to_tree(command_token[1].operator int());
		}
		else
		if(command_token[0] == "find")
		{
			//cout << "Find command!!" << endl;
			num_commands++;
			start = clock();
			find_node_in_tree(command_token[1].operator int());
			finish = clock();
			duration += (double)(finish - start) / CLOCKS_PER_SEC;
		}
		else
		if(command_token[0] == "end")
		{
			//cout << "End command!!"<< endl;
			break;
		}
		else
		if(command_token[0] == "addq")
		{
			bst_node_t node;
			node.key = command_token[1].operator int();
			node.r = -1;
			node.l = -1;
			add_to_queue(node,true,0);
			//cout << "Added!!"<< endl;
		}
		else
		if(command_token[0] == "remq")
		{
			remove_from_queue();
			//cout << "Removed!!"<< endl;
		}
		else
		if(command_token[0] == "printq")
		{
			print_queue();
			//cout << "Print done!!"<< endl;
		}
		else
		if(command_token[0] == "print")
		{
			print_tree();
			//cout << "Print done!!"<< endl;
		}
		else
		{
			cout << "Invalid Command!!"<< endl;
		}
	}

	printf( "Sum : %2.6f seconds\n", duration );
	printf( "Avg : %2.6f seconds\n", duration/num_commands);
	return 0;
}

