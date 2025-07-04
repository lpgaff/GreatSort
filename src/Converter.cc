#include "Converter.hh"

GreatConverter::GreatConverter( std::shared_ptr<GreatSettings> myset ) {

	// We need to do initialise, but only after Settings are added
	set = myset;

	my_tm_stp_msb = 0;
	my_tm_stp_hsb = 0;
	
	// Resize counters
	ctr_caen_hit.resize( set->GetNumberOfCAENModules() );
	ctr_caen_ext.resize( set->GetNumberOfCAENModules() );

	// Start counters at zero
	StartFile();
	
	// Default that we do not have a source only run
	flag_source = false;
	
	// No progress bar by default
	_prog_ = false;
	
}

void GreatConverter::StartFile(){
	
	// Start counters at zero
	for( unsigned int i = 0; i < set->GetNumberOfCAENModules(); ++i ) {
				
		ctr_caen_hit[i] = 0;	// hits on each module
		ctr_caen_ext[i] = 0;	// external timestamps

	}
	
	return;
	
}

void GreatConverter::SetOutput( std::string output_file_name ){

	// Open output file
	output_file = new TFile( output_file_name.data(), "recreate" );
	//if( !flag_source ) output_file->SetCompressionLevel(0);

	return;

};


void GreatConverter::MakeTree() {

	// Create Root tree
	const int splitLevel = 2; // don't split branches = 0, full splitting = 99
	const int bufsize = sizeof(GreatCaenData) + sizeof(GreatInfoData);
	sorted_tree = new TTree( "great_sort", "Time sorted, calibrated Great data" );
	write_packet = std::make_shared<GreatDataPackets>();
	sorted_tree->Branch( "data", "GreatDataPackets", write_packet.get(), bufsize, splitLevel );
	sorted_tree->SetDirectory( output_file->GetDirectory("/") );

	caen_data = std::make_shared<GreatCaenData>();
	info_data = std::make_shared<GreatInfoData>();

	caen_data->ClearData();
	info_data->ClearData();
	
	return;
	
}

