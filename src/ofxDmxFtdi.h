//
//  FtdiDmx.h
//  genericDmx
//
//  Created by Tim TVL on 15/03/15.
//
//

#ifndef __genericDmx__FtdiDmx__
#define __genericDmx__FtdiDmx__

#include <stdio.h>
#include "ftd2xx.h"
#include "ofMain.h"

static const Byte PURGE_RX = 1;
static const Byte PURGE_TX = 2;

class ofxDmxFtdi : public ofThread {
    
public:
    
    enum FTDI_BUFFER_TYPE { RX_TX_BUFFER, RX_BUFFER, TX_BUFFER };
    
    /* NOTE: enum members must have different names than in libftdi in order to
     access them. Such beauty. :S */
    enum FTDI_DATABITS_TYPE { DBITS_7 = FT_BITS_7, DBITS_8 = FT_BITS_8 };
    
    enum FTDI_STOPBITS_TYPE {
        SBITS_1 = FT_STOP_BITS_1, SBITS_2 = FT_STOP_BITS_2
    };
    
    enum FTDI_PARITY_TYPE {
        PAR_NONE = FT_PARITY_NONE, PAR_ODD = FT_PARITY_ODD, PAR_EVEN = FT_PARITY_EVEN, PAR_MARK = FT_PARITY_MARK, PAR_SPACE = FT_PARITY_SPACE
    };
    
    enum FTDI_BREAK_TYPE { BRK_ON = 1, BRK_OFF = 0 };
    //	FT_SetBreakOn
    
    enum FTDI_FLOWCTL_TYPE {
        FLOW_NONE = FT_FLOW_NONE, FLOW_RTS_CTS = FT_FLOW_RTS_CTS,
        FLOW_DTR_DSR = FT_FLOW_DTR_DSR, FLOW_XON_XOFF = FT_FLOW_XON_XOFF
    };
    
    ofxDmxFtdi();
    ~ofxDmxFtdi();
    
    /** Open the widget */
    bool open(const unsigned int number=0);
    
    void initOpenDMX();
    
    /** Close the widget */
    bool close();
    
    /** Check if the widget is open */
    bool isOpen() const;
    
    unsigned int listDevices();
    
    //    /** Reset the communications line */
    //    bool reset();
    //
    //    * Setup communications line for 8N2 traffic
    //    bool setLineProperties();
    //
    //    /** Set 250kbps baud rate */
    //    bool setBaudRate();
    //
    //    /** Disable flow control */
    //    bool setFlowControl();
    //
    //    /** Clear the RTS bit */
    //    bool clearRts();
    //
    //    /** Purge TX & RX buffers */
    //    bool purgeBuffers();
    //
    //    /** Toggle communications line BREAK condition on/off */
    //    bool setBreak(bool on);
    
    /** Write data to a previously-opened line */
    
    int writeDmx(const unsigned char* data, int length);
    
    bool badChannel(unsigned int channel);
    void setLevel(unsigned int channel, unsigned char level);
    
    static FT_STATUS ftdi_get_strings(DWORD deviceIndex,
                                      string& vendor,
                                      string& description,
                                      string& serial);
    
protected:
//    void threadedFunction();
    int write(FT_HANDLE handle, const unsigned char* data, int length);
    
private:
    FT_STATUS status;
    FTDI_STOPBITS_TYPE stopBitsType;
    FTDI_DATABITS_TYPE dataBitsType;
    FT_HANDLE handle;
    unsigned int bytesWritten;
    unsigned char * buffer;
    
    bool bOpen, needsUpdate;
    vector<unsigned char> levels;
};


#endif /* defined(__genericDmx__FtdiDmx__) */
