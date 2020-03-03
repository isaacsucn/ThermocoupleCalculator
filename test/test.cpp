#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <map>
#include <algorithm>
#include <string.h>
#include <stdio.h>

#include "../src/thermocouple.h"
using namespace std;

const unsigned short NUM_PER_LINE=11;
const unsigned short MAX_TEMP_NUM=2000;

enum TableBaseIndex
{
	TableBaseLowLimit=0,
	TableBaseHighLimit,
	TableBaseLowLimitSub,
	TableBaseHighLimitSub,
    TableBaseLineNum,
    TableBaseIndexNum
};
const char *ThermalCoupleTypeString[]={
    "TC_TYPE_R",
    "TC_TYPE_S",
    "TC_TYPE_B",
	"TC_TYPE_J",
	"TC_TYPE_T",
	"TC_TYPE_E",
	"TC_TYPE_N",
	"TC_TYPE_K",
	"TC_TYPE_C",
	"TC_TYPE_A"
};
static double its90Table[TC_TYPE_NUM][MAX_TEMP_NUM][NUM_PER_LINE];
static int its90TableBase[TC_TYPE_NUM][TableBaseIndexNum];   

const double EMF_ERROR_PERMIT=0.12;
const double T_ERROR_PERMIT=0.2;
int read_its90_table_from_file(string& file_name)
{
    ifstream its90in(file_name.c_str(), std::ios::binary);
    if(!its90in) { 
        cout << "error while open its90 file when write." << endl;
        return -1;
    }
    its90in.read((char*)its90TableBase, sizeof(int)*TC_TYPE_NUM*TableBaseIndexNum);
    its90in.read((char*)its90Table,sizeof(double)*TC_TYPE_NUM*MAX_TEMP_NUM*NUM_PER_LINE);
    its90in.close();
    cout<<"read its90 table file"<<file_name<<" success!"<<endl;
    return 0;
}

int show_title(ostream& out_file,int sign)
{
    if(sign<0)
    {
        out_file<<"°C        ";
        for(int i=0;i<NUM_PER_LINE;i++)
        {
            out_file<<setw(12)<<-i;
        }
        out_file<<setw(11)<<"max error"<<endl;
    }
    else
    {
        out_file<<"°C        ";
        for(int i=0;i<NUM_PER_LINE;i++)
        {
            out_file<<setw(12)<<i;
        }
        out_file<<setw(11)<<"max error"<<endl;
    } 
    return 0;
}

int write_test_result(string&file_name)
{
    ofstream testResult(file_name.c_str(), std::ios::binary);
    if(!testResult) { 
        cout << "error while open its90 file when write." << endl;
        return -1;
    }
    int lineNum;
    int columnNum;
    int sign;
    int initRange;
    int baseTemperature;
    double exactTemperature;
    double calTemperature;
    double emf;
    int rc;
    double diff;
    double maxError;
    float errorRangeLow,errorRangeHigh;
    for(int type=TC_TYPE_R;type<TC_TYPE_NUM;type++)
    {
        testResult<<" ITS-90 Table for type "<<ThermalCoupleTypeString[type]<< " thermocouple"<<endl;
        testResult<<" range "<<its90TableBase[type][TableBaseLowLimit]<<"."<<its90TableBase[type][TableBaseLowLimitSub];
        testResult<<"~"<<its90TableBase[type][TableBaseHighLimit]<<"."<<its90TableBase[type][TableBaseHighLimitSub]<<"°C"<<endl;
        if(its90TableBase[type][TableBaseLowLimit]<0)
        {
            sign=-1;
            initRange=-1;
        }
        else
        {
            sign=1;
            initRange=0;
        }
        show_title(testResult,sign);
        
        for(int i=0;i<its90TableBase[type][TableBaseLineNum];i++)
        {
            if(initRange<0&&sign>0)
            {
                baseTemperature=(its90TableBase[type][TableBaseLowLimit]+(i-1)*10);
            }
            else
            {
                baseTemperature=(its90TableBase[type][TableBaseLowLimit]+i*10);
            }
            
            if(baseTemperature==10&&(sign<0)&&(initRange<0))
            {
                sign=1;
                baseTemperature=0;
                show_title(testResult,sign);
            }
            testResult<<"Table:"<<setw(4)<<baseTemperature;
            columnNum=NUM_PER_LINE;
            if((i==0)&&(its90TableBase[type][TableBaseLowLimit]<0))
            {
                columnNum=its90TableBase[type][TableBaseLowLimitSub]+1;
            }
            if(i==its90TableBase[type][TableBaseLineNum]-1)
            {
                columnNum=its90TableBase[type][TableBaseHighLimitSub]+1;
            }
            for(int j=0;j<columnNum;j++)
            {
                testResult<<setw(12)<<its90Table[type][i][j];
            }
            testResult<<endl;
            testResult<<"Refer:"<<setw(4)<<baseTemperature;
            maxError=0.0f;
            for(int j=0;j<columnNum;j++)
            {
                exactTemperature=(double)baseTemperature+(double)(sign*j);
                rc=TcTtoEMFwithRc((ThermalCoupleType)type,exactTemperature,emf);
                testResult<<" "<<setw(11)<<emf;
                if(rc==TC_CAL_SUCCESS)
                {
                    diff=abs(emf-its90Table[type][i][j]);
                    if(diff>maxError)
                    {
                        maxError=diff;
                    }
                }
            }
            testResult<<" "<<setw(11)<<maxError<<endl;
            if(maxError>EMF_ERROR_PERMIT)
            {
                cout<<"TtoEMF test failed! "<<ThermalCoupleTypeString[type]<<", temperature line "<<baseTemperature;
                cout<<" error is "<<maxError<<endl;
            }
            maxError=0.0f;
            testResult<<"Inver:"<<setw(4)<<baseTemperature;
            for(int j=0;j<columnNum;j++)
            {
                exactTemperature=(double)baseTemperature+(double)(sign*j);
                rc=TcEMFtoTwithRc((ThermalCoupleType)type,its90Table[type][i][j],calTemperature,errorRangeLow,errorRangeHigh);
                testResult<<" "<<setw(11)<<calTemperature;
                if(rc==TC_CAL_SUCCESS)
                {
                    diff=abs(calTemperature-exactTemperature);
                    if(diff>maxError)
                    {
                        maxError=diff;
                    }
                }
            }           
            testResult<<" "<<setw(11)<<maxError;
            testResult<<"  "<<errorRangeLow<<"~"<<errorRangeHigh<<endl;
            if(maxError>T_ERROR_PERMIT)
            {
                cout<<"EMFtoT test failed! "<<ThermalCoupleTypeString[type]<<", temperature line "<<baseTemperature;
                cout<<" error is "<<maxError<<endl;
            }
        }       
    }
    testResult.close();
}

int main()
{
    string its90FileName="its90.dat";
    string its90TestResultName="its90test_result.txt";
    read_its90_table_from_file(its90FileName);
    write_test_result(its90TestResultName);
}