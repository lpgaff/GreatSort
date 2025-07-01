#include "EventBuilder.hh"
///////////////////////////////////////////////////////////////////////////////
/// This constructs the event-builder object, setting parameters for this process by grabbing information from the settings file (or using default parameters defined in the constructor)
/// \param[in] myset The GreatSettings object which is constructed by the GreatSettings constructor used in iss_sort.cc
GreatEventBuilder::GreatEventBuilder( std::shared_ptr<GreatSettings> myset ){
	
	// First get the settings
	set = myset;
	
	// No calibration file by default
	overwrite_cal = false;
	
	// No input file at the start by default
	flag_input_file = false;
	
	// No progress bar by default
	_prog_ = false;

	// ------------------------------------------------------------------------ //
	// Initialise variables and flags
	// ------------------------------------------------------------------------ //
	build_window = set->GetEventWindow();

	// Resize vectors to match modules of detectors
	caen_time_start.resize( set->GetNumberOfCAENModules() );
	caen_time_stop.resize( set->GetNumberOfCAENModules() );

	return;
	
}

////////////////////////////////////////////////////////////////////////////////
/// Reset private-member counters, arrays and flags for processing the next input file. 
/// Called in the GreatEventBuilder::SetInputFile and GreatEventBuilder::SetInputTree functions
void GreatEventBuilder::StartFile(){
	
	// Call for every new file
	// Reset counters etc.
	
	time_prev = 0;
	time_min = 0;
	time_max = 0;
	time_first = 0;

	n_caen_data	= 0;
	n_info_data	= 0;

	tac_ctr		= 0;
	gamma_ctr	= 0;

	for( unsigned int i = 0; i < set->GetNumberOfCAENModules(); ++i ) {

		caen_time_start[i] = 0;
		caen_time_stop[i] = 0;

	}

}

////////////////////////////////////////////////////////////////////////////////
/// The ROOT file is opened in read-only mode to avoid modification. If the file is not found, an error message is printed and the function does not set an input tree, and does not call GreatEventBuilder::StartFile.
/// \param [in] input_file_name The ROOT file containing the time-sorted events from that run (typical suffix is "_sort.root")
void GreatEventBuilder::SetInputFile( std::string input_file_name ) {
	
	/// Overloaded function for a single file or multiple files
	//input_tree = new TTree( "iss" );
	//input_tree->Add( input_file_name.data() );
	
	// Open next Root input file.
	input_file = new TFile( input_file_name.data(), "read" );
	if( input_file->IsZombie() ) {
		
		std::cout << "Cannot open " << input_file_name << std::endl;
		return;
		
	}
	
	flag_input_file = true;
	
	// Set the input tree
	SetInputTree( (TTree*)input_file->Get("great_sort") );
	StartFile();

	return;
	
}

////////////////////////////////////////////////////////////////////////////////
/// Sets the private member input_tree to the parameter user_tree, sets the branch address and calls the GreatEventBuilder::StartFile function
/// \param [in] user_tree The name of the tree in the ROOT file containing the time-sorted events
void GreatEventBuilder::SetInputTree( TTree *user_tree ){
	
	// Find the tree and set branch addresses
	input_tree = user_tree;
	input_tree->SetBranchAddress( "data", &in_data );

	return;
	
}


////////////////////////////////////////////////////////////////////////////////
/// Constructs a number of objects for storing measurements from different detectors, that can then be wrapped up into physics events. Also creates an output file and output tree, and calls the GreatEventBuilder::MakeHists function
/// \param [in] output_file_name The ROOT file for storing the events from the event-building process (typical suffix is "_events.root")
void GreatEventBuilder::SetOutput( std::string output_file_name ) {

	// These are the branches we need
	write_evts	= std::make_unique<GreatEvts>();
	tac_evt		= std::make_shared<GreatTACEvt>();
	gamma_evt	= std::make_shared<GreatGammaRayEvt>();

	// ------------------------------------------------------------------------ //
	// Create output file and create events tree
	// ------------------------------------------------------------------------ //
	output_file = new TFile( output_file_name.data(), "recreate" );
	output_tree = new TTree( "evt_tree", "evt_tree" );
	output_tree->Branch( "GreatEvts", "GreatEvts", write_evts.get() );
	output_tree->SetAutoFlush();

	// Create log file.
	std::string log_file_name = output_file_name.substr( 0, output_file_name.find_last_of(".") );
	log_file_name += ".log";
	log_file.open( log_file_name.data(), std::ios::app );
	
	// Hisograms in separate function
	MakeHists();
	
}

