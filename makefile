all: RFRcvCmplxData

RFRcvCmplxData: ./rc-switch/RCSwitch.o RFRcvCmplxData.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -lwiringPi -lwiringPiDev -lcrypt

clean:
	$(RM) ../rc-switch/*.o *.o RFRcvCmplxData