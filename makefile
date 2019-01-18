
# Defines the RPI variable which is needed by rc-switch/RCSwitch.h
CXXFLAGS=-DRPI

all: RFRcvCmplxData

RFRcvCmplxData: ./rc-switch/RCSwitch.o RFRcvCmplxData.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -lwiringPi -lcurl

clean:
	$(RM) ../rc-switch/*.o *.o RFRcvCmplxData