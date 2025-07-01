#include "DataPackets.hh"

ClassImp(GreatCaenData)
ClassImp(GreatInfoData)
ClassImp(GreatDataPackets)

GreatCaenData::GreatCaenData(){}
GreatCaenData::~GreatCaenData(){}

GreatInfoData::GreatInfoData(){}
GreatInfoData::~GreatInfoData(){}


void GreatDataPackets::SetData( std::shared_ptr<GreatCaenData> data ){
	
	// Reset the vector to size = 0
	// We only want to have one element per Tree entry
	ClearData();
	
	// Make a copy of the input data and push it back
	GreatCaenData fill_data;
	
	fill_data.SetTimeStamp( data->GetTimeStamp() );
	fill_data.SetFineTime( data->GetFineTime() );
	fill_data.SetTrace( data->GetTrace() );
	fill_data.SetQlong( data->GetQlong() );
	fill_data.SetQshort( data->GetQshort() );
	fill_data.SetModule( data->GetModule() );
	fill_data.SetChannel( data->GetChannel() );
	fill_data.SetEnergy( data->GetEnergy() );
	fill_data.SetThreshold( data->IsOverThreshold() );
	
	//std::cout << fill_data.GetTimeStamp() << "\t" << fill_data.GetFineTime() << std::endl;

	caen_packets.push_back( fill_data );

	//std::cout << caen_packets[0].GetTimeStamp() << "\t" << caen_packets[0].GetFineTime() << std::endl;

}

void GreatDataPackets::SetData( std::shared_ptr<GreatInfoData> data ){
	
	// Reset the vector to size = 0
	// We only want to have one element per Tree entry
	ClearData();
	
	// Make a copy of the input data and push it back
	GreatInfoData fill_data;
	fill_data.SetTimeStamp( data->GetTime() );
	fill_data.SetCode( data->GetCode() );
	fill_data.SetModule( data->GetModule() );

	info_packets.push_back( fill_data );
	
}

void GreatDataPackets::ClearData(){
	
	caen_packets.clear();
	info_packets.clear();
	
	std::vector<GreatCaenData>().swap(caen_packets);
	std::vector<GreatInfoData>().swap(info_packets);

	return;
	
}

double GreatDataPackets::GetTime() const {

	if( IsCaen() ) return GetCaenData()->GetTime();
	if( IsInfo() ) return GetInfoData()->GetTime();

	return 0;
	
}

unsigned long long GreatDataPackets::GetTimeStamp() const {

	if( IsCaen() ) return GetCaenData()->GetTimeStamp();
	if( IsInfo() ) return GetInfoData()->GetTimeStamp();

	return 0;
	
}

UInt_t GreatDataPackets::GetTimeMSB() const {

	return ( ((unsigned long long)this->GetTimeStamp() >> 32) & 0xFFFFFFFF );
	
}

UInt_t GreatDataPackets::GetTimeLSB() const {

	return (UInt_t)this->GetTimeStamp();
	
}

void GreatCaenData::ClearData(){
	
	timestamp = 0.0;
	finetime = 0.0;
	baseline = 0.0;
	trace.clear();
	Qlong = 0;
	Qshort = 0;
	mod = 255;
	ch = 255;
	energy = -999.;
	thres = true;

	return;
	
}

void GreatInfoData::ClearData(){
	
	timestamp = 0.0;
	code = 0;
	mod = 255;
	
	return;
	
}

