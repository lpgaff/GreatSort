#ifndef __EVENTBUILDER_HH
#define __EVENTBUILDER_HH

#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <memory>

#include <TFile.h>
#include <TTree.h>
#include <TTreeIndex.h>
#include <TMath.h>
#include <TChain.h>
#include <TH1.h>
#include <TH2.h>
#include <TProfile.h>
#include <TVector2.h>
#include <TVector3.h>
#include <TGProgressBar.h>
#include <TSystem.h>


// Settings header
#ifndef __SETTINGS_HH
# include "Settings.hh"
#endif

// Calibration header
#ifndef __CALIBRATION_HH
# include "Calibration.hh"
#endif

// Data packets header
#ifndef __DATAPACKETS_HH
# include "DataPackets.hh"
#endif

// Great Events tree
#ifndef __GREATEVTS_HH
# include "GreatEvts.hh"
#endif

/*!
* \brief Builds physics events after all hits have been time sorted.
*
* \details The GreatEventBuilder Class takes a list of time-sorted events from all of the detectors, and packages them up into a series of physics events.
*
* When the event window closes, each detector has its own "finder function":
* These functions process the events on each detector, imposing prompt coincidence conditions amongst other sanity checks.
* Once processed, all of these hits on the different detectors are packaged up into a single event in an GreatEvts tree.
* The constructor for this class requires an GreatSettings object, which allows it to use parameters defined in the "settings.dat" file. This includes:
* - Settings which encode the wiring of the detectors e.g. the number of CAEN modules used.
* - The size of the event window used to combine events
*
* This size of the event window is crucial for determining which signals belong to which events.
* The default parameter for this is 3 microseconds.
*/

class GreatEventBuilder {
	
public:
	
	GreatEventBuilder( std::shared_ptr<GreatSettings> myset ); ///< Constructor
	virtual ~GreatEventBuilder(){}; /// Destructor (currently empty)

	void	SetInputFile( std::string input_file_name ); ///< Function to set the input data file from which events are built
	void	SetInputTree( TTree* user_tree ); ///< Grabs the input tree from the input file defined in GreatEventBuilder::SetInputFile

	void	SetOutput( std::string output_file_name ); ///< Configures the output for the class

	void	StartFile();	///< Called for every file
	void	Initialise();	///< Called for every event
	void	MakeHists(); ///< Creates histograms for events that occur
	void	ResetHists(); ///< Empties the histograms during the DataSpy

	/// Adds the calibration from the external calibration file to the class
	/// \param[in] mycal The GreatCalibration object which is constructed by the GreatCalibration constructor used in iss_sort.cc
	inline void AddCalibration( std::shared_ptr<GreatCalibration> mycal ){
		cal = mycal;
		overwrite_cal = true;
	};
	
	unsigned long	BuildEvents(); ///< The heart of this class
	unsigned long	BuildSimulatedEvents(); ///< The heart of this class

	// Resolve multiplicities etc
	void TACFinder(); ///< Processes all hits in the TAC  that fall within the build window
	void CeBr3Finder(); ///< Processes hits in the CeBr3 detectors
	void HPGeFinder(); ///< Processes hits in the HPGe detectors

	inline TFile* GetFile(){ return output_file; }; ///< Getter for the output_file pointer
	inline TTree* GetTree(){ return output_tree; }; ///< Getter for the output tree pointer
	
	inline void CloseOutput(){
		output_tree->ResetBranchAddresses();
		PurgeOutput();
		output_file->Close();
		input_file->Close();
		log_file.close(); //?? to close or not to close?
	}; ///< Closes the output files from this class
	inline void PurgeOutput(){ output_file->Purge(2); }
	void CleanHists(); ///< Deletes histograms from memory and clears vectors that store histograms

	inline void AddProgressBar( std::shared_ptr<TGProgressBar> myprog ){
		prog = myprog;
		_prog_ = true;
	}; ///< Adds a progress bar to the GUI
	///< \param[in] myprog pointer to the EventBuilder progress bar for the GUI


private:
	
	/// Input treze
	TFile *input_file; ///< Pointer to the time-sorted input ROOT file
	TTree *input_tree; ///< Pointer to the TTree in the data input file
	GreatDataPackets *in_data = nullptr; ///< Pointer to the TBranch containing the data in the time-sorted input ROOT file
	std::shared_ptr<GreatCaenData> caen_data; ///< Pointer to a given entry in the tree of some data from the CAEN
	std::shared_ptr<GreatInfoData> info_data; ///< Pointer to a given entry in the tree of the "info" datatype

	/// Event structures
	std::shared_ptr<GreatTACEvt> tac_evt;
	std::shared_ptr<GreatCeBr3Evt> cebr3_evt;
	std::shared_ptr<GreatHPGeEvt> hpge_evt;

	/// Outputs
	TFile *output_file; ///< Pointer to the output ROOT file containing events
	TTree *output_tree; ///< Pointer to the output ROOT tree containing events
	std::unique_ptr<GreatEvts> write_evts; ///< Container for storing hits on all detectors in order to construct events
	
	// Do calibration
	std::shared_ptr<GreatCalibration> cal; ///< Pointer to an GreatCalibration object, used for accessing gain-matching parameters and thresholds
	bool overwrite_cal; ///< Boolean determining whether an energy calibration should be used (true) or not (false). Set in the GreatEventBuilder::AddCalibration function
	
