all: RFRcvCmplxData

RFRcvCmplxData: ../rc-switch/RCSwitch.o RFRcvCmplxData.o
   -o $@ -lwiringPi

clean:
    $(RM) *.o RFRcvCmplxData