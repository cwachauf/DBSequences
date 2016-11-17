//
//  main.cpp
//  DeBruijnSequence
//
//  Created by Christian Wachauf on 11.11.16.
//  Copyright © 2016 Christian Wachauf. All rights reserved.
//

#include <vector>
#include <list>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <iostream>
#include <math.h>
#include <algorithm>
#include <time.h>
#include <random>
#include <iterator>

using namespace std;
char letters[4] = {'A','C','G','T'};

std::random_device rd;
std::mt19937 g(rd());

/*class CDFS
 {
 private:
	vector<unsigned char> m_parents;
	vector<bool> visited;
 public:
	void DFS(CDBGraph& dbg,int index_starting_node);
 };*/

class CDBGraph
{
private:
    vector<string> m_tuples;
    vector<list<unsigned short> > m_adjacency_list;
    int m_order;
    int m_sequ_index;
    string m_sequence;
    string m_start_sequ;
    int m_curr_tuple_index; // current position in the graph...
    int m_curr_letter_index; // current letter (how many steps have been already taken ?)
    string m_final_tuple; // sequence of the final tuple (if you've reached it you're done...
    void CreateSequenceRecursively(char* curr_sequ,int curr_tuple_index, int curr_depth);
    
    vector<int> m_parents;
    vector<bool> m_visited;
    vector<int> m_depth; // used in depth-first search...
    
    void DFSRecursively(int curr_index,int index_parent,int curr_depth);
public:
    // setters and getters
    void SetOrder(int order){m_order=order;};
    
    int GetCurrTupleIndex(){return m_curr_tuple_index;};
    int GetFinalTupleIndex(){return GetIndexFromString(m_final_tuple);};
    
    int GetOrder(){return m_order;};
    
    // print information
    void PrintInfo();
    void PrintTuples();
    void PrintAdjacencyList();
    
    // functionality / the "meat"
    // creating graph
    void CreateTuples();
    void FullyConnectGraph();
    void FastFullyConnectGraph();
    
    void ReadStartSequence(char* filename);
    void ConsiderStartSequence();
    
    // helper functions (converting between index and name (e.g. "AAAAAC" and 000002)
    unsigned short GetIndexFromString(string node_name);
    string GetStringFromIndex(int node_index);
    
    list<unsigned short>& GetNeighbors(string node_a);
    list<unsigned short>& GetNeighbors(unsigned short node_index);
    
    int GetOutDegree(unsigned short node_index);
    int GetOutDegree(string node_name);
    
    bool HasEdge(string node_name_a, string node_name_b);
    bool HasEdge(unsigned short node_index_a,unsigned short node_index_b);
    
    void DeleteEdge(unsigned short node_index_a,unsigned short node_index_b);
    void DeleteEdge(string node_name_a,string node_name_b);
    
    void DeleteNode(unsigned short node_index_a);
    void DeleteNode(string node_name_a);
    
    void DeleteSequenceFromFile(char* filename);
    
    void CreateSequence(); //
    void DFS(int index_start); // depth-first search (start with "index_start");
    
    list<unsigned int> FindPath(int start_index, int final_index);
    bool HasPath(int start_index,int final_index);
};

bool CDBGraph::HasPath(int start_index,int final_index)
{
    DFS(start_index);
    return m_visited[final_index];
    
}

int CDBGraph::GetOutDegree(unsigned short node_index)
{
    return m_adjacency_list[node_index].size();
}

int CDBGraph::GetOutDegree(string node_name)
{
    return GetOutDegree(GetIndexFromString(node_name));
}

//int CDBGraph::GetInDegree(unsigned short node_index)
//{
//}

void CDBGraph::DeleteSequenceFromFile(char* filename)
{
    FILE* fp_sequence = fopen(filename,"rt");
    char* sequ_to_delete = new char[8064];
    fscanf(fp_sequence,"%s\n",sequ_to_delete);
    string stdel = string(sequ_to_delete);
    cout << "sequence to delete: " << endl;
    cout << stdel << endl;
    vector<string> vec_of_strings;
    for(int i=0;i<stdel.length()-m_order+1;i++)
        vec_of_strings.push_back(stdel.substr(i,m_order));
    for(int i=0;i<vec_of_strings.size()-1;i++)
        DeleteEdge(vec_of_strings[i],vec_of_strings[i+1]);
    fclose(fp_sequence);
}

list<unsigned int> CDBGraph::FindPath(int start_index,int final_index)
{
    DFS(start_index);
    if(m_visited[final_index]==false)
        cout << "error, index can not be visited..." << endl;
    else {
        cout << "can be reached!" << endl;
        cout << "depth is: " << m_depth[final_index] << endl;
    }
    
    list<unsigned int> list_of_indices;
    int curr_index = final_index;
    do
    {
        list_of_indices.push_front(curr_index);
        curr_index = m_parents[curr_index];
    } while (curr_index!=start_index);
    
    list_of_indices.push_front(start_index);
    
    
    
    return list_of_indices;
}