	// Settings file
	std::shared_ptr<GreatSettings> set; ///< Pointer to the settings object. Assigned in constructor
	
	// Progress bar
	bool _prog_; ///< Boolean determining if there is a progress bar (in the GUI)
	std::shared_ptr<TGProgressBar> prog; ///< Progress bar for the GUI

	// Log file
	std::ofstream log_file; ///< Log file for recording the results of the GreatEventBuilder
	
	// Flag to know we've opened a file on disk
	bool flag_input_file;
	
	// These things are in the settings file
	long build_window;  ///< Length of build window in ns
	
	// Flags
	bool flag_close_event; ///< Determines if the event should be closed for a given hit
	bool event_open; ///< Flag for deciding whether an event is currently being recorded or not

	// Time variables
	double		time_diff;	///< Time difference between first hit in event and current hit
	double		time_prev;	///< Holds time of previous event
	double		time_min;	///< The minimum time in an event containing hits
	double		time_max;	///< The maximum time in an event containing hits
	double		time_first;	///< Time of the first event in a file
	double		ebis_prev;	///< Holds time of previous ebis pulse
	double		t1_prev;	///< Holds time of previous T1 pulse
	double		sc_prev;	///< Holds time of previous SuperCycle pulse
	double		laser_prev;	///< Holds time of previous Laser status pulse
	double		caen_time;	///< Time from the caen DAQ
	double		caen_prev;	///< Holds previous time from the CAEN DAQ
	std::vector<double> caen_time_start;		///< Holds the time of the first hit on each caen in the input time-sorted tree (index denotes caen module)
	std::vector<double> caen_time_stop;		///< Holds the time of the last hit on each asic in the input time-sorted tree (index denotes caen module)

	// Data variables - generic
	unsigned char		mymod;		///< module number
	unsigned char		mych;		///< channel number
	double				mytime;		///< absolute timestamp
	float 				myenergy;	///< calibrated energy
	bool				mythres;	///< above threshold?

	// Data variables
	unsigned short		myid;		///< generic detector id
	unsigned short		myseg;		///< generic segment id


	// TAC variables
	std::vector<float>			tac_td_list;	///< list of TAC time difference, calibrated to ps
	std::vector<double>			tac_ts_list;	///< list of TAC time stamps (ns)
	std::vector<short>			tac_id_list;	///< list of TAC IDs for the MWPC

	// CeBr3 variables
	std::vector<float>			cebr3_en_list;	///< list of CeBr3 energies for CeBr3Finder
	std::vector<double>			cebr3_ts_list;	///< list of CeBr3 time stamps (ns) for CeBr3Finder
	std::vector<short>			cebr3_id_list;	///< list of CeBr3 detectors ids for CeBr3Finder

	// HPGe variables
	std::vector<float>			hpge_en_list;	///< list of HPGe energies for HPGeFinder
	std::vector<double>			hpge_ts_list;	///< list of HPGe time stamps (ns) for HPGeFinder
	std::vector<short>			hpge_id_list;	///< list of HPGe detectors ids for HPGeFinder
	std::vector<short>			hpge_seg_list;	///< list of HPGe segments ids for HPGeFinder

	// Counters
	unsigned int		hit_ctr;		///< Counts the number of hits that make up an event within a given file
	unsigned int		tac_ctr;		///< Counts the number of TAC events within a given file
	unsigned int		cebr3_ctr;		///< Counts the number of CeBr3 events within a given file
	unsigned int		hpge_ctr;		///< Counts the number of HPGe events within a given file
	unsigned long		n_caen_data;	///< Counter for number of caen data packets in a file
	unsigned long		n_info_data; 	///< Counter for number of info data packets in a file
	unsigned long long	n_entries; 		///< Number of entries in the time-sorted data input tree

	// Timing histograms
	TH1F *tdiff;					///< Histogram containing the time difference between each real (not infodata) signal in the file
	TH1F *tdiff_clean;				///< Histogram containing the time difference between the real signals *above threshold* (mythres)

	// TAC histograms
	std::vector<TH1F*> htac_id; ///< The TAC singles spectra in the MWPC

	// GammaRay histograms
	TH1F *cebr3_E;						///< Sum gamma-ray energy histogram for CeBr3 detectors
	TH2F *cebr3_E_vs_det;				///< Gamma-ray energy verus detector ID for CeBr3 detectors
	TH2F *cebr3_cebr3_E;				///< Gamma-gamma matrix, no prompt time condition for CeBr3 detectors
	TH1F *cebr3_cebr3_td;				///< Gamma-gamma time difference for CeBr3 detectors
	TH1F *hpge_E;						///< Sum gamma-ray energy histogram for HPGe detectors
	TH2F *hpge_E_vs_det;				///< Gamma-ray energy verus detector ID for HPGe detectors
	TH2F *hpge_hpge_E;					///< Gamma-gamma matrix, no prompt time condition for HPGe detectors
	TH1F *hpge_hpge_td;					///< Gamma-gamma time difference for HPGe detectors
	std::vector<TH1F*> hpge_seg_td;		///< Gamma-gamma time difference for HPGe core and segments

};

#endif

