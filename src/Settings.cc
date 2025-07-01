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
	cebr3_hit_window = config->GetValue( "CeBr3HitWindow", 500 );
	hpge_hit_window = config->GetValue( "HPGeHitWindow", 500 );

	
	// Data things
	block_size = config->GetValue( "DataBlockSize", 0x10000 );
	flag_caen_only = config->GetValue( "CAENOnlyData", false );

	
	// TAC modules
	n_tacs = config->GetValue( "NumberOfTACModules", 0 );
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

	
	// CeBr3
	n_cebr3_detector = config->GetValue( "NumberOfCeBr3Detectors", 0 );
	cebr3_mod.resize( n_cebr3_detector );
	cebr3_ch.resize( n_cebr3_detector );
	cebr3_id.resize( n_caen_mod );
	for( unsigned int i = 0; i < n_caen_mod; ++i )
		for( unsigned int j = 0; j < n_caen_ch; ++j )
			cebr3_id[i].push_back( -1 );


	for( unsigned int i = 0; i < n_cebr3_detector; ++i ){

		cebr3_mod[i] = config->GetValue( Form( "CeBr3_%d.Module", i ), (int)0 );
		cebr3_ch[i]  = config->GetValue( Form( "CeBr3_%d.Channel", i ), (int)i );

		if( cebr3_mod[i] < n_caen_mod && cebr3_ch[i] < n_caen_ch )
			cebr3_id[cebr3_mod[i]][cebr3_ch[i]] = i;

		else {

			std::cerr << "Dodgy CeBr3 settings: module = " << cebr3_mod[i];
			std::cerr << " channel = " << cebr3_ch[i] << std::endl;

		}

	}


	// HPGe
	n_hpge_detector = config->GetValue( "NumberOfHPGeDetectors", 0 );
	n_hpge_segments = config->GetValue( "NumberOfHPGeSegments", 1 );
	hpge_mod.resize( n_hpge_detector );
	hpge_ch.resize( n_hpge_detector );
	hpge_id.resize( n_caen_mod );
	hpge_seg.resize( n_caen_mod );
	for( unsigned int i = 0; i < n_caen_mod; ++i ) {
		hpge_id[i].resize( n_caen_ch, -1 );
		hpge_seg[i].resize( n_caen_ch, -1 );
	}


	for( unsigned int i = 0; i < n_hpge_detector; ++i ){

		hpge_mod[i].resize( n_hpge_segments );
		hpge_ch[i].resize( n_hpge_segments );

		for( unsigned int j = 0; j < n_hpge_segments; ++j ){

			hpge_mod[i][j] = config->GetValue( Form( "HPGe_%d_%d.Module", i, j ), (int)0 );
			hpge_ch[i][j]  = config->GetValue( Form( "HPGe_%d_%d.Channel", i, j ), (int)(i*n_hpge_segments+j) );

			if( hpge_mod[i][j] < n_caen_mod && hpge_ch[i][j] < n_caen_ch ){

				hpge_id[hpge_mod[i][j]][hpge_ch[i][j]] = i;
				hpge_seg[hpge_mod[i][j]][hpge_ch[i][j]] = j;

			}

			else {

				std::cerr << "Dodgy HPGe settings: module = " << hpge_mod[i][j];
				std::cerr << " channel = " << hpge_ch[i][j] << std::endl;

			}

		} // j

	} // i


	// Finished
	delete config;
	
}


bool GreatSettings::IsTAC( unsigned char mod, unsigned char ch ) {
	
	/// Return true if this is a TAC event
	if( tac_id[(int)mod][(int)ch] >= 0) return true;
	else return false;
	
}

short GreatSettings::GetTACID( unsigned char mod, unsigned char ch ) {

	/// Return the TAC ID  by module and channel number
	if( mod < n_caen_mod && ch < n_caen_ch )
		return tac_id[(int)mod][(int)ch];
	
	else {
		
		std::cerr << "Bad TAC event: module = " << (int)mod;
		std::cerr << " channel = " << (int)ch << std::endl;
		return -1;
		
	}
	
}

bool GreatSettings::IsCeBr3( unsigned char mod, unsigned char ch ) {

	/// Return true if this is an CeBr3 event
	if( cebr3_id[(int)mod][(int)ch] >= 0 ) return true;
	else return false;
	
}


short GreatSettings::GetCeBr3Detector( unsigned char mod, unsigned char ch ) {

	/// Return the detector ID of a CeBr3 event by module and channel number
	if( mod < n_caen_mod && ch < n_caen_ch )
		return cebr3_id[(int)mod][(int)ch];

	else {
		
		std::cerr << "Bad CeBr3 event: module = " << (int)mod;
		std::cerr << " channel = " << (int)ch << std::endl;
		return -1;
		
	}
	
}

bool GreatSettings::IsHPGe( unsigned char mod, unsigned char ch ) {

	/// Return true if this is an HPGe event
	if( hpge_id[(int)mod][(int)ch] >= 0 ) return true;
	else return false;

}

short GreatSettings::GetHPGeDetector( unsigned char mod, unsigned char ch ) {

	/// Return the detector ID of a HPGe event by module and channel number
	if( mod < n_caen_mod && ch < n_caen_ch )
		return hpge_id[(int)mod][(int)ch];

	else {

		std::cerr << "Bad HPGe event: module = " << (int)mod;
		std::cerr << " channel = " << (int)ch << std::endl;
		return -1;

	}

}

short GreatSettings::GetHPGeSegment( unsigned char mod, unsigned char ch ) {

	/// Return the segment ID of a HPGe event by module and channel number
	if( mod < n_caen_mod && ch < n_caen_ch )
		return hpge_seg[(int)mod][(int)ch];

	else {

		std::cerr << "Bad HPGe event: module = " << (int)mod;
		std::cerr << " channel = " << (int)ch << std::endl;
		return -1;

	}

}