////////////////////////////////////////////////////////////////////////////////
/// Clears the vectors that store energies, time differences, ids, module numbers, row numbers, recoil sectors etc. Also resets flags that are relevant for building events
void GreatEventBuilder::Initialise(){

	/// This is called at the end of every execution/loop
	
	flag_close_event = false;
	event_open = false;
	
	hit_ctr = 0;
	
	// Now swap all these vectors with empty vectors to ensure they are fully cleared
	std::vector<float>().swap(tactd_list);
	std::vector<double>().swap(tacts_list);
	std::vector<char>().swap(tacid_list);

	std::vector<float>().swap(gen_list);
	std::vector<double>().swap(gts_list);
	std::vector<char>().swap(gid_list);

	write_evts->ClearEvt();
	
	return;
	
}

////////////////////////////////////////////////////////////////////////////////
/// This loops over all events found in the input file and wraps them up and stores them in the output file
/// \return The number of entries in the tree that have been sorted (=0 if there is an error)
unsigned long GreatEventBuilder::BuildEvents() {
	
	/// Function to loop over the sort tree and build array and recoil events

	// Load the full tree if possible
	//output_tree->SetMaxVirtualSize(5e8); // 500 MB
	//input_tree->SetMaxVirtualSize(5e8); // 500 MB
	//input_tree->LoadBaskets(5e8); // Load 500 MB of data to memory

	if( input_tree->LoadTree(0) < 0 ){
		
		std::cout << " Event Building: nothing to do" << std::endl;
		return 0;
		
	}
	
	// Get ready and go
	Initialise();
	n_entries = input_tree->GetEntries();

	std::cout << " Event Building: number of entries in input tree = ";
	std::cout << n_entries << std::endl;

	// ------------------------------------------------------------------------ //
	// Main loop over TTree to find events
	// ------------------------------------------------------------------------ //
	for( unsigned long i = 0; i < n_entries; ++i ) {
		
		// Get time-ordered event index (with or without walk correction)
		unsigned long long idx = i; // no correction

		// Current event data
		if( i == 0 ) input_tree->GetEntry(idx);
		
		// Get the time of the event
		mytime = in_data->GetTime();

		// check time stamp monotonically increases!
		// but allow for the fine time of the CAEN system
		if( (unsigned long long)time_prev > in_data->GetTimeStamp() + 5.0 ) {
			
			std::cout << "Out of order event in file ";
			std::cout << input_tree->GetName() << std::endl;
			
		}
		
		// record time of this event
		time_prev = mytime;
		
		// assume this is above threshold initially
		mythres = true;
		
		
		// ------------------------------------------ //
		// Find CAEN ADC data
		// ------------------------------------------ //
		if( in_data->IsCaen() ) {
			
			// Increment event counter
			n_caen_data++;
			
			caen_data = in_data->GetCaenData();
			mymod = caen_data->GetModule();
			mych = caen_data->GetChannel();

			//std::cout << i << "\t" << caen_data->GetTimeStamp() << "\t" << caen_data->GetFineTime() << std::endl;
			
			if( overwrite_cal ) {
				
				std::string entype = cal->CaenType( mymod, mych );
				unsigned short adc_value = 0;
				if( entype == "Qlong" ) adc_value = caen_data->GetQlong();
				else if( entype == "Qshort" ) adc_value = caen_data->GetQshort();
				else if( entype == "Qdiff" ) adc_value = caen_data->GetQdiff();
				else {
					std::cerr << "Incorrect CAEN energy type must be Qlong, Qshort or Qdiff" << std::endl;
					adc_value = caen_data->GetQlong();
				}
				myenergy = cal->CaenEnergy( mymod, mych, adc_value );
				
				if( adc_value < cal->CaenThreshold( mymod, mych ) )
					mythres = false;

			}
			
			else {
				
				myenergy = caen_data->GetEnergy();
				mythres = caen_data->IsOverThreshold();

			}
			
			// If it's below threshold do not use as window opener
			if( mythres ) event_open = true;

			// DETERMINE WHICH TYPE OF CAEN EVENT THIS IS
			// Is it a TAC?
			if( set->IsTAC( mymod, mych ) && mythres ) {
				
				myid = set->GetTACID( mymod, mych );

				tactd_list.push_back( myenergy );
				tacts_list.push_back( mytime );
				tacid_list.push_back( myid );

				hit_ctr++; // increase counter for bits of data included in this event

			}
			
			// Is it a Gamma-ray?
			else if( set->IsGammaRay( mymod, mych ) && mythres ) {
			
				myid = set->GetGammaRayDetector( mymod, mych );
				
				gen_list.push_back( myenergy );
				gts_list.push_back( mytime );
				gid_list.push_back( myid );
				
				hit_ctr++; // increase counter for bits of data included in this event

			}
			

			// Is it the start event?
			if( caen_time_start.at( mymod ) == 0 )
				caen_time_start.at( mymod ) = mytime;
			
			// or is it the end event (we don't know so keep updating)
			caen_time_stop.at( mymod ) = mytime;

		}
		
		
		// ------------------------------------------ //
		// Find info events, like timestamps etc
		// ------------------------------------------ //
		else if( in_data->IsInfo() ) {
			
			// Increment event counter
			n_info_data++;
			info_data = in_data->GetInfoData();
			
			// if there are no data so far, set this as time_first - multiple info events will just update this so won't be a problem
			if( hit_ctr == 0 )
				time_first = mytime;
			
			
		}
		
		// Sort out the timing for the event window
		// but only if it isn't an info event, i.e only for real data
		if ( !in_data->IsInfo() ){
			
			// if this is first datum included in Event
			if( hit_ctr == 1 && mythres ) {
				
				time_min	= mytime;
				time_max	= mytime;
				time_first	= mytime;
				
			}
			
			// Update max time
			if( mytime > time_max ) time_max = mytime;
			else if( mytime < time_min ) time_min = mytime;
			
		} // not info data

		//------------------------------
		//  check if last datum from this event and do some cleanup
		//------------------------------
		unsigned long long idx_next = i+1;
		
		if( input_tree->GetEntry(idx_next) ) {
					
			// Time difference to next event
			time_diff = in_data->GetTime() - time_first; // no correction

			// window = time_stamp_first + time_window
			if( time_diff > build_window )
				flag_close_event = true; // set flag to close this event

			// we've gone on to the next file in the chain
			else if( time_diff < 0 )
				flag_close_event = true; // set flag to close this event
				
			// Fill tdiff hist only for real data
			if( !in_data->IsInfo() ) {
				
				tdiff->Fill( time_diff );
				if( mythres )
					tdiff_clean->Fill( time_diff );
			
			}

		}
		
		//----------------------------
		// if close this event or last entry
		//----------------------------
		if( flag_close_event || (i+1) == n_entries ) {
			
			// If we opened the event, then sort it out
			if( event_open ) {
			
				//----------------------------------
				// Build array events, recoils, etc
				//----------------------------------
				TACFinder();		// add an TACEvt for pair of TAC events
				GammaRayFinder();	// add a GammaRay event for CeBr detectors

				// Fill only if we have some physics events
				if( write_evts->GetTACMultiplicity() ||
					write_evts->GetGammaRayMultiplicity() )
					output_tree->Fill();

			}
			
			//--------------------------------------------------
			// clear values of arrays to store intermediate info
			//--------------------------------------------------
			Initialise();
			
		} // if close event
				
		// Progress bar
		bool update_progress = false;
		if( n_entries < 200 )
			update_progress = true;
		else if( i % (n_entries/100) == 0 || i+1 == n_entries )
			update_progress = true;
		
		if( update_progress ) {

			// Percent complete
			float percent = (float)(i+1)*100.0/(float)n_entries;

			// Progress bar in GUI
			if( _prog_ ) {
				
				prog->SetPosition( percent );
				gSystem->ProcessEvents();
				
			}

			// Progress bar in terminal
			std::cout << " " << std::setw(6) << std::setprecision(4);
			std::cout << percent << "%    \r";
			std::cout.flush();

		}
		
		
	} // End of main loop over TTree to process raw MIDAS data entries (for n_entries)
	
	// TODO -> if we end on a pause with no resume, add any remaining time to the dead time
	
	//--------------------------
	// Clean up
	//--------------------------
	std::stringstream ss_log;
	ss_log << "\n GreatEventBuilder finished..." << std::endl;
	ss_log << "  CAEN data packets = " << n_caen_data << std::endl;
	for( unsigned int i = 0; i < set->GetNumberOfCAENModules(); ++i ) {
		ss_log << "   Module " << i << " live time = ";
		ss_log << (caen_time_stop[i]-caen_time_start[i])/1e9;
		ss_log << " s" << std::endl;
	}
	ss_log << "  Info data packets = " << n_info_data << std::endl;
	ss_log << "   TAC events = " << tac_ctr << std::endl;
	ss_log << "   Gamma-ray events = " << gamma_ctr << std::endl;
	ss_log << "  Tree entries = " << output_tree->GetEntries() << std::endl;

	std::cout << ss_log.str();
	if( log_file.is_open() && flag_input_file ) log_file << ss_log.str();
	
	std::cout << " Writing output file...\r";
	std::cout.flush();
	
	// Force the rest of the events in the buffer to disk
	output_tree->FlushBaskets();
	output_file->Write( 0, TObject::kWriteDelete );
	//output_file->Print();
	//output_file->Close();
	
	// Dump the input buffers
	input_tree->DropBaskets();

	std::cout << " Writing output file... Done!" << std::endl << std::endl;

	return n_entries;
	
}

