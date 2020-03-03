/* MIT License

Copyright (c) 2020 isaacsucn,https://github.com/isaacsucn/

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Read data from file all.tab, which is acquired from nist.gov.
Write its90 table to its90.dat in binary, which can be used by test applications.
Write its90 table to its90.txt in text, which can be read.
Write compareIts90.txt to test its90.dat file. 
    The program read data from its90.dat then write compareIts90.txt.
    You can compare its90.txt and compareIts90.txt to verify data is saved.
Write the coefficient data to termocouple.h which can be used under fold ../src
 */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <map>
#include <algorithm>
#include <string.h>
#include <stdio.h>

using namespace std; 

const unsigned short TC_MAX_SUBRANGE_NUM=6;
const unsigned short TC_MAX_COEFFICIENTS_NUM=15;
const unsigned short TC_TYPE_K_EXPONENTIAL_CONST_NUM=3;
const unsigned short MAX_CHAR_NUM=30;
enum ThermalCoupleType
{
	TC_TYPE_R=0,
	TC_TYPE_S,
	TC_TYPE_B,
	TC_TYPE_J,
	TC_TYPE_T,
	TC_TYPE_E,
	TC_TYPE_N,
	TC_TYPE_K,
	TC_TYPE_C,
	TC_TYPE_A,
    TC_TYPE_NUM
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
map<char,ThermalCoupleType> thermoCoupleMap;

enum ErrorRangeIndex
{
	ErrorRangeLowLimit=0,
	ErrorRangeHighLimit,
    ErrorRangeIndexNum
};
enum ActionType
{
	ActionIts90Table=0,
	ActionReferenceFunction,
	ActionInverseFunctionCoefficient,
    ActionTemperaturRange,
    ActionVoltageRange,
    ActionErrorRange,
    ActionExponential
};

const unsigned short NUM_PER_LINE=11;
const unsigned short MAX_TEMP_NUM=2000;
enum TableBaseIndex
{
	TableBaseLowLimit=0,
	TableBaseHighLimit,
	TableBaseLowLimitSub,
	TableBaseHighLimitSub,
    TableBaseLineNum,
    TalbeBaseIndexNum
};
static double its90Table[TC_TYPE_NUM][MAX_TEMP_NUM][NUM_PER_LINE];
static int its90TableBase[TC_TYPE_NUM][TalbeBaseIndexNum];  

static unsigned short tc_reference_function_subrange_num[TC_TYPE_NUM];
static unsigned short tc_reference_function_coefficients_num[TC_TYPE_NUM][TC_MAX_SUBRANGE_NUM];
static double tc_reference_function_subrange_base[TC_TYPE_NUM][TC_MAX_SUBRANGE_NUM+1];
static char tc_reference_function_subrange_base_char[TC_TYPE_NUM][TC_MAX_SUBRANGE_NUM+1][MAX_CHAR_NUM];
static double tc_reference_function_coefficients[TC_TYPE_NUM][TC_MAX_SUBRANGE_NUM][TC_MAX_COEFFICIENTS_NUM];
static char tc_reference_function_coefficients_char[TC_TYPE_NUM][TC_MAX_SUBRANGE_NUM][TC_MAX_COEFFICIENTS_NUM][MAX_CHAR_NUM];
static double tc_reference_function_typeK_exponential_const[TC_TYPE_K_EXPONENTIAL_CONST_NUM];
static char tc_reference_function_typeK_exponential_const_char[TC_TYPE_K_EXPONENTIAL_CONST_NUM][MAX_CHAR_NUM];

static unsigned short tc_inverse_function_subrange_num[TC_TYPE_NUM];
static unsigned short tc_inverse_function_coefficients_num[TC_TYPE_NUM][TC_MAX_SUBRANGE_NUM];
static double tc_inverse_function_subrange_base[TC_TYPE_NUM][TC_MAX_SUBRANGE_NUM+1];
static char tc_inverse_function_subrange_base_char[TC_TYPE_NUM][TC_MAX_SUBRANGE_NUM+1][MAX_CHAR_NUM];
static double tc_inverse_function_temp_subrange_base[TC_TYPE_NUM][TC_MAX_SUBRANGE_NUM+1];
static char tc_inverse_function_temp_subrange_base_char[TC_TYPE_NUM][TC_MAX_SUBRANGE_NUM+1][MAX_CHAR_NUM];
static double tc_inverse_function_coefficients[TC_TYPE_NUM][TC_MAX_SUBRANGE_NUM][TC_MAX_COEFFICIENTS_NUM];
static char tc_inverse_function_coefficients_char[TC_TYPE_NUM][TC_MAX_SUBRANGE_NUM][TC_MAX_COEFFICIENTS_NUM][MAX_CHAR_NUM];
static float tc_inverse_function_error_range[TC_TYPE_NUM][TC_MAX_SUBRANGE_NUM][ErrorRangeIndexNum];

int read_data_from_file(string& file_name)
{
    int nameCount(0),typeCount(0),TemperatureCount(0),rangeCount(0),RangeCount(0),inverseCount(0),voltageCount(0),errorCount(0);
    int RangeInTemperatureCount(0),RangeInVoltageCount(0),RangeInErrorCount(0);
    ifstream inFile(file_name.c_str(),ios::in);
    if(!inFile) { 
        cout << "error opening source file." << endl;
        return -1;
    }
    
    char currentTypeChar;
    ThermalCoupleType currentType;
    ActionType  currentAction;
    bool notFirstRun=false;
    int rc=0;

    string s,s1,s2;
    istringstream is;
    char s0;
    int lineNum=0;

    double coefficient;
    int coefficientIndex;

    int its90TableIndex=0;
    int its90Temperature=0;
    double its90Voltage;
    bool foundBaseTemp=false;

    int rangeIndex=0;
    int coefficientNum=0;
    double rangeBase;
    
    int i;
    while(getline(inFile,s))
    {
        lineNum++;
        if(s.empty())
        {
            continue;
        }
        if(s.find(',')!=-1)
        {
            replace(s.begin(),s.end(),',',' ');
        }
        is.clear();
        s1.clear();
        is.str(s);
        is>>s1;
        if(s1.empty())
        {
            continue;
        } 
        s0=s1.at(0);
        if((s0=='*')||(s0=='\260'))
        {
            continue;
        }      
        if(s1==string("Thermoelectric")||s1==string("emf")||s1==string("temperature"))
        {
            continue;
        } 
        if(s1==string("type:"))
        {
            typeCount++;
            if(!is)
            {
                cout<<"ERROR, expected data at line "<<lineNum<<endl;
                cout<<"The line is :"<<s<<endl;
                return -1;
            }
            is>>s2;
            if(s2.empty())
            {
                cout<<"ERROR, the string afte type is empty at line "<<lineNum<<endl;
                cout<<"The line is :"<<s<<endl;
                return -1;
            }
            if(s2.at(0)!=currentTypeChar)
            {
                cout<<"ERROR, the string "<<s2.at(0)<<" afte type is not pair with current type "<<currentTypeChar<<"  at line "<<lineNum<<endl;
                cout<<"The line is :"<<s<<endl;
                return -1;
            }
            continue;
        }
        if(s1==string("ITS-90"))
        {
            int p=s.find("type"); 
            currentTypeChar=s.at(p+5); 

            if(thermoCoupleMap.find(currentTypeChar)==thermoCoupleMap.end())
            {
                cout<<"ERROR, wrong type: "<<currentTypeChar<<" at line"<<lineNum<<endl;
                cout<<"The line is :"<<s<<endl;
                return -1;
            }
            currentType=thermoCoupleMap[currentTypeChar];
            if(currentAction!=ActionIts90Table)
            {
                currentAction=ActionIts90Table;
                its90TableIndex=0;
                foundBaseTemp=false;
            }
            continue;
        }
        if(s1==string("name:"))
        {
            nameCount++;
            currentAction=ActionReferenceFunction;
            rangeIndex=-1;
            continue;
        }
        if(s1==string("Voltage"))
        {
            voltageCount++;
            rangeIndex=0;
            if(!is)
            {
                cout<<"ERROR, expected data at line "<<lineNum<<endl;
                cout<<"The line is :"<<s<<endl;
                return -1;
            }
            is>>s2;
            try
            {
                rangeBase=stod(s2);
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << "t line"<< lineNum <<'\n';
            }
            tc_inverse_function_subrange_base[currentType][rangeIndex]=rangeBase;
            strcpy(tc_inverse_function_subrange_base_char[currentType][rangeIndex],s2.c_str());
            rangeIndex++;
            currentAction=ActionVoltageRange;
            continue;
        }
        if(s1==string("Error"))
        {
            errorCount++;
            rangeIndex=0;
            while(is>>s2)
            {
                try
                {
                    rangeBase=stof(s2);
                }
                catch(const std::exception& e)
                {
                    std::cerr << e.what() << "t line"<< lineNum <<'\n';
                }
                tc_inverse_function_error_range[currentType][rangeIndex][ErrorRangeLowLimit]=rangeBase;
                rangeIndex++;
            }
            currentAction=ActionErrorRange;
            continue;
        }
        if(s1==string("Temperature"))
        {
            TemperatureCount++;
            rangeIndex=0;
            if(!is)
            {
                cout<<"ERROR, expected data at line "<<lineNum<<endl;
                cout<<"The line is :"<<s<<endl;
                return -1;
            }
            is>>s2;
            try
            {
                rangeBase=stod(s2);
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << "t line"<< lineNum <<'\n';
            }
            tc_inverse_function_temp_subrange_base[currentType][rangeIndex]=rangeBase;
            strcpy(tc_inverse_function_temp_subrange_base_char[currentType][rangeIndex],s2.c_str());
            rangeIndex++;
            currentAction=ActionTemperaturRange;
            continue;
        }  
        if(s1==string("range:"))
        {
            rangeCount++;
            if(currentAction!=ActionReferenceFunction)
            {
                cout<<"ERROR, string range appeared in wrong place at line "<<lineNum<<endl;
                cout<<"The line is :"<<s<<endl;
                return -1;
            }
            if(!is)
            {
                cout<<"ERROR, expected data at line "<<lineNum<<endl;
                cout<<"The line is :"<<s<<endl;
                return -1;
            }
            is>>s2;
            try
            {
                rangeBase=stod(s2);
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << "t line"<< lineNum <<'\n';
                return -1;
            }
            rangeIndex++;
            tc_reference_function_subrange_base[currentType][rangeIndex]=rangeBase;
            strcpy(tc_reference_function_subrange_base_char[currentType][rangeIndex],s2.c_str());
            if(!is)
            {
                cout<<"ERROR, expected data at line "<<lineNum<<endl;
                cout<<"The line is :"<<s<<endl;
                return -1;
            }
            is>>s2;
            try
            {
                rangeBase=stod(s2);
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << "t line"<< lineNum <<'\n';
                return -1;
            }
            tc_reference_function_subrange_base[currentType][rangeIndex+1]=rangeBase;
            strcpy(tc_reference_function_subrange_base_char[currentType][rangeIndex+1],s2.c_str());
            is>>coefficientNum;
            tc_reference_function_coefficients_num[currentType][rangeIndex]=coefficientNum+1;
            tc_reference_function_subrange_num[currentType]=rangeIndex+1;
            coefficientIndex=0;
            continue;
        }
        if(s1==string("Range:"))
        {
            RangeCount++;
            switch (currentAction)
            {
            case ActionTemperaturRange:
                RangeInTemperatureCount++;
                while(is>>s2)
                {
                    try
                    {
                        rangeBase=stod(s2);
                    }
                    catch(const std::exception& e)
                    {
                        std::cerr << e.what() << "t line"<< lineNum <<'\n';
                    }
                    tc_inverse_function_temp_subrange_base[currentType][rangeIndex]=rangeBase;
                    strcpy(tc_inverse_function_temp_subrange_base_char[currentType][rangeIndex],s2.c_str());
                    rangeIndex++;
                }
                tc_inverse_function_subrange_num[currentType]=rangeIndex-1;
                break;
            case ActionVoltageRange:
                RangeInVoltageCount++;
                while(is>>s2)
                {
                    try
                    {
                        rangeBase=stod(s2);
                    }
                    catch(const std::exception& e)
                    {
                        std::cerr << e.what() << "t line"<< lineNum <<'\n';
                    }
                    tc_inverse_function_subrange_base[currentType][rangeIndex]=rangeBase;
                    strcpy(tc_inverse_function_subrange_base_char[currentType][rangeIndex],s2.c_str());
                    rangeIndex++;
                } 
                currentAction=ActionInverseFunctionCoefficient;
                coefficientIndex=0;
                break;
            case ActionErrorRange:
                RangeInErrorCount++;
                rangeIndex=0;
                while(is>>s2)
                {
                    try
                    {
                        rangeBase=stod(s2);
                    }
                    catch(const std::exception& e)
                    {
                        std::cerr << e.what() << "t line"<< lineNum <<'\n';
                        return -1;
                    }
                    tc_inverse_function_error_range[currentType][rangeIndex][ErrorRangeHighLimit]=rangeBase;
                    rangeIndex++;
                }
            default:
                break;
            }
            continue;
        }
        if(s1==string("Inverse"))
        {
            inverseCount++;
            int p=s.find("type"); 
            currentTypeChar=s.at(p+5); 
            if(thermoCoupleMap.find(currentTypeChar)==thermoCoupleMap.end())
            {
                cout<<"ERROR, wrong type: "<<currentTypeChar<<" at line"<<lineNum<<endl;
                cout<<"The line is :"<<s<<endl;
                return -1;
            }
            if(currentType!=thermoCoupleMap[currentTypeChar])
            {
                cout<<"ERROR, type: "<<currentTypeChar<<" is not same as the current type at line"<<lineNum<<endl;
                cout<<"The line is :"<<s<<endl;
                return -1;             
            }
            currentAction=ActionInverseFunctionCoefficient;
            continue;
        }    
        if(s1==string("exponential:"))
        {
            currentAction=ActionExponential;
            coefficientIndex=0;
            continue;
        }
        switch (currentAction)
        {
        case ActionIts90Table:
            try
            {
                its90Temperature = stoi (s1);
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << "t line"<< lineNum <<'\n';
                return -1;
            }
            if(!foundBaseTemp)
            {
                its90TableBase[currentType][TableBaseLowLimit]=its90Temperature;
                foundBaseTemp=true;
            }
            else
            {
                its90TableBase[currentType][TableBaseHighLimit]=its90Temperature;
            }
            i=0;
            while(is>>its90Voltage)
            {
                its90TableBase[currentType][TableBaseHighLimitSub]=i;
                its90Table[currentType][its90TableIndex][i]=its90Voltage;
                i++;
            } ;
            its90TableIndex++;
            its90TableBase[currentType][TableBaseLineNum]=its90TableIndex;     
            break;
        case ActionReferenceFunction:
            try
            {
                coefficient=stod(s1);
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << "t line"<< lineNum <<'\n';
                return -1;
            }
            tc_reference_function_coefficients[currentType][rangeIndex][coefficientIndex]=coefficient;
            strcpy(tc_reference_function_coefficients_char[currentType][rangeIndex][coefficientIndex],s1.c_str());
            coefficientIndex++;
            break;
        case ActionInverseFunctionCoefficient:
            rangeIndex=0;
            try
            {
                coefficient=stod(s1);
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << "t line"<< lineNum <<'\n';
                return -1;
            }
            tc_inverse_function_coefficients[currentType][rangeIndex][coefficientIndex]=coefficient;
            strcpy(tc_inverse_function_coefficients_char[currentType][rangeIndex][coefficientIndex],s1.c_str());
            tc_inverse_function_coefficients_num[currentType][rangeIndex]=coefficientIndex+1;
            rangeIndex++;
            while(is>>s2)
            {
                try
                {
                    coefficient=stod(s2);
                }
                catch(const std::exception& e)
                {
                    std::cerr << e.what() << "t line"<< lineNum <<'\n';
                    return -1;
                }
                tc_inverse_function_coefficients[currentType][rangeIndex][coefficientIndex]=coefficient;
                strcpy(tc_inverse_function_coefficients_char[currentType][rangeIndex][coefficientIndex],s2.c_str());
                tc_inverse_function_coefficients_num[currentType][rangeIndex]=coefficientIndex+1;
                rangeIndex++;
            }
            coefficientIndex++;
            break;
        case ActionExponential:
            try
            {
                is>>s2;
                is>>s2;
                is>>s2;
                coefficient=stod(s2);
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << "t line"<< lineNum <<'\n';
                return -1;
            }
            tc_reference_function_typeK_exponential_const[coefficientIndex]=coefficient;
            strcpy(tc_reference_function_typeK_exponential_const_char[coefficientIndex],s2.c_str());
            coefficientIndex++;
            break;
        default:
            break;
        }
    }
    inFile.close();
    cout<<"read file "<<file_name<<"success!"<<endl;
    return 0;
}

int write_coefficient_const(ostream& outFile)
{
    outFile<<"#ifndef _THERMOCOUPLE_H"<<endl;
    outFile<<"#define _THERMOCOUPLE_H"<<endl;
    outFile<<"#include <stdio.h>"<<endl;
    outFile<<"#include <math.h>"<<endl<<endl;
    outFile<<"const unsigned short TC_MAX_SUBRANGE_NUM=6;"<<endl;
    outFile<<"const unsigned short TC_MAX_COEFFICIENTS_NUM=15;"<<endl;
    outFile<<"const unsigned short TC_TYPE_K_EXPONENTIAL_CONST_NUM=3;"<<endl<<endl;
    outFile<<"enum ThermalCoupleType"<<endl;
    outFile<<"{"<<endl;
    outFile<<"    TC_TYPE_R=0,"<<endl;
    outFile<<"    TC_TYPE_S,"<<endl;
    outFile<<"    TC_TYPE_B,"<<endl;
    outFile<<"    TC_TYPE_J,"<<endl;
    outFile<<"    TC_TYPE_T,"<<endl;
    outFile<<"    TC_TYPE_E,"<<endl;
    outFile<<"    TC_TYPE_N,"<<endl;
    outFile<<"    TC_TYPE_K,"<<endl;
    outFile<<"    TC_TYPE_C,"<<endl;
    outFile<<"    TC_TYPE_A,"<<endl;
    outFile<<"    TC_TYPE_NUM"<<endl;
    outFile<<"};"<<endl;
    outFile<<"enum ErrorRangeIndex"<<endl;
    outFile<<"{"<<endl;
    outFile<<"	ErrorRangeLowLimit=0,"<<endl;
    outFile<<"	ErrorRangeHighLimit,"<<endl;
    outFile<<"    ErrorRangeIndexNum"<<endl;
    outFile<<"};"<<endl<<endl;
    return 0;
}


int write_coefficient_end(ostream& outFile)
{
    outFile<<"#include \"function.h\""<<endl;
    outFile<<endl;
    outFile<<"#endif //_THERMOCOUPLE_H";
    return 0;
}
int write_reference_function_coefficients(ostream& outFile)
{
    outFile<<"static unsigned short tc_reference_function_subrange_num[TC_TYPE_NUM]="<<endl;
    outFile<<"{"<<endl;
    for(int outType=TC_TYPE_R;outType<=TC_TYPE_A;outType++)
    {
        outFile<<"    "<<tc_reference_function_subrange_num[outType];
        if(outType<=(TC_TYPE_A-1))
        {
            outFile<<",";
        }
        outFile<<"//"<<ThermalCoupleTypeString[outType]<<endl;
    } 
    outFile<<"};"<<endl<<endl;

    outFile<<"static double tc_reference_function_subrange_base[TC_TYPE_NUM][TC_MAX_SUBRANGE_NUM+1]="<<endl;
    outFile<<"{"<<endl;
    for(int outType=TC_TYPE_R;outType<=TC_TYPE_A;outType++)
    {
        outFile<<"    {";
        for(int outSubrangeIndex=0;outSubrangeIndex<(tc_reference_function_subrange_num[outType]+1);outSubrangeIndex++)
        {
            outFile<<tc_reference_function_subrange_base_char[outType][outSubrangeIndex];
            if(outSubrangeIndex<tc_reference_function_subrange_num[outType])
            {
                outFile<<",";
            }
        }
        outFile<<"}";
        if(outType<=(TC_TYPE_A-1))
        {
            outFile<<",";
        }
        outFile<<"//"<<ThermalCoupleTypeString[outType]<<endl;
    } 
    outFile<<"};"<<endl<<endl;

    outFile<<"static unsigned short tc_reference_function_coefficients_num[TC_TYPE_NUM][TC_MAX_SUBRANGE_NUM]="<<endl;
    outFile<<"{"<<endl;
    for(int outType=TC_TYPE_R;outType<=TC_TYPE_A;outType++)
    {
        outFile<<"    {";
        for(int outSubrangeIndex=0;outSubrangeIndex<tc_reference_function_subrange_num[outType];outSubrangeIndex++)
        {
            outFile<<tc_reference_function_coefficients_num[outType][outSubrangeIndex];
            if(outSubrangeIndex<(tc_reference_function_subrange_num[outType]-1))
            {
                outFile<<",";
            }
        }
        outFile<<"}";
        if(outType<=(TC_TYPE_A-1))
        {
            outFile<<",";
        }
        outFile<<"//"<<ThermalCoupleTypeString[outType]<<endl;
    } 
    outFile<<"};"<<endl<<endl;

    outFile<<"static double tc_reference_function_coefficients[TC_TYPE_NUM][TC_MAX_SUBRANGE_NUM][TC_MAX_COEFFICIENTS_NUM]="<<endl;
    outFile<<"{"<<endl;
    for(int outType=TC_TYPE_R;outType<=TC_TYPE_A;outType++)
    {
        outFile<<"    {//"<<ThermalCoupleTypeString[outType]<<endl;
        for(int outSubrangeIndex=0;outSubrangeIndex<tc_reference_function_subrange_num[outType];outSubrangeIndex++)
        {
            outFile<<"        {";
            for(int outCoefficientIndex=0;outCoefficientIndex<tc_reference_function_coefficients_num[outType][outSubrangeIndex];outCoefficientIndex++)
            {
                outFile<<tc_reference_function_coefficients_char[outType][outSubrangeIndex][outCoefficientIndex];
                if(outCoefficientIndex<(tc_reference_function_coefficients_num[outType][outSubrangeIndex]-1))
                {
                    outFile<<",";
                }
            }
            outFile<<"}";
            if(outSubrangeIndex<(tc_reference_function_subrange_num[outType]-1))
            {
                outFile<<",";
            }
            outFile<<"//"<<tc_reference_function_subrange_base_char[outType][outSubrangeIndex]<<"~"<<tc_reference_function_subrange_base_char[outType][outSubrangeIndex+1];
            outFile<<",n="<<tc_reference_function_coefficients_num[outType][outSubrangeIndex]<<endl;
        }
        outFile<<"    }";
        if(outType<=(TC_TYPE_A-1))
        {
            outFile<<",";
        }
        outFile<<endl;
    } 
    outFile<<"};"<<endl<<endl;

    outFile<<"static double tc_reference_function_typeK_exponential_const[TC_TYPE_K_EXPONENTIAL_CONST_NUM]="<<endl;
    outFile<<"{";
    for(int outi=0;outi<=TC_TYPE_K_EXPONENTIAL_CONST_NUM;outi++)
    {
        outFile<<tc_reference_function_typeK_exponential_const_char[outi];
        if(outi<(TC_TYPE_K_EXPONENTIAL_CONST_NUM-1))
        {
           outFile<<","; 
        }
    } 
    outFile<<"};"<<endl<<endl;
    return 0;
}

int write_inverse_function_coefficients(ostream& outFile)
{
    outFile<<"static unsigned short tc_inverse_function_subrange_num[TC_TYPE_NUM]="<<endl;
    outFile<<"{"<<endl;
    for(int outType=TC_TYPE_R;outType<=TC_TYPE_A;outType++)
    {
        outFile<<"    "<<tc_inverse_function_subrange_num[outType];
        if(outType<=(TC_TYPE_A-1))
        {
            outFile<<",";
        }
        outFile<<"//"<<ThermalCoupleTypeString[outType]<<endl;
    } 
    outFile<<"};"<<endl<<endl;
    
    outFile<<"static double tc_inverse_function_temp_subrange_base[TC_TYPE_NUM][TC_MAX_SUBRANGE_NUM+1]="<<endl;
    outFile<<"{"<<endl;
    for(int outType=TC_TYPE_R;outType<=TC_TYPE_A;outType++)
    {
        outFile<<"    {";
        for(int outSubrangeIndex=0;outSubrangeIndex<(tc_inverse_function_subrange_num[outType]+1);outSubrangeIndex++)
        {
            outFile<<tc_inverse_function_temp_subrange_base_char[outType][outSubrangeIndex];
            if(outSubrangeIndex<tc_inverse_function_subrange_num[outType])
            {
                outFile<<",";
            }
        }
        outFile<<"}";
        if(outType<=(TC_TYPE_A-1))
        {
            outFile<<",";
        }
        outFile<<"//"<<ThermalCoupleTypeString[outType]<<endl;
    } 
    outFile<<"};"<<endl<<endl;

    outFile<<"static double tc_inverse_function_subrange_base[TC_TYPE_NUM][TC_MAX_SUBRANGE_NUM+1]="<<endl;
    outFile<<"{"<<endl;
    for(int outType=TC_TYPE_R;outType<=TC_TYPE_A;outType++)
    {
        outFile<<"    {";
        for(int outSubrangeIndex=0;outSubrangeIndex<(tc_inverse_function_subrange_num[outType]+1);outSubrangeIndex++)
        {
            outFile<<tc_inverse_function_subrange_base_char[outType][outSubrangeIndex];
            if(outSubrangeIndex<tc_inverse_function_subrange_num[outType])
            {
                outFile<<",";
            }
        }
        outFile<<"}";
        if(outType<=(TC_TYPE_A-1))
        {
            outFile<<",";
        }
        outFile<<"//"<<ThermalCoupleTypeString[outType]<<endl;
    } 
    outFile<<"};"<<endl<<endl;

    outFile<<"static unsigned short tc_inverse_function_coefficients_num[TC_TYPE_NUM][TC_MAX_SUBRANGE_NUM]="<<endl;
    outFile<<"{"<<endl;
    for(int outType=TC_TYPE_R;outType<=TC_TYPE_A;outType++)
    {
        outFile<<"    {";
        for(int outSubrangeIndex=0;outSubrangeIndex<tc_inverse_function_subrange_num[outType];outSubrangeIndex++)
        {
            outFile<<tc_inverse_function_coefficients_num[outType][outSubrangeIndex];
            if(outSubrangeIndex<(tc_inverse_function_subrange_num[outType]-1))
            {
                outFile<<",";
            }
        }
        outFile<<"}";
        if(outType<=(TC_TYPE_A-1))
        {
            outFile<<",";
        }
        outFile<<"//"<<ThermalCoupleTypeString[outType]<<endl;
    } 
    outFile<<"};"<<endl<<endl;
 
    outFile<<"static double tc_inverse_function_coefficients[TC_TYPE_NUM][TC_MAX_SUBRANGE_NUM][TC_MAX_COEFFICIENTS_NUM]="<<endl;
    outFile<<"{"<<endl;
    for(int outType=TC_TYPE_R;outType<=TC_TYPE_A;outType++)
    {
        outFile<<"    {//"<<ThermalCoupleTypeString[outType]<<endl;
        for(int outSubrangeIndex=0;outSubrangeIndex<tc_inverse_function_subrange_num[outType];outSubrangeIndex++)
        {
            outFile<<"        {";
            for(int outCoefficientIndex=0;outCoefficientIndex<tc_inverse_function_coefficients_num[outType][outSubrangeIndex];outCoefficientIndex++)
            {
                outFile<<tc_inverse_function_coefficients_char[outType][outSubrangeIndex][outCoefficientIndex];
                if(outCoefficientIndex<(tc_inverse_function_coefficients_num[outType][outSubrangeIndex]-1))
                {
                    outFile<<",";
                }
            }
            outFile<<"}";
            if(outSubrangeIndex<(tc_inverse_function_subrange_num[outType]-1))
            {
                outFile<<",";
            }
            outFile<<"//"<<tc_inverse_function_subrange_base_char[outType][outSubrangeIndex]<<"~"<<tc_inverse_function_subrange_base_char[outType][outSubrangeIndex+1];
            outFile<<",n="<<tc_inverse_function_coefficients_num[outType][outSubrangeIndex]<<endl;
        }
        outFile<<"    }";
        if(outType<=(TC_TYPE_A-1))
        {
            outFile<<",";
        }
        outFile<<endl;
    } 
    outFile<<"};"<<endl<<endl;

    outFile<<"static float tc_inverse_function_error_range[TC_TYPE_NUM][TC_MAX_SUBRANGE_NUM][ErrorRangeIndexNum]="<<endl;
    outFile<<"{"<<endl;
    for(int outType=TC_TYPE_R;outType<=TC_TYPE_A;outType++)
    {
        outFile<<"    {//"<<ThermalCoupleTypeString[outType]<<endl;
        for(int outSubrangeIndex=0;outSubrangeIndex<tc_inverse_function_subrange_num[outType];outSubrangeIndex++)
        {
            outFile<<"		{";
            outFile<<tc_inverse_function_error_range[outType][outSubrangeIndex][ErrorRangeLowLimit];
            outFile<<","<<tc_inverse_function_error_range[outType][outSubrangeIndex][ErrorRangeHighLimit];
            outFile<<"},//"<<tc_inverse_function_subrange_base_char[outType][outSubrangeIndex]<<"~"<<tc_inverse_function_subrange_base_char[outType][outSubrangeIndex+1];
            outFile<<",n="<<tc_inverse_function_coefficients_num[outType][outSubrangeIndex]<<endl;
        }
        outFile<<"    }";
        if(outType<=(TC_TYPE_A-1))
        {
            outFile<<",";
        }
        outFile<<endl;
    } 
    outFile<<"};"<<endl<<endl;
    return 0;
}

int write_coefficients_to_file(string& file_name)
{
    ofstream outFile(file_name.c_str(),ios::out);
    if(!outFile) { 
        cout << "error while open des file." << endl;
        return -1;
    }
    write_coefficient_const(outFile);
    write_reference_function_coefficients(outFile);
    write_inverse_function_coefficients(outFile);
    write_coefficient_end(outFile);
    outFile.close();
    cout<<"write coefficient file"<<file_name<<" success!"<<endl;
    return 0;
}

int show_title(ostream& out_file,int sign)
{
    if(sign<0)
    {
        out_file<<"°C      0     -1     -2     -3     -4     -5     -6     -7     -8     -9    -10 "<<endl;

    }
    else
    {
        out_file<<" °C      0      1      2      3      4      5      6      7      8      9     10 "<<endl;
    } 
    return 0;
}

int write_its90_table_to_txt_file(string& file_name)
{

    ofstream its90txtout(file_name.c_str(), std::ios::binary);
    if(!its90txtout) { 
        cout << "error while open its90 file when write." << endl;
        return -1;
    }
    int lineNum;
    int columnNum;
    int sign;
    int initRange;
    int showTemperature;
    for(int type=TC_TYPE_R;type<TC_TYPE_NUM;type++)
    {
        its90txtout<<" ITS-90 Table for type "<<ThermalCoupleTypeString[type]<< " thermocouple"<<endl;
        its90txtout<<" range "<<its90TableBase[type][TableBaseLowLimit]<<"."<<its90TableBase[type][TableBaseLowLimitSub];
        its90txtout<<"~"<<its90TableBase[type][TableBaseHighLimit]<<"."<<its90TableBase[type][TableBaseHighLimitSub]<<"°C"<<endl;
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
        show_title(its90txtout,sign);
        
        for(int i=0;i<its90TableBase[type][TableBaseLineNum];i++)
        {
            if(initRange<0&&sign>0)
            {
                showTemperature=(its90TableBase[type][TableBaseLowLimit]+(i-1)*10);
            }
            else
            {
                showTemperature=(its90TableBase[type][TableBaseLowLimit]+i*10);
            }
            
            if(showTemperature==10&&(sign<0)&&(initRange<0))
            {
                sign=1;
                showTemperature=0;
                show_title(its90txtout,sign);
            }
            its90txtout<<setw(4)<<showTemperature;
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
                its90txtout<<setw(7)<<its90Table[type][i][j];
            }
            its90txtout<<endl;
        }       
    }
    its90txtout.close();
    cout<<"write its90 table txt file"<<file_name<<" success!"<<endl;
    return 0;
}

