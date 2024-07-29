#include "Settings.hh"

#include <iomanip>

GreatSettings::GreatSettings( std::string filename ) {
	
	SetFile( filename );
	ReadSettings();
	
}

void GreatSettings::ReadSettings() {
	
	TEnv *config = new TEnv( fInputFile.data() );
	
	// CAEN initialisation
	n_caen_mod = config->GetValue( "NumberOfCAENModules", 2 );
	n_caen_ch = config->GetValue( "NumberOfCAENChannels", 16 );
	for( unsigned int i = 0; i < n_caen_mod; ++i ) {
		
		caen_model.push_back( config->GetValue( Form( "CAEN_%d.Model", i ), 1725 ) );
		caen_fw.push_back( config->GetValue( Form( "CAEN_%d.Firmware", i ), "PSD" ) );
	
	}
	
	
	// Info code initialisation
	sync_code = config->GetValue( "SyncCode", 4 );
	thsb_code = config->GetValue( "TimestampCode", 5 );

	
	// Event builder
	event_window = config->GetValue( "EventWindow", 3e3 );
	gamma_hit_window = config->GetValue( "GammaRayHitWindow", 500 );

	
	// Data things
	block_size = config->GetValue( "DataBlockSize", 0x10000 );
	flag_caen_only = config->GetValue( "CAENOnlyData", false );

	
	// TAC modules
	n_tacs = config->GetValue( "NumberOfTACModules", 1 );
	tac_mod.resize( n_tacs );
	tac_ch.resize( n_tacs );
	tac_id.resize( n_caen_mod );
	for( unsigned int i = 0; i < n_caen_mod; ++i )
		for( unsigned int j = 0; j < n_caen_ch; ++j )
			tac_id[i].push_back( -1 );

	for( unsigned int i = 0; i < n_tacs; ++i ){
				
		tac_mod[i] = config->GetValue( Form( "TAC_%d.Module", i ), (int)0 );
		tac_ch[i]  = config->GetValue( Form( "TAC_%d.Channel", i ), (int)2 );
		
		if( tac_mod[i] < n_caen_mod && tac_ch[i] < n_caen_ch )
			tac_id[tac_mod[i]][tac_ch[i]] = i;

		else {
			
			std::cerr << "Dodgy TAC settings: module = " << tac_mod[i];
			std::cerr << " channel = " << tac_ch[i] << std::endl;
			
		}
			
	}

	
	// Gamma-rays
	n_gamma_detector = config->GetValue( "NumberOfGammaRayDetectors", 2 );
	gamma_mod.resize( n_gamma_detector );
	gamma_ch.resize( n_gamma_detector );
	gamma_id.resize( n_caen_mod );
	for( unsigned int i = 0; i < n_caen_mod; ++i )
		for( unsigned int j = 0; j < n_caen_ch; ++j )
			gamma_id[i].push_back( -1 );

	
	for( unsigned int i = 0; i < n_gamma_detector; ++i ){

		gamma_mod[i] = config->GetValue( Form( "GammaRay_%d.Module", i ), (int)0 );
		gamma_ch[i]  = config->GetValue( Form( "GammaRay_%d.Channel", i ), (int)i );
		
		if( gamma_mod[i] < n_caen_mod && gamma_ch[i] < n_caen_ch )
			gamma_id[gamma_mod[i]][gamma_ch[i]] = i;

		else {
			
			std::cerr << "Dodgy gamma-ray settings: module = " << gamma_mod[i];
			std::cerr << " channel = " << gamma_ch[i] << std::endl;
			
		}
		
	}
	

	// Finished
	delete config;
	
}


bool GreatSettings::IsTAC( unsigned char mod, unsigned char ch ) {
	
	/// Return true if this is a TAC event
	if( tac_id[(int)mod][(int)ch] >= 0) return true;
	else return false;
	
}

char GreatSettings::GetTACID( unsigned char mod, unsigned char ch ) {
	
	/// Return the TAC ID  by module and channel number
	if( mod < n_caen_mod && ch < n_caen_ch )
		return tac_id[(int)mod][(int)ch];
	
	else {
		
		std::cerr << "Bad TAC event: module = " << (int)mod;
		std::cerr << " channel = " << (int)ch << std::endl;
		return -1;
		
	}
	
}

bool GreatSettings::IsGammaRay( unsigned char mod, unsigned char ch ) {
	
	/// Return true if this is an GammaRay event
	if( gamma_id[(int)mod][(int)ch] >= 0 ) return true;
	else return false;
	
}


char GreatSettings::GetGammaRayDetector( unsigned char mod, unsigned char ch ) {
	
	/// Return the detector ID of a GammaRay event by module and channel number
	if( mod < n_caen_mod && ch < n_caen_ch )
		return gamma_id[(int)mod][(int)ch];
	
	else {
		
		std::cerr << "Bad gamma-ray event: module = " << (int)mod;
		std::cerr << " channel = " << (int)ch << std::endl;
		return -1;
		
	}
	
}