////////////////////////////////////////////////////////////////////////////////
/// Assesses the validity of hits in the TAC module
void GreatEventBuilder::TACFinder() {

	//std::cout << __PRETTY_FUNCTION__ << std::endl;
	
	// Loop over TAC events
	for( unsigned int i = 0; i < tactd_list.size(); ++i ) {

		// TAC singles spectra
		htac_id[tacid_list[i]]->Fill( tactd_list[i] );
		
		// Set the TAC event
		tac_evt->SetTACTime( tactd_list[i] );
		tac_evt->SetID( tacid_list[i] );
		gamma_evt->SetSegment( 0 );
		gamma_evt->SetType( 2 );
		tac_evt->SetTime( tacts_list[i] );

		// Write event to tree
		write_evts->AddEvt( tac_evt );
		tac_ctr++;

	}
	
	// Clean up
	//delete tac_evt;

	return;
	
}

////////////////////////////////////////////////////////////////////////////////
/// Builds gamma-ray events from the gamma-ray detectors and maybe also HPGe detectors in the future
void GreatEventBuilder::GammaRayFinder() {
	
	//std::cout << __PRETTY_FUNCTION__ << std::endl;
	
	// Loop over gamma-ray events
	for( unsigned int i = 0; i < gen_list.size(); ++i ) {
		
		// Histogram the data
		gamma_E->Fill( gen_list[i] );
		gamma_E_vs_det->Fill( gid_list[i], gen_list[i] );

		// TODO: Here one could do reconstruction of core-segment hits
		// it is not done yet, but there are examples in MiniballSort
		// or contact me (liam.gaffney@liverpool.ac.uk) if you have a
		// use case for segmented detectors and would like it implementing

		// Coincidences
		for( unsigned int j = i+1; j < gen_list.size(); ++j ) {
			
			double tdiff = gts_list[j] - gts_list[i];
			gamma_gamma_td->Fill( tdiff );
			gamma_gamma_td->Fill( -tdiff );
			
			// Just prompt hits for now in a gg matrix
			// This should really be used for add-back?
			if( TMath::Abs(tdiff) < set->GetGammaRayHitWindow() ){
				
				gamma_gamma_E->Fill( gen_list[i], gen_list[j] );
				gamma_gamma_E->Fill( gen_list[j], gen_list[i] );

			} // prompt
				
		} // j

		// Set the GammaRay event
		gamma_evt->SetEnergy( gen_list[i] );
		gamma_evt->SetID( gid_list[i] );
		gamma_evt->SetSegment( 0 );
		gamma_evt->SetType( 0 );
		gamma_evt->SetTime( gts_list[i] );

		// Write event to tree
		write_evts->AddEvt( gamma_evt );
		gamma_ctr++;
		
	} // i
	
	return;
	
}



