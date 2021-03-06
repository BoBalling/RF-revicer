
# Defines the RPI variable which is needed by rc-switch/RCSwitch.h
CXXFLAGS=-DRPI

all: RFRcvCmplxData RFRcvCmplxDataMQTT

RFRcvCmplxData: ../rc-switch/RCSwitch.o RFRcvCmplxData.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -lwiringPi

RFRcvCmplxDataMQTT: ../rc-switch/RCSwitch.o RFRcvCmplxDataMQTT.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -lwiringPi
	
clean:
	$(RM) ../rc-switch/*.o *.o RFRcvCmplxData