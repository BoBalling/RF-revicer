all: RFRcvCmplxData

RFRcvCmplxData: ./rc-switch/RCSwitch.o RFRcvCmplxData.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -lwiringPi

clean:
	$(RM) *.o RFRcvCmplxData