////////////////////////////////////////////////////////////////////////////////
/// *This function doesn't fill any histograms*, but just creates them. Called by the GreatEventBuilder::SetOutput function
void GreatEventBuilder::MakeHists(){
	
	std::string hname, htitle;
	std::string dirname, maindirname, subdirname;
	
	// ----------------- //
	// Timing histograms //
	// ----------------- //
	dirname =  "timing";
	if( !output_file->GetDirectory( dirname.data() ) )
		output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );

	tdiff = new TH1F( "tdiff", "Time difference to first trigger;#Delta t [ns]", 1.5e3, -0.5e5, 1.0e5 );
	tdiff_clean = new TH1F( "tdiff_clean", "Time difference to first trigger without noise;#Delta t [ns]", 1.5e3, -0.5e5, 1.0e5 );

	// -------------- //
	// TAC histograms //
	// -------------- //
	dirname = "tac";
	if( !output_file->GetDirectory( dirname.data() ) )
		output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );
	
	htac_id.resize( set->GetNumberOfTACs() );

	// Loop over number of TAC modules
	for( unsigned int i = 0; i < set->GetNumberOfTACs(); ++i ) {

		hname = "tac_" + std::to_string(i);
		htitle = "TAC time, ID " + std::to_string(i) + ";TAC time (ps);Counts per 2 ps";
		htac_id[i] = new TH1F( hname.data(), htitle.data(), 16384, -16384, 16384 );
		
	}
		
	// -------------------- //
	// Gamma-ray histograms //
	// -------------------- //
	dirname = "gammas";
	if( !output_file->GetDirectory( dirname.data() ) )
		output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );
	

	hname = "gamma_E_vs_det";
	htitle = "Gamma-ray energy vs detector ID;Detector ID;Energy [keV];Counts per 2 keV";
	gamma_E_vs_det = new TH2F( hname.data(), htitle.data(),
			set->GetNumberOfGammaRayDetectors()+1, -0.5, set->GetNumberOfGammaRayDetectors()+0.5, 4000, 0, 8000 );

	hname = "gamma_E";
	htitle = "Gamma-ray energy;Energy [keV];Counts per 2 keV";
	gamma_E = new TH1F( hname.data(), htitle.data(), 4000, 0, 8000 );

	hname = "gamma_gamma_E";
	htitle = "Gamma-ray energy coincidence matrix;Energy [keV];Energy [keV];Counts";
	gamma_gamma_E = new TH2F( hname.data(), htitle.data(), 4000, 0, 8000, 4000, 0, 8000 );

	hname = "gamma_gamma_td";
	htitle = "Gamma-gamma time difference;#Deltat [ns];Counts";
	gamma_gamma_td = new TH1F( hname.data(), htitle.data(), 600, -1.0*set->GetEventWindow()-20, set->GetEventWindow()+20 );

	
	return;
	
}

////////////////////////////////////////////////////////////////////////////////
/// This function empties the histograms used in the EventBuilder class; used during the DataSpy
void GreatEventBuilder::ResetHists() {

	for( unsigned int i = 0; i < htac_id.size(); i++ )
		htac_id[i]->Reset("ICESM");
	
	gamma_E->Reset("ICESM");
	gamma_E_vs_det->Reset("ICESM");
	gamma_gamma_E->Reset("ICESM");
	gamma_gamma_td->Reset("ICESM");

	tdiff->Reset("ICESM");
	tdiff_clean->Reset("ICESM");

	return;

}