void CDBGraph::DFS(int index_start)
{
    //cout << "starting DFS" << endl;
    // clear DFS-vectors:
    m_parents.clear();
    m_visited.clear();
    m_depth.clear();
    
    int num_nodes = pow(4,m_order);
    
    m_parents.reserve(num_nodes);
    m_visited.reserve(num_nodes);
    m_depth.reserve(num_nodes);
    
    for(int i=0;i<num_nodes;i++)
        m_visited[i] = false;
    
    DFSRecursively(index_start,-1,0);
    //cout << "finished with DFS" << endl;
}

void CDBGraph::DFSRecursively(int curr_index,int index_parent,int curr_depth)
{
    m_visited[curr_index] = true;
    m_parents[curr_index] = index_parent;
    m_depth[curr_index] = curr_depth;
    
    // obtain list of neighbors:
    list<unsigned short> nbs = GetNeighbors(curr_index);
    list<unsigned short>::iterator it_list;
    for(it_list = nbs.begin();it_list!=nbs.end();it_list++)
    {
        // check, whether the index has already been visited:
        if(m_visited[*it_list]==false)
            DFSRecursively(*it_list,curr_index,curr_depth+1);
    }
}

void CDBGraph::CreateSequence()
{
    char curr_sequence[4097];
    strcpy(curr_sequence,m_sequence.c_str());
    //cout << "current sequence: " << curr_sequence << endl;
    //cout << "current tuple: " << GetStringFromIndex(m_curr_tuple_index) << endl;
    //cout << "current letter index: " << m_curr_letter_index << endl;
    CreateSequenceRecursively(curr_sequence,m_curr_tuple_index,m_curr_letter_index);
}

void CDBGraph::CreateSequenceRecursively(char* curr_sequence,int curr_tuple_index,int curr_depth)
{
    if(curr_depth==4096)
    {
        if(curr_tuple_index==GetIndexFromString(m_final_tuple))
        {
            cout << "SUCCES" << endl;
           
            //cout << curr_sequence << endl;
            char str_to_search[16];
            //strcpy(str_to_search,"ACGTTGAAG");
           strcpy(str_to_search,"ACGTTG");
            char* ptr = strstr(curr_sequence,str_to_search);
            if(ptr)
            {
                cout << "pointer at: " << ptr-curr_sequence << endl;
                //cout << curr_sequence << endl;
                strcpy(str_to_search,"ACGTTGAAG");
                char* ptr2 = strstr(curr_sequence,str_to_search);
                printf("%.*s",50,ptr);
                if(ptr2)
                {
                    cout << "FOUND IT" << endl;
                    cout << curr_sequence;
                }
                // cout << "YEAHHHHH" << endl << endl << endl;
                
                
            }
            
        }
        //		else
        //			cout << "FAIL" << endl;
        
    }
    else
    {
        //	if(curr_depth>7550)
        //		cout << "current depth: " << curr_depth << endl;
        
        list<unsigned short> nbs = GetNeighbors(curr_tuple_index);
        vector<unsigned short> vec(nbs.begin(),nbs.end());
        shuffle(vec.begin(), vec.end(),g);
        
        for(int i=0;i<vec.size();i++)
        {
            //if((curr_tuple_index>7540)||(vec[i]!=GetIndexFromString(m_final_tuple)))
            //{
            
            
            //list<unsigned int> verts = FindPath(curr_tuple_index,GetIndexFromString(m_final_tuple));
       //     bool hp = HasPath(curr_tuple_index,GetIndexFromString(m_final_tuple));
            //if(curr_depth > 4090)
             //   cout << "curr_depth: " << curr_depth << endl;
           // if(curr_depth%100==0)
             //   cout << "curr_depth: " << curr_depth << endl;
            // get out path-length
            //		cout << verts.size() << endl;
            // are we still connected ?
            
            //	}
        //}
         //   if(hp==true)
           // {
                string next_string = GetStringFromIndex(vec[i]);
                
                curr_sequence[curr_depth] = *next_string.substr(0,1).c_str();
                m_adjacency_list[curr_tuple_index].remove(vec[i]);
                
                CreateSequenceRecursively(curr_sequence,vec[i],curr_depth+1);
                m_adjacency_list[curr_tuple_index].push_back(vec[i]);
            //}
            //else
            //{
             //   cout << curr_depth << endl;
            //}
            
       }
    }
}

