#include "Histogrammer.hh"

GreatHistogrammer::GreatHistogrammer( std::shared_ptr<GreatReaction> myreact, std::shared_ptr<GreatSettings> myset ){
	
	react = myreact;
	set = myset;
	
	// No progress bar by default
	_prog_ = false;
	
	// Make the histograms track the sum of the weights for correctly
	// performing the error propagation when subtracting
	TH1::SetDefaultSumw2();
	
}

void GreatHistogrammer::MakeHists() {
	
    std::string hname, htitle;
    std::string dirname;
	
	// Timing histograms
	dirname = "Timing";
	if( !output_file->GetDirectory( dirname.data() ) )
		output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );

	// Gamma-gamma time
	hname = "gamma_gamma_td";
	htitle = "Time difference between gamma-ray events";
	htitle += ";#Deltat;Counts";
	gamma_gamma_td = new TH1F( hname.data(), htitle.data(),
									1000, -1.0*set->GetEventWindow()-50, 1.0*set->GetEventWindow()+50 );
	
	// Gamma-TAC time
	hname = "gamma_tac_td";
	htitle = "Time difference between gamma-ray and TAC events";
	htitle += ";#Deltat;Counts";
	gamma_tac_td = new TH1F( hname.data(), htitle.data(),
									1000, -1.0*set->GetEventWindow()-50, 1.0*set->GetEventWindow()+50 );
	
}


void GreatHistogrammer::ResetHists() {
	
	std::cout << "in GreatHistogrammer::Reset_Hist()" << std::endl;
	
	// Timing
	gamma_gamma_td->Reset("ICESM");
	gamma_tac_td->Reset("ICESM");

	return;
	
}

void GreatHistogrammer::SetInputTree( TTree *user_tree ){
	
	// Find the tree and set branch addresses
	input_tree = (TChain*)user_tree;
	input_tree->SetBranchAddress( "GreatEvts", &read_evts );
	
	return;
	
}

void GreatHistogrammer::SetInputFile( std::vector<std::string> input_file_names ) {
	
	/// Overlaaded function for a single file or multiple files
	input_tree = new TChain( "evt_tree" );
	for( unsigned int i = 0; i < input_file_names.size(); i++ ) {
		
		input_tree->Add( input_file_names[i].data() );
		
	}
	input_tree->SetBranchAddress( "GreatEvts", &read_evts );
	
	return;
	
}

void GreatHistogrammer::SetInputFile( std::string input_file_name ) {
	
	/// Overloaded function for a single file or multiple files
	input_tree = new TChain( "evt_tree" );
	input_tree->Add( input_file_name.data() );
	input_tree->SetBranchAddress( "GreatEvts", &read_evts );
	
	return;
	
}

// Main routine for filling the histograms
unsigned long GreatHistogrammer::FillHists() {
	
	/// Main function to fill the histograms
	n_entries = input_tree->GetEntries();
	
	std::cout << " GreatHistogrammer: number of entries in event tree = ";
	std::cout << n_entries << std::endl;
	
	if( !n_entries ){
		
		std::cout << " GreatHistogrammer: Nothing to do..." << std::endl;
		return n_entries;
		
	}
	else {
		
		std::cout << " GreatHistogrammer: Start filling histograms" << std::endl;
		
	}
	
	// ------------------------------------------------------------------------ //
	// Main loop over TTree to find events
	// ------------------------------------------------------------------------ //
	for( unsigned int i = 0; i < n_entries; ++i ){
		
		// Current event data
		input_tree->GetEntry(i);
		
		// tdiff variable
		double tdiff;
		
		// Loop over gamma-ray events
		for( unsigned int j = 0; j < read_evts->GetGammaRayMultiplicity(); ++j ){
			
			// Loop over gamma-ray events
			for( unsigned int k = j+1; k < read_evts->GetGammaRayMultiplicity(); ++k ){
				
				// Time difference
				double tdiff = read_evts->GetGammaRayEvt(j)->GetTime();
				tdiff -= read_evts->GetGammaRayEvt(k)->GetTime();
				gamma_gamma_td->Fill( tdiff );
				gamma_gamma_td->Fill( -tdiff );
				
			}
			
			// Loop over TAC events
			for( unsigned int k = 0; k < read_evts->GetTACMultiplicity(); ++k ){
				
				// Time difference
				double tdiff = read_evts->GetGammaRayEvt(j)->GetTime();
				tdiff -= read_evts->GetTACEvt(k)->GetTime();
				gamma_gamma_td->Fill( -tdiff );
				
			}
			
		}
		
		
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
		
	} // all events
	
	output_file->Write();
	
	return n_entries;
	
}

