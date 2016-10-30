//
//  FtdiDmx.cpp
//  genericDmx
//
//  Created by Tim TVL on 15/03/15.
//
//

#include "ofxDmxFtdi.h"

ofxDmxFtdi::ofxDmxFtdi():
stopBitsType(SBITS_2), dataBitsType(DBITS_8), handle(0), bOpen(false)
{
    buffer = new unsigned char(513);
    
}

ofxDmxFtdi::~ofxDmxFtdi() {
    delete [] buffer;
//    if (bOpen) FT_Close(handle);
}

bool ofxDmxFtdi::open(const unsigned int number) {
    
    DWORD device_count = listDevices();
    if (device_count == 0) {
        ofLog(OF_LOG_VERBOSE, "No ftdi devices found");
        return false;
    } else if (number>device_count-1) {
        ofLog(OF_LOG_VERBOSE, "No such ftdi device");
        return false;
    }
    
    handle = 0;
    status = FT_Open(number, &handle);
    if (status == FT_OK) {
        bOpen = true;
    }
    initOpenDMX();
    
    //    startThread();
    //    setDmxValue(0, 0);  //Set DMX Start Code
}

unsigned int ofxDmxFtdi::listDevices() {
    
    unsigned int num = 0;
    status = FT_CreateDeviceInfoList(&num);
    if (status != FT_OK)
    {
        //        ofLog("CreateDeviceInfoList:" + ofToString((unsigned int)status));
        ofLog(OF_LOG_VERBOSE, "CreateDeviceInfoList:" +ofToString(status));
        //        return list;
    }
    else if (num <= 0)
    {
        ofLog(OF_LOG_VERBOSE, "No device: " +ofToString(num));
        //        return list;
    }
    
    ofLog(OF_LOG_VERBOSE, "Number of devices: " +ofToString(num));
    FT_DEVICE_LIST_INFO_NODE* devInfo = new FT_DEVICE_LIST_INFO_NODE[num];
    
    if (FT_GetDeviceInfoList(devInfo, &num) == FT_OK)
    {
        for (DWORD i = 0; i < num; i++)
        {
            string vendor, description, serial;
            FT_STATUS s = ftdi_get_strings(i, vendor, description, serial);
            if (s != FT_OK || description != "" || serial != "")
            {
                // Seems that some otherwise working devices don't provide
                // FT_PROGRAM_DATA struct used by qlcftdi_get_strings().
                description = string(devInfo[i].Description);
                serial = string(devInfo[i].SerialNumber);
                vendor = "";
            }
        }
    }
    
    if (num > 0) {
        // allocate storage for list based on numDevs
        devInfo = (FT_DEVICE_LIST_INFO_NODE*)malloc(sizeof(FT_DEVICE_LIST_INFO_NODE)*num);
        
        // get the device information list
        status = FT_GetDeviceInfoList(devInfo, &num);
        
        if (status == FT_OK) {
            for (int i = 0; i < num; i++) {
                ofLog(OF_LOG_VERBOSE, "Dev: " + ofToString(i));
                ofLog(OF_LOG_VERBOSE, " Flags: " + ofToString(devInfo[i].Flags));
                ofLog(OF_LOG_VERBOSE, " Type: " + ofToString(devInfo[i].Type));
                ofLog(OF_LOG_VERBOSE, " ID: " + ofToString(devInfo[i].ID));
                ofLog(OF_LOG_VERBOSE, " LocId: " + ofToString(devInfo[i].LocId));
                ofLog(OF_LOG_VERBOSE, " SerialNumber: " + ofToString(devInfo[i].SerialNumber));
                ofLog(OF_LOG_VERBOSE, " Description: " + ofToString(devInfo[i].Description));
                ofLog(OF_LOG_VERBOSE, " ftHandle: " + ofToString(devInfo[i].ftHandle));
            }
        }
    }
    
    return num;
}

bool ofxDmxFtdi::close() {
    status = FT_Close(handle);
    if (status == FT_OK) {
        bOpen = false;
        handle = NULL;
    }
}

bool ofxDmxFtdi::isOpen() const
{
    return bOpen;
}


//void ofxDmxFtdi::threadedFunction(){
//    while(isThreadRunning()){
//        lock();
//        initOpenDMX();
//        FT_SetBreakOn(handle);
//        FT_SetBreakOff(handle);
//        unsigned char buffer_[513];
//        memcpy(&data, &buffer_, length);
//        bytesWritten = write(handle, buffer, sizeof(buffer));
//        sleep(20);
//        unlock();
//    }
//}

int ofxDmxFtdi::writeDmx(const unsigned char * data, int length)
{
    //    LPVOID ptr = new void(length);
    //    lock();
    if(status != FT_OK) {
        // FT_Open failed
        return;
    }
    //    unlock();
    
    //    bytesWritten = write(handle, buffer, length);
    unsigned char buffer_[513];
    memcpy(buffer_, data, length);
    
    initOpenDMX();
    FT_SetBreakOn(handle);
    FT_SetBreakOff(handle);
    bytesWritten = 0;
    status = FT_Write(handle, (char *)data, length, &bytesWritten);
    
    return bytesWritten;
}

int ofxDmxFtdi::write(FT_HANDLE handle, const unsigned char * data, int length) {
    bytesWritten = 0;
    status = FT_Write(handle, (unsigned char *)buffer, sizeof(unsigned char)*length, &bytesWritten);
    return (int)bytesWritten;
}

bool ofxDmxFtdi::badChannel(unsigned int channel) {
    if(channel > levels.size()) {
        ofLogError() << "Channel " + ofToString(channel) + " is out of bounds. Only " + ofToString(levels.size()) + " channels are available.";
        return true;
    }
    if(channel == 0) {
//        ofLogError() << "Channel 0 does not exist. DMX channels start at 1.";
        return true;
    }
    return false;
}

void ofxDmxFtdi::setLevel(unsigned int channel, unsigned char level) {
    if(badChannel(channel)) {
        return;
    }
    //    channel--; // convert from 1-initial to 0-initial
    if(level != levels[channel]) {
        levels[channel] = level;
        needsUpdate = true;
    }
}

// ----------------------------------------

FT_STATUS ofxDmxFtdi::ftdi_get_strings(DWORD deviceIndex,
                                       string& vendor,
                                       string& description,
                                       string& serial)
{
    char cVendor[256];
    char cVendorId[256];
    char cDescription[256];
    char cSerial[256];
    
    FT_HANDLE handle;
    
    FT_STATUS status = FT_Open(deviceIndex, &handle);
    if (status != FT_OK)
        return status;
    
    FT_PROGRAM_DATA pData;
    pData.Signature1 = 0;
    pData.Signature2 = 0xFFFFFFFF;
    pData.Version = 0x00000005;
    pData.Manufacturer = cVendor;
    pData.ManufacturerId = cVendorId;
    pData.Description = cDescription;
    pData.SerialNumber = cSerial;
    status = FT_EE_Read(handle, &pData);
    if (status == FT_OK)
    {
        vendor = cVendor;
        description = cDescription;
        serial = cSerial;
    }
    
    FT_Close(handle);
    
    return status;
}


void ofxDmxFtdi::initOpenDMX()
{
    status = FT_ResetDevice(handle);
    status = FT_SetDivisor(handle, (char)12);  // set baud rate
    status = FT_SetDataCharacteristics(handle, DBITS_8, SBITS_2, PAR_NONE);
    status = FT_SetFlowControl(handle, (char)FLOW_NONE, 0, 0);
    status = FT_ClrRts(handle);
    status = FT_Purge(handle, PURGE_TX);
    status = FT_Purge(handle, PURGE_RX);
}

