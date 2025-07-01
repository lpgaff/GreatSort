#include "GreatEvts.hh"

ClassImp(GreatTACEvt)
ClassImp(GreatGammaRayEvt)
ClassImp(GreatDetectorEvt)
ClassImp(GreatEvts)

// Get minimum time from any old event
double GreatEvts::GetTime() const {

	double min_time = -1;

	// Check minimum time from all TAC events
	for( unsigned int i = 0; i < this->GetTACMultiplicity(); ++i ){

		double cur_time = this->GetTACEvt(i)->GetTime();
		if( cur_time < min_time || min_time < 0 )
			min_time = cur_time;

	}

	// Check minimum time from all gamma-ray events
	for( unsigned int i = 0; i < this->GetGammaRayMultiplicity(); ++i ){

		double cur_time = this->GetGammaRayEvt(i)->GetTime();
		if( cur_time < min_time || min_time < 0 )
			min_time = cur_time;

	}

	// if it's still not been set, make it zero
	if( min_time < 0 ) min_time = 0;

	return min_time;

}
