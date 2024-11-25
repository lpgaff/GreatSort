#include "great_sort.hh"
#include <iostream>

int ResetConv(){
	reset_conv_hists();
	std::cout << "Reset singles histograms" << std::endl;
	return 0;
}

int ResetEvnt(){
	reset_evnt_hists();
	std::cout << "Reset event builder stage histograms" << std::endl;
	return 0;
}

int ResetHist(){
	reset_phys_hists();
	std::cout << "Reset physics stage histograms" << std::endl;
	return 0;
}

int StopMonitor(){
	stop_monitor();
	std::cout << "Stop monitoring" << std::endl;
	return 0;
}

int StartMonitor(){
	start_monitor();
	std::cout << "Start monitoring" << std::endl;
	return 0;
}