void GreatConverter::MakeHists() {
	
	std::string hname, htitle;
	std::string dirname, maindirname, subdirname;
	
	// Make directories
	maindirname = "caen_hists";
	
	// Resive vectors
	hcaen_qlong.resize( set->GetNumberOfCAENModules() );
	hcaen_qshort.resize( set->GetNumberOfCAENModules() );
	hcaen_qdiff.resize( set->GetNumberOfCAENModules() );
	hcaen_cal.resize( set->GetNumberOfCAENModules() );

	// Loop over CAEN modules
	for( unsigned int i = 0; i < set->GetNumberOfCAENModules(); ++i ) {
		
		hcaen_qlong[i].resize( set->GetNumberOfCAENChannels() );
		hcaen_qshort[i].resize( set->GetNumberOfCAENChannels() );
		hcaen_qdiff[i].resize( set->GetNumberOfCAENChannels() );
		hcaen_cal[i].resize( set->GetNumberOfCAENChannels() );
		dirname = maindirname + "/module_" + std::to_string(i);
		
		if( !output_file->GetDirectory( dirname.data() ) )
			output_file->mkdir( dirname.data() );
		output_file->cd( dirname.data() );

		// Loop over channels of each CAEN module
		for( unsigned int j = 0; j < set->GetNumberOfCAENChannels(); ++j ) {
			
			// Uncalibrated - Qlong
			hname = "caen_" + std::to_string(i);
			hname += "_" + std::to_string(j);
			hname += "_qlong";
			
			htitle = "Raw CAEN V1725 spectra for module " + std::to_string(i);
			htitle += ", channel " + std::to_string(j);
			
			htitle += ";Qlong;Counts";
			
			if( output_file->GetListOfKeys()->Contains( hname.data() ) )
				hcaen_qlong[i][j] = (TH1F*)output_file->Get( hname.data() );
			
			else {
				
				hcaen_qlong[i][j] = new TH1F( hname.data(), htitle.data(),
										   65536, -0.5, 65535.5 );
			
				hcaen_qlong[i][j]->SetDirectory(
						output_file->GetDirectory( dirname.data() ) );
				
			}
			
			// Uncalibrated - Qshort
			hname = "caen_" + std::to_string(i);
			hname += "_" + std::to_string(j);
			hname += "_qshort";
			
			htitle = "Raw CAEN V1725 spectra for module " + std::to_string(i);
			htitle += ", channel " + std::to_string(j);
			
			htitle += ";Qshort;Counts";
			
			if( output_file->GetListOfKeys()->Contains( hname.data() ) )
				hcaen_qshort[i][j] = (TH1F*)output_file->Get( hname.data() );
			
			else {
				
				hcaen_qshort[i][j] = new TH1F( hname.data(), htitle.data(),
										   32768, -0.5, 32767.5 );
			
				hcaen_qshort[i][j]->SetDirectory(
						output_file->GetDirectory( dirname.data() ) );
				
			}
			
			// Uncalibrated - Qshort
			hname = "caen_" + std::to_string(i);
			hname += "_" + std::to_string(j);
			hname += "_qdiff";
			
			htitle = "Raw CAEN V1725 spectra for module " + std::to_string(i);
			htitle += ", channel " + std::to_string(j);
			
			htitle += ";Qdiff;Counts";
			
			if( output_file->GetListOfKeys()->Contains( hname.data() ) )
				hcaen_qdiff[i][j] = (TH1F*)output_file->Get( hname.data() );
			
			else {
				
				hcaen_qdiff[i][j] = new TH1F( hname.data(), htitle.data(),
										   65536, -0.5, 65535.5 );
			
				hcaen_qdiff[i][j]->SetDirectory(
						output_file->GetDirectory( dirname.data() ) );
				
			}
			
			// Calibrated
			hname = "caen_" + std::to_string(i);
			hname += "_" + std::to_string(j);
			hname += "_cal";
			
			htitle = "Calibrated CAEN V1725 spectra for module " + std::to_string(i);
			htitle += ", channel " + std::to_string(j);
			
			htitle += ";Energy (keV);Counts per 10 keV";
			
			if( output_file->GetListOfKeys()->Contains( hname.data() ) )
				hcaen_cal[i][j] = (TH1F*)output_file->Get( hname.data() );
			
			else {
				
				hcaen_cal[i][j] = new TH1F( hname.data(), htitle.data(),
										   4000, -5, 39995 );
			
				hcaen_cal[i][j]->SetDirectory(
						output_file->GetDirectory( dirname.data() ) );
				
			}

			
		}
					
	}
	
	// Make directories
	dirname = "timing_hists";
	if( !output_file->GetDirectory( dirname.data() ) )
		output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );
	
	// Resize vectors
	hcaen_hit.resize( set->GetNumberOfCAENModules() );
	hcaen_ext.resize( set->GetNumberOfCAENModules() );
	
	// Loop over CAEN modules
	for( unsigned int i = 0; i < set->GetNumberOfCAENModules(); ++i ) {
		
		hname = "hcaen_hit" + std::to_string(i);
		htitle = "Profile of ts versus hit_id in CAEN module " + std::to_string(i);

		if( output_file->GetListOfKeys()->Contains( hname.data() ) )
			hcaen_hit[i] = (TProfile*)output_file->Get( hname.data() );

		else {

			hcaen_hit[i] = new TProfile( hname.data(), htitle.data(), 10800, 0., 1080000. );
			hcaen_hit[i]->SetDirectory(
					output_file->GetDirectory( dirname.data() ) );

		}

	
		hname = "hcaen_ext" + std::to_string(i);
		htitle = "Profile of external trigger ts versus hit_id in CAEN module " + std::to_string(i);

		if( output_file->GetListOfKeys()->Contains( hname.data() ) )
			hcaen_ext[i] = (TProfile*)output_file->Get( hname.data() );

		else {

			hcaen_ext[i] = new TProfile( hname.data(), htitle.data(), 10800, 0., 108000. );
			hcaen_ext[i]->SetDirectory(
					output_file->GetDirectory( dirname.data() ) );

		}

	}
	
	return;
	
}