int write_its90_table_to_bin_file(string& file_name)
{

    ofstream its90out(file_name.c_str(), std::ios::binary);
    if(!its90out) { 
        cout << "error while open its90 file when write." << endl;
        return -1;
    }
    its90out.write((char*)its90TableBase, sizeof(int)*TC_TYPE_NUM*TalbeBaseIndexNum);
    its90out.write((char*)its90Table,sizeof(double)*TC_TYPE_NUM*MAX_TEMP_NUM*NUM_PER_LINE);
    its90out.close();
    cout<<"write its90 table file"<<file_name<<" success!"<<endl;
    return 0;
}

int read_its90_table_from_bin_file(string& file_name)
{
    ifstream its90in(file_name.c_str(), std::ios::binary);
    if(!its90in) { 
        cout << "error while open its90 file when write." << endl;
        return -1;
    }
    its90in.read((char*)its90TableBase, sizeof(int)*TC_TYPE_NUM*TalbeBaseIndexNum);
    its90in.read((char*)its90Table,sizeof(double)*TC_TYPE_NUM*MAX_TEMP_NUM*NUM_PER_LINE);
    its90in.close();
    cout<<"read its90 table file"<<file_name<<" success!"<<endl;
    return 0;
}
int main()
{
    thermoCoupleMap['R']=TC_TYPE_R;
	thermoCoupleMap['S']=TC_TYPE_S;
	thermoCoupleMap['B']=TC_TYPE_B;
	thermoCoupleMap['J']=TC_TYPE_J;
	thermoCoupleMap['T']=TC_TYPE_T;
	thermoCoupleMap['E']=TC_TYPE_E;
	thermoCoupleMap['N']=TC_TYPE_N;
	thermoCoupleMap['K']=TC_TYPE_K;
	thermoCoupleMap['C']=TC_TYPE_C;
	thermoCoupleMap['A']=TC_TYPE_A;
    int rc;
    string sourceFileName,coefficientFileName,its90BinFileName,its90TxtFileName,testIts90TxtFileName;
    sourceFileName="all.tab";
    coefficientFileName="thermocouple.h";
    its90BinFileName="its90.dat";
    its90TxtFileName="its90.txt";
    testIts90TxtFileName="compareIts90.txt";
    rc=read_data_from_file(sourceFileName);
    if(rc)
    {
        return rc;
    }
    rc=write_coefficients_to_file(coefficientFileName);
    if(rc)
    {
        return rc;
    }
    rc=write_its90_table_to_txt_file(its90TxtFileName);
    if(rc)
    {
        return rc;
    }
    rc=write_its90_table_to_bin_file(its90BinFileName);
    if(rc)
    {
        return rc;
    }
    rc=read_its90_table_from_bin_file(its90BinFileName);
    if(rc)
    {
        return rc;
    }
    rc=write_its90_table_to_txt_file(testIts90TxtFileName);
    if(rc)
    {
        return rc;
    }
    return 0;
}

