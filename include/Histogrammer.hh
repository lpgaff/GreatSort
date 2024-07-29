#ifndef __HISTOGRAMMER_HH
#define __HISTOGRAMMER_HH

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

#include <TFile.h>
#include <TTree.h>
#include <TMath.h>
#include <TRandom3.h>
#include <TChain.h>
#include <TProfile.h>
#include <TH1.h>
#include <TH2.h>
#include <TCutG.h>
#include <TGProgressBar.h>
#include <TSystem.h>


// Reaction header
#ifndef __REACTION_HH
# include "Reaction.hh"
#endif

// Great Events tree
#ifndef __GREATEVTS_HH
# include "GreatEvts.hh"
#endif

// Settings file
#ifndef __SETTINGS_HH
# include "Settings.hh"
#endif


// Compiler switch for the pside only histogramming
// uncomment the line below to ignore the p/n coincidences
//#define pside_only


class GreatHistogrammer {
	
public:

	GreatHistogrammer( std::shared_ptr<GreatReaction> myreact, std::shared_ptr<GreatSettings> myset );
	virtual ~GreatHistogrammer(){};
	
	void MakeHists();
	void ResetHists();
	unsigned long FillHists();
	
	void SetInputFile( std::vector<std::string> input_file_names );
	void SetInputFile( std::string input_file_name );
	void SetInputTree( TTree* user_tree );

	inline void SetOutput( std::string output_file_name ){
		output_file = new TFile( output_file_name.data(), "recreate" );
		MakeHists();
	};
	inline void CloseOutput(){
		PurgeOutput();
		output_file->Close();
		//input_tree->ResetBranchAddresses();
	};
	inline void PurgeOutput(){ output_file->Purge(2); }

	inline TFile* GetFile(){ return output_file; };
	
	inline void AddProgressBar( std::shared_ptr<TGProgressBar> myprog ){
		prog = myprog;
		_prog_ = true;
	};
	

private:
	
	// Reaction
	std::shared_ptr<GreatReaction> react;
	
	// Settings file
	std::shared_ptr<GreatSettings> set;
	
	/// Input tree
	TChain *input_tree;
	GreatEvts *read_evts = nullptr;
	std::shared_ptr<GreatTACEvt> tac_evt;
	std::shared_ptr<GreatGammaRayEvt> gamma_evt;
	
	/// Output file
	TFile *output_file;
	
	// Progress bar
	bool _prog_;
	std::shared_ptr<TGProgressBar> prog;

	// Counters
	unsigned long n_entries;
	
	//------------//
	// Histograms //
	//------------//
	
	// Timing
	TH1F *gamma_gamma_td, *gamma_tac_td;

	
};

#endif