void GreatConverter::ResetHists() {
	
	std::cout << "in GreatConverter::ResetHist()" << std::endl;
	
	for( unsigned int i = 0; i < hcaen_hit.size(); ++i )
		hcaen_hit[i]->Reset("ICESM");

	for( unsigned int i = 0; i < hcaen_ext.size(); ++i )
		hcaen_ext[i]->Reset("ICESM");
	
	for( unsigned int i = 0; i < hcaen_qlong.size(); ++i )
		for( unsigned int j = 0; j < hcaen_qlong[i].size(); ++j )
			hcaen_qlong[i][j]->Reset("ICESM");
	
	for( unsigned int i = 0; i < hcaen_qshort.size(); ++i )
		for( unsigned int j = 0; j < hcaen_qshort[i].size(); ++j )
			hcaen_qshort[i][j]->Reset("ICESM");
	
	for( unsigned int i = 0; i < hcaen_qdiff.size(); ++i )
		for( unsigned int j = 0; j < hcaen_qdiff[i].size(); ++j )
			hcaen_qdiff[i][j]->Reset("ICESM");
	
	for( unsigned int i = 0; i < hcaen_cal.size(); ++i )
		for( unsigned int j = 0; j < hcaen_cal[i].size(); ++j )
			hcaen_cal[i][j]->Reset("ICESM");
	
	return;
	
}

// Function to copy the header from a DataSpy, for example
void GreatConverter::SetBlockHeader( char *input_header ){
	
	// Copy header
	for( unsigned int i = 0; i < HEADER_SIZE; i++ )
		block_header[i] = input_header[i];

	return;
	
}

// Function to process header words
void GreatConverter::ProcessBlockHeader( unsigned long nblock ){
		
	// For each new header, reset the swap mode
	swap = 0;
	
	// Flags for CAEN data items
	flag_caen_data0 = false;
	flag_caen_data1 = false;
	flag_caen_data2 = false;
	flag_caen_data3 = false;
	flag_caen_trace = false;

	// Flag when we find the end of the data
	flag_terminator = false;

	// Process header.
	for( UInt_t i = 0; i < 8; i++ )
		header_id[i] = block_header[i];
	
	header_sequence =
	(block_header[8] & 0xFF) << 24 | (block_header[9]& 0xFF) << 16 |
	(block_header[10]& 0xFF) << 8  | (block_header[11]& 0xFF);
	
	header_stream = (block_header[12] & 0xFF) << 8 | (block_header[13]& 0xFF);
	
	header_tape = (block_header[14] & 0xFF) << 8 | (block_header[15]& 0xFF);
	
	header_MyEndian = (block_header[16] & 0xFF) << 8 | (block_header[17]& 0xFF);
	
	header_DataEndian = (block_header[18] & 0xFF) << 8 | (block_header[19]& 0xFF);
	
	header_DataLen =
	(block_header[20] & 0xFF) | (block_header[21]& 0xFF) << 8 |
	(block_header[22] & 0xFF) << 16  | (block_header[23]& 0xFF) << 24 ;
	

	if( std::string(header_id).substr(0,8) != "EBYEDATA" ) {
	
		std::cerr << "Bad header in block " << nblock << std::endl;
		exit(0);
	
	}
	
	return;
	
}


// Function to copy the main data from a DataSpy, for example
void GreatConverter::SetBlockData( char *input_data ){
	
	// Copy header
	for( UInt_t i = 0; i < MAIN_SIZE; i++ )
		block_data[i] = input_data[i];

	return;
	
}