void CDBGraph::ReadStartSequence(char* filename)
{
    //cout << filename << endl;
    FILE* fp_starting_sequ  = fopen(filename,"rt");
    if(fp_starting_sequ==NULL)
    {
        cout << "could not open file" << filename << endl;
    }
    char* loc_start_sequ =  new char[1024];
    fscanf(fp_starting_sequ, "%s\n",&loc_start_sequ[0]);
    fclose(fp_starting_sequ);
    
    m_start_sequ = string(loc_start_sequ);
    cout << m_start_sequ << endl;
}

void CDBGraph::ConsiderStartSequence()
{
    vector<string> vec_of_strings;
    for(int i=0;i<m_start_sequ.length()-m_order+1;i++)
        vec_of_strings.push_back(m_start_sequ.substr(i,m_order));
    
    for(int i=0;i<vec_of_strings.size()-1;i++)
    {
        DeleteEdge(vec_of_strings[i],vec_of_strings[i+1]);
        cout << "deleting edge between: " << vec_of_strings[i] << " and: " << vec_of_strings[i+1] << endl;
    }
    m_curr_tuple_index = GetIndexFromString(vec_of_strings[vec_of_strings.size()-1]);
    cout << "index of current tuple: " << m_curr_tuple_index << endl;
    
    
    m_sequence = m_start_sequ.substr(0,vec_of_strings.size());
    
    m_curr_letter_index = vec_of_strings.size();
    
    
    cout << "final start sequence: " << m_sequence << endl;
    cout << "current letter index: " << m_curr_letter_index << endl;
    
    m_final_tuple = m_start_sequ.substr(0,m_order);
    
    // remove all outgoing edges from last sequ...":
    // necessary ?
    //m_adjacency_list[GetIndexFromString(m_final_tuple)].clear();
    cout << "final sequence: " << m_final_tuple << endl;
    
}

void CDBGraph::DeleteNode(unsigned short node_index_a)
{
    list<unsigned short> list_of_nbs = GetNeighbors(node_index_a);
    
    // look at all possible predecessors:
    
    string curr_string = GetStringFromIndex(node_index_a);
    
    vector<string> nb_strings;
    
    nb_strings.push_back(string("A")+curr_string.substr(0,m_order-1));
    nb_strings.push_back(string("C")+curr_string.substr(0,m_order-1));
    nb_strings.push_back(string("G")+curr_string.substr(0,m_order-1));
    nb_strings.push_back(string("T")+curr_string.substr(0,m_order-1));
    
    for (int i=0;i<4;i++)
    {
        cout << nb_strings[i] << endl;
        list<unsigned short> list_of_predecessors = GetNeighbors(nb_strings[i]);
        //list_of_predecessors.remove(node_index_a);
        m_adjacency_list[GetIndexFromString(nb_strings[i])].remove(node_index_a);
    }
    
    // now remove all indices from the current node:
    m_adjacency_list[node_index_a].clear();
    
}

void CDBGraph::DeleteNode(string node_name_a)
{
    DeleteNode(GetIndexFromString(node_name_a));
}

void CDBGraph::DeleteEdge(unsigned short node_index_a,unsigned short node_index_b)
{
    m_adjacency_list[node_index_a].remove(node_index_b);
}

void CDBGraph::DeleteEdge(string node_name_a,string node_name_b)
{
    DeleteEdge(GetIndexFromString(node_name_a),GetIndexFromString(node_name_b));
}

list<unsigned short>& CDBGraph::GetNeighbors(unsigned short node_index)
{
    return m_adjacency_list[node_index];
}

list<unsigned short>& CDBGraph::GetNeighbors(string node_name_a)
{
    return GetNeighbors(GetIndexFromString(node_name_a));
}

bool CDBGraph::HasEdge(unsigned short node_index_a,unsigned short node_index_b)
{
    list<unsigned short> nb_list = GetNeighbors(node_index_a);
    
    // iterate through list:
    list<unsigned short>::iterator it;
    for(it=nb_list.begin();it!=nb_list.end();it++)
    {
        if(*it==node_index_b)
            return true;
    }
    return false;
}

bool CDBGraph::HasEdge(string node_name_a,string node_name_b)
{
    return HasEdge(GetIndexFromString(node_name_a),GetIndexFromString(node_name_b));
}

string CDBGraph::GetStringFromIndex(int node_index)
{return m_tuples[node_index];}

unsigned short CDBGraph::GetIndexFromString(string node_string)
{
    // convert to c-string
    char* c_string = new char[6];
    strcpy(c_string,node_string.c_str());
    
    unsigned short index = 0;
    unsigned short value = 1;
    
    for(int i=m_order-1;i>=0;i-=1)
    {
        int curr_index_value=0;
        switch(c_string[i])
        {
            case 'A':
                curr_index_value=0;
                break;
            case 'C':
                curr_index_value=1;
                break;
            case 'G':
                curr_index_value=2;
                break;
            case 'T':
                curr_index_value=3;
                break;
        }
        index+=curr_index_value*value;
        value*=4;
    }
    return index;
}

