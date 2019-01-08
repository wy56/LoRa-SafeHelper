#include "mbed.h"
#include "mDot.h"
#include "MTSLog.h"
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <sstream>
using namespace std;

// define pin 
//DigitalOut led(LED1);
AnalogIn in(PA_5);


// set lora app eui and app key
static uint8_t config_app_eui[] = {0x86,0xe4,0xef,0xc7,0x10,0x4f,0x68,0x29 };
static uint8_t config_app_key[] = { 0xa3,0x46,0xb6,0xfa,0xef,0x2b,0xd3,0x3c,0x16,0xfe,0x9b,0x1d,0x8d,0x47,0xa1,0x1d};

// use the same subband as gateway that gateway can listen to you
static uint8_t config_frequency_sub_band = 1;
void config(mDot *dot)
{
    uint8_t ret;
    //  reset to default config so we know what state we're in  
    dot->resetConfig();
    //  set how many log info will be show 
    dot->setLogLevel(mts::MTSLog::INFO_LEVEL);
    // set subband frequency the same as gateway so gateway can listen to you 
    logInfo("setting frequency sub band\r\n");
    if ((ret = dot->setFrequencySubBand(config_frequency_sub_band)) != mDot::MDOT_OK) {
        logError("failed to set frequency sub band %d:%s\r\n", ret, mDot::getReturnCodeString(ret).c_str());
    }
    // lora has private network and public network here we use public network
    logInfo("setting public network mode");    
    if ((ret = dot->setPublicNetwork(true)) != mDot::MDOT_OK) {
        logError("failed to public network mode");
    }
    std::vector<uint8_t> temp;
    
    for (int i = 0; i < 8; i++) {
        temp.push_back(config_app_eui[i]);
    }
    // set network id 
    logInfo("setting app eui\r\n");
    if ((ret = dot->setNetworkId(temp)) != mDot::MDOT_OK) {
        logError("failed to set app eui %d:%s\r\n", ret, mDot::getReturnCodeString(ret).c_str());
    }
    temp.clear();
    for (int i = 0; i < 16; i++) {
        temp.push_back(config_app_key[i]);
    }
    // set network key
    logInfo("setting app key\r\n");
    if ((ret = dot->setNetworkKey(temp)) != mDot::MDOT_OK) {
        logError("failed to set app key %d:%s\r\n", ret, mDot::getReturnCodeString(ret).c_str());
    }
    // a higher spreading factor allows for longer range but lower throughput
    // in the 915 (US) frequency band, spreading factors 7 - 10 are available
    logInfo("setting TX spreading factor\r\n");
    if ((ret = dot->setTxDataRate(mDot::SF_10)) != mDot::MDOT_OK) {
        logError("failed to set TX datarate %d:%s\r\n", ret, mDot::getReturnCodeString(ret).c_str());
    }
    // request receive confirmation of packets from the gateway
    logInfo("enabling ACKs\r\n");
    if ((ret = dot->setAck(1)) != mDot::MDOT_OK) {
        logError("failed to enable ACKs %d:%s\r\n", ret, mDot::getReturnCodeString(ret).c_str());
    }
    // Set Tx Power
    logInfo("enabling Tx Power\r\n");
    if ((ret = dot->setTxPower(20)) != mDot::MDOT_OK) {
        logError("failed to enable Tx Power %d:%s\r\n", ret, mDot::getReturnCodeString(ret).c_str());
    }
    // request receive confirmation of packets from the gateway
    logInfo("enabling Tx Data Rate\r\n");
    if ((ret = dot->setTxDataRate(3)) != mDot::MDOT_OK) {
        logError("failed to enable Tx Data Rate %d:%s\r\n", ret, mDot::getReturnCodeString(ret).c_str());
    }
    // save this configuration to the mDot's NVM
    logInfo("saving config\r\n");
    if (! dot->saveConfig()) {
        logError("failed to save configuration\r\n");
    }
    logInfo("joining network\r\n");
    while ((ret = dot->joinNetwork()) != mDot::MDOT_OK) {
        logError("failed to join network %d:%s\r\n", ret, mDot::getReturnCodeString(ret).c_str());
        osDelay(std::max((uint32_t)1000, (uint32_t)dot->getNextTxMs()));
    }
    return;
}
int main()
{
    // object to control the debug board
    mDot *dot; 
    dot = mDot::getInstance();
    
    // set network
    config(dot);    

    while (true) {
        int tmp,ret;
        std::vector<uint8_t> data;
        stringstream ss;
        std::string data_str ;
        
        tmp = in.read_u16();
        printf("%d\r\n",tmp);
        
        // format data for sending to the gateway
        ss << tmp;
        ss >> data_str;
        for (std::string::iterator it = data_str.begin(); it != data_str.end(); it++)
            data.push_back((uint8_t) *it);
        
        // send the data to the gateway
        if ((ret = dot->send(data)) != mDot::MDOT_OK) {
            logError("failed to send\r\n", ret, mDot::getReturnCodeString(ret).c_str());
        } else {
            logInfo("successfully sent data to gateway\r\n");
        }

        // we use US but in the 868 (EU) frequency band, we need to wait until another channel is available before transmitting again
        osDelay(std::max((uint32_t)5000, (uint32_t)dot->getNextTxMs()));
    }

    return 0;

}