// Function to process data words
void GreatConverter::ProcessBlockData( unsigned long nblock ){

	(void) nblock; // Avoid unused parameter warning.
	
	if( nblock < 10 ) {
		flag_terminator = true;
		return;
	}
	
	// Get the data in 64-bit words and check endieness and swap if needed
	// Data format here: http://npg.dl.ac.uk/documents/edoc504/edoc504.html
	// Unpack in to two 32-bit words for purposes of data format
		
	// Swap mode is unknown for the first block of data, so let's work it out
	if( (swap & SWAP_KNOWN) == 0 ) {
		
		// See if we can figure out the swapping - the DataEndian word of the
		// header is 256 if the endianness is correct, otherwise swap endianness
		if( header_DataEndian != 256 ) swap |= SWAP_ENDIAN;
		
		// However, that is not all, the words may also be swapped, so check
		// for that. Bits 31:30 should always be zero in the timestamp word
		for( UInt_t i = 0; i < WORD_SIZE; i++ ) {
			word = (swap & SWAP_ENDIAN) ? Swap64(data[i]) : data[i];
			if( word & 0xC000000000000000LL ) {
				swap |= SWAP_KNOWN;
				break;
			}
			if( word & 0x00000000C0000000LL ) {
				swap |= SWAP_KNOWN;
				swap |= SWAP_WORDS;
				break;
			}
		}
		
	}

	
	// Process all words
	for( UInt_t i = 0; i < WORD_SIZE; i++ ) {
		
		word = GetWord(i);
		word_0 = (word & 0xFFFFFFFF00000000) >> 32;
		word_1 = (word & 0x00000000FFFFFFFF);
		
		real_DataLen = i;
		
		// Check the trailer: reject or keep the block.
		if( ( word_0 & 0xFFFFFFFF ) == 0xFFFFFFFF ||
		   ( word_0 & 0xFFFFFFFF ) == 0x5E5E5E5E ||
		   ( word_1 & 0xFFFFFFFF ) == 0xFFFFFFFF ||
		   ( word_1 & 0xFFFFFFFF ) == 0x5E5E5E5E ){
			
			flag_terminator = true;
			break;
			
		}
		else if( i >= header_DataLen/sizeof(ULong64_t) ){
			
			flag_terminator = true;
			real_DataLen = header_DataLen/sizeof(ULong64_t);
			break;
			
		}
		
	}

	// Process data in the buffer
	for( UInt_t i = 0; i < real_DataLen; i++ ) {
			
		word = GetWord(i);
		word_0 = (word & 0xFFFFFFFF00000000) >> 32;
		word_1 = (word & 0x00000000FFFFFFFF);

		// Data type is highest two bits
		my_type = ( word_0 >> 30 ) & 0x3;
		
		// ADC data - assume CAEN for now, before we generalise
		if( my_type == 0x3 ){
			
			// Check if it's new data
			ProcessCAENData();
			FinishCAENData();

		}
		
		// Information data
		else if( my_type == 0x2 ){
			
			ProcessInfoData();

		}
		
		// Trace header
		else if( my_type == 0x1 ){
			
			// Process trace data
			i = ProcessTraceData(i);
			FinishCAENData();

		}
		
		else {
			
			// output error message!
			std::cerr << "WARNING: WRONG TYPE! word 0: " << std::hex << " 0x";
			std::cerr << word_0 << std::dec << ", my_type: " << my_type << std::endl;
		
		}
		
	} // loop - i < header_DataLen
	
	return;

}

bool GreatConverter::GetCAENChanID(){
	
	// ADCchannelIdent are bits 28:16
	// mod_id= bit 12:8, data_id= bit 7:6, ch_id= bit 5:0
	// data_id: Qlong = 0; Qshort = 1; baseline = 2; fine timing = 3
	unsigned int ADCchanIdent = (word_0 >> 16) & 0x1FFF; // 13 bits from 16
	my_mod_id = (ADCchanIdent >> 8) & 0x001F; // 5 bits from 8
	my_data_id = (ADCchanIdent >> 6 ) & 0x0003; // 2 bits from 6
	my_ch_id = ADCchanIdent & 0x003F; // 6 bits from 0
	
	if( my_mod_id < set->GetNumberOfCAENModules() &&
	    my_ch_id < set->GetNumberOfCAENChannels() )
		return true;
	else return false;
	
}

