
const unsigned char TC_CAL_SUCCESS=0;
const unsigned char TC_INVALID_TYPE=-1;
const unsigned char TC_CAL_OUT_OF_LOW_RANGE=-2;
const unsigned char TC_CAL_OUT_OF_HIGH_RANGE=-3;

const int TC_SUBRANGE_INIT_VALUE=-2;

int TcTypeRange(ThermalCoupleType type,double& temperature_range_low,double& temperatur_range_high, \
                    double& emf_range_low,double &emf_range_high)
{
    if(type>TC_TYPE_A)
    {
        return TC_INVALID_TYPE;
    }
    int tempRangeNum=tc_reference_function_subrange_num[type];
    int emfRangeNum=tc_inverse_function_subrange_num[type];

    temperature_range_low=tc_reference_function_subrange_base[type][0];
    temperatur_range_high=tc_reference_function_subrange_base[type][tempRangeNum];
    emf_range_low=tc_inverse_function_subrange_base[type][0];
    emf_range_high=tc_inverse_function_subrange_base[type][emfRangeNum];

    return TC_CAL_SUCCESS;
}

//calculate temperature with EMF, which is expressed in millivolt
//ITS-90 temperature is expressed in degrees Celsius (°C);
//return TC_CAL_SUCCESS if calculate right value;
//return TC_CAL_OUT_OF_LOW_RANGE if the input is out of low range;
//return TC_CAL_OUT_OF_HIGH_RANGE if the input is out of high range;
int TcEMFtoTwithRc(ThermalCoupleType type,double emf,double &temperature,float& error_range_low,float& error_range_high)
{
    int rc=TC_CAL_SUCCESS;
    int subrange;
    double calTemp;
    int coefficientsNum;

    if(type>TC_TYPE_A)
    {
        temperature=0.0f;
        return TC_INVALID_TYPE;
    }

    subrange=TC_SUBRANGE_INIT_VALUE;
    for(int i=0;i<(tc_inverse_function_subrange_num[type]+1);i++)
    {
        if(emf<tc_inverse_function_subrange_base[type][i])
        {
            subrange=i-1;
            break;
        }
    }

    if(subrange==-1)
    {
        subrange=0;
        emf=tc_inverse_function_subrange_base[type][0];
        rc=TC_CAL_OUT_OF_LOW_RANGE;
    }

    if(subrange==TC_SUBRANGE_INIT_VALUE)
    {
        subrange=tc_inverse_function_subrange_num[type]-1;
        emf=tc_inverse_function_subrange_base[type][subrange+1];
        rc=TC_CAL_OUT_OF_HIGH_RANGE;
    }

    coefficientsNum=tc_inverse_function_coefficients_num[type][subrange];
    calTemp=tc_inverse_function_coefficients[type][subrange][coefficientsNum-1];
    for(int i=coefficientsNum-1;i>0;i--)
    {
        calTemp=calTemp*emf+tc_inverse_function_coefficients[type][subrange][i-1];
    }

    temperature=calTemp;
    error_range_low=tc_inverse_function_error_range[type][subrange][ErrorRangeLowLimit];
    error_range_high=tc_inverse_function_error_range[type][subrange][ErrorRangeHighLimit];

    return rc;
}

//calculate temperature with EMF, which is expressed in millivolt
//return ITS-90 temperature, expressed in degrees Celsius (°C);
double TcEMFtoT(ThermalCoupleType type,double emf)
{
    double temperature;
    float errorRangeLow,errorRangeHigh;
    TcEMFtoTwithRc(type,emf,temperature,errorRangeLow,errorRangeHigh);
    return temperature;
}

//calculate EMF with ITS-90 temperature, which is expressed in degrees Celsius (°C)
//emf is expressed in millivolt;
//return TC_CAL_SUCCESS if calculate right value;
//return TC_CAL_OUT_OF_LOW_RANGE if the input is out of low range;
//return TC_CAL_OUT_OF_HIGH_RANGE if the input is out of high range;
int TcTtoEMFwithRc(ThermalCoupleType type,double temperature,double& emf)
{
    int rc=TC_CAL_SUCCESS;
    int subrange;
    double calEmf;
    int coefficientsNum;
    
    if(type>TC_TYPE_A)
    {
        emf=0.0f;
        return TC_INVALID_TYPE;
    }

    subrange=TC_SUBRANGE_INIT_VALUE;
    for(int i=0;i<(tc_reference_function_subrange_num[type]+1);i++)
    {
        if(temperature<tc_reference_function_subrange_base[type][i])
        {
            subrange=i-1;
            break;
        }
    }

    if(subrange==-1)
    {
        subrange=0;
        temperature=tc_reference_function_subrange_base[type][0];
        rc=TC_CAL_OUT_OF_LOW_RANGE;
    }

    if(subrange==TC_SUBRANGE_INIT_VALUE)
    {
        subrange=tc_reference_function_subrange_num[type]-1;
        temperature=tc_reference_function_subrange_base[type][subrange+1];
        rc=TC_CAL_OUT_OF_HIGH_RANGE;
    }


    coefficientsNum=tc_reference_function_coefficients_num[type][subrange];
    calEmf=tc_reference_function_coefficients[type][subrange][coefficientsNum-1];
    for(int i=coefficientsNum-1;i>0;i--)
    {
        calEmf=calEmf*temperature+tc_reference_function_coefficients[type][subrange][i-1];
    }
    
    const int TC_TYPE_K_EXP_SUBRANGE=1;
    if((type==TC_TYPE_K)||(subrange==TC_TYPE_K_EXP_SUBRANGE))
    {
        double calTemp;
        calTemp=tc_reference_function_typeK_exponential_const[1]*pow((temperature-tc_reference_function_typeK_exponential_const[2]),2);
        calEmf+=tc_reference_function_typeK_exponential_const[0]*exp(calTemp);
    }

    emf=calEmf;
    return rc;
}

//calculate EMF with ITS-90 temperature, which is expressed in degrees Celsius (°C)
//return EMF, expressed in millivolt;
double TcTtoEMF(ThermalCoupleType type,double temperature)
{
    double emf;
    TcTtoEMFwithRc(type,temperature,emf);
    return emf;
}