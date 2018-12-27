all: RFRcvCmplxData

RFRcvCmplxData: RCSwitch.o RFRcvCmplxData.o
    $(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -lwiringPi

clean:
    $(RM) *.o RFRcvCmplxData