void GreatConverter::ProcessCAENData(){

	// CAEN data format
	my_adc_data = word_0 & 0xFFFF; // 16 bits from 0
	GetCAENChanID();
	
	// Check things make sense
	if( my_mod_id >= set->GetNumberOfCAENModules() ||
		my_ch_id >= set->GetNumberOfCAENChannels() ) {
		
		std::cout << "Bad CAEN event with mod_id=" << (int) my_mod_id;
		std::cout << " ch_id=" << (int) my_ch_id;
		std::cout << " data_id=" << (int) my_data_id << std::endl;
		return;

	}
	
	// reconstruct time stamp= MSB+LSB
	my_tm_stp_lsb = word_1 & 0x0FFFFFFF;  // 28 bits from 0
	my_tm_stp = ( my_tm_stp_msb << 28 ) | my_tm_stp_lsb;
	
	// Get timestamp in the correct units
	my_tm_stp *= set->GetCAENTimeStampUnits( my_mod_id );

	// Make a test of whether we have new data
	bool flag_new_data = false;
	if( !flag_caen_data0 && !flag_caen_data1 &&
	    !flag_caen_data2 && !flag_caen_data3 &&
	   !flag_caen_trace ) flag_new_data = true;

	// Make a test of whether we have finished a data packet
	bool flag_finished = false;
	if( flag_caen_data0 ) {

		// If we have the PSD firmware, we need the short energy too
		if( set->GetCAENFirmware( caen_data->GetModule() ) == "PSD" ){
			if( flag_caen_data1 && ( flag_caen_data2 || flag_caen_data3 ) ) flag_finished = true;
		}

		// but for the PHA, we just get one energy item
		else {
			flag_caen_data1 = true;
			flag_caen_data3 = true;
			flag_finished = true;
		}

	}

	// First of the data items
	if( flag_new_data ){

		// Make a CaenData item, need to add Qlong, Qshort and traces
		caen_data->SetTimeStamp( my_tm_stp );
		caen_data->SetModule( my_mod_id );
		caen_data->SetChannel( my_ch_id );

	}
	
	// If we already have all the data items, then the next event has
	// already occured before we found traces. This means that there
	// is not trace data. So set the flag to be true and finish the
	// event with an empty trace.
	else if( flag_finished ){
		
		// Finish up the previous event
		flag_caen_trace = true;
		FinishCAENData();

		// Then set the info correctly for the next event
		caen_data->SetTimeStamp( my_tm_stp );
		caen_data->SetModule( my_mod_id );
		caen_data->SetChannel( my_ch_id );
		
	}

	// Qlong
	if( my_data_id == 0 ) {
		
		// Check we don't already have this data type
		if( flag_caen_data0 ) {
			std::cout << "Got data0 twice for the same event" << std::endl;
			return;
		}
		
		// Fill histograms
		hcaen_qlong[my_mod_id][my_ch_id]->Fill( my_adc_data );
		if( my_adc_data == 0xFFFF ) caen_data->SetQlong( 0 );
		else caen_data->SetQlong( my_adc_data );
		flag_caen_data0 = true;

	}
	
	// Qshort
	if( my_data_id == 1 ) {
		
		// Check we don't already have this data type
		if( flag_caen_data1 ) {
			std::cout << "Got data1 twice for the same event" << std::endl;
			return;
		}
		
		my_adc_data = my_adc_data & 0x7FFF; // 15 bits from 0
		hcaen_qshort[my_mod_id][my_ch_id]->Fill( my_adc_data );
		if( my_adc_data == 0x7FFF ) caen_data->SetQshort( 0 );
		else caen_data->SetQshort( my_adc_data );
		flag_caen_data1 = true;

	}

	// MIDAS says my_data_id == 2 or 3
	//  2: basline
	//  3: fine timing
	// http://npg.dl.ac.uk/documents/edoc504/edoc504.html
	
	// Baseline
	if( my_data_id == 2 ) {
		
		// Check we don't already have this data type
		if( flag_caen_data2 ) {
			std::cout << "Got data2 twice for the same event" << std::endl;
			return;
		}

		my_adc_data = my_adc_data & 0xFFFF; // 16 bits from 0
		flag_caen_data2 = true;

		caen_data->SetFineTime( 0.0 );
		caen_data->SetBaseline( (float)my_adc_data / 4. );

	}
	
	// Fine timing
	if( my_data_id == 3 ) {
		
		// Check we don't already have this data type
		if( flag_caen_data3 ) {
			std::cout << "Got data3 twice for the same event" << std::endl;
			return;
		}

		my_adc_data = my_adc_data & 0x03FF; // 10 bits from 0
		flag_caen_data3 = true;

		// CAEN timestamps are 4 ns precision for V1725 and 2 ns for V1730
		if( set->GetCAENModel( my_mod_id ) == 1730 )
			caen_data->SetFineTime( (float)my_adc_data * 2. / 1000. );
		else if( set->GetCAENModel( my_mod_id ) == 1725 )
			caen_data->SetFineTime( (float)my_adc_data * 4. / 1000. );
		caen_data->SetBaseline( 0.0 );

	}
	
	return;

}