void CDBGraph::FastFullyConnectGraph()
{
    int num_nodes = pow(4,m_order);
    m_adjacency_list.reserve(num_nodes);
    int curr_index=0;
    for(int i=0;i<num_nodes;i+=1)
    {
        list<unsigned short> temp_list;
        for(int j=0;j<4;j+=1)
            temp_list.push_back(curr_index+j);
        curr_index+=4;
        m_adjacency_list.push_back(temp_list);
        if(curr_index>=num_nodes)
            curr_index=0;
    }
}

void CDBGraph::PrintAdjacencyList()
{
    int num_nodes = pow(4,m_order);
    for(int i=0;i<num_nodes;i+=1)
    {
        list<unsigned short>::iterator it;
        
        cout << i << ": " << m_tuples[i] << "\t";
        
        for(it=m_adjacency_list[i].begin();it!=m_adjacency_list[i].end();it++)
        {
            cout << *it << " ";
        }
        cout << endl;
    }
}

void CDBGraph::FullyConnectGraph()
{
    // loop over all tuples:
    int num_tuples = pow(4,m_order);
    m_adjacency_list.reserve(num_tuples);
    
    for(int i=0;i<num_tuples;i+=1)
    {
        list<unsigned short> temp_list;
        
        for(int j=0;j<num_tuples;j+=1)
        {
            string postfix = m_tuples[i].substr(1,m_order-1);
            string prefix = m_tuples[j].substr(0,m_order-1);
            if(postfix==prefix) // add to adjacency list for current
            {
                temp_list.push_back(j);
            }
        }
        m_adjacency_list.push_back(temp_list);
    }
}

void CDBGraph::PrintTuples()
{
    for(int i=0;i<pow(4,m_order);i+=1)
        cout << i << " " << m_tuples[i] << endl;
}

void CDBGraph::PrintInfo()
{
    
    int num_nodes = pow(4,m_order);
    int num_edges = pow(4,m_order+1);
    cout << "order of the De Bruijn graph: " << m_order << endl;
    cout << "number of nodes in this graph: " << num_nodes << endl;
    cout << "number of edges in this graph: " << num_edges << endl;
    
}

void CDBGraph::CreateTuples()
{
    char temp_c_char[m_order];
    for(int i=0;i<pow(4,m_order);i+=1)
    {
        for(int z=0;z<m_order;z+=1)
            temp_c_char[m_order-z-1] = letters[(i/(int)pow(4,z))%4];
        
        string temp_str(temp_c_char);
        m_tuples.push_back(temp_str);
        ///cout << m_tuples[m_tuples.size()-1] << endl;
    }
}

int main (int argc, char * const argv[]) 
{
    
    srand(unsigned(time(NULL)));
    
    CDBGraph db_graph;
    db_graph.SetOrder(5);
    
    db_graph.PrintInfo();
    cout << "Create tuples: " << endl;
    db_graph.CreateTuples();
    cout << "finished create tuples: " << endl;
    cout << "fully connect graph: " << endl;
    db_graph.FastFullyConnectGraph();
    cout << "finished to fully connect graph: " << endl;
    cout << "print adjacency list: " << endl;
    db_graph.PrintAdjacencyList();
    cout << "finished to print adjacency list: " << endl;
    db_graph.PrintTuples();
    
    // check some functionality:
    char* filename_start_sequ = new char[256];
    char* filename_to_delete = new char[256];
    
    strcpy(filename_start_sequ,"//Users//christianwachauf//Documents//sonstiges//SequenzenDB//startsequenz.txt");
    //strcpy(filename_to_delete,"//Users//christian//Documents//C-Code//DB_graph_algorithms//candidate_sequence9.txt");
    
   //db_graph.ReadStartSequence(filename_start_sequ);
   //db_graph.ConsiderStartSequence();
    
  //  string undesired_sequ1("CGTCTC");
    //string undesired_sequ2("GAGACG");
    
    //db_graph.DeleteNode(undesired_sequ1);
    //db_graph.DeleteNode(undesired_sequ2);
    //db_graph.DeleteEdge(string("ACGTTG"),string("CGTTGT"));
    //db_graph.DeleteSequenceFromFile(filename_to_delete);
    
    //list<unsigned int> path = db_graph.FindPath(db_graph.GetIndexFromString("TTGAAG"),db_graph.GetIndexFromString("ATTGAA"));
    
    //int outdegree = db_graph.GetOutDegree("ACGCGC");
    //	cout << "outdegree of: " << "ACGCGC: " << outdegree << endl;
    std::srand ( unsigned ( std::time(NULL) ) );
    for(int i=0;i<100000;i+=1)
    {
        db_graph.CreateSequence();
    }
    int d;
    cout << "Eingabe zum Beenden: " << endl;
    cin >> d;
    delete[] filename_start_sequ;
    return 0;
}
