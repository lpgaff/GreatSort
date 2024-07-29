#include "GreatEvts.hh"

ClassImp(GreatTACEvt)
ClassImp(GreatGammaRayEvt)
ClassImp(GreatEvts)


// ---------- //
// Great events //
// ---------- //
GreatEvts::GreatEvts(){}
GreatEvts::~GreatEvts(){}

void GreatEvts::ClearEvt() {
	
	tac_event.clear();
	gamma_event.clear();

	std::vector<GreatTACEvt>().swap(tac_event);
	std::vector<GreatGammaRayEvt>().swap(gamma_event);
	
	return;

}

void GreatEvts::AddEvt( std::shared_ptr<GreatTACEvt> event ) {
	
	// Make a copy of the event and push it back
	GreatTACEvt fill_evt;
	fill_evt.SetEvent( event->GetTACTime(),
					   event->GetID(),
					   event->GetTime() );
	
	tac_event.push_back( fill_evt );
	
}

void GreatEvts::AddEvt( std::shared_ptr<GreatGammaRayEvt> event ) {
	
	// Make a copy of the event and push it back
	GreatGammaRayEvt fill_evt;
	fill_evt.SetEvent( event->GetEnergy(),
					   event->GetID(),
					   event->GetType(),
					   event->GetTime() );
	
	gamma_event.push_back( fill_evt );
	
}


// ----------- //
// TAC events //
// ----------- //
GreatTACEvt::GreatTACEvt(){}
GreatTACEvt::~GreatTACEvt(){}

void GreatTACEvt::SetEvent( int mytactd, unsigned char mytacid,
						double mytime ) {
	
	tactd = mytactd;
	id = mytacid;
	time = mytime;
	
	return;
	
}


// ---------------- //
// Gamma-ray events //
// ---------------- //
GreatGammaRayEvt::GreatGammaRayEvt(){}
GreatGammaRayEvt::~GreatGammaRayEvt(){}

void GreatGammaRayEvt::SetEvent( float myenergy, unsigned char myid,
							  unsigned char mytype, double mytime ) {
	
	energy = myenergy;
	id = myid;
	type = mytype;
	time = mytime;
	
	return;
	
}


// Get minimum time from any old event
double GreatEvts::GetTime(){
	
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