int GreatConverter::ProcessTraceData( int pos ){
	
	// Channel ID, etc
	if( !GetCAENChanID() ) return pos;
	
	// contains the sample length
	nsamples = word_0 & 0xFFFF; // 16 bits from 0
	
	// reconstruct time stamp= MSB+LSB
	my_tm_stp_lsb = word_1 & 0x0FFFFFFF;  // 28 bits from 0
	my_tm_stp = ( my_tm_stp_msb << 28 ) | my_tm_stp_lsb;
	
	// Get timestamp in the correct units
	my_tm_stp *= set->GetCAENTimeStampUnits( my_mod_id );

	// Check the info because the trace comes first
	caen_data->SetTimeStamp( my_tm_stp );
	caen_data->SetModule( my_mod_id );
	caen_data->SetChannel( my_ch_id );

	// Get the samples from the trace
	for( UInt_t j = 0; j < nsamples/4; j++ ){
		
		// get next word
		ULong64_t sample_packet = GetWord(++pos);
		
		UInt_t block_test = ( sample_packet >> 32 ) & 0x00000000FFFFFFFF;
		if( block_test != 0x5E5E5E5E ){
			
			// Pairs need to be swapped
			caen_data->AddSample( ( sample_packet >> 32 ) & 0x0000000000003FFF );
			caen_data->AddSample( ( sample_packet >> 48 ) & 0x0000000000003FFF );
			caen_data->AddSample( sample_packet & 0x0000000000003FFF );
			caen_data->AddSample( ( sample_packet >> 16 ) & 0x0000000000003FFF );
			
		}
		
		else {
			
			//std::cout << "This isn't a trace anymore..." << std::endl;
			//std::cout << "Sample #" << j << " of " << nsamples << std::endl;
			//std::cout << " trace_test = " << (int)trace_test << std::endl;

			pos--;
			break;
			
		}
		
	}
	
	flag_caen_trace = true;

	return pos;
	
}


void GreatConverter::FinishCAENData(){
	
	// Check items
	bool caen_data_check = flag_caen_data0 && flag_caen_data1 && ( flag_caen_data2 || flag_caen_data3 ) && flag_caen_trace;
	bool event_ts_check = (long long)my_tm_stp != (long long)caen_data->GetTimeStamp();
	
	// Got all items
	if( caen_data_check ){

		// Fill histograms
		hcaen_hit[caen_data->GetModule()]->Fill( ctr_caen_hit[caen_data->GetModule()], caen_data->GetTime(), 1 );

		// Difference between Qlong and Qshort
		int qdiff = (int)caen_data->GetQlong() - (int)caen_data->GetQshort();
		hcaen_qdiff[caen_data->GetModule()][caen_data->GetChannel()]->Fill( qdiff );

		// Choose the energy we want to use
		unsigned short adc_value = 0;
		std::string entype = cal->CaenType( caen_data->GetModule(), caen_data->GetChannel() );
		if( entype == "Qlong" ) adc_value = caen_data->GetQlong();
		else if( entype == "Qshort" ) adc_value = caen_data->GetQshort();
		else if( entype == "Qdiff" ) adc_value = caen_data->GetQdiff();
		else {
			std::cerr << "Incorrect CAEN energy type must be Qlong, Qshort or Qdiff" << std::endl;
			adc_value = caen_data->GetQlong();
		}
		my_energy = cal->CaenEnergy( caen_data->GetModule(), caen_data->GetChannel(), adc_value );
		caen_data->SetEnergy( my_energy );
		hcaen_cal[caen_data->GetModule()][caen_data->GetChannel()]->Fill( my_energy );

		// Check if it's over threshold
		if( adc_value > cal->CaenThreshold( caen_data->GetModule(), caen_data->GetChannel() ) )
			caen_data->SetThreshold( true );
		else caen_data->SetThreshold( false );


		// Set this data and fill event to tree
		// Also add the time offset when we do this
		caen_data->SetTimeStamp( caen_data->GetTimeStamp() + cal->CaenTime( caen_data->GetModule(), caen_data->GetChannel() ) );
		if( !flag_source ) {
			std::shared_ptr<GreatDataPackets> data_packet =
				std::make_shared<GreatDataPackets>( caen_data );
			data_vector.emplace_back( data_packet );
			data_map.push_back( std::make_pair<unsigned long,double>(
				data_vector.size()-1, data_packet->GetTime() ) );
		}

	}
	
	// missing something
	else if( event_ts_check ) {
		
		std::cout << "Missing something in CAEN data and new event occured" << std::endl;
		std::cout << " Qlong       = " << flag_caen_data0 << std::endl;
		std::cout << " Qshort      = " << flag_caen_data1 << std::endl;
		std::cout << " baseline    = " << flag_caen_data2 << std::endl;
		std::cout << " fine timing = " << flag_caen_data3 << std::endl;
		std::cout << " trace data  = " << flag_caen_trace << std::endl;
		std::cout << "TS: 0x" << std::hex << my_tm_stp << " =/= 0x";
		std::cout << caen_data->GetTimeStamp() << std::dec << std::endl;
		
	}

	// This is normal, just not finished yet
	else return;

	// Count the hit, even if it's bad
	ctr_caen_hit[caen_data->GetModule()]++;
	
	// Assuming it did finish, in a good way or bad, clean up.
	flag_caen_data0 = false;
	flag_caen_data1 = false;
	flag_caen_data2 = false;
	flag_caen_data3 = false;
	flag_caen_trace = false;
	info_data->ClearData();
	caen_data->ClearData();
	
	return;

}

