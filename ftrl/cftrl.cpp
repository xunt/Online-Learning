//  Created by bob on 14-5-15.
//  Copyright (c) 2014年 bob. All rights reserved.

#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <vector>
#include <math.h>
#include <stdlib.h>
#include <tr1/unordered_map>

using namespace std;

typedef std::tr1::unordered_map<std::string, float> string_map_float;
typedef std::tr1::unordered_map<std::string, vector<float> > string_map_vector;

//string
string & trim(string &);
void split(string& s, string& delim, vector<string >& ret);
//file
const char * file_wzn_out = "data/test/wzn20140506";
const char * file_w_out = "data/test/w20140506";
const char * file_train_in = "data/20140506";
const char * file_weight_in = "data/bias_model_weight";
//Parameter
const float R1 = 0.005;
const float R2 = 1.0;
const float A = 0.5;
const float B= 1.0;

inline int sgn(float x)
{
    if(x>1e-6) return 1;
    else if(x<-1e-6) return -1;
    else return 0;
}

inline float mul(float a, float b)
{
	return sgn(a) * sgn(b) * exp( log(fabs(a)) + log(fabs(b)) );
}

void initWZN(string_map_vector &wzn)
{
    fstream fin(file_weight_in);
    string line;
    while(getline(fin, line))
    {
        vector<string > ret;
        string dot("\t");
        split(line, dot, ret);
        vector<float > tmp(3, 0.0);
        tmp[0] = atof(ret[1].c_str());
        wzn[ret[0]] = tmp;
    }
    fin.close();
}

void output(string_map_vector &wzn, long count)
{
	char tfile[256];
	sprintf(tfile, "%s_%ld", file_wzn_out,count);
    fstream outWZN(tfile,ios::out|ios::trunc);

	sprintf(tfile, "%s_%ld", file_w_out,count);
    fstream outW(tfile, ios::out|ios::trunc);

	string_map_vector::iterator itt;
    for(itt = wzn.begin();itt != wzn.end(); ++itt)
    {
        string key = itt->first;
        vector<float> values = itt->second;
        outWZN<<key<<"\t"<<values[0]<<","<<values[1]<<","<<values[2]<<endl;
        outW<<key<<"\t"<<values[0]<<endl;
    }
    outWZN.close();
    outW.close();
	cout<<"output "<<count<<endl;
}

void line_loop(string_map_float &v, int yt, string_map_vector &wzn)
{
	string_map_float::iterator it;
    
    //check and add new keys
    for(it=v.begin() ;it!=v.end(); ++it)
    {
        string key = it->first;
        string_map_vector::iterator it = wzn.find(key);
        if(it == wzn.end()) {
            //没找到
            wzn[key] = vector<float >(3, 0.0);
        }
    }
    
    //change wzn
	string_map_vector::iterator itt;
    for(itt = wzn.begin();itt != wzn.end(); ++itt)
    {
        string key = itt->first;
        vector<float> values = itt->second;
        if(fabs(values[1])<=R1)
            values[0] = 0.0;
        else
            values[0] = -1.0 * (values[1] - sgn(values[1])*R1) / (R2 + (B + sqrt(values[2]))/A);
        itt->second = values;
    }
    
    //count pt
    float vw = 0.0;
    for(it=v.begin() ;it!=v.end(); ++it)
        vw += mul(it->second,wzn[it->first][0]);
    float pt = 1.0 / (1 + exp(0.0-vw) );
    
    //travelal
    for(it=v.begin() ;it!=v.end(); ++it)
    {
        string key = it->first;
        float value = it->second;
        float gi = mul((pt-yt) , value);
        float oi = (sqrt(wzn[key][2] + gi*gi) - sqrt(wzn[key][2])) /A;
        wzn[key][1] = wzn[key][1] + gi - oi * wzn[key][0];
        wzn[key][2] = wzn[key][2] + mul(gi,gi);
    }
}


void ftrl()
{
    //init Model
	string_map_vector wzn;   //model
    initWZN(wzn);
    // insert code here...
    fstream fin(file_train_in);
    string line;
	long count=-1;
    output(wzn, count);
	string_map_float v;   //训练集v
    while(getline(fin,line))
    {
		//split line
        vector<string > ret;
        string dot("\t");
        split(line, dot, ret);

        int yt = atoi(ret[0].c_str());			//fill yt
		v.clear();
        for(unsigned i=1; i<ret.size(); i++)	//fill V
        {
            dot = ":";
            vector<string > train;
            split(ret[i],dot, train);
			if(train[0]!="timestamp")
            	v[train[0]] = atof(train[1].c_str());
        }
		line_loop(v, yt, wzn);       			//loop V, change wzn

		//output
		count++;
		if(!(count%100000)) cout<<count<<endl;
		if(!(count%5000000)) output(wzn,count);
    }
    fin.close();
    
    output(wzn,count);
}

string& trim(std::string &s)
{
    if (s.empty())
    {
        return s;
    }
    s.erase(0,s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return s;
}

//注意：当字符串为空时，也会返回一个空字符串
void split(string& s, string& delim,vector<string >& ret)
{
	size_t last = 0;
	size_t index=s.find_first_of(delim,last);
	while (index!=std::string::npos)
	{
		ret.push_back(s.substr(last,index-last));
		last=index+1;
		index=s.find_first_of(delim,last);
	}
	if (index-last>0)
	{
		ret.push_back(s.substr(last,index-last));
	}
}

int main(int argc, const char * argv[])
{
    ftrl();
}