void GreatConverter::ProcessInfoData(){

	// MIDAS info data format
	my_mod_id = (word_0 >> 24) & 0x003F; // bits 24:29

	my_info_field = word_0 & 0x000FFFFF; //bits 0:19
	my_info_code = (word_0 >> 20) & 0x0000000F; //bits 20:23
	my_tm_stp_lsb = word_1 & 0x0FFFFFFF;  //bits 0:27

	// HSB of timestamp
	if( my_info_code == set->GetTimestampCode() ) {
		
		my_tm_stp_hsb = my_info_field & 0x0000FFFF;

	}
	
	// MSB of timestamp in sync pulse or CAEN extended time stamp
	else if( my_info_code == set->GetSyncCode() ) {
		
		// In CAEN this is the extended timestamp
		my_tm_stp_msb = my_info_field & 0x000FFFFF;
		my_tm_stp = ( my_tm_stp_hsb << 48 ) | ( my_tm_stp_msb << 28 ) | ( my_tm_stp_lsb & 0x0FFFFFFF );

	}
	
	// Otherwise it's useful info for the tree
	else {
		
		// Create an info event and fill the tree for info codes
		info_data->SetModule( my_mod_id );
		info_data->SetTimeStamp( my_tm_stp );
		info_data->SetCode( my_info_code );

		if( !flag_source ) {
			std::shared_ptr<GreatDataPackets> data_packet =
				std::make_shared<GreatDataPackets>( info_data );
			data_vector.emplace_back( data_packet );
			data_map.push_back( std::make_pair<unsigned long,double>(
				data_vector.size()-1, data_packet->GetTime() ) );
		}

	}

	return;
	
}

// Common function called to process data in a block from file or DataSpy
bool GreatConverter::ProcessCurrentBlock( int nblock ) {
	
	// Process header.
	ProcessBlockHeader( nblock );

	// Process the main block data until terminator found
	data = (ULong64_t *)(block_data);
	ProcessBlockData( nblock );
			
	// Check once more after going over left overs....
	if( !flag_terminator ){

		std::cerr << std::endl << __PRETTY_FUNCTION__ << std::endl;
		std::cerr << "\tERROR - Didn't complete block data correctly.\n";
		return false;
		
	}
	
	return true;

}

// Function to convert a block of data from DataSpy
int GreatConverter::ConvertBlock( char *input_block, int nblock ) {
	
	// Get the header.
	std::memmove( &block_header, &input_block[0], HEADER_SIZE );
	
	// Get the block
	std::memmove( &block_data, &input_block[HEADER_SIZE], MAIN_SIZE );
	
	// Process the data
	ProcessCurrentBlock( nblock );
	
	// Print time
	//std::cout << "Last time stamp of block = " << my_tm_stp << std::endl;

	return nblock+1;
	
}

// Function to run the conversion for a single file
int GreatConverter::ConvertFile( std::string input_file_name,
							 unsigned long start_block,
							 long end_block ) {
	
	// Read the file.
	std::ifstream input_file( input_file_name, std::ios::in|std::ios::binary );
	if( !input_file.is_open() ){
		
		std::cout << "Cannot open " << input_file_name << std::endl;
		return -1;
		
	}
	
	// Reset counters
	StartFile();

	// Conversion starting
	std::cout << "Converting file: " << input_file_name;
	std::cout << " from block " << start_block << std::endl;
	
	
	// Calculate the size of the file.
	input_file.seekg( 0, input_file.end );
	unsigned long long size_end = input_file.tellg();
	input_file.seekg( 0, input_file.beg );
	unsigned long long size_beg = input_file.tellg();
	unsigned long long FILE_SIZE = size_end - size_beg;
	
	// Calculate the number of blocks in the file.
	unsigned long BLOCKS_NUM = FILE_SIZE / DATA_BLOCK_SIZE;
	
	//a sanity check for file size...
	//QQQ: add more strict test?
	if( FILE_SIZE % DATA_BLOCK_SIZE != 0 ){
		
		std::cout << " *WARNING* " << __PRETTY_FUNCTION__;
		std::cout << "\tMissing data blocks?" << std::endl;

	}
	
	sslogs << "\t File size = " << FILE_SIZE << std::endl;
	sslogs << "\tBlock size = " << DATA_BLOCK_SIZE << std::endl;
	sslogs << "\t  N blocks = " << BLOCKS_NUM << std::endl;

	std::cout << sslogs.str() << std::endl;
	sslogs.str( std::string() ); // clean up
	
	// Data format: http://npg.dl.ac.uk/documents/edoc504/edoc504.html
	// The information is split into 2 words of 32 bits (4 byte).
	// We will collect the data in 64 bit words and split later
	
	
	// Loop over all the blocks.
	for( unsigned long nblock = 0; nblock < BLOCKS_NUM ; nblock++ ){
		
		// Take one block each time and analyze it.
		if( nblock % 200 == 0 || nblock+1 == BLOCKS_NUM ) {
			
			// Percent complete
			float percent = (float)(nblock+1)*100.0/(float)BLOCKS_NUM;
			
			// Progress bar in GUI
			if( _prog_ ) {
				
				prog->SetPosition( percent );
				gSystem->ProcessEvents();
				
			}

			// Progress bar in terminal
			std::cout << " " << std::setw(8) << std::setprecision(4);
			std::cout << percent << "%\r";
			std::cout.flush();

		}

		
		// Get the header.
		input_file.read( (char*)&block_header, HEADER_SIZE );
		// Get the block
		input_file.read( (char*)&block_data, MAIN_SIZE );


		// Check if we are before the start block or after the end block
		if( nblock < start_block || ( (long)nblock > end_block && end_block > 0 ) )
			continue;

		// Process current block. If it's the end, stop.
		if( !ProcessCurrentBlock( nblock ) ) break;
		
		
	} // loop - nblock < BLOCKS_NUM
	
	// Close input
	input_file.close();
	
	// Print time
	//std::cout << "Last time stamp in file = " << my_tm_stp << std::endl;
	
	return BLOCKS_NUM;
	
}

bool GreatConverter::MapComparator( const std::pair<unsigned long,double> &lhs,
								    const std::pair<unsigned long,double> &rhs ) {

	return lhs.second < rhs.second;

}

void GreatConverter::SortDataMap() {

	// Sort the data vector as we go along
	std::sort( data_map.begin(), data_map.end(), MapComparator );

}

unsigned long long GreatConverter::SortTree( bool do_sort ){

	// Reset the sorted tree so it's empty before we start
	sorted_tree->Reset();
	
	// Get number of data packets
	long long int n_ents = data_vector.size();	// std::vector method

	// Check we have entries and build time-ordered index
	if( n_ents && do_sort ) {
		std::cout << "Time ordering " << n_ents << " data items..." << std::endl;
		SortDataMap();
	}
	else return 0;

	// Loop on t_raw entries and fill t
	std::cout << "Writing time-ordered data items to the output tree..." << std::endl;
	for( long long int i = 0; i < n_ents; ++i ) {

		// Get the data item back from the vector
		unsigned long idx = data_map[i].first;
		write_packet->SetData( data_vector[idx] );

		// Fill the sorted tree
		sorted_tree->Fill();

		// Progress bar
		bool update_progress = false;
		if( n_ents < 200 )
			update_progress = true;
		else if( i % (n_ents/100) == 0 || i+1 == n_ents )
			update_progress = true;

		if( update_progress ) {

			// Percent complete
			float percent = (float)(i+1)*100.0/(float)n_ents;

			// Progress bar in GUI
			if( _prog_ ) {

				prog->SetPosition( percent );
				gSystem->ProcessEvents();

			}

			// Progress bar in terminal
			std::cout << " " << std::setw(6) << std::setprecision(4);
			std::cout << percent << "%    \r";
			std::cout.flush();

		} // progress bar

	} // i

	return n_ents;

